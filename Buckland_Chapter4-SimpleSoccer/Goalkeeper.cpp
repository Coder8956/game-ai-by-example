//==============================================================================================
//【文件说明】GoalKeeper.cpp —— 守门员类的实现
//
//【这个文件是干什么的?】
//  构造守门员并装好状态机;Update() 每帧驱动状态机、算转向力、加速位移、
//  更新朝向,并让"注视点"始终指向球;另提供拦截范围/离门距离/门线站位点等判断。
//
//【本文件包含了谁?】
//  自己的 .h、SteeringBehaviors.h、SoccerTeam.h、SoccerPitch.h、GoalKeeperStates.h、
//  Goal.h、ParamLoader.h、绘图 gdi、2D 变换工具等。
//==============================================================================================
#include "GoalKeeper.h"
#include "misc/Cgdi.h"
#include "SteeringBehaviors.h"
#include "SoccerTeam.h"
#include "SoccerPitch.h"
#include "2D/transformations.h"
#include "GoalKeeperStates.h"
#include "Goal.h"
#include "game/EntityFunctionTemplates.h"
#include "ParamLoader.h"



//----------------------------- ctor ------------------------------------
//-----------------------------------------------------------------------
// 构造函数:先调父类 PlayerBase(角色固定 goal_keeper),再 new 状态机;
// 设当前/上一状态为传入的 start_state,全局状态为 GlobalKeeperState,并调 Enter(this)。
GoalKeeper::GoalKeeper(SoccerTeam*        home_team,
                       int                home_region,
                       State<GoalKeeper>* start_state,
                       Vector2D           heading,
                       Vector2D           velocity,
                       double              mass,
                       double              max_force,
                       double              max_speed,
                       double              max_turn_rate,
                       double              scale): PlayerBase(home_team,
                                                             home_region,
                                                             heading,
                                                             velocity,
                                                             mass,
                                                             max_force,
                                                             max_speed,
                                                             max_turn_rate,
                                                             scale,
                                                             PlayerBase::goal_keeper)
                                         
                                        
{   
   //set up the state machine
  m_pStateMachine = new StateMachine<GoalKeeper>(this);

  m_pStateMachine->SetCurrentState(start_state);
  m_pStateMachine->SetPreviousState(start_state);
  m_pStateMachine->SetGlobalState(GlobalKeeperState::Instance());

  m_pStateMachine->CurrentState()->Enter(this);        
}



//-------------------------- Update --------------------------------------

//--------------------------------------------------------------------------------
// Update 每帧:①状态机决策;②算转向力;③加速度=力÷质量,叠加到速度;④截断最大速度;
// ⑤更新位置;⑥按开关做防穿透;⑦速度非零时把朝向转向运动方向;⑧注视点始终指向球。
//--------------------------------------------------------------------------------
void GoalKeeper::Update()
{ 
  //run the logic for the current state
  m_pStateMachine->Update();

  //calculate the combined force from each steering behavior 
  Vector2D SteeringForce = m_pSteering->Calculate();



  //Acceleration = Force/Mass
  Vector2D Acceleration = SteeringForce / m_dMass;

  //update velocity
  m_vVelocity += Acceleration;

  //make sure player does not exceed maximum velocity
  m_vVelocity.Truncate(m_dMaxSpeed);

  //update the position
  m_vPosition += m_vVelocity;


  //enforce a non-penetration constraint if desired
  if(Prm.bNonPenetrationConstraint)
  {
    EnforceNonPenetrationContraint(this, AutoList<PlayerBase>::GetAllMembers());
  }

  //update the heading if the player has a non zero velocity
  if ( !m_vVelocity.isZero())
  {    
    m_vHeading = Vec2DNormalize(m_vVelocity);

    m_vSide = m_vHeading.Perp();
  }

  //look-at vector always points toward the ball
  if (!Pitch()->GoalKeeperHasBall())
  {
   m_vLookAt = Vec2DNormalize(Ball()->Pos() - Pos());
  }
}


// 下面三个是守门员的位置判断:球离球门中心多近可去拦截;离门线站位点是否太远;
// GetRearInterposeTarget 按球的 y 坐标折算出门线上该守的位置点。
bool GoalKeeper::BallWithinRangeForIntercept()const
{
  return (Vec2DDistanceSq(Team()->HomeGoal()->Center(), Ball()->Pos()) <=
          Prm.GoalKeeperInterceptRangeSq);
}

bool GoalKeeper::TooFarFromGoalMouth()const
{
  return (Vec2DDistanceSq(Pos(), GetRearInterposeTarget()) >
          Prm.GoalKeeperInterceptRangeSq);
}

Vector2D GoalKeeper::GetRearInterposeTarget()const
{
  double xPosTarget = Team()->HomeGoal()->Center().x;

  double yPosTarget = Pitch()->PlayingArea()->Center().y - 
                     Prm.GoalWidth*0.5 + (Ball()->Pos().y*Prm.GoalWidth) /
                     Pitch()->PlayingArea()->Height();

  return Vector2D(xPosTarget, yPosTarget); 
}

//-------------------- HandleMessage -------------------------------------
//
//  routes any messages appropriately
//------------------------------------------------------------------------
// HandleMessage:把消息转发给守门员状态机处理。
bool GoalKeeper::HandleMessage(const Telegram& msg)
{
  return m_pStateMachine->HandleMessage(msg);
}

//--------------------------- Render -------------------------------------
//
//------------------------------------------------------------------------
// Render:按球队颜色选画笔;用注视点 m_vLookAt(而非身体朝向)把守门员四边形画出来,
// 再画圆头;按开关显示编号与当前状态名。
void GoalKeeper::Render()                                         
{
  if (Team()->Color() == SoccerTeam::blue) 
    gdi->BluePen();
  else 
    gdi->RedPen();
  
  m_vecPlayerVBTrans = WorldTransform(m_vecPlayerVB,
                                       Pos(),
                                       m_vLookAt,
                                       m_vLookAt.Perp(),
                                       Scale());

  gdi->ClosedShape(m_vecPlayerVBTrans);
  
  //draw the head
  gdi->BrownBrush();
  gdi->Circle(Pos(), 6);

  //draw the ID
  if (Prm.bIDs)
  {
    gdi->TextColor(0, 170, 0);;
    gdi->TextAtPos(Pos().x-20, Pos().y-20, ttos(ID()));
  }

  //draw the state
  if (Prm.bStates)
  { 
    gdi->TextColor(0, 170, 0); 
    gdi->TransparentText(); 
    gdi->TextAtPos(m_vPosition.x, m_vPosition.y -20, std::string(m_pStateMachine->GetNameOfCurrentState()));
  }
}