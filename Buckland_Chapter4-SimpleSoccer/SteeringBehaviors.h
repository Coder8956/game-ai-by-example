//==============================================================================================
//【文件说明】SteeringBehaviors.h —— 球员的"转向行为"大脑(决定往哪走)
//
//【这个文件是干什么的?】
//  一个球员每帧该往哪走,不是写死的,而是由一组"转向行为"叠加算出:
//   seek(直奔目标)、arrive(减速到位)、separation(躲开队友)、
//   pursuit(追球/追未来位置)、interpose(插到对方与球之间拦截)。
//  这些行为用"位开关"同时开几个,本类把它们的力加权求和,得出最终转向力。
//
//【谁在使用这个文件?】
//  PlayerBase.cpp —— 每个球员构造时 new 出一个本类对象,挂在 m_pSteering 上;
//  FieldPlayer.cpp / Goalkeeper.cpp / 各状态文件 —— 用 SeekOn()/ArriveOn() 等开关行为;
//  SteeringBehaviors.cpp —— 各行为的具体实现。
//
//【本文件包含了谁?】
//  <vector>/<windows.h>/<string> —— 标准库与 Windows;
//  "2D/Vector2D.h" —— 2D 向量(转向力都是向量)。
//
//【C++ 小课堂:位标志(bit flags)——用一个整数同时表示多个开关】
//  下面 behavior_type 里 none=0x0000、seek=0x0001、arrive=0x0002、separation=0x0004……
//  注意它们都是"2 的幂"(0x 开头是十六进制),写成二进制后各自只占一位:
//     seek=0001, arrive=0010, separation=0100, pursuit=1000……
//  于是 m_iFlags 这个整数的每一位都当一个独立开关用:
//     m_iFlags |= seek   —— 按位或|,把 seek 那一位"点亮"(行为开启);
//     m_iFlags ^= seek   —— 按位异或^,把该位"翻转"(开变关、关变开);
//     (m_iFlags & bt)==bt —— 按位与&,取出 bt 那一位,判断它是否为 1(是否开启)。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护 + #pragma warning(disable:4786):原理见 Goal.h 与 SoccerPitch.h。
//--------------------------------------------------------------------------------
#ifndef SteeringBehaviorsS_H
#define SteeringBehaviorsS_H
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   SteeringBehaviorss.h
//
//  Desc:   class to encapsulate steering behaviors for a soccer player
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:SteeringBehaviorss.h(原作者这里文件名拼错了,多写一个 s)
//   描述  :封装足球运动员转向行为的类。
//   作者  :Mat Buckland,2002 年(本书作者)
//
//------------------------------------------------------------------------
#include <vector>
#include <windows.h>
#include <string>


#include "2D/Vector2D.h"

// 前置声明:PlayerBase/SoccerPitch/SoccerBall 等只以指针形式出现,先报名字即可。
class PlayerBase;
class SoccerPitch;
class SoccerBall;
class CWall;
class CObstacle;


//---------------------------- class details -----------------------------

//--------------------------------------------------------------------------------
// class SteeringBehaviors —— 转向行为类。它不继承任何类,专门服务一个球员
// (m_pPlayer),每帧把开启的行为力合成出一个最终转向向量。
//--------------------------------------------------------------------------------
class SteeringBehaviors
{
private:
  
  PlayerBase*   m_pPlayer;                                                  

  SoccerBall*   m_pBall;

  //the steering force created by the combined effect of all
  //the selected behaviors
//(原文注释:所有被选中的行为叠加后产生的总转向力)
  Vector2D     m_vSteeringForce;

  //the current target (usually the ball or predicted ball position)
//(原文注释:当前目标点(通常是球或预测的球位置))
  Vector2D     m_vTarget;

  //the distance the player tries to interpose from the target
  double        m_dInterposeDist;

  //multipliers. 
//(原文注释:权重系数)——separation 力的放大倍数 m_dMultSeparation。
  double        m_dMultSeparation;

  //how far it can 'see'
//(原文注释:它能"看到"多远)——感知邻居球员的半径 m_dViewDistance。
  double        m_dViewDistance;


  //binary flags to indicate whether or not a behavior should be active
  int           m_iFlags;

// m_iFlags 是一个整数,用它的每一位当一个行为开关(位标志,原理见文件头小课堂)。
// 下面枚举给每一位起名字:0x0001=seek、0x0002=arrive、0x0004=separation、
// 0x0008=pursuit、0x0010=interpose——每个值只占一个二进制位,互不重叠。
  enum behavior_type
  {
    none               = 0x0000,
    seek               = 0x0001,
    arrive             = 0x0002,
    separation         = 0x0004,
    pursuit            = 0x0008,
    interpose          = 0x0010
  };

  //used by group behaviors to tag neighbours
//(原文注释:群体行为用它给邻居球员打"标记")——本帧视野内的邻居会被 Tag。
  bool         m_bTagged;
  
  //Arrive makes use of these to determine how quickly a vehicle
  //should decelerate to its target
//(原文注释:Arrive 用它决定以多快的速度减速到目标)——slow=3/normal=2/fast=1。
  enum Deceleration{slow = 3, normal = 2, fast = 1};


//--------------------------------------------------------------------------------
// 下面是 private 的五个行为函数,每个返回一个转向向量(实现见 .cpp)。
//--------------------------------------------------------------------------------
  //this behavior moves the agent towards a target position
  Vector2D Seek(Vector2D target);

  //this behavior is similar to seek but it attempts to arrive 
  //at the target with a zero velocity
  Vector2D Arrive(Vector2D target, Deceleration decel);

  //This behavior predicts where its prey will be and seeks
  //to that location
  Vector2D Pursuit(const SoccerBall* ball);
 
  Vector2D Separation();

  //this attempts to steer the agent to a position between the opponent
  //and the object
  Vector2D Interpose(const SoccerBall* ball,
                     Vector2D pos,
                     double    DistFromTarget);


  //finds any neighbours within the view radius
  void      FindNeighbours();


  //this function tests if a specific bit of m_iFlags is set
// On(bt):测试 m_iFlags 的 bt 那一位是否点亮。
// (m_iFlags & bt)==bt:按位与取出该位,等于 bt 本身说明该位为 1(行为已开启)。
  bool      On(behavior_type bt){return (m_iFlags & bt) == bt;}

  bool      AccumulateForce(Vector2D &sf, Vector2D ForceToAdd);

  Vector2D  SumForces();

  //a vertex buffer to contain the feelers rqd for dribbling
  std::vector<Vector2D> m_Antenna;

  
public:

//--------------------------------------------------------------------------------
// public 区:构造函数(绑定本球员/球场/球)、Calculate() 每帧算总转向力,
// 以及下面一组 SeekOn/SeekOff/SeekIsOn 开关与查询函数。
//--------------------------------------------------------------------------------
  SteeringBehaviors(PlayerBase*       agent,
                    SoccerPitch*  world,
                    SoccerBall*   ball);

  virtual ~SteeringBehaviors(){}

 
  Vector2D Calculate();

  //calculates the component of the steering force that is parallel
  //with the vehicle heading
  double    ForwardComponent();

  //calculates the component of the steering force that is perpendicuar
  //with the vehicle heading
  double    SideComponent();

  Vector2D Force()const{return m_vSteeringForce;}

  //renders visual aids and info for seeing how each behavior is
  //calculated
  void      RenderInfo();
  void      RenderAids();

  Vector2D  Target()const{return m_vTarget;}
  void      SetTarget(const Vector2D t){m_vTarget = t;}

  double     InterposeDistance()const{return m_dInterposeDist;}
  void      SetInterposeDistance(double d){m_dInterposeDist = d;}

  bool      Tagged()const{return m_bTagged;}
  void      Tag(){m_bTagged = true;}
  void      UnTag(){m_bTagged = false;}
  

//--------------------------------------------------------------------------------
// 下面 On/Off/IsOn 三组函数:用 |= 点亮开关开启行为,用 ^= 翻转关闭,
// 用 On() 查询是否开启(位运算原理见文件头小课堂)。InterposeOn 还顺带记下距离。
//--------------------------------------------------------------------------------
  void SeekOn(){m_iFlags |= seek;}
  void ArriveOn(){m_iFlags |= arrive;}
  void PursuitOn(){m_iFlags |= pursuit;}
  void SeparationOn(){m_iFlags |= separation;}
  void InterposeOn(double d){m_iFlags |= interpose; m_dInterposeDist = d;}

  
  void SeekOff()  {if(On(seek))   m_iFlags ^=seek;}
  void ArriveOff(){if(On(arrive)) m_iFlags ^=arrive;}
  void PursuitOff(){if(On(pursuit)) m_iFlags ^=pursuit;}
  void SeparationOff(){if(On(separation)) m_iFlags ^=separation;}
  void InterposeOff(){if(On(interpose)) m_iFlags ^=interpose;}


  bool SeekIsOn(){return On(seek);}
  bool ArriveIsOn(){return On(arrive);}
  bool PursuitIsOn(){return On(pursuit);}
  bool SeparationIsOn(){return On(separation);}
  bool InterposeIsOn(){return On(interpose);}

};




#endif