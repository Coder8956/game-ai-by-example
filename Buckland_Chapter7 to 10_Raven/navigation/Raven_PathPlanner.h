//==============================================================================================
//【文件说明】navigation\Raven_PathPlanner.h —— 机器人的「寻路器」
//
//【这个文件是干什么的?】
//  每个机器人身上挂一个寻路器。机器人想去某处时,对它说:我要去某坐标/某类物品,
//  它就在导航图上发起一次 A* 或 Dijkstra 搜索(时间分片版,见 TimeSlicedGraphAlgorithms.h),
//  算好后把路径整理平滑,再以一串 PathEdge 的形式交还给机器人去走。
//
//【谁在使用这个文件?】
//  Raven_Bot.h/.cpp —— 每个 bot 持有一个 Raven_PathPlanner,想走路时调它;
//  Raven_Game.h/.cpp —— PathManager 持有所有 PathPlanner,每帧调 CycleOnce()。
//
//【本文件包含了谁?】
//  <list>                          —— 标准库链表;
//  TimeSlicedGraphAlgorithms.h      —— A*/Dijkstra 时间分片搜索(刚注释过);
//  Graph/GraphAlgorithms.h          —— 其它图算法(Common 目录);
//  Graph/SparseGraph.h              —— 稀疏图类;
//  PathEdge.h                       —— 路径段;
//  ../Raven_Map.h                  —— 游戏地图(导航图住在地图里)。
//==============================================================================================
#ifndef PATHPLANNER_H
#define PATHPLANNER_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Raven_PathPlanner.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to handle the creation of paths through a navigation graph
//(原文注释翻译:负责在导航图上创建路径的类)
//-----------------------------------------------------------------------------
// 下面是本文件需要的头文件,详见文件头【本文件包含了谁】。
#include <list>
#include "TimeSlicedGraphAlgorithms.h"
#include "Graph/GraphAlgorithms.h"
#include "Graph/SparseGraph.h"
#include "PathEdge.h"
#include "../Raven_Map.h"

// 前置声明:Raven_Bot 类(只用到指针,不需要完整定义)。
class Raven_Bot;


                                               
//--------------------------------------------------------------------------------
// class Raven_PathPlanner —— 寻路器类。
//--------------------------------------------------------------------------------
class Raven_PathPlanner
{
private:

  //for legibility
// 匿名枚举:找不到最近节点时返回 -1。
//(原文注释:为了可读性)
  enum {no_closest_node_found = -1};

public:

  //for ease of use typdef the graph edge/node types used by the navgraph
// 三个 typedef 起别名:EdgeType=图的边,NodeType=图的节点,Path=一串 PathEdge。
//(原文注释:为方便使用,给导航图用到的边/节点类型起别名)
  typedef Raven_Map::NavGraph::EdgeType           EdgeType;
  typedef Raven_Map::NavGraph::NodeType           NodeType;
  typedef std::list<PathEdge>                     Path;
  
private:

  //A pointer to the owner of this class
// m_pOwner:持有本寻路器的那个 bot。
//(原文注释:指向本类拥有者(bot)的指针)
  Raven_Bot*                          m_pOwner;

  //a reference to the navgraph
// m_NavGraph:导航图(常引用,只读借用地图里那张图)。
//(原文注释:对导航图的引用)
  const Raven_Map::NavGraph&          m_NavGraph;

  //a pointer to an instance of the current graph search algorithm.
// m_pCurrentSearch:当前正在跑的搜索(基类指针,实际指向 AStar_TS 或 Dijkstras_TS)。
//(原文注释:指向当前图搜索算法实例的指针)
  Graph_SearchTimeSliced<EdgeType>*  m_pCurrentSearch;
  
  //this is the position the bot wishes to plan a path to reach
//(原文注释:bot 想走到的那个目标位置)
  Vector2D                            m_vDestinationPos;


  //returns the index of the closest visible and unobstructed graph node to
  //the given position
// GetClosestNodeToPosition:找最近可见节点(内部工具函数)。
//(原文注释翻译:返回离给定位置最近、且可见无遮挡的图节点编号)
  int   GetClosestNodeToPosition(Vector2D pos)const;

  //smooths a path by removing extraneous edges. (may not remove all
  //extraneous edges)
// SmoothPathEdgesQuick/Precise:两种路径平滑(快速/精确)。
//(原文注释翻译:通过去掉多余边来平滑路径——快速版,可能不删干净)
  void  SmoothPathEdgesQuick(Path& path);

  //smooths a path by removing extraneous edges. (removes *all* extraneous
  //edges)
//(原文注释翻译:通过去掉多余边来平滑路径——精确版,删掉所有多余边)
  void  SmoothPathEdgesPrecise(Path& path);

  //called at the commencement of a new search request. It clears up the 
  //appropriate lists and memory in preparation for a new search request
//(原文注释翻译:发起新搜索请求时调用,清理列表和内存,为新搜索做准备)
  void  GetReadyForNewSearch();



public:

// 析构函数与构造函数声明。
  ~Raven_PathPlanner();

  Raven_PathPlanner(Raven_Bot* owner);

  //creates an instance of the A* time-sliced search and registers it with
  //the path manager
// RequestPathToItem:我要去捡某类物品(血/枪),发起 Dijkstra 搜索。
  bool       RequestPathToItem(unsigned int ItemType);

  //creates an instance of the Dijkstra's time-sliced search and registers 
  //it with the path manager
// RequestPathToPosition:我要去某坐标点,发起 A* 搜索。
//(原文注释翻译:创建一个时间分片 Dijkstra 搜索,并注册进路径管理器)
  bool       RequestPathToPosition(Vector2D TargetPos);

  //called by an agent after it has been notified that a search has terminated
  //successfully. The method extracts the path from m_pCurrentSearch, adds
  //additional edges appropriate to the search type and returns it as a list of
  //PathEdges.
// GetPath():取出最终路径。
//(原文注释翻译:bot 收到「搜索成功」通知后调用本方法,从当前搜索里取出路径,
//           按搜索类型补上必要的边,以一串 PathEdge 返回。)
  Path       GetPath();

  //returns the cost to travel from the bot's current position to a specific 
  //graph node. This method makes use of the pre-calculated lookup table
  //created by Raven_Game
// GetCostToNode:查预计算表,到某节点的代价。
//(原文注释翻译:返回从 bot 当前位置到某图节点的代价;用 Raven_Game 预计算好的查找表)
  double      GetCostToNode(unsigned int NodeIdx)const;

  //returns the cost to the closest instance of the GiverType. This method
  //also makes use of the pre-calculated lookup table. Returns -1 if no active
  //trigger found
// GetCostToClosestItem:到最近某类物品的代价。
//(原文注释翻译:返回到最近某类给予物的代价;同样用预计算表;没找到返回 -1)
  double      GetCostToClosestItem(unsigned int GiverType)const;

  
  //the path manager calls this to iterate once though the search cycle
  //of the currently assigned search algorithm. When a search is terminated
  //the method messages the owner with either the msg_NoPathAvailable or
  //msg_PathReady messages
// CycleOnce():让当前搜索算一步(被 PathManager 每帧调用)。
//(原文注释翻译:路径管理器调用本方法让当前搜索往前算一步;搜索结束时,
//           它给 bot 发 msg_NoPathAvailable 或 msg_PathReady 消息。)
  int        CycleOnce()const;

// 访问器:读/写目标位置。
  Vector2D   GetDestination()const{return m_vDestinationPos;}
  void       SetDestination(Vector2D NewPos){m_vDestinationPos = NewPos;}

  //used to retrieve the position of a graph node from its index. (takes
  //into account the enumerations 'non_graph_source_node' and 
  //'non_graph_target_node'
// GetNodePosition:按编号取节点坐标。
//(原文注释翻译:按节点编号取节点位置;要处理 non_graph_source_node /
//           non_graph_target_node 这两个特殊编号。)
  Vector2D  GetNodePosition(int idx)const;
};


#endif

