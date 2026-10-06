//==============================================================================================
//【文件说明】FuzzySet_Triangle.cpp —— 三角形模糊集合的 CalculateDOM 实现
//
//【本文件干什么?】算给定值 val 的隶属度:在顶点处=1,左右斜边线性降到 0,范围外=0。
//【谁包含它】FuzzySet_Triangle.h。
#include "FuzzySet_Triangle.h"


// CalculateDOM:计算 val 在本三角集合中的隶属度(0~1)。:: 是"作用域解析符",
// 表示这个函数属于 FuzzySet_Triangle 类。
double FuzzySet_Triangle::CalculateDOM(double val)const
{
  //test for the case where the triangle's left or right offsets are zero
  //(to prevent divide by zero errors below)
  // ↓↓↓ 上面两行英文注释的翻译:先测试"左半宽或右半宽为 0"的极端情况
  //   (防止下面除以零)。若半宽为 0 且 val 正好等于顶点,隶属度直接=1.0。
  if ( (isEqual(m_dRightOffset, 0.0) && (isEqual(m_dPeakPoint, val))) ||
       (isEqual(m_dLeftOffset, 0.0) && (isEqual(m_dPeakPoint, val))) )
  {
    return 1.0;
  }

  //find DOM if left of center
  //(原文注释:val 在顶点左侧时,求隶属度)——从左端点线性升到顶点:斜率=1/左宽。
  if ( (val <= m_dPeakPoint) && (val >= (m_dPeakPoint - m_dLeftOffset)) )
  {
    double grad = 1.0 / m_dLeftOffset;

    return grad * (val - (m_dPeakPoint - m_dLeftOffset));
  }
  //find DOM if right of center
  //(原文注释:val 在顶点右侧时,求隶属度)——从顶点线性降到右端点:斜率=1/(-右宽)。
  else if ( (val > m_dPeakPoint) && (val < (m_dPeakPoint + m_dRightOffset)) )
  {
    double grad = 1.0 / -m_dRightOffset;

    return grad * (val - m_dPeakPoint) + 1.0;
  }
  //out of range of this FLV, return zero
  //(原文注释:超出本语言变量范围,返回 0)——既不在左也不在右,隶属度=0。
  else
  {
    return 0.0;
  }
}