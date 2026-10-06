//==============================================================================================
//【文件说明】lua_using_cpp/main.cpp —— "Lua 脚本反过来调用 C++ 函数"
//
//【这个小演示干什么?】
//  与上一个相反:游戏流程写在 Lua 脚本里,C++ 只提供两个函数(电脑出拳、判胜负),
//  用 lua_register 把它们以名字 cpp_GetAIMove / cpp_EvaluateTheGuesses 登记进 Lua,
//  脚本里就能直接按名字调用它们。
//
//【文件地图】main.cpp + RockPaperScissors.h(C++ 逻辑与 Lua 包装函数)+ LuaHelperFunctions.h;
//  第三方库 Common\lua-5.1.5。脚本 Rock_Paper_Scissors_Using_C++_Funcs.lua 与 exe 同目录。
//【调用流程】main → lua_open/打开库 → lua_register 登记两个 C++ 函数 → RunLuaScript 跑脚本
//           (脚本内部会回调上面注册的函数)→ lua_close。
//==============================================================================================
extern "C"
{
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

#pragma comment(lib, "lua5.1.lib")
//#pragma comment(lib, "lualib.lib")

#include <iostream>
#include <string>
using namespace std;

#include "LuaHelperFunctions.h"
#include "RockPaperScissors.h"




// lua_register(状态, "脚本里看到的名字", C++函数指针):把 C++ 函数交给 Lua 按名调用。
int main()
{
  //create a lua state
  lua_State* pL = lua_open();

  //open the lua libaries - new in lua5.1
  luaL_openlibs(pL);

  //register the functions with lua
  lua_register(pL, "cpp_GetAIMove", cpp_GetAIMove);
  lua_register(pL, "cpp_EvaluateTheGuesses", cpp_EvaluateTheGuesses);

  //run the script
  RunLuaScript(pL, "Rock_Paper_Scissors_Using_C++_Funcs.lua");
  
  //tidy up
  lua_close(pL);

  cout << "\n\n\n";
    
  return 0;
}