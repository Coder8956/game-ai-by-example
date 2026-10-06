//==============================================================================================
//【文件说明】GoalKeeperStates.h —— 守门员的 5 个状态类声明
//
//【这个文件是干什么的?】
//  把守门员行为拆成 5 个状态类,都继承 State<GoalKeeper> 模板接口;
//  各自实现 Enter/Execute/Exit/OnMessage(单例写法同 FieldPlayerStates.h)。
//
//【守门员状态全景图】(切换逻辑实现在 GoalKeeperStates.cpp):
//   GlobalKeeperState  全局状态:每帧先跑(本工程里为空实现);
//   TendGoal           守门:守在门前门线附近,随球左右移动站位(初始状态);
//   InterceptBall      拦截:球进入拦截范围→冲出去把球截下;
//   ReturnHome         回门:冲过头离门太远→退回门前;
//   PutBallBackInPlay  发球:拿到球后把球踢还给本方球员,恢复比赛。
//  【口诀】平时守门;球靠近→拦截;离门太远→回门;拿到球→发球。
//
//【与相关文件的关系】FSM/State.h 接口;class GoalKeeper;(前置声明);
//  GoalKeeperStates.cpp 是实现;GoalKeeper.h/.cpp 用 m_pStateMachine 指向这些状态。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护:原理详见 Goal.h。
#ifndef KEEPERSTATES_H
#define KEEPERSTATES_H
//------------------------------------------------------------------------
//
//  Name: GoalKeeperStates.h
//
//  Desc:   Declarations of all the states used by a Simple Soccer
//          goalkeeper
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:GoalKeeperStates.h
//   描述  :SimpleSoccer 守门员所用全部状态的声明。
//   作者  :Mat Buckland,2003 年(本书作者)
//
//------------------------------------------------------------------------
#include <string>
#include "FSM/State.h"
#include "Messaging/Telegram.h"
#include "constants.h"


class GoalKeeper;
class SoccerPitch;


//-----------------------------------------------------------------------------
// 下面 5 个类都是单例状态(私有构造 + 静态 Instance),实现 State<GoalKeeper> 接口。
//-----------------------------------------------------------------------------
class GlobalKeeperState: public State<GoalKeeper>
{
private:
  
  GlobalKeeperState(){}

public:

  //this is a singleton
  static GlobalKeeperState* Instance();

  void Enter(GoalKeeper* keeper){}

  void Execute(GoalKeeper* keeper){}

  void Exit(GoalKeeper* keeper){}

  bool OnMessage(GoalKeeper*, const Telegram&);
};

//-----------------------------------------------------------------------------

class TendGoal: public State<GoalKeeper>
{
private:
  
  TendGoal(){}

public:

  //this is a singleton
  static TendGoal* Instance();

  void Enter(GoalKeeper* keeper);

  void Execute(GoalKeeper* keeper);

  void Exit(GoalKeeper* keeper);

  bool OnMessage(GoalKeeper*, const Telegram&){return false;}
};

//------------------------------------------------------------------------
class InterceptBall: public State<GoalKeeper>
{
private:
  
  InterceptBall(){}

public:

  //this is a singleton
  static InterceptBall* Instance();

  void Enter(GoalKeeper* keeper);

  void Execute(GoalKeeper* keeper);

  void Exit(GoalKeeper* keeper);

  bool OnMessage(GoalKeeper*, const Telegram&){return false;}
};

//------------------------------------------------------------------------
class ReturnHome: public State<GoalKeeper>
{
private:
  
  ReturnHome(){}

public:

  //this is a singleton
  static ReturnHome* Instance();

  void Enter(GoalKeeper* keeper);

  void Execute(GoalKeeper* keeper);

  void Exit(GoalKeeper* keeper);

  bool OnMessage(GoalKeeper*, const Telegram&){return false;}
};

//------------------------------------------------------------------------
class PutBallBackInPlay: public State<GoalKeeper>
{
private:
  
  PutBallBackInPlay(){}

public:

  //this is a singleton
  static PutBallBackInPlay* Instance();

  void Enter(GoalKeeper* keeper);

  void Execute(GoalKeeper* keeper);

  void Exit(GoalKeeper* keeper){}

  bool OnMessage(GoalKeeper*, const Telegram&){return false;}
};





#endif