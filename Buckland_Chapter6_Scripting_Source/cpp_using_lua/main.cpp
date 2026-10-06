//==============================================================================================
//【文件说明】cpp_using_lua/main.cpp —— "C++ 主动从 Lua 脚本里取数据/调函数"
//
//【这个小演示干什么?】
//  运行 cpp_using_lua.lua 后,C++ 通过 Lua 的"栈"依次:① 取出全局变量 age/name;
//  ② 取出表里 simple_table.name/age;③ 调用 Lua 函数 add(5,8) 拿回结果。
//  演示 C++ 侧如何读 Lua 写的数据、如何调用 Lua 定义的函数。
//
//【文件地图】本目录只有 main.cpp;LuaHelperFunctions.h 提供 RunLuaScript 小工具;
//  lua.h 等来自第三方库 Common\lua-5.1.5。脚本 cpp_using_lua.lua 与 exe 同目录。
//【调用流程】main → lua_open/打开库 → RunLuaScript 跑脚本 → 三次"取栈上数据"演示 → lua_close。
//==============================================================================================
extern "C"
{
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

#pragma comment(lib, "lua5.1.lib")
//#pragma comment(lib, "lua.lib")
//#pragma comment(lib, "lualib.lib")

#include <iostream>
#include <string>
using namespace std;

#include "LuaHelperFunctions.h"



// lua_State* pL:Lua 实例。lua_settop(pL,0)=清空栈,重新开始摆放要取的变量。
int main()
{
  //create a lua state
  lua_State* pL = lua_open();

  //open the libraries - new in Lua5.1
  luaL_openlibs(pL);
  
  RunLuaScript(pL, "cpp_using_lua.lua");
 
  cout << "\n[C++]:  1. Assigning lua string and number types to C++ std::string & int types\n";
  
  //reset the stack index
  lua_settop(pL, 0);

  //put the global variables 'age' and 'name' on the stack.
  lua_getglobal(pL, "age");
  lua_getglobal(pL, "name");

  //check that the variables are the correct type. (notice how the 
  //stack index starts at 1, not 0)
  if (!lua_isnumber(pL, 1) || !lua_isstring(pL, 2))
  {
    cout << "\n[C++]: ERROR: Invalid type!";
  }

  //now assign the values to C++ variables
  string name = lua_tostring(pL, 2);

  //notice the cast to int with this.
  int    age = (int)lua_tonumber(pL, 1);

  cout << "\n\n[C++]: name = " << name 
       << "\n[C++]: age  = " << age << endl;





// ---- 第 1 段:读全局变量 age/name ----
// lua_getglobal 把全局变量压到栈上;lua_isnumber/isstring 验类型;
// lua_tostring/lua_tonumber 真正取值。栈下标从 1 开始(注意不是 0)。
  cout << "\n\n[C++]:  2. Retrieving simple table";


  //put the table on the stack
  lua_getglobal(pL, "simple_table");

  if (!lua_istable(pL, -1))
  {
    cout << "\n[C++]: ERROR: simple_table is not a valid table";
  }

  else
  {
    //push the key onto the stack
    lua_pushstring(pL, "name");

    //table is now at -2 (key is at -1). lua_gettable now pops the key off
    //the stack and then puts the data found at the key location on the stack
    lua_gettable(pL, -2);

    //check that is the correct type
    if (!lua_isstring(pL, -1))
    {
      cout << "\n[C++]: ERROR: invalid type";
    }

    //grab the data
    name = lua_tostring(pL, -1);

    cout << "\n\n[C++]: name = " << name;

    lua_pop(pL, 1);

    /* now to do the same for the age */

    lua_pushstring(pL, "age");
    lua_gettable(pL, -2);
    if (!lua_isnumber(pL, -1))
    {
      cout << "\n[C++]: ERROR: invalid type";
    }

    //grab the data
    age = (int)lua_tonumber(pL, -1);

    lua_pop(pL, 1);

    cout << "\n[C++]: age  = " << age;
    
  }

 

// ---- 第 2 段:读表 simple_table ----
// lua_getglobal 把表压栈;再 push 键名 "name",用 lua_gettable 按键取值;
// 用完 lua_pop 弹出,保持栈整洁。-1 表示栈顶(倒数第一)。
   cout << "\n\n[C++]: 3. Calling a simple Lua function: add(a,b)";

   //get the function from the global table and push it on the stack
   lua_getglobal(pL, "add");

   //check that it is there
   if (!lua_isfunction(pL, -1))
   {
     cout << "\n\n[C++]: Oops! The lua function 'add' has not been defined";
   }

   //push some variables onto the lua stack
   lua_pushnumber(pL, 5);
   lua_pushnumber(pL, 8);

   //calling the function with parameters to set the number of parameters in
   //the lua func and how many return values it returns. Puts the result at
   //the top of the stack.
   lua_call(pL, 2, 1);

   //grab the result from the top of the stack
   int result = (int)lua_tonumber(pL, -1);

   lua_pop(pL, 1);

   cout << "\n\n[C++]: <lua>add(5,8) = " << result;



  
// ---- 第 3 段:调用 Lua 函数 add(5,8) ----
// 先把 add 函数压栈,再依次压两个参数,lua_call(pL,2,1)="2 个参数、要 1 个返回值";
// 返回值留在栈顶,lua_tonumber 取回。这是"C++ 调 Lua 函数"的标准套路。
  //tidy up
  lua_close(pL);

  cout << "\n\n\n";
    
  return 0;
}