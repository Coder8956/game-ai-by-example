//==============================================================================================
//【文件说明】FzSet.h —— 模糊集合的"代理",让它能当规则术语用
//
//【这个文件是干什么的?】
//  FuzzySet 本身不是 FuzzyTerm,不能直接写进规则。FzSet 是一个薄薄的代理(wrapper):
//  内部包着一个 FuzzySet 引用,对外却冒充 FuzzyTerm(继承它)。这样"AddTriangularSet
//  返回的东西"就能直接出现在 IF/THEN 里。
//
//【谁在使用这个文件?】FuzzyVariable 各 AddXxxSet 返回 FzSet;FuzzyHedges(FzVery 等)是它的友元。
//【本文件包含了谁?】FuzzyTerm.h、FuzzySet.h。
//==============================================================================================
#ifndef PROXY_FUZZY_SET
#define PROXY_FUZZY_SET
//-----------------------------------------------------------------------------
//
//  Name:   FzSet.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to provide a proxy for a fuzzy set. The proxy inherits from
//          FuzzyTerm and therefore can be used to create fuzzy rules
//-----------------------------------------------------------------------------
#include "FuzzyTerm.h"
#include "FuzzySet.h"

// ↓↓↓ 原文翻译【FzSet —— 模糊集合代理】:给模糊集合提供一个代理类。代理继承自 FuzzyTerm,
//   因此可用来构造模糊规则。作者:Mat Buckland。
//------------------------------------------------------------------------
// class FzAND; 前置声明:后面友元会用到,这里先打个招呼。
class FzAND;

// class FzSet : public FuzzyTerm —— 模糊集合代理,继承模糊术语接口。
class FzSet : public FuzzyTerm
{
private:
  
  //let the hedge classes be friends 
  //(原文注释:让"语气词"类当友元)—— FzVery/FzFairly 能直接访问本代理内部的 m_Set。
  friend class FzVery;
  friend class FzFairly;

private:

  //a reference to the fuzzy set this proxy represents
  //(原文注释:本代理所代表的那个模糊集合的引用)—— m_Set:真正干活的集合。
  FuzzySet& m_Set;

public:

  // 构造函数:把真正的模糊集合 fs 包进来。下面四个函数都是"转发"给 m_Set 做。
  FzSet(FuzzySet& fs):m_Set(fs){}

  FuzzyTerm* Clone()const{return new FzSet(*this);}
  double     GetDOM()const {return m_Set.GetDOM();}
  void       ClearDOM(){m_Set.ClearDOM();}
  void       ORwithDOM(double val){m_Set.ORwithDOM(val);}
};


#endif

