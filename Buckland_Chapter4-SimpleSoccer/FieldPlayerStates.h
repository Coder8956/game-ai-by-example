//==============================================================================================
//【文件说明】FieldPlayerStates.h —— 场上球员的 8 个状态类声明(状态机角色表)
//
//【这个文件是干什么的?】
//  把场上球员的行为拆成 8 个状态类,每个都继承 State<FieldPlayer> 模板接口,
//  各自实现 Enter(进入一次)/Execute(每步)/Exit(离开一次)/OnMessage(收消息)。
//
//【场上球员状态全景图】(切换逻辑实现在 FieldPlayerStates.cpp):
//   GlobalPlayerState  全局状态:每帧都先跑,负责"收到消息先归位"等公共逻辑;
//   Wait               等待:开球前/没轮到我时原地待命,脸转向球;
//   ReturnToHomeRegion 回位:跑回自己老家区域;
//   ChaseBall          追球:我离球最近且没控球队员时追上去;
//   Dribble            带球:我控着球,边推进边找射门/传球机会;
//   KickBall           踢球:决定射门还是传球的瞬间动作;
//   ReceiveBall        接球:队友要传给我,我跑到接应点等球;
//   SupportAttacker   支援:作为接应者跑到最佳甜区支援控球者。
//  【口诀】没人控球→追球;自己控球→带球;带球时决定射门/传球→踢球;
//         队友传球给我→接球;让我接应→支援;叫我回家→回位;没事→等待。
//
//【与相关文件的关系】
//  FSM/State.h —— 状态接口模板;class FieldPlayer;(下面前置声明);
//  FieldPlayerStates.cpp —— 8 个状态类的具体实现;
//  FieldPlayer.h/.cpp —— 球员用 m_pStateMachine 指向这些状态。
//
//【单例回顾】每个状态类都是单例(私有构造 + 静态 Instance()),写法与
//  WestWorld1/MinerOwnedStates.h 完全相同,此处不再逐行展开,原理详见该处注释。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护:原理详见 Goal.h。
#ifndef FIELDPLAYERSTATES_H
#define FIELDPLAYERSTATES_H
//------------------------------------------------------------------------
//
//  Name: FieldPlayerStates.h
//
//  Desc: States for the field players of Simple Soccer. See my book
//        for detailed descriptions
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:FieldPlayerStates.h
//   描述  :SimpleSoccer 场上球员的各个状态(详见原书)。
//   作者  :Mat Buckland,2003 年(本书作者)
//
//------------------------------------------------------------------------

#include <string>

#include "FSM/State.h"
#include "Messaging/Telegram.h"
#include "constants.h"


class FieldPlayer;
class SoccerPitch;


//------------------------------------------------------------------------
//------------------------------------------------------------------------
// 下面 8 个类都是"单例状态"(私有构造 + 静态 Instance),实现 State<FieldPlayer> 接口。
// OnMessage 返回 true 表示消息已处理、不再传递;false 表示没处理。
//------------------------------------------------------------------------
class GlobalPlayerState : public State<FieldPlayer>
{
private:
  
  GlobalPlayerState(){}

public:

  //this is a singleton
  static GlobalPlayerState* Instance();

  void Enter(FieldPlayer* player){}

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player){}

  bool OnMessage(FieldPlayer*, const Telegram&);
};

//------------------------------------------------------------------------
class ChaseBall : public State<FieldPlayer>
{
private:
  
  ChaseBall(){}

public:

  //this is a singleton
  static ChaseBall* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player);

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};

//------------------------------------------------------------------------
class Dribble : public State<FieldPlayer>
{
private:
  
  Dribble(){}

public:

  //this is a singleton
  static Dribble* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player){}

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};


//------------------------------------------------------------------------
class ReturnToHomeRegion: public State<FieldPlayer>
{
private:
  
  ReturnToHomeRegion(){}

public:

  //this is a singleton
  static ReturnToHomeRegion* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player);

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};

//------------------------------------------------------------------------
class Wait: public State<FieldPlayer>
{
private:
  
  Wait(){}

public:

  //this is a singleton
  static Wait* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player);

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};

//------------------------------------------------------------------------
class KickBall: public State<FieldPlayer>
{
private:
  
  KickBall(){}

public:

  //this is a singleton
  static KickBall* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player){}

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};

//------------------------------------------------------------------------
class ReceiveBall: public State<FieldPlayer>
{
private:
  
  ReceiveBall(){}

public:

  //this is a singleton
  static ReceiveBall* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player);

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};


//------------------------------------------------------------------------
class SupportAttacker: public State<FieldPlayer>
{
private:
  
  SupportAttacker(){}

public:

  //this is a singleton
  static SupportAttacker* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player);

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};




  
#endif