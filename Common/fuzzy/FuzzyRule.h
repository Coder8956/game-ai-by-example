//==============================================================================================
//【文件说明】FuzzyRule.h —— 一条模糊规则:IF 前提 THEN 结论
//
//【这个文件是干什么的?】
//  每条模糊规则就是一句话:"IF 距离很近 AND 血很少 THEN 快跑"。本类把"前提"和"结论"
//  各存一个模糊术语指针,规则触发时(Calculate)把前提的隶属度"或"进结论的隶属度,
//  代表"根据这条规则,结论成立到什么程度"。
//
//【谁在使用这个文件?】FuzzyModule::AddRule 创建规则,Defuzzify 时逐条 Calculate。
//
//【本文件包含了谁?】<vector>、FuzzySet.h、FuzzyOperators.h、misc/utils.h。
//==============================================================================================
#ifndef FUZZY_RULE_H
#define FUZZY_RULE_H
//-----------------------------------------------------------------------------
//
//  Name:   FuzzyRule.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   This class implements a fuzzy rule of the form:
//  
//          IF fzVar1 AND fzVar2 AND ... fzVarn THEN fzVar.c
//
//          
//-----------------------------------------------------------------------------
#include <vector>
#include "Fuzzy/FuzzySet.h"
#include "fuzzy/FuzzyOperators.h"
#include "misc/utils.h"


// ↓↓↓ 原文翻译【FuzzyRule —— 模糊规则】:实现形如 IF fzVar1 AND fzVar2 ... THEN fzVar.c
//   的模糊规则。作者:Mat Buckland。
//------------------------------------------------------------------------
// class FuzzyRule —— 一条模糊规则。
class FuzzyRule
{ 
private:

  //antecedent (usually a composite of several fuzzy sets and operators)
  //(原文注释:前提,通常是若干模糊集合和运算符组合而成)—— m_pAntecedent:IF 那一半。
  const FuzzyTerm*  m_pAntecedent;

  //consequence (usually a single fuzzy set, but can be several ANDed together)
  //(原文注释:结论,通常是单个模糊集合,也可以是几个 AND 起来)—— m_pConsequence:THEN 那一半。
  FuzzyTerm*        m_pConsequence;

  //it doesn't make sense to allow clients to copy rules
  //(原文注释:不允许客户复制规则)——下面两行把拷贝构造和赋值运算符声明为 private 且不实现,
  // 这样谁都不能复制规则(防止重复 delete)。
  FuzzyRule(const FuzzyRule&);
  FuzzyRule& operator=(const FuzzyRule&);


public:

  // 构造函数:传进来前提 ant、结论 con;各自 Clone() 一份存下来(克隆,不是直接拿指针)。
  FuzzyRule(const FuzzyTerm& ant,
            const FuzzyTerm& con):m_pAntecedent(ant.Clone()),
                                  m_pConsequence(con.Clone())
  {}

  ~FuzzyRule(){delete m_pAntecedent; delete m_pConsequence;}

  // 析构:删除前提和结论两个指针(它们是构造时 Clone 出来的,要自己释放)。
  // SetConfidenceOfConsequentToZero:把结论隶属度清零(每次去模糊前都先清零重来)。
  void SetConfidenceOfConsequentToZero(){m_pConsequence->ClearDOM();}

  //this method updates the DOM (the confidence) of the consequent term with
  //the DOM of the antecedent term. 
  // ↓↓↓ 上面两行英文注释的翻译:Calculate() 用前提的隶属度去更新结论的隶属度。
  //  具体就是结论.ORwithDOM(前提.GetDOM())——取较大者。
  void Calculate()
  {
    m_pConsequence->ORwithDOM(m_pAntecedent->GetDOM());
  }
};

#endif