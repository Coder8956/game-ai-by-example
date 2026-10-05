//==============================================================================================
//【文件说明】EntityFunctionTemplates.h —— 一批"实体通用操作"的函数模板
//
//【这个文件是干什么的?】
//  提供 5 个"模板函数"(对任何类型的实体容器都能用的通用操作):
//    ① Overlapped —— 判断新实体是否与容器里已有实体"重叠";
//    ② TagNeighbors —— 给某实体半径范围内的邻居"打标记";
//    ③ EnforceNonPenetrationConstraint —— 把互相压着的实体"推开",
//       防止穿模(不穿透);
//    ④ GetEntityLineSegmentIntersections —— 找出与线段 AB 相交的所有实体;
//    ⑤ GetClosestEntityLineSegmentIntersection —— 找与 AB 相交的"最近"实体。
//
//【与相关文件的关系】
//  BaseGameEntity.h —— 操作对象:这些函数调用实体的 Pos/BRadius/ID/
//                       Tag/UnTag/SetPos 等接口,都继承自它;
//  2d/geometry.h    —— 几何函数 TwoCirclesOverlapped(两圆是否重叠)、
//                       DistToLineSegment(点到线段距离)、Vec2DDistanceSq 等;
//  谁在使用本文件?
//    GameWorld.cpp —— Overlapped(生成障碍物时不重叠)、
//                     EnforceNonPenetrationConstraint(防穿模,经 GameWorld.h);
//    SteeringBehaviors.cpp —— 通过 GameWorld 的 TagVehiclesWithinViewRange
//                    等间接使用 TagNeighbors。
//
//【C++ 小课堂:函数模板 template】
//  template <class T, class conT> —— T 是"元素类型",conT 是"容器类型"。
//  函数模板 = "图纸":不针对具体类型写死,编译器看到实际调用时(如传
//  std::vector<BaseGameEntity*>),自动按这份图纸生成对应的函数实例。
//  typename conT::const_iterator —— 容器内部定义的"迭代器类型"(书签类型),
//    必须加 typename 前缀告诉编译器"这是个类型名"。
//==============================================================================================
#ifndef GAME_ENTITY_FUNCTION_TEMPLATES
#define GAME_ENTITY_FUNCTION_TEMPLATES

// 包含实体基类:模板函数要用实体的 Pos()/BRadius()/Tag() 等接口。
#include "BaseGameEntity.h"
// 包含 2D 几何工具(Common 目录):TwoCirclesOverlapped、DistToLineSegment 等
// 来自这里。注意原文件里是小写 2d/geometry.h,保持原样。
#include "2d/geometry.h"



// ↓↓↓ 原文注释翻译:【一些有用的模板函数】
//////////////////////////////////////////////////////////////////////////
//
//  Some useful template functions
//
//////////////////////////////////////////////////////////////////////////

// ↓↓↓ 原文注释翻译:
//  【Overlapped 重叠检测】测试某个实体是否与存储在 std 容器中的任意实体重叠。
//------------------------- Overlapped -----------------------------------
//
//  tests to see if an entity is overlapping any of a number of entities
//  stored in a std container
//------------------------------------------------------------------------
// 模板声明:T = 实体类型,conT = 容器类型(如 std::vector<Obstacle*>)。
template <class T, class conT>
// 函数签名:
//   const T* ob —— 待检测的实体指针(只读);
//   const conT& conOb —— 现有实体容器(只读借用,不拷贝);
//   double MinDistBetweenObstacles = 40.0 —— 允许的"最小间隔"(默认 40 像素),
//     即两圆至少要隔这么远,否则算"重叠"。
bool Overlapped(const T* ob, const conT& conOb, double MinDistBetweenObstacles = 40.0)
{
// 定义容器的只读迭代器 it(书签):const_iterator 只能读、不能改容器元素。
// typename 前缀:告诉编译器 conT::const_iterator 是一个类型名。
  typename conT::const_iterator it;

// for 循环:书签从容器开头(begin)走到结尾(end),逐个检查容器里的实体。
// ++it 让书签向后移一位;!= 判断是否还没到末尾哨兵。
  for (it=conOb.begin(); it != conOb.end(); ++it)
  {
// TwoCirclesOverlapped(圆1圆心, 圆1半径, 圆2圆心, 圆2半径):
// 判断两个圆是否重叠(两圆心距离 < 半径之和)。
//  ob->Pos() —— 待检测实体的圆心;
//  ob->BRadius() + MinDistBetweenObstacles —— 把最小间隔并进半径再比,
//    保证障碍物之间留出空隙;
//  (*it)->Pos() / (*it)->BRadius() —— 当前被检查实体的圆心与半径。
//  (*it):迭代器先用 * 解引用取出"实体指针",再 -> 调它的函数。
    if (TwoCirclesOverlapped(ob->Pos(),
                             ob->BRadius()+MinDistBetweenObstacles,                             
                             (*it)->Pos(),
                             (*it)->BRadius()))
    {
// 只要有一个重叠,立刻返回 true(不必再查后面的)。
      return true;
    }
  }

// 全部查完都不重叠,返回 false(可以安心放置)。
  return false;
}

// ↓↓↓ 原文注释翻译:
//  【TagNeighbors 给邻居打标记】给 std 容器中、位于"单个实体参数"半径范围内
//  的所有实体打上标记。
//  用途:先圈出"够近的邻居",后续(分离/对齐/内聚)只处理打过钩的,
//  省去每对实体都做距离计算的浪费。
//----------------------- TagNeighbors ----------------------------------
//
//  tags any entities contained in a std container that are within the
//  radius of the single entity parameter
//------------------------------------------------------------------------
// 模板声明:T = 实体类型,conT = 容器类型。
template <class T, class conT>
// 函数签名:
//   const T& entity —— 中心实体(以它的位置为中心);
//   conT& ContainerOfEntities —— 待检查的实体容器(要修改其中元素的标记,
//                                所以不用 const);
//   double radius —— 打标记的半径范围。
void TagNeighbors(const T& entity, conT& ContainerOfEntities, double radius)
{
// ↓↓↓ 原文注释翻译:遍历所有实体,检查是否在范围内。
  //iterate through all entities checking for range
// for 循环:书签 curEntity 从容器开头走到结尾。
// (三行书写只是排版,等价于 for(初始化;条件;步进))。
  for (typename conT::iterator curEntity = ContainerOfEntities.begin();
       curEntity != ContainerOfEntities.end();
       ++curEntity)
  {
// ↓↓↓ 原文注释翻译:首先清掉它当前可能带有的任何标记。
    //first clear any current tag
// 每个实体先取消标记:保证每轮从"零标记"重新圈选。
// (*curEntity) 取出指针,->UnTag() 调用基类的取消标记函数。
    (*curEntity)->UnTag();
    
// 计算"从自己指向该邻居"的向量 to(终点 - 起点)。
// 运算符重载:Vector2D 支持 - 和 + 向量运算。
    Vector2D to = (*curEntity)->Pos() - entity->Pos();

// ↓↓↓ 原文注释翻译:对方的包围半径也被算进范围里(把它的半径加到半径上)。
    //the bounding radius of the other is taken into account by adding it 
    //to the range
// 有效范围 = 设定半径 + 对方半径:对方个头越大,越早算"够近"。
    double range = radius + (*curEntity)->BRadius();

// ↓↓↓ 原文注释翻译:如果在范围内,就打上标记供后续处理使用(用"距离平方"
//    比较,避免开平方)。
    //if entity within range, tag for further consideration. (working in
    //distance-squared space to avoid sqrts)
// 两个条件同时满足才打钩:
//   (*curEntity) != entity —— 不是自己(实体地址不同);
//   to.LengthSq() < range*range —— 距离平方 < 范围平方(等价于距离<范围,
//     但省一次 sqrt 开方)。
//  && 读作"并且"。
    if ( ((*curEntity) != entity) && (to.LengthSq() < range*range))
    {
// 条件成立:给这个邻居打上标记(Tag() = 把标记设为 true)。
      (*curEntity)->Tag();
    }
    
// 本轮循环结束,检查下一个实体(原注释 next entity = 下一个实体)。
  }//next entity
}


// ↓↓↓ 原文注释翻译:
//  【EnforceNonPenetrationConstraint 防穿模约束】给定一个实体指针和一个
//  "附近实体"指针容器,本函数检查实体间是否重叠;若重叠,就把它们互相推开。
//  用途:机器人挤成一团时,强制把"互相穿透"的圆分开,画面不会穿模。
//------------------- EnforceNonPenetrationConstraint ---------------------
//
//  Given a pointer to an entity and a std container of pointers to nearby
//  entities, this function checks to see if there is an overlap between
//  entities. If there is, then the entities are moved away from each
//  other
//------------------------------------------------------------------------
// 模板声明:T = 实体类型,conT = 容器类型。
template <class T, class conT>
// 函数签名:const T& entity(要推开的实体,只读借用) + const conT& 容器。
// 注意:本函数只移动 entity(见下方 SetPos),容器本身不修改。
void EnforceNonPenetrationConstraint(const T&    entity, 
                                    const conT& ContainerOfEntities)
{
// ↓↓↓ 原文注释翻译:遍历所有实体,检查是否有包围半径重叠。
  //iterate through all entities checking for any overlap of bounding radii
// for 循环:书签从容器开头走到结尾(排版分行,逻辑同单行 for)。
  for (typename conT::const_iterator curEntity =ContainerOfEntities.begin();
       curEntity != ContainerOfEntities.end();
       ++curEntity)
  {
// ↓↓↓ 原文注释翻译:确保不检查"它自己"。
    //make sure we don't check against the individual
// 如果容器里这个实体就是 entity 本人,跳过本轮(continue = 跳到下一轮)。
// == 比较指针地址:同一个对象地址才相等。
    if (*curEntity == entity) continue;

// ↓↓↓ 原文注释翻译:计算两个实体位置之间的距离。
    //calculate the distance between the positions of the entities
// 从对方指向自己的向量 ToEntity = 自己位置 - 对方位置。
// 注意方向:之后要把自己往"离开对方"的方向推。
    Vector2D ToEntity = entity->Pos() - (*curEntity)->Pos();

// 两圆心实际距离 = 该向量的长度(Length() 开平方)。
    double DistFromEachOther = ToEntity.Length();

// ↓↓↓ 原文注释翻译:如果这个距离小于两半径之和,本实体就必须沿着 ToEntity
//    向量方向(即背向对方)被推开。
    //if this distance is smaller than the sum of their radii then this
    //entity must be moved away in the direction parallel to the
    //ToEntity vector   
// 重叠量 = (对方半径 + 自己半径) - 两圆心距离。
// 大于 0 说明"挤进去了",数值就是挤进去的深度。
    double AmountOfOverLap = (*curEntity)->BRadius() + entity->BRadius() -
                             DistFromEachOther;

// 只有确实重叠(重叠量 ≥ 0)才处理。
    if (AmountOfOverLap >= 0)
    {
// ↓↓↓ 原文注释翻译:把实体推开一段等于重叠量的距离。
      //move the entity a distance away equivalent to the amount of overlap.
// 把位置改成:当前位置 + 推开方向 × 推开距离。
//   ToEntity / DistFromEachOther —— 把"对方指向自己"的向量归一化,得到
//     推开方向(除以长度,长度变 1);
//   × AmountOfOverLap —— 乘以重叠量:挤多深就推多远;
//   entity->SetPos(...) —— 调用基类的设位置函数。
      entity->SetPos(entity->Pos() + (ToEntity/DistFromEachOther) *
                     AmountOfOverLap);
    }
// 检查下一个实体(next entity)。
  }//next entity
}




// ↓↓↓ 原文注释翻译:
//  【GetEntityLineSegmentIntersections 求线段交点实体】
//  用线段 AB 去测试一个实体容器。首先确认实体位于 one_to_ignore(位于 A 点)
//  的指定范围内;在范围内才做相交测试。返回所有"测试为相交"的实体列表。
//  用途:例如检测"子弹飞行路径上会撞到哪些障碍"。
//-------------------- GetEntityLineSegmentIntersections ----------------------
//
//  tests a line segment AB against a container of entities. First of all
//  a test is made to confirm that the entity is within a specified range of 
//  the one_to_ignore (positioned at A). If within range the intersection test
//  is made.
//
//  returns a list of all the entities that tested positive for intersection
//-----------------------------------------------------------------------------
// 模板声明:T = 实体类型,conT = 容器类型。
template <class T, class conT>
// 函数签名:
//   const conT& entities —— 实体容器(只读);
//   int the_one_to_ignore —— 要忽略的实体编号(自己,避免"撞到自己");
//   Vector2D A, B —— 线段 AB 的两个端点;
//   double range = MaxDouble —— 检测范围(默认"无限大")。
// 返回 std::list<T>:装命中的实体指针的链表。
std::list<T> GetEntityLineSegmentIntersections(const conT& entities,
                                               int         the_one_to_ignore,
                                               Vector2D    A,
                                               Vector2D    B,
                                               double       range = MaxDouble)
{
// 只读迭代器 it(书签)。
  typename conT::const_iterator it;

// 命中的实体放进链表 hits(先建一个空链表)。
  std::list<T> hits;

// ↓↓↓ 原文注释翻译:遍历所有实体,逐一用线段 AB 检测。
  //iterate through all entities checking against the line segment AB
  for (it=entities.begin(); it != entities.end(); ++it)
  {
// ↓↓↓ 原文注释翻译:如果实体不在范围内,或者被检查的实体正是 one_to_ignore,
//    就直接继续下一个实体。
    //if not within range or the entity being checked is the_one_to_ignore
    //just continue with the next entity
// 两个条件任一成立就跳过:
//   (*it)->ID() == the_one_to_ignore —— 就是要忽略的那个编号;
//   Vec2DDistanceSq(位置, A) > range*range —— 离 A 太远(距离平方比较,省开方)。
// || 读作"或者"。
    if ( ((*it)->ID() == the_one_to_ignore) ||
         (Vec2DDistanceSq((*it)->Pos(), A) > range*range) )
    {
// 跳过本轮,检查下一个实体。
      continue;
    }

// ↓↓↓ 原文注释翻译:如果该实体到 AB 的距离小于它的包围半径,就是相交,
//    把它加进 hits。
    //if the distance to AB is less than the entities bounding radius then
    //there is an intersection so add it to hits
// DistToLineSegment(A, B, 位置) < 半径:圆心到线段的距离小于半径 =
// 圆与线段相交(几何上"碰到线段")。
    if (DistToLineSegment(A, B, (*it)->Pos()) < (*it)->BRadius())
    {
// 命中!把实体指针追加到结果链表末尾(push_back)。
      hits.push_back(*it);
    }

  }

// 返回全部命中的实体链表(按值返回拷贝)。
  return hits;
}

// ↓↓↓ 原文注释翻译:
//  【GetClosestEntityLineSegmentIntersection 求最近交点实体】
//  与上一个函数逻辑相同,但只返回"离 A 最近"的那个相交实体;
//  若没有任何相交实体,返回 NULL(空指针)。
//------------------------ GetClosestEntityLineSegmentIntersection ------------
//
//  tests a line segment AB against a container of entities. First of all
//  a test is made to confirm that the entity is within a specified range of 
//  the one_to_ignore (positioned at A). If within range the intersection test
//  is made.
//
//  returns the closest entity that tested positive for intersection or NULL
//  if none found
//-----------------------------------------------------------------------------

// 模板声明:T = 实体类型,conT = 容器类型。
template <class T, class conT>
// 函数签名:同上一函数,返回类型变成 T*(实体指针)。
T* GetClosestEntityLineSegmentIntersection(const conT& entities,
                                          int         the_one_to_ignore,
                                          Vector2D    A,
                                          Vector2D    B,
                                          double       range = MaxDouble)
{
// 只读迭代器 it(书签)。
  typename conT::const_iterator it;

// 记录"当前最近命中实体",先初始化为 NULL(什么都没找到)。
  T* ClosestEntity = NULL;

// 记录"当前最近距离",先初始化为 MaxDouble(极大值,第一次必然被替换)。
  double ClosestDist = MaxDouble;

// ↓↓↓ 原文注释翻译:遍历所有实体,逐一用线段 AB 检测。
  //iterate through all entities checking against the line segment AB
  for (it=entities.begin(); it != entities.end(); ++it)
  {
// 先算出该实体到 A 点的距离平方 distSq(后面既当范围判断,又当"远近"比较)。
    double distSq = Vec2DDistanceSq((*it)->Pos(), A);

// ↓↓↓ 原文注释翻译:如果不在范围内或是 one_to_ignore,直接继续下一个实体。
    //if not within range or the entity being checked is the_one_to_ignore
    //just continue with the next entity
// 条件同上一函数:忽略指定编号、或离 A 太远 → 跳过。
    if ( ((*it)->ID() == the_one_to_ignore) || (distSq > range*range) )
    {
// 跳过本轮。
      continue;
    }

// ↓↓↓ 原文注释翻译:如果到 AB 的距离小于包围半径,就是相交;
//    若它比已记录的最远距离更近,则更新记录。
    //if the distance to AB is less than the entities bounding radius then
    //there is an intersection so add it to hits
// 圆心到线段距离 < 半径 = 相交(同上一函数)。
    if (DistToLineSegment(A, B, (*it)->Pos()) < (*it)->BRadius())
    {
// 如果这个命中点比已记录的"最近"还近(distSq 更小)……
      if (distSq < ClosestDist)
      {
// ……更新最近距离;
        ClosestDist = distSq;

// ……并记录"最近实体"指针。
        ClosestEntity = *it;
      }
    }

  }

// 返回最近命中的实体指针;一个都没找到就返回 NULL(初始值)。
  return ClosestEntity;
}


#endif