//==============================================================================================
//【文件说明】FieldPlayer.h —— 场上球员(进攻/防守球员)类
//
//【这个文件是干什么的?】
//  它继承 PlayerBase(球员基类),专门表示能在场上跑动、带球、传球、射门的球员。
//  和基类一样,它也是一个小型状态机(StateMachine<FieldPlayer>):在追球、带球、
//  传球、接应、回位等状态间切换;并带一个"踢球限频器"防止出脚太频繁。
//
//【谁在使用这个文件?】
//  SoccerTeam.cpp —— CreatePlayers 里 new 出 4 个 FieldPlayer;
//  FieldPlayerStates.h/.cpp —— 本球员的各个状态类;
//  SoccerTeam.cpp / PlayerBase.cpp —— 以 FieldPlayer* 调用其方法。
//
//【本文件包含了谁?】
//  <vector>/<string>/<algorithm>/<cassert>、FieldPlayerStates.h、2D/Vector2D.h、
//  FSM/StateMachine.h、PlayerBase.h、time/Regulator.h。
//==============================================================================================
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786) + 包含保护:原理见 SoccerPitch.h 与 Goal.h。
//--------------------------------------------------------------------------------
#pragma warning (disable:4786)
#ifndef FIELDPLAYER_H
#define FIELDPLAYER_H
//------------------------------------------------------------------------
//
//  Name:   FieldPlayer.h
//
//  Desc:   Derived from a PlayerBase, this class encapsulates a player
//          capable of moving around a soccer pitch, kicking, dribbling,
//          shooting etc
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:FieldPlayer.h
//   描述  :派生自 PlayerBase,封装一名能在球场跑动、踢球、盘带、射门的球员。
//   作者  :Mat Buckland,2003 年(本书作者)
//
//------------------------------------------------------------------------
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>

#include "FieldPlayerStates.h"
#include "2D/Vector2D.h"
#include "FSM/StateMachine.h"
#include "PlayerBase.h"
#include "FSM/StateMachine.h"
#include "time/Regulator.h"

class CSteeringBehavior;
class SoccerTeam;
class SoccerPitch;
class Goal;
struct Telegram;


//--------------------------------------------------------------------------------
// class FieldPlayer : public PlayerBase —— 场上球员类,公有继承球员基类;
// 额外增加:自己的状态机 m_pStateMachine 与踢球限频器 m_pKickLimiter。
//--------------------------------------------------------------------------------
class FieldPlayer : public PlayerBase
{
private:

   //an instance of the state machine class
//(原文注释:状态机类的一个实例)——管本球员当前处于哪个场上状态。
  StateMachine<FieldPlayer>*  m_pStateMachine;
  
  //limits the number of kicks a player may take per second
//(原文注释:限制球员每秒最多出脚次数)——isReadyForNextKick() 查是否允许再踢。
  Regulator*                  m_pKickLimiter;

  
public:

// public 区:构造函数(参数含初始状态 start_state)、析构、Update/Render/HandleMessage,
// 以及 GetFSM()(取状态机)、isReadyForNextKick()(能否再踢)两个内联访问器。
  FieldPlayer(SoccerTeam*    home_team,
             int        home_region,
             State<FieldPlayer>* start_state,
             Vector2D  heading,
             Vector2D      velocity,
             double         mass,
             double         max_force,
             double         max_speed,
             double         max_turn_rate,
             double         scale,
             player_role    role);   
  
  ~FieldPlayer();

  //call this to update the player's position and orientation
  void        Update();   

  void        Render();

  bool        HandleMessage(const Telegram& msg);

  StateMachine<FieldPlayer>* GetFSM()const{return m_pStateMachine;}

  bool        isReadyForNextKick()const{return m_pKickLimiter->isReady();}

         
};




#endif