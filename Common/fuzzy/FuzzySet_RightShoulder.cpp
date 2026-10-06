//==============================================================================================
//【文件说明】FuzzySet_RightShoulder.cpp —— 右肩形集合的 CalculateDOM 实现
//
//【逻辑】val 在顶点及右边一段:隶属度=1;val 在顶点左边一点点:线性降到 0;再外:0。
//【谁包含它】FuzzySet_RightShoulder.h。
#include "FuzzySet_RightShoulder.h"
#include <cassert>


// CalculateDOM:算 val 的隶属度。
double FuzzySet_RightShoulder::CalculateDOM(double val)const
{
  //test for the case where the left or right offsets are zero
  //(to prevent divide by zero errors below)
  // ↓↓↓ 上面两行英文注释的翻译:先测半宽为 0 的极端情况,防止除以零;此时 val==顶点则=1。
  if ( (isEqual(m_dRightOffset, 0.0) && (isEqual(m_dPeakPoint, val))) ||
       (isEqual(m_dLeftOffset, 0.0) && (isEqual(m_dPeakPoint, val))) )
  {
    return 1.0;
  }
  
  //find DOM if left of center
  //(原文注释:val 在顶点左侧时求隶属度)——从顶点往左线性降到 0。
  else if ( (val <= m_dPeakPoint) && (val > (m_dPeakPoint - m_dLeftOffset)) )
  {
    double grad = 1.0 / m_dLeftOffset;

    return grad * (val - (m_dPeakPoint - m_dLeftOffset));
  }
  //find DOM if right of center and less than center + right offset
  //(原文注释:val 在顶点右侧、且不超过"顶点+右宽"时求隶属度)——右边平顶段,隶属度恒=1。
  else if ( (val > m_dPeakPoint) && (val <= m_dPeakPoint+m_dRightOffset) )
  {
    return 1.0;
  }

  else
  {
    return 0;
  }
}