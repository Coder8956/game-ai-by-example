//==============================================================================================
//【文件说明】FuzzySet_Singleton.h —— 单点模糊集合
//
//【这个文件是干什么的?】
//  最简单的模糊集合:一个小矩形区间。val 落在 [中点-左宽, 中点+右宽] 内隶属度就=1.0,
//  之外=0。像一个"非此即彼"的窄条,常用于输出端的单点。
//
//【谁在使用这个文件?】FuzzyVariable::AddSingletonSet。
//【本文件包含了谁?】fuzzy/FuzzySet.h、misc/utils.h。
//==============================================================================================
#ifndef FUZZYSET_SINGLETON_H
#define FUZZYSET_SINGLETON_H
//-----------------------------------------------------------------------------
//
//  Name:   FuzzySet_Singleton.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   This defines a fuzzy set that is a singleton (a range
//          over which the DOM is always 1.0)
//-----------------------------------------------------------------------------
#include "fuzzy/FuzzySet.h"
#include "misc/utils.h"



// ↓↓↓ 原文翻译【FuzzySet_Singleton —— 单点模糊集合】:定义一个"单点"模糊集合,即在一个
//   区间上隶属度恒为 1.0。作者:Mat Buckland。
//------------------------------------------------------------------------
// class FuzzySet_Singleton : public FuzzySet —— 单点模糊集合。
class FuzzySet_Singleton : public FuzzySet
{
private:

    //the values that define the shape of this FLV
  //(原文注释:定义本集合形状的值)—— m_dMidPoint=中点;m_dLeftOffset/m_dRightOffset=左右半宽。
  double   m_dMidPoint;
  double   m_dLeftOffset;
  double   m_dRightOffset;

public:
  
  // 构造函数:mid=中点,lft=左宽,rgt=右宽。
  FuzzySet_Singleton(double       mid,
                     double       lft,
                     double       rgt):FuzzySet(mid),
                                      m_dMidPoint(mid),
                                      m_dLeftOffset(lft),
                                      m_dRightOffset(rgt)
  {}

  //this method calculates the degree of membership for a particular value
  //(原文注释:计算某个具体值的隶属度)—— CalculateDOM,实现在 .cpp。
  double     CalculateDOM(double val)const; 
};


#endif