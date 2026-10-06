//==============================================================================================
//【文件说明】PlayerBase.cpp —— 球员基类的实现(构造/析构 + 各种查询函数)
//
//【这个文件是干什么的?】
//  给出 PlayerBase.h 里声明的函数的具体做法:构造时摆好四边形球员外形并创建转向行为;
//  TrackBall/TrackTarget 让球员脸转向球或目标;isThreatened/InHomeRegion/AtTarget 等
//  一堆"位置判断"函数;FindSupport 负责派消息给合适的队友来支援进攻。
//
//【本文件包含了谁?】
//  自己的 PlayerBase.h,以及 SteeringBehaviors.h、SoccerTeam.h、Goal.h、SoccerBall.h、
//  SoccerPitch.h、Messaging/MessageDispatcher.h(消息分发器)、ParamLoader.h、绘图/几何工具等。
//
//【C++ 小课堂:const_iterator 迭代器 与 全局函数给 std::sort 用】
//  std::vector<PlayerBase*>::const_iterator 是"只读遍历器":像游标一样逐个指向
//  容器里的元素,begin() 指向第一个,end() 指向末尾之后,++ 移到下一个。
//  文件中部两个 SortBy... 是自由函数(不属于任何类),作为"比较谓词"传给 std::sort,
//  决定球员按离对方球门远近排序的方向。
//==============================================================================================
#include "PlayerBase.h"
#include "SteeringBehaviors.h"
#include "2D/Transformations.h"
#include "2D/Geometry.h"
#include "misc/Cgdi.h"
#include "2D/C2DMatrix.h"
#include "Game/Region.h"
#include "ParamLoader.h"
#include "Messaging/MessageDispatcher.h"
#include "SoccerMessages.h"
#include "SoccerTeam.h"
#include "ParamLoader.h"
#include "Goal.h"
#include "SoccerBall.h"
#include "SoccerPitch.h"
#include "Debug/DebugConsole.h"


// using std::vector;:从此处起,本文件里写 vector 就代表 std::vector(省去每次写前缀)。
using std::vector;


//----------------------------- dtor -------------------------------------
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 析构函数 ~PlayerBase:球员对象销毁时自动调用。
// 只做一件事:delete m_pSteering——释放构造时 new 出来的转向行为对象,防内存泄漏。
//--------------------------------------------------------------------------------
PlayerBase::~PlayerBase()
{
  delete m_pSteering;
}

//----------------------------- ctor -------------------------------------
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 构造函数:创建球员。冒号 : 后面是"初始化列表"——逐一把父类 MovingEntity 和
// 本类成员初始化好。home_team->Pitch()->... 是"链式调用":先取球队,再取球场,
// 再按区域编号取区域,再取区域中心作为球员出生点。
//--------------------------------------------------------------------------------
PlayerBase::PlayerBase(SoccerTeam* home_team,
                       int   home_region,
                       Vector2D  heading,
                       Vector2D velocity,
                       double    mass,
                       double    max_force,
                       double    max_speed,
                       double    max_turn_rate,
                       double    scale,
                       player_role role):    

    MovingEntity(home_team->Pitch()->GetRegionFromIndex(home_region)->Center(),
                 scale*10.0,
                 velocity,
                 max_speed,
                 heading,
                 mass,
                 Vector2D(scale,scale),
                 max_turn_rate,
                 max_force),
   m_pTeam(home_team),
   m_dDistSqToBall(MaxFloat),
   m_iHomeRegion(home_region),
   m_iDefaultRegion(home_region),
   m_PlayerRole(role)
{
  
//(原文注释:设置顶点缓冲并计算包围半径)
// 下面 4 个 Vector2D 是球员四边形的四个角点(局部坐标);for 循环逐个存进 m_vecPlayerVB,
// 同时取 x/y 绝对值的最大值作为"包围半径"(碰撞检测用)。abs()=取绝对值。
  //setup the vertex buffers and calculate the bounding radius
  const int NumPlayerVerts = 4;
  const Vector2D player[NumPlayerVerts] = {Vector2D(-3, 8),
                                            Vector2D(3,10),
                                            Vector2D(3,-10),
                                            Vector2D(-3,-8)};

  for (int vtx=0; vtx<NumPlayerVerts; ++vtx)
  {
    m_vecPlayerVB.push_back(player[vtx]);

    //set the bounding radius to the length of the 
    //greatest extent
    if (abs(player[vtx].x) > m_dBoundingRadius)
    {
      m_dBoundingRadius = abs(player[vtx].x);
    }

    if (abs(player[vtx].y) > m_dBoundingRadius)
    {
      m_dBoundingRadius = abs(player[vtx].y);
    }
  }

//(原文注释:创建转向行为对象)
// new SteeringBehaviors(this,...):在堆上新建一个转向行为对象,this=本球员自己,
// 并把它挂到 m_pSteering。new 返回的指针将来在析构里 delete。
  //set up the steering behavior class
  m_pSteering = new SteeringBehaviors(this,
                                      m_pTeam->Pitch(),
                                      Ball());  
  
  //a player's start target is its start position (because it's just waiting)
//(原文注释:球员开局的目标点就是出生位置——因为开球前他只需原地待命)
  m_pSteering->SetTarget(home_team->Pitch()->GetRegionFromIndex(home_region)->Center());
}




//----------------------------- TrackBall --------------------------------
//
//  sets the player's heading to point at the ball
//------------------------------------------------------------------------
// TrackBall:把球员朝向旋转到"脸正对球"。Ball()->Pos() 取球位置。
void PlayerBase::TrackBall()
{
  RotateHeadingToFacePosition(Ball()->Pos());  
}

//----------------------------- TrackTarget --------------------------------
//
//  sets the player's heading to point at the current target
//------------------------------------------------------------------------
// TrackTarget:把球员朝向旋转到"脸正对当前转向目标点"。
// Steering()->Target() - Pos():从自己指向目标的向量;Vec2DNormalize 归一化成方向。
void PlayerBase::TrackTarget()
{
  SetHeading(Vec2DNormalize(Steering()->Target() - Pos()));
}


//------------------------------------------------------------------------
//
//binary predicates for std::sort (see CanPassForward/Backward)
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 两个自由函数:作为 std::sort 的比较谓词(返回 true 表示 p1 应排在 p2 前面)。
// 前者按离对方球门"近→远"排,后者反向。
bool  SortByDistanceToOpponentsGoal(const PlayerBase*const p1,
                                    const PlayerBase*const p2)
{
  return (p1->DistToOppGoal() < p2->DistToOppGoal());
}

bool  SortByReversedDistanceToOpponentsGoal(const PlayerBase*const p1,
                                            const PlayerBase*const p2)
{
  return (p1->DistToOppGoal() > p2->DistToOppGoal());
}


//------------------------- WithinFieldOfView ---------------------------
//
//  returns true if subject is within field of view of this player
//-----------------------------------------------------------------------
// PositionInFrontOfPlayer:判断 position 是否在本球员正前方。
// ToSubject.Dot(Heading())>0 用向量点积:两向量夹角小于 90° 时点积为正=方向大致一致=在前方。
bool PlayerBase::PositionInFrontOfPlayer(Vector2D position)const
{
  Vector2D ToSubject = position - Pos();

  if (ToSubject.Dot(Heading()) > 0) 
    
    return true;

  else

    return false;
}

//------------------------- IsThreatened ---------------------------------
//
//  returns true if there is an opponent within this player's 
//  comfort zone
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// isThreatened:遍历对方全员,若有人既在我前方、又闯进我的舒适区(距离平方 <
// Prm.PlayerComfortZoneSq),说明我被威胁了,立刻 return true。
//--------------------------------------------------------------------------------
bool PlayerBase::isThreatened()const
{
  //check against all opponents to make sure non are within this
  //player's comfort zone
  std::vector<PlayerBase*>::const_iterator curOpp;  
  curOpp = Team()->Opponents()->Members().begin();
 
  for (curOpp; curOpp != Team()->Opponents()->Members().end(); ++curOpp)
  {
    //calculate distance to the player. if dist is less than our
    //comfort zone, and the opponent is infront of the player, return true
    if (PositionInFrontOfPlayer((*curOpp)->Pos()) &&
       (Vec2DDistanceSq(Pos(), (*curOpp)->Pos()) < Prm.PlayerComfortZoneSq))
    {        
      return true;
    }
   
  }// next opp

  return false;
}

//----------------------------- FindSupport -----------------------------------
//
//  determines the player who is closest to the SupportSpot and messages him
//  to tell him to change state to SupportAttacker
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// FindSupport:挑选最合适的接应队友,并用消息分发器 Dispatcher 给他发消息:
// 需要支援就让他切到"支援进攻"状态;换人时给旧接应者发 Msg_GoHome 让他归位。
// Dispatcher->DispatchMsg(...) 是 Common\Messaging 里的全局消息分发器单例。
//--------------------------------------------------------------------------------
void PlayerBase::FindSupport()const
{    
  //if there is no support we need to find a suitable player.
  if (Team()->SupportingPlayer() == NULL)
  {
    PlayerBase* BestSupportPly = Team()->DetermineBestSupportingAttacker();

    Team()->SetSupportingPlayer(BestSupportPly);

    Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                            ID(),
                            Team()->SupportingPlayer()->ID(),
                            Msg_SupportAttacker,
                            NULL);
  }
    
  PlayerBase* BestSupportPly = Team()->DetermineBestSupportingAttacker();
    
  //if the best player available to support the attacker changes, update
  //the pointers and send messages to the relevant players to update their
  //states
  if (BestSupportPly && (BestSupportPly != Team()->SupportingPlayer()))
  {
    
    if (Team()->SupportingPlayer())
    {
      Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                              ID(),
                              Team()->SupportingPlayer()->ID(),
                              Msg_GoHome,
                              NULL);
    }
    
    
    
    Team()->SetSupportingPlayer(BestSupportPly);

    Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                            ID(),
                            Team()->SupportingPlayer()->ID(),
                            Msg_SupportAttacker,
                            NULL);
  }
}


  //calculate distance to opponent's goal. Used frequently by the passing//methods
// 下面是一组短小的查询函数,大多是"距离平方"比较(用平方避免开方,更快):
// DistToOppGoal/DistToHomeGoal 取到对方/己方球门的水平距离(fabs=绝对值)。
double PlayerBase::DistToOppGoal()const
{
  return fabs(Pos().x - Team()->OpponentsGoal()->Center().x);
}

double PlayerBase::DistToHomeGoal()const
{
  return fabs(Pos().x - Team()->HomeGoal()->Center().x);
}

bool PlayerBase::isControllingPlayer()const
{return Team()->ControllingPlayer()==this;}

bool PlayerBase::BallWithinKeeperRange()const
{
  return (Vec2DDistanceSq(Pos(), Ball()->Pos()) < Prm.KeeperInBallRangeSq);
}

bool PlayerBase::BallWithinReceivingRange()const
{
  return (Vec2DDistanceSq(Pos(), Ball()->Pos()) < Prm.BallWithinReceivingRangeSq);
}

bool PlayerBase::BallWithinKickingRange()const
{
  return (Vec2DDistanceSq(Ball()->Pos(), Pos()) < Prm.PlayerKickingDistanceSq);
}


bool PlayerBase::InHomeRegion()const
{
  if (m_PlayerRole == goal_keeper)
  {
    return Pitch()->GetRegionFromIndex(m_iHomeRegion)->Inside(Pos(), Region::normal);
  }
  else
  {
    return Pitch()->GetRegionFromIndex(m_iHomeRegion)->Inside(Pos(), Region::halfsize);
  }
}

bool PlayerBase::AtTarget()const
{
  return (Vec2DDistanceSq(Pos(), Steering()->Target()) < Prm.PlayerInTargetRangeSq);
}

bool PlayerBase::isClosestTeamMemberToBall()const
{
  return Team()->PlayerClosestToBall() == this;
}

bool PlayerBase::isClosestPlayerOnPitchToBall()const
{
  return isClosestTeamMemberToBall() && 
         (DistSqToBall() < Team()->Opponents()->ClosestDistToBallSq());
}

bool PlayerBase::InHotRegion()const
{
  return fabs(Pos().y - Team()->OpponentsGoal()->Center().y ) <
         Pitch()->PlayingArea()->Length()/3.0;
}

bool PlayerBase::isAheadOfAttacker()const
{
  return fabs(Pos().x - Team()->OpponentsGoal()->Center().x) <
         fabs(Team()->ControllingPlayer()->Pos().x - Team()->OpponentsGoal()->Center().x);
}

SoccerBall* const PlayerBase::Ball()const
{
  return Team()->Pitch()->Ball();
}

SoccerPitch* const PlayerBase::Pitch()const
{
  return Team()->Pitch();
}

const Region* const PlayerBase::HomeRegion()const
{
  return Pitch()->GetRegionFromIndex(m_iHomeRegion);
}


