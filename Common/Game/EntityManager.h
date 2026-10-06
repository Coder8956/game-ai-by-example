//==============================================================================================
//【文件说明】EntityManager.h —— 实体管理器(单例,按 ID 查实体的"字典")
//
//【这个文件是干什么的?】
//  全程序所有游戏实体都注册到这里。它内部用 std::map<int, BaseGameEntity*>,
//  把每个实体的 ID 当键、实体指针当值。消息调度器按电报里的 Receiver ID
//  来这里查接收者是谁,然后才能把消息送过去。
//
//【谁在使用这个文件?】
//  MessageDispatcher.cpp(GetEntityFromID 找接收者)、各工程的 main(注册实体)、
//  MovingEntity/球员等具体类。宏 EntityMgr = EntityManager::Instance() 让代码更短。
//
//【本文件包含了谁?】
//  <map>    —— 标准库有序字典(按键快速查找);
//  <cassert> —— 断言(查不到 ID 时报错);
//  class BaseGameEntity; —— 前置声明(只存指针,不需要完整定义)。
//==============================================================================================
#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786) 原理详见 SoccerPitch.h;包含保护原理详见 Goal.h。
//--------------------------------------------------------------------------------
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   EntityManager.h
//
//  Desc:   Singleton class to handle the  management of Entities.          
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <map>
#include <cassert>


class BaseGameEntity;

//provide easy access
#define EntityMgr EntityManager::Instance()



//--------------------------------------------------------------------------------
// class EntityManager —— 实体管理器(单例)。
// typedef std::map<int, BaseGameEntity*> EntityMap; —— 给 map 起短名 EntityMap。
//--------------------------------------------------------------------------------
class EntityManager
{
private:

  typedef std::map<int, BaseGameEntity*> EntityMap;

private:

  // m_EntityMap:ID→实体指针 的字典。私有构造/拷贝=单例。
  //to facilitate quick lookup the entities are stored in a std::map, in which
  //pointers to entities are cross referenced by their identifying number
  EntityMap m_EntityMap;

  EntityManager(){}

  //copy ctor and assignment should be private
  EntityManager(const EntityManager&);
  EntityManager& operator=(const EntityManager&);

public:

  // Instance():全局唯一入口;RegisterEntity 注册;GetEntityFromID 按 ID 查;
  // RemoveEntity 移除;Reset 清空。
  static EntityManager* Instance();

  //this method stores a pointer to the entity in the std::vector
  //m_Entities at the index position indicated by the entity's ID
  //(makes for faster access)
  void            RegisterEntity(BaseGameEntity* NewEntity);

  //returns a pointer to the entity with the ID given as a parameter
  BaseGameEntity* GetEntityFromID(int id)const;

  //this method removes the entity from the list
  void            RemoveEntity(BaseGameEntity* pEntity);

  //clears all entities from the entity map
  void            Reset(){m_EntityMap.clear();}
};







#endif