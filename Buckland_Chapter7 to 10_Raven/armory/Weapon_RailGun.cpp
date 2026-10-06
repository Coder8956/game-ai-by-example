//==============================================================================================
//【文件说明】Weapon_RailGun.cpp —— 轨道炮的实现
//
//【这个文件是干什么的?】
//  构造:脚本参数交父类,设枪外形,初始化模糊模块;
//  ShootAt:有弹且冷却好就 AddRailGunSlug 射出 Slug、减一弹、加声音触发;
//  GetDesirability:无弹则0;否则把距离和弹药量都模糊化,反模糊出欲望分;
//  InitializeFuzzyModule:定义距离/欲望/弹药量三组模糊量与 9 条组合规则;
//  Render:蓝色画枪外形。
#include "Weapon_RailGun.h"
#include "../Raven_Bot.h"
#include "misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 构造函数:调父类 Raven_Weapon 构造(type_rail_gun 及脚本参数)。
RailGun::RailGun(Raven_Bot*   owner):

                      Raven_Weapon(type_rail_gun,
                                   script->GetInt("RailGun_DefaultRounds"),
                                   script->GetInt("RailGun_MaxRoundsCarried"),
                                   script->GetDouble("RailGun_FiringFreq"),
                                   script->GetDouble("RailGun_IdealRange"),
                                   script->GetDouble("Slug_MaxSpeed"),
                                   owner)
{

    //setup the vertex buffer
//(原文注释:设定顶点缓冲)—— 枪的矩形外形 4 顶点加入 m_vecWeaponVB。
  const int NumWeaponVerts = 4;
  const Vector2D weapon[NumWeaponVerts] = {Vector2D(0, -1),
                                           Vector2D(10, -1),
                                           Vector2D(10, 1),
                                           Vector2D(0, 1)
                                           };

  
// 循环加入外形顶点。
  for (int vtx=0; vtx<NumWeaponVerts; ++vtx)
  {
    m_vecWeaponVB.push_back(weapon[vtx]);
  }

  //setup the fuzzy module
//(原文注释:初始化模糊模块)
  InitializeFuzzyModule();

}


//------------------------------ ShootAt --------------------------------------

//--------------------------------------------------------------------------------
// ShootAt:开火。
inline void RailGun::ShootAt(Vector2D pos)
{ 
// && :逻辑与——有子弹(>0)且冷却好才开火。
  if (NumRoundsRemaining() > 0 && isReadyForNextShot())
  {
    //fire a round
// 射出 Slug。
//(原文注释:打一发)—— 让世界 AddRailGunSlug 生成一颗高速弹。
    m_pOwner->GetWorld()->AddRailGunSlug(m_pOwner, pos);

// 刷新冷却。
    UpdateTimeWeaponIsNextAvailable();

// -- :自减——消耗一发弹药。
    m_iNumRoundsLeft--;

    //add a trigger to the game so that the other bots can hear this shot
    //(provided they are within range)
// 加声音触发点。
//(原文注释:加声音触发,让附近机器人听见枪声)
    m_pOwner->GetWorld()->GetMap()->AddSoundTrigger(m_pOwner, script->GetDouble("RailGun_SoundRange"));
  }
}

//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// GetDesirability:算用枪欲望。
double RailGun::GetDesirability(double DistToTarget)
{
// == :等于——没子弹了,欲望直接为 0。
  if (m_iNumRoundsLeft == 0)
  {
    m_dLastDesirabilityScore = 0;
  }
  else
  {
    //fuzzify distance and amount of ammo
// 分别把距离、当前弹药数输入模糊模块。
//(原文注释:把距离与弹药量模糊化)
    m_FuzzyModule.Fuzzify("DistanceToTarget", DistToTarget);
    m_FuzzyModule.Fuzzify("AmmoStatus", (double)m_iNumRoundsLeft);

// 反模糊出欲望分数。
    m_dLastDesirabilityScore = m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);
  }

// 返回欲望分。
  return m_dLastDesirabilityScore;
}

//----------------------- InitializeFuzzyModule -------------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// InitializeFuzzyModule:定义模糊变量与规则。
void RailGun::InitializeFuzzyModule()
{ 

// 创建"距离"变量,分近/中/远三档。
  FuzzyVariable& DistanceToTarget = m_FuzzyModule.CreateFLV("DistanceToTarget");
  
  FzSet& Target_Close = DistanceToTarget.AddLeftShoulderSet("Target_Close", 0, 25, 150);
  FzSet& Target_Medium = DistanceToTarget.AddTriangularSet("Target_Medium", 25, 150, 300);
  FzSet& Target_Far = DistanceToTarget.AddRightShoulderSet("Target_Far", 150, 300, 1000);

// 创建输出"欲望"变量,分很诱人/诱人/不诱人三档。
  FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability");
  
  FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 50, 75, 100);
  FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 25, 50, 75);
  FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 25, 50);

// 创建"弹药状态"变量,分充足/尚可/不足三档。
  FuzzyVariable& AmmoStatus = m_FuzzyModule.CreateFLV("AmmoStatus");
  FzSet& Ammo_Loads = AmmoStatus.AddRightShoulderSet("Ammo_Loads", 15, 30, 100);
  FzSet& Ammo_Okay = AmmoStatus.AddTriangularSet("Ammo_Okay", 0, 15, 30);
  FzSet& Ammo_Low = AmmoStatus.AddTriangularSet("Ammo_Low", 0, 0, 15);

  

// AddRule:FzAND=模糊与、FzVery/FzFairly=非常/还算。
// 下面 9 条规则:距离×弹药状态 → 欲望(远+弹药足→最诱人,体现轨道炮是远程狙击枪)。
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads), FzFairly(Desirable));
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay),  FzFairly(Desirable));
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low), Undesirable);

  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Loads), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Okay), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Low), Desirable);

  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads), FzVery(VeryDesirable));
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay), FzVery(VeryDesirable));
  m_FuzzyModule.AddRule(FzAND(Target_Far, FzFairly(Ammo_Low)), VeryDesirable);
}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:画枪。
void RailGun::Render()
{
// 把枪顶点按机器人位置朝向变换到世界坐标。
    m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
                                   m_pOwner->Pos(),
                                   m_pOwner->Facing(),
                                   m_pOwner->Facing().Perp(),
                                   m_pOwner->Scale());

  gdi->BluePen();

// 蓝笔闭合形状画出枪。
  gdi->ClosedShape(m_vecWeaponVBTrans);
}