//==============================================================================================
//【文件说明】FuzzySet_RightShoulder.h —— 右肩形模糊集合
//
//【这个文件是干什么的?】
//  "右肩"形状:与左肩镜像。顶点在左,从顶点往左斜着降到 0,顶点往右一整段恒为 1。
//  适合"至多这么小"的概念,比如"距离很近":只要距离 <= 某个值,隶属度=1。
//
//【谁在使用这个文件?】FuzzyVariable::AddRightShoulderSet。
//【本文件包含了谁?】fuzzy/FuzzySet.h、misc/utils.h。
//==============================================================================================
#ifndef FUZZYSET_RIGHTSHOULDER_H
#define FUZZYSET_RIGHTSHOULDER_H
//-----------------------------------------------------------------------------
//
//  Name:   FuzzySet_RightShoulder.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   definition of a fuzzy set that has a right shoulder shape. (the
//          maximum value this variable can accept is *any* value greater than 
//          the midpoint.
//-----------------------------------------------------------------------------
#include "fuzzy/FuzzySet.h"
#include "misc/utils.h"



// ↓↓↓ 原文翻译【FuzzySet_RightShoulder —— 右肩形模糊集合】:最大可接受值是"任何大于中点
//   的值"的右肩形状集合。作者:Mat Buckland。
//------------------------------------------------------------------------
// class FuzzySet_RightShoulder : public FuzzySet —— 右肩形模糊集合。
class FuzzySet_RightShoulder : public FuzzySet
{
private:

  //the values that define the shape of this FLV
  //(原文注释:定义本集合形状的值)—— m_dPeakPoint=顶点;m_dLeftOffset/m_dRightOffset=左右半宽。
  double   m_dPeakPoint;
  double   m_dLeftOffset;
  double   m_dRightOffset;

public:
  
  // 构造函数:peak=顶点,LeftOffset/RightOffset=左右半宽。
  // 传给父类的代表值 = ((顶点+右宽)+顶点)/2,即右边平顶那段的中点。
  FuzzySet_RightShoulder(double peak,
                         double LeftOffset,
                         double RightOffset):
  
                  FuzzySet( ((peak + RightOffset) + peak) / 2),
                  m_dPeakPoint(peak),
                  m_dLeftOffset(LeftOffset),
                  m_dRightOffset(RightOffset)

  {}

  //this method calculates the degree of membership for a particular value
  //(原文注释:计算某个具体值的隶属度)—— CalculateDOM,实现在 .cpp。
  double CalculateDOM(double val)const;
};


#endif