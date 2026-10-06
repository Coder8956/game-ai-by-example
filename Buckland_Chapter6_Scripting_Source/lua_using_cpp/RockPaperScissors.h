//==============================================================================================
//【文件说明】RockPaperScissors.h —— "Lua 调用 C++ 函数"演示:石头剪刀布的 C++ 逻辑
//
//【这个文件是干什么的?】
//  游戏流程写在 Lua 脚本里,但"电脑随机出拳"和"判胜负打分"这两块用 C++ 实现。
//  本文件把它们成对给出:一个普通 C++ 函数(GetAIMove/EvaluateTheGuesses),
//  再加一个"包装函数"(cpp_ 开头)——包装函数负责从 Lua 栈取参数、调原函数、
//  把返回值压回栈,这样 Lua 脚本就能通过 lua_register 调用到 C++。
//
//【谁在使用这个文件?】lua_using_cpp/main.cpp —— #include 它,并把 cpp_ 开头的两个
//      包装函数注册进 Lua。
//【第三方库说明】lua.h 等来自 Common\lua-5.1.5(Lua 官方发行版),本工程用它做 C++/Lua 互调;
//      OpenLuaStates.h、misc/utils.h 是本书公共工具(RandInt 随机整数)。这些库内部不展开注释。
//==============================================================================================
#ifndef ROCK_PAPER_SCISSORS_H
#define ROCK_PAPER_SCISSORS_H
#pragma warning (disable:4786)

#include "OpenLuaStates.h"
#include "misc/utils.h"
#include <string>
#include <iostream>


const int         NumPlayStrings = 3;
const std::string PossiblePlayStrings[NumPlayStrings] = {"scissors", "rock", "paper"};
// NumPlayStrings=3 种出拳;PossiblePlayStrings 数组列出三种出拳的英文名字。

//-------------------------- GetAIMove ----------------------------------------
//
//  this is GetAIMove as you would normally use it in C++
//-----------------------------------------------------------------------------
//---- GetAIMove:普通 C++ 版"电脑出拳"——从三种里随机挑一个(RandInt(0,2))----
std::string GetAIMove()
{
  return PossiblePlayStrings[RandInt(0,2)];
}

//----------------------- cpp_GetAIMove ---------------------------------------
//
//  this is the wrapper written for GetAIMove to expose the function to lua
//-----------------------------------------------------------------------------
//---- cpp_GetAIMove:给 Lua 调用的包装版 ----
// Lua 通过"栈(stack)"与 C++ 传参:lua_gettop 数栈上有几个参数;
// c_str() 把 std::string 转成 C 风格字符串再压栈;return 1 表示压回 1 个返回值。
int cpp_GetAIMove(lua_State* pL)
{
  //get the number of parameters passed to this function from the lua
  //stack and make sure it is equal to the correct number of parameters
  //for GetAIMove
  int n = lua_gettop(pL);

  if (n!=0)
  {
    std::cout << "\n[C++]: Wrong number of arguments for cpp_GetAIMove";
    
    return 0;
  }

  //push the result from GetAIMove on the stack. Notice how the std::string 
  //is converted to a C type string first
  lua_pushstring(pL, GetAIMove().c_str());

  //return the number of return values
  return 1;
}

//------------------------ GuessToIndex ---------------------------------------
//
//  given a play string, this function returns its key in PossiblePlayStrings
//-----------------------------------------------------------------------------
// GuessToIndex:把出拳名字(rock/paper/scissors)转成数组下标 0/1/2,查不到返回 -1。
int  GuessToIndex(const std::string& guess)
{
  for (int i=0; i<NumPlayStrings; ++i)
  {
    if (guess == PossiblePlayStrings[i]) return i;
  }

  //this value will force an error
  return -1;
}

 
//------------------------- EvaluateTheGuesses --------------------------------
//
//  Given the computer's play string and the users play string, and references
//  to the scores, this function decides who has won the round and assigns
//  points accordingly
//----------------------------------------------------------------------------- 
//---- EvaluateTheGuesses:判一轮胜负 ----
// score_table 是 3x3 结果表:行=玩家、列=电脑;1=玩家赢,-1=电脑赢,0=平。
// user_score/comp_score 用 & 引用传递,函数内直接累加(出函数仍生效)。
void EvaluateTheGuesses(std::string user_guess,
                        std::string  comp_guess,
                        int&   user_score,
                        int&   comp_score)
{

  static const int score_table[NumPlayStrings][NumPlayStrings] = 
  { 
    {0,-1,1},
    {1,0,-1},
    {-1,1,0}
  };

  std::cout << "\nuser guess..." + user_guess + "  comp guess..." + comp_guess;
  
  if (score_table[GuessToIndex(user_guess)][GuessToIndex(comp_guess)] == 1)
  {
    std::cout << "\nYou have won this round!";

    ++user_score;
  }
  else if (score_table[GuessToIndex(user_guess)][GuessToIndex(comp_guess)] == -1)
  {
    std::cout << "\nComputer wins this round.";

    ++comp_score;
  }

  else
  {
    std::cout << "\nIt's a draw!";
  }
}


//------------------------------ cpp_EvaluateTheGuesses -----------------------
//
//  the wrapper for EvaluateTheGuesses
//-----------------------------------------------------------------------------
//---- cpp_EvaluateTheGuesses:上面判胜负函数的 Lua 包装版 ----
// 从栈上依次取 4 个参数(玩家出拳、电脑出拳、当前双方分),调原函数后,
// 再把更新后的两个分数压回栈,return 2 表示两个返回值。
int cpp_EvaluateTheGuesses(lua_State* pL)
{
  //get the number of parameters passed to this function from the lua
  //stack and make sure it is equal to the correct number of parameters
  //for EvaluateTheGuesses.
  int n = lua_gettop(pL);

  if (n!=4)
  {
    std::cout << "\n[C++]: Wrong number of arguments for cpp_EvaluateTheGuesses";
    
    return 0;
  }

   //check that the parameters are of the correct type. 
  if (!lua_isstring(pL, 1) || !lua_isstring(pL, 2) ||
      !lua_isnumber(pL, 3) || !lua_isnumber(pL, 4))
  {
    std::cout << "\n[C++]: ERROR: Invalid types passed to cpp_EvaluateTheGuesses";
  }

  //grab the parameters off the stack
  std::string user_guess = lua_tostring(pL, 1); 
  std::string comp_guess = lua_tostring(pL, 2); 
  int         user_score = (int)lua_tonumber(pL, 3); 
  int         comp_score = (int)lua_tonumber(pL, 4); 

  //call the C++ function proper
  EvaluateTheGuesses(user_guess, comp_guess, user_score, comp_score);

  //now push the updated scores onto the stack
  lua_pushnumber(pL, user_score);
  lua_pushnumber(pL, comp_score);

  //return the number of values pushed onto the stack
  return 2;
}


#endif