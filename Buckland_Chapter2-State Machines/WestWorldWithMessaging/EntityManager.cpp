//==============================================================================================
//【文件说明】EntityManager.cpp —— 户口登记处的"办事窗口"(实现文件)
//
//【这个文件是干什么的?】
//  头文件 EntityManager.h 只写了"有哪些功能"(上户口/查户口/销户口),
//  本文件给出每个功能的具体实现,以及单例 Instance() 的"唯一实例"从哪来。
//
//【与相关文件的关系】
//  1. EntityManager.h     —— 第一行就包含它(类声明都来自这里);
//  2. BaseGameEntity.h    —— 被管理的角色类型(取角色的 ID() 要用它);
//  3. main.cpp            —— 调用 RegisterEntity 给 Bob、Elsa 上户口;
//  4. MessageDispatcher.cpp —— 调用 GetEntityFromID 按编号找接收者。
//
//【单例的实现套路(本文件的核心)】
//  Instance() 里用了一个"函数内 static 局部变量":
//    static EntityManager instance;
//  这行代码只在第一次调用 Instance() 时执行一次(创建唯一对象),
//  之后每次都返回同一个对象的地址——全游戏只有一个户口本。
//==============================================================================================

// 包含头文件:EntityManager 的类声明(以及宏 EntityMgr)。
#include "EntityManager.h"
// 包含基类头文件:因为本文件要用 BaseGameEntity 的成员函数
// (比如 pEntity->ID()、NewEntity->ID()),编译器必须先看到它的完整定义。
#include "BaseGameEntity.h"


//--------------------------- Instance ----------------------------------------
//
//   this class is a singleton
//-----------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Instance —— 取唯一实例】:
//   这个类是单例。
//-----------------------------------------------------------------------------
EntityManager* EntityManager::Instance()
{
  // 【函数内静态变量】static EntityManager instance;
  //   static(静态):这个局部变量的生命周期从第一次执行这里开始,
  //   直到程序结束才销毁(普通局部变量则函数一返回就没了)。
  //   它不放在栈上,而是放在"静态存储区",所以能一直活着。
  //   只初始化一次 → 全程序只有这一份户口本。
  //   注意:EntityManager 的构造函数是私有的,但这里是"类的成员函数",
  //   成员函数有权访问自己的私有构造函数,所以能创建它。
  static EntityManager instance;

  // &instance:取地址运算符,返回这个唯一实例的指针(供外面用 -> 调用)。
  return &instance;
}

//------------------------- GetEntityFromID -----------------------------------
//-----------------------------------------------------------------------------
// ↓↓↓ 原文翻译【GetEntityFromID —— 按编号查户口】:
//   凭角色编号,从户口本里找出对应的角色指针。
//-----------------------------------------------------------------------------
BaseGameEntity* EntityManager::GetEntityFromID(int id)const
{
  //find the entity
  //(原文注释:查找该实体)

  // 【迭代器】EntityMap::const_iterator ent = m_EntityMap.find(id);
  //   m_EntityMap.find(id) :在户口本里按编号 id 查找,返回一个"迭代器"。
  //   迭代器(iterator)可以理解为"指向容器中某个位置的指针/游标":
  //   找到 → 游标指向那个条目;没找到 → 游标指向 end()(末尾哨兵位置)。
  //   EntityMap::const_iterator:带 const 的迭代器,只读遍历、不能改内容。
  //   std::map 的条目是"键值对",用 ->first 取键(编号)、->second 取值(指针)。
  EntityMap::const_iterator ent = m_EntityMap.find(id);

  //assert that the entity is a member of the map
  //(原文注释:断言该实体确实在户口本里)

  // 【断言】查到的位置必须不是末尾哨兵 end()——否则说明编号没登记过,
  // 那就是程序逻辑 bug(比如发消息给一个没创建的角色),立刻停下报错。
  // 用法与 BaseGameEntity.cpp 里的 assert 相同:条件为假 → 终止程序。
  assert ( (ent !=  m_EntityMap.end()) && "<EntityManager::GetEntityFromID>: invalid ID");

  // ent->second:迭代器 -> 取"值"(即该编号对应的角色指针),返回给调用者。
  return ent->second;
}

//--------------------------- RemoveEntity ------------------------------------
//-----------------------------------------------------------------------------
// ↓↓↓ 原文翻译【RemoveEntity —— 销户口】:
//   把指定实体从户口本里移除。
//-----------------------------------------------------------------------------
void EntityManager::RemoveEntity(BaseGameEntity* pEntity)
{    
  // m_EntityMap.find(...):先找到该角色所在的条目;
  // m_EntityMap.erase(...):再按这个位置把它从 map 中删除。
  // pEntity->ID():取该角色的编号(作为查找的键)。
  // 注意:这里只删"户口本里的登记",不 delete 角色对象本身
  // (角色内存的释放由创建者负责,见 main.cpp 末尾的 delete)。
  m_EntityMap.erase(m_EntityMap.find(pEntity->ID()));
} 

//---------------------------- RegisterEntity ---------------------------------
//-----------------------------------------------------------------------------
// ↓↓↓ 原文翻译【RegisterEntity —— 上户口】:
//   把新实体登记进户口本。
//-----------------------------------------------------------------------------
void EntityManager::RegisterEntity(BaseGameEntity* NewEntity)
{
  // std::make_pair(键, 值):制作一个"键值对"(编号, 指针);
  // m_EntityMap.insert(...):把这对数据插入户口本。
  // 以后就能用 GetEntityFromID(编号) 快速找到这个角色。
  m_EntityMap.insert(std::make_pair(NewEntity->ID(), NewEntity));
}
