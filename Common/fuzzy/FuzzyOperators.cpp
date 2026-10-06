//==============================================================================================
//【文件说明】FuzzyOperators.cpp —— FzAND / FzOR 的实现
//
//【要点】FzAND 的 GetDOM 返回所有子项隶属度里最小的;FzOR 返回最大的。
//【谁包含它】FuzzyOperators.h。
#include "FuzzyOperators.h"
 
///////////////////////////////////////////////////////////////////////////////
//
//  implementation of FzAND
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// ↓↓↓ FzAND 的实现开始。
// FzAND::FzAND(const FzAND& fa):拷贝构造函数——把 fa 的每个子术语 Clone 一份装进自己。
FzAND::FzAND(const FzAND& fa)
{
   std::vector<FuzzyTerm*>::const_iterator curTerm;
   for (curTerm = fa.m_Terms.begin(); curTerm != fa.m_Terms.end(); ++curTerm)
   {
     m_Terms.push_back((*curTerm)->Clone());
   }
}
   
  //ctor using two terms
FzAND::FzAND(FuzzyTerm& op1, FuzzyTerm& op2)
{
   m_Terms.push_back(op1.Clone());
   m_Terms.push_back(op2.Clone());
}

//ctor using three terms
FzAND::FzAND(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3)
{
   m_Terms.push_back(op1.Clone());
   m_Terms.push_back(op2.Clone());
   m_Terms.push_back(op3.Clone());
}

      //ctor using four terms
FzAND::FzAND(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3, FuzzyTerm& op4)
{
   m_Terms.push_back(op1.Clone());
   m_Terms.push_back(op2.Clone());
   m_Terms.push_back(op3.Clone());
   m_Terms.push_back(op4.Clone());
}


FzAND::~FzAND()
{
  std::vector<FuzzyTerm*>::iterator curTerm;
  for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
  {
    delete *curTerm;
  }
}
  

//--------------------------- GetDOM ------------------------------------------
//
//  the AND operator returns the minimum DOM of the sets it is operating on
//-----------------------------------------------------------------------------
// FzAND::GetDOM:AND 的隶属度 = 所有子项里最小的那个。smallest 初始设成最大浮点数,
// 然后逐个比较,谁小取谁。
double FzAND::GetDOM()const
{
  double smallest = MaxDouble;

  std::vector<FuzzyTerm*>::const_iterator curTerm;
  for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
  {
    if ((*curTerm)->GetDOM() < smallest)
    {
      smallest = (*curTerm)->GetDOM();
    }
  }

  return smallest;
}


//------------------------- ORwithDOM -----------------------------------------
// FzAND::ORwithDOM:把 val 分发到每个子项(因为 AND 的结论要同时影响每个子项)。
void FzAND::ORwithDOM(double val)
{
  std::vector<FuzzyTerm*>::iterator curTerm;
  for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
  {
    (*curTerm)->ORwithDOM(val);
  }
}

//---------------------------- ClearDOM ---------------------------------------
// FzAND::ClearDOM:逐个把每个子项的隶属度清零。
void FzAND::ClearDOM()
{
  std::vector<FuzzyTerm*>::iterator curTerm;
  for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
  {
    (*curTerm)->ClearDOM();
  }
}

///////////////////////////////////////////////////////////////////////////////
//
//  implementation of FzOR
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// ↓↓↓ FzOR 的实现开始。
// FzOR::FzOR(const FzOR& fa):拷贝构造——把 fa 的每个子术语 Clone 一份装进自己。
FzOR::FzOR(const FzOR& fa)
{
   std::vector<FuzzyTerm*>::const_iterator curTerm;
   for (curTerm = fa.m_Terms.begin(); curTerm != fa.m_Terms.end(); ++curTerm)
   {
     m_Terms.push_back((*curTerm)->Clone());
   }
}
   
  //ctor using two terms
FzOR::FzOR(FuzzyTerm& op1, FuzzyTerm& op2)
{
   m_Terms.push_back(op1.Clone());
   m_Terms.push_back(op2.Clone());
}

    //ctor using three terms
FzOR::FzOR(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3)
{
   m_Terms.push_back(op1.Clone());
   m_Terms.push_back(op2.Clone());
   m_Terms.push_back(op3.Clone());
}

      //ctor using four terms
FzOR::FzOR(FuzzyTerm& op1, FuzzyTerm& op2, FuzzyTerm& op3, FuzzyTerm& op4)
{
   m_Terms.push_back(op1.Clone());
   m_Terms.push_back(op2.Clone());
   m_Terms.push_back(op3.Clone());
   m_Terms.push_back(op4.Clone());
}


FzOR::~FzOR()
{
  std::vector<FuzzyTerm*>::iterator curTerm;
  for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
  {
    delete *curTerm;
  }
}
  

//--------------------------- GetDOM ------------------------------------------
//
//  the OR operator returns the maximum DOM of the sets it is operating on
//----------------------------------------------------------------------------- 
// FzOR::GetDOM:OR 的隶属度 = 所有子项里最大的那个。largest 初始设成最小浮点数,逐个比大。
double FzOR::GetDOM()const
{
  double largest = MinFloat;

  std::vector<FuzzyTerm*>::const_iterator curTerm;
  for (curTerm = m_Terms.begin(); curTerm != m_Terms.end(); ++curTerm)
  {
    if ((*curTerm)->GetDOM() > largest)
    {
      largest = (*curTerm)->GetDOM();
    }
  }

  return largest;
}
