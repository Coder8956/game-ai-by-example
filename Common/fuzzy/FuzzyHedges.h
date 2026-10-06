//==============================================================================================
//【文件说明】FuzzyHedges.h —— 模糊"语气词":非常(Very)、比较(Fairly)
//
//【这个文件是干什么的?】
//  自然语言里有"非常近""比较近"这种程度词。本文件把它们也做成 FuzzyTerm:
//   FzVery(非常):隶属度 = 原隶属度的平方(变得更苛刻,0.7 变成 0.49);
//   FzFairly(比较):隶属度 = 原隶属度开平方(变得更宽松,0.49 变成 0.7)。
//  它们包着一个 FzSet,改一下隶属度的缩放方式而已。
//
//【谁在使用这个文件?】写规则时可包一层,如 FzVery(distance.Close)。
//【本文件包含了谁?】FuzzySet.h、FuzzyTerm.h、<math.h>(sqrt 开平方)。
//==============================================================================================
#ifndef FUZZY_HEDGES_H
#define FUZZY_HEDGES_H
//-----------------------------------------------------------------------------
//
//  Name:   FuzzyHedges.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   classes to implement fuzzy hedges 
//-----------------------------------------------------------------------------
#include "FuzzySet.h"
#include "FuzzyTerm.h"
#include <math.h>

// ↓↓↓ 原文翻译【FuzzyHedges —— 模糊语气词】:实现模糊语气词的类。作者:Mat Buckland。
//------------------------------------------------------------------------
// class FzVery : public FuzzyTerm —— "非常":把隶属度平方。
class FzVery : public FuzzyTerm
{
private:

  FuzzySet& m_Set;

  //prevent copying and assignment by clients
  FzVery(const FzVery& inst):m_Set(inst.m_Set){}
  FzVery& operator=(const FzVery&);
 

public:

  // 构造函数:ft 是被修饰的集合;内部记它的引用 m_Set。GetDOM 返回 m_Set.GetDOM() 的平方。
  FzVery(FzSet& ft):m_Set(ft.m_Set){}

  double GetDOM()const
  {
    return m_Set.GetDOM() * m_Set.GetDOM();
  }

  FuzzyTerm* Clone()const{return new FzVery(*this);}

  void ClearDOM(){m_Set.ClearDOM();}
  void ORwithDOM(double val){m_Set.ORwithDOM(val * val);}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// class FzFairly : public FuzzyTerm —— "比较":把隶属度开平方(sqrt)。
class FzFairly : public FuzzyTerm
{
private:

  FuzzySet& m_Set;

  //prevent copying and assignment
  FzFairly(const FzFairly& inst):m_Set(inst.m_Set){}
  FzFairly& operator=(const FzFairly&);

public:

  // 构造函数:ft 是被修饰的集合。GetDOM 返回 sqrt(m_Set.GetDOM())。
  FzFairly(FzSet& ft):m_Set(ft.m_Set){}

  double GetDOM()const
  {
    return sqrt(m_Set.GetDOM());
  }

  FuzzyTerm* Clone()const{return new FzFairly(*this);}

  void ClearDOM(){m_Set.ClearDOM();}
  void ORwithDOM(double val){m_Set.ORwithDOM(sqrt(val));}
};



#endif