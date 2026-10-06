//==============================================================================================
//【文件说明】StateMachine.h —— 状态机"管家":管角色当前在哪个状态
//
//【这个文件是干什么的?】
//  每个角色身上挂一个状态机对象,它记住"当前状态、上一个状态、全局状态"三个指针,
//  并提供:Update(每帧驱动当前状态)、ChangeState(切换状态,自动调旧状态的 Exit 和新
//  状态的 Enter)、HandleMessage(把消息先交给当前状态,处理不了再交给全局状态)。
//
//【谁在使用这个文件?】
//  Miner.h(WestWorld1)—— 矿工继承它,拥有一个状态机;
//  Raven、第 4 章等所有状态机角色同理。
//
//【本文件包含了谁?】
//  <cassert>   —— 断言 assert();<string> —— std::string;
//  State.h     —— 状态基类;Messaging/Telegram.h —— 消息体。
//==============================================================================================
 #ifndef STATEMACHINE_H
#define STATEMACHINE_H

//------------------------------------------------------------------------
//
//  Name:   StateMachine.h
//
//  Desc:   State machine class. Inherit from this class and create some 
//          states to give your agents FSM functionality
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【StateMachine —— 状态机类】:继承本类并创建若干状态,即可让你的角色
//   拥有状态机功能。作者:Mat Buckland。
//------------------------------------------------------------------------
#include <cassert>
#include <string>

#include "State.h"
#include "Messaging/Telegram.h"


// template <class entity_type>:模板,与 State.h 一样,用的时候指定角色类型(如 Miner)。
template <class entity_type>
// class StateMachine —— 状态机类。
class StateMachine
{
private:

  //a pointer to the agent that owns this instance
  //(原文注释:指向"拥有本状态机的那个角色")—— m_pOwner:本状态机属于哪个角色。
  entity_type*          m_pOwner;

  // 三个状态指针:m_pCurrentState=当前正处的状态;m_pPreviousState=上一个状态;
  // m_pGlobalState=全局状态(每帧都执行,不管当前状态是什么,比如"时刻要喘气")。
  State<entity_type>*   m_pCurrentState;
  
  //a record of the last state the agent was in
  State<entity_type>*   m_pPreviousState;

  //this is called every time the FSM is updated
  State<entity_type>*   m_pGlobalState;
  

public:

  // 构造函数:记下拥有者 owner,三个状态指针先都设为 NULL(空指针,表示还没设状态)。
  StateMachine(entity_type* owner):m_pOwner(owner),
                                   m_pCurrentState(NULL),
                                   m_pPreviousState(NULL),
                                   m_pGlobalState(NULL)
  {}

  virtual ~StateMachine(){}

  //(原文注释:用下面这些方法来初始化状态机)—— Set 系列:设置当前/全局/上一个状态。
  //use these methods to initialize the FSM
  void SetCurrentState(State<entity_type>* s){m_pCurrentState = s;}
  void SetGlobalState(State<entity_type>* s) {m_pGlobalState = s;}
  void SetPreviousState(State<entity_type>* s){m_pPreviousState = s;}
  
  //call this to update the FSM
  //(原文注释:调用它来更新状态机)—— Update:每帧调用。先跑全局状态,再跑当前状态。
  void  Update()const
  {
    //if a global state exists, call its execute method, else do nothing
    if(m_pGlobalState)   m_pGlobalState->Execute(m_pOwner);

    //same for the current state
    if (m_pCurrentState) m_pCurrentState->Execute(m_pOwner);
  }

  // HandleMessage:收到消息时调用。先交给当前状态处理;处理不了(返回 false),
  // 再交给全局状态。都不行才返回 false。
  bool  HandleMessage(const Telegram& msg)const
  {
    //first see if the current state is valid and that it can handle
    //the message
    if (m_pCurrentState && m_pCurrentState->OnMessage(m_pOwner, msg))
    {
      return true;
    }
  
    //if not, and if a global state has been implemented, send 
    //the message to the global state
    if (m_pGlobalState && m_pGlobalState->OnMessage(m_pOwner, msg))
    {
      return true;
    }

    return false;
  }

  //change to a new state
  //(原文注释:切换到新状态)—— ChangeState 四步:① 记下旧状态为"上一个";
  // ② 调旧状态的 Exit;③ 换成新状态;④ 调新状态的 Enter。
  void  ChangeState(State<entity_type>* pNewState)
  {
    assert(pNewState && "<StateMachine::ChangeState>:trying to assign null state to current");

    //keep a record of the previous state
    m_pPreviousState = m_pCurrentState;

    //call the exit method of the existing state
    m_pCurrentState->Exit(m_pOwner);

    //change state to the new state
    m_pCurrentState = pNewState;

    //call the entry method of the new state
    m_pCurrentState->Enter(m_pOwner);
  }

  //(原文注释:切回上一个状态)—— RevertToPreviousState:直接 ChangeState(上一个状态)。
  //change state back to the previous state
  void  RevertToPreviousState()
  {
    ChangeState(m_pPreviousState);
  }

  //returns true if the current state's type is equal to the type of the
  //class passed as a parameter. 
  // ↓↓↓ 上面两行英文注释的翻译:若"当前状态"的类型与参数状态类型相同,返回 true。
  // isInState:判断角色现在是不是正处在某种状态。typeid(x) 取对象的"类型编号",
  // 两个类型相同则相等。
  bool  isInState(const State<entity_type>& st)const
  {
    if (typeid(*m_pCurrentState) == typeid(st)) return true;
    return false;
  }

  // 访问器:只读返回当前/全局/上一个状态指针。
  State<entity_type>*  CurrentState()  const{return m_pCurrentState;}
  State<entity_type>*  GlobalState()   const{return m_pGlobalState;}
  State<entity_type>*  PreviousState() const{return m_pPreviousState;}

  //(原文注释:仅调试时用来取当前状态的名字)
  // GetNameOfCurrentState:用 typeid 拿到状态类的名字字符串,去掉开头 "class " 几个字。
  //only ever used during debugging to grab the name of the current state
  std::string         GetNameOfCurrentState()const
  {
    std::string s(typeid(*m_pCurrentState).name());

    //remove the 'class ' part from the front of the string
    if (s.size() > 5)
    {
      s.erase(0, 6);
    }

    return s;
  }
};




#endif


