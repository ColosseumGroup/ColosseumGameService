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
#include "rng.h"


void initState( MatchState *state )
{
  state->numActions = 0;
  state->numRounds = 1;
  state->firstPlayer = 1;
  state->currentPlayer = state->firstPlayer;
}
void resetState( MatchState *state )
{
  state->boardState->clearBoard();
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
  if(cor.row==secondRow&&cor.col==secondCol)
    return 1;
  if(state->boardState->getPiece(cor)!=state->boardState->getPiece(secondCol,secondRow))
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

int checkWinningPiece(const MatchState *state, const std::unordered_map<Cordinate,uint8_t>::iterator piece){
  Cordinate cor = piece->first;
  uint8_t type = piece->second;
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
  if(!action->cor.isValidCordinate())
    return 0;
  //找不到点就是合法的
  if(!curState->boardState->getPiece(action->cor,action->type)){
    return 1;
  }else{
    return 0;
  }
}


void doAction( const Action *action, MatchState *state )
{
  if(isValidAction(state,action)){
    state->boardState->addPiece(action->cor,action->type);
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
  for(auto p : state->boardState->board){
    //只检查刚刚下子的点
    if(checkWinningPiece(state, p)&&p->second==type)
      return 1;
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
