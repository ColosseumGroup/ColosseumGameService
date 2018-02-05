/*
Copyright (C) 2011 by the Computer Poker Research Group, University of Alberta
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "renju.h"

void initBoardState(BoardState *boardState)
{
	boardState->lcol=0;
	boardState->lrow=0;
	boardState->ltype=0;
	for(int i=0;i<BOARD_SIZE*BOARD_SIZE;++i)
		boardState->board[i] = 0;
}


uint8_t getPiece(const BoardState* bs, const uint8_t col,const uint8_t row) {
	if((col<0 || col>BOARD_SIZE || row<0 || row>BOARD_SIZE))
		return 3;  //应该没有“3”状态，故为错误
	return bs->board[col*BOARD_SIZE + row];
}

uint8_t addPiece(BoardState*bs, const uint8_t col,const uint8_t row,const uint8_t type) {
	if ((col<0 || col>BOARD_SIZE || row<0 || row>BOARD_SIZE))
		return 0;
	if (!getPiece(bs, col, row)) {
		bs->board[col*BOARD_SIZE + row] = type;
		bs->lrow = row;
		bs->lcol = col;
		bs->ltype = type;
		return 1;
	} else {
		return 0;
	}
}

void clearBoard(BoardState*bs) {
	memset(bs->board, 0, (BOARD_SIZE*BOARD_SIZE * sizeof(uint8_t)));
}

void initState( MatchState *state )
{
	state->numGames = 0;
  state->numActions = 0;
  state->numRounds = 1;
  state->firstPlayer = 1;
  state->currentPlayer = state->firstPlayer;
	state->finished = 0;
	state->viewingPlayer = 1;
}
void resetState( MatchState *state )
{
	state->finished = 0;
  state->numActions = 0;
  state->numRounds = 1;
  state->firstPlayer = (state->firstPlayer==1)?2:1;
  state->currentPlayer = state->firstPlayer;
	state->viewingPlayer = state->firstPlayer;
  ++state->numGames;
}

int checkLine(const BoardState *boardState, const Action *action, const uint8_t secondCol, const uint8_t secondRow)
{
  if(action->row==secondRow&&action->col==secondCol)
    return 1;
	/* with origin piece valid, 
	when accessing the invalided target which has the output of 3,
	, the return of the following expression will be false */
  if(getPiece(boardState,action->col,action->row)!=getPiece(boardState, secondCol,secondCol))
    return 0;

  uint8_t next_row,next_col;
  if(secondRow<action->row)
    next_row = secondRow + 1;
  else if(secondRow>action->row)
      next_row = secondRow -1;
    else
      next_row = secondRow;
  
  if(secondCol<action->col)
    next_col = secondCol + 1;
    else if(secondCol>action->col)
      next_col = secondCol - 1;
    else
      next_col = secondCol;
  
  return checkLine(boardState,action,next_col,next_row);
}

int checkWinningPiece(const BoardState *boardState, const Action *action){
  //检查边界五点
  return checkLine(boardState,action,action->col-4,action->row)
  ||checkLine(boardState,action,action->col-4,action->row-4)
  ||checkLine(boardState,action,action->col,action->row-4)
  ||checkLine(boardState,action,action->col+4,action->row)
  ||checkLine(boardState,action,action->col+4,action->row+4)
  ||checkLine(boardState,action,action->col,action->row+4)
  ||checkLine(boardState,action,action->col+4,action->row-4)
  ||checkLine(boardState,action,action->col-4,action->row+4);
}

uint8_t currentPlayer( const MatchState *state )
{
  return state->currentPlayer;
}

uint8_t numRounds( const MatchState *state )
{
  return state->numRounds;
}

uint8_t numAction( const MatchState *state )
{
  return state->numActions;
}

int isValidAction( const BoardState *boardState,const Action *action )
{
  //only when a piece is 0 will the location be valid
  if(getPiece(boardState,action->col,action->row)==0){
    return 1;
  }else{
    return 0;
  }
}

void doAction( Action *action, MatchState *state ,BoardState* boardState )
{
  if(action->type!=3){   //if not give up
    ++state->numActions;
    if(state->currentPlayer==state->firstPlayer){
      action->type = 1;
      ++state->numRounds;
    }else{
      action->type = 2;
    }
    addPiece(boardState,action->col,action->row,action->type);
    state->currentPlayer = (state->currentPlayer==1)?2:1;
	/*做完动作检查是否已经胜利*/
	state->finished = isWin(boardState, action->type);
  }else{
	  state->finished = (state->currentPlayer==1)?2:1;  //opponent win
  }
}

int isWin(const BoardState *boardState, const uint8_t type)
{
	Action act;
	act.type = type;
	for (int i=0; i < BOARD_SIZE; i++) {
		for (int j=0; j < BOARD_SIZE; j++) {
			if (getPiece(boardState,i,j)==type) {
				act.row = j;
				act.col = i;
				if (checkWinningPiece(boardState, &act))
					return type;   //确认胜利，返回胜利的类型
			}
		}
	}
  return 0;
}

int printMatchCommonState( const MatchState *state,
		     const int maxLen, char *string )
{
	int c,r;
	c=0;
	/* General State: MATCHSTATE:viewingplayer:currentplayer:currentGames:currentRounds:finishedFlag */
	/* HEADER = MATCHSTATE:viewingplayer */
	r = snprintf( string, maxLen - c, "MATCHSTATE:%"SCNu8, state->viewingPlayer );
  if( r < 0 ) {
    return -1;
  }
  c += r;
	
  /*:currentplayer*/
	r = snprintf( string+c, maxLen - c, ":%"SCNu8, state->currentPlayer );
  if( r < 0 ) {
    return -1;
  }
  c += r;
	
	/*:games*/
	r = snprintf( string+c, maxLen - c, ":%"SCNu8, state->numGames );
  if( r < 0 ) {
    return -1;
  }
  c += r;
	/*:rounds*/
	r = snprintf( string+c, maxLen - c, ":%"SCNu8, state->numRounds );
  if( r < 0 ) {
    return -1;
  }
  c += r;
	/*:finishedflag*/
	r = snprintf( string+c, maxLen - c, ":%"SCNu8, state->finished );
  if( r < 0 ) {
    return -1;
  }
  c += r;
	if( c >= maxLen ) {
    return -1;
  }
  string[ c ] = 0;
  return c;
}

int printMatchState( const MatchState *state, const BoardState *boardState,
		     const int maxLen, char *string )
{
	int c,t;
  c=0;
  t = printMatchCommonState(state,maxLen,string);
  c += t;
	if(state->numActions!=0){
		Action act;
		act.row = boardState->lrow;
		act.col = boardState->lcol;
		act.type = boardState->ltype;
		t = printAction(&act, maxLen - c, string + c);
		c += t;
	}else{
		string[c] = ':';
		++c;
	}
	return c;
}

int readMatchState( const char *string,const MatchState *state)
{
	int c=0, t;
	uint8_t tempNum;
/* General State: MATCHSTATE:viewingplayer:currentplayer
:currentGames:currentRounds:finishedFlag */
/*viewingplayer*/
	if (sscanf(string , "MATCHSTATE:%"SCNu8"%n", &tempNum, &t) < 1) {
		return -1;
	}
 //viewing player bu zuo pan duan
	c += t;

	/* currentplayer */
	if (sscanf(string+c, ":%"SCNu8"%n", &tempNum, &t) < 1) {
		return -1;
	}
	if(tempNum!=state->currentPlayer){
		return -1;
	}
	c += t;

		/* currentGames */
	if (sscanf(string+c, ":%"SCNu8"%n", &tempNum, &t) < 1) {
		return -1;
	}
	if(tempNum!=state->numGames){
		return -1;
	}
	c += t;

	/* currentRounds */
	if (sscanf(string+c, ":%"SCNu8"%n", &tempNum, &t) < 1) {
		return -1;
	}
	if(tempNum!=state->numRounds){
		return -1;
	}
	c += t;

	/* finished flag */
	if (sscanf(string+c, ":%"SCNu8"%n", &tempNum, &t) < 1) {
		return -1;
	}
	if(tempNum!=state->finished){
		return -1;
	}
	c += t;

	return c;
}

int readAction( const char *string, Action *action )
{
	int c=0, t;
	uint8_t tempNum;
	/* General Action: col/row */
	/*col*/
	if (sscanf(string , "%"SCNu8"%n", &tempNum, &t) < 1) {
		return -1;
	}

	action->col = tempNum;
	c += t;

	/* row */
	if (sscanf(string+c, "/%"SCNu8"%n", &tempNum, &t) < 1) {
		return -1;
	}
	action->row = tempNum;
	c += t;
	return c;
}
int printState( const MatchState *state,const int maxLen, char *string )
{
int c,r;
	c=0;
	/* General State: MATCHSTATE:currentGames:totalRounds:finishedFlag */
	/* HEADER = MATCH:currentGames */
	r = snprintf( string, maxLen - c, "MATCH:%"SCNu8, state->numGames );
  if( r < 0 ) {
    return -1;
  }
  c += r;
	
  /*:totalRounds*/
	r = snprintf( string+c, maxLen - c, ":%"SCNu8, state->numRounds );
  if( r < 0 ) {
    return -1;
  }
  c += r;
	
	/*:finishedFlag*/
	r = snprintf( string+c, maxLen - c, ":%"SCNu8, state->finished );
  if( r < 0 ) {
    return -1;
  }
  c += r;
	if( c >= maxLen ) {
    return -1;
  }
  string[ c ] = 0;
  return c;
}
int printAction( const Action *action,
		 const int maxLen, char *string )
{
	int c, r;
	c = 0;
	/* :Action */
	/* Action = col/row/type */
	r = snprintf(&string[c], maxLen - c, ":%"PRIu8,
		action->col);
	if (r < 0) {
		return -1;
	}
	c += r;
	r = snprintf(&string[c], maxLen - c, "/%"PRIu8,
		action->row);
	if (r < 0) {
		return -1;
	}
	c += r;
	r = snprintf(&string[c], maxLen - c, "/%"PRIu8,
		action->type);
	if (r < 0) {
		return -1;
	}
	c += r;
	if (c >= maxLen) {
		return -1;
	}
	string[c] = 0;
	return c;
}