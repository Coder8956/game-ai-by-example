//==============================================================================================
//【文件说明】FuzzyModule.cpp —— 模糊模块的实现(析构/加规则/建变量/打印)
//【谁包含它】Fuzzy/FuzzyModule.h。
//==============================================================================================
#pragma warning (disable:4786)
#include <stdarg.h>
#include <iostream>
#include <cassert>

#include "Fuzzy/FuzzyModule.h"

//------------------------------ dtor -----------------------------------------
//------------------------------ 析构函数 -----------------------------------------
// ~FuzzyModule:先把所有模糊变量 delete 掉,再把所有规则 delete 掉(都用 new 出来的)。
FuzzyModule::~FuzzyModule()
{
  VarMap::iterator curVar = m_Variables.begin();
  for (curVar; curVar != m_Variables.end(); ++curVar)
  {
    delete curVar->second;
  }

  std::vector<FuzzyRule*>::iterator curRule = m_Rules.begin();
  for (curRule; curRule != m_Rules.end(); ++curRule)
  {
    delete *curRule;
  }
}

//----------------------------- AddRule ---------------------------------------
//----------------------------- AddRule 加规则 ---------------------------------------
// AddRule:new 一条 FuzzyRule(前件, 后件),加进 m_Rules 列表。
void FuzzyModule::AddRule(FuzzyTerm& antecedent, FuzzyTerm& consequence)
{
  m_Rules.push_back(new FuzzyRule(antecedent, consequence));
}

 
//-------------------------- CreateFLV ---------------------------
//
//  creates a new fuzzy variable and returns a reference to it
//-----------------------------------------------------------------------------
//-------------------------- CreateFLV 建语言变量 ---------------------------
// ↓↓↓ 上面两行英文注释的翻译:新建一个模糊变量,返回它的引用。
FuzzyVariable& FuzzyModule::CreateFLV(const std::string& VarName)
{
  m_Variables[VarName] = new FuzzyVariable();;

  return *m_Variables[VarName];
}


//---------------------------- WriteAllDOMs -----------------------------------
//---------------------------- WriteAllDOMs 打印全部隶属度 -----------------------------------
// 遍历所有变量,调各自的 WriteDOMs,打印每个变量当前各集合的隶属度(调试用)。
std::ostream& FuzzyModule::WriteAllDOMs(std::ostream& os)
{
  os << "\n\n";
  
  VarMap::iterator curVar = m_Variables.begin();
  for (curVar; curVar != m_Variables.end(); ++curVar)
  {
    os << "\n--------------------------- " << curVar->first;;
    curVar->second->WriteDOMs(os);

    os << std::endl;
  }

  return os;
}

