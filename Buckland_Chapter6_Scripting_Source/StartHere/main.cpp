//==============================================================================================
//【文件说明】StartHere/main.cpp —— 第 6 章最简单的"起步"示例:C++ 跑一个 Lua 脚本
//
//【这个小演示干什么?】
//  只做三步:打开一个 Lua 状态(lua_State)→ 打开标准库 → 运行 your_first_lua_script.lua。
//  用来验证 C++ 与 Lua 的链接是否打通,是读者跟着书敲的第一个程序。
//
//【文件地图】本目录只有这一个 main.cpp;它依赖 Common\lua-5.1.5(第三方 Lua 官方库),
//  脚本 your_first_lua_script.lua 与本 exe 同目录。第三方库内部不展开注释。
//【调用流程】main → lua_open 建状态 → luaL_openlibs → luaL_dofile 跑脚本 → lua_close 收尾。
//==============================================================================================
// extern "C" { ... }:Lua 是用 C 写的库,C++ 包含它的头文件时要用 extern "C"
// 告诉编译器"按 C 的方式编译这些声明",否则函数名会被 C++ 改名导致链接不上。
 extern "C"
{
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

//include the lua libraries. If your compiler doesn't support this pragma
//then don't forget to add the libraries in your project settings!
//#pragma comment(lib, "lua.lib")
//#pragma comment(lib, "lualib.lib")
 #pragma comment(lib, "lua5.1.lib")

#include <iostream>

// lua_State*:Lua 的"状态机实例",所有 Lua 操作都围绕它。lua_open 新建,用完 lua_close。
int main()
{
  //create a lua state
  lua_State* pL = lua_open();

  //enable access to the standard libraries
 /* luaopen_base(pL);
  luaopen_string(pL);
  luaopen_table(pL);
  luaopen_math(pL);
  luaopen_io(pL);*/

  //open the lua libaries - new in lua5.1
  luaL_openlibs(pL);
//(原文注释:打开 Lua 标准库——Lua5.1 新写法)
// luaL_dofile:加载并执行整个脚本文件;出错(返回非 0)就打印错误号并退出。
  
  if (int error = luaL_dofile(pL, "your_first_lua_script.lua") != 0)
  {
    std::cout << "\n[C++]: ERROR(" << error << "): Problem with lua script file!\n\n" << std::endl;

    return 0;
  }

  //tidy up
  lua_close(pL);

  return 0;
}