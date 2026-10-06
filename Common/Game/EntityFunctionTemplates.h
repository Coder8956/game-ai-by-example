//==============================================================================================
//【文件说明】EntityFunctionTemplates.h —— 实体常用自由函数模板(重叠/邻居/防穿透)
//
//【这个文件是干什么的?】
//  三个跟"一群实体"打交道的通用函数模板(任何装实体指针的容器都能用):
//    Overlapped                    —— 某个实体是否与容器里任一实体的碰撞圆重叠;
//    TagNeighbors                  —— 把"在指定半径内"的邻居实体打上标记位;
//    EnforceNonPenetrationContraint —— 两实体圆重叠时,把它们推开一点(防穿模)。
//
//【谁在使用这个文件?】
//  SimpleSoccer(球员间防穿透)、Raven(敌人之间防重叠)等。
//
//【本文件包含了谁?】
//  "game/BaseGameEntity.h" —— 实体基类(要用到 Pos/BRadius/Tag 等接口);
//  "2d/geometry.h"         —— TwoCirclesOverlapped 两圆重叠判断。
//==============================================================================================
#ifndef GAME_ENTITY_FUNCTION_TEMPLATES
#define GAME_ENTITY_FUNCTION_TEMPLATES
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------

#include "game/BaseGameEntity.h"
#include "2d/geometry.h"



//////////////////////////////////////////////////////////////////////////
//
//  Some useful template functions
//
//////////////////////////////////////////////////////////////////////////

//------------------------- Overlapped -----------------------------------
//
//  tests to see if an entity is overlapping any of a number of entities
//  stored in a std container
//------------------------------------------------------------------------
template <class T, class conT>
//--------------------------------------------------------------------------------
// Overlapped:遍历容器 conOb 中每个实体,用 TwoCirclesOverlapped 判断
// ob 的碰撞圆(半径加一个缓冲 MinDist)是否与它重叠;有任一重叠即返回 true。
//   typename conT::const_iterator 是"容器类型的只读迭代器类型"。
//--------------------------------------------------------------------------------
bool Overlapped(const T* ob, const conT& conOb, double MinDistBetweenObstacles = 40.0)
{
  typename conT::const_iterator it;

  for (it=conOb.begin(); it != conOb.end(); ++it)
  {
    if (TwoCirclesOverlapped(ob->Pos(),
                             ob->BRadius()+MinDistBetweenObstacles,                             
                             (*it)->Pos(),
                             (*it)->BRadius()))
    {
      return true;
    }
  }

  return false;
}

//----------------------- TagNeighbors ----------------------------------
//
//  tags any entities contained in a std container that are within the
//  radius of the single entity parameter
//------------------------------------------------------------------------
template <class T, class conT>
//--------------------------------------------------------------------------------
// TagNeighbors:先把容器里所有实体 UnTag 清掉;
// 再遍历:凡与 entity 距离(用平方比较省开方)小于 radius+对方半径 的非自己实体,
// 都 Tag() 打标记。AI 常用这个标记表示"附近的敌人/队友"。
//--------------------------------------------------------------------------------
void TagNeighbors(T* entity, conT& others, const double radius)
{
  typename conT::iterator it;

  //iterate through all entities checking for range
  for (it=others.begin(); it != others.end(); ++it)
  {
    //first clear any current tag
    (*it)->UnTag();

    //work in distance squared to avoid sqrts
    Vector2D to = (*it)->Pos() - entity->Pos();

    //the bounding radius of the other is taken into account by adding it 
    //to the range
    double range = radius + (*it)->BRadius();

    //if entity within range, tag for further consideration
    if ( ((*it) != entity) && (to.LengthSq() < range*range))
    {
      (*it)->Tag();
    }
    
  }//next entity
}


//------------------- EnforceNonPenetrationContraint ---------------------
//
//  Given a pointer to an entity and a std container of pointers to nearby
//  entities, this function checks to see if there is an overlap between
//  entities. If there is, then the entities are moved away from each
//  other
//------------------------------------------------------------------------
template <class T, class conT>
//--------------------------------------------------------------------------------
// EnforceNonPenetrationContraint:遍历其他实体,若两圆重叠(距离 < 半径和),
// 就算出穿透量 AmountOfOverLap,把 entity 沿连线方向推开这么远。
//--------------------------------------------------------------------------------
void EnforceNonPenetrationContraint(T entity, const conT& others)
{
  typename conT::const_iterator it;

  //iterate through all entities checking for any overlap of bounding
  //radii
  for (it=others.begin(); it != others.end(); ++it)
  {
    //make sure we don't check against this entity
    if (*it == entity) continue;

    //calculate the distance between the positions of the entities
    Vector2D ToEntity = entity->Pos() - (*it)->Pos();

    double DistFromEachOther = ToEntity.Length();

    //if this distance is smaller than the sum of their radii then this
    //entity must be moved away in the direction parallel to the
    //ToEntity vector   
    double AmountOfOverLap = (*it)->BRadius() + entity->BRadius() -
                             DistFromEachOther;

    if (AmountOfOverLap >= 0)
    {
      //move the entity a distance away equivalent to the amount of overlap.
      entity->SetPos(entity->Pos() + (ToEntity/DistFromEachOther) *
                     AmountOfOverLap);
    }
  }//next entity
}










#endif