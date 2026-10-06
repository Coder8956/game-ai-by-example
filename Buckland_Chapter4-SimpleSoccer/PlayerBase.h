//==============================================================================================
//【文件说明】PlayerBase.h —— 足球运动员的"基类"(守门员和场上球员的共同祖宗)
//
//【这个文件是干什么的?】
//  场上球员(FieldPlayer)和守门员(Goalkeeper)有大量共同点:都有位置、朝向、速度、
//  所属球队、老家区域、转向行为(SteeringBehaviors)、以及一堆"我在哪/离球多远/
//  是否受威胁"的查询函数。这些共性全部抽进 PlayerBase,两个子类只写各自的差异部分。
//
//【谁在使用这个文件?】
//  FieldPlayer.h / Goalkeeper.h —— 它们都以 ": public PlayerBase" 继承本类;
//  SoccerTeam.h/.cpp、SteeringBehaviors.h/.cpp、各状态文件 —— 把球员当 PlayerBase* 使用。
//
//【本文件包含了谁?】
//  <vector>/<string>/<cassert> —— 标准库;
//  "misc/autolist.h"       —— AutoList 模板(自动把新建球员登记进全局列表);
//  "2D/Vector2D.h"          —— 2D 向量;
//  "Game/MovingEntity.h"    —— Common\Game 里的"运动物体"基类(位置/速度/朝向等)。
//
//【C++ 小课堂:多继承与模板】
//  class PlayerBase : public MovingEntity, public AutoList<PlayerBase>
//  表示一个类同时继承两个父类:MovingEntity(运动物体的物理属性)+ AutoList<...>(自动列表)。
//  AutoList<PlayerBase> 里的 <PlayerBase> 是"模板参数":AutoList 是个泛型模板,
//  尖括号里填什么类型,它就管理哪种对象——这里专管 PlayerBase 指针的全局链表。
//==============================================================================================
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786):关闭老 VC6 的 4786 号警告(详见 SoccerPitch.h 注释)。
//--------------------------------------------------------------------------------
#pragma warning (disable:4786)
#ifndef PLAYERBASE_H
#define PLAYERBASE_H
// 包含保护(include guard):原理详见 Goal.h。
//------------------------------------------------------------------------
//
//  Name: PlayerBase.h
//
//  Desc: Definition of a soccer player base class. The player inherits
//        from the autolist class so that any player created will be 
//        automatically added to a list that is easily accesible by any
//        other game objects. (mainly used by the steering behaviors and
//        player state classes)
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:PlayerBase.h
//   描述  :足球运动员基类的定义。球员继承自 autolist 类,这样任何新建的球员都会
//          自动加入一个全局可访问的列表(主要供转向行为和球员状态类使用)。
//   作者  :Mat Buckland,2003 年(本书作者)
//
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 本文件需要的头文件:标准库(vector/string/cassert)+ 三个工程头文件。
//--------------------------------------------------------------------------------
#include <vector>
#include <string>
#include <cassert>
#include "misc/autolist.h"
#include "2D/Vector2D.h"
#include "Game/MovingEntity.h"

//--------------------------------------------------------------------------------
// 前置声明:下面这些类只以"指针"形式出现在本文件里(如 SoccerTeam* m_pTeam),
// 指针只需知道类名即可,故不必现在 include 它们的完整定义(原理详见 SoccerPitch.h)。
//--------------------------------------------------------------------------------
class SoccerTeam;
class SoccerPitch;
class SoccerBall;
class SteeringBehaviors;
class Region;



//--------------------------------------------------------------------------------
// class PlayerBase —— 球员基类(多继承 MovingEntity 与 AutoList,见文件头小课堂)。
//--------------------------------------------------------------------------------
class PlayerBase : public MovingEntity,
                   public AutoList<PlayerBase>
{

public:
  
// 嵌套枚举 player_role:球员在队里的三种角色——goal_keeper(守门员)、
// attacker(进攻球员)、defender(防守球员)。写在类内部,用 PlayerBase::goal_keeper 引用。
  enum player_role{goal_keeper, attacker, defender};

protected:

  //this player's role in the team
//(原文注释:本球员在队里的角色)下面是 protected(保护)成员——
// protected:介于 private 与 public 之间:本类自己和子类能访问,外界碰不到。
  player_role             m_PlayerRole;

  //a pointer to this player's team
//(原文注释:指向本球员所属球队的指针)
  SoccerTeam*             m_pTeam;
 
  //the steering behaviors
//(原文注释:转向行为对象指针——决定球员"往哪走、怎么走"的大脑)
  SteeringBehaviors*      m_pSteering;

  //the region that this player is assigned to.
//(原文注释:分配给本球员的区域编号 m_iHomeRegion;i 表示 int)
  int                     m_iHomeRegion;

  //the region this player moves to before kickoff
//(原文注释:开球前本球员站位的默认区域编号 m_iDefaultRegion)
  int                     m_iDefaultRegion;

  //the distance to the ball (in squared-space). This value is queried 
  //a lot so it's calculated once each time-step and stored here.
//(原文注释:到球距离的平方。这个值每帧要被查很多次,故每帧只算一次存这里)
  double                   m_dDistSqToBall;

  
  //the vertex buffer
// 顶点缓冲:球员画成一个四边形,下面两个 vector 存"原始顶点"和"旋转平移后的顶点"。
  std::vector<Vector2D>   m_vecPlayerVB;
  //the buffer for the transformed vertices
  std::vector<Vector2D>   m_vecPlayerVBTrans;

public:


//--------------------------------------------------------------------------------
// public 区:构造函数(创建球员时初始化全部状态)+ 下面一堆查询函数。
// 参数依次:所属球队、老家区域编号、朝向、初速度、质量、最大力、最大速度、
// 最大转向角速度、尺寸缩放、角色。实现在 PlayerBase.cpp。
//--------------------------------------------------------------------------------
  PlayerBase(SoccerTeam*    home_team,
             int            home_region,
             Vector2D       heading,
             Vector2D       velocity,
             double          mass,
             double          max_force,
             double          max_speed,
             double          max_turn_rate,
             double          scale,
             player_role    role);

// virtual ~PlayerBase():虚析构函数。基类的析构函数必须加 virtual,
// 这样用基类指针 delete 子类对象时,才会正确调用子类的析构(否则内存泄漏)。
  virtual ~PlayerBase();


  //returns true if there is an opponent within this player's 
  //comfort zone
//(原文注释:若本球员的"舒适区"内有对方球员,则返回 true)
  bool        isThreatened()const;

//--------------------------------------------------------------------------------
// 下面是一组纯查询/动作函数(声明在这,实现在 .cpp),
// 末尾 const 表示"只读函数,保证不修改任何成员"。
//--------------------------------------------------------------------------------
  //rotates the player to face the ball or the player's current target
  void        TrackBall();
  void        TrackTarget();

  //this messages the player that is closest to the supporting spot to
  //change state to support the attacking player
  void        FindSupport()const;

  //returns true if the ball can be grabbed by the goalkeeper
  bool        BallWithinKeeperRange()const;

  //returns true if the ball is within kicking range
  bool        BallWithinKickingRange()const;

  //returns true if a ball comes within range of a receiver
  bool        BallWithinReceivingRange()const;

  //returns true if the player is located within the boundaries 
  //of his home region
  bool        InHomeRegion()const;

  //returns true if this player is ahead of the attacker
  bool        isAheadOfAttacker()const;
  
  //returns true if a player is located at the designated support spot
  bool        AtSupportSpot()const;

  //returns true if the player is located at his steering target
  bool        AtTarget()const;

  //returns true if the player is the closest player in his team to
  //the ball
  bool        isClosestTeamMemberToBall()const;

  //returns true if the point specified by 'position' is located in
  //front of the player
  bool        PositionInFrontOfPlayer(Vector2D position)const;

  //returns true if the player is the closest player on the pitch to the ball
  bool        isClosestPlayerOnPitchToBall()const;

  //returns true if this player is the controlling player
  bool        isControllingPlayer()const;

  //returns true if the player is located in the designated 'hot region' --
  //the area close to the opponent's goal
  bool        InHotRegion()const;

// 下面是一组"访问器"(inline 内联,直接 return 成员):
// 对外只读地暴露角色、到球距离、球队、球场、转向行为等数据。
  player_role Role()const{return m_PlayerRole;}

  double       DistSqToBall()const{return m_dDistSqToBall;}
  void        SetDistSqToBall(double val){m_dDistSqToBall = val;}

  //calculate distance to opponent's/home goal. Used frequently by the passing
  //methods
  double       DistToOppGoal()const;
  double       DistToHomeGoal()const;

  void        SetDefaultHomeRegion(){m_iHomeRegion = m_iDefaultRegion;}

  SoccerBall* const        Ball()const;
  SoccerPitch* const       Pitch()const;
  SteeringBehaviors*const  Steering()const{return m_pSteering;}
  const Region* const      HomeRegion()const;
  void                     SetHomeRegion(int NewRegion){m_iHomeRegion = NewRegion;}
  SoccerTeam*const         Team()const{return m_pTeam;}
  
};





#endif