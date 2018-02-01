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

int matchStatesEqual( const MatchState *a, const MatchState *b )
{
  return 0;
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
  }else{
    //重发信息
    //还是乱下一个？
  }
}
int isWin(const MatchState *state, const uint8_t type)
{
	Cordinate temp;
	for (; temp.col < BOARD_SIZE; temp.col++) {
		for (; temp.row < BOARD_SIZE; temp.row++) {
			if (getPiece(state->boardState,temp)==type) {
				if (checkWinningPiece(state, temp, type))
					return 1;
			}
		}
	}
  return 0;
}

int readMatchState( const char *string,
		    MatchState *state )
{
  
}


int printState( const MatchState *state,
		const int maxLen, char *string )
{
  
}

int printMatchState( const MatchState *state,
		     const int maxLen, char *string )
{
  
}

int readAction( const char *string, Action *action )
{

}

int printAction( const Action *action,
		 const int maxLen, char *string )
{
  
}


int printBoards( const MatchState *states,
		const int maxLen, char *string )
{

}
