//==============================================================================================
//【文件说明】ScriptedStateMachine.h —— "脚本化状态机"模板类
//
//【这个文件是干什么的?】
//  与第 2 章用 C++ 类写状态不同,这里每个"状态"是一个 Lua 表(里面有 Enter/Execute/Exit
//  三个函数)。本 C++ 类只做一件事:记住"当前状态",每帧调用它的 Execute,切换时先调
//  旧状态 Exit 再调新状态 Enter。这样状态逻辑全写在 Lua 脚本里,改行为不用重编译 C++。
//
//【谁在使用这个文件?】Miner.h —— 矿工持有它的对象 m_pStateMachine;
//                    main.cpp —— 用 luabind 把 ChangeState/SetCurrentState 暴露给 Lua。
//【C++ 小课堂:template 模板】template<class entity_type> 表示这是个泛型类,
//  entity_type 是占位类型,用时填上(如 ScriptedStateMachine<Miner>),一份代码适用各种角色。
//==============================================================================================
#ifndef SCRIPTEDScriptedStateMachine_H
#define SCRIPTEDScriptedStateMachine_H
#pragma warning (disable : 4786)
//------------------------------------------------------------------------
//
//  Name:   ScriptedStateMachine.h
//
//  Desc:   A simple scripted state machine class. Inherit from this class and 
//          create some states in Lua to give your agents FSM functionality
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------

extern "C"
{
  #include <lua.h>
}

#include <luabind/luabind.hpp>
#include <cassert>


template <class entity_type>
class ScriptedStateMachine
{

private:

  //pointer to the agent that owns this instance
  entity_type*      m_pOwner;

  //the current state is a lua table of lua functions. A table may be
  //represented in C++ using a luabind::object
  luabind::object   m_CurrentState;
//(原文注释:当前状态是一个装着若干 Lua 函数的 table,在 C++ 里用 luabind::object 表示)
// m_pOwner:持有本状态机的角色指针; m_CurrentState:当前状态(Lua 表的 C++ 侧代理对象)。
  
public:

  ScriptedStateMachine(entity_type* owner):m_pOwner(owner){}

  //use these methods to initialize the FSM
  void SetCurrentState(const luabind::object& s){m_CurrentState = s;}

  
  //call this to update the FSM
// SetCurrentState:直接设定当前状态(开机初始化用)。
  void  Update()
  {
    //make sure the state is valid before calling its Execute 'method'
    if (m_CurrentState.is_valid())  //this could also be written as 'if(m_CurrentState)'
    { 
      m_CurrentState["Execute"](m_pOwner);
// Update:每帧调用。先判断状态对象有效(is_valid),再用 [] 取它的 "Execute" 函数,
// 并把 m_pOwner(矿工自己)当参数传给它——即执行当前状态的"这一步行为"。
    }
  }

  //change to a new state
  void  ChangeState(const luabind::object& new_state)
  {
    //call the exit method of the existing state
    m_CurrentState["Exit"](m_pOwner);

    //change state to the new state
    m_CurrentState = new_state;

    //call the entry method of the new state
    m_CurrentState["Enter"](m_pOwner);
// ChangeState(切换到 new_state):① 调旧状态的 Exit;② 换上新状态;③ 调新状态的 Enter。
  }

  //retrieve the current state
  const luabind::object&  CurrentState()const{return m_CurrentState;}
};




#endif


