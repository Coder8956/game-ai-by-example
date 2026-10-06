//==============================================================================================
//【文件说明】FuzzyOperators.h —— 模糊逻辑 AND / OR 运算符
//
//【这个文件是干什么的?】
//  规则里要写 "A AND B"、"A OR B"。本文件把 AND、OR 也做成 FuzzyTerm 子类:
//  FzAND:隶属度=它所有子项里最小的那个(模糊逻辑"与"=取最小);
//  FzOR:隶属度=它所有子项里最大的那个(模糊逻辑"或"=取最大)。
//
//【谁在使用这个文件?】写规则时用,如 AddRule( FzAND(distance.Close, health.Low), aim.High )。
//【本文件包含了谁?】<vector>、<cassert>、misc/utils.h、FuzzyTerm.h。
//==============================================================================================
#ifndef FUZZY_OPERATORS_H
#define FUZZY_OPERATORS_H
//-----------------------------------------------------------------------------
//
//  Name:   FuzzyOperators.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   classes to provide the fuzzy AND and OR operators to be used in
//          the creation of a fuzzy rule base
//-----------------------------------------------------------------------------
#include <vector>
#include <cassert>
#include "misc/utils.h"
#include "FuzzyTerm.h"

///////////////////////////////////////////////////////////////////////////////
//
//  a fuzzy AND operator class
//
///////////////////////////////////////////////////////////////////////////////
// ↓↓↓ 原文翻译【FuzzyOperators —— 模糊运算符】:为模糊规则库提供 AND、OR 运算符。
//------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
// ↓↓↓ 下面是"模糊 AND 运算符类"。class FzAND : public FuzzyTerm。
class FzAND : public FuzzyTerm
{
private:

  //an instance of this class may AND together up to 4 terms
  std::vector<FuzzyTerm*> m_Terms;

  //disallow assignment
  FzAND& operator=(const FzAND&);
  //(原文注释:一个本类实例最多把 4 个术语 AND 在一起)—— m_Terms:装子术语指针的 vector。

public:

  ~FzAND();

  //copy ctor
  FzAND(const FzAND& fa);
   
  //ctors accepting fuzzy terms.
  // 下面三个构造函数分别接收 2/3/4 个术语;每个都 Clone 一份存进 m_Terms。
  FzAND(FuzzyTerm& op1, FuzzyTerm& op2);
  FzAND(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3);
  FzAND(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3, FuzzyTerm& op4);

  //virtual ctor
  FuzzyTerm* Clone()const{return new FzAND(*this);}
  
  double GetDOM()const;
  void  ClearDOM();
  void  ORwithDOM(double val);
};


///////////////////////////////////////////////////////////////////////////////
//
//  a fuzzy OR operator class
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// ↓↓↓ 下面是"模糊 OR 运算符类"。class FzOR : public FuzzyTerm。
class FzOR : public FuzzyTerm
{
private:

  //an instance of this class may AND together up to 4 terms
  std::vector<FuzzyTerm*> m_Terms;

  //no assignment op necessary
  FzOR& operator=(const FzOR&);
  //(原文注释:一个本类实例最多组合 4 个术语)—— m_Terms:装子术语指针的 vector。

public:

  ~FzOR();

  //copy ctor
  FzOR(const FzOR& fa);
   
  //ctors accepting fuzzy terms.
  // 下面三个构造函数分别接收 2/3/4 个术语;每个都 Clone 一份存进 m_Terms。
  FzOR(FuzzyTerm& op1, FuzzyTerm& op2);
  FzOR(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3);
  FzOR(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3, FuzzyTerm& op4);

  //virtual ctor
  FuzzyTerm* Clone()const{return new FzOR(*this);}
  
  double GetDOM()const;

  //unused
  //(原文注释:未使用)—— FzOR 只当前提用,不会被 ClearDOM/ORwithDOM 调用,
  //  一旦误调就 assert(0) 报错。
  void ClearDOM(){assert(0 && "<FzOR::ClearDOM>: invalid context");}
  void ORwithDOM(double val){assert(0 && "<FzOR::ORwithDOM>: invalid context");}
};



#endif

