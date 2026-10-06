//==============================================================================================
//【文件说明】FuzzyVariable.h —— 模糊语言变量(FLV)
//
//【这个文件是干什么的?】
//  一个"语言变量"就是一个具体概念,比如"距离",它下面挂好几个模糊集合:
//  很近 / 中等 / 很远。本类负责:往里加各种形状的集合(Fuzzify 模糊化:给一个真实数值,
//  算出它在每个集合里的隶属度;DeFuzzify 去模糊:把一堆模糊结论压成一个具体数字输出。
//
//【谁在使用这个文件?】FuzzyModule::CreateFLV 创建它。
//【本文件包含了谁?】<map>、<iosfwd>、<string>;前置声明 FuzzySet/FzSet/FuzzyModule。
//==============================================================================================
#ifndef FLV_H
#define FLV_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   FuzzyVariable.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   Class to define a fuzzy linguistic variable (FLV).
//
//          An FLV comprises of a number of fuzzy sets  
//
//-----------------------------------------------------------------------------
#include <map>
#include <iosfwd>
#include <string>

// ↓↓↓ 原文翻译【FuzzyVariable —— 模糊语言变量 FLV】:定义模糊语言变量的类,一个 FLV
//   由若干个模糊集合组成。作者:Mat Buckland。
//------------------------------------------------------------------------
// 前置声明三个类(只用指针/引用,不必展开完整定义)。
class FuzzySet;
class FzSet;
class FuzzyModule;


// class FuzzyVariable —— 模糊语言变量。
class FuzzyVariable
{
private:
  
  // typedef:给"字符串→模糊集合指针 的 map"起个短名字 MemberSets。
  // map 像字典:用名字(如"很近")就能取出对应集合。
  typedef std::map<std::string, FuzzySet*>  MemberSets;
    
private:
  
  //disallow copies
  //(原文注释:不允许拷贝)——把拷贝构造/赋值声明为 private 且不实现。
  FuzzyVariable(const FuzzyVariable&);
  FuzzyVariable& operator=(const FuzzyVariable&);

private:
 
  //a map of the fuzzy sets that comprise this variable
  //(原文注释:组成本变量的那些模糊集合的字典)—— m_MemberSets:名字→集合。
  MemberSets   m_MemberSets;

  //the minimum and maximum value of the range of this variable
  //(原文注释:本变量取值范围的最小/最大值)—— m_dMinRange/m_dMaxRange。
  double        m_dMinRange;
  double        m_dMaxRange;
  

  // AdjustRangeToFit:每加一个集合,就用它的上下界把整体范围扩到能覆盖。
  //this method is called with the upper and lower bound of a set each time a
  //new set is added to adjust the upper and lower range values accordingly
  void AdjustRangeToFit(double min, double max);

  //a client retrieves a reference to a fuzzy variable when an instance is
  //created via FuzzyModule::CreateFLV(). To prevent the client from deleting
  //the instance the FuzzyVariable destructor is made private and the 
  //FuzzyModule class made a friend.
  // ↓↓↓ 上面四行英文注释的翻译:客户通过 FuzzyModule::CreateFLV 拿到本类引用。
  //   为防止客户自己 delete 掉实例,析构函数被设成 private,并把 FuzzyModule 设为友元。
  ~FuzzyVariable();

  friend class FuzzyModule;


public:

  FuzzyVariable():m_dMinRange(0.0),m_dMaxRange(0.0){}
  
  //the following methods create instances of the sets named in the method
  //name and add them to the member set map. Each time a set of any type is
  //added the m_dMinRange and m_dMaxRange are adjusted accordingly. All of the
  //methods return a proxy class representing the newly created instance. This
  //proxy set can be used as an operand when creating the rule base.
  // ↓↓↓ 上面五行英文注释的翻译:下面几个方法分别创建左肩/右肩/三角/单点集合,
  //   加进字典,并返回一个 FzSet 代理(可直接拿去写规则)。每加一个都会调整取值范围。
  FzSet  AddLeftShoulderSet(std::string name, double minBound, double peak, double maxBound);

  FzSet  AddRightShoulderSet(std::string name, double minBound, double peak, double maxBound);

  FzSet  AddTriangularSet(std::string name,
                             double       minBound,
                             double       peak,
                             double       maxBound);

  FzSet  AddSingletonSet(std::string name,
                            double       minBound,
                            double       peak,
                            double       maxBound);
  
  
  //(原文注释:模糊化——给一个真实值,算出它在每个子集里的隶属度)—— Fuzzify。
  //fuzzify a value by calculating its DOM in each of this variable's subsets
  void        Fuzzify(double val);

  //(原文注释:用"最大平均法"去模糊)—— DeFuzzifyMaxAv。
  //defuzzify the variable using the max average method
  double       DeFuzzifyMaxAv()const;

  //(原文注释:用"重心法"去模糊)—— DeFuzzifyCentroid,NumSamples 是采样点数。
  //defuzzify the variable using the centroid method
  double       DeFuzzifyCentroid(int NumSamples)const;



  std::ostream& WriteDOMs(std::ostream& os);
};




          
#endif