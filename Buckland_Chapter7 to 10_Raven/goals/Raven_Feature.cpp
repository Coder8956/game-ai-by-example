//==============================================================================================
//【文件说明】Raven_Feature.cpp —— 特征提取器的函数实现
//
//【这个文件是干什么的?】
//  实现头文件声明的四个静态函数:DistanceToItem(离物品多远)、
//  IndividualWeaponStrength(单武器弹药)、TotalWeaponStrength(总火力)、Health(血量)。
//  另外还有一个文件内辅助函数 GetMaxRoundsBotCanCarryForWeapon,按武器型号查"最大携弹量"。
//
//【本文件包含了谁?】
//  Raven_Feature.h                 —— 本类声明;
//  ../Raven_Bot.h                  —— 机器人类;
//  ../navigation/Raven_PathPlanner.h —— 路径规划器(算"到某类物品最近的代价");
//  ../armory/Raven_Weapon.h         —— 武器类(查弹药);
//  ../Raven_WeaponSystem.h         —— 武器系统(管理机器人身上所有武器);
//  ../Raven_ObjectEnumerations.h   —— 物品/武器编号枚举;
//  ../lua/Raven_Scriptor.h         —— 脚本参数单例 script(从 lua 配置读数值)。
//==============================================================================================
#include "Raven_Feature.h"
#include "../Raven_Bot.h"
#include "../navigation/Raven_PathPlanner.h"
#include "../armory/Raven_Weapon.h"
#include "../Raven_WeaponSystem.h"
#include "../Raven_ObjectEnumerations.h"
#include "../lua/Raven_Scriptor.h"

//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// DistanceToItem:算"离最近的某类物品有多远",再折算成 0~1。
//--------------------------------------------------------------------------------
double Raven_Feature::DistanceToItem(Raven_Bot* pBot, int ItemType)
{
  //determine the distance to the closest instance of the item type
// 向路径规划器问"到最近该类物品的代价";-> 是"指针访问对象成员"的箭头。
// 同名局部变量 DistanceToItem 与函数同名,只是巧合(此处遮蔽,不影响)。
//(原文注释:求出到该类物品最近一个实例的距离)
  double DistanceToItem = pBot->GetPathPlanner()->GetCostToClosestItem(ItemType);

  //if the previous method returns a negative value then there is no item of
  //the specified type present in the game world at this time.
// 没有该物品 → 直接返回 1(评估器据此判断"反正捡不到,无所谓")。
//(原文注释:若上面那个函数返回负数,说明此刻游戏世界里根本没有该类物品)
  if (DistanceToItem < 0 ) return 1;

  //these values represent cutoffs. Any distance over MaxDistance results in
  //a value of 0, and value below MinDistance results in a value of 1
// const = 常量,定义后不许改。MaxDistance=500(再远也算 0)、MinDistance=50(贴脸算 1)。
//(原文注释:这两个值是截断点:距离超过 MaxDistance 记 0,小于 MinDistance 记 1)
  const double MaxDistance = 500.0;
  const double MinDistance = 50.0;

// Clamp(值, 下限, 上限):把数值夹在 [Min,Max] 区间内(小于下限抬到下限,大于上限压到上限)。
  Clamp(DistanceToItem, MinDistance, MaxDistance);

// 距离 ÷ 最大距离 = 0~1 的结果:离得越远,值越大(评估器据此判断"要不要跑远路去捡")。
  return DistanceToItem / MaxDistance;
}


//----------------------- GetMaxRoundsBotCanCarryForWeapon --------------------
//
//  helper function to tidy up IndividualWeapon method
//  returns the maximum rounds of ammo a bot can carry for the given weapon
//(原文注释:辅助函数,整理 IndividualWeaponStrength 的代码;返回机器人该武器的最大携弹量)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 自由辅助函数:按武器型号去 lua 脚本配置里查"最大携带弹药数"。
// script->GetDouble("名字"):从脚本配置读一个浮点数(script 是脚本单例宏)。
//--------------------------------------------------------------------------------
double GetMaxRoundsBotCanCarryForWeapon(int WeaponType)
{
  switch(WeaponType)
  {
  case type_rail_gun:

    return script->GetDouble("RailGun_MaxRoundsCarried");

  case type_rocket_launcher:

    return script->GetDouble("RocketLauncher_MaxRoundsCarried");

  case type_shotgun:

    return script->GetDouble("ShotGun_MaxRoundsCarried");

  default:

// throw:抛出异常(程序出错,直接中断);std::runtime_error 是标准库运行时错误类。
// 走到这里说明传了一个"不认识的武器型号"。
    throw std::runtime_error("trying to calculate  of unknown weapon");

  }//end switch
}


//----------------------- IndividualWeaponStrength ----------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// IndividualWeaponStrength:某武器现有弹药 ÷ 最大携弹量 = 0~1 的充足度。
//--------------------------------------------------------------------------------
double Raven_Feature::IndividualWeaponStrength(Raven_Bot* pBot,
                                               int        WeaponType)
{
  //grab a pointer to the gun (if the bot owns an instance)
// wp = 武器指针;若机器人没有这把枪,GetWeaponFromInventory 返回空指针 NULL。
//(原文注释:取出指向该枪的指针——前提是机器人身上真有这把枪)
  Raven_Weapon* wp = pBot->GetWeaponSys()->GetWeaponFromInventory(WeaponType);

// if(wp):指针非空才执行(等于 if(wp != NULL))。
  if (wp)
  {
// 剩余弹药 ÷ 最大弹药 = 充足度(浮点除法,故前面把 NumRoundsRemaining 的结果按 double 用)。
    return wp->NumRoundsRemaining() / GetMaxRoundsBotCanCarryForWeapon(WeaponType);
  }

  else
  {
   return 0.0;
  }
}

//--------------------- TotalWeaponStrength --------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// TotalWeaponStrength:把三种可捡武器的弹药加总,再折算成 0~1 的总火力。
//--------------------------------------------------------------------------------
double Raven_Feature::TotalWeaponStrength(Raven_Bot* pBot)
{
  const double MaxRoundsForShotgun = GetMaxRoundsBotCanCarryForWeapon(type_shotgun);
  const double MaxRoundsForRailgun = GetMaxRoundsBotCanCarryForWeapon(type_rail_gun);
  const double MaxRoundsForRocketLauncher = GetMaxRoundsBotCanCarryForWeapon(type_rocket_launcher);
  const double TotalRoundsCarryable = MaxRoundsForShotgun + MaxRoundsForRailgun + MaxRoundsForRocketLauncher;

// 分别取出三种武器的现有弹药;(double) 是强制类型转换,把整数弹药转成浮点,
// 免得整数除法把小数截掉。NumSlugs=轨道炮弹,NumCartridges=霰弹枪子弹,NumRockets=火箭。
  double NumSlugs      = (double)pBot->GetWeaponSys()->GetAmmoRemainingForWeapon(type_rail_gun);
  double NumCartridges = (double)pBot->GetWeaponSys()->GetAmmoRemainingForWeapon(type_shotgun);
  double NumRockets    = (double)pBot->GetWeaponSys()->GetAmmoRemainingForWeapon(type_rocket_launcher);

  //the value of the tweaker (must be in the range 0-1) indicates how much
  //desirability value is returned even if a bot has not picked up any weapons.
//(原文注释:这个 0~1 的微调值 Tweaker 表示——就算机器人一把可捡武器都没拿,
//           也仍给它一个基础分(等于给它那把"天生的"爆能枪打底))
  //(it basically adds in an amount for a bot's persistent weapon -- the blaster)
  const double Tweaker = 0.1;

// 公式:基础分 0.1 + (1-0.1) × (现有总弹药 ÷ 可携带总弹药)。
// 这样即使空着手也有 0.1 的底分,捡满武器接近 1 分。
  return Tweaker + (1-Tweaker)*(NumSlugs + NumCartridges + NumRockets)/(MaxRoundsForShotgun + MaxRoundsForRailgun + MaxRoundsForRocketLauncher);
}

//------------------------------- HealthScore ---------------------------------
//
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Health:当前血量 ÷ 最大血量 = 0~1。
double Raven_Feature::Health(Raven_Bot* pBot)
{
// (double) 把整数血量转浮点再相除;满血=1,空血=0。
  return (double)pBot->Health() / (double)pBot->MaxHealth();

}