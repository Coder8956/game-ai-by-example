//==============================================================================================
//【文件说明】GetHealthGoal_Evaluator.cpp —— "去吃血包"评估器的实现
//
//【这个文件是干什么的?】
//  打分逻辑:离血包越近、自己血越少,"去吃血"越诱人。
//  若地图上根本没有血包(特征分=1),则直接返回 0(不值得去)。
//==============================================================================================
#include "GetHealthGoal_Evaluator.h"
#include "../Raven_ObjectEnumerations.h"
#include "Goal_Think.h"
#include "Raven_Goal_Types.h"
#include "misc/Stream_Utility_Functions.h"
#include "Raven_Feature.h"


//---------------------- CalculateDesirability -------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// CalculateDesirability:算"吃血包"的渴望分。
double GetHealthGoal_Evaluator::CalculateDesirability(Raven_Bot* pBot)
{
  //first grab the distance to the closest instance of a health item
// 调特征提取器:返回 0~1 的距离分(越大=离得越远)。type_health=血包编号。
//(原文注释:先取出到最近一个血包的距离特征值)
  double Distance = Raven_Feature::DistanceToItem(pBot, type_health);

  //if the distance feature is rated with a value of 1 it means that the
  //item is either not present on the map or too far away to be worth 
  //considering, therefore the desirability is zero
// == :等于判断。距离特征为 1 = 没有血包。
//(原文注释:若距离特征=1,说明地图上没有血包或太远不值得考虑,于是渴望分为 0)
  if (Distance == 1)
  {
    return 0;
  }
  else
  {
    //value used to tweak the desirability
// Tweaker=0.2:缩放系数,把分数压到合理区间。
//(原文注释:用来微调渴望分的系数)
    const double Tweaker = 0.2;
  
    //the desirability of finding a health item is proportional to the amount
    //of health remaining and inversely proportional to the distance from the
    //nearest instance of a health item.
// 公式:Tweaker × (1 - 当前血量占比) ÷ 距离特征。
// (1-Health) 越大=血越少;分母越大=离得越远,分数越低。
//(原文注释:找到血包的渴望度与"剩余血量成反比"——血越少越想吃;
//           与"离最近血包的距离成反比"——越远越懒得去)
    double Desirability = Tweaker * (1-Raven_Feature::Health(pBot)) / 
                        (Raven_Feature::DistanceToItem(pBot, type_health));
 
    //ensure the value is in the range 0 to 1
// Clamp:把分数夹在 [0,1],防止算出负数或超过 1。
//(原文注释:确保分数落在 0~1 区间)
    Clamp(Desirability, 0, 1);
  
    //bias the value according to the personality of the bot
// 乘性格偏置(父类成员),让不同机器人"惜命程度"不同。
//(原文注释:按机器人的性格偏置调整这个分数)
    Desirability *= m_dCharacterBias;

    return Desirability;
  }
}



//----------------------------- SetGoal ---------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// SetGoal:选中后,让大脑加"去捡某类物品"目标,这里物品类型是血包。
void GetHealthGoal_Evaluator::SetGoal(Raven_Bot* pBot)
{
// AddGoal_GetItem(type_health):让大脑生成"去捡血包"目标。
  pBot->GetBrain()->AddGoal_GetItem(type_health); 
}

//-------------------------- RenderInfo ---------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// RenderInfo:屏幕上画 "H: 分数"。
void GetHealthGoal_Evaluator::RenderInfo(Vector2D Position, Raven_Bot* pBot)
{
// gdi->TextAtPos:在 Position 写字;return 之后的代码被原作者有意雪藏(调试用)。
  gdi->TextAtPos(Position, "H: " + ttos(CalculateDesirability(pBot), 2));
  return;
  
  std::string s = ttos(1-Raven_Feature::Health(pBot)) + ", " + ttos(Raven_Feature::DistanceToItem(pBot, type_health));
  gdi->TextAtPos(Position+Vector2D(0,15), s);
}