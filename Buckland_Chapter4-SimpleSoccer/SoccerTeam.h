//==============================================================================================
//【文件说明】SoccerTeam.h —— 一支足球队(球队级有限状态机)
//
//【这个文件是干什么的?】
//  一支球队 = 5 名球员(4 名场上 + 1 名门将)+ 本方球门 + 对方球门 + 状态机。
//  球队本身也是一个有限状态机(StateMachine<SoccerTeam>):在"开球/进攻/防守"
//  等状态间切换,并统管"谁控球、谁接应、谁接球、谁离球最近"等关键信息。
//  射门 CanShoot、传球 FindPass/IsPassSafe、找接应点 DetermineBestSupportingAttacker
//  这些球队战术决策函数都在本类声明、在 SoccerTeam.cpp 实现。
//
//【谁在使用这个文件?】
//  SoccerPitch.cpp —— new 出红、蓝两支球队;
//  PlayerBase.cpp/.h、FieldPlayer、Goalkeeper、各状态文件、SupportSpotCalculator.cpp
//  —— 球员随时问球队"球在哪/谁控球/我队友是谁"。
//
//【本文件包含了谁?】
//  <vector>、Game/Region.h、SupportSpotCalculator.h、FSM/StateMachine.h。
//
//【C++ 小课堂:StateMachine<SoccerTeam> 模板】
//  StateMachine 是个泛型状态机模板,尖括号里填 SoccerTeam 表示"这是管 SoccerTeam 的状态机";
//  它帮球队记录"当前处于哪个状态",并把每帧 Update 转发给当前状态的 Execute。
//  球队的具体状态类在 TeamStates.h/.cpp(开球/进攻/防守)。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护 + #pragma warning(disable:4786):原理见 Goal.h 与 SoccerPitch.h。
//--------------------------------------------------------------------------------
#ifndef SOCCERTEAM_H
#define SOCCERTEAM_H
#pragma warning (disable:4786)

//------------------------------------------------------------------------
//
//  Name:   SoccerTeam.h
//
//  Desc:   class to define a team of soccer playing agents. A SoccerTeam
//          contains several field players and one goalkeeper. A SoccerTeam
//          is implemented as a finite state machine and has states for
//          attacking, defending, and KickOff.
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:SoccerTeam.h
//   描述  :定义一支由智能体球员组成的足球队。一支球队含若干场上球员和一名守门员;
//          球队实现为有限状态机,有进攻、防守、开球等状态。
//   作者  :Mat Buckland,2003 年(本书作者)
//
//------------------------------------------------------------------------

#include <vector>

#include "Game/Region.h"
#include "SupportSpotCalculator.h"
#include "FSM/StateMachine.h"

// 前置声明:Goal/PlayerBase/FieldPlayer/SoccerPitch/GoalKeeper 等只以指针形式出现。
class Goal;
class PlayerBase;
class FieldPlayer;
class SoccerPitch;
class GoalKeeper;
class SupportSpotCalculator;




                
//--------------------------------------------------------------------------------
// class SoccerTeam —— 球队类。enum team_color{blue,red} 枚举球队颜色。
//--------------------------------------------------------------------------------
class SoccerTeam 
{
public:
  
  enum team_color {blue, red};

private:

   //an instance of the state machine class
//(原文注释:状态机类的一个实例)—— m_pStateMachine 掌管球队当前处于哪个战术状态。
  StateMachine<SoccerTeam>*  m_pStateMachine;

  //the team must know its own color!
//(原文注释:球队必须知道自己的颜色!)
  team_color                m_Color;

  //pointers to the team members
//(原文注释:球队成员指针列表)—— m_Players 是装着 5 名球员指针的 vector。
  std::vector<PlayerBase*>  m_Players;

  //a pointer to the soccer pitch
  SoccerPitch*              m_pPitch;

  //pointers to the goals
//(原文注释:本方球门与对方球门指针)
  Goal*                     m_pOpponentsGoal;
  Goal*                     m_pHomeGoal;
  
  //a pointer to the opposing team
  SoccerTeam*               m_pOpponents;
   
  //pointers to 'key' players
//(原文注释:几个"关键球员"指针)——控球者/接应者/接球者/离球最近者。
  PlayerBase*               m_pControllingPlayer;
  PlayerBase*               m_pSupportingPlayer;
  PlayerBase*               m_pReceivingPlayer;
  PlayerBase*               m_pPlayerClosestToBall;

  //the squared distance the closest player is from the ball
//(原文注释:离球最近的本队球员到球的距离的平方)
  double                     m_dDistSqToBallOfClosestPlayer;

  //players use this to determine strategic positions on the playing field
//(原文注释:球员用它来确定自己在场上的战术位置)——接应甜区计算器。
  SupportSpotCalculator*    m_pSupportSpotCalc;


// private 方法:CreatePlayers() 创建本队全部球员;
// CalculateClosestPlayerToBall() 每帧算出谁离球最近。
  //creates all the players for this team
  void CreatePlayers();

  //called each frame. Sets m_pClosestPlayerToBall to point to the player
  //closest to the ball. 
  void CalculateClosestPlayerToBall();


public:

  SoccerTeam(Goal*        home_goal,
             Goal*        opponents_goal,
             SoccerPitch* pitch,
             team_color   color);

  ~SoccerTeam();

//--------------------------------------------------------------------------------
// public 区:构造/析构,Render()/Update() 每帧调度;
// 下面一大串是球队战术决策函数(射门/传球/安全判断/找接应),实现在 .cpp。
//--------------------------------------------------------------------------------
  //the usual suspects
  void        Render()const;
  void        Update();

  //calling this changes the state of all field players to that of 
  //ReturnToHomeRegion. Mainly used when a goal keeper has
  //possession
  void        ReturnAllFieldPlayersToHome()const;

  //returns true if player has a clean shot at the goal and sets ShotTarget
  //to a normalized vector pointing in the direction the shot should be
  //made. Else returns false and sets heading to a zero vector
  bool        CanShoot(Vector2D  BallPos,
                       double     power, 
                       Vector2D& ShotTarget = Vector2D())const;

  //The best pass is considered to be the pass that cannot be intercepted 
  //by an opponent and that is as far forward of the receiver as possible  
  //If a pass is found, the receiver's address is returned in the 
  //reference, 'receiver' and the position the pass will be made to is 
  //returned in the  reference 'PassTarget'
  bool        FindPass(const PlayerBase*const passer,
                      PlayerBase*&           receiver,
                      Vector2D&              PassTarget,
                      double                  power,
                      double                  MinPassingDistance)const;

  //Three potential passes are calculated. One directly toward the receiver's
  //current position and two that are the tangents from the ball position
  //to the circle of radius 'range' from the receiver.
  //These passes are then tested to see if they can be intercepted by an
  //opponent and to make sure they terminate within the playing area. If
  //all the passes are invalidated the function returns false. Otherwise
  //the function returns the pass that takes the ball closest to the 
  //opponent's goal area.
  bool        GetBestPassToReceiver(const PlayerBase* const passer,
                                    const PlayerBase* const receiver,
                                    Vector2D& PassTarget,
                                    const double power)const;

  //test if a pass from positions 'from' to 'target' kicked with force 
  //'PassingForce'can be intercepted by an opposing player
  bool        isPassSafeFromOpponent(Vector2D    from,
                                     Vector2D    target,
                                     const PlayerBase* const receiver,
                                     const PlayerBase* const opp,
                                     double       PassingForce)const;

  //tests a pass from position 'from' to position 'target' against each member
  //of the opposing team. Returns true if the pass can be made without
  //getting intercepted
  bool        isPassSafeFromAllOpponents(Vector2D from,
                                         Vector2D target,
                                         const PlayerBase* const receiver,
                                         double     PassingForce)const;

  //returns true if there is an opponent within radius of position
  bool        isOpponentWithinRadius(Vector2D pos, double rad);

  //this tests to see if a pass is possible between the requester and
  //the controlling player. If it is possible a message is sent to the
  //controlling player to pass the ball asap.
  void        RequestPass(FieldPlayer* requester)const;

  //calculates the best supporting position and finds the most appropriate
  //attacker to travel to the spot
  PlayerBase* DetermineBestSupportingAttacker();
  

//--------------------------------------------------------------------------------
// 下面是一组内联访问器(直接 return 成员):暴露球员列表、状态机、双方球门、
// 球场、对方球队、球队颜色、关键球员、接应点等。SetControllingPlayer 里还会
// 顺手通知对方球队 LostControl()——"我们控球了,你们丢球权"。
//--------------------------------------------------------------------------------
  const std::vector<PlayerBase*>& Members()const{return m_Players;}  

  StateMachine<SoccerTeam>* GetFSM()const{return m_pStateMachine;}
  
  Goal*const           HomeGoal()const{return m_pHomeGoal;}
  Goal*const           OpponentsGoal()const{return m_pOpponentsGoal;}

  SoccerPitch*const    Pitch()const{return m_pPitch;}           

  SoccerTeam*const     Opponents()const{return m_pOpponents;}
  void                 SetOpponents(SoccerTeam* opps){m_pOpponents = opps;}

  team_color           Color()const{return m_Color;}

  void                 SetPlayerClosestToBall(PlayerBase* plyr){m_pPlayerClosestToBall=plyr;}
  PlayerBase*          PlayerClosestToBall()const{return m_pPlayerClosestToBall;}
  
  double               ClosestDistToBallSq()const{return m_dDistSqToBallOfClosestPlayer;}

  Vector2D             GetSupportSpot()const{return m_pSupportSpotCalc->GetBestSupportingSpot();}

  PlayerBase*          SupportingPlayer()const{return m_pSupportingPlayer;}
  void                 SetSupportingPlayer(PlayerBase* plyr){m_pSupportingPlayer = plyr;}

  PlayerBase*          Receiver()const{return m_pReceivingPlayer;}
  void                 SetReceiver(PlayerBase* plyr){m_pReceivingPlayer = plyr;}

  PlayerBase*          ControllingPlayer()const{return m_pControllingPlayer;}
  void                 SetControllingPlayer(PlayerBase* plyr)
  {
    m_pControllingPlayer = plyr;

    //rub it in the opponents faces!
    Opponents()->LostControl();
  }


  bool  InControl()const{if(m_pControllingPlayer)return true; else return false;}
  void  LostControl(){m_pControllingPlayer = NULL;}

  PlayerBase*  GetPlayerFromID(int id)const;
  

  void SetPlayerHomeRegion(int plyr, int region)const;

  void DetermineBestSupportingPosition()const{m_pSupportSpotCalc->DetermineBestSupportingPosition();}

  void UpdateTargetsOfWaitingPlayers()const;

  //returns false if any of the team are not located within their home region
  bool AllPlayersAtHome()const;

  std::string Name()const{if (m_Color == blue) return "Blue"; return "Red";}

};

#endif