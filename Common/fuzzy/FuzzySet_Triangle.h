//==============================================================================================
//【文件说明】FuzzySet_Triangle.h —— 三角形模糊集合
//
//【这个文件是干什么的?】
//  定义一种最常见的模糊集合形状:三角形。用"顶点位置 + 左半宽 + 右半宽"三个数确定。
//  在顶点处隶属度=1,往左右两边线性降到 0,超出范围就是 0。
//  例:"近距离"集合:顶点在 10 米,左右各宽 10 米——10 米时隶属度 1,0 米或 20 米时 0。
//
//【谁在使用这个文件?】FuzzyVariable::AddTriangularSet 创建它。
//
//【本文件包含了谁?】fuzzy/FuzzySet.h(父类)、misc/utils.h(isEqual 等工具函数)。
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
#include "misc/utils.h"



// ↓↓↓ 原文翻译【FuzzySetTriangle —— 三角形模糊集合】:一个简单类,定义三角形的模糊集合,
//   由一个中点、向左的偏移量、向右的偏移量确定。作者:Mat Buckland。
//------------------------------------------------------------------------
// class FuzzySet_Triangle : public FuzzySet —— 三角形模糊集合,继承自模糊集合基类。
class FuzzySet_Triangle : public FuzzySet
{
private:

  //the values that define the shape of this FLV
  //(原文注释:定义本语言变量形状的几个值)—— m_dPeakPoint=顶点;m_dLeftOffset=左半宽;
  //  m_dRightOffset=右半宽。
  double   m_dPeakPoint;
  double   m_dLeftOffset;
  double   m_dRightOffset;

public:
  
  // 构造函数:mid=顶点,lft=左半宽,rgt=右半宽;先调用父类构造把代表值设为 mid。
  FuzzySet_Triangle(double mid,
                    double lft,
                    double rgt):FuzzySet(mid), 
                               m_dPeakPoint(mid),
                               m_dLeftOffset(lft),
                               m_dRightOffset(rgt)
  {}

  //this method calculates the degree of membership for a particular value
  //(原文注释:计算某个具体值的隶属度)—— CalculateDOM:实现在 .cpp 里(纯虚的实现)。
  double CalculateDOM(double val)const;
};



#endif