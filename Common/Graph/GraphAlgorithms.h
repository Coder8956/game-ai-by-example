//==============================================================================================
//【文件说明】GraphAlgorithms.h —— 图搜索算法模板集(DFS/BFS/Dijkstra/A*/最小生成树)
//
//【这个文件是干什么的?】
//  在 SparseGraph(稀疏图)之上,本文件实现了 5 个经典图算法类,供第 5 章寻路工程使用:
//    Graph_SearchDFS       —— 深度优先搜索(一条路走到黑,走不通再回头);
//    Graph_SearchBFS       —— 广度优先搜索(一层一层向外扩,无权图的最短路);
//    Graph_SearchDijkstra  —— Dijkstra 最短路(按代价从小到大扩展,带权图的最短路);
//    Graph_SearchAStar     —— A* 搜索(Dijkstra + 启发函数引导,实际寻路的主力);
//    Graph_MinSpanningTree —— Prim 最小生成树(连遍所有节点且总代价最小)。
//  每个算法都是模板类,实例化时传入具体的图类型,算法自动适配。
//
//【谁在使用这个文件?】
//  HandyGraphFunctions.h(建全对最短路表)、第 5 章 Pathfinder.cpp(演示寻路);
//  Raven(第 7~10 章)用 Graph_SearchA* 做 AI 导航。
//
//【本文件包含了谁?】
//  <vector>/<list>/<queue>/<stack> —— 标准库容器(数组/链表/队列/栈);
//  "graph/SparseGraph.h"        —— 被搜索的图类;
//  "misc/PriorityQueue.h"        —— 索引优先队列(Dijkstra/A* 按代价排序用)。
//
//【C++ 小课堂:模板嵌套类型 typename】
//  类里常写 typedef typename graph_type::EdgeType Edge;
//    typename 是因为 graph_type 是模板参数,编译器不知道 EdgeType 是类型还是静态成员,
//    加 typename 明确告诉它"这是个类型",否则编译报错。
//==============================================================================================
#ifndef GRAPHALGORITHMS_H
#define GRAPHALGORITHMS_H
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786) 原理详见 SoccerPitch.h;包含保护原理详见 Goal.h。
//--------------------------------------------------------------------------------
#pragma warning (disable:4786)

//------------------------------------------------------------------------
//
//  Name:   GraphSearches.h
//
//  Desc:   classes to implement graph algorithms, including DFS, BFS,
//          Dijkstra's, A*, Prims etc. (based upon the code created
//          by Robert Sedgewick in his book "Algorithms in C++")
//
//          Any graphs passed to these functions must conform to the
//          same interface used by the SparseGraph
//          
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <vector>
#include <list>
#include <queue>
#include <stack>

#include "graph/SparseGraph.h"
#include "misc/PriorityQueue.h"


//----------------------------- Graph_SearchDFS -------------------------------
//
//  class to implement a depth first search. 
//-----------------------------------------------------------------------------
template<class graph_type>
//--------------------------------------------------------------------------------
// Graph_SearchDFS:深度优先搜索。用 std::stack(栈,后进先出)做待办列表;
//   m_Visited 记录已访问节点;m_Route 记录每个节点的"父节点"(用于回溯出路径);
//   m_SpanningTree 记录搜过的所有边(纯可视化用,不影响算法结果)。
// 匿名枚举 {visited, unvisited, no_parent_assigned} 给状态起名字,提高可读性。
//--------------------------------------------------------------------------------
class Graph_SearchDFS
{
private:

  //to aid legibility
  enum {visited, unvisited, no_parent_assigned};

  //create a typedef for the edge and node types used by the graph
  typedef typename graph_type::EdgeType Edge;
  typedef typename graph_type::NodeType Node;

private:

  //a reference to the graph to be searched
  const graph_type& m_Graph;

  //this records the indexes of all the nodes that are visited as the
  //search progresses
  std::vector<int>  m_Visited;

  //this holds the route taken to the target. Given a node index, the value
  //at that index is the node's parent. ie if the path to the target is
  //3-8-27, then m_Route[8] will hold 3 and m_Route[27] will hold 8.
  std::vector<int>  m_Route;

  //As the search progresses, this will hold all the edges the algorithm has
  //examined. THIS IS NOT NECESSARY FOR THE SEARCH, IT IS HERE PURELY
  //TO PROVIDE THE USER WITH SOME VISUAL FEEDBACK
  std::vector<const Edge*>  m_SpanningTree;

  //the source and target node indices
  int               m_iSource,
                    m_iTarget;

  //true if a path to the target has been found
  bool              m_bFound;


  //this method performs the DFS search
  bool Search();
  
public:

  Graph_SearchDFS(const graph_type& graph,
                  int          source,
                  int          target = -1 ):
  
                                      m_Graph(graph),
                                      m_iSource(source),
                                      m_iTarget(target),
                                      m_bFound(false),
                                      m_Visited(m_Graph.NumNodes(), unvisited),
                                      m_Route(m_Graph.NumNodes(), no_parent_assigned)

  {                                                                         
    m_bFound = Search(); 
  }


  //returns a vector containing pointers to all the edges the search has examined
  std::vector<const Edge*> GetSearchTree()const{return m_SpanningTree;}

  //returns true if the target node has been located
  bool   Found()const{return m_bFound;}

  //returns a vector of node indexes that comprise the shortest path
  //from the source to the target
  std::list<int> GetPathToTarget()const;  
};

//-----------------------------------------------------------------------------
template <class graph_type>
  // DFS::Search:把"伪边"入栈,然后反复"取栈顶→标记访问→把未访问邻居入栈",
  //   直到找到目标或栈空。栈的特点是后进先出,所以会先往深处钻。
bool Graph_SearchDFS<graph_type>::Search()
{
  //create a std stack of edges
  std::stack<const Edge*> stack;

  //create a dummy edge and put on the stack
  Edge Dummy(m_iSource, m_iSource, 0);
  
  stack.push(&Dummy);

  //while there are edges in the stack keep searching
  while (!stack.empty())
  {
    //grab the next edge
    const Edge* Next = stack.top();

    //remove the edge from the stack
    stack.pop();

    //make a note of the parent of the node this edge points to
    m_Route[Next->To()] = Next->From();

    //put it on the tree. (making sure the dummy edge is not placed on the tree)
    if (Next != &Dummy)
    {
      m_SpanningTree.push_back(Next);
    }
   
    //and mark it visited
    m_Visited[Next->To()] = visited;

    //if the target has been found the method can return success
    if (Next->To() == m_iTarget)
    {
      return true;
    }

    //push the edges leading from the node this edge points to onto
    //the stack (provided the edge does not point to a previously 
    //visited node)
    graph_type::ConstEdgeIterator ConstEdgeItr(m_Graph, Next->To());

    for (const Edge* pE=ConstEdgeItr.begin();
        !ConstEdgeItr.end();
         pE=ConstEdgeItr.next())
    {
      if (m_Visited[pE->To()] == unvisited)
      {
        stack.push(pE);
      }
    }
  }

  //no path to target
  return false;
}

//-----------------------------------------------------------------------------
template <class graph_type>
  // GetPathToTarget:从目标节点沿 m_Route 父节点链一路 push_front 回溯回起点,
  //   得到从起点到目标的节点编号链表。
std::list<int> Graph_SearchDFS<graph_type>::GetPathToTarget()const 
{
  std::list<int> path;

  //just return an empty path if no path to target found or if
  //no target has been specified
  if (!m_bFound || m_iTarget<0) return path;

  int nd = m_iTarget;

  path.push_front(nd);

  while (nd != m_iSource)
  {
    nd = m_Route[nd];

    path.push_front(nd);
  }

  return path;
}



//----------------------------- Graph_SearchBFS -------------------------------
//
//-----------------------------------------------------------------------------
template<class graph_type>
//--------------------------------------------------------------------------------
// Graph_SearchBFS:广度优先搜索。与 DFS 几乎一样,唯一区别是用 std::queue
//   (队列,先进先出)代替栈——先扩散近的节点,再扩散远的;无权图中即最短路。
//--------------------------------------------------------------------------------
class Graph_SearchBFS
{
private:

  //to aid legibility
  enum {visited, unvisited, no_parent_assigned};

  //create a typedef for the edge type used by the graph
  typedef typename graph_type::EdgeType Edge;

private:

  //a reference to the graph to be searched
  const graph_type&      m_Graph;

  //this records the indexes of all the nodes that are visited as the
  //search progresses
  std::vector<int>  m_Visited;

  //this holds the route taken to the target. Given a node index, the value
  //at that index is the node's parent. ie if the path to the target is
  //3-8-27, then m_Route[8] will hold 3 and m_Route[27] will hold 8.
  std::vector<int>  m_Route;

  //the source and target node indices
  int               m_iSource,
                    m_iTarget;

  //true if a path to the target has been found
  bool              m_bFound;

  //As the search progresses, this will hold all the edges the algorithm has
  //examined. THIS IS NOT NECESSARY FOR THE SEARCH, IT IS HERE PURELY
  //TO PROVIDE THE USER WITH SOME VISUAL FEEDBACK
  std::vector<const Edge*>  m_SpanningTree;

  
  //the BFS algorithm is very similar to the DFS except that it uses a
  //FIFO queue instead of a stack.
  bool Search();
  
  
public:

  Graph_SearchBFS(const graph_type& graph,
             int          source,
             int          target = -1 ):m_Graph(graph),
                                        m_iSource(source),
                                        m_iTarget(target),
                                        m_bFound(false),
                                        m_Visited(m_Graph.NumNodes(), unvisited),
                                        m_Route(m_Graph.NumNodes(), no_parent_assigned)

  {                                                                         
    m_bFound = Search();   
  }

  bool   Found()const{return m_bFound;}

  //returns a vector containing pointers to all the edges the search has examined
  std::vector<const Edge*> GetSearchTree()const{return m_SpanningTree;}

  //returns a vector of node indexes that comprise the shortest path
  //from the source to the target
  std::list<int> GetPathToTarget()const; 
};

//-----------------------------------------------------------------------------

template <class graph_type>
  // BFS::Search:逻辑同 DFS,但用队列(front 取队首/pop 弹出)。
bool Graph_SearchBFS<graph_type>::Search()
{
  //create a std queue of edges
  std::queue<const Edge*> Q;

  const Edge Dummy(m_iSource, m_iSource, 0);

  //create a dummy edge and put on the queue
  Q.push(&Dummy);

  //mark the source node as visited
  m_Visited[m_iSource] = visited;

  //while there are edges in the queue keep searching
  while (!Q.empty())
  {
    //grab the next edge
    const Edge* Next = Q.front();

    Q.pop();

    //mark the parent of this node
    m_Route[Next->To()] = Next->From();

    //put it on the tree. (making sure the dummy edge is not placed on the tree)
    if (Next != &Dummy)
    {
      m_SpanningTree.push_back(Next);
    }

    //exit if the target has been found
    if (Next->To() == m_iTarget)
    {
      return true;
    }

    //push the edges leading from the node at the end of this edge 
    //onto the queue
    graph_type::ConstEdgeIterator ConstEdgeItr(m_Graph, Next->To());

    for (const Edge* pE=ConstEdgeItr.begin();
        !ConstEdgeItr.end();
         pE=ConstEdgeItr.next())
    {
      //if the node hasn't already been visited we can push the
      //edge onto the queue
      if (m_Visited[pE->To()] == unvisited)
      {
        Q.push(pE);

        //and mark it visited
        m_Visited[pE->To()] = visited;
      }
    }
  }

  //no path to target
  return false;
}


//-----------------------------------------------------------------------------
template <class graph_type>
std::list<int> Graph_SearchBFS<graph_type>::GetPathToTarget()const 
{
  std::list<int> path;

  //just return an empty path if no path to target found or if
  //no target has been specified
  if (!m_bFound || m_iTarget<0) return path;

  int nd = m_iTarget;

  path.push_front(nd);

  while (nd != m_iSource)
  {
    nd = m_Route[nd];

    path.push_front(nd);
  }

  return path;
}



//----------------------- Graph_SearchDijkstra --------------------------------
//
//  Given a graph, source and optional target this class solves for
//  single source shortest paths (without a target being specified) or 
//  shortest path from source to target.
//
//  The algorithm used is a priority queue implementation of Dijkstra's.
//  note how similar this is to the algorithm used in Graph_MinSpanningTree.
//  The main difference is in the calculation of the priority in the line:
//  
//  double NewCost = m_CostToThisNode[best] + pE->Cost;
//------------------------------------------------------------------------
template <class graph_type>
//--------------------------------------------------------------------------------
// Graph_SearchDijkstra:Dijkstra 单源最短路。
//   三个关键数组:
//     m_ShortestPathTree —— 最短路径树(每个节点"确定"的父边);
//     m_CostToThisNode   —— 当前已知到各节点的最小代价;
//     m_SearchFrontier   —— 边界(已发现但未确定的节点,每条的父边)。
//   用索引优先队列 IndexedPriorityQLow 按"代价最小"每次取一个节点扩展;
//   扩展时做"松弛(relax)":若经当前节点到邻居更便宜,就更新邻居代价并入队。
//--------------------------------------------------------------------------------
class Graph_SearchDijkstra
{
private:

  //create a typedef for the edge type used by the graph
  typedef typename graph_type::EdgeType Edge;
  
private:

  const graph_type&             m_Graph;

  //this vector contains the edges that comprise the shortest path tree -
  //a directed subtree of the graph that encapsulates the best paths from 
  //every node on the SPT to the source node.
  std::vector<const Edge*>      m_ShortestPathTree;

  //this is indexed into by node index and holds the total cost of the best
  //path found so far to the given node. For example, m_CostToThisNode[5]
  //will hold the total cost of all the edges that comprise the best path
  //to node 5, found so far in the search (if node 5 is present and has 
  //been visited)
  std::vector<double>            m_CostToThisNode; 

  //this is an indexed (by node) vector of 'parent' edges leading to nodes 
  //connected to the SPT but that have not been added to the SPT yet. This is
  //a little like the stack or queue used in BST and DST searches.
  std::vector<const Edge*>     m_SearchFrontier;

  int                           m_iSource;
  int                           m_iTarget;

  void Search();

public:

  Graph_SearchDijkstra(const graph_type   &graph,
                       int           source,
                       int           target = -1):m_Graph(graph),
                                       m_ShortestPathTree(graph.NumNodes()),                              
                                       m_SearchFrontier(graph.NumNodes()),
                                       m_CostToThisNode(graph.NumNodes()),
                                       m_iSource(source),
                                       m_iTarget(target)
  {                                           
    Search();     
  }
 
  //returns the vector of edges that defines the SPT. If a target was given
  //in the constructor then this will be an SPT comprising of all the nodes
  //examined before the target was found, else it will contain all the nodes
  //in the graph.
  std::vector<const Edge*> GetSPT()const{return m_ShortestPathTree;}

  //returns a vector of node indexes that comprise the shortest path
  //from the source to the target. It calculates the path by working
  //backwards through the SPT from the target node.
  std::list<int> GetPathToTarget()const;

  //returns the total cost to the target
  double GetCostToTarget()const{return m_CostToThisNode[m_iTarget];}

  //returns the total cost to the given node
  double GetCostToNode(unsigned int nd)const{return m_CostToThisNode[nd];}
};


//-----------------------------------------------------------------------------
template <class graph_type>
  // Dijkstra::Search:① 起点入队;② 循环取代价最小节点,放进 SPT;
  //   ③ 遍历其所有出边,做松弛(更便宜就更新代价/父边);④ 遇目标即返回。
void Graph_SearchDijkstra<graph_type>::Search()
{
  //create an indexed priority queue that sorts smallest to largest
  //(front to back).Note that the maximum number of elements the iPQ
  //may contain is N. This is because no node can be represented on the 
  //queue more than once.
  IndexedPriorityQLow<double> pq(m_CostToThisNode, m_Graph.NumNodes());

  //put the source node on the queue
  pq.insert(m_iSource);

  //while the queue is not empty
  while(!pq.empty())
  {
    //get lowest cost node from the queue. Don't forget, the return value
    //is a *node index*, not the node itself. This node is the node not already
    //on the SPT that is the closest to the source node
    int NextClosestNode = pq.Pop();

    //move this edge from the frontier to the shortest path tree
    m_ShortestPathTree[NextClosestNode] = m_SearchFrontier[NextClosestNode];

    //if the target has been found exit
    if (NextClosestNode == m_iTarget) return;

    //now to relax the edges.
    graph_type::ConstEdgeIterator ConstEdgeItr(m_Graph, NextClosestNode);

    //for each edge connected to the next closest node
    for (const Edge* pE=ConstEdgeItr.begin();
        !ConstEdgeItr.end();
        pE=ConstEdgeItr.next())
    {
      //the total cost to the node this edge points to is the cost to the
      //current node plus the cost of the edge connecting them.
      double NewCost = m_CostToThisNode[NextClosestNode] + pE->Cost();

      //if this edge has never been on the frontier make a note of the cost
      //to get to the node it points to, then add the edge to the frontier
      //and the destination node to the PQ.
      if (m_SearchFrontier[pE->To()] == 0)
      {
        m_CostToThisNode[pE->To()] = NewCost;

        pq.insert(pE->To());

        m_SearchFrontier[pE->To()] = pE;
      }

      //else test to see if the cost to reach the destination node via the
      //current node is cheaper than the cheapest cost found so far. If
      //this path is cheaper, we assign the new cost to the destination
      //node, update its entry in the PQ to reflect the change and add the
      //edge to the frontier
      else if ( (NewCost < m_CostToThisNode[pE->To()]) &&
                (m_ShortestPathTree[pE->To()] == 0) )
      {
        m_CostToThisNode[pE->To()] = NewCost;

        //because the cost is less than it was previously, the PQ must be
        //re-sorted to account for this.
        pq.ChangePriority(pE->To());

        m_SearchFrontier[pE->To()] = pE;
      }
    }
  }
}

//-----------------------------------------------------------------------------
template <class graph_type>
std::list<int> Graph_SearchDijkstra<graph_type>::GetPathToTarget()const
{
  std::list<int> path;

  //just return an empty path if no target or no path found
  if (m_iTarget < 0)  return path;

  int nd = m_iTarget;

  path.push_front(nd);

  while ((nd != m_iSource) && (m_ShortestPathTree[nd] != 0))
  {
    nd = m_ShortestPathTree[nd]->From();

    path.push_front(nd);
  }

  return path;
}

//------------------------------- Graph_SearchAStar --------------------------
//
//  this searchs a graph using the distance between the target node and the 
//  currently considered node as a heuristic.
//
//  This search is more commonly known as A* (pronounced Ay-Star)
//-----------------------------------------------------------------------------
template <class graph_type, class heuristic>
//--------------------------------------------------------------------------------
// Graph_SearchAStar:A* 搜索。比 Dijkstra 多一个模板参数 heuristic(启发函数类,
//   见 AStarHeuristicPolicies.h)。估价函数 F = G + H:
//     G(m_GCosts)= 从起点到当前节点的真实累计代价;
//     H         = 从当前节点到目标的估计距离(启发函数 Calculate 算);
//     F(m_FCosts)= G+H,优先队列按 F 排序——既已走代价小,又离目标近的节点先扩展。
//--------------------------------------------------------------------------------
class Graph_SearchAStar
{
private:

  //create a typedef for the edge type used by the graph
  typedef typename graph_type::EdgeType Edge;

private:

  const graph_type&              m_Graph;

  //indexed into my node. Contains the 'real' accumulative cost to that node
  std::vector<double>             m_GCosts; 

  //indexed into by node. Contains the cost from adding m_GCosts[n] to
  //the heuristic cost from n to the target node. This is the vector the
  //iPQ indexes into.
  std::vector<double>             m_FCosts;

  std::vector<const Edge*>       m_ShortestPathTree;
  std::vector<const Edge*>       m_SearchFrontier;

  int                            m_iSource;
  int                            m_iTarget;

  //the A* search algorithm
  void Search();

public:

  Graph_SearchAStar(graph_type &graph,
                    int   source,
                    int   target):m_Graph(graph),
                                  m_ShortestPathTree(graph.NumNodes()),                              
                                  m_SearchFrontier(graph.NumNodes()),
                                  m_GCosts(graph.NumNodes(), 0.0),
                                  m_FCosts(graph.NumNodes(), 0.0),
                                  m_iSource(source),
                                  m_iTarget(target)
  {
    Search();   
  }
 
  //returns the vector of edges that the algorithm has examined
  std::vector<const Edge*> GetSPT()const{return m_ShortestPathTree;}

  //returns a vector of node indexes that comprise the shortest path
  //from the source to the target
  std::list<int> GetPathToTarget()const;

  //returns the total cost to the target
  double GetCostToTarget()const{return m_GCosts[m_iTarget];}
};

//-----------------------------------------------------------------------------
template <class graph_type, class heuristic>
  // A*::Search:与 Dijkstra 几乎相同,区别在于入队排序用的是 F=G+H 而不是仅 G。
void Graph_SearchAStar<graph_type, heuristic>::Search()
{
  //create an indexed priority queue of nodes. The nodes with the
  //lowest overall F cost (G+H) are positioned at the front.
  IndexedPriorityQLow<double> pq(m_FCosts, m_Graph.NumNodes());

  //put the source node on the queue
  pq.insert(m_iSource);

  //while the queue is not empty
  while(!pq.empty())
  {
    //get lowest cost node from the queue
    int NextClosestNode = pq.Pop();

    //move this node from the frontier to the spanning tree
    m_ShortestPathTree[NextClosestNode] = m_SearchFrontier[NextClosestNode];

    //if the target has been found exit
    if (NextClosestNode == m_iTarget) return;

    //now to test all the edges attached to this node
    graph_type::ConstEdgeIterator ConstEdgeItr(m_Graph, NextClosestNode);

    for (const Edge* pE=ConstEdgeItr.begin();
        !ConstEdgeItr.end(); 
         pE=ConstEdgeItr.next())
    {
      //calculate the heuristic cost from this node to the target (H)                       
      double HCost = heuristic::Calculate(m_Graph, m_iTarget, pE->To()); 

      //calculate the 'real' cost to this node from the source (G)
      double GCost = m_GCosts[NextClosestNode] + pE->Cost();

      //if the node has not been added to the frontier, add it and update
      //the G and F costs
      if (m_SearchFrontier[pE->To()] == NULL)
      {
        m_FCosts[pE->To()] = GCost + HCost;
        m_GCosts[pE->To()] = GCost;

        pq.insert(pE->To());

        m_SearchFrontier[pE->To()] = pE;
      }

      //if this node is already on the frontier but the cost to get here
      //is cheaper than has been found previously, update the node
      //costs and frontier accordingly.
      else if ((GCost < m_GCosts[pE->To()]) && (m_ShortestPathTree[pE->To()]==NULL))
      {
        m_FCosts[pE->To()] = GCost + HCost;
        m_GCosts[pE->To()] = GCost;

        pq.ChangePriority(pE->To());

        m_SearchFrontier[pE->To()] = pE;
      }
    }
  }
}

//-----------------------------------------------------------------------------
template <class graph_type, class heuristic>
std::list<int> Graph_SearchAStar<graph_type, heuristic>::GetPathToTarget()const
{
  std::list<int> path;

  //just return an empty path if no target or no path found
  if (m_iTarget < 0)  return path;    

  int nd = m_iTarget;

  path.push_front(nd);
    
  while ((nd != m_iSource) && (m_ShortestPathTree[nd] != 0))
  {
    nd = m_ShortestPathTree[nd]->From();

    path.push_front(nd);
  }

  return path;
} 



//---------------------- Graph_MinSpanningTree --------------------------------
//
//  given a graph and a source node you can use this class to calculate
//  the minimum spanning tree. If no source node is specified then the 
//  algorithm will calculate a spanning forest starting from node 1 
//
//  It uses a priority first queue implementation of Prims algorithm
//------------------------------------------------------------------------
template <class graph_type>
//--------------------------------------------------------------------------------
// Graph_MinSpanningTree:Prim 最小生成树。目的不是"找路",而是用最少量的总代价
//   把所有节点连起来。算法与 Dijkstra 极像,但每次取"连到树的边"中代价最小的;
//   若构造时不给起点(source<0),就从每个未连节点出发各跑一遍,生成"生成森林"。
//--------------------------------------------------------------------------------
class Graph_MinSpanningTree
{
private:

  //create a typedef for the edge type used by the graph
  typedef typename graph_type::EdgeType Edge;

  const graph_type&              m_Graph;

  std::vector<double>            m_CostToThisNode; 

  std::vector<const Edge*>  m_SpanningTree;
  std::vector<const Edge*>  m_Fringe;

  void Search(const int source)
  {
    //create a priority queue
    IndexedPriorityQLow<double> pq(m_CostToThisNode, m_Graph.NumNodes());

    //put the source node on the queue
    pq.insert(source);

    //while the queue is not empty
    while(!pq.empty())
    {
      //get lowest cost edge from the queue
      int best = pq.Pop();

      //move this edge from the fringe to the spanning tree
      m_SpanningTree[best] = m_Fringe[best];

      //now to test the edges attached to this node
      graph_type::ConstEdgeIterator ConstEdgeItr(m_Graph, best);

      for (const Edge* pE=ConstEdgeItr.beg(); !ConstEdgeItr.end(); pE=ConstEdgeItr.nxt())
      {
        double Priority = pE->Cost;

        if (m_Fringe[pE->To()] == 0)
        {
          m_CostToThisNode[pE->To()] = Priority;

          pq.insert(pE->To());

          m_Fringe[pE->To()] = pE;
        }

        else if ((Priority<m_CostToThisNode[pE->To()]) && (m_SpanningTree[pE->To()]==0))
        {
          m_CostToThisNode[pE->To()] = Priority;

          pq.ChangePriority(pE->To());

          m_Fringe[pE->To()] = pE;
        }
      }
    }
  }

public:

  Graph_MinSpanningTree(graph_type &G,
                   int   source = -1):m_Graph(G),
                                   m_SpanningTree(G.NumNodes()),                              
                                   m_Fringe(G.NumNodes()),
                                   m_CostToThisNode(G.NumNodes(), -1)
  {                                                                             
    if (source < 0)
    {
      for (int nd=0; nd<G.NumNodes(); ++nd)
      {
        if (m_SpanningTree[nd] == 0)
        {
          Search(nd);
        }
      }
    }

    else
    {
      Search(source);   
    }
  }

  std::vector<const Edge*> GetSpanningTree()const{return m_SpanningTree;}
  
};



#endif