//==============================================================================================
//【文件说明】ScriptedStateMachine/main.cpp —— 第 6 章压轴:"用 Lua 脚本驱动的矿工状态机"
//
//【这个小演示干什么?】
//  复刻第 2 章矿工 Bob,但状态逻辑(挖矿/存钱/睡觉)全写在 Lua 脚本
//  StateMachineScript.lua 里。C++ 侧先用 luabind 把 Entity/Miner/状态机三类的方法注册给 Lua,
//  跑脚本后创建矿工 bob,把他的初始状态设成脚本里的 State_GoHome,再循环 Update 10 步——
//  每一步具体干什么,由 Lua 脚本里当前状态的 Execute 决定。
//
//【文件地图】main.cpp + Miner.h/.cpp(矿工)+ Entity.h(对象基类)+ ScriptedStateMachine.h(状态机模板);
//  luabind 来自第三方库 Common\luabind,Lua 来自 Common\lua-5.1.5。
//【调用流程】main → 注册三类到 Lua → RunLuaScript 定义好各状态 → new Miner bob →
//           从 globals 取出状态表 → SetCurrentState(回家)→ for 10 次 bob.Update()。
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

#include "Entity.h"
#include "Miner.h"
#include "LuaHelperFunctions.h"
#include "ScriptedStateMachine.h"


// 下面三个 Register*WithLua:分别用 luabind 把 状态机类 / Entity 基类 / Miner 子类
// 的方法(.def)导出给 Lua,脚本才能调用它们。Miner 用 bases<Entity> 声明继承。
void RegisterScriptedStateMachineWithLua(lua_State* pLua)
{
  module(pLua)
    [
      class_<ScriptedStateMachine<Miner> >("ScriptedStateMachine")
    
        .def("ChangeState", &ScriptedStateMachine<Miner>::ChangeState)
        .def("CurrentState", &ScriptedStateMachine<Miner>::CurrentState)
        .def("SetCurrentState", &ScriptedStateMachine<Miner>::SetCurrentState)
    ];  
}


void RegisterEntityWithLua(lua_State* pLua)
{
  module(pLua)
    [
      class_<Entity>("Entity")

        .def("Name", &Entity::Name)
        .def("ID", &Entity::ID)   
    ];  
}


void RegisterMinerWithLua(lua_State* pLua)
{
  module(pLua)
    [   
      class_<Miner, bases<Entity> >("Miner")

        .def("GoldCarried", &Miner::GoldCarried)
        .def("SetGoldCarried", &Miner::SetGoldCarried)
        .def("AddToGoldCarried", &Miner::AddToGoldCarried)
        .def("Fatigued", &Miner::Fatigued)
        .def("DecreaseFatigue", &Miner::DecreaseFatigue)
        .def("IncreaseFatigue", &Miner::IncreaseFatigue) 
        .def("GetFSM", &Miner::GetFSM)
    ];  
}



// LuaExceptionGuard guard:RAII 守护对象,离开作用域时自动检查 Lua 异常(本书工具)。
int main()
{
  //create a lua state
  lua_State* pLua = lua_open();

  LuaExceptionGuard guard(pLua);

  //open the lua libaries - new in lua5.1
  luaL_openlibs(pLua);
  
  //open luabind
  open(pLua);
  
  //bind the relevant classes to Lua
  RegisterEntityWithLua(pLua);
  RegisterMinerWithLua(pLua);
  RegisterScriptedStateMachineWithLua(pLua);
  
 
  //load and run the script
  RunLuaScript(pLua, "StateMachineScript.lua");
  
// globals(pLua) 取出 Lua 的全局表,里面装着脚本定义的所有状态函数/变量。
  //create a miner
  Miner bob("bob");

  //grab the global table from the lua state. This will inlclude
  //all the functions and variables defined in the scripts run so far
  //(StateMachineScript.lua in this example)
  object states = globals(pLua);

  //ensure states is a table
  if (type(states) == LUA_TTABLE)
  {
    //make sure Bob's CurrentState object is set to a valid state.
    bob.GetFSM()->SetCurrentState(states["State_GoHome"]);
// SetCurrentState(脚本里的 State_GoHome):把矿工初始状态设成"回家";
// 随后 for 循环 10 次 bob.Update(),每帧由当前状态的 Lua Execute 驱动矿工行动。

    //run him through a few update cycles
    for (int i=0; i<10; ++i)
    {
      bob.Update();
    }
  }

  return 0;
}


