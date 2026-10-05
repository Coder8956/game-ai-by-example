//==============================================================================================
//【文件说明】MovingEntity.h —— "会动的实体"基类:机器人(Vehicle)的父类
//
//【这个文件是干什么的?】
//  在 BaseGameEntity(位置/编号/半径)之上,再加一层"运动属性":
//    速度 velocity、朝向 heading、侧向 side、质量 mass、
//    最大速度 max_speed、最大推力 max_force、最大转向速率 max_turn_rate。
//  并提供了"原地转身面向某个目标"的能力(转向受最大转向速率限制)。
//  有了这些,子类 Vehicle 才谈得上"按转向力移动"。
//
//【与相关文件的关系】
//  BaseGameEntity.h —— 父类:位置 m_vPos 等身体数据继承自它;
//  2D/Vector2D.h    —— 所有向量运算(Perp 垂直、Dot 点积、Length 长度、
//                       Vec2DNormalize 归一化等)来自 Common 目录;
//  <cassert>        —— 断言工具(SetHeading 校验用);
//  Vehicle.h/.cpp   —— 继承本类,再添加"转向行为控制器"和"朝向平滑器";
//  SteeringBehaviors.cpp —— 大量调用 Heading()/Side()/Velocity() 等接口。
//
//【C++ 小课堂:朝向 heading 与侧向 side】
//  m_vHeading —— "车头方向":单位长度(长度为 1)的向量,表示面朝哪;
//  m_vSide    —— "车身侧向":与车头垂直(Perp)的单位向量,表示左右方向。
//  这两个向量组成物体的"本地坐标系",计算转向力时非常有用。
//==============================================================================================
#ifndef MOVING_ENTITY
#define MOVING_ENTITY
//------------------------------------------------------------------------
//
//  Name:   MovingEntity.h
//
//  Desc:   A base class defining an entity that moves. The entity has 
//          a local coordinate system and members for defining its
//          mass and velocity.
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// 包含断言工具 <cassert>:提供 assert(条件) 宏,条件为假时程序中断报错。
#include <cassert>

// 包含 2D 向量工具(Common 目录):Vector2D 及 Perp/Dot/Normalize 等运算。
// "2D/xxx.h" 表示"附加包含目录 Common 下的 2D 子文件夹"。
#include "2D/Vector2D.h"
// 包含父类"游戏实体基类":本类要继承它,必须先见到它的完整定义。
#include "BaseGameEntity.h"



//【类定义开始】class MovingEntity : public BaseGameEntity
//  公有继承 BaseGameEntity:自动拥有位置、编号、半径等全部基类能力,
//  再添加"会动"的属性。
class MovingEntity : public BaseGameEntity
{
// protected: —— 保护区:这些成员子类(Vehicle)可以直接访问,外界不能。
// 速度/朝向是子类每帧都要改的"身体数据",放这里最合适。
protected:
  
// 速度向量 m_vVelocity:带大小和方向的移动速度(单位:像素/秒)。
// m_ 前缀 = member(成员),v = vector(向量)。
  Vector2D    m_vVelocity;
  
// ↓↓↓ 原文注释翻译:一个"归一化"(长度=1)的向量,指向实体当前朝向的方向。
  //a normalized vector pointing in the direction the entity is heading. 
// 朝向向量 m_vHeading:只表方向、不带速度信息(所以归一化成单位长度)。
  Vector2D    m_vHeading;

// ↓↓↓ 原文注释翻译:一个与朝向向量垂直的向量(即"车身侧向")。
  //a vector perpendicular to the heading vector
// 侧向 m_vSide:与车头垂直,指"右边"。Perp() 返回垂直向量。
  Vector2D    m_vSide; 

// 质量 m_dMass(单位随意,本工程设为 1.0)。牛顿第二定律 F=ma 要用它:
// 加速度 = 力 ÷ 质量(见 Vehicle.cpp 的 Update)。
  double       m_dMass;
  
// ↓↓↓ 原文注释翻译:该实体允许达到的最大速度。
  //the maximum speed this entity may travel at.
// 最大速度 m_dMaxSpeed:速度向量超过它时会被截断(Truncate)。
  double       m_dMaxSpeed;

// ↓↓↓ 原文注释翻译:该实体能产生的最大推力(想想火箭和喷气推进)。
  //the maximum force this entity can produce to power itself 
  //(think rockets and thrust)
// 最大推力 m_dMaxForce:所有转向行为的合力不能超过它——模拟"引擎极限"。
  double        m_dMaxForce;
  
// ↓↓↓ 原文注释翻译:该实体每秒最大可旋转的速率(单位:弧度/秒)。
  //the maximum rate (radians per second)this vehicle can rotate         
// 最大转向速率 m_dMaxTurnRate:转弯"急不得",一帧最多转这么多弧度。
  double       m_dMaxTurnRate;

// public: —— 公开区:下面是对外接口。
public:


//--------------------------------------------------------------------------------
//【构造函数:一次性设置全部运动属性】
//  参数很多,依次是:出生位置、半径、初始速度、最大速度、初始朝向、
//  质量、缩放、最大转向速率、最大推力。
//  冒号后初始化列表逐项赋值,要点:
//   BaseGameEntity(0, position, radius) —— 先初始化基类:类型 0、位置、半径;
//   m_vSide(m_vHeading.Perp()) —— 侧向由朝向"垂直"得到,所以必须先有朝向;
//   m_vScale = scale(函数体内) —— 缩放经基类 SetScale 的路径赋值,保证
//     包围半径同步更新(原理见 BaseGameEntity.h 的 SetScale 注释)。
//--------------------------------------------------------------------------------
  MovingEntity(Vector2D position,
               double    radius,
               Vector2D velocity,
               double    max_speed,
               Vector2D heading,
               double    mass,
               Vector2D scale,
               double    turn_rate,
               double    max_force):BaseGameEntity(0, position, radius),
                                  m_vHeading(heading),
                                  m_vVelocity(velocity),
                                  m_dMass(mass),
                                  m_vSide(m_vHeading.Perp()),
                                  m_dMaxSpeed(max_speed),
                                  m_dMaxTurnRate(turn_rate),
                                  m_dMaxForce(max_force)
  {
    m_vScale = scale;
  }

// 虚析构:空的(本类无资源要清理),virtual 保证多态删除安全(见基类注释)。
  virtual ~MovingEntity(){}

// ↓↓↓ 原文注释翻译:访问器(accessors)——读写成员的小函数。
  //accessors
// 读速度向量。const 结尾:不修改成员。
  Vector2D  Velocity()const{return m_vVelocity;}
// 设置速度。const Vector2D& NewVel —— 只读借用(别名),避免拷贝开销。
  void      SetVelocity(const Vector2D& NewVel){m_vVelocity = NewVel;}
  
// 读质量。
  double     Mass()const{return m_dMass;}
  
// 读侧向向量(与车头垂直的方向)。
  Vector2D  Side()const{return m_vSide;}

// 读最大速度。
  double     MaxSpeed()const{return m_dMaxSpeed;}                       
// 设置最大速度(调试时按键盘可调)。
  void      SetMaxSpeed(double new_speed){m_dMaxSpeed = new_speed;}
  
// 读最大推力。
  double     MaxForce()const{return m_dMaxForce;}
// 设置最大推力(调试时按 Insert/Delete 可调)。
  void      SetMaxForce(double mf){m_dMaxForce = mf;}

// 判断"速度是否已达上限":用"速度平方 ≥ 最大速度平方"比较——
// 避免开平方(sqrt),省计算。LengthSq() = 向量长度的平方。
  bool      IsSpeedMaxedOut()const{return m_dMaxSpeed*m_dMaxSpeed >= m_vVelocity.LengthSq();}
// 读当前速度大小(调用 Length() 开平方)。
  double     Speed()const{return m_vVelocity.Length();}
// 读当前速度大小的平方(避免开平方的版本,比较大小用)。
  double     SpeedSq()const{return m_vVelocity.LengthSq();}
  
// 读朝向向量(单位向量)。
  Vector2D  Heading()const{return m_vHeading;}
// 声明:设置朝向向量(实现在本文件下方,见 SetHeading 注释)。
  void      SetHeading(Vector2D new_heading);
// 声明:旋转身体朝向目标位置(实现在本文件下方)。
  bool      RotateHeadingToFacePosition(Vector2D target);

// 读最大转向速率。
  double     MaxTurnRate()const{return m_dMaxTurnRate;}
// 设置最大转向速率。
  void      SetMaxTurnRate(double val){m_dMaxTurnRate = val;}

};


//--------------------------------------------------------------------------------
//【RotateHeadingToFacePosition:原地转身,面向某个目标点】
//  给定一个目标位置,本函数把实体的朝向和侧向向量旋转一个"不超过最大转向
//  速率"的角度,直到正对目标。当朝向已经指向目标方向时返回 true。
//  (本函数以 inline 形式写在头文件里,因为会被每帧频繁调用。)
//--------------------------------------------------------------------------------
//--------------------------- RotateHeadingToFacePosition ---------------------
//
//  given a target position, this method rotates the entity's heading and
//  side vectors by an amount not greater than m_dMaxTurnRate until it
//  directly faces the target.
//
//  returns true when the heading is facing in the desired direction
//-----------------------------------------------------------------------------
// 从自己位置指向目标的向量,归一化成单位向量 toTarget(只取方向)。
// Vec2DNormalize(向量) = 把向量长度缩成 1、方向不变。
inline bool MovingEntity::RotateHeadingToFacePosition(Vector2D target)
{
// 目标方向 toTarget:指向目标点的单位向量。
  Vector2D toTarget = Vec2DNormalize(target - m_vPos);

  //first determine the angle between the heading vector and the target
// 求夹角:acos(反余弦)作用在"朝向·目标方向"的点积上。
// m_vHeading.Dot(toTarget) —— 点积(两个单位向量的点积 = 夹角余弦值);
// acos(余弦值) —— 反推回夹角(弧度)。
  double angle = acos(m_vHeading.Dot(toTarget));

  //return true if the player is facing the target
// 夹角已经小于 0.00001 弧度(≈0.0006°):基本正对目标,直接返回 true。
// (不转向,省得做无用的旋转计算。)
  if (angle < 0.00001) return true;

  //clamp the amount to turn to the max turn rate
// 夹角超过最大转向速率:一次最多转这么多(限幅,clamp)。
// 因为参数是 double,按值传递,这里改了不影响调用者。
  if (angle > m_dMaxTurnRate) angle = m_dMaxTurnRate;
  
// ↓↓↓ 原文注释翻译:接下来几行用"旋转矩阵"来旋转朝向向量。
  //The next few lines use a rotation matrix to rotate the player's heading
  //vector accordingly
// 创建 2D 旋转矩阵对象 C2DMatrix(来自 Common 目录 2D/C2DMatrix.h)。
// 注意行首是制表符,原样保留。
	C2DMatrix RotationMatrix;
  
// ↓↓↓ 原文注释翻译:注意,创建旋转矩阵时必须确定旋转方向(顺时针/逆时针)。
// m_vHeading.Sign(toTarget) —— 判断目标在"朝向的哪一侧",返回 +1 或 -1,
//   决定按正角度还是负角度转(转对方向才抄近路)。
  //notice how the direction of rotation has to be determined when creating
  //the rotation matrix
// 旋转矩阵按"角度×方向符号"旋转:angle * ±1 决定转多少、往哪转。
	RotationMatrix.Rotate(angle * m_vHeading.Sign(toTarget));	
// 用旋转矩阵变换朝向向量 m_vHeading(原地旋转它)。
// TransformVector2Ds(向量) = 让矩阵作用在这个向量上。
  RotationMatrix.TransformVector2Ds(m_vHeading);
// 顺便也旋转速度向量 m_vVelocity:车头转向,速度方向也跟着转。
  RotationMatrix.TransformVector2Ds(m_vVelocity);

// ↓↓↓ 原文注释翻译:最后重新生成侧向向量 m_vSide。
  //finally recreate m_vSide
// 侧向永远 = 朝向的垂直方向(Perp)。朝向变完,侧向要重新算。
  m_vSide = m_vHeading.Perp();

// 返回 false:表示"还没完全正对目标,下一帧还要继续转"。
  return false;
}


//--------------------------------------------------------------------------------
//【SetHeading:直接设置朝向向量】
//  先检查给定的新朝向不是零长度向量;若合法,就设置朝向并同步更新侧向。
//  (原注释中 fumction 是 function 的笔误,不影响理解。)
//--------------------------------------------------------------------------------
//------------------------- SetHeading ----------------------------------------
//
//  first checks that the given heading is not a vector of zero length. If the
//  new heading is valid this fumction sets the entity's heading and side 
//  vectors accordingly
//-----------------------------------------------------------------------------
inline void MovingEntity::SetHeading(Vector2D new_heading)
{
// assert:校验新朝向是"单位长度"——(长度平方 - 1) 必须接近 0(< 0.00001)。
// 因为朝向必须归一化,否则后续几何运算(点积、Perp)会出错。
// LengthSq() 返回向量长度的平方。
  assert( (new_heading.LengthSq() - 1.0) < 0.00001);
  
// 把新朝向存进 m_vHeading。
  m_vHeading = new_heading;

// ↓↓↓ 原文注释翻译:侧向向量必须始终与朝向垂直。
  //the side vector must always be perpendicular to the heading
// 重新计算侧向:朝向的垂直方向(Perp)。
  m_vSide = m_vHeading.Perp();
}



#endif