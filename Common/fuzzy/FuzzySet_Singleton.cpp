//==============================================================================================
//【文件说明】FuzzySet_Singleton.cpp —— 单点集合的 CalculateDOM 实现
//
//【逻辑】val 落在 [中点-左宽, 中点+右宽] 内=1.0,否则=0.0。
//【谁包含它】FuzzySet_Singleton.h。
#include "FuzzySet_Singleton.h"

// CalculateDOM:算 val 的隶属度。val 在区间内返回 1.0,否则 0.0。
double FuzzySet_Singleton::CalculateDOM(double val)const
{
  if ( (val >= m_dMidPoint-m_dLeftOffset) &&
       (val <= m_dMidPoint+m_dRightOffset) )
  {
    return 1.0;
  }

  //out of range of this FLV, return zero
  //(原文注释:超出本语言变量范围,返回 0)。
  else
  {
    return 0.0;
  }
}