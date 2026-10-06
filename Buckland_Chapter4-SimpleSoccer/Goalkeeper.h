//==============================================================================================
//【文件说明】GoalKeeper.h —— 守门员类(GoalKeeper)
//
//【这个文件是干什么的?】
//  它继承 PlayerBase,表示本方球门前的守门员。同样是个小状态机
//  (StateMachine<GoalKeeper>):主要在"守门(TendGoal)"和"拦截球(Intercept)"间切换。
//  额外维护一个 m_vLookAt 向量:画图时让守门员的脸始终转向球,显得一直在观察球。
//
//【谁在使用这个文件?】
//  SoccerTeam.cpp —— CreatePlayers 里 new 出 1 个 GoalKeeper;
//  GoalKeeperStates.h/.cpp —— 守门员的各个状态类;
//  FieldPlayerStates.cpp / SoccerTeam.cpp —— 球员/球队会问守门员相关位置判断。
//
//【本文件包含了谁?】
//  2D/Vector2D.h、PlayerBase.h、FSM/StateMachine.h。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护:原理详见 Goal.h。(宏名 GOALY_H 是原作者拼写,原样保留)
#ifndef GOALY_H
#define GOALY_H
//------------------------------------------------------------------------
//
//  Name:   GoalKeeper.h
//
//  Desc:   class to implement a goalkeeper agent
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:GoalKeeper.h
//   描述  :实现守门员智能体的类。
//   作者  :Mat Buckland,2003 年(本书作者)
//
//------------------------------------------------------------------------
#include "2D/Vector2D.h"
#include "PlayerBase.h"
#include "FSM/StateMachine.h"

class PlayerBase;




//--------------------------------------------------------------------------------
// class GoalKeeper : public PlayerBase —— 守门员类,公有继承球员基类;
// 额外增加:自己的状态机 m_pStateMachine 与"注视点"向量 m_vLookAt。
//--------------------------------------------------------------------------------
class GoalKeeper : public PlayerBase
{
private:
  
   //an instance of the state machine class
  StateMachine<GoalKeeper>*  m_pStateMachine;
  
  //this vector is updated to point towards the ball and is used when
  //rendering the goalkeeper (instead of the underlaying vehicle's heading)
  //to ensure he always appears to be watching the ball
//(原文注释:该向量始终指向球,渲染守门员时用它(而不是身体朝向),
//           保证守门员看起来一直在盯着球看)
  Vector2D   m_vLookAt;

public:
  
// public 区:构造/析构(析构内联 delete 状态机)、Update/Render/HandleMessage,
// 以及 BallWithinRangeForIntercept/TooFarFromGoalMouth/GetRearInterposeTarget 等判断函数。
   GoalKeeper(SoccerTeam*        home_team,
              int                home_region,
              State<GoalKeeper>* start_state,
              Vector2D           heading,
              Vector2D           velocity,
              double              mass,
              double              max_force,
              double              max_speed,
              double              max_turn_rate,
              double              scale);

   ~GoalKeeper(){delete m_pStateMachine;}

   //these must be implemented
   void        Update();
   void        Render();
   bool        HandleMessage(const Telegram& msg);


   //returns true if the ball comes close enough for the keeper to 
   //consider intercepting
//(原文注释:球靠近到守门员认为可以去拦截时返回 true)
   bool        BallWithinRangeForIntercept()const;

   //returns true if the keeper has ventured too far away from the goalmouth
//(原文注释:守门员离球门太远时返回 true)
   bool        TooFarFromGoalMouth()const;

   //this method is called by the Intercept state to determine the spot
   //along the goalmouth which will act as one of the interpose targets
   //(the other is the ball).
   //the specific point at the goal line that the keeper is trying to cover
   //is flexible and can move depending on where the ball is on the field.
   //To achieve this we just scale the ball's y value by the ratio of the
   //goal width to playingfield width
//(原文注释:Intercept 状态用它定门线上的拦截位置:把球的 y 坐标按
//           "球门宽/场地宽"的比例折算,随球移动选取站位点)
   Vector2D    GetRearInterposeTarget()const;

   StateMachine<GoalKeeper>* GetFSM()const{return m_pStateMachine;}

   
   Vector2D    LookAt()const{return m_vLookAt;}
   void        SetLookAt(Vector2D v){m_vLookAt=v;}
};



#endif