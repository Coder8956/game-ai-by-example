//==============================================================================================
//【文件说明】MovingEntity.h —— 会动的实体基类(带速度/朝向/质量/上限)
//
//【这个文件是干什么的?】
//  在 BaseGameEntity(有 ID、位置)之上,再加"运动学"属性:
//    m_vVelocity   —— 当前速度向量;
//    m_vHeading    —— 朝向(单位向量);m_vSide —— 朝向的垂直向量;
//    m_dMass       —— 质量(决定加速度);
//    m_dMaxSpeed   —— 速度上限;m_dMaxForce —— 推进力上限;m_dMaxTurnRate —— 每秒最大转角。
//  球员、坦克、Raven 的 AI 都从这个类派生,再挂 SteeringBehavior 做移动。
//
//【谁在使用这个文件?】
//  SimpleSoccer 的 PlayerBase、Raven 中的 Raven_Bot、第 5 章寻路的车辆。
//
//【本文件包含了谁?】
//  <cassert>          —— 断言;
//  "2D/Vector2D.h"    —— 向量运算;
//  "Game/BaseGameEntity.h" —— 父类。
//==============================================================================================
#ifndef MOVING_ENTITY
#define MOVING_ENTITY
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//------------------------------------------------------------------------
//
//  Name:   MovingEntity.h
//
//  Desc:   A base class defining an entity that moves. The entity has 
//          a local coordinate system and members for defining its
//          mass and velocity.
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------

#include <cassert>

#include "2D/Vector2D.h"
#include "Game/BaseGameEntity.h"



//--------------------------------------------------------------------------------
// class MovingEntity : public BaseGameEntity —— 运动实体,公有继承实体基类。
//--------------------------------------------------------------------------------
class MovingEntity : public BaseGameEntity
{
protected:
  
//--------------------------------------------------------------------------------
// 运动成员:速度、朝向、侧向(Perp 垂直)、质量、最大速度/力/转角。
//--------------------------------------------------------------------------------
  Vector2D    m_vVelocity;
  
  //a normalized vector pointing in the direction the entity is heading. 
  Vector2D    m_vHeading;

  //a vector perpendicular to the heading vector
  Vector2D    m_vSide; 

  double      m_dMass;
  
  //the maximum speed this entity may travel at.
  double      m_dMaxSpeed;

  //the maximum force this entity can produce to power itself 
  //(think rockets and thrust)
  double      m_dMaxForce;
  
  //the maximum rate (radians per second)this vehicle can rotate         
  double      m_dMaxTurnRate;

public:


//--------------------------------------------------------------------------------
// 构造函数:先调父类构造 BaseGameEntity(GetNextValidID()) 自动分配新 ID,
// 再用初始化列表把上面 7 个运动成员一次设好;函数体只补设位置/半径/缩放。
//--------------------------------------------------------------------------------
  MovingEntity(Vector2D position,
               double   radius,
               Vector2D velocity,
               double   max_speed,
               Vector2D heading,
               double   mass,
               Vector2D scale,
               double   turn_rate,
               double   max_force):BaseGameEntity(BaseGameEntity::GetNextValidID()),
                                  m_vHeading(heading),
                                  m_vVelocity(velocity),
                                  m_dMass(mass),
                                  m_vSide(m_vHeading.Perp()),
                                  m_dMaxSpeed(max_speed),
                                  m_dMaxTurnRate(turn_rate),
                                  m_dMaxForce(max_force)
  {
    m_vPosition = position;
    m_dBoundingRadius = radius; 
    m_vScale = scale;
  }


  virtual ~MovingEntity(){}

  // 下面一堆访问器:Velocity/Mass/Side/MaxSpeed/MaxForce/Heading/MaxTurnRate 等;
  // IsSpeedMaxedOut 判断是否已超速;Speed/SpeedSq 返回速度大小(平方省开方)。
  //accessors
  Vector2D  Velocity()const{return m_vVelocity;}
  void      SetVelocity(const Vector2D& NewVel){m_vVelocity = NewVel;}
  
  double    Mass()const{return m_dMass;}
  
  Vector2D  Side()const{return m_vSide;}

  double    MaxSpeed()const{return m_dMaxSpeed;}                       
  void      SetMaxSpeed(double new_speed){m_dMaxSpeed = new_speed;}
  
  double    MaxForce()const{return m_dMaxForce;}
  void      SetMaxForce(double mf){m_dMaxForce = mf;}

  bool      IsSpeedMaxedOut()const{return m_dMaxSpeed*m_dMaxSpeed >= m_vVelocity.LengthSq();}
  double    Speed()const{return m_vVelocity.Length();}
  double    SpeedSq()const{return m_vVelocity.LengthSq();}
  
  Vector2D  Heading()const{return m_vHeading;}
  void      SetHeading(Vector2D new_heading);
  bool      RotateHeadingToFacePosition(Vector2D target);

  double    MaxTurnRate()const{return m_dMaxTurnRate;}
  void      SetMaxTurnRate(double val){m_dMaxTurnRate = val;}

};


//--------------------------- RotateHeadingToFacePosition ---------------------
//
//  given a target position, this method rotates the entity's heading and
//  side vectors by an amount not greater than m_dMaxTurnRate until it
//  directly faces the target.
//
//  returns true when the heading is facing in the desired direction
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// RotateHeadingToFacePosition:让朝向逐渐转向 target。
//   ① 算目标方向 toTarget 与当前朝向的夹角(点积 dot → acos);
//   ② 转角不超过 m_dMaxTurnRate;
//   ③ 用旋转矩阵 C2DMatrix 把朝向和速度一起转过去,再重算侧向。
//   返回 true 表示已正对目标;false 表示还在转。
//--------------------------------------------------------------------------------
inline bool MovingEntity::RotateHeadingToFacePosition(Vector2D target)
{
  Vector2D toTarget = Vec2DNormalize(target - m_vPosition);

  double dot = m_vHeading.Dot(toTarget);

  //some compilers lose acurracy so the value is clamped to ensure it
  //remains valid for the acos
  Clamp(dot, -1, 1);

  //first determine the angle between the heading vector and the target
  double angle = acos(dot);

  //return true if the player is facing the target
  if (angle < 0.00001) return true;

  //clamp the amount to turn to the max turn rate
  if (angle > m_dMaxTurnRate) angle = m_dMaxTurnRate;
  
  //The next few lines use a rotation matrix to rotate the player's heading
  //vector accordingly
	C2DMatrix RotationMatrix;
  
  //notice how the direction of rotation has to be determined when creating
  //the rotation matrix
	RotationMatrix.Rotate(angle * m_vHeading.Sign(toTarget));	
  RotationMatrix.TransformVector2Ds(m_vHeading);
  RotationMatrix.TransformVector2Ds(m_vVelocity);

  //finally recreate m_vSide
  m_vSide = m_vHeading.Perp();

  return false;
}


//------------------------- SetHeading ----------------------------------------
//
//  first checks that the given heading is not a vector of zero length. If the
//  new heading is valid this fumction sets the entity's heading and side 
//  vectors accordingly
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// SetHeading:设置朝向(断言它是单位向量);侧向必须始终与朝向垂直,同步重算。
//--------------------------------------------------------------------------------
inline void MovingEntity::SetHeading(Vector2D new_heading)
{
  assert( (new_heading.LengthSq() - 1.0) < 0.00001);
  
  m_vHeading = new_heading;

  //the side vector must always be perpendicular to the heading
  m_vSide = m_vHeading.Perp();
}




#endif