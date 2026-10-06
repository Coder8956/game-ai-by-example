//==============================================================================================
//【文件说明】Weapon_Blaster.cpp —— 爆能枪的实现
//
//【这个文件是干什么的?】
//  构造:从脚本读参数交给父类,设定枪的外形顶点,初始化模糊模块;
//  ShootAt:冷却好就 AddBolt 射出光束、刷新冷却、加声音触发(让附近机器人听见枪声);
//  GetDesirability:用模糊逻辑按目标距离算用枪欲望;
//  InitializeFuzzyModule:定义距离/欲望的模糊集合与规则;
//  Render:把枪的顶点按机器人位置朝向变换后画出来。
#include "Weapon_Blaster.h"
#include "../Raven_Bot.h"
#include "misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 构造函数:调父类 Raven_Weapon 构造(type_blaster 及脚本里的弹药/射速/射程/子弹速度)。
Blaster::Blaster(Raven_Bot*   owner):

                      Raven_Weapon(type_blaster,
                                   script->GetInt("Blaster_DefaultRounds"),
                                   script->GetInt("Blaster_MaxRoundsCarried"),
                                   script->GetDouble("Blaster_FiringFreq"),
                                   script->GetDouble("Blaster_IdealRange"),
                                   script->GetDouble("Bolt_MaxSpeed"),
                                   owner)
{
  //setup the vertex buffer
//(原文注释:设定顶点缓冲)—— 定义枪的矩形外形(4 个顶点),塞进 m_vecWeaponVB。
  const int NumWeaponVerts = 4;
  const Vector2D weapon[NumWeaponVerts] = {Vector2D(0, -1),
                                           Vector2D(10, -1),
                                           Vector2D(10, 1),
                                           Vector2D(0, 1)
                                           };

  
// for 循环把 4 个顶点加入外形列表;++vtx 是自增。
  for (int vtx=0; vtx<NumWeaponVerts; ++vtx)
  {
    m_vecWeaponVB.push_back(weapon[vtx]);
  }

  //setup the fuzzy module
// 初始化模糊规则。
//(原文注释:初始化模糊模块)
  InitializeFuzzyModule();
}


//------------------------------ ShootAt --------------------------------------

//--------------------------------------------------------------------------------
// ShootAt:开火。
inline void Blaster::ShootAt(Vector2D pos)
{ 
// 冷却好才开火。
  if (isReadyForNextShot())
  {
    //fire!
// 射出 Bolt。
//(原文注释:开火!)—— 让世界 AddBolt 生成一颗光束子弹。
    m_pOwner->GetWorld()->AddBolt(m_pOwner, pos);

// 刷新下一枪冷却时刻。
    UpdateTimeWeaponIsNextAvailable();

    //add a trigger to the game so that the other bots can hear this shot
    //(provided they are within range)
// 在地图上加一个声音触发点(吸引敌人注意)。
//(原文注释:加声音触发,让附近机器人听见这声枪响(在范围内的话))
    m_pOwner->GetWorld()->GetMap()->AddSoundTrigger(m_pOwner, script->GetDouble("Blaster_SoundRange"));
  }
}



//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// GetDesirability:按到目标距离算用枪欲望(0~1)。
double Blaster::GetDesirability(double DistToTarget)
{
  //fuzzify distance and amount of ammo
// Fuzzify:输入精确距离值,变成模糊量。
//(原文注释:把距离(与弹药量)模糊化)
  m_FuzzyModule.Fuzzify("DistToTarget", DistToTarget);

// DeFuzzify:把模糊结论反模糊化成一个精确分数(max_av 取最大平均)。
  m_dLastDesirabilityScore = m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);

// 返回用枪欲望分数。
  return m_dLastDesirabilityScore;
}

//----------------------- InitializeFuzzyModule -------------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// InitializeFuzzyModule:定义模糊变量与规则。
void Blaster::InitializeFuzzyModule()
{
// CreateFLV:创建模糊语言变量;AddLeftShoulder/Triangular/RightShoulderSet:定义近/中/远三档。
  FuzzyVariable& DistToTarget = m_FuzzyModule.CreateFLV("DistToTarget");

  FzSet& Target_Close = DistToTarget.AddLeftShoulderSet("Target_Close",0,25,150);
  FzSet& Target_Medium = DistToTarget.AddTriangularSet("Target_Medium",25,150,300);
  FzSet& Target_Far = DistToTarget.AddRightShoulderSet("Target_Far",150,300,1000);

// 定义输出"欲望"的三档:很诱人/诱人/不诱人。
  FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability"); 
  FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 50, 75, 100);
  FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 25, 50, 75);
  FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 25, 50);

// AddRule:模糊规则——近→诱人;中→很不诱人;远→很不诱人。
  m_FuzzyModule.AddRule(Target_Close, Desirable);
  m_FuzzyModule.AddRule(Target_Medium, FzVery(Undesirable));
  m_FuzzyModule.AddRule(Target_Far, FzVery(Undesirable));
}


//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:画枪。
void Blaster::Render()
{
// WorldTransform:把枪的顶点按机器人位置/朝向/尺度变换到世界坐标。
   m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
                                   m_pOwner->Pos(),
                                   m_pOwner->Facing(),
                                   m_pOwner->Facing().Perp(),
                                   m_pOwner->Scale());

  gdi->GreenPen();

// 绿笔把变换后的顶点连成闭合形状(画出枪)。
  gdi->ClosedShape(m_vecWeaponVBTrans);
}