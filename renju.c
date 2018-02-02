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


void initState( MatchState *state )
{
  state->numActions = 0;
  state->numRounds = 1;
  state->firstPlayer = 1;
  state->currentPlayer = state->firstPlayer;
}
void resetState( MatchState *state )
{
  clearBoard(state->boardState);
  state->numActions = 0;
  state->numRounds = 1;
  state->firstPlayer = (state->firstPlayer==1)?2:1;
  state->currentPlayer = state->firstPlayer;
}
static uint8_t nextPlayer( const MatchState *state)
{
  return state->currentPlayer%MAX_PLAYERS + 1;
}

int checkLine(const MatchState *state, const Cordinate cor, const uint8_t secondCol, const uint8_t secondRow)
{
	Cordinate tempCor;
	tempCor.col = secondCol; tempCor.row = secondRow;
  if(cor.row==secondRow&&cor.col==secondCol)
    return 1;
  if(getPiece(state->boardState,cor)!=getPiece(state->boardState, tempCor))
    return 0;
  uint8_t next_row,next_col;
  if(secondRow<cor.row)
    next_row = secondRow + 1;
  else if(secondRow>cor.row)
      next_row = secondRow -1;
    else
      next_row = secondRow;
  
  if(secondCol<cor.col)
    next_col = secondCol + 1;
    else if(secondCol>cor.col)
      next_col = secondCol - 1;
    else
      next_col = secondCol;
  
  return checkLine(state,cor,next_col,next_row);
}

int checkWinningPiece(const MatchState *state, const Cordinate cor, const uint8_t type){
  //检查边界五点
  return checkLine(state,cor,cor.col-5,cor.row)
  ||checkLine(state,cor,cor.col-5,cor.row-5)
  ||checkLine(state,cor,cor.col,cor.row-5)
  ||checkLine(state,cor,cor.col+5,cor.row)
  ||checkLine(state,cor,cor.col+5,cor.row+5)
  ||checkLine(state,cor,cor.col,cor.row+5)
  ||checkLine(state,cor,cor.col+5,cor.row-5)
  ||checkLine(state,cor,cor.col-5,cor.row+5);
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

int isValidAction( const MatchState *curState,const Action *action )
{
  if(!isValidCordinate(action->cor))
    return 0;
  //找不到点就是合法的
  if(getPiece(curState->boardState,action->cor)!=action->type){
    return 1;
  }else{
    return 0;
  }
}


void doAction( const Action *action, MatchState *state )
{
  if(isValidAction(state,action)){
    addPiece(state->boardState,action->cor,action->type);
    ++state->numActions;
    if(state->currentPlayer==state->firstPlayer)
      ++state->numRounds;
	/*做完动作检查是否已经胜利*/
	state->finished = isWin(state, action->type);
  }else{
	  state->finished = -1;
  }
}
int isWin(const MatchState *state, const uint8_t type)
{
	Cordinate temp;
	for (; temp.col < BOARD_SIZE; temp.col++) {
		for (; temp.row < BOARD_SIZE; temp.row++) {
			if (getPiece(state->boardState,temp)==type) {
				if (checkWinningPiece(state, temp, type))
					return type;   //确认胜利，返回胜利的类型
			}
		}
	}
  return 0;
}

int readMatchState( const char *string,
		    MatchState *state )
{
	uint8_t tempNum;
	int c,t;
	/* General State: MATCHSTATE:currentplayer:currentRounds */
	/* HEADER = MATCHSTATE:player */
	if (sscanf(string, "MATCHSTATE:%"SCNu8"%n",
		&tempNum, &c) < 1
		|| state->currentPlayer != tempNum) {
		return -1;
	}
	/*:rounds*/
	if (sscanf(string + c, ":%"SCNu8"%n", &tempNum, &t) < 1
		|| state->numRounds != tempNum) {
		return -1;
	}
	c += t;
	
	return c;
}
int printState(const MatchState *state,
	const int maxLen, char *string) {
	int c, r;

	c = 0;
	/* MATCHSTATE:player */
	r = snprintf(&string[c], maxLen - c, "MATCHSTATE:%"PRIu8,
		state->currentPlayer);
	if (r < 0) {
		return -1;
	}
	c += r;
	/* :numRounds */
	r = snprintf(&string[c], maxLen - c, ":%"PRIu8,
		state->numRounds);
	if (r < 0) {
		return -1;
	}
	c += r;
	/* :finishedFlag */
	r = snprintf(&string[c], maxLen - c, ":%"PRIu8,
		state->finished);
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

int printMatchState( const MatchState *state,
		     const int maxLen, char *string )
{
	int c, r;

	c = 0;
	/* MATCHSTATE:player */
	r = snprintf(&string[c], maxLen - c, "MATCHSTATE:%"PRIu8,
		state->currentPlayer);
	if (r < 0) {
		return -1;
	}
	c += r;
	/* :numRounds */
	r = snprintf(&string[c], maxLen - c, ":%"PRIu8,
		state->numRounds);
	if (r < 0) {
		return -1;
	}
	c += r;
	/* :finishedFlag */
	r = snprintf(&string[c], maxLen - c, ":%"PRIu8,
		state->finished);
	if (r < 0) {
		return -1;
	}
	c += r;
	r = printAction(state->boardState->lastAction, maxLen - c, string + c);
	c += r;
	if (c >= maxLen) {
		return -1;
	}
	string[c] = 0;
	return c;
}

int readAction( const char *string, Action *action )
{
	int c=0, t;
	uint8_t tempNum;
	/* General Action: col/row/type */
	/*:col*/
	if (sscanf(string , ":%"SCNu8"%n", &tempNum, &t) < 1) {
		return -1;
	}
	action->cor.col = tempNum;
	c += t;

	/* row */
	if (sscanf(string+c, "/%"SCNu8"%n", &tempNum, &t) < 1) {
		return -1;
	}
	action->cor.row = tempNum;
	c += t;

	/* type */
	if (sscanf(string + c, "/%"SCNu8"%n", &tempNum, &t) < 1) {
		return -1;
	}
	action->type = tempNum;
	c += t;
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
		action->cor.col);
	if (r < 0) {
		return -1;
	}
	c += r;
	r = snprintf(&string[c], maxLen - c, "/%"PRIu8,
		action->cor.row);
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
