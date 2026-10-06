//==============================================================================================
//【文件说明】SteeringBehaviors.cpp —— 转向行为各算法的实现
//
//【这个文件是干什么的?】
//  实现 Seek(直奔)/Arrive(减速到)/Pursuit(追球)/Separation(躲队友)/Interpose(拦截)
//  这几个行为,以及 Calculate() 总调度:把当前开启的行为力依次累加,直到达到最大力。
//
//【本文件包含了谁?】
//  自己的 .h,以及 PlayerBase.h、SoccerTeam.h、SoccerBall.h、ParamLoader.h、
//  misc/autolist.h(全局球员列表)、misc/utils.h、2D/Transformations.h。
//==============================================================================================
#include "SteeringBehaviors.h"
#include "PlayerBase.h"
#include "2D/Transformations.h"
#include "misc/utils.h"
#include "SoccerTeam.h"
#include "misc/autolist.h"
#include "ParamLoader.h"
#include "SoccerBall.h"


using std::string;
using std::vector;

//------------------------- ctor -----------------------------------------
//
//------------------------------------------------------------------------
// 构造函数:绑定所属球员 agent、球场 world、球 ball;m_iFlags(位开关)初始为 0(全关);
// m_dMultSeparation/m_dViewDistance 直接取参数 Prm;最后的 m_Antenna(5,Vector2D())
// 是给转向向量缓冲预建 5 个零向量元素。
SteeringBehaviors::SteeringBehaviors(PlayerBase*  agent,
                                     SoccerPitch* world,
                                     SoccerBall*  ball):
                                  
             m_pPlayer(agent),
             m_iFlags(0),
             m_dMultSeparation(Prm.SeparationCoefficient),
             m_bTagged(false),
             m_dViewDistance(Prm.ViewDistance),
             m_pBall(ball),
             m_dInterposeDist(0.0),
             m_Antenna(5,Vector2D())
{
}

//--------------------- AccumulateForce ----------------------------------
//
//  This function calculates how much of its max steering force the 
//  vehicle has left to apply and then applies that amount of the
//  force to add.
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// AccumulateForce:"力的累加器"。球员每帧的总转向力不能超过 MaxForce;
// 本函数先算还剩多少额度,再把本次行为力按剩余额度截断后加进去;
// 额度用完(<=0)就返回 false,通知调用者别再加了。sf 是引用传递(直接改它)。
//--------------------------------------------------------------------------------
bool SteeringBehaviors::AccumulateForce(Vector2D &sf, Vector2D ForceToAdd)
{
  //first calculate how much steering force we have left to use
  double MagnitudeSoFar = sf.Length();

  double magnitudeRemaining = m_pPlayer->MaxForce() - MagnitudeSoFar;

  //return false if there is no more force left to use
  if (magnitudeRemaining <= 0.0) return false;

  //calculate the magnitude of the force we want to add
  double MagnitudeToAdd = ForceToAdd.Length();
  
  //now calculate how much of the force we can really add  
  if (MagnitudeToAdd > magnitudeRemaining)
  {
    MagnitudeToAdd = magnitudeRemaining;
  }

  //add it to the steering force
  sf += (Vec2DNormalize(ForceToAdd) * MagnitudeToAdd); 
  
  return true;
}

//---------------------- Calculate ---------------------------------------
//
//  calculates the overall steering force based on the currently active
//  steering behaviors. 
//------------------------------------------------------------------------
// Calculate:每帧入口。先清零,再调 SumForces() 叠加各行为力,
// 最后 Truncate(MaxForce) 截断到最大力,返回最终转向力。
Vector2D SteeringBehaviors::Calculate()
{                                                                         
  //reset the force
  m_vSteeringForce.Zero();

  //this will hold the value of each individual steering force
  m_vSteeringForce = SumForces();

  //make sure the force doesn't exceed the vehicles maximum allowable
  m_vSteeringForce.Truncate(m_pPlayer->MaxForce());

  return m_vSteeringForce;
}

//-------------------------- SumForces -----------------------------------
//
//  this method calls each active steering behavior and acumulates their
//  forces until the max steering force magnitude is reached at which
//  time the function returns the steering force accumulated to that 
//  point
//------------------------------------------------------------------------
// SumForces:依次检查每个行为开关是否开启(On(...)),开了就把它的力累加进总力;
// 一旦 AccumulateForce 返回 false(力额度用尽),立刻提前返回,不再加后面的行为。
Vector2D SteeringBehaviors::SumForces()
{
   Vector2D force;
  
  //the soccer players must always tag their neighbors
   FindNeighbours();

  if (On(separation))
  {
    force += Separation() * m_dMultSeparation;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }    

  if (On(seek))
  {
    force += Seek(m_vTarget);

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(arrive))
  {
    force += Arrive(m_vTarget, fast);

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(pursuit))
  {
    force += Pursuit(m_pBall);

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(interpose))
  {
    force += Interpose(m_pBall, m_vTarget, m_dInterposeDist);

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  return m_vSteeringForce;
}

//------------------------- ForwardComponent -----------------------------
//
//  calculates the forward component of the steering force
//------------------------------------------------------------------------
// ForwardComponent/SideComponent:把总转向力投影到球员"前进方向"和"侧面方向"上,
// 得到向前多大、侧移多大两个标量,供球员移动时分解使用(点积 Dot 即投影)。
double SteeringBehaviors::ForwardComponent()
{
  return m_pPlayer->Heading().Dot(m_vSteeringForce);
}

//--------------------------- SideComponent ------------------------------
//
//  //  calculates the side component of the steering force
//------------------------------------------------------------------------
double SteeringBehaviors::SideComponent()
{
  return m_pPlayer->Side().Dot(m_vSteeringForce) * m_pPlayer->MaxTurnRate();
}


//------------------------------- Seek -----------------------------------
//
//  Given a target, this behavior returns a steering force which will
//  allign the agent with the target and move the agent in the desired
//  direction
//------------------------------------------------------------------------
// Seek:直奔目标。期望速度=指向目标的单位向量×最大速度;
// 返回"期望速度 - 当前速度"=需要施加的转向力。
Vector2D SteeringBehaviors::Seek(Vector2D target)
{
 
  Vector2D DesiredVelocity = Vec2DNormalize(target - m_pPlayer->Pos())
                            * m_pPlayer->MaxSpeed();

  return (DesiredVelocity - m_pPlayer->Velocity());
}


//--------------------------- Arrive -------------------------------------
//
//  This behavior is similar to seek but it attempts to arrive at the
//  target with a zero velocity
//------------------------------------------------------------------------
// Arrive:减速到位。离目标远时全速,越近越慢,使得到达时速度趋近 0;
// speed=距离÷(减减速档×微调系数),再用 min() 限制不超过最大速度。
Vector2D SteeringBehaviors::Arrive(Vector2D    target,
                                   Deceleration deceleration)
{
  Vector2D ToTarget = target - m_pPlayer->Pos();

  //calculate the distance to the target
  double dist = ToTarget.Length();

  if (dist > 0)
  {
    //because Deceleration is enumerated as an int, this value is required
    //to provide fine tweaking of the deceleration..
    const double DecelerationTweaker = 0.3;

    //calculate the speed required to reach the target given the desired
    //deceleration
    double speed =  dist / ((double)deceleration * DecelerationTweaker);                    

    //make sure the velocity does not exceed the max
    speed = min(speed, m_pPlayer->MaxSpeed());

    //from here proceed just like Seek except we don't need to normalize 
    //the ToTarget vector because we have already gone to the trouble
    //of calculating its length: dist. 
    Vector2D DesiredVelocity =  ToTarget * speed / dist;

    return (DesiredVelocity - m_pPlayer->Velocity());
  }

  return Vector2D(0,0);
}


//------------------------------ Pursuit ---------------------------------
//
//  this behavior creates a force that steers the agent towards the 
//  ball
//------------------------------------------------------------------------
// Pursuit:追球。根据"我离球多远÷球速"估算前瞻时间,
// 用 ball->FuturePosition() 算出球未来会到哪,再对那个预测点做 Arrive。
Vector2D SteeringBehaviors::Pursuit(const SoccerBall* ball)
{
  Vector2D ToBall = ball->Pos() - m_pPlayer->Pos();
 
  //the lookahead time is proportional to the distance between the ball
  //and the pursuer; 
  double LookAheadTime = 0.0;

  if (ball->Speed() != 0.0)
  {
    LookAheadTime = ToBall.Length() / ball->Speed();
  }

  //calculate where the ball will be at this time in the future
  m_vTarget = ball->FuturePosition(LookAheadTime);

  //now seek to the predicted future position of the ball
  return Arrive(m_vTarget, fast);
}


//-------------------------- FindNeighbours ------------------------------
//
//  tags any vehicles within a predefined radius
//------------------------------------------------------------------------
// FindNeighbours:遍历全局球员列表 AutoList<PlayerBase>::GetAllMembers();
// 先把每个人的标记清掉(UnTag),再把距离平方小于视野半径平方的人打上 Tag。
// list 是 std::list(链表),用迭代器 curPlyr 逐个遍历。
void SteeringBehaviors::FindNeighbours()
{
  std::list<PlayerBase*>& AllPlayers = AutoList<PlayerBase>::GetAllMembers();
  std::list<PlayerBase*>::iterator curPlyr;
  for (curPlyr = AllPlayers.begin(); curPlyr!=AllPlayers.end(); ++curPlyr)
  {
    //first clear any current tag
    (*curPlyr)->Steering()->UnTag();

    //work in distance squared to avoid sqrts
    Vector2D to = (*curPlyr)->Pos() - m_pPlayer->Pos();

    if (to.LengthSq() < (m_dViewDistance * m_dViewDistance))
    {
      (*curPlyr)->Steering()->Tag();
    }
  }//next
}


//---------------------------- Separation --------------------------------
//
// this calculates a force repelling from the other neighbors
//------------------------------------------------------------------------
// Separation:躲开邻居。对每个被 Tag 的邻居,沿"从它指向我"的方向推一把,
// 且距离越近推力越大(单位向量÷距离),避免球员叠在一起。
Vector2D SteeringBehaviors::Separation()
{  
   //iterate through all the neighbors and calculate the vector from the
  Vector2D SteeringForce;
  
  std::list<PlayerBase*>& AllPlayers = AutoList<PlayerBase>::GetAllMembers();
  std::list<PlayerBase*>::iterator curPlyr;
  for (curPlyr = AllPlayers.begin(); curPlyr!=AllPlayers.end(); ++curPlyr)
  {
    //make sure this agent isn't included in the calculations and that
    //the agent is close enough
    if((*curPlyr != m_pPlayer) && (*curPlyr)->Steering()->Tagged())
    {
      Vector2D ToAgent = m_pPlayer->Pos() - (*curPlyr)->Pos();

      //scale the force inversely proportional to the agents distance  
      //from its neighbor.
      SteeringForce += Vec2DNormalize(ToAgent)/ToAgent.Length();
    }
  }

  return SteeringForce;
}

  
//--------------------------- Interpose ----------------------------------
//
//  Given an opponent and an object position this method returns a 
//  force that attempts to position the agent between them
//------------------------------------------------------------------------
// Interpose:拦截。在"球与目标点之间"、距目标 DistFromTarget 处找个位置,再 Arrive 过去;
// 守门员回门线、挡球时用它。
Vector2D SteeringBehaviors::Interpose(const SoccerBall* ball,
                                      Vector2D  target,
                                      double     DistFromTarget)
{
  return Arrive(target + Vec2DNormalize(ball->Pos() - target) * 
                DistFromTarget, normal);
}


//----------------------------- RenderAids -------------------------------
//
//------------------------------------------------------------------------
// RenderAids:调试可视化——用红笔把当前转向力画成一条箭头线(菜单开关控制是否显示)。
void SteeringBehaviors::RenderAids( )
{ 
  //render the steering force
  gdi->RedPen();

  gdi->Line(m_pPlayer->Pos(), m_pPlayer->Pos() + m_vSteeringForce * 20);


  
}


