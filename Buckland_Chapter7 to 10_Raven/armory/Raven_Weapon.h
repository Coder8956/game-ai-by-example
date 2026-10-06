//==============================================================================================
//【文件说明】Raven_Weapon.h —— 武器基类
//
//【这个文件是干什么的?】
//  机器人的武器都继承本类。它记录:所属机器人、武器类型、弹药数、射速、
//  理想交战距离、子弹速度等;用模糊逻辑(FuzzyModule)评估"当前该不该用这把枪"。
//  ShootAt()/Render()/GetDesirability() 是纯虚函数,由 Blaster/RailGun/RocketLauncher/ShotGun 实现。
//
//【谁在使用这个文件?】—— 四把具体枪(Weapon_Blaster 等)继承它;
//   Raven_WeaponSystem(武器系统)持有一把枪的列表,负责选枪与开火。
//【本文件包含了谁?】—— <vector>、2d/Vector2D.h、time/CrudeTimer.h、misc/utils.h、
//   ../lua/Raven_Scriptor.h(读脚本参数)、../Raven_Bot.h、Fuzzy/FuzzyModule.h(模糊逻辑)。
//==============================================================================================
#ifndef WEAPON_BASE_H
#define WEAPON_BASE_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Weapon.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   Base Weapon class for the raven project
//-----------------------------------------------------------------------------
#include <vector>

#include "2d/Vector2D.h"
#include "time/CrudeTimer.h"
#include "misc/utils.h"
#include "../lua/Raven_Scriptor.h"
#include "../Raven_Bot.h"
#include "Fuzzy/FuzzyModule.h"



class  Raven_Bot;

//--------------------------------------------------------------------------------
// class Raven_Weapon:武器基类(无继承父类,是独立的基类)。
class Raven_Weapon
{
protected:

  //a weapon is always (in this game) carried by a bot
// m_pOwner:携带这把枪的机器人。
//(原文注释:本游戏里武器总是由机器人携带)
  Raven_Bot*    m_pOwner;

  //an enumeration indicating the type of weapon
// m_iType:武器类型编号。
//(原文注释:表示武器类型的枚举编号)
  unsigned int  m_iType;

  //fuzzy logic is used to determine the desirability of a weapon. Each weapon
  //owns its own instance of a fuzzy module because each has a different rule 
  //set for inferring desirability.
// m_FuzzyModule:模糊逻辑模块(算"这枪多诱人")。
//(原文注释:用模糊逻辑判定武器可取性;每把枪有自己的模糊模块,规则集不同)
  FuzzyModule   m_FuzzyModule;

  //amount of ammo carried for this weapon
// m_iNumRoundsLeft:剩余子弹。
//(原文注释:这把枪当前弹药数)
  unsigned int  m_iNumRoundsLeft;

  //maximum number of rounds a bot can carry for this weapon
// m_iMaxRoundsCarried:弹药上限。
//(原文注释:这把枪最多能带多少子弹)
  unsigned int  m_iMaxRoundsCarried;
  
  //the number of times this weapon can be fired per second
// m_dRateOfFire:射速(发/秒)。
//(原文注释:这把枪每秒能打几次(射速))
  double         m_dRateOfFire;

  //the earliest time the next shot can be taken
// m_dTimeNextAvailable:冷却结束时刻。
//(原文注释:下一枪最早可打的时刻)
  double         m_dTimeNextAvailable;

  //this is used to keep a local copy of the previous desirability score
  //so that we can give some feedback for debugging
// m_dLastDesirabilityScore:上次评估分数(调试用)。
//(原文注释:记下上次的可取性分数,便于调试显示)
  double         m_dLastDesirabilityScore;

  //this is the prefered distance from the enemy when using this weapon
// m_dIdealRange:理想交战距离。
//(原文注释:用这把枪时与敌人的理想距离)
  double         m_dIdealRange;

  //the max speed of the projectile this weapon fires
// m_dMaxProjectileSpeed:子弹速度。
//(原文注释:这把枪射出子弹的最大速度)
  double         m_dMaxProjectileSpeed;

  //The number of times a weapon can be discharges depends on its rate of fire.
  //This method returns true if the weapon is able to be discharged at the 
  //current time. (called from ShootAt() )
// isReadyForNextShot:是否冷却结束可开火。
//(原文注释:武器能否开火取决于射速;当前时刻能否开火由本函数判断,在 ShootAt 里调用)
  bool          isReadyForNextShot();

  //this is called when a shot is fired to update m_dTimeNextAvailable
// UpdateTimeWeaponIsNextAvailable:刷新冷却时刻。
//(原文注释:开火后更新下一枪可打时刻)
  void          UpdateTimeWeaponIsNextAvailable();

  //this method initializes the fuzzy module with the appropriate fuzzy 
  //variables and rule base.
// InitializeFuzzyModule:纯虚函数,各枪自己定义模糊规则。
//(原文注释:用合适的模糊变量与规则集初始化模糊模块)——纯虚函数,子类实现。
  virtual void  InitializeFuzzyModule() = 0;

  //vertex buffers containing the weapon's geometry
// 武器外形顶点列表(用于画出枪的样子)。
//(原文注释:存武器外形的顶点缓冲)—— m_vecWeaponVB 原始形状,VBTrans 变换后形状。
  std::vector<Vector2D>   m_vecWeaponVB;
  std::vector<Vector2D>   m_vecWeaponVBTrans;



// public:对外接口。
public:

// 构造函数:传入枪类型/初始弹药/弹药上限/射速/理想距离/子弹速度/所属机器人;
// 初始化列表给各成员赋值;花括号里把下一枪时刻设为当前时间。
  Raven_Weapon(unsigned int TypeOfGun,
               unsigned int DefaultNumRounds,
               unsigned int MaxRoundsCarried,
               double        RateOfFire,
               double        IdealRange,
               double        ProjectileSpeed,
               Raven_Bot*   OwnerOfGun):m_iType(TypeOfGun),
                                 m_iNumRoundsLeft(DefaultNumRounds),
                                 m_pOwner(OwnerOfGun),
                                 m_dRateOfFire(RateOfFire),
                                 m_iMaxRoundsCarried(MaxRoundsCarried),
                                 m_dLastDesirabilityScore(0),
                                 m_dIdealRange(IdealRange),
                                 m_dMaxProjectileSpeed(ProjectileSpeed)
  {  
    m_dTimeNextAvailable = Clock->GetCurrentTime();
  }

// 虚析构函数:用 virtual 声明,保证通过基类指针删除子类对象时能正确析构。
  virtual ~Raven_Weapon(){}

  //this method aims the weapon at the given target by rotating the weapon's
  //owner's facing direction (constrained by the bot's turning rate). It returns  
  //true if the weapon is directly facing the target.
// AimAt:瞄准目标。
//(原文注释:转动持枪者朝向对准目标(受转向速度限制);已正对目标则返回 true)
  bool          AimAt(Vector2D target)const;

  //this discharges a projectile from the weapon at the given target position
  //(provided the weapon is ready to be discharged... every weapon has its
  //own rate of fire)
// ShootAt:纯虚函数,各枪自己实现开火。
//(原文注释:朝目标位置发射一颗子弹(需冷却就绪);各枪射速不同)
  virtual void  ShootAt(Vector2D pos) = 0;

  //each weapon has its own shape and color
// Render:纯虚函数,画枪。
//(原文注释:每把枪有自己的外形与颜色)—— Render 纯虚函数。
  virtual void  Render() = 0;

  //this method returns a value representing the desirability of using the
  //weapon. This is used by the AI to select the most suitable weapon for
  //a bot's current situation. This value is calculated using fuzzy logic
// GetDesirability:纯虚函数,根据到目标距离算"用这枪的可取性",AI 据此选枪。
  virtual double GetDesirability(double DistToTarget)=0;

  //returns the desirability score calculated in the last call to GetDesirability
  //(just used for debugging)
// 一组内联小函数:取上次分数/子弹速度/剩余弹药/类型/理想距离。
//(原文注释:返回上次 GetDesirability 算出的分数,仅调试用)
  double         GetLastDesirabilityScore()const{return m_dLastDesirabilityScore;}

  //returns the maximum speed of the projectile this weapon fires
  double         GetMaxProjectileSpeed()const{return m_dMaxProjectileSpeed;}

  //returns the number of rounds remaining for the weapon
  int           NumRoundsRemaining()const{return m_iNumRoundsLeft;}
// DecrementNumRounds:打一发就减一(-- 自减;>0 才减,防止变负)。
  void          DecrementNumRounds(){if (m_iNumRoundsLeft>0) --m_iNumRoundsLeft;}
// IncrementRounds:加弹药(下面 inline 实现)。
  void          IncrementRounds(int num); 
  unsigned int  GetType()const{return m_iType;}
  double         GetIdealRange()const{return m_dIdealRange;}
};


///////////////////////////////////////////////////////////////////////////////
//------------------------ ReadyForNextShot -----------------------------------
//
//  returns true if the weapon is ready to be discharged
//(原文注释:武器是否准备好开火)—— inline 内联函数(直接编译进调用处)。
//-----------------------------------------------------------------------------
// 当前时间 > 可开火时刻 → 已冷却好。
inline bool Raven_Weapon::isReadyForNextShot()
{
  if (Clock->GetCurrentTime() > m_dTimeNextAvailable)
  {
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
// 更新冷却:下一枪时刻 = 当前时间 + 1/射速(每秒发数的倒数=每发间隔秒数)。
inline void Raven_Weapon::UpdateTimeWeaponIsNextAvailable()
{
  m_dTimeNextAvailable = Clock->GetCurrentTime() + 1.0/m_dRateOfFire;
}


//-----------------------------------------------------------------------------
// 让机器人转向对准目标位置。
inline bool Raven_Weapon::AimAt(Vector2D target)const
{
  return m_pOwner->RotateFacingTowardPosition(target);
}

//-----------------------------------------------------------------------------
// 加弹药 num,再用 Clamp 把它限制在 [0, 上限] 区间内(防超上限)。
inline void Raven_Weapon::IncrementRounds(int num)
{
  m_iNumRoundsLeft+=num;
  Clamp(m_iNumRoundsLeft, 0, m_iMaxRoundsCarried);
} 





#endif