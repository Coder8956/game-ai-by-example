//==============================================================================================
//【文件说明】Entity.h —— "脚本化状态机"演示里所有游戏对象的基类
//
//【这个文件是干什么的?】
//  Entity 是一切游戏角色的"祖宗类":它给每个对象发一个独一无二的 ID,记一个名字,
//  并规定"所有子类都必须实现 Update()"(纯虚函数 = 强制接口)。Miner(矿工)就继承它。
//
//【谁在使用这个文件?】Miner.h —— class Miner : public Entity(矿工继承它);
//                      main.cpp —— 用 luabind 把 Entity 的 Name/ID 暴露给 Lua 脚本。
//==============================================================================================
#ifndef ENTITY_H
#define ENTITY_H
//------------------------------------------------------------------------
//
//  Name:   Entity.h
//
//  Desc:   Base class for a game object
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <string>


class Entity
{

private:

  int          m_ID;

  std::string  m_Name;
// 私有成员:m_ID=本对象唯一编号;m_Name=名字。

  //used by the constructor to give each entity a unique ID
  int NextValidID(){static int NextID = 0; return NextID++;}
// NextValidID:每造一个对象就发一个新编号。static int NextID=0 —— static 局部变量
// 在函数调用之间"记住"上一次的值,所以每次返回后自增,编号 0、1、2…… 不重复。

public:

  Entity(std::string name = "NoName"):m_ID(NextValidID()), m_Name(name)
  {}

// 构造函数:默认名字 "NoName",同时用 NextValidID() 领一个 ID。
  virtual ~Entity()
  {}

//  virtual ~Entity() 虚析构:保证通过 Entity 指针 delete 子类对象时清理正确。
  //all entities must implement an update function
  virtual void  Update()=0;
//(原文注释:所有实体都必须实现一个 update 函数)
// =0 纯虚函数:只声明"必须有 Update()",不提供实现;谁继承谁就得写,
// 否则它自己也变成"抽象类"不能被 new。这是 C++ 用来"规定接口"的手法。

  //accessors
  int         ID()const{return m_ID;}  
  std::string Name()const{return m_Name;}

};




#endif


