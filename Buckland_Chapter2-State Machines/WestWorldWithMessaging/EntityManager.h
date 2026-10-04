//==============================================================================================
//【文件说明】EntityManager.h —— 全游戏角色的"户口登记处"(单例管理器)
//
//【这个文件是干什么的?】
//  游戏运行中,角色被 new 出来之后散落在各处(在 main 里有 Bob、Elsa 两个指针)。
//  可消息系统发消息时只知道"接收者的编号"(一个 int),怎么凭编号找到那个角色?
//  答案:用一个"户口本"把所有角色登记起来——按编号查角色。
//  本文件就定义了这样一个"户口登记处"类 EntityManager:
//    RegisterEntity(角色)      —— 上户口:把角色指针按编号登记进表;
//    GetEntityFromID(编号)     —— 查户口:凭编号立刻取出角色指针;
//    RemoveEntity(角色)        —— 销户口:从表中删除该角色。
//
//【单例(Singleton)——全游戏只造一个户口本】
//  户口本这种东西,全游戏只需要一份,谁要查都来这一份。
//  本文件用宏 EntityMgr 提供"便捷入口":代码里写 EntityMgr 就等价于
//  EntityManager::Instance()(拿到那个唯一的户口本指针)。
//
//【谁在使用这个文件?】
//  1. main.cpp                —— 创建 Bob、Elsa 后,用 EntityMgr->RegisterEntity()
//                                 给两人上户口;
//  2. MessageDispatcher.cpp   —— 派发消息时用 EntityMgr->GetEntityFromID(编号)
//                                 找到接收者,再调用它的 HandleMessage();
//  3. EntityManager.cpp       —— 本文件声明的函数在那里实现;
//  4. BaseGameEntity.h        —— 被管理对象的基类(前向声明,见下面)。
//
//【数据结构小课堂:std::map(字典/映射表)】
//  std::map<编号, 指针> 是一种"键→值"对应表:给一个键(角色编号 int),
//  立即查到对应的值(角色指针 BaseGameEntity*)。查找速度很快(对数级)。
//==============================================================================================

//------------------------------------------------------------------------------------------------
// 包含保护(原理详解见 Locations.h):防止本文件被重复包含。
//------------------------------------------------------------------------------------------------
#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H
// #pragma warning (disable:4786):
//   #pragma 是"给编译器的特殊指令";warning(disable:4786) 意思是
//   "把编号 4786 的警告关掉"。4786 是旧版 VC 对标准库容器(如 std::map)
//   调试信息过长的无意义警告,原作者直接禁用以保持输出干净。
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   EntityManager.h
//
//  Desc:   Singleton class to handle the  management of Entities.          
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:EntityManager.h
//   描述  :Singleton class to handle the management of Entities
//          —— 处理实体(角色)管理的单例类
//   作者  :Mat Buckland,2002 年
//------------------------------------------------------------------------

// <map>:C++ 标准库的"字典"容器(键值对映射表),下面 EntityMap 要用。
#include <map>
// <cassert>:assert 断言宏的头文件,EntityManager.cpp 里会用它。
#include <cassert>
// <string>:标准库字符串(本文件未直接用,保留是原作者习惯)。
#include <string>


// 【前向声明】class BaseGameEntity;
//   只告诉编译器"存在一个叫 BaseGameEntity 的类",不给完整定义。
//   因为下面只用到"BaseGameEntity*"指针,而指针只需要知道类型存在即可,
//   不必知道类内部长什么样。真正的定义在 BaseGameEntity.h 里。
class BaseGameEntity;

//provide easy access
//(原文注释:提供便捷访问)

//--------------------------------------------------------------------------------
//【宏 #define】#define EntityMgr EntityManager::Instance()
//   宏 = 文本替换:以后代码里凡写 EntityMgr,预处理器都会先把它
//   替换成 EntityManager::Instance()(再继续编译)。
//   效果:写 EntityMgr->RegisterEntity(...) 比写
//   EntityManager::Instance()->RegisterEntity(...) 短很多。
//   Instance() 是单例的"取唯一实例"函数(实现见 EntityManager.cpp)。
//--------------------------------------------------------------------------------
#define EntityMgr EntityManager::Instance()



//--------------------------------------------------------------------------------
//【类 class EntityManager】户口登记处。class 关键字定义类。
//--------------------------------------------------------------------------------
class EntityManager
{
// private: 私有区——下面内容外部不可直接访问。
private:

  // typedef:给类型起别名。
  // std::map<int, BaseGameEntity*> :C++ 标准库的字典:
  //   键(int)= 角色编号;值(BaseGameEntity*)= 指向角色的指针;
  // EntityMap:给这个字典类型起的别名,以后写 EntityMap 就等于写整个长类型。
  // 语法:<键类型, 值类型> 用尖括号括起来。
  typedef std::map<int, BaseGameEntity*> EntityMap;

private:

  //to facilitate quick lookup the entities are stored in a std::map, in which
  //pointers to entities are cross referenced by their identifying number
  //(原文注释:为便于快速查找,实体被存在 std::map 里,实体的指针
  //  按它们的标识编号交叉引用)

  // 【成员变量】m_EntityMap —— 真正的"户口本"。
  //   类型 EntityMap(就是上面 typedef 的 map),里面存了
  //   "编号 → 角色指针"的全部登记信息。m_ 前缀表示类的成员变量。
  EntityMap m_EntityMap;

  // 【私有构造函数】EntityManager(){} —— 单例三件套第 1 件。
  //   构造函数私有 → 外部代码不能直接 new 一个户口本,
  //   全游戏唯一一份由 Instance() 内部创建并管理。
  EntityManager(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  // 拷贝构造函数(私有、无实现)→ 禁止复制户口本(单例三件套第 2 件)。
  EntityManager(const EntityManager&);
  // 赋值运算符(私有、无实现)→ 禁止用 = 覆盖户口本内容。
  EntityManager& operator=(const EntityManager&);

// public: 公有区——下面内容外部可以调用。
public:

  // 【静态成员函数】static EntityManager* Instance();
  //   返回全局唯一户口本实例的指针。static 表示属于类、不属于某个对象,
  //   用 EntityManager::Instance() 调用(实现见 EntityManager.cpp)。
  //   单例三件套第 3 件——全世界拿户口本的唯一入口。
  static EntityManager* Instance();

  //this method stores a pointer to the entity in the std::vector
  //m_Entities at the index position indicated by the entity's ID
  //(makes for faster access)
  //(原文注释:本方法把实体指针存进 m_Entities 向量,下标就是实体的编号
  //  ——便于快速访问)

  // 【上户口】void RegisterEntity(BaseGameEntity* NewEntity);
  //   参数:指向角色的指针(如 main 里的 Bob);
  //   作用:把"角色编号 → 角色指针"登记进 m_EntityMap
  //   (实现见 EntityManager.cpp)。
  void            RegisterEntity(BaseGameEntity* NewEntity);

  //returns a pointer to the entity with the ID given as a parameter
  //(原文注释:返回指定 ID 对应的实体指针)

  // 【查户口】BaseGameEntity* GetEntityFromID(int id)const;
  //   参数:角色编号;返回:对应的角色指针。
  //   const(在函数名后):本函数不修改户口本内容,只读查询。
  //   消息系统靠它把"编号"变成"能调用的对象"。
  BaseGameEntity* GetEntityFromID(int id)const;

  //this method removes the entity from the list
  //(原文注释:本方法把实体从列表中移除)

  // 【销户口】void RemoveEntity(BaseGameEntity* pEntity);
  //   参数:要移除的角色指针;作用:从 m_EntityMap 里删掉这条登记。
  void            RemoveEntity(BaseGameEntity* pEntity);
};






// 包含保护结束:与文件开头的 #ifndef ENTITYMANAGER_H 配对。

#endif