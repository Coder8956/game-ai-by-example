//==============================================================================================
//【文件说明】navigation\TimeSlicedGraphAlgorithms.h —— 时间分片版 A* 与 Dijkstra 寻路
//
//【这个文件是干什么的?】
//  经典 A* / Dijkstra 寻路是一口气算到底的。机器人一多,某一帧可能要算好几毫秒,
//  画面就卡。本文件把这两种算法改造成时间分片版:不再一次算完,而是暴露一个
//  CycleOnce()——每调用一次只往前探一个节点,没算完就返回 search_incomplete,
//  下一帧再接着探。这样算力被摊到很多帧,画面始终流畅。PathManager(见 PathManager.h)
//  就是负责轮流调用各家的 CycleOnce()。
//
//【谁在使用这个文件?】
//  navigation\Raven_PathPlanner.h/.cpp —— 机器人的寻路器 new 出 AStar_TS 或
//      Dijkstras_TS 对象,每帧让它 CycleOnce(),算完取出路径。
//
//【本文件包含了谁?】
//  <vector>/<list>/<queue>/<stack>  —— 标准库容器;
//  graph/SparseGraph.h              —— 稀疏图类(导航图本身),Common 目录;
//  misc/PriorityQueue.h             —— 索引优先队列(PQ,算法核心数据结构);
//  Graph/AStarHeuristicPolicies.h   —— A* 的启发函数(H 值怎么算);
//  SearchTerminationPolicies.h      —— 何时停的策略类(刚注释过);
//  PathEdge.h                       —— 路径段(最终路径的输出格式)。
//
//【C++ 小课堂:纯虚函数 = 0 与多态】
//   virtual int CycleOnce() = 0;
//   virtual = 虚函数:允许子类重写它的实现;
//   = 0    = 纯虚函数:基类自己不给实现,只规定子类必须有这个函数。
//   带纯虚函数的类叫抽象类,不能直接 new,只能当接口用。
//   Graph_SearchTimeSliced 就是抽象基类,规定所有时间分片搜索都必须有
//   CycleOnce / GetSPT / GetPathToTarget 等接口;AStar_TS 与 Dijkstras_TS 是两个具体子类。
//==============================================================================================
#ifndef TIME_SLICED_GRAPHALGORITHMS_H
#define TIME_SLICED_GRAPHALGORITHMS_H
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   TimeSlicedGraphAlgorithms.h
//
//  Desc:   classes to implement graph algorithms that can be distributed
//          over multiple update-steps
//
//          Any graphs passed to these functions must conform to the
//          same interface used by the SparseGraph
//          
//  Author: Mat Buckland (fup@ai-junkie.com)
//(原文注释翻译:这些类实现了可以分摊到多个更新帧里跑的图搜索算法;
//           传给这些函数的图必须符合 SparseGraph 同样的接口。)
//
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 标准库容器头文件:<vector>动态数组、<list>链表、<queue>队列、<stack>栈。
// 下面 5 条双引号 include 见文件头【本文件包含了谁】。
//--------------------------------------------------------------------------------
#include <vector>
#include <list>
#include <queue>
#include <stack>

#include "graph/SparseGraph.h"
#include "misc/PriorityQueue.h"
#include "Graph/AStarHeuristicPolicies.h"
#include "SearchTerminationPolicies.h"
#include "PathEdge.h"



//--------------------------------------------------------------------------------
// 匿名枚举:三种搜索状态的返回码。
//   target_found=0(找到目标)、target_not_found=1(图空了没找到)、
//   search_incomplete=2(还没探完,下一帧继续)。
//--------------------------------------------------------------------------------
//these enums are used as return values from each search update method
enum {target_found, target_not_found, search_incomplete};



//--------------------------------------------------------------------------------
// 抽象基类 Graph_SearchTimeSliced:给所有时间分片搜索定统一接口。
//--------------------------------------------------------------------------------
//------------------------ Graph_SearchTimeSliced -----------------------------
//
// base class to define a common interface for graph search algorithms
//(原文注释翻译:图搜索算法的公共接口基类)
//-----------------------------------------------------------------------------
template <class edge_type>
class Graph_SearchTimeSliced
{
public:

// 嵌套枚举:记录本次搜索用的是哪种算法(A* 还是 Dijkstra)。
  enum SearchType{AStar, Dijkstra};

private:

  SearchType m_SearchType;


public:

// 构造函数:记下算法类型。
  Graph_SearchTimeSliced(SearchType type):m_SearchType(type){}

// 虚析构函数:~开头。有子类继承时基类析构必须加 virtual,
// 否则 delete 基类指针时只调基类析构、子类内存泄漏。
  virtual ~Graph_SearchTimeSliced(){}

  //When called, this method runs the algorithm through one search cycle. The
  //method returns an enumerated value (target_found, target_not_found,
  //search_incomplete) indicating the status of the search
// 纯虚函数:子类必须实现。每调用一次探一个节点。
  virtual int                           CycleOnce()=0;

  //returns the vector of edges that the algorithm has examined
  virtual std::vector<const edge_type*> GetSPT()const=0;


  //returns the total cost to the target
  virtual double                         GetCostToTarget()const=0;

  //returns a list of node indexes that comprise the shortest path
  //from the source to the target
  virtual std::list<int>                GetPathToTarget()const=0;

  //returns the path as a list of PathEdges
  virtual std::list<PathEdge>           GetPathAsPathEdges()const=0;

// 只读查询:返回本次用的算法类型。
  SearchType                            GetType()const{return m_SearchType;}
};




//--------------------------------------------------------------------------------
// 具体类 Graph_SearchAStar_TS —— 时间分片版 A* 算法。
// 继承(冒号 public)上面的抽象基类。占位参数:graph_type=图类型,heuristic=启发函数。
//
// A* 直觉:从起点往外一圈圈探,每圈挑「目前离目标估计最近」的节点先探;
//   F = G + H(G=起点到该节点真实代价,H=该节点到目标的估计距离)。
//   用优先队列把 F 最小的节点排最前,下次先探它。
//--------------------------------------------------------------------------------
//-------------------------- Graph_SearchAStar_TS -----------------------------
//
//  a A* class that enables a search to be completed over multiple update-steps
//(原文注释翻译:一个 A* 类,让一次搜索能分摊到多个更新帧里完成)
//-----------------------------------------------------------------------------
template <class graph_type, class heuristic>
class Graph_SearchAStar_TS : public Graph_SearchTimeSliced<typename graph_type::EdgeType>
{
private:
  
  //create typedefs for the node and edge types used by the graph
//--------------------------------------------------------------------------------
// typedef:给类型起别名。Edge=图的边类型,Node=图的节点类型。
//   typename 关键字:告诉编译器 graph_type::EdgeType 是个类型名(模板里嵌套类型必须加)。
//--------------------------------------------------------------------------------
  typedef typename graph_type::EdgeType Edge;
  typedef typename graph_type::NodeType Node;

private:

  const graph_type&              m_Graph;

  //indexed into my node. Contains the 'real' accumulative cost to that node
// m_GCosts:G 值表(向量,下标=节点编号)。
//(原文注释:按下标为节点取值,存从起点到该节点的真实累计代价 G)
  std::vector<double>            m_GCosts; 

  //indexed into by node. Contains the cost from adding m_GCosts[n] to
  //the heuristic cost from n to the target node. This is the vector the
  //iPQ indexes into.
// m_FCosts:F 值表(G+H,优先队列按它排序)。
//(原文注释翻译:按下标为节点取值,存 G+H;这就是优先队列 iPQ 用来排序的向量)
  std::vector<double>            m_FCosts;

// m_ShortestPathTree:最短路径树 SPT——已敲定「从哪条边过来」的节点;
// m_SearchFrontier  :搜索前沿——已发现但还没敲定的节点。
  std::vector<const Edge*>       m_ShortestPathTree;
  std::vector<const Edge*>       m_SearchFrontier;

// m_iSource=起点节点编号;m_iTarget=目标节点编号。
  int                            m_iSource;
  int                            m_iTarget;

  //create an indexed priority queue of nodes. The nodes with the
  //lowest overall F cost (G+H) are positioned at the front.
// m_pPQ:指向索引优先队列对象的指针(new 出来,析构里 delete)。
  IndexedPriorityQLow<double>*    m_pPQ;

 
public:

//--------------------------------------------------------------------------------
// 构造函数:给定图 G、起点 source、目标 target。
//   :Graph_SearchTimeSliced<Edge>(AStar) 先调基类构造,声明用 A* 算法;
//   初始化列表把 G/F 表按节点数建好,初值 0。
//--------------------------------------------------------------------------------
  Graph_SearchAStar_TS(const graph_type& G,
                      int                source,
                      int                target):Graph_SearchTimeSliced<Edge>(AStar),
  
                                              m_Graph(G),
                                              m_ShortestPathTree(G.NumNodes()),                              
                                              m_SearchFrontier(G.NumNodes()),
                                              m_GCosts(G.NumNodes(), 0.0),
                                              m_FCosts(G.NumNodes(), 0.0),
                                              m_iSource(source),
                                              m_iTarget(target)
  { 
     //create the PQ   
// new 在堆上建优先队列对象;然后把起点 insert 进去,A* 从这里开探。
     m_pPQ =new IndexedPriorityQLow<double>(m_FCosts, m_Graph.NumNodes());

    //put the source node on the queue
    m_pPQ->insert(m_iSource);
  }

// 析构函数:delete 掉 new 出来的优先队列,防内存泄漏。
   ~Graph_SearchAStar_TS(){delete m_pPQ;}


  //When called, this method pops the next node off the PQ and examines all
  //its edges. The method returns an enumerated value (target_found,
  //target_not_found, search_incomplete) indicating the status of the search
// CycleOnce() 的声明;实现见本文件下面(模板成员函数写在头文件里)。
  int                      CycleOnce();

  //returns the vector of edges that the algorithm has examined
  std::vector<const Edge*> GetSPT()const{return m_ShortestPathTree;}

  //returns a vector of node indexes that comprise the shortest path
  //from the source to the target
  std::list<int>         GetPathToTarget()const;

  //returns the path as a list of PathEdges
  std::list<PathEdge>    GetPathAsPathEdges()const;

  //returns the total cost to the target
  double            GetCostToTarget()const{return m_GCosts[m_iTarget];}
};

//-----------------------------------------------------------------------------
template <class graph_type, class heuristic>
//--------------------------------------------------------------------------------
// A* 版 CycleOnce 实现:每调用一次探一个节点。
//--------------------------------------------------------------------------------
int Graph_SearchAStar_TS<graph_type, heuristic>::CycleOnce()
{
  //if the PQ is empty the target has not been found
// 优先队列空了 = 能到的节点都探过了还没找到 → 没路。
  if (m_pPQ->empty())
  {
    return target_not_found;
  }

  //get lowest cost node from the queue
// Pop():弹出 F 值最小的节点,本轮就探它。
  int NextClosestNode = m_pPQ->Pop();

  //put the node on the SPT
// 从前沿把这条边搬到 SPT:这个节点的最优选父边定下来了。
  m_ShortestPathTree[NextClosestNode] = m_SearchFrontier[NextClosestNode];

  //if the target has been found exit
// 弹出的这个节点就是目标 → A* 找到路了。
  if (NextClosestNode == m_iTarget)
  {
    return target_found;
  }

  //now to test all the edges attached to this node
// 边迭代器:遍历这个节点发出的所有边(像书签一样逐个翻)。
  graph_type::ConstEdgeIterator ConstEdgeItr(m_Graph, NextClosestNode);
// for 循环:逐个处理该节点发出的边 pE。begin()从头,end()遍历完,next()下一条。
  for (const Edge* pE=ConstEdgeItr.begin();
      !ConstEdgeItr.end();
       pE=ConstEdgeItr.next())
  {
    //calculate the heuristic cost from this node to the target (H)                       
// HCost=H 值(启发估计);GCost=起点到本节点真实 G+这条边权。
    double HCost = heuristic::Calculate(m_Graph, m_iTarget, pE->To()); 

    //calculate the 'real' cost to this node from the source (G)
    double GCost = m_GCosts[NextClosestNode] + pE->Cost();

    //if the node has not been added to the frontier, add it and update
    //the G and F costs
// 邻节点没探过(NULL)→ 记 G/F,入队,挂前沿;
    if (m_SearchFrontier[pE->To()] == NULL)
    {
      m_FCosts[pE->To()] = GCost + HCost;
      m_GCosts[pE->To()] = GCost;

      m_pPQ->insert(pE->To());

      m_SearchFrontier[pE->To()] = pE;
    }

    //if this node is already on the frontier but the cost to get here
    //is cheaper than has been found previously, update the node
    //costs and frontier accordingly.
// 已在前沿、新走法更便宜、还没进 SPT → 更新代价、调队列优先级、换边。
    else if ((GCost < m_GCosts[pE->To()]) && (m_ShortestPathTree[pE->To()]==NULL))
    {
      m_FCosts[pE->To()] = GCost + HCost;
      m_GCosts[pE->To()] = GCost;

      m_pPQ->ChangePriority(pE->To());

      m_SearchFrontier[pE->To()] = pE;
    }
  }
  
  //there are still nodes to explore
// 返回没探完,等下一帧再 CycleOnce()。
  return search_incomplete;
}

//-----------------------------------------------------------------------------
template <class graph_type, class heuristic>
std::list<int> 
//--------------------------------------------------------------------------------
// GetPathToTarget:从目标节点沿 SPT 倒着往起点爬,得到一串节点编号。
//--------------------------------------------------------------------------------
Graph_SearchAStar_TS<graph_type, heuristic>::GetPathToTarget()const
{
  std::list<int> path;

  //just return an empty path if no target or no path found
  if (m_iTarget < 0)  return path;    

// 从目标节点开始倒着往回爬;若没目标就返回空路径。
  int nd = m_iTarget;

  path.push_back(nd);
    
// 只要还没爬回起点、且当前节点在 SPT 上有父边,就继续。
  while ((nd != m_iSource) && (m_ShortestPathTree[nd] != 0))
  {
    nd = m_ShortestPathTree[nd]->From();

    path.push_front(nd);
  }

  return path;
} 


//--------------------------------------------------------------------------------
// GetPathAsPathEdges:和上面类似,但把路径打包成一串 PathEdge(路径段)。
//--------------------------------------------------------------------------------
//-------------------------- GetPathAsPathEdges -------------------------------
//
//  returns the path as a list of PathEdges
//-----------------------------------------------------------------------------
template <class graph_type, class heuristic>
std::list<PathEdge> 
Graph_SearchAStar_TS<graph_type, heuristic>::GetPathAsPathEdges()const
{
  std::list<PathEdge> path;

  //just return an empty path if no target or no path found
  if (m_iTarget < 0)  return path;    

  int nd = m_iTarget;
    
  while ((nd != m_iSource) && (m_ShortestPathTree[nd] != 0))
  {
// 为每段造一个 PathEdge:起点坐标、终点坐标、行为标志、相交实体ID。
    path.push_front(PathEdge(m_Graph.GetNode(m_ShortestPathTree[nd]->From()).Pos(),
                             m_Graph.GetNode(m_ShortestPathTree[nd]->To()).Pos(),
                             m_ShortestPathTree[nd]->Flags(),
                             m_ShortestPathTree[nd]->IDofIntersectingEntity()));

    nd = m_ShortestPathTree[nd]->From();
  }

  return path;
}

//--------------------------------------------------------------------------------
// 具体类 Graph_SearchDijkstras_TS —— 时间分片版 Dijkstra 算法。
// 与 A* 区别:Dijkstra 不算 H 值(纯靠 G 值排序),且何时停由模板参数
//   termination_condition(终止策略,见 SearchTerminationPolicies.h)决定——
//   可以不是固定节点,而是找到某个触发器(如血包)就停。
//   下面 CycleOnce/GetPathToTarget/GetPathAsPathEdges 的结构与 A* 版几乎相同,
//   不再逐行重复翻译,关键差别在 isSatisfied 那一行。
//--------------------------------------------------------------------------------
//-------------------------- Graph_SearchDijkstras_TS -------------------------
//
//  Dijkstra's algorithm class modified to spread a search over multiple
//  update-steps
//(原文注释翻译:Dijkstra 算法类改造为可分摊到多个更新帧里搜索)
//-----------------------------------------------------------------------------
template <class graph_type, class termination_condition>
class Graph_SearchDijkstras_TS : public Graph_SearchTimeSliced<typename graph_type::EdgeType>
{
private:

  //create typedefs for the node and edge types used by the graph
  typedef typename graph_type::EdgeType Edge;
  typedef typename graph_type::NodeType Node;

private:

  const graph_type&                   m_Graph;

  //indexed into my node. Contains the accumulative cost to that node
// m_CostToThisNode:到每个节点的累计代价(Dijkstra 没有 F/G 之分,只有 G)。
//(原文注释:按下标为节点取值,存到该节点的累计代价)
  std::vector<double>             m_CostToThisNode; 

  std::vector<const Edge*>  m_ShortestPathTree;
  std::vector<const Edge*>  m_SearchFrontier;

  int                            m_iSource;
  int                            m_iTarget;

  //create an indexed priority queue of nodes. The nodes with the
  //lowest overall F cost (G+H) are positioned at the front.
  IndexedPriorityQLow<double>*     m_pPQ;

 

public:

// 构造函数:和 A* 版几乎一样,只是基类参数传 Dijkstra。
  Graph_SearchDijkstras_TS(const graph_type&  G,
                          int                   source,
                          int                   target):Graph_SearchTimeSliced<Edge>(Dijkstra),
  
                                              m_Graph(G),
                                              m_ShortestPathTree(G.NumNodes()),                              
                                              m_SearchFrontier(G.NumNodes()),
                                              m_CostToThisNode(G.NumNodes(), 0.0),
                                              m_iSource(source),
                                              m_iTarget(target)
  { 
     //create the PQ         ,
     m_pPQ =new IndexedPriorityQLow<double>(m_CostToThisNode, m_Graph.NumNodes());

    //put the source node on the queue
    m_pPQ->insert(m_iSource);
  }

  //let the search class take care of tidying up memory (the wary amongst
  //you may prefer to use std::auto_ptr or similar to replace the pointer
  //to the termination condition)
   ~Graph_SearchDijkstras_TS()
   {
// 析构里 delete 优先队列。
     delete m_pPQ;
   }


  //When called, this method pops the next node off the PQ and examines all
  //its edges. The method returns an enumerated value (target_found,
  //target_not_found, search_incomplete) indicating the status of the search
  int              CycleOnce();

  //returns the vector of edges that the algorithm has examined
  std::vector<const Edge*> GetSPT()const{return m_ShortestPathTree;}

  //returns a vector of node indexes that comprise the shortest path
  //from the source to the target
  std::list<int> GetPathToTarget()const;

  //returns the path as a list of PathEdges
  std::list<PathEdge>    GetPathAsPathEdges()const;

  //returns the total cost to the target
  double            GetCostToTarget()const{return m_CostToThisNode[m_iTarget];}
};

//-----------------------------------------------------------------------------
template <class graph_type, class termination_condition>
//--------------------------------------------------------------------------------
// Dijkstra 版 CycleOnce:和 A* 几乎一样,但不算 H,用终止策略判断停不停。
//--------------------------------------------------------------------------------
int Graph_SearchDijkstras_TS<graph_type, termination_condition>::CycleOnce()
{
  //if the PQ is empty the target has not been found
  if (m_pPQ->empty())
  {
    return target_not_found;
  }

  //get lowest cost node from the queue
  int NextClosestNode = m_pPQ->Pop();

  //move this node from the frontier to the spanning tree
  m_ShortestPathTree[NextClosestNode] = m_SearchFrontier[NextClosestNode];

  //if the target has been found exit
// 关键不同点:不是节点编号==目标,而是调终止策略类的 isSatisfied 判断;
//   策略由模板参数传入(如 FindActiveTrigger:探到想要的触发器就停)。
  if (termination_condition::isSatisfied(m_Graph, m_iTarget, NextClosestNode))
  {
    //make a note of the node index that has satisfied the condition. This
    //is so we can work backwards from the index to extract the path from
    //the shortest path tree.
// 把实际满足条件的节点记成新目标,后面还原路径从它开始倒推。
    m_iTarget = NextClosestNode;

    return target_found;
  }

  //now to test all the edges attached to this node
  graph_type::ConstEdgeIterator ConstEdgeItr(m_Graph, NextClosestNode);
  for (const Edge* pE=ConstEdgeItr.begin();
      !ConstEdgeItr.end();
       pE=ConstEdgeItr.next())
  {
    //the total cost to the node this edge points to is the cost to the
    //current node plus the cost of the edge connecting them.
// NewCost:经由当前节点走到邻节点的新代价;逻辑与 A* 版相同。
    double NewCost = m_CostToThisNode[NextClosestNode] + pE->Cost();

    //if this edge has never been on the frontier make a note of the cost
    //to get to the node it points to, then add the edge to the frontier
    //and the destination node to the PQ.
// 邻节点没探过 → 记代价、入队、挂前沿。
    if (m_SearchFrontier[pE->To()] == 0)
    {
      m_CostToThisNode[pE->To()] = NewCost;

      m_pPQ->insert(pE->To());

      m_SearchFrontier[pE->To()] = pE;
    }

    //else test to see if the cost to reach the destination node via the
    //current node is cheaper than the cheapest cost found so far. If
    //this path is cheaper, we assign the new cost to the destination
    //node, update its entry in the PQ to reflect the change and add the
    //edge to the frontier
// 更便宜且还没进 SPT → 更新代价、重排 PQ、换边。
    else if ( (NewCost < m_CostToThisNode[pE->To()]) &&
              (m_ShortestPathTree[pE->To()] == 0) )
    {
      m_CostToThisNode[pE->To()] = NewCost;

      //because the cost is less than it was previously, the PQ must be
      //re-sorted to account for this.
      m_pPQ->ChangePriority(pE->To());

      m_SearchFrontier[pE->To()] = pE;
    }
  }
  
  //there are still nodes to explore
  return search_incomplete;
}

//-----------------------------------------------------------------------------
template <class graph_type, class termination_condition>
std::list<int> 
// Dijkstra 版 GetPathToTarget:逻辑与 A* 版完全一样(从目标倒爬 SPT)。
Graph_SearchDijkstras_TS<graph_type, termination_condition>::GetPathToTarget()const
{
  std::list<int> path;

  //just return an empty path if no target or no path found
  if (m_iTarget < 0)  return path;    

  int nd = m_iTarget;

  path.push_back(nd);
    
  while ((nd != m_iSource) && (m_ShortestPathTree[nd] != 0))
  {
    nd = m_ShortestPathTree[nd]->From();

    path.push_front(nd);
  }

  return path;
} 


//-------------------------- GetPathAsPathEdges -------------------------------
//
//  returns the path as a list of PathEdges
//-----------------------------------------------------------------------------
template <class graph_type, class termination_condition>
std::list<PathEdge> 
// Dijkstra 版 GetPathAsPathEdges:逻辑与 A* 版完全一样(打包成 PathEdge 串)。
Graph_SearchDijkstras_TS<graph_type, termination_condition>::GetPathAsPathEdges()const
{
  std::list<PathEdge> path;

  //just return an empty path if no target or no path found
  if (m_iTarget < 0)  return path;    

  int nd = m_iTarget;
    
  while ((nd != m_iSource) && (m_ShortestPathTree[nd] != 0))
  {
    path.push_front(PathEdge(m_Graph.GetNode(m_ShortestPathTree[nd]->From()).Pos(),
                             m_Graph.GetNode(m_ShortestPathTree[nd]->To()).Pos(),
                             m_ShortestPathTree[nd]->Flags(),
                             m_ShortestPathTree[nd]->IDofIntersectingEntity()));
    
    nd = m_ShortestPathTree[nd]->From();
  }

  return path;
}

#endif