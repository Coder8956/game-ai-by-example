//==============================================================================================
//【文件说明】Pet.h —— "宠物"类,继承自 Animal
//
//【这个文件是干什么的?】
//  Pet 在 Animal(腿数+叫声)的基础上多加一个"名字"。它演示"继承":
//  class Pet : public Animal —— 冒号 public 表示 Pet 是 Animal 的子类,
//  白捡了 Animal 的所有成员,再扩展自己的名字。
//
//【谁在使用这个文件?】main.cpp —— #include 后用 luabind 注册 Pet 给 Lua。
//==============================================================================================
#ifndef PET_H
#define PET_H


#include <string>
#include <iostream>
#include "Animal.h"



// class Pet : public Animal —— 继承:Pet 是一种 Animal。
// public 继承 = 外界可以把 Pet 当 Animal 用(是一种"is-a"关系)。
class Pet : public Animal
{
private:
  
  std::string  m_Name;
// 子类新增的私有成员:宠物的名字 m_Name。

public:

  Pet(std::string name,
      std::string noise,
      int         NumLegs):Animal(noise, NumLegs),
                           m_Name(name)
  {}

// 构造函数:Pet(name, noise, legs)——先把 noise、legs 交给父类 Animal 的构造,
// 再初始化自己的 m_Name。GetName() 只读返回名字。
  std::string GetName()const{return m_Name;}  
};





#endif 