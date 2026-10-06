//==============================================================================================
//【文件说明】FuzzySet_Shoulder.h —— 注意:本文件磁盘上的实际内容是一个三角形模糊集合
//
//【说明】这个文件名虽叫 Shoulder,但里面定义的类是 FuzzySet_Triangle(与 FuzzySet_Triangle.h
// 近似,构造函数多带一个名字参数,CalculateDOM 直接内联在头文件里)。这是原作者工程里
// 本来就有的样子,本注释按文件实际内容说明,不改源码。
//
//【三角形集合】顶点处隶属度=1,左右线性降到 0,范围外=0。
//【本文件包含了谁?】fuzzy/FuzzySet.h、utils.h。
//==============================================================================================
#ifndef FUZZYSET_TRIANGLE_H
#define FUZZYSET_TRIANGLE_H
//-----------------------------------------------------------------------------
//
//  Name:   FuzzySetTriangle.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   This is a simple class to define fuzzy sets that have a triangular 
//          shape and can be defined by a mid point, a left displacement and a
//          right displacement. 
//-----------------------------------------------------------------------------
#include "fuzzy/FuzzySet.h"
#include "utils.h"



// ↓↓↓ 原文翻译【FuzzySetTriangle —— 三角形模糊集合】:由一个中点、向左/向右偏移量确定的三角形。
//------------------------------------------------------------------------
// class FuzzySet_Triangle : public FuzzySet —— 三角形模糊集合。
class FuzzySet_Triangle : public FuzzySet
{
private:

  //the values that define the shape of this FLV
  //(原文注释:定义本集合形状的值)—— m_dMidPoint=顶点;m_dLeftOffset/m_dRightOffset=左右半宽。
  double   m_dMidPoint;
  double   m_dLeftOffset;
  double   m_dRightOffset;

public:
  
  // 构造函数:name=名字,mid=顶点,lft/rgt=左右半宽。
  FuzzySet_Triangle(std::string name,
                double mid,
                double lft,
                double rgt):FuzzySet(name),
                           m_dMidPoint(mid),
                           m_dLeftOffset(lft),
                           m_dRightOffset(rgt)
  {}

  //this method calculates the degree of membership for a particular value
  //(原文注释:计算某个具体值的隶属度)—— CalculateDOM,下面内联实现。
  inline double CalculateDOM(double val);

  //for a triangular set this is the range value at the midpoint
  //(原文注释:对三角集合来说,这就是中点处的那个值)—— RepresentativeValue:返回顶点。
  double RepresentativeValue()const{return m_dMidPoint;}
  
};

///////////////////////////////////////////////////////////////////////////////

inline
// CalculateDOM:算 val 的隶属度。逻辑与 FuzzySet_Triangle.cpp 相同:顶点=1,斜边线性降,范围外=0。
double FuzzySet_Triangle::CalculateDOM(double val)
{
  //test for the case where the triangle's left or right offsets are zero
  if ( (isEqual(m_dRightOffset, 0.0) && (isEqual(m_dMidPoint, val))) ||
       (isEqual(m_dLeftOffset, 0.0) && (isEqual(m_dMidPoint, val))) )
  {
    return 1.0;
  }

  //find DOM if left of center
  if ( (val <= m_dMidPoint) && (val > (m_dMidPoint - m_dLeftOffset)) )
  {
    double grad = 1.0 / m_dLeftOffset;

    return grad * (val - (m_dMidPoint - m_dLeftOffset));
  }
  //find DOM if right of center
  else if ( (val > m_dMidPoint) && (val < (m_dMidPoint + m_dRightOffset)) )
  {
    double grad = 1.0 / -m_dRightOffset;

    return grad * (val - m_dMidPoint) + 1.0;
  }
  //out of range of this FLV, return zero
  else
  {
    return 0.0;
  }

}

#endif