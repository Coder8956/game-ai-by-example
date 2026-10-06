//==============================================================================================
//【文件说明】GraphNodeTypes.h —— 图论模块的"节点(顶点)"类定义
//
//【这个文件是干什么的?】
//  一张图 = 节点 + 边(边见 GraphEdgeTypes.h)。本文件定义两种节点:
//    GraphNode   —— 最基础节点:只记一个整数编号(index);
//    NavGraphNode—— 导航节点:在编号之外再加坐标(Vector2D)和附加信息,
//                  寻路时节点对应地图上的一个路口/坐标点。
//
//【谁在使用这个文件?】
//  SparseGraph.h / GraphAlgorithms.h / HandyGraphFunctions.h —— 第 5 章寻路工程;
//  Raven(第 7~10 章)也用 NavGraphNode 表示掩体/道具位置。
//
//【本文件包含了谁?】
//  <list>   —— 标准库双向链表容器;
//  <ostream>/<fstream> —— 输出流/文件流(打印与读盘);
//  "2D/Vector2D.h"     —— 2D 向量(节点坐标);
//  "graph/NodeTypeEnumerations.h" —— 借用 invalid_node_index(-1)常量。
//
//【C++ 小课堂:模板(template)是什么?】
//  下面 template <class extra_info = void*> 表示这是个泛型图纸:
//  extra_info 是一个占位类型,用 NavGraphNode<int> 就把它换成 int,
//  用 NavGraphNode<SomeClass*> 就换成指针。= void* 是缺省值:不写时默认 void*。
//  模板让同一份代码能装不同类型的附加信息,避免为每种类型重复写一遍类。
//==============================================================================================
#ifndef GRAPH_NODE_TYPES_H
#define GRAPH_NODE_TYPES_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Name:   GraphNodeTypes.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   Node classes to be used with graphs
//-----------------------------------------------------------------------------
#include <list>
#include <ostream>
#include <fstream>
#include "2D/Vector2D.h"
#include "graph/NodeTypeEnumerations.h"




//--------------------------------------------------------------------------------
// class GraphNode —— 基础节点类:只有一个成员 m_iIndex(节点编号)。
// protected 表示只有子类能直接碰这个成员,外界走访问器。
//--------------------------------------------------------------------------------
class GraphNode
{  
protected:

  //every node has an index. A valid index is >= 0
  int        m_iIndex;
  //(原文注释:每个节点都有一个编号,合法编号 >= 0)

public:
  
//--------------------------------------------------------------------------------
// 三个构造函数:空参(编号=-1)、给定编号、从文件流读编号。
//--------------------------------------------------------------------------------
  GraphNode():m_iIndex(invalid_node_index){}
  GraphNode(int idx):m_iIndex(idx){}
  GraphNode(std::ifstream& stream){char buffer[50]; stream >> buffer >> m_iIndex;}

  // 虚析构函数(原理见 GraphEdgeTypes.h 小课堂):父类指针删除子类对象时不泄漏。
  virtual ~GraphNode(){}

  // 访问器 Index/SetIndex:读写节点编号;下面的友元 operator<<:打印节点编号。
  int  Index()const{return m_iIndex;}
  void SetIndex(int NewIndex){m_iIndex = NewIndex;}
  


  //for reading and writing to streams.
  friend std::ostream& operator<<(std::ostream& os, const GraphNode& n)
  {
    os << "Index: " << n.m_iIndex << std::endl; return os;
  }

};   



//-----------------------------------------------------------------------------
//
//  Graph node for use in creating a navigation graph.This node contains
//  the position of the node and a pointer to a BaseGameEntity... useful
//  if you want your nodes to represent health packs, gold mines and the like
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// template <class extra_info = void*> —— 模板声明(详解见文件头 C++ 小课堂)。
// class NavGraphNode : public GraphNode —— 导航节点,公有继承基础节点。
//--------------------------------------------------------------------------------
template <class extra_info = void*>
class NavGraphNode : public GraphNode
{
protected:

//--------------------------------------------------------------------------------
// 新增两个成员:
//   m_vPosition —— 节点的 2D 坐标(v = Vector2D 向量,匈牙利前缀);
//   m_ExtraInfo —— 附加信息(模板参数 extra_info 类型),可用来装道具类型/指针。
//--------------------------------------------------------------------------------
  //the node's position
  Vector2D     m_vPosition;

  //often you will require a navgraph node to contain additional information.
  //For example a node might represent a pickup such as armor in which
  //case m_ExtraInfo could be an enumerated value denoting the pickup type,
  //thereby enabling a search algorithm to search a graph for specific items.
  //Going one step further, m_ExtraInfo could be a pointer to the instance of
  //the item type the node is twinned with. This would allow a search algorithm
  //to test the status of the pickup during the search. 
  extra_info  m_ExtraInfo;
  //(原文注释:节点坐标 / 附加信息可用来表示道具类型或指向道具实例的指针)

public:
  
//--------------------------------------------------------------------------------
// 构造函数:空参(附加信息用默认构造值);带编号和坐标版(先调父类构造 GraphNode(idx))。
//--------------------------------------------------------------------------------
  //ctors
  NavGraphNode():m_ExtraInfo(extra_info()){}

  NavGraphNode(int      idx,
               Vector2D pos):GraphNode(idx),
                             m_vPosition(pos),
                             m_ExtraInfo(extra_info())
  {}

  //stream constructor
//--------------------------------------------------------------------------------
// 从文件流读节点:依次读编号、x 坐标、y 坐标(m_vPosition.x/.y 是向量的两个分量)。
//--------------------------------------------------------------------------------
  NavGraphNode(std::ifstream& stream):m_ExtraInfo(extra_info())
  {
    char buffer[50];
    stream >> buffer >> m_iIndex >> buffer >> m_vPosition.x >> buffer >> m_vPosition.y;
  }
 

  virtual ~NavGraphNode(){}

  // 访问器:Pos/SetPos 读写坐标;ExtraInfo/SetExtraInfo 读写附加信息。
  Vector2D   Pos()const{return m_vPosition;}
  void       SetPos(Vector2D NewPosition){m_vPosition = NewPosition;}

  extra_info ExtraInfo()const{return m_ExtraInfo;}
  void       SetExtraInfo(extra_info info){m_ExtraInfo = info;}

  //for reading and writing to streams.
  friend std::ostream& operator<<(std::ostream& os, const NavGraphNode& n)
  {
    os << "Index: " << n.m_iIndex << " PosX: " << n.m_vPosition.x << " PosY: " << n.m_vPosition.y << std::endl;

    return os;
  }
  
};


#endif
