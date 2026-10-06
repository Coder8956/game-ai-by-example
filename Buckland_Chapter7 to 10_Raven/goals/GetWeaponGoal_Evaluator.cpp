//==============================================================================================
//【文件说明】GetWeaponGoal_Evaluator.cpp —— "去捡某种武器"评估器的实现
//
//【这个文件是干什么的?】
//  打分规则:离这件武器越近、自己血越多(敢出去捡)、这件武器弹药越少(越缺)→
//  越想去捡。若地图上没有这件武器,直接返回 0。
//==============================================================================================
#include "GetWeaponGoal_Evaluator.h"
#include "../Raven_ObjectEnumerations.h"
#include "misc/Stream_Utility_Functions.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "Goal_Think.h"
#include "Raven_Goal_Types.h"
#include "Raven_Feature.h"

#include <string>




//------------------- CalculateDesirability ---------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// CalculateDesirability:算"捡武器"的渴望分。
double GetWeaponGoal_Evaluator::CalculateDesirability(Raven_Bot* pBot)
{
  //grab the distance to the closest instance of the weapon type
// 距离特征 0~1(越大=越远),m_iWeaponType 是本评估器盯的武器型号。
//(原文注释:取出到最近一件该武器的距离特征值)
  double Distance = Raven_Feature::DistanceToItem(pBot, m_iWeaponType);

  //if the distance feature is rated with a value of 1 it means that the
  //item is either not present on the map or too far away to be worth 
  //considering, therefore the desirability is zero
// 没有该武器 → 返回 0。
//(原文注释:距离特征=1 说明地图上没这武器或太远,渴望分为 0)
  if (Distance == 1)
  {
    return 0;
  }
  else
  {
    //value used to tweak the desirability
// Tweaker=0.15:缩放系数。
//(原文注释:微调系数)
    const double Tweaker = 0.15;

// 声明两个局部变量:Health=血量占比,WeaponStrength=该武器现有弹药充足度。
    double Health, WeaponStrength;

// 取血量占比(0~1)。
    Health = Raven_Feature::Health(pBot);

// 取该武器弹药充足度(0~1):越少=越需要补充。
    WeaponStrength = Raven_Feature::IndividualWeaponStrength(pBot,
                                                             m_iWeaponType);
    
// 公式:Tweaker × 血量 × (1-弹药充足度) ÷ 距离。
// 血越多、弹药越缺、离得越近 → 越想去捡。
    double Desirability = (Tweaker * Health * (1-WeaponStrength)) / Distance;

    //ensure the value is in the range 0 to 1
// Clamp:限幅。
//(原文注释:夹到 0~1)
    Clamp(Desirability, 0, 1);

// 乘性格偏置。
    Desirability *= m_dCharacterBias;

    return Desirability;
  }
}



//------------------------------ SetGoal --------------------------------------
//--------------------------------------------------------------------------------
// SetGoal:选中后给大脑加"去捡某类物品"目标,这里物品类型是本评估器盯的武器。
void GetWeaponGoal_Evaluator::SetGoal(Raven_Bot* pBot)
{
// AddGoal_GetItem(武器型号):生成去捡该武器的目标。
  pBot->GetBrain()->AddGoal_GetItem(m_iWeaponType); 
}

//-------------------------- RenderInfo ---------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// RenderInfo:按武器型号显示不同缩写(RG=轨道炮 RL=火箭筒 SG=霰弹枪)。
void GetWeaponGoal_Evaluator::RenderInfo(Vector2D Position, Raven_Bot* pBot)
{
  std::string s;
// switch:按武器型号分支,把缩写字符串存进 s。
  switch(m_iWeaponType)
  {
  case type_rail_gun:
    s="RG: ";break;
  case type_rocket_launcher:
    s="RL: "; break;
  case type_shotgun:
    s="SG: "; break;
  }
  
// 屏幕画出 "缩写: 分数"。
  gdi->TextAtPos(Position, s + ttos(CalculateDesirability(pBot), 2));
}