//==============================================================================================
//【文件说明】EntityManager.cpp —— 实体管理器的实现(单例/查/注册/移除)
//
//【本文件是干什么的?】
//  实现 EntityManager.h 声明的 4 个函数:Instance(单例)、GetEntityFromID(按 ID 查)、
//  RegisterEntity(注册)、RemoveEntity(移除)。
//==============================================================================================
#include "game/EntityManager.h"
#include "game/BaseGameEntity.h"


//--------------------------- Instance ----------------------------------------
//
//   this class is a singleton
//-----------------------------------------------------------------------------
  // Instance:函数内静态局部变量,全程序只创建一次,返回其地址(单例经典写法)。
EntityManager* EntityManager::Instance()
{
  static EntityManager instance;

  return &instance;
}

//------------------------- GetEntityFromID -----------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// GetEntityFromID:用 map.find(id) 查找;找不到(==end)就断言报错;
// 找到则返回 ent->second(map 每个条目是 pair,first=ID,second=实体指针)。
//--------------------------------------------------------------------------------
BaseGameEntity* EntityManager::GetEntityFromID(int id)const
{
  //find the entity
  EntityMap::const_iterator ent = m_EntityMap.find(id);

  //assert that the entity is a member of the map
  assert ( (ent !=  m_EntityMap.end()) && "<EntityManager::GetEntityFromID>: invalid ID");

  return ent->second;
}

//--------------------------- RemoveEntity ------------------------------------
//-----------------------------------------------------------------------------
  // RemoveEntity:按 pEntity->ID() 找到 map 条目并 erase 删除。
void EntityManager::RemoveEntity(BaseGameEntity* pEntity)
{    
  m_EntityMap.erase(m_EntityMap.find(pEntity->ID()));
} 

//---------------------------- RegisterEntity ---------------------------------
//-----------------------------------------------------------------------------
  // RegisterEntity:把 (ID, 指针) 这对条目 insert 进 map。
void EntityManager::RegisterEntity(BaseGameEntity* NewEntity)
{
  m_EntityMap.insert(std::make_pair(NewEntity->ID(), NewEntity));
}
