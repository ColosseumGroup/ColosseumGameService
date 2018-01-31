/*
Copyright (C) 2011 by the Computer Poker Research Group, University of Alberta
*/

#ifndef _RENJU_H
#define _RENJU_H
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "rng.h"
#include "net.h"
#include <unordered_map>

#define VERSION_MAJOR 2
#define VERSION_MINOR 0
#define VERSION_REVISION 0


#define MAX_PLAYERS 2
#define MAX_BOARD_CARDS 7
#define MAX_LINE_LEN READBUF_LEN

#define NUM_ACTION_TYPES 3
#define BOARD_SIZE 15

typedef struct {
  uint8_t col;
  uint8_t row;
  int isValidCordinate(){
    return !(col<0||col>BOARD_SIZE||row<0||row>BOARD_SIZE);
  };
}Cordinate;


typedef struct {
  uint8_t type;
  Cordinate cor;
} Action;


typedef struct {
  //棋局应该比较稀疏
  std::unordered_map<Cordinate,uint8_t> board;
  
  uint8_t getPiece(Cordinate cor){
    if(board.find(cor)==board.end()){
      return 0;
    }else{
      return board[cor];
    }
    return getPiece(cor.col,cor.row);
  };
  uint8_t getPiece(uint8_t col,uint8_t row){
    Cordinate temp;
    temp.col = col;
    temp.row = row;
    return getPiece(temp);
  };
  uint8_t addPiece(Cordinate cor,uint8_t type){
    if(!cor.isValidCordinate())
      return 0;
    if(!getPiece(cor)){
      board[cor] = type;
      return 1;
    }else{
      return 0;
    }
  }
  void clearBoard(){
    board.clear();
  }
}BoardState;


typedef struct {
  //保存棋局信息
  BoardState *boardState;
  //保存动作次数
  uint8_t numActions;
  //记录回合次数
  uint8_t numRounds;
  //当前玩家
  uint8_t currentPlayer;
  //先手玩家
  uint8_t firstPlayer;
  //结束flag
  uint8_t finished;
} MatchState;

/*初始化棋局状态，为下一局做准备*/
void initState( MatchState *state );


int matchStatesEqual( const MatchState *a, const MatchState *b );

/* 检查是不是把棋子下在了已有棋子上 */
int isValidAction( const MatchState *curState, Action *action );

void doAction( const Action *action, MatchState *state );

/* 返回0为无玩家胜利，返回1为执黑玩家胜，返回2为执白玩家胜*/
int isWin(const MatchState *state);

/* 检查该是否能取胜 */
int checkWinningPiece(const MatchState *state, const std::unordered_map<Cordinate,uint8_t>::iterator piece);

/* 检查某一条线是否为五连 */
int checkLine(const MatchState *state, const Cordinate cor, const uint8_t secondCol, const uint8_t secondRow);

/* returns non-zero if hand is finished, zero otherwise */
#define stateFinished( constStatePtr ) ((constStatePtr)->finished)

/* get the current player to act in the state */
uint8_t currentPlayer( const MatchState *state );

/* the current round */
uint8_t numRounds( const MatchState *state );



/* number of actions performed */
uint8_t numAction( const MatchState *state );

/* returns number of characters consumed on success, -1 on failure
   state will be modified even on a failure to read */
int readMatchState( const char *string, MatchState *state );

/* print a state to a string, as viewed by viewingPlayer
   returns the number of characters in string, or -1 on error
   DOES NOT COUNT FINAL 0 TERMINATOR IN THIS COUNT!!! */
int printState( const MatchState *state,
		const int maxLen, char *string );

/* print a state to a string, as viewed by viewingPlayer
   returns the number of characters in string, or -1 on error
   DOES NOT COUNT FINAL 0 TERMINATOR IN THIS COUNT!!! */
int printMatchState( const MatchState *state,
		     const int maxLen, char *string );

/* read an action, returning the action in the passed pointer
   action and size will be modified even on a failure to read
   returns number of characters consumed on succes, -1 on failure */
int readAction( const char *string, Action *action );

/* print an action to a string
   returns the number of characters in string, or -1 on error
   DOES NOT COUNT FINAL 0 TERMINATOR IN THIS COUNT!!! */
int printAction( const Action *action,
		 const int maxLen, char *string );

int printBoards( const MatchState *state, const int maxLen, char *string);
#endif
