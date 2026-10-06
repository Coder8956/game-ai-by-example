//==============================================================================================
//【文件说明】Weapon_RocketLauncher.cpp —— 火箭发射器的实现
//
//【这个文件是干什么的?】
//  构造:脚本参数交父类,设枪外形(8 顶点),初始化模糊模块;
//  ShootAt:有弹且冷却好就 AddRocket 射出火箭、减弹、刷新冷却、加声音触发;
//  GetDesirability:无弹则0;否则距离+弹药量模糊化后反模糊出欲望分;
//  InitializeFuzzyModule:距离/欲望/弹药量三组模糊量与 9 条规则;
//  Render:红色画枪外形。
#include "Weapon_RocketLauncher.h"
#include "../Raven_Bot.h"
#include "misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 构造函数:调父类 Raven_Weapon 构造(type_rocket_launcher 及脚本参数)。
RocketLauncher::RocketLauncher(Raven_Bot*   owner):

                      Raven_Weapon(type_rocket_launcher,
                                   script->GetInt("RocketLauncher_DefaultRounds"),
                                   script->GetInt("RocketLauncher_MaxRoundsCarried"),
                                   script->GetDouble("RocketLauncher_FiringFreq"),
                                   script->GetDouble("RocketLauncher_IdealRange"),
                                   script->GetDouble("Rocket_MaxSpeed"),
                                   owner)
{
    //setup the vertex buffer
//(原文注释:设定顶点缓冲)—— 火箭筒外形 8 个顶点。
  const int NumWeaponVerts = 8;
  const Vector2D weapon[NumWeaponVerts] = {Vector2D(0, -3),
                                           Vector2D(6, -3),
                                           Vector2D(6, -1),
                                           Vector2D(15, -1),
                                           Vector2D(15, 1),
                                           Vector2D(6, 1),
                                           Vector2D(6, 3),
                                           Vector2D(0, 3)
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
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// ShootAt:开火。
inline void RocketLauncher::ShootAt(Vector2D pos)
{ 
// && :逻辑与——有弹且冷却好才开火。
  if (NumRoundsRemaining() > 0 && isReadyForNextShot())
  {
    //fire off a rocket!
// 射出 Rocket。
//(原文注释:发射一枚火箭!)—— 让世界 AddRocket 生成一枚火箭。
    m_pOwner->GetWorld()->AddRocket(m_pOwner, pos);

// -- :自减——消耗一发。
    m_iNumRoundsLeft--;

// 刷新冷却。
    UpdateTimeWeaponIsNextAvailable();

    //add a trigger to the game so that the other bots can hear this shot
    //(provided they are within range)
// 加声音触发点。
//(原文注释:加声音触发,让附近机器人听见枪声)
    m_pOwner->GetWorld()->GetMap()->AddSoundTrigger(m_pOwner, script->GetDouble("RocketLauncher_SoundRange"));
  }
}

//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// GetDesirability:算用枪欲望。
double RocketLauncher::GetDesirability(double DistToTarget)
{
// 没子弹 → 欲望 0。
  if (m_iNumRoundsLeft == 0)
  {
    m_dLastDesirabilityScore = 0;
  }
  else
  {
    //fuzzify distance and amount of ammo
// 距离、弹药量分别输入模糊模块。
//(原文注释:把距离与弹药量模糊化)
    m_FuzzyModule.Fuzzify("DistToTarget", DistToTarget);
    m_FuzzyModule.Fuzzify("AmmoStatus", (double)m_iNumRoundsLeft);

// 反模糊出欲望分。
    m_dLastDesirabilityScore = m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);
  }

// 返回欲望分。
  return m_dLastDesirabilityScore;
}

//-------------------------  InitializeFuzzyModule ----------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// InitializeFuzzyModule:定义模糊变量与规则。
void RocketLauncher::InitializeFuzzyModule()
{
// 距离变量:近/中/远三档。
  FuzzyVariable& DistToTarget = m_FuzzyModule.CreateFLV("DistToTarget");

  FzSet& Target_Close = DistToTarget.AddLeftShoulderSet("Target_Close",0,25,150);
  FzSet& Target_Medium = DistToTarget.AddTriangularSet("Target_Medium",25,150,300);
  FzSet& Target_Far = DistToTarget.AddRightShoulderSet("Target_Far",150,300,1000);

// 欲望变量:很诱人/诱人/不诱人三档。
  FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability"); 
  FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 50, 75, 100);
  FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 25, 50, 75);
  FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 25, 50);

// 弹药量变量:充足/尚可/不足三档。
  FuzzyVariable& AmmoStatus = m_FuzzyModule.CreateFLV("AmmoStatus");
  FzSet& Ammo_Loads = AmmoStatus.AddRightShoulderSet("Ammo_Loads", 10, 30, 100);
  FzSet& Ammo_Okay = AmmoStatus.AddTriangularSet("Ammo_Okay", 0, 10, 30);
  FzSet& Ammo_Low = AmmoStatus.AddTriangularSet("Ammo_Low", 0, 0, 10);


// AddRule/FzAND:下面 9 条规则——近距离一律不诱人(火箭怕自伤),中距离最诱人。
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low), Undesirable);

  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Loads), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Okay), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Low), Desirable);

  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Low), Undesirable);
}


//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:画枪。
void RocketLauncher::Render()
{
// 把枪顶点按机器人位置朝向变换到世界坐标。
    m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
                                   m_pOwner->Pos(),
                                   m_pOwner->Facing(),
                                   m_pOwner->Facing().Perp(),
                                   m_pOwner->Scale());

  gdi->RedPen();

// 红笔闭合形状画出枪。
  gdi->ClosedShape(m_vecWeaponVBTrans);
}