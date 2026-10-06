//==============================================================================================
//【文件说明】FuzzyTerm.h —— 模糊规则里"一个术语"的抽象接口
//
//【这个文件是干什么的?】
//  模糊规则长这样:"IF 距离很近 AND 敌人很弱 THEN 开枪力度很大"。IF 后面(前提)和
//  THEN 后面(结论)都是一个"模糊术语"。本文件把"模糊术语"定成一份抽象接口:任何
//  术语(单个模糊集合、AND 组合、OR 组合、"非常/比较"修饰)都必须能做四件事:
//  克隆自己、取隶属度、清零隶属度、按或方式更新隶属度。
//
//【谁在使用这个文件?】FuzzySet(模糊集合本体)、FzSet(代理)、FzAND/FzOR(逻辑组合)、
//  FzVery/FzFairly(语气词),全部继承它;最终被 FuzzyRule(规则)使用。
//
//【C++ 小课堂:纯虚接口】virtual ... =0 表示"只定规矩不写实现";含纯虚函数的类不能
//  直接造对象,只能被继承。Clone() 叫"虚构造函数":让基类指针也能正确复制出子类对象。
//==============================================================================================
#ifndef FUZZYTERM_H
#define FUZZYTERM_H
//-----------------------------------------------------------------------------
//
//  Name:   FuzzyTerm.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   abstract class to provide an interface for classes able to be
//          used as terms in a fuzzy if-then rule base.
//-----------------------------------------------------------------------------

// ↓↓↓ 原文翻译【FuzzyTerm —— 模糊术语基类】:一个抽象类,为"能用作模糊 if-then 规则中
//   术语"的各类提供统一接口。作者:Mat Buckland。
//------------------------------------------------------------------------
// class FuzzyTerm —— 模糊术语抽象基类。
class FuzzyTerm
{  
public:

  virtual ~FuzzyTerm(){}

  //all terms must implement a virtual constructor
  //(原文注释:所有术语都必须实现一个"虚构造函数")—— Clone():复制出一份自己。
  virtual FuzzyTerm* Clone()const = 0;

  //retrieves the degree of membership of the term
  //(原文注释:取回本术语的隶属度)—— GetDOM():返回当前隶属度(0~1,0=完全不属于,1=完全属于)。
  virtual double      GetDOM()const=0;

  //clears the degree of membership of the term
  //(原文注释:清空本术语的隶属度)—— ClearDOM():把隶属度重置为 0。
  virtual void       ClearDOM()=0;

  //method for updating the DOM of a consequent when a rule fires
  //(原文注释:规则触发时,用来更新结论术语隶属度的方法)—— ORwithDOM(val):
  //  取"现有隶属度"与 val 的较大值(模糊逻辑里"或"=取最大)。
  virtual void       ORwithDOM(double val)=0;

   
};



#endif