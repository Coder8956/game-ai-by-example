//==============================================================================================
//【文件说明】FuzzyModule.h —— 模糊逻辑"整机":把变量、集合、规则组装起来
//
//【模糊推理一次完整的数据流(用户请求重点)】
//
//   真实输入值(如距离=15米)
//        │
//        ▼  ① Fuzzify 模糊化:FuzzyVariable 算它在"近/中/远"各集合里的隶属度
//   ┌──────────────────┐
//   │ 近=0.7 中=0.3 远=0│   (每个集合一个 0~1 的隶属度)
//   └──────────────────┘
//        │
//        ▼  ② 规则推理:逐条 IF-THEN 规则 Calculate,把前件隶属度或进后件
//   ┌──────────────────┐
//   │ 输出集合 aim.High=0.7, aim.Low=0.3 │  (多条规则命中同一后件时取最大)
//   └──────────────────┘
//        │
//        ▼  ③ DeFuzzify 去模糊化:把模糊结论压成一个精确数字(重心法 MaxAv/Centroid)
//   精确输出值(如瞄准强度=0.55)
//
//【谁在使用这个文件?】Raven 各武器类(Weapon_Blaster/ShotGun/RailGun/RocketLauncher),
//   根据距离等输入,用模糊逻辑算出瞄准/喷射精度等输出。
//
//【本文件包含了谁?】<vector>/<string>/<map>/<iosfwd>、FuzzySet.h、FuzzyVariable.h、
//   FuzzyRule.h、FuzzyOperators.h、FzSet.h、FuzzyHedges.h。
//==============================================================================================
#ifndef FUZZY_MODULE_H
#define FUZZY_MODULE_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   FuzzyModule.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   this class describes a fuzzy module: a collection of fuzzy variables
//          and the rules that operate on them.
//
//-----------------------------------------------------------------------------
// ↓↓↓ 原文翻译【FuzzyModule —— 模糊模块】:一个模糊模块 = 若干模糊变量 + 作用于它们的规则。
//   作者:Mat Buckland。
//------------------------------------------------------------------------
#include <vector>
#include <string>
#include <map>
#include <iosfwd>

#include "FuzzySet.h"
#include "FuzzyVariable.h"
#include "FuzzyRule.h"
#include "FuzzyOperators.h"
#include "FzSet.h"
#include "FuzzyHedges.h"



// class FuzzyModule —— 模糊逻辑整机。
class FuzzyModule
{
private:

  typedef std::map<std::string, FuzzyVariable*> VarMap;
  
public:

  //you must pass one of these values to the defuzzify method. This module
  //only supports the MaxAv and centroid methods.
  // ↓↓↓ 上面两行英文注释的翻译:调用 DeFuzzify 时必须从下面两个里选一个去模糊方法。
  // enum DefuzzifyMethod{max_av, centroid}:枚举——max_av=最大平均法;centroid=重心法。
  enum DefuzzifyMethod{max_av, centroid};

  //when calculating the centroid of the fuzzy manifold this value is used
  //to determine how many cross-sections should be sampled
  // ↓↓↓ 上面两行英文注释的翻译:算重心时,用来决定把范围切成多少段采样。
  // enum {NumSamples = 15}:匿名枚举,采样段数=15。
  enum {NumSamples = 15};

private:

  //a map of all the fuzzy variables this module uses
  //(原文注释:本模块用到的所有模糊变量的字典)—— m_Variables:名字→变量指针。
  VarMap                   m_Variables;

  //a vector containing all the fuzzy rules
  //(原文注释:装着所有模糊规则的 vector)—— m_Rules:规则指针列表。
  std::vector<FuzzyRule*>   m_Rules;
 

  //zeros the DOMs of the consequents of each rule. Used by Defuzzify()
  //(原文注释:把每条规则后件的隶属度清零,供 DeFuzzify 用)—— SetConfidencesOfConsequentsToZero。
  inline void SetConfidencesOfConsequentsToZero();


public:

  ~FuzzyModule();

  //creates a new 'empty' fuzzy variable and returns a reference to it.
  //(原文注释:新建一个空的模糊变量,返回它的引用)—— CreateFLV。
  FuzzyVariable&  CreateFLV(const std::string& VarName);
  
  //adds a rule to the module
  //(原文注释:往模块里加一条规则)—— AddRule(前件, 后件)。
  void            AddRule(FuzzyTerm& antecedent, FuzzyTerm& consequence);

  //this method calls the Fuzzify method of the named FLV 
  //(原文注释:调用指定名字变量的 Fuzzify)—— Fuzzify:模糊化入口。
  inline void     Fuzzify(const std::string& NameOfFLV, double val);

  //given a fuzzy variable and a deffuzification method this returns a 
  //crisp value
  // ↓↓↓ 上面两行英文注释的翻译:给定一个变量和去模糊方法,返回一个精确值。
  // DeFuzzify:去模糊化入口,默认用 max_av。
  inline double    DeFuzzify(const std::string& key,
                            DefuzzifyMethod    method = max_av);
    
  
  //writes the DOMs of all the variables in the module to an output stream
  //(原文注释:把模块里所有变量的隶属度写到输出流,调试用)—— WriteAllDOMs。
  std::ostream&   WriteAllDOMs(std::ostream& os);

};

///////////////////////////////////////////////////////////////////////////////

//----------------------------- Fuzzify ---------------------------------------
//
//  this method calls the Fuzzify method of the variable with the same name
//  as the key
//-----------------------------------------------------------------------------
//----------------------------- Fuzzify 模糊化 ---------------------------------------
// ↓↓↓ 上面两行英文注释的翻译:按名字找到变量,调它的 Fuzzify(val)。
inline void FuzzyModule::Fuzzify(const std::string& NameOfFLV, double val)
{
  //first make sure the key exists
  assert ( (m_Variables.find(NameOfFLV) != m_Variables.end()) &&
          "<FuzzyModule::Fuzzify>:key not found");

  m_Variables[NameOfFLV]->Fuzzify(val);
}

//---------------------------- DeFuzzify --------------------------------------
//
//  given a fuzzy variable and a deffuzification method this returns a 
//  crisp value
//-----------------------------------------------------------------------------
inline double
//---------------------------- DeFuzzify 去模糊 --------------------------------------
// ↓↓↓ 上面两行英文注释的翻译:给定变量和方法,返回精确值。四步:
//   ① 先把所有后件隶属度清零;② 逐条规则 Calculate(推理);③ 按选的方法去模糊。
FuzzyModule::DeFuzzify(const std::string& NameOfFLV, DefuzzifyMethod method)
{
  //first make sure the key exists
  assert ( (m_Variables.find(NameOfFLV) != m_Variables.end()) &&
          "<FuzzyModule::DeFuzzifyMaxAv>:key not found");

  //clear the DOMs of all the consequents of all the rules
  SetConfidencesOfConsequentsToZero();

  //process the rules
  std::vector<FuzzyRule*>::iterator curRule = m_Rules.begin();
  for (curRule; curRule != m_Rules.end(); ++curRule)
  {
    (*curRule)->Calculate();
  }

  //now defuzzify the resultant conclusion using the specified method
  switch (method)
  {
  case centroid:

    return m_Variables[NameOfFLV]->DeFuzzifyCentroid(NumSamples);

    break;

  case max_av:

    return m_Variables[NameOfFLV]->DeFuzzifyMaxAv();

    break;
  }

  return 0;
}



//-------------------------- ClearConsequents ---------------------------------
//
//  zeros the DOMs of the consequents of each rule
//-----------------------------------------------------------------------------
//-------------------------- SetConfidencesOfConsequentsToZero ---------------------------------
// ↓↓↓ 上面两行英文注释的翻译:把每条规则后件的隶属度清零。
inline void FuzzyModule::SetConfidencesOfConsequentsToZero()
{
  std::vector<FuzzyRule*>::iterator curRule = m_Rules.begin();
  for (curRule; curRule != m_Rules.end(); ++curRule)
  {
    (*curRule)->SetConfidenceOfConsequentToZero();
  }
}


#endif