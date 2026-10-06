//==============================================================================================
//【文件说明】FuzzySet.h —— 模糊集合基类
//
//【这个文件是干什么的?】
//  普通集合里一个数要么属于、要么不属于。模糊集合里,一个数"有百分之多少属于"它,
//  这个比例叫隶属度(DOM,degree of membership,0~1)。比如距离=15米,对"近距离"这个
//  模糊集合的隶属度可能是 0.7。本文件是所有模糊集合形状(三角/左肩/右肩/单点)的爸爸类,
//  保存当前隶属度 m_dDOM 和一个"代表值" m_dRepresentativeValue。
//
//【谁在使用这个文件?】FuzzySet_Triangle/LeftShoulder/RightShoulder/Singleton 等具体形状,
//  以及 FuzzyVariable(语言变量由若干模糊集合组成)。
//
//【本文件包含了谁?】<string>、<cassert>。
//==============================================================================================
#ifndef FUZZYSET_H
#define FUZZYSET_H
//-----------------------------------------------------------------------------
//
//  Name:   FuzzySet.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to define an interface for a fuzzy set
//-----------------------------------------------------------------------------
// ↓↓↓ 原文翻译【FuzzySet —— 模糊集合】:定义模糊集合接口的类。作者:Mat Buckland。
//------------------------------------------------------------------------
#include <string>
#include <cassert>


// class FuzzySet —— 模糊集合基类。protected 表示"只有我和我儿子们能直接用"。
class FuzzySet
{
protected:
  
  //this will hold the degree of membership of a given value in this set 
  //(原文注释:保存某个给定值在本集合中的隶属度)—— m_dDOM:当前隶属度(0~1)。
  double        m_dDOM;

  //this is the maximum of the set's membership function. For instamce, if
  //the set is triangular then this will be the peak point of the triangular.
  //if the set has a plateau then this value will be the mid point of the 
  //plateau. This value is set in the constructor to avoid run-time
  //calculation of mid-point values.
  // ↓↓↓ 上面五行英文注释的翻译:m_dRepresentativeValue 是本集合"隶属函数最高点"对应的
  //   那个数值。比如三角集合就是顶点;有平顶的集合就是平顶中点。构造时就定好,免得运行时算。
  //   去模糊化(重心法/最大平均法)时要拿它当"这个集合代表的数值"。
  double        m_dRepresentativeValue;

public:

  // 构造函数:传入代表值 RepVal;隶属度 m_dDOM 先置 0.0。
  FuzzySet(double RepVal):m_dDOM(0.0), m_dRepresentativeValue(RepVal){}

  //return the degree of membership in this set of the given value. NOTE,
  //this does not set m_dDOM to the DOM of the value passed as the parameter.
  //This is because the centroid defuzzification method also uses this method
  //to determine the DOMs of the values it uses as its sample points.
  // ↓↓↓ 上面四行英文注释的翻译:CalculateDOM(val) 计算"给定值 val 在本集合中的隶属度",
  //   是纯虚函数,由各形状子类实现。注意:它只读、不修改 m_dDOM(因为重心法去模糊时
  //   还要反复用它算各个采样点的隶属度)。
  virtual double      CalculateDOM(double val)const = 0;

  //if this fuzzy set is part of a consequent FLV, and it is fired by a rule 
  //then this method sets the DOM (in this context, the DOM represents a
  //confidence level)to the maximum of the parameter value or the set's 
  //existing m_dDOM value
  // ↓↓↓ 上面四行英文注释的翻译:ORwithDOM(val)。若本集合是某条规则的结论,且该规则被触发,
  //   就把结论的隶属度设为"val 与现有 m_dDOM 的较大值"(这里 DOM 表示"置信度")。
  void               ORwithDOM(double val){if (val > m_dDOM) m_dDOM = val;}

  //(原文注释:访问器方法)——下面是 GetRepresentativeVal/ClearDOM/GetDOM/SetDOM。
  //accessor methods
  double             GetRepresentativeVal()const{return m_dRepresentativeValue;}
  
  void               ClearDOM(){m_dDOM = 0.0;}  
  double             GetDOM()const{return m_dDOM;}
  void               SetDOM(double val)
  {
    assert ((val <=1) && (val >= 0) && "<FuzzySet::SetDOM>: invalid value");
    m_dDOM = val;
  }
};


#endif