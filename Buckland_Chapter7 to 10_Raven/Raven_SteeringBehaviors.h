//==============================================================================================
//【文件说明】Raven_SteeringBehaviors.h —— 机器人的「转向行为」合集
//
//【这个文件是干什么的?】
//  机器人怎么动?答案就是「转向力(steering force)」。
//  本类把几种经典转向行为封装成函数:
//    Seek(朝目标直冲)、Arrive(减速到目标停下)、Wander(随机闲逛)、
//    Separation(躲开周围同伴)、WallAvoidance(避墙)。
//  每个行为返回一个「期望的力向量」,本类按权重加起来,喂给机器人去移动。
//
//【谁在使用这个文件?】
//  Raven_Bot.h/.cpp —— 每个 bot 持有一个 Raven_Steering,每帧调 Calculate() 拿移动力。
//==============================================================================================
#ifndef STEERINGBEHAVIORS_H
#define STEERINGBEHAVIORS_H
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   Raven_SteeringBehavior.h
//
//  Desc:   class to encapsulate steering behaviors for a Raven_Bot
//(原文注释翻译:封装 Raven_Bot 转向行为的类)
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <vector>
#include <windows.h>
#include <string>
#include <list>
#include "2d/Vector2D.h"
#include "constants.h"

class Raven_Bot;
class Wall2D;
class BaseGameEntity;
class Raven_Game;





//--------------------------- Constants ----------------------------------

//the radius of the constraining circle for the wander behavior
// 下面三个常数是 wander(闲逛)行为的参数:圆半径、圆距机器人多远、每帧抖动量。
//(原文注释:wander 行为里那个约束圆的半径)
const double WanderRad    = 1.2;
//distance the wander circle is projected in front of the agent
const double WanderDist   = 2.0;
//the maximum amount of displacement along the circle each frame
const double WanderJitterPerSec = 40.0;

                                          



//------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// class Raven_Steering —— 转向行为类。
class Raven_Steering
{
public:
  
// 求和方式枚举:加权平均 / 优先级 / 抖动(本工程主要用优先级)。
  enum summing_method{weighted_average, prioritized, dithered};

private:

//--------------------------------------------------------------------------------
// 行为类型枚举:用位标志(0x2/0x8/...)表示,这样可以用一个 int 同时开多个行为。
//   位或 | 打开、异或 ^ 关闭、与 & 查询。
  enum behavior_type
  {
    none               = 0x00000,
    seek               = 0x00002,
    arrive             = 0x00008,
    wander             = 0x00010,
    separation         = 0x00040,
    wall_avoidance     = 0x00200,
  };

private:

  
  //a pointer to the owner of this instance
  Raven_Bot*     m_pRaven_Bot; 
  
  //pointer to the world data
  Raven_Game*    m_pWorld;
  
  //the steering force created by the combined effect of all
//(原文注释:所有选中行为合力算出的最终转向力)
  //the selected behaviors
// m_vSteeringForce:当前合力;m_pTargetAgent1/2:目标 bot(追/逃用)。
  Vector2D       m_vSteeringForce;
 
  //these can be used to keep track of friends, pursuers, or prey
  Raven_Bot*     m_pTargetAgent1;
  Raven_Bot*     m_pTargetAgent2;

  //the current target
  Vector2D    m_vTarget;


  //a vertex buffer to contain the feelers rqd for wall avoidance  
// m_Feelers:避墙时向前伸的几条触角射线;长度由 m_dWallDetectionFeelerLength 定。
//(原文注释:避墙用的「触角」射线顶点缓冲)
  std::vector<Vector2D> m_Feelers;
  
  //the length of the 'feeler/s' used in wall detection
  double                 m_dWallDetectionFeelerLength;


  //the current position on the wander circle the agent is
  //attempting to steer towards
// m_vWanderTarget:闲逛圆上当前瞄准的点;下面三个是 wander 参数。
//(原文注释翻译:wander 圆上当前那个转向目标点)
  Vector2D     m_vWanderTarget; 

  //explained above
  double        m_dWanderJitter;
  double        m_dWanderRadius;
  double        m_dWanderDistance;


  //multipliers. These can be adjusted to effect strength of the  
  //appropriate behavior.
// 下面 5 个 m_dWeight*:各行为权重(分离/闲逛/避墙/seek/arrive)。
//(原文注释翻译:权重倍数;调这些数可以改变对应行为的强度)
  double        m_dWeightSeparation;
  double        m_dWeightWander;
  double        m_dWeightWallAvoidance;
  double        m_dWeightSeek;
  double        m_dWeightArrive;


  //how far the agent can 'see'
// m_dViewDistance:可见距离;m_iFlags:行为开关位标志。
//(原文注释:机器人能「看」多远)
  double        m_dViewDistance;

  //binary flags to indicate whether or not a behavior should be active
  int           m_iFlags;

  
  //Arrive makes use of these to determine how quickly a Raven_Bot
  //should decelerate to its target
// 减速枚举:慢=3/中=2/快=1;m_Deceleration 当前选用哪档。
//(原文注释翻译:Arrive 用它决定减速快慢)
  enum Deceleration{slow = 3, normal = 2, fast = 1};

  //default
  Deceleration m_Deceleration;

  //is cell space partitioning to be used or not?
// m_bCellSpaceOn:是否用格子空间;m_SummingMethod:合力求和方式。
//(原文注释:是否启用格子空间分区加速?)
  bool          m_bCellSpaceOn;
 
  //what type of method is used to sum any active behavior
  summing_method  m_SummingMethod;


  //this function tests if a specific bit of m_iFlags is set
// On():位与测试;AccumulateForce:累加力(不超过最大力);CreateFeelers:画触角。
//(原文注释:测试 m_iFlags 里某一位是否被置上)
  bool      On(behavior_type bt){return (m_iFlags & bt) == bt;}

  bool      AccumulateForce(Vector2D &sf, Vector2D ForceToAdd);

  //creates the antenna utilized by the wall avoidance behavior
  void      CreateFeelers();



   /* .......................................................

                    BEGIN BEHAVIOR DECLARATIONS

      .......................................................*/


  //this behavior moves the agent towards a target position
//--------------------------------------------------------------------------------
// 下面是 5 个行为函数的声明,实现在 .cpp:
//(原文注释:这个行为让机器人朝目标位置直冲)
  Vector2D Seek(const Vector2D &target);

  //this behavior is similar to seek but it attempts to arrive 
  //at the target with a zero velocity
//(原文注释翻译:类似 seek,但到达目标时速度减到 0)
  Vector2D Arrive(const Vector2D    &target,
                  const Deceleration deceleration);

  //this behavior makes the agent wander about randomly
//(原文注释:让机器人随机闲逛)
  Vector2D Wander();

  //this returns a steering force which will keep the agent away from any
  //walls it may encounter
// WallAvoidance:避墙;Separation:躲开周围同伴。
//(原文注释翻译:返回一个转向力,让机器人远离遇到的墙)
  Vector2D WallAvoidance(const std::vector<Wall2D*> &walls);

  
  Vector2D Separation(const std::list<Raven_Bot*> &agents);


    /* .......................................................

                       END BEHAVIOR DECLARATIONS

      .......................................................*/

  //calculates and sums the steering forces from any active behaviors
// CalculatePrioritized:按优先级累加各行为的力。
  Vector2D CalculatePrioritized();

  
public:

// 构造/析构;Calculate() 每帧算合力;ForwardComponent/SideComponent 取前后/侧向分量。
  Raven_Steering(Raven_Game* world, Raven_Bot* agent);

  virtual ~Raven_Steering();

  //calculates and sums the steering forces from any active behaviors
  Vector2D Calculate();

  //calculates the component of the steering force that is parallel
  //with the Raven_Bot heading
// ForwardComponent:前后分量;SideComponent:侧向分量(相对朝向)。
  double    ForwardComponent();

  //calculates the component of the steering force that is perpendicuar
  //with the Raven_Bot heading
  double    SideComponent();


  void      SetTarget(Vector2D t){m_vTarget = t;}
  Vector2D  Target()const{return m_vTarget;}

  void      SetTargetAgent1(Raven_Bot* Agent){m_pTargetAgent1 = Agent;}
  void      SetTargetAgent2(Raven_Bot* Agent){m_pTargetAgent2 = Agent;}


  Vector2D  Force()const{return m_vSteeringForce;}

  void      SetSummingMethod(summing_method sm){m_SummingMethod = sm;}


// 下面一大串 On/Off/IsOn:用位或/异或开关对应行为。
  void SeekOn(){m_iFlags |= seek;}
  void ArriveOn(){m_iFlags |= arrive;}
  void WanderOn(){m_iFlags |= wander;}
  void SeparationOn(){m_iFlags |= separation;}
  void WallAvoidanceOn(){m_iFlags |= wall_avoidance;}

  void SeekOff()  {if(On(seek))   m_iFlags ^=seek;}
  void ArriveOff(){if(On(arrive)) m_iFlags ^=arrive;}
  void WanderOff(){if(On(wander)) m_iFlags ^=wander;}
  void SeparationOff(){if(On(separation)) m_iFlags ^=separation;}
  void WallAvoidanceOff(){if(On(wall_avoidance)) m_iFlags ^=wall_avoidance;}

  bool SeekIsOn(){return On(seek);}
  bool ArriveIsOn(){return On(arrive);}
  bool WanderIsOn(){return On(wander);}
  bool SeparationIsOn(){return On(separation);}
  bool WallAvoidanceIsOn(){return On(wall_avoidance);}

  const std::vector<Vector2D>& GetFeelers()const{return m_Feelers;}
  
  double WanderJitter()const{return m_dWanderJitter;}
  double WanderDistance()const{return m_dWanderDistance;}
  double WanderRadius()const{return m_dWanderRadius;}

  double SeparationWeight()const{return m_dWeightSeparation;}

};




#endif