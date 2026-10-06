//==============================================================================================
//【文件说明】CreatingClassesUsingLuabind/main.cpp —— luabind 最小骨架(创建/使用类)
//
//【这个小演示干什么?】
//  它只搭好 luabind 的架子:开 Lua、开 luabind、跑脚本 classes_in_lua.lua。
//  类的定义与用法全写在那个 Lua 脚本里(脚本里用 luabind 风格创建类),
//  C++ 这边不写任何类注册代码,是"纯 Lua 侧"的对照示例。
//
//【文件地图】本目录只有 main.cpp;luabind 来自第三方库 Common\luabind,Lua 来自 Common\lua-5.1.5。
//【调用流程】main → lua_open → luaL_openlibs → luabind::open → RunLuaScript → lua_close。
//==============================================================================================
//include the libraries
#pragma comment(lib, "lua5.1.lib")
#pragma comment(lib, "luabind.lib")
//#pragma comment(lib, "lua.lib")
//#pragma comment(lib, "lualib.lib")
#pragma warning (disable : 4786)

extern "C"
{
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

#include <string>
#include <iostream>
using namespace std;

//include the luabind headers. Make sure you have the paths set correctly
//to the lua, luabind and Boost files.
#include <luabind/luabind.hpp>
using namespace luabind;

#include "LuaHelperFunctions.h"



// open(pLua):luabind 的初始化函数,必须在打开 Lua 库后调用一次,Lua 才能用绑定功能。
int main()
{
  //create a lua state
  lua_State* pLua = lua_open();

  //open the lua libaries - new in lua5.1
  luaL_openlibs(pLua);

  //open luabind
  open(pLua);
 
  //load and run the script
  RunLuaScript(pLua, "classes_in_lua.lua");

  lua_close(pLua);
    
  return 0;
}