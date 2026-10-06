//==============================================================================================
//【文件说明】FuzzySet_LeftShoulder.h —— 左肩形模糊集合
//
//【这个文件是干什么的?】
//  "左肩"形状:像一个向右歪的三角——顶点在右,从顶点往左(数值更小方向)斜着降到 0,
//  而顶点左边一整段都是满的(隶属度恒=1)。适合表示"至少这么大"的概念,比如
//  "距离很远":只要距离 >= 某个值,隶属度=1,再往小才递减。
//
//【谁在使用这个文件?】FuzzyVariable::AddLeftShoulderSet。
//【本文件包含了谁?】fuzzy/FuzzySet.h、misc/utils.h。
//==============================================================================================
#ifndef FUZZYSET_LEFTSHOULDER_H
#define FUZZYSET_LEFTSHOULDER_H
//-----------------------------------------------------------------------------
//
//  Name:   FuzzySet_LeftShoulder.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   definition of a fuzzy set that has a left shoulder shape. (the
//          minimum value this variable can accept is *any* value less than the
//          midpoint.
//-----------------------------------------------------------------------------
#include "fuzzy/FuzzySet.h"
#include "misc/utils.h"



// ↓↓↓ 原文翻译【FuzzySet_LeftShoulder —— 左肩形模糊集合】:最小可接受值是"任何小于中点
//   的值"的左肩形状集合。作者:Mat Buckland。
//------------------------------------------------------------------------
// class FuzzySet_LeftShoulder : public FuzzySet —— 左肩形模糊集合。
class FuzzySet_LeftShoulder : public FuzzySet
{
private:

  //the values that define the shape of this FLV
  //(原文注释:定义本集合形状的值)—— m_dPeakPoint=顶点;m_dLeftOffset/m_dRightOffset=左右半宽。
  double   m_dPeakPoint;
  double   m_dRightOffset;
  double   m_dLeftOffset;

public:
  
  // 构造函数:peak=顶点,LeftOffset=左半宽,RightOffset=右半宽。
  // 传给父类的代表值 = ((顶点-左宽)+顶点)/2,即平顶那段的中点(去模糊时用)。
  FuzzySet_LeftShoulder(double peak,
                        double LeftOffset,
                        double RightOffset):  
  
                  FuzzySet( ((peak - LeftOffset) + peak) / 2),
                  m_dPeakPoint(peak),
                  m_dLeftOffset(LeftOffset),
                  m_dRightOffset(RightOffset)
  {}

  //this method calculates the degree of membership for a particular value
  //(原文注释:计算某个具体值的隶属度)—— CalculateDOM,实现在 .cpp。
  double CalculateDOM(double val)const;  
};



#endif