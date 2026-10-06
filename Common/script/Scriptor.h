//==============================================================================================
//【文件说明】Scriptor.h —— Lua 配置文件读取器
//
//【这个文件是干什么的?】
//  游戏常把参数(敌人数量、移动速度等)写在 Lua 配置文件里,而不是硬编码。本类包了一个
//  Lua 解释器(lua_State),构造时打开 Lua 库,然后用 RunScriptFile 跑脚本,再用 GetInt/
//  GetFloat/GetDouble/GetString/GetBool 按变量名把值读回来,省去改代码重编译。
//
//【谁在使用这个文件?】第 6 章脚本化工程、Raven 的 GameWorld(读参数)。
//【本文件包含了谁?】lua.h/lualib.h/lauxlib.h(Lua C 头文件)、LuaHelperFunctions.h。
//==============================================================================================
#ifndef SCRIPTOR_H
#define SCRIPTOR_H
//-----------------------------------------------------------------------------
//
//  Name:   Scriptor.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class encapsulating the basic functionality necessary to read a
//          Lua config file
//-----------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Scriptor —— 脚本器】:封装"读取 Lua 配置文件"所需基本功能的类。
//------------------------------------------------------------------------
// extern "C":Lua 是 C 写的,这里告诉编译器"按 C 的方式链接下面几个头文件",
//   否则 C++ 的名字改编会导致链接不上 Lua 库。
extern "C"
{
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

// #pragma comment(lib, ...):让链接器自动链接 lua5.1.lib(省得在工程设置里手动加)。
#pragma comment(lib, "lua5.1.lib")
//#pragma comment(lib, "lualib.lib")

#include "LuaHelperFunctions.h"



// class Scriptor —— Lua 读取器。m_pLuaState 是 Lua 解释器实例句柄。
class Scriptor
{
private:

  lua_State* m_pLuaState;

public:

  // 构造函数:luaL_newstate() 新建一个 Lua 解释器;luaL_openlibs 打开标准库。
  Scriptor():m_pLuaState(luaL_newstate())
  {
    //open the libraries
    luaL_openlibs(m_pLuaState);
  }

  ~Scriptor(){lua_close(m_pLuaState);}

  // 析构:lua_close 关闭解释器。RunScriptFile:运行指定名字的 Lua 脚本文件。
  void RunScriptFile(char* ScriptName)
  {
     RunLuaScript(m_pLuaState, ScriptName);
  }

  lua_State* GetState(){return m_pLuaState;}


  // 下面是按名字读各种类型值的便捷函数:GetInt/GetFloat/GetDouble/GetString/GetBool,
  // 都转调 LuaHelperFunctions.h 里的 PopLuaXxx。
  int GetInt(char* VariableName)
  {
    return PopLuaNumber<int>(m_pLuaState, VariableName);
  }
    
  double GetFloat(char* VariableName)
  {
    return PopLuaNumber<float>(m_pLuaState, VariableName);
  }

  double GetDouble(char* VariableName)
  {
    return PopLuaNumber<double>(m_pLuaState, VariableName);
  }

  std::string GetString(char* VariableName)
  {
    return PopLuaString(m_pLuaState, VariableName);
  }

  bool GetBool(char* VariableName)
  {
    return PopLuaBool(m_pLuaState, VariableName);
  }
};

#endif

 
  

