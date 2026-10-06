//==============================================================================================
//【文件说明】FuzzySet_LeftShoulder.cpp —— 左肩形集合的 CalculateDOM 实现
//
//【逻辑】val 在顶点及左边一段:隶属度=1;val 越过顶点往右一点点:线性降到 0;再外:0。
//【谁包含它】FuzzySet_LeftShoulder.h。
#include "FuzzySet_LeftShoulder.h"
#include <cassert>


// CalculateDOM:算 val 的隶属度。
double FuzzySet_LeftShoulder::CalculateDOM(double val)const
{
  //test for the case where the left or right offsets are zero
  //(to prevent divide by zero errors below)
  // ↓↓↓ 上面两行英文注释的翻译:先测半宽为 0 的极端情况,防止下面除以零;此时 val==顶点则=1。
  if ( (isEqual(m_dRightOffset, 0.0) && (isEqual(m_dPeakPoint, val))) ||
       (isEqual(m_dLeftOffset, 0.0) && (isEqual(m_dPeakPoint, val))) )
  {
    return 1.0;
  }

  //find DOM if right of center
  //(原文注释:val 在顶点右侧时求隶属度)——从顶点往右线性降到 0。
  else if ( (val >= m_dPeakPoint) && (val < (m_dPeakPoint + m_dRightOffset)) )
  {
    double grad = 1.0 / -m_dRightOffset;

    return grad * (val - m_dPeakPoint) + 1.0;
  }

  //find DOM if left of center
  //(原文注释:val 在顶点左侧时求隶属度)——左边一整段都是平顶,隶属度恒=1。
  else if ( (val < m_dPeakPoint) && (val >= m_dPeakPoint-m_dLeftOffset) )
  {
    return 1.0;
  }

  //out of range of this FLV, return zero
  //(原文注释:超出范围,返回 0)。
  else
  {
    return 0.0;
  }

}