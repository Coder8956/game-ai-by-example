//==============================================================================================
//【文件说明】Weapon_ShotGun.cpp —— 霰弹枪的实现
//
//【这个文件是干什么的?】
//  构造:脚本参数交父类,读每发弹丸数与散射角,设枪外形,初始化模糊模块;
//  ShootAt:有弹且冷却好,循环打出 numBalls 颗带随机散射角的 Pellet,减弹、刷新冷却、加声音触发;
//  GetDesirability:无弹则0;否则距离+弹药量模糊化后反模糊出欲望分;
//  InitializeFuzzyModule:距离/欲望/弹药量三组模糊量与 9 条规则(近距离最诱人);
//  Render:棕色画枪外形折线。
#include "Weapon_ShotGun.h"
#include "../Raven_Bot.h"
#include "misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "misc/utils.h"
#include "fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 构造函数:调父类 Raven_Weapon 构造(type_shotgun 及脚本参数);
// 再从脚本读每发弹丸数 ShotGun_NumBallsInShell、散射角 ShotGun_Spread。
ShotGun::ShotGun(Raven_Bot*   owner):

                      Raven_Weapon(type_shotgun,
                                   script->GetInt("ShotGun_DefaultRounds"),
                                   script->GetInt("ShotGun_MaxRoundsCarried"),
                                   script->GetDouble("ShotGun_FiringFreq"),
                                   script->GetDouble("ShotGun_IdealRange"),
                                   script->GetDouble("Pellet_MaxSpeed"),
                                   owner),

            m_iNumBallsInShell(script->GetInt("ShotGun_NumBallsInShell")),
            m_dSpread(script->GetDouble("ShotGun_Spread"))
{

    //setup the vertex buffer
//(原文注释:设定顶点缓冲)—— 枪外形 8 个顶点。
  const int NumWeaponVerts = 8;
  const Vector2D weapon[NumWeaponVerts] = {Vector2D(0, 0),
                                           Vector2D(0, -2),
                                           Vector2D(10, -2),
                                           Vector2D(10, 0),
                                           Vector2D(0, 0),
                                           Vector2D(0, 2),
                                           Vector2D(10, 2),
                                           Vector2D(10, 0)
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
inline void ShotGun::ShootAt(Vector2D pos)
{ 
// && :逻辑与——有弹且冷却好才开火。
  if (NumRoundsRemaining() > 0 && isReadyForNextShot())
  {
    //a shotgun cartridge contains lots of tiny metal balls called pellets. 
    //Therefore, every time the shotgun is discharged we have to calculate
    //the spread of the pellets and add one for each trajectory
// 循环打出 m_iNumBallsInShell 颗弹丸。
//(原文注释:霰弹每发含很多小弹丸;开火时要算散射角,每条弹道加一颗弹丸)
    for (int b=0; b<m_iNumBallsInShell; ++b)
    {
      //determine deviation from target using a bell curve type distribution
// deviation:随机散射角(两个 [0,spread] 随机数相加减 spread,近似正态分布)。
//(原文注释:用钟形(高斯)分布算每颗弹丸偏离目标的角度)
      double deviation = RandInRange(0, m_dSpread) + RandInRange(0, m_dSpread) - m_dSpread;

// 从机器人指向目标的向量。
      Vector2D AdjustedTarget = pos - m_pOwner->Pos();
 
      //rotate the target vector by the deviation
// 旋转目标向量,得到这颗弹丸的实际方向。
//(原文注释:把目标向量按散射角旋转)
      Vec2DRotateAroundOrigin(AdjustedTarget, deviation);
 
      //add a pellet to the game world
// 射出一颗 Pellet(方向已加散射)。
//(原文注释:向游戏世界加入一颗弹丸)
      m_pOwner->GetWorld()->AddShotGunPellet(m_pOwner, AdjustedTarget + m_pOwner->Pos());

    }

// -- :自减——消耗一发。
    m_iNumRoundsLeft--;

// 刷新冷却。
    UpdateTimeWeaponIsNextAvailable();

    //add a trigger to the game so that the other bots can hear this shot
    //(provided they are within range)
// 加声音触发点。
//(原文注释:加声音触发,让附近机器人听见枪声)
    m_pOwner->GetWorld()->GetMap()->AddSoundTrigger(m_pOwner, script->GetDouble("ShotGun_SoundRange"));
  }
}

//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// GetDesirability:算用枪欲望。
inline double ShotGun::GetDesirability(double DistToTarget)
{
// 没子弹 → 欲望 0。
  if (m_iNumRoundsLeft == 0)
  {
    m_dLastDesirabilityScore = 0;
  }
  else
  {
    //fuzzify distance and amount of ammo
// 距离、弹药量输入模糊模块。
//(原文注释:把距离与弹药量模糊化)
    m_FuzzyModule.Fuzzify("DistanceToTarget", DistToTarget);
    m_FuzzyModule.Fuzzify("AmmoStatus", (double)m_iNumRoundsLeft);

// 反模糊出欲望分。
    m_dLastDesirabilityScore = m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);
  }

// 返回欲望分。
  return m_dLastDesirabilityScore;
}

//--------------------------- InitializeFuzzyModule ---------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// InitializeFuzzyModule:定义模糊变量与规则。
void ShotGun::InitializeFuzzyModule()
{  
// 距离变量:近/中/远三档。
  FuzzyVariable& DistanceToTarget = m_FuzzyModule.CreateFLV("DistanceToTarget");

  FzSet& Target_Close = DistanceToTarget.AddLeftShoulderSet("Target_Close", 0, 25, 150);
  FzSet& Target_Medium = DistanceToTarget.AddTriangularSet("Target_Medium", 25, 150, 300);
  FzSet& Target_Far = DistanceToTarget.AddRightShoulderSet("Target_Far", 150, 300, 1000);

// 欲望变量:很诱人/诱人/不诱人三档。
  FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability");
  
  FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 50, 75, 100);
  FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 25, 50, 75);
  FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 25, 50);

// 弹药量变量:充足/尚可/不足三档。
  FuzzyVariable& AmmoStatus = m_FuzzyModule.CreateFLV("AmmoStatus");
  FzSet& Ammo_Loads = AmmoStatus.AddRightShoulderSet("Ammo_Loads", 30, 60, 100);
  FzSet& Ammo_Okay = AmmoStatus.AddTriangularSet("Ammo_Okay", 0, 30, 60);
  FzSet& Ammo_Low = AmmoStatus.AddTriangularSet("Ammo_Low", 0, 0, 30);


// AddRule/FzAND:9 条规则——近距离任意弹药量都很诱人(霰弹是近程利器)。
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low), VeryDesirable);

  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Loads), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Okay), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Low), Undesirable);

  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Low), Undesirable);
}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:画枪。
void ShotGun::Render()
{
// 把枪顶点按机器人位置朝向变换到世界坐标。
  m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
                                   m_pOwner->Pos(),
                                   m_pOwner->Facing(),
                                   m_pOwner->Facing().Perp(),
                                   m_pOwner->Scale());

  gdi->BrownPen();

// 棕笔用折线(PolyLine)连起顶点画出枪。
  gdi->PolyLine(m_vecWeaponVBTrans);

}