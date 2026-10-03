//==============================================================================================
//【文件说明】MinersWifeOwnedStates.h —— 妻子的 3 个"具体状态"类声明(她的角色表)
//
//【这个文件是干什么的?】
//  把妻子 Elsa 的人生拆成 3 个状态类,每个类都继承 State.h 的 State<MinersWife>
//  接口,各自实现 Enter(进入)/Execute(每步)/Exit(离开)三个函数:
//    ① WifesGlobalState —— 全局状态:不干具体活,每步只"掷骰子"——
//                          1/10 的概率突然想上厕所,想上就切到③;
//    ② DoHouseWork      —— 做家务(妻子的当前状态):从拖地/洗碗/铺床
//                          三件家务里随机挑一件干;
//    ③ VisitBathroom    —— 上厕所(典型的"状态 blip"):上完自动退回
//                          被打断的家务状态。
//
//【状态切换全景图】(机制详解见 StateMachine.h 头部的"全局状态与状态 blip")
//   状态机每走一步的执行顺序:先①全局状态、再②当前状态。
//
//        ①全局状态(每步检查:1/10 想上厕所?)──想──▶ ③上厕所(blip 插播)
//              │ 没想上 → 什么都不做                        │
//              ▼                                           │ 上完厕所
//        ②做家务(拖地/洗碗/铺床随机轮换)◀──RevertToPreviousState──┘
//
//  【口诀】全局状态先登场;想上厕所就插播;上完退回家务里,什么都没耽误。
//
//【与相关文件的关系】
//  State.h                    —— 3 个类的"共同父类"(State<MinersWife> 模板);
//  class MinersWife;(下面)    —— 前置声明:接口函数的参数是"MinersWife 指针",
//                                指针不需要完整定义(原理详见 State.h);
//  MinersWifeOwnedStates.cpp   —— 3 个类的具体实现(掷骰子、台词、切换全在那边);
//  MinersWife.h / MinersWife.cpp —— 妻子构造函数安装初始状态:
//       SetCurrentState(②做家务) + SetGlobalState(①全局状态)。
//
//【单例(Singleton)】3 个类与矿工的 4 个状态类写法完全相同:
//  私有构造 + 私有拷贝/赋值 + 静态 Instance()。三件套的逐行详解不再重复,
//  原理详见 MinerOwnedStates.h 中 EnterMineAndDigForNugget 的注释。
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
//          —— 能分配给"矿工妻子"类的所有状态(妻子专属的一组状态)
//   作者  :Mat Buckland,2002 年(本书作者)
//------------------------------------------------------------------------

// State 接口的完整定义:下面 3 个类都要写": public State<MinersWife>"
// 继承它(编译器必须先见到父类的完整定义)。
#include "State.h"

// 前置声明:只报"MinersWife 是个类"的名字,不给内容(接口函数的参数
// 只用到"MinersWife 指针")。完整定义在 MinersWife.h,由实现文件
// MinersWifeOwnedStates.cpp 去包含。
class MinersWife;

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

// ↓↓↓(原文此处只留了空的注释框,没有写说明文字)补上本文件风格的中文说明:
// 【状态①:妻子的全局状态】状态机每步先执行它的 Execute:以 1/10 的概率
// 决定"想上厕所"→ 切换到 VisitBathroom;不想上则什么都不做(继续干家务)。
// 它没有台词、也不需要 Enter/Exit 的开场白/告别词(下面两个空函数体)。
//------------------------------------------------------------------------
class WifesGlobalState : public State<MinersWife>
{  
private:
  
  // 私有构造(单例第 1 件,三件套详解见 MinerOwnedStates.h)。
  WifesGlobalState(){}
  
  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  // 私有拷贝构造 + 私有赋值(单例第 2 件)。
  WifesGlobalState(const WifesGlobalState&);
  WifesGlobalState& operator=(const WifesGlobalState&);
 
// public:(公有区)单例入口 + 接口的三个函数。
public:

  // 静态取唯一实例的入口(单例第 3 件)。
  static WifesGlobalState* Instance();
  
  // 【空函数体】全局状态不需要"进入"的开场白——花括号{}里什么都不写,
  // 调用它等于什么都不干。(虚函数在子类里可省 virtual,保留是醒目写法。)
  virtual void Enter(MinersWife* wife){}

  // 唯一有实际工作的函数:掷骰子决定要不要上厕所
  // (实现见 MinersWifeOwnedStates.cpp)。
  virtual void Execute(MinersWife* wife);

  // 同 Enter:全局状态不需要"离开"的告别词,空函数体。
  virtual void Exit(MinersWife* wife){}
};


//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

// ↓↓↓(原文此处只留了空的注释框)补上说明:
// 【状态②:做家务】妻子的"当前状态":每步从拖地(Moppin' the floor)、
// 洗碗(Washin' the dishes)、铺床(Makin' the bed)三件家务里随机挑一件。
// 它永不主动切换状态——只有全局状态能把她"叫走"上厕所。
//------------------------------------------------------------------------
class DoHouseWork : public State<MinersWife>
{

private:
  
  // 私有构造(单例三件套,详见 MinerOwnedStates.h)。
  DoHouseWork(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  DoHouseWork(const DoHouseWork&);
  DoHouseWork& operator=(const DoHouseWork&); 
  
public:

  //this is a singleton
  //(原文注释:这是一个单例)

  static DoHouseWork* Instance();
  
  // 实现 State<MinersWife> 接口的三个动作:
  // 进入家务状态时执行一次(本类留空——家务是常态,无需开场白)。
  virtual void Enter(MinersWife* wife);

  // 每步执行:随机挑一件家务干(台词随机三选一)。
  virtual void Execute(MinersWife* wife);

  // 离开家务状态时执行一次(本类留空)。
  virtual void Exit(MinersWife* wife);

};



//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

// ↓↓↓(原文此处只留了空的注释框)补上说明:
// 【状态③:上厕所】一个"状态 blip"(短暂插播):由全局状态随机触发进入;
// Execute 里"上完厕所"后立刻调用 wife->GetFSM()->RevertToPreviousState()
// 退回被打断的家务状态——来了就走,仿佛什么都没发生。
//------------------------------------------------------------------------
class VisitBathroom : public State<MinersWife>
{
private:
  
  // 私有构造(单例三件套,详见 MinerOwnedStates.h)。
  VisitBathroom(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  VisitBathroom(const VisitBathroom&);
  VisitBathroom& operator=(const VisitBathroom&);
 
public:

  //this is a singleton
  //(原文注释:这是一个单例)

  static VisitBathroom* Instance();
  
  // 实现 State<MinersWife> 接口的三个动作:
  // 进入厕所状态时执行一次(念台词:走去卫生间)。
  virtual void Enter(MinersWife* wife);

  // 每步执行:念台词(上完了)→ RevertToPreviousState 退回家务。
  virtual void Execute(MinersWife* wife);

  // 离开厕所状态时执行一次(念台词:离开卫生间)。
  virtual void Exit(MinersWife* wife);

};


// 配对文件上方 #ifndef MINERSWIFE_OWNED_STATES_H 的收尾(作用见
// Locations.h 的"包含保护"注释)。
#endif