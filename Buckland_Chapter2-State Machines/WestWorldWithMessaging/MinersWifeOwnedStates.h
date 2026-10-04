//==============================================================================================
//【文件说明】MinersWifeOwnedStates.h —— 妻子的 4 个"具体状态"类声明(状态机的角色表)
//
//【这个文件是干什么的?】
//  把矿工妻子 Elsa 的人生拆成 4 个状态类,每个都继承 State<MinersWife> 接口:
//    ① WifesGlobalState —— 全局状态:每帧最先执行,随机冒出"想上厕所"的念头
//       (这是"全局状态"——不管当前在干嘛,每帧都先跑一遍);
//    ② DoHouseWork      —— 做家务:扫地/洗碗/铺床随机三选一(妻子的默认状态);
//    ③ VisitBathroom    —— 去厕所:全局状态随机触发,上完就回到之前的状态;
//    ④ CookStew         —— 炖菜:收到丈夫"我回来了"消息后进入,
//                           放菜进烤箱后给自己发"延迟 1.5 秒"的消息;
//                           时间到 → 告诉丈夫"炖菜好了"。
//
//【状态切换全景图】(切换代码在 MinersWifeOwnedStates.cpp)
//     ┌────────────── 全局状态(每帧先跑):10% 概率 → ③去厕所 ──────────────┐
//     │                                                                   │
//     ②做家务 ◀──(炖菜好了,从 CookStew 切回)── ④炖菜 ◀──收到"我回来了"消息──┐
//     │                                             │                      │
//     └──────(10%概率)──────────────────────────────┘                      │
//     ③去厕所 ──上完(Execute)→ RevertToPreviousState 回到上一个状态 ────────┘
//
//【与相关文件的关系】
//  fsm/State.h          —— 4 个类的"共同父类"(模板,参数是 MinersWife);
//  class MinersWife;    —— 前置声明(接口参数只用指针);
//  MinersWifeOwnedStates.cpp —— 4 个类的具体实现;
//  MinersWife.h / .cpp  —— 妻子持有状态机 m_pStateMachine,构造函数把初始
//                           当前状态设为 ②、全局状态设为 ①;
//  MinerOwnedStates.h   —— 妻子状态实现文件会用到矿工的状态(见 .cpp),
//                           头文件本身不依赖它。
//
//【设计模式:单例 + 全局状态】
//  与 MinerOwnedStates.h 相同,4 个类都用"单例三件套"(私有构造、私有拷贝、
//  静态 Instance())。全局状态的特殊之处:StateMachine::Update 每帧都会
//  先执行它(见 StateMachine.h),所以"随机想上厕所"这种时刻都该考虑的事
//  放在这里最合适。
//==============================================================================================

//------------------------------------------------------------------------------------------------
// 包含保护(原理详解见 Locations.h):防止本文件被重复包含。
//------------------------------------------------------------------------------------------------
#ifndef MINERSWIFE_OWNED_STATES_H
#define MINERSWIFE_OWNED_STATES_H
//------------------------------------------------------------------------
//
//  Name:   MinersWifeOwnedStates.h
//
//  Desc:   All the states that can be assigned to the MinersWife class
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:MinersWifeOwnedStates.h
//   描述  :All the states that can be assigned to the MinersWife class
//          —— 能分配给"矿工妻子"类的所有状态
//   作者  :Mat Buckland,2002 年
//------------------------------------------------------------------------

// 状态接口模板:下面 4 个类都要写": public State<MinersWife>"继承它。
#include "fsm/State.h"

// 【前置声明】class MinersWife;
//   只报名字不给内容(参数只用 MinersWife* 指针)。完整定义在 MinersWife.h。
class MinersWife;



//------------------------------------------------------------------------
//

//------------------------------------------------------------------------
// ↓↓↓ 原文分隔线(内容为空)。下面是【状态①:全局状态】。
//--------------------------------------------------------------------------------
//【类 WifesGlobalState : public State<MinersWife>】全局状态。
//  特点:Enter 和 Exit 都是空实现(全局状态永远"在场",无所谓进出),
//  只有 Execute(每帧检查是否想去厕所)和 OnMessage(收消息)有内容。
//--------------------------------------------------------------------------------
class WifesGlobalState : public State<MinersWife>
{  
private:
  
  // 【私有构造函数】单例三件套第 1 件:外界不能 new 第二份。
  WifesGlobalState(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  // 拷贝构造/赋值运算符私有:单例三件套第 2 件。
  WifesGlobalState(const WifesGlobalState&);
  WifesGlobalState& operator=(const WifesGlobalState&);
 
public:

  //this is a singleton
  //(原文注释:这是一个单例)

  // 【静态 Instance】单例三件套第 3 件:拿唯一实例的入口。
  static WifesGlobalState* Instance();
  
  // 【Enter 空实现】{}(在声明处直接给空函数体):
  // 全局状态没有"进入"动作,什么都不做。
  virtual void Enter(MinersWife* wife){}

  // 【Execute】每帧执行:10% 概率切换去上厕所(实现见 .cpp)。
  virtual void Execute(MinersWife* wife);

  // 【Exit 空实现】全局状态没有"离开"动作。
  virtual void Exit(MinersWife* wife){}

  // 【OnMessage】全局状态收消息:丈夫"我回来了"→ 切到炖菜(实现见 .cpp)。
  virtual bool OnMessage(MinersWife* wife, const Telegram& msg);
};


//------------------------------------------------------------------------
//

//------------------------------------------------------------------------
// ↓↓↓ 原文分隔线(内容为空)。下面是【状态②:做家务】。
//--------------------------------------------------------------------------------
//【类 DoHouseWork : public State<MinersWife>】做家务(妻子的默认状态)。
//  丈夫没回家、没炖菜时,她就在扫地/洗碗/铺床之间随机切换着干。
//--------------------------------------------------------------------------------
class DoHouseWork : public State<MinersWife>
{
private:

  // 【私有构造函数】单例三件套第 1 件。
  DoHouseWork(){}
  
  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  // 拷贝构造/赋值运算符私有:单例三件套第 2 件。
  DoHouseWork(const DoHouseWork&);
  DoHouseWork& operator=(const DoHouseWork&);

public:

  //this is a singleton
  //(原文注释:这是一个单例)

  // 【静态 Instance】单例三件套第 3 件。
  static DoHouseWork* Instance();
  
  virtual void Enter(MinersWife* wife);

  virtual void Execute(MinersWife* wife);

  virtual void Exit(MinersWife* wife);
  
  virtual bool OnMessage(MinersWife* wife, const Telegram& msg);

};



//------------------------------------------------------------------------
//

//------------------------------------------------------------------------
// ↓↓↓ 原文分隔线(内容为空)。下面是【状态③:去厕所】。
//--------------------------------------------------------------------------------
//【类 VisitBathroom : public State<MinersWife>】去厕所。
//  全局状态随机触发进入;上完厕所(Execute)后 RevertToPreviousState
//  回到进入前的状态(通常是做家务)。
//--------------------------------------------------------------------------------
class VisitBathroom : public State<MinersWife>
{
private:
  
  // 【私有构造函数】单例三件套第 1 件。
  VisitBathroom(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  // 拷贝构造/赋值运算符私有:单例三件套第 2 件。
  VisitBathroom(const VisitBathroom&);
  VisitBathroom& operator=(const VisitBathroom&);
 
public:

  //this is a singleton
  //(原文注释:这是一个单例)

  // 【静态 Instance】单例三件套第 3 件。
  static VisitBathroom* Instance();
  
  virtual void Enter(MinersWife* wife);

  virtual void Execute(MinersWife* wife);

  virtual void Exit(MinersWife* wife);

  virtual bool OnMessage(MinersWife* wife, const Telegram& msg);

};


//------------------------------------------------------------------------
//

//------------------------------------------------------------------------
// ↓↓↓ 原文分隔线(内容为空)。下面是【状态④:炖菜】。
//--------------------------------------------------------------------------------
//【类 CookStew : public State<MinersWife>】炖菜。
//  丈夫回家(收到"我回来了"消息)后进入:放菜进烤箱,给自己发一条
//  "延迟 1.5 秒"的"炖菜好了"消息;时间到 → 通知丈夫"炖菜好了"。
//  注意:炖菜状态结束后切回做家务(DoHouseWork),而不是回上一个状态。
//--------------------------------------------------------------------------------
class CookStew : public State<MinersWife>
{
private:
  
  // 【私有构造函数】单例三件套第 1 件。
  CookStew(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  // 拷贝构造/赋值运算符私有:单例三件套第 2 件。
  CookStew(const CookStew&);
  CookStew& operator=(const CookStew&);
 
public:

  //this is a singleton
  //(原文注释:这是一个单例)

  // 【静态 Instance】单例三件套第 3 件。
  static CookStew* Instance();
  
  virtual void Enter(MinersWife* wife);

  virtual void Execute(MinersWife* wife);

  virtual void Exit(MinersWife* wife);

  virtual bool OnMessage(MinersWife* wife, const Telegram& msg);
};


// 包含保护结束:与文件开头的 #ifndef MINERSWIFE_OWNED_STATES_H 配对。
#endif