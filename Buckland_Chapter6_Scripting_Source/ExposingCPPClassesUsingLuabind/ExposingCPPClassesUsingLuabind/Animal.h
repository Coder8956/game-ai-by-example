//==============================================================================================
//【文件说明】Animal.h —— "暴露 C++ 类给 Lua"演示中的动物基类
//
//【这个文件是干什么的?】
//  第 6 章讲"脚本化":用 Lua 脚本控制 C++ 程序。本小演示先用 luabind 把一个
//  C++ 类注册给 Lua,让 Lua 脚本能 new 出动物、调用它的 Speak()。Animal 是基类,
//  记录"几条腿"和"发出什么叫声"。
//
//【谁在使用这个文件?】
//  main.cpp(本工程入口)—— #include 它和 Pet.h,再用 luabind 的 module/class_ 把
//      Animal、Pet 注册进 Lua,然后 RunLuaScript 跑 Lua 脚本驱动它们。
//
//【C++ 小课堂:virtual 虚函数 / 虚析构】
//  virtual 让函数"按对象真实类型"调用(多态):子类 Pet 若重写 Speak,
//  通过 Animal 指针调用时会执行 Pet 的版本。virtual ~Animal() 虚析构则保证
//  delete 一个 Animal 指针时,能正确调用到子类的析构,不漏清理。
//==============================================================================================
#ifndef ANIMAL_H
#define ANIMAL_H

#include <string>
#include <iostream>



// class Animal —— 动物基类(本小演示的"图纸")。
class Animal
{
private:
  
  int          m_iNumLegs;

  std::string  m_NoiseEmitted;
// private 数据:m_iNumLegs=腿的数量;m_NoiseEmitted=它发出的叫声字符串。

public:

  Animal(std::string NoiseEmitted,
         int         NumLegs):m_iNumLegs(NumLegs),
                              m_NoiseEmitted(NoiseEmitted)
  {}

// 构造函数:传入叫声与腿数,用初始化列表直接给两个成员赋初值。
  virtual ~Animal(){}

// virtual Speak():打印叫声。const 承诺不改成员;NumLegs() 只读返回腿数。
  virtual void Speak()const
  {std::cout << "\n[C++]: " << m_NoiseEmitted << std::endl;}

  int          NumLegs()const{return m_iNumLegs;}
                                 
};




#endif