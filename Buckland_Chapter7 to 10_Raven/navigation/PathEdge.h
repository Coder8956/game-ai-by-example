//==============================================================================================
//【文件说明】navigation\PathEdge.h —— 一条「路径段」(路径的最小单位)
//
//【这个文件是干什么的?】
//  机器人从 A 走到 B 的整条路径,可以拆成一段一段直线路径段。
//  每一段 PathEdge 记录:从哪(Source)走到哪(Destination)、
//  走这段时该用哪种移动行为(Behavior,如普通走/躲门)、
//  以及这段是否穿过一扇门(DoorID)。路径规划器(PathPlanner)算好
//  一条路后,就是一串 PathEdge 排好队交给机器人照着走。
//
//【谁在使用这个文件?】
//  navigation\Raven_PathPlanner.h/.cpp —— 算路结果用 PathEdge 装;
//  navigation\TimeSlicedGraphAlgorithms.h —— 图搜索算法产出 PathEdge;
//  goals\ 下 Goal_FollowPath / Goal_NegotiateDoor / Goal_TraverseEdge.h
//      (机器人照路径走时逐段消费)——goals/ 属别的分片,不动。
//
//【本文件包含了谁?】
//  2d/vector2D.h —— 2D 向量类(Source/Destination 都是坐标点)。
//
//【C++ 小课堂:类的访问器(setter/getter)】
//  成员变量放在 private(私有),外面改不到;再在 public 区成对提供
//   Destination() const      —— 读(getter);
//   SetDestination(新值)    —— 写(setter)。
//  这样对外只暴露「能读能写」的操作,内部数据不被乱写,是封装的基本套路。
//==============================================================================================
//--------------------------------------------------------------------------------
// 原作者文件头注释(Name/Author/Desc),原样保留。
//--------------------------------------------------------------------------------
#ifndef PATHEDGE_H
#define PATHEDGE_H
//-----------------------------------------------------------------------------
//
//  Name:   PathEdge.h
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:   class to represent a path edge. This path can be used by a path
//          planner in the creation of paths. 
//(原文注释翻译:这个类表示一条路径段;路径规划器用它来拼出完整路径)
//
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 2d/vector2D.h:2D 向量类(坐标点、加减乘、长度等运算)。
// Source/Destination 两个成员都是 Vector2D 坐标,所以先拿它的说明书。
//--------------------------------------------------------------------------------
#include "2d/vector2D.h"

//--------------------------------------------------------------------------------
// class PathEdge —— 一条路径段类。
//  private 区(下面 4 行)是内部数据,外面碰不到;public 区是对外操作。
//--------------------------------------------------------------------------------
class PathEdge
{
private:

  //positions of the source and destination nodes this edge connects
//(原文注释:这条边连接的起点节点和终点节点的位置)
  Vector2D m_vSource;
  Vector2D m_vDestination;

  //the behavior associated with traversing this edge
//(原文注释:走过这条边时要采用的行为编号)
  int      m_iBehavior;

  int      m_iDoorID;

// m_iDoorID:这条路径段穿过哪扇门的编号;0 表示不穿门。
public:
  
//--------------------------------------------------------------------------------
// 构造函数:创建一条路径段时,给出起点、终点、行为;DoorID 可省略(默认 0)。
//   DoorID = 0  —— 函数参数这里的「=0」是默认实参:调用者不传就当 0;
//   :m_vSource(Source) 这种冒号写法叫「初始化列表」,比进函数体再赋值高效。
//--------------------------------------------------------------------------------
  PathEdge(Vector2D Source,
           Vector2D Destination,
           int      Behavior,
           int      DoorID = 0):m_vSource(Source),
                                m_vDestination(Destination),
                                m_iBehavior(Behavior),
                                m_iDoorID(DoorID)
  {}

//--------------------------------------------------------------------------------
// 下面是一堆访问器(读/写私有成员)。末尾 const 表示只读函数,不改对象。
//   Destination()/Source()  —— 读终点/起点;
//   SetDestination/SetSource—— 改终点/起点;
//   DoorID()/Behavior()     —— 读门编号/行为编号。
//--------------------------------------------------------------------------------
  Vector2D Destination()const{return m_vDestination;}
  void     SetDestination(Vector2D NewDest){m_vDestination = NewDest;}
  
  Vector2D Source()const{return m_vSource;}
  void     SetSource(Vector2D NewSource){m_vSource = NewSource;}

  int      DoorID()const{return m_iDoorID;}
  int      Behavior()const{return m_iBehavior;}
};


#endif
