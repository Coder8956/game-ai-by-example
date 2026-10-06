//==============================================================================================
//【文件说明】ExposingCPPFunctionsToLua/main.cpp —— 用 luabind 把 C++ 函数暴露给 Lua
//
//【这个小演示干什么?】
//  前面 lua_register 方式要手写"栈进出"包装函数,很繁琐。luabind 是一个绑定库,
//  用 module(状态)[def("名字", &函数)...] 一行就把 C++ 函数 HelloWorld、add 交给 Lua 调用,
//  参数/返回值自动转换,不用手撸栈。
//
//【文件地图】本目录只有 main.cpp;luabind 来自第三方库 Common\luabind(依赖 Boost),
//  Lua 来自 Common\lua-5.1.5。脚本 ExposingCPPFunctionsToLua.lua 与 exe 同目录。
//【调用流程】main → lua_open/打开库/luabind::open → module[def 两个函数] →
//           RunLuaScript 跑脚本(Lua 里直接调 HelloWorld、add)→ lua_close。
//==============================================================================================
//(原文注释:包含库)#pragma comment(lib,...)让链接器自动链接 lua/luabind 的 .lib。
//include the libraries
#pragma comment(lib, "lua5.1.lib")
#pragma comment(lib, "luabind.lib")
//#pragma comment(lib, "lua.lib")
//#pragma comment(lib, "lualib.lib")

//turn off the inevitable warnings
#pragma warning (disable : 4786)

extern "C"
{
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

#include <iostream>
using namespace std;

//include the luabind headers. Make sure you have the paths set correctly
//to the lua, luabind and Boost files.
#include <luabind/luabind.hpp>
using namespace luabind;

//include the helper functions
#include "LuaHelperFunctions.h"


//define a couple of simple functions
// ---- 两个普通 C++ 自由函数,下面用 luabind 暴露给 Lua ----
void HelloWorld()
{
  cout << "\n[C++]: Hello World!" << endl;
}

int add(int a, int b)
{
  return a + b;
}



int main()
{
  //create a lua state
  lua_State* pLua = lua_open();

  //open the lua libaries - new in lua5.1
  luaL_openlibs(pLua);

  //open luabind
  open(pLua);

// module(pLua)[ def("HelloWorld",&HelloWorld), def("add",&add) ]:
// luabind 语法:方括号里列出要暴露的函数,Lua 脚本就能用同名调用了。&函数名 = 取地址。
  module(pLua)
  [
	  def("HelloWorld", &HelloWorld),
    def("add", &add)
  ];
 
  //load and run the script
  RunLuaScript(pLua, "ExposingCPPFunctionsToLua.lua");

  
  //tidy up
  lua_close(pLua);

    
  return 0;
}