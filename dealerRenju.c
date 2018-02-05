/*
Copyright (C) 2011 by the Computer Poker Research Group, University of Alberta
*/

#include <stdlib.h>
#include <stdio.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <getopt.h>
#include "renju.h"
#include "net.h"

/* the ports for players to connect to will be printed on standard out
(in player order)

if log file is enabled, matchName.log will contain finished states
and values, followed by the final total values for each player

if transaction file is enabled, matchName.tlog will contain a list
of actions taken and timestamps that is sufficient to recreate an
interrupted match

if the quiet option is not enabled, standard error will print out
the messages sent to and receieved from the players

the final total values for each player will be printed to both
standard out and standard error

exit value is EXIT_SUCCESS if the match was a success,
or EXIT_FAILURE on any failure */

#define DEFAULT_MAX_INVALID_ACTIONS UINT32_MAX
#define DEFAULT_MAX_RESPONSE_MICROS 600000000

typedef struct
{
	uint32_t maxInvalidActions;
	uint64_t maxResponseMicros;

	uint32_t numInvalidActions[MAX_PLAYERS];
} ErrorInfo;

static void printUsage(FILE *file, int verbose)
{
	fprintf(file, "usage: dealer matchName #Hands p1name p2name ... [options]\n");
	fprintf(file, "  -f use fixed dealer button at table\n");
	fprintf(file, "  -l/L disable/enable log file - enabled by default\n");
	fprintf(file, "  -p player1_port,player2_port,... [default is random]\n");
	fprintf(file, "  -t/T disable/enable transaction file - disabled by default\n");
	fprintf(file, "  -a append to log/transaction files - disabled by default\n");
	fprintf(file, "  --t_response [milliseconds] maximum time per response\n");
	fprintf(file, "  --t_hand [milliseconds] maximum player time per hand\n");
	fprintf(file, "  --t_per_hand [milliseconds] maximum average player time for match\n");
	fprintf(file, "  --start_timeout [milliseconds] maximum time to wait for players to connect\n");
	fprintf(file, "    <0 [default] is no timeout\n");
}

/* returns >= 0 on success, -1 on error */
static int scanPortString(const char *string,
	uint16_t listenPort[MAX_PLAYERS])
{
	int c, r, p;

	c = 0;
	for (p = 0; p < MAX_PLAYERS; ++p) {

		if (string[c] == 0) {
			/* finished parsing the string */

			break;
		}

		if (p) {
			/* look for separator */
			if (string[c] != ',') {
				/* numbers should be comma separated */
				return -1;
			}
			++c;
		}

		if (sscanf(&string[c], "%" SCNu16 "%n", &listenPort[p], &r) < 1) {
			/* couldn't get a number */

			return -1;
		}
		c += r;
	}

	return 0;
}

static void initErrorInfo(const uint32_t maxInvalidActions,
	const uint64_t maxResponseMicros,
	ErrorInfo *info)
{
	int s;

	info->maxInvalidActions = maxInvalidActions;
	info->maxResponseMicros = maxResponseMicros;

	for (s = 0; s < MAX_PLAYERS; ++s) {
		info->numInvalidActions[s] = 0;
	}
}

/* update the number of invalid actions for seat
returns >= 0 if match should continue, -1 for failure */
static int checkErrorInvalidAction(const uint8_t player, ErrorInfo *info)
{
	++(info->numInvalidActions[player]);

	if (info->numInvalidActions[player] > info->maxInvalidActions) {
		return -1;
	}

	return 0;
}

/* returns >= 0 if match should continue, -1 for failure */
static int checkErrorTimes(const struct timeval *sendTime,
	const struct timeval *recvTime,
	ErrorInfo *info)
{
	uint64_t responseMicros;

	/* calls to gettimeofday can return earlier times on later calls :/ */
	if (recvTime->tv_sec < sendTime->tv_sec || (recvTime->tv_sec == sendTime->tv_sec && recvTime->tv_usec < sendTime->tv_usec)) {
		return 0;
	}

	/* figure out how many microseconds the response took */
	responseMicros = (recvTime->tv_sec - sendTime->tv_sec) * 1000000 + recvTime->tv_usec - sendTime->tv_usec;

	/* check time used for the response */
	if (responseMicros > info->maxResponseMicros) {
		return -1;
	}

	return 0;
}

/* returns >= 0 if match should continue, -1 for failure */
static int sendPlayerMessage(const MatchState *state,
	const BoardState *boardState,
	const int quiet, const uint8_t player,
	const int seatFD, struct timeval *sendTime)
{
	int c;
	char line[MAX_LINE_LEN];
	/* prepare the message */
	c = printMatchState(state, boardState, MAX_LINE_LEN, line);
	if (c < 0 || c > MAX_LINE_LEN - 3) {
		/* message is too long */

		fprintf(stderr, "ERROR: state message too long\n");
		return -1;
	}
	line[c] = '\r';
	line[c + 1] = '\n';
	line[c + 2] = 0;
	c += 2;

	/* send it to the player and flush */
	if (write(seatFD, line, c) != c) {
		/* couldn't send the line */

		fprintf(stderr, "ERROR: could not send state to seat %" PRIu8 "\n",
			player + 1);
		return -1;
	}

	/* note when we sent the message */
	gettimeofday(sendTime, NULL);

	/* log the message */
	if (!quiet) {
		fprintf(stderr, "TO %d at %zu.%.06zu %s", player + 1,
			sendTime->tv_sec, sendTime->tv_usec, line);
	}

	return 0;
}

/* returns >= 0 if action/size has been set to a valid action
returns -1 for failure (disconnect, timeout, too many bad actions, etc) */
static int readPlayerResponse(const MatchState *state,
	const BoardState *boardstate,
	const int quiet,
	const uint8_t player,
	const struct timeval *sendTime,
	ErrorInfo *errorInfo,
	ReadBuf *readBuf,
	Action *action,
	struct timeval *recvTime)
{
	int c, r;
	char line[MAX_LINE_LEN];

	while (1) {

		/* read a line of input from player */
		struct timeval start;
		gettimeofday(&start, NULL);
		if (getLine(readBuf, MAX_LINE_LEN, line,
			errorInfo->maxResponseMicros) <= 0) {
			/* couldn't get any input from player */

			struct timeval after;
			gettimeofday(&after, NULL);
			uint64_t micros_spent =
				(uint64_t)(after.tv_sec - start.tv_sec) * 1000000 + (after.tv_usec - start.tv_usec);
			fprintf(stderr, "ERROR: could not get action from player %" PRIu8 "\n",
				player + 1);
			// Print out how much time has passed so we can see if this was a
			// timeout as opposed to some other sort of failure (e.g., socket
			// closing).
			fprintf(stderr, "%.1f seconds spent waiting; timeout %.1f\n",
				micros_spent / 1000000.0,
				errorInfo->maxResponseMicros / 1000000.0);
			return -1;
		}

		/* note when the message arrived */
		gettimeofday(recvTime, NULL);

		/* log the response */
		if (!quiet) {
			fprintf(stderr, "FROM %d at %zu.%06zu %s", player + 1,
				recvTime->tv_sec, recvTime->tv_usec, line);
		}

		/* ignore comments */
		if (line[0] == '#' || line[0] == ';') {
			continue;
		}

		/* check for any timeout issues */
		if (checkErrorTimes(sendTime, recvTime, errorInfo) < 0) {

			fprintf(stderr, "ERROR: player %" PRIu8 " ran out of time\n", player + 1);
			return -1;
		}

		/* parse out the state */
		c = readMatchState(line, state);
		if (c < 0) {
			/* couldn't get an intelligible state */

			fprintf(stderr, "WARNING: bad state format in response\n");
			continue;
		}

		/* get the action */
		if (line[c++] != ':' || (r = readAction(&line[c], action)) < 0) {
			if (checkErrorInvalidAction(player, errorInfo) < 0) {
				fprintf(stderr, "ERROR: bad action format in response\n");
			}
			fprintf(stderr, "WARNING: bad action format in response, changed to i dont know what to do 2333\n");
			action->type = 0;
			goto doneRead;
		}
		c += r;
		/* make sure the action is valid */
		if (!isValidAction(boardstate, action)) {
			if (checkErrorInvalidAction(player, errorInfo) < 0) {
				fprintf(stderr, "ERROR: invalid action\n");
				return -1;
			}
		}
		goto doneRead;
	}

doneRead:
	return 0;
}

/* returns >= 0 if match should continue, -1 on failure */
static int checkVersion(const uint8_t player,
	ReadBuf *readBuf)
{
	uint32_t major, minor, rev;
	char line[MAX_LINE_LEN];

	if (getLine(readBuf, MAX_LINE_LEN, line, -1) <= 0) {

		fprintf(stderr,
			"ERROR: could not read version string from player %" PRIu8 "\n",
			player + 1);
		return -1;
	}

	if (sscanf(line, "VERSION:%" SCNu32 ".%" SCNu32 ".%" SCNu32,
		&major, &minor, &rev) < 3) {
		fprintf(stderr,
			"ERROR: invalid version string %s", line);
		return -1;
	}
	if (major != VERSION_MAJOR || minor > VERSION_MINOR) {
		fprintf(stderr, "ERROR: this server is currently using version %" SCNu32 ".%" SCNu32 ".%" SCNu32 "\n", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
	}
	return 0;
}

/* returns >= 0 if match should continue, -1 on failure */
static int addToLogFile(const MatchState *state,
	const BoardState *boardState,
	const int value[MAX_PLAYERS],
	char *seatName[MAX_PLAYERS], FILE *logFile)
{
	int c, r;
	uint8_t p;
	char line[MAX_LINE_LEN];

	/* prepare the message */
	c = printState(state, MAX_LINE_LEN, line);
	if (c < 0) {
		/* message is too long */

		fprintf(stderr, "ERROR: log state message too long\n");
		return -1;
	}

	/* add the values */
	for (p = 0; p < MAX_PLAYERS; ++p) {

		r = snprintf(&line[c], MAX_LINE_LEN - c,
			p ? "|%d" : ":%d", value[p]);
		if (r < 0) {
			fprintf(stderr, "ERROR: log message too long\n");
			return -1;
		}
		c += r;
		/* remove trailing zeros after decimal-point */
		while (line[c - 1] == '0') {
			--c;
		}
		if (line[c - 1] == '.') {
			--c;
		}
		line[c] = 0;
	}

	/* add the player names */
	for (p = 0; p < MAX_PLAYERS; ++p) {

		r = snprintf(&line[c], MAX_LINE_LEN - c,
			p ? "|%s" : ":%s",
			seatName[p]);
		if (r < 0) {

			fprintf(stderr, "ERROR: log message too long\n");
			return -1;
		}
		c += r;
	}

	/* print the line to log and flush */
	if (fprintf(logFile, "%s\n", line) < 0) {

		fprintf(stderr, "ERROR: logging failed for game %s\n", line);
		return -1;
	}
	fflush(logFile);

	return 0;
}

/* returns >= 0 if match should continue, -1 on failure */
static int printInitialMessage(const char *matchName,
	const uint32_t numRounds,
	const ErrorInfo *info, FILE *logFile)
{
	int c;
	char line[MAX_LINE_LEN];

	c = snprintf(line, MAX_LINE_LEN, "# name/Rounds %s Renju %" PRIu32 "\n#--t_response %" PRIu64 "\n",
		matchName, numRounds,
		info->maxResponseMicros / 1000);
	if (c < 0) {
		/* message is too long */

		fprintf(stderr, "ERROR: initial game comment too long\n");
		return -1;
	}

	fprintf(stderr, "%s", line);
	if (logFile) {

		fprintf(logFile, "%s", line);
	}

	return 0;
}

/* returns >= 0 if match should continue, -1 on failure */
static int printFinalMessage(char *seatName[MAX_PLAYERS],
	const int totalValue[MAX_PLAYERS],
	FILE *logFile)
{
	int c, r;
	uint8_t s;
	char line[MAX_LINE_LEN];

	c = snprintf(line, MAX_LINE_LEN, "SCORE");
	if (c < 0) {
		/* message is too long */

		fprintf(stderr, "ERROR: value state message too long\n");
		return -1;
	}

	for (s = 0; s < MAX_PLAYERS; ++s) {

		r = snprintf(&line[c], MAX_LINE_LEN - c,
			s ? "|%d" : ":%d", totalValue[s]);
		if (r < 0) {

			fprintf(stderr, "ERROR: value message too long\n");
			return -1;
		}
		c += r;

		/* remove trailing zeros after decimal-point */
		while (line[c - 1] == '0') {
			--c;
		}
		if (line[c - 1] == '.') {
			--c;
		}
		line[c] = 0;
	}

	/* add the player names */
	for (s = 0; s < MAX_PLAYERS; ++s) {

		r = snprintf(&line[c], MAX_LINE_LEN - c,
			s ? "|%s" : ":%s", seatName[s]);
		if (r < 0) {

			fprintf(stderr, "ERROR: log message too long\n");
			return -1;
		}
		c += r;
	}

	fprintf(stdout, "%s\n", line);
	fprintf(stderr, "%s\n", line);

	if (logFile) {

		fprintf(logFile, "%s\n", line);
	}

	return 0;
}

/* run a match of numHands hands of the supplied game

cards are dealt using rng, error conditions like timeouts
are controlled and stored in errorInfo

actions are read/sent to seat p on seatFD[ p ]

if quiet is not zero, only print out errors, warnings, and final value

if logFile is not NULL, print out a single line for each completed
match with the final state and all player values.  The values are
printed in player, not seat order.

returns >=0 if the match finished correctly, -1 on error */
static int gameLoop(char *seatName[MAX_PLAYERS],
	const uint32_t numHands, const int quiet,
	ErrorInfo *errorInfo, const int seatFD[MAX_PLAYERS],
	ReadBuf *readBuf[MAX_PLAYERS],
	FILE *logFile)
{
	uint8_t player, currentP;
	struct timeval t, sendTime, recvTime;
	Action action;
	MatchState state;
	BoardState boardState;
	int totalValue[MAX_PLAYERS];
	boardState.board = (uint8_t*)malloc(BOARD_SIZE*BOARD_SIZE*sizeof(uint8_t));
	/* check version string for each player */
	for (player = 0; player < MAX_PLAYERS; ++player) {
		if (checkVersion(player, readBuf[player]) < 0) {
			/* error messages already handled in function */
			return -1;
		}
	}

	gettimeofday(&sendTime, NULL);
	if (!quiet) {
		fprintf(stderr, "STARTED at %zu.%06zu\n",
			sendTime.tv_sec, sendTime.tv_usec);
	}

	/* start at the first game */

	initState(&state);
	initBoardState(&boardState);
	for (player = 0; player < MAX_PLAYERS; ++player) {
		totalValue[player] = 0;
	}

	if (state.numGames >= numHands) {
		goto finishedGameLoop;
	}

	/* play all the (remaining) hands */
	while (1) {

		/* play the hand */
		while (!stateFinished(&state)) {
			/* find the current player */
			currentP = currentPlayer(&state);
			/* send state to each player */
			for (player = 0; player < MAX_PLAYERS; ++player) {
				state.viewingPlayer = player + 1;
				if (sendPlayerMessage(&state, &boardState, quiet, player,
					seatFD[player], &t) < 0) {
					/* error messages already handled in function */
					return -1;
				}
				/* remember the seat and send time if player is acting */
				if (player + 1 == currentP) {
					sendTime = t;
				}
			}

			/* get action from current player */
			if (readPlayerResponse(&state, &boardState, quiet, currentP-1, &sendTime,
				errorInfo, readBuf[currentP-1],
				&action, &recvTime) < 0) {
				/* error messages already handled in function */
				return -1;
			}

			/* do the action */
			doAction(&action, &state, &boardState);
			if(state.finished==3)
				return -1;
		}

		/* add the game to the log */
		if (logFile != NULL) {
			if (addToLogFile(&state, &boardState, totalValue, seatName, logFile) < 0) {
				/* error messages already handled in function */
				return -1;
			}
		}

		/* send final state to each player */
		for (player = 0; player < MAX_PLAYERS; ++player) {
			state.viewingPlayer = player + 1;
			if (sendPlayerMessage(&state, &boardState, quiet, player,
				seatFD[player], &t) < 0) {
				/* error messages already handled in function */
				return -1;
			}
		}
		/* start a new hand */
		++totalValue[state.finished - 1];  // 记录胜利者
		for(int j;j<MAX_PLAYERS;++j){
			fprintf(stderr, "player:%d:%"SCNu8"\n",j,totalValue[j]);
		}
		resetState(&state); //局数加一在这里完成
		initBoardState(&boardState);
		if (state.numGames >= numHands) {
			break;
		}
	}
finishedGameLoop:
	/* print out the final values */
	if (printFinalMessage(seatName, totalValue, logFile) < 0) {
		/* error messages already handled in function */
		return -1;
	}
	free(boardState.board);
	return 0;
}

int main(int argc, char **argv)
{
	int i, listenSocket[MAX_PLAYERS], v, longOpt;
	int quiet, append;
	int seatFD[MAX_PLAYERS];
	FILE *logFile;
	ReadBuf *readBuf[MAX_PLAYERS];
	ErrorInfo errorInfo;
	struct sockaddr_in addr;
	socklen_t addrLen;
	char *seatName[MAX_PLAYERS];

	int useLogFile;
	uint64_t maxResponseMicros;
	int64_t startTimeoutMicros;
	uint32_t numHands, maxInvalidActions;
	uint16_t listenPort[MAX_PLAYERS];

	struct timeval startTime, tv;

	char name[MAX_LINE_LEN];
	static struct option longOptions[] = {
		{ "t_response", 1, 0, 0 },
	{ "t_hand", 1, 0, 0 },
	{ "t_per_hand", 1, 0, 0 },
	{ "start_timeout", 1, 0, 0 },
	{ 0, 0, 0, 0 } };

	/* set defaults */

	/* game error conditions */
	maxInvalidActions = DEFAULT_MAX_INVALID_ACTIONS;
	maxResponseMicros = DEFAULT_MAX_RESPONSE_MICROS;

	/* use random ports */
	for (i = 0; i < MAX_PLAYERS; ++i) {
		listenPort[i] = 0;
	}

	/* use log file, don't use transaction file */
	useLogFile = 1;

	/* print all messages */
	quiet = 0;

	/* by default, overwrite preexisting log/transaction files */
	append = 0;

	/* no timeout on startup */
	startTimeoutMicros = -1;

	/* parse options */
	while (1) {
		i = getopt_long(argc, argv, "flLp:qtTa", longOptions, &longOpt);
		if (i < 0)  //返回0解析完毕
		{
			break;
		}

		switch (i) {
			//具体参照上面的longoption结构体
		case 0:
			/* long option longOpt */
			switch (longOpt) {
			case 0:
				/* t_response */
				//optarg 是全局变量
				if (sscanf(optarg, "%" SCNu64, &maxResponseMicros) < 1) {
					fprintf(stderr, "ERROR: could not get response timeout from %s\n", optarg);
					exit(EXIT_FAILURE);
				}
				/* convert from milliseconds to microseconds */
				maxResponseMicros *= 1000;
				break;

			case 3:
				/* start_timeout */

				if (sscanf(optarg, "%" SCNd64, &startTimeoutMicros) < 1) {
					fprintf(stderr, "ERROR: could not get start timeout %s\n", optarg);
					exit(EXIT_FAILURE);
				}
				/* convert from milliseconds to microseconds */
				if (startTimeoutMicros > 0) {
					startTimeoutMicros *= 1000;
				}
				break;
			}
			break;

		case 'l':
			useLogFile = 0;
			break;

		case 'L':
			useLogFile = 1;
			break;

		case 'p':
			/* port specification */

			if (scanPortString(optarg, listenPort) < 0) {
				fprintf(stderr, "ERROR: bad port string %s\n", optarg);
				exit(EXIT_FAILURE);
			}

			break;

		case 'a':

			append = 1;
			break;

		default:

			fprintf(stderr, "ERROR: unknown option %c\n", i);
			exit(EXIT_FAILURE);
		}
	}

	if (optind + 2 > argc) {
		printUsage(stdout, 0);
		exit(EXIT_FAILURE);
	}

	/* save the seat names */
	if (optind + 2 + MAX_PLAYERS > argc) {
		printUsage(stdout, 0);
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < MAX_PLAYERS; ++i) {
		seatName[i] = argv[optind + 2 + i];
	}

	/* get number of hands */
	if (sscanf(argv[optind + 1], "%" SCNu32, &numHands) < 1 || numHands == 0) {
		fprintf(stderr, "ERROR: invalid number of hands %s\n",
			argv[optind + 1]);
		exit(EXIT_FAILURE);
	}

	if (useLogFile) {
		/* create/open the log */
		if (snprintf(name, MAX_LINE_LEN, "%s.log", argv[optind]) < 0) {

			fprintf(stderr, "ERROR: match file name too long %s\n", argv[optind]);
			exit(EXIT_FAILURE);
		}
		if (append) {
			logFile = fopen(name, "a+");
		} else {
			logFile = fopen(name, "w");
		}
		if (logFile == NULL) {
			fprintf(stderr, "ERROR: could not open log file %s\n", name);
			exit(EXIT_FAILURE);
		}
	} else {
		/* no log file */

		logFile = NULL;
	}

	/* set up the error info */
	initErrorInfo(maxInvalidActions, maxResponseMicros, &errorInfo);

	/* open sockets for players to connect to */
	for (i = 0; i < MAX_PLAYERS; ++i) {
		listenSocket[i] = getListenSocket(&listenPort[i]);
		if (listenSocket[i] < 0) {
			fprintf(stderr, "ERROR: could not create listen socket for player %d\n",
				i + 1);
			exit(EXIT_FAILURE);
		}
	}

	/* print out the final port assignments */
	for (i = 0; i < MAX_PLAYERS; ++i) {
		printf(i ? " %" PRIu16 : "%" PRIu16, listenPort[i]);
	}
	printf("\n");
	fflush(stdout);

	/* print out usage information */
	printInitialMessage(argv[optind], 
		numHands, &errorInfo, logFile);

	/* wait for each player to connect */
	gettimeofday(&startTime, NULL);
	for (i = 0; i < MAX_PLAYERS; ++i) {

		if (startTimeoutMicros >= 0) {
			uint64_t startTimeLeft;
			fd_set fds;

			gettimeofday(&tv, NULL);
			startTimeLeft = startTimeoutMicros - (uint64_t)(tv.tv_sec - startTime.tv_sec) * 1000000 - (tv.tv_usec - startTime.tv_usec);
			if (startTimeLeft < 0) {

				startTimeLeft = 0;
			}
			tv.tv_sec = startTimeLeft / 1000000;
			tv.tv_usec = startTimeLeft % 1000000;

			FD_ZERO(&fds);
			FD_SET(listenSocket[i], &fds);
			if (select(listenSocket[i] + 1, &fds, NULL, NULL, &tv) < 1) {
				/* no input ready within time, or an actual error */

				fprintf(stderr, "ERROR: timed out waiting for seat %d to connect\n",
					i + 1);
				exit(EXIT_FAILURE);
			}
		}

		addrLen = sizeof(addr);
		seatFD[i] = accept(listenSocket[i],
			(struct sockaddr *)&addr, &addrLen);
		if (seatFD[i] < 0) {

			fprintf(stderr, "ERROR: seat %d could not connect\n", i + 1);
			exit(EXIT_FAILURE);
		}
		close(listenSocket[i]);

		v = 1;
		setsockopt(seatFD[i], IPPROTO_TCP, TCP_NODELAY,
			(char *)&v, sizeof(int));

		readBuf[i] = createReadBuf(seatFD[i]);
	}
	/* play the match */
	if (gameLoop(seatName, numHands, quiet, &errorInfo,
		seatFD, readBuf, logFile) < 0) {
		/* should have already printed an error message */
		exit(EXIT_FAILURE);
	}

	fflush(stderr);
	fflush(stdout);
	if (logFile != NULL) {
		fclose(logFile);
	}

	return EXIT_SUCCESS;
}
