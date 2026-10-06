//==============================================================================================
//【文件说明】TeamStates.h —— 球队级的 3 个战术状态类声明
//
//【这个文件是干什么的?】
//  整支球队作为一个整体,也有一个状态机,在 3 个战术状态间切换;
//  每个都是单例,继承 State<SoccerTeam> 模板接口。
//
//【球队状态全景图】(切换逻辑实现在 TeamStates.cpp):
//   PrepareForKickOff  开球准备:比赛开始/进球后,叫所有人回各自老家区域,
//                      全员就位后才吹响哨子进入比赛;
//   Attacking          进攻:本方控球时,组织压上、找射门/传球机会;
//   Defending          防守:对方控球时,全员回防(球队初始状态)。
//  【口诀】丢球→防守;控球→进攻;开球或进球后→先全体归位再开赛。
//
//【与相关文件的关系】FSM/State.h 接口;class SoccerTeam;(前置声明);
//  TeamStates.cpp 是实现;SoccerTeam.h/.cpp 用 m_pStateMachine 指向这些状态。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护:原理详见 Goal.h。
#ifndef TEAMSTATES_H
#define TEAMSTATES_H
//------------------------------------------------------------------------
//
//  Name: TeamStates.h
//
//  Desc: State prototypes for soccer team states
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:TeamStates.h
//   描述  :足球队各状态的原型声明。
//   作者  :Mat Buckland,2003 年(本书作者)
//
//------------------------------------------------------------------------
#include <string>

#include "FSM/State.h"
#include "Messaging/Telegram.h"


class SoccerTeam;





//------------------------------------------------------------------------
//------------------------------------------------------------------------
// 下面 3 个类都是单例状态(私有构造 + 静态 Instance),实现 State<SoccerTeam> 接口。
//------------------------------------------------------------------------
class Attacking : public State<SoccerTeam>
{ 
private:
  
  Attacking(){}

public:

  //this is a singleton
  static Attacking* Instance();

  void Enter(SoccerTeam* team);

  void Execute(SoccerTeam* team);

  void Exit(SoccerTeam* team);

  bool OnMessage(SoccerTeam*, const Telegram&){return false;}
};

//------------------------------------------------------------------------
class Defending : public State<SoccerTeam>
{ 
private:
  
  Defending(){}

public:

    //this is a singleton
  static Defending* Instance();

  void Enter(SoccerTeam* team);

  void Execute(SoccerTeam* team);

  void Exit(SoccerTeam* team);

  bool OnMessage(SoccerTeam*, const Telegram&){return false;}
};

//------------------------------------------------------------------------
class PrepareForKickOff : public State<SoccerTeam>
{ 
private:
  
  PrepareForKickOff(){}

public:

    //this is a singleton
  static PrepareForKickOff* Instance();
  
  void Enter(SoccerTeam* team);

  void Execute(SoccerTeam* team);

  void Exit(SoccerTeam* team);

  bool OnMessage(SoccerTeam*, const Telegram&){return false;}
};


#endif