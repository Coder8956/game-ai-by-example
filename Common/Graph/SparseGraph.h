//==============================================================================================
//【文件说明】SparseGraph.h —— 图论模块的核心:稀疏图(邻接表实现)
//
//【这个文件是干什么的?】
//  这是第 5 章寻路工程(以及 Raven)真正使用的"图"类。所谓"稀疏图"(sparse graph),
//  是指一张图里大多数节点之间并没有边相连(地图上几百个路口,每个路口只连几条路)。
//  它用"邻接表"(adjacency list)存储:每个节点配一个链表,只装"自己连出去的边"。
//  对比"邻接矩阵"(n×n 方阵),邻接表省内存、适合边稀疏的真实地图。
//
//【谁在使用这个文件?】
//  GraphAlgorithms.h(Dijkstra/A* 算法)、HandyGraphFunctions.h(建网格/画图);
//  第 5 章 Pathfinder.cpp 与 Raven(第 7~10 章)用它表示导航图。
//
//【本文件包含了谁?】
//  <vector>/<list>/<cassert>/<string>/<iostream> —— 标准库(动态数组/链表/断言);
//  "2D/Vector2D.h"、"misc/utils.h"、"graph/NodeTypeEnumerations.h"。
//
//【C++ 小课堂:模板 + typedef + 迭代器】
//  本类是模板类:template <class node_type, class edge_type> 表示节点类型、边类型
//  都由使用者指定(基础图/导航图各配不同的节点和边)。
//  typedef 给复杂类型起短名:如 typedef std::vector<node_type> NodeVector;
//    之后写 NodeVector 就等价于 std::vector<node_type>。
//  迭代器(iterator)是标准库容器的"指针式游标":begin() 指向第一个,
//    ++it 走到下一个,end() 表示已走到末尾。
//==============================================================================================
#ifndef SPARSEGRAPH_H
#define SPARSEGRAPH_H
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786) 原理详见 SoccerPitch.h 中的同名注释(历史遗留警告)。
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   SparseGraph.h
//
//  Desc:   Graph class using the adjacency list representation.
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <vector>
#include <list>
#include <cassert>
#include <string>
#include <iostream>


#include "2D/Vector2D.h"
#include "misc/utils.h" 
#include "graph/NodeTypeEnumerations.h"



template <class node_type, class edge_type>   
//--------------------------------------------------------------------------------
// 上面 template <class node_type, class edge_type> 是模板声明:
//   node_type = 节点类型;edge_type = 边类型,都由使用方在实例化时指定。
// class SparseGraph —— 稀疏图类(邻接表)。
//--------------------------------------------------------------------------------
class SparseGraph                                 
{
public:

//--------------------------------------------------------------------------------
// 下面 5 行 typedef(类型别名):给外层使用者一个"短名",方便他们写代码。
//   EdgeType/NodeType      —— 把模板参数 node_type/edge_type 暴露出去;
//   NodeVector             —— std::vector<node_type> = 装所有节点的动态数组;
//   EdgeList               —— std::list<edge_type>  = 一个节点的所有出边链表;
//   EdgeListVector         —— vector<EdgeList> = 每个节点各配一条边链表的大数组。
//--------------------------------------------------------------------------------
  //enable easy client access to the edge and node types used in the graph
  typedef edge_type                EdgeType;
  typedef node_type                NodeType;
  
  //a couple more typedefs to save my fingers and to help with the formatting
  //of the code on the printed page
  typedef std::vector<node_type>   NodeVector;
  typedef std::list<edge_type>     EdgeList;
  typedef std::vector<EdgeList>    EdgeListVector;

 
private:
  
//--------------------------------------------------------------------------------
// private 成员(只有本类能直接访问):
//   m_Nodes        —— 全部节点(按下标编号存储);
//   m_Edges        —— 邻接表:m_Edges[i] 就是节点 i 的所有出边链表;
//   m_bDigraph     —— true=有向图(边只走一个方向),false=无向(自动加反向边);
//   m_iNextNodeIndex —— 下一个新节点要用的编号(自增计数器)。
//--------------------------------------------------------------------------------
  //the nodes that comprise this graph
  NodeVector      m_Nodes;

  //a vector of adjacency edge lists. (each node index keys into the 
  //list of edges associated with that node)
  EdgeListVector  m_Edges;
 
  //is this a directed graph?
  bool            m_bDigraph;

  //the index of the next node to be added
  int             m_iNextNodeIndex;
  
    
  // UniqueEdge:新加边前检查"这条 from→to 的边是否已存在",防止重复连边。
  // CullInvalidEdges:删除节点后,清理所有指向"已作废节点"的悬空边。
  //returns true if an edge is not already present in the graph. Used
  //when adding edges to make sure no duplicates are created.
  bool  UniqueEdge(int from, int to)const;

  //iterates through all the edges in the graph and removes any that point
  //to an invalidated node
  void  CullInvalidEdges();
  
public:
  
//--------------------------------------------------------------------------------
// 构造函数:参数 digraph = 是否有向图;nextNodeIndex 从 0 开始。
//--------------------------------------------------------------------------------
  //ctor
  SparseGraph(bool digraph): m_iNextNodeIndex(0), m_bDigraph(digraph){}

  // GetNode/GetEdge 各有两份:const 版(只读)与非 const 版(可改)。
  //returns the node at the given index
  const NodeType&  GetNode(int idx)const;

  //non const version
  NodeType&  GetNode(int idx);

  //const method for obtaining a reference to an edge
  const EdgeType& GetEdge(int from, int to)const;

  //non const version
  EdgeType& GetEdge(int from, int to);
    

  // 下面是图的核心操作:GetNextFreeNodeIndex 取下一个可用编号;AddNode 加节点;
  // RemoveNode 标记作废(不真删,避免挪动所有下标);AddEdge/RemoveEdge 加/删边;
  // SetEdgeCost 改某条边的代价;NumNodes/NumActiveNodes/NumEdges 统计数量。
  //retrieves the next free node index
  int   GetNextFreeNodeIndex()const{return m_iNextNodeIndex;}
  
  //adds a node to the graph and returns its index
  int   AddNode(node_type node);

  //removes a node by setting its index to invalid_node_index
  void  RemoveNode(int node);

  //Use this to add an edge to the graph. The method will ensure that the
  //edge passed as a parameter is valid before adding it to the graph. If the
  //graph is a digraph then a similar edge connecting the nodes in the opposite
  //direction will be automatically added.
  void  AddEdge(EdgeType edge);

  //removes the edge connecting from and to from the graph (if present). If
  //a digraph then the edge connecting the nodes in the opposite direction 
  //will also be removed.
  void  RemoveEdge(int from, int to);

  //sets the cost of an edge
  void  SetEdgeCost(int from, int to, double cost);

  //returns the number of active + inactive nodes present in the graph
  int   NumNodes()const{return m_Nodes.size();}
  
  //returns the number of active nodes present in the graph (this method's
  //performance can be improved greatly by caching the value)
  int   NumActiveNodes()const
  {
    int count = 0;

    for (unsigned int n=0; n<m_Nodes.size(); ++n) if (m_Nodes[n].Index() != invalid_node_index) ++count;

    return count;
  }

  //returns the total number of edges present in the graph
  int   NumEdges()const
  {
    int tot = 0;

    for (EdgeListVector::const_iterator curEdge = m_Edges.begin();
         curEdge != m_Edges.end();
         ++curEdge)
    {
      tot += curEdge->size();
    }

    return tot;
  }

  //returns true if the graph is directed
  bool  isDigraph()const{return m_bDigraph;}

  //returns true if the graph contains no nodes
  bool	isEmpty()const{return m_Nodes.empty();}

  //returns true if a node with the given index is present in the graph
  bool isNodePresent(int nd)const;

  //returns true if an edge connecting the nodes 'to' and 'from'
  //is present in the graph
  bool isEdgePresent(int from, int to)const;

  //methods for loading and saving graphs from an open file stream or from
  //a file name 
  bool  Save(const char* FileName)const;
  bool  Save(std::ofstream& stream)const;

  bool  Load(const char* FileName);
  bool  Load(std::ifstream& stream);

  //clears the graph ready for new node insertions
  void Clear(){m_iNextNodeIndex = 0; m_Nodes.clear(); m_Edges.clear();}

  void RemoveEdges()
  {
    for (EdgeListVector::iterator it = m_Edges.begin(); it != m_Edges.end(); ++it)
    {
      it->clear();
    }
  }

  
//--------------------------------------------------------------------------------
// 嵌套迭代器类(定义在图类内部,"图的专用游标"):
//   EdgeIterator       —— 遍历某节点的所有出边(可改);
//   ConstEdgeIterator  —— 同上,但只读;
//   NodeIterator      —— 遍历图中所有有效节点(可改,自动跳过已作废节点);
//   ConstNodeIterator  —— 只读版。
//  每个迭代器都提供 begin()/next()/end() 三件套:begin 回到开头,next 走下一个,
//  end() 返回 true 表示已经走完。friend class 让迭代器能直接访问图的私有成员。
//--------------------------------------------------------------------------------
    //non const class used to iterate through all the edges connected to a specific node. 
      class EdgeIterator
      {
      private:                                                                

        typename EdgeList::iterator         curEdge;

        SparseGraph<node_type, edge_type>&  G;

        const int                           NodeIndex;

      public:

        EdgeIterator(SparseGraph<node_type, edge_type>& graph,
                     int                                node): G(graph),
                                                               NodeIndex(node)
        {
          /* we don't need to check for an invalid node index since if the node is
             invalid there will be no associated edges
         */

          curEdge = G.m_Edges[NodeIndex].begin();
        }

        EdgeType*  begin()
        {        
          curEdge = G.m_Edges[NodeIndex].begin();
    
          return &(*curEdge);
        }

        EdgeType*  next()
        {
          ++curEdge;

		  if (end()) return NULL;
    
          return &(*curEdge);

        }

        //return true if we are at the end of the edge list
        bool end()
        {
          return (curEdge == G.m_Edges[NodeIndex].end());
        }
      };

  friend class EdgeIterator;

  //const class used to iterate through all the edges connected to a specific node. 
      class ConstEdgeIterator
      {
      private:                                                                

        typename EdgeList::const_iterator        curEdge;

        const SparseGraph<node_type, edge_type>& G;

        const int                                NodeIndex;

      public:

        ConstEdgeIterator(const SparseGraph<node_type, edge_type>& graph,
                          int                           node): G(graph),
                                                               NodeIndex(node)
        {
          /* we don't need to check for an invalid node index since if the node is
             invalid there will be no associated edges
         */

          curEdge = G.m_Edges[NodeIndex].begin();
        }

        const EdgeType*  begin()
        {        
          curEdge = G.m_Edges[NodeIndex].begin();
    
          return &(*curEdge);
        }

        const EdgeType*  next()
        {
          ++curEdge;

		  if(end())
		  {
			  return NULL;
		  }
		  else
		  {
			return &(*curEdge);
		  }

        }

        //return true if we are at the end of the edge list
        bool end()
        {
          return (curEdge == G.m_Edges[NodeIndex].end());
        }
      };

  friend class ConstEdgeIterator;

  //non const class used to iterate through the nodes in the graph
    class NodeIterator
    {
    private:

      typename NodeVector::iterator         curNode;
      
      SparseGraph<node_type, edge_type>&    G;

      //if a graph node is removed, it is not removed from the 
      //vector of nodes (because that would mean changing all the indices of 
      //all the nodes that have a higher index). This method takes a node
      //iterator as a parameter and assigns the next valid element to it.
      void GetNextValidNode(typename NodeVector::iterator& it)
      {
        if ( curNode == G.m_Nodes.end() || it->Index() != invalid_node_index) return;

        while ( (it->Index() == invalid_node_index) )
        {
          ++it;

          if (curNode == G.m_Nodes.end()) break;
        }
      }

    public:
      
      NodeIterator(SparseGraph<node_type, edge_type> &graph):G(graph)
      {
        curNode = G.m_Nodes.begin();
      }


      node_type* begin()
      {      
        curNode = G.m_Nodes.begin();

        GetNextValidNode(curNode);

        return &(*curNode);
      }

      node_type* next()
      {
        ++curNode;

		if (end()) return NULL;

        GetNextValidNode(curNode);

        return &(*curNode);
      }

      bool end()
      {
        return (curNode == G.m_Nodes.end());
      }
    };

     
  friend class NodeIterator;

    //const class used to iterate through the nodes in the graph
    class ConstNodeIterator
    {
    private:

      typename NodeVector::const_iterator			curNode;

      const SparseGraph<node_type, edge_type>&      G;

      //if a graph node is removed or switched off, it is not removed from the 
      //vector of nodes (because that would mean changing all the indices of 
      //all the nodes that have a higher index. This method takes a node
      //iterator as a parameter and assigns the next valid element to it.
      void GetNextValidNode(typename NodeVector::const_iterator& it)
      {
        if ( curNode == G.m_Nodes.end() || it->Index() != invalid_node_index) return;

        while ( (it->Index() == invalid_node_index) )
        {
          ++it;

          if (curNode == G.m_Nodes.end()) break;
        }
      }

    public:

      ConstNodeIterator(const SparseGraph<node_type, edge_type> &graph):G(graph)
      {
        curNode = G.m_Nodes.begin();
      }


      const node_type* begin()
      {      
        curNode = G.m_Nodes.begin();

        GetNextValidNode(curNode);

        return &(*curNode);
      }

      const node_type* next()
      {
        ++curNode;

		if (end())
		{
			return NULL;
		}
		else
		{
			GetNextValidNode(curNode);

			return &(*curNode);
		}
      }

      bool end()
      {
        return (curNode == G.m_Nodes.end());
      }
    };

  friend class ConstNodeIterator;
};


//--------------------------- isNodePresent --------------------------------
//
//  returns true if a node with the given index is present in the graph
//--------------------------------------------------------------------------
template <class node_type, class edge_type>
//--------------------------------------------------------------------------------
// 下面是在类外实现的成员函数(模板类的实现习惯写在 .h 里,因为编译器要看到源码
// 才能按使用者给出的节点/边类型生成具体代码)。
// 函数名写法:返回值 类名<模板参数>::函数名 —— 作用域解析符::表示"这是模板类的成员函数"。
// isNodePresent:编号 nd 在图中且未被作废 → 返回 true。
//--------------------------------------------------------------------------------
bool SparseGraph<node_type, edge_type>::isNodePresent(int nd)const
{
    if ((nd >= (int)m_Nodes.size() || (m_Nodes[nd].Index() == invalid_node_index)))
    {
      return false;
    }
    else return true;
}

//--------------------------- isEdgePresent --------------------------------
//
//  returns true if an edge with the given from/to is present in the graph
//--------------------------------------------------------------------------
template <class node_type, class edge_type>
  // isEdgePresent:遍历 from 节点的出边链表,看有没有终点为 to 的边。
bool SparseGraph<node_type, edge_type>::isEdgePresent(int from, int to)const
{
    if (isNodePresent(from) && isNodePresent(from))
    {
       for (EdgeList::const_iterator curEdge = m_Edges[from].begin();
            curEdge != m_Edges[from].end();
            ++curEdge)
        {
          if (curEdge->To() == to) return true;
        }

        return false;
    }
    else return false;
}
//------------------------------ GetNode -------------------------------------
//
//  const and non const methods for obtaining a reference to a specific node
//----------------------------------------------------------------------------
template <class node_type, class edge_type>
  // GetNode(只读):断言下标合法后返回节点引用(引用 = 别名,不复制对象)。
const node_type&  SparseGraph<node_type, edge_type>::GetNode(int idx)const
{
    assert( (idx < (int)m_Nodes.size()) &&
            (idx >=0)              &&
           "<SparseGraph::GetNode>: invalid index");

    return m_Nodes[idx];
}

  //non const version
template <class node_type, class edge_type>
node_type&  SparseGraph<node_type, edge_type>::GetNode(int idx)
{
    assert( (idx < (int)m_Nodes.size()) &&
            (idx >=0)             &&
          "<SparseGraph::GetNode>: invalid index");
    
    return m_Nodes[idx];
}

//------------------------------ GetEdge -------------------------------------
//
//  const and non const methods for obtaining a reference to a specific edge
//----------------------------------------------------------------------------
template <class node_type, class edge_type>
  // GetEdge(只读):先断言两节点合法,再在 from 的出边链表中找 to 边;
  // 找不到就 assert(0 && ...) 让程序在调试期立刻报错。
const edge_type& SparseGraph<node_type, edge_type>::GetEdge(int from, int to)const
{
  assert( (from < m_Nodes.size()) &&
          (from >=0)              &&
           m_Nodes[from].Index() != invalid_node_index &&
          "<SparseGraph::GetEdge>: invalid 'from' index");

  assert( (to < m_Nodes.size()) &&
          (to >=0)              &&
          m_Nodes[to].Index() != invalid_node_index &&
          "<SparseGraph::GetEdge>: invalid 'to' index");

  for (EdgeList::const_iterator curEdge = m_Edges[from].begin();
       curEdge != m_Edges[from].end();
       ++curEdge)
  {
    if (curEdge->To() == to) return *curEdge;
  }

  assert (0 && "<SparseGraph::GetEdge>: edge does not exist");
}

//non const version
template <class node_type, class edge_type>
edge_type& SparseGraph<node_type, edge_type>::GetEdge(int from, int to)
{
  assert( (from < m_Nodes.size()) &&
          (from >=0)              &&
           m_Nodes[from].Index() != invalid_node_index &&
          "<SparseGraph::GetEdge>: invalid 'from' index");

  assert( (to < m_Nodes.size()) &&
          (to >=0)              &&
          m_Nodes[to].Index() != invalid_node_index &&
          "<SparseGraph::GetEdge>: invalid 'to' index");

  for (EdgeList::iterator curEdge = m_Edges[from].begin();
       curEdge != m_Edges[from].end();
       ++curEdge)
  {
    if (curEdge->To() == to) return *curEdge;
  }

  assert (0 && "<SparseGraph::GetEdge>: edge does not exist");
}

//-------------------------- AddEdge ------------------------------------------
//
//  Use this to add an edge to the graph. The method will ensure that the
//  edge passed as a parameter is valid before adding it to the graph. If the
//  graph is a digraph then a similar edge connecting the nodes in the opposite
//  direction will be automatically added.
//-----------------------------------------------------------------------------
template <class node_type, class edge_type>
//--------------------------------------------------------------------------------
// AddEdge:加一条边。先断言两节点编号合法、两节点都未作废;
// 再用 UniqueEdge 查重,没有重复就 push_back 进 m_Edges[from] 链表;
// 若为无向图(!m_bDigraph),还要复制一条反向边(to→from)加进去。
//--------------------------------------------------------------------------------
void SparseGraph<node_type, edge_type>::AddEdge(EdgeType edge)
{
  //first make sure the from and to nodes exist within the graph 
  assert( (edge.From() < m_iNextNodeIndex) && (edge.To() < m_iNextNodeIndex) &&
          "<SparseGraph::AddEdge>: invalid node index");

  //make sure both nodes are active before adding the edge
  if ( (m_Nodes[edge.To()].Index() != invalid_node_index) && 
       (m_Nodes[edge.From()].Index() != invalid_node_index))
  {
    //add the edge, first making sure it is unique
    if (UniqueEdge(edge.From(), edge.To()))
    {
      m_Edges[edge.From()].push_back(edge);
    }

    //if the graph is undirected we must add another connection in the opposite
    //direction
    if (!m_bDigraph)
    {
      //check to make sure the edge is unique before adding
      if (UniqueEdge(edge.To(), edge.From()))
      {
        EdgeType NewEdge = edge;

        NewEdge.SetTo(edge.From());
        NewEdge.SetFrom(edge.To());

        m_Edges[edge.To()].push_back(NewEdge);
      }
    }
  }
}


//----------------------------- RemoveEdge ---------------------------------
template <class node_type, class edge_type>
  // RemoveEdge:在 from 的出边链表中找到 to 边并用 erase 删除;
  // 无向图时还要顺便删掉 to→from 的反向边。
void SparseGraph<node_type, edge_type>::RemoveEdge(int from, int to)
{
  assert ( (from < (int)m_Nodes.size()) && (to < (int)m_Nodes.size()) &&
           "<SparseGraph::RemoveEdge>:invalid node index");

  EdgeList::iterator curEdge;
  
  if (!m_bDigraph)
  {
    for (curEdge = m_Edges[to].begin();
         curEdge != m_Edges[to].end();
         ++curEdge)
    {
      if (curEdge->To() == from){curEdge = m_Edges[to].erase(curEdge);break;}
    }
  }

  for (curEdge = m_Edges[from].begin();
       curEdge != m_Edges[from].end();
       ++curEdge)
  {
    if (curEdge->To() == to){curEdge = m_Edges[from].erase(curEdge);break;}
  }
}

//-------------------------- AddNode -------------------------------------
//
//  Given a node this method first checks to see if the node has been added
//  previously but is now innactive. If it is, it is reactivated.
//
//  If the node has not been added previously, it is checked to make sure its
//  index matches the next node index before being added to the graph
//------------------------------------------------------------------------
template <class node_type, class edge_type>
//--------------------------------------------------------------------------------
// AddNode:加节点。两种情况:
//   ① 节点编号已存在(说明之前被作废过)→ 重新激活,放回原位置;
//   ② 全新编号 → push_back 进节点数组,并给它配一条空的边链表,编号自增。
//--------------------------------------------------------------------------------
int SparseGraph<node_type, edge_type>::AddNode(node_type node)
{
  if (node.Index() < (int)m_Nodes.size())
  {
    //make sure the client is not trying to add a node with the same ID as
    //a currently active node
    assert (m_Nodes[node.Index()].Index() == invalid_node_index &&
      "<SparseGraph::AddNode>: Attempting to add a node with a duplicate ID");
    
    m_Nodes[node.Index()] = node;

    return m_iNextNodeIndex;
  }
  
  else
  {
    //make sure the new node has been indexed correctly
    assert (node.Index() == m_iNextNodeIndex && "<SparseGraph::AddNode>:invalid index");

    m_Nodes.push_back(node);
    m_Edges.push_back(EdgeList());

    return m_iNextNodeIndex++;
  }
}

//----------------------- CullInvalidEdges ------------------------------------
//
//  iterates through all the edges in the graph and removes any that point
//  to an invalidated node
//-----------------------------------------------------------------------------
template <class node_type, class edge_type>
  // CullInvalidEdges:双重循环遍历所有边,凡端点是 invalid_node_index 的就 erase 掉。
void SparseGraph<node_type, edge_type>::CullInvalidEdges()
{
  for (EdgeListVector::iterator curEdgeList = m_Edges.begin(); curEdgeList != m_Edges.end(); ++curEdgeList)
  {
    for (EdgeList::iterator curEdge = (*curEdgeList).begin(); curEdge != (*curEdgeList).end(); ++curEdge)
    {
      if (m_Nodes[curEdge->To()].Index() == invalid_node_index || 
          m_Nodes[curEdge->From()].Index() == invalid_node_index)
      {
        curEdge = (*curEdgeList).erase(curEdge);
      }
    }
  }
}

  
//------------------------------- RemoveNode -----------------------------
//
//  Removes a node from the graph and removes any links to neighbouring
//  nodes
//------------------------------------------------------------------------
template <class node_type, class edge_type>
//--------------------------------------------------------------------------------
// RemoveNode:把节点编号设成 invalid_node_index(逻辑删除,不物理挪动下标)。
//   无向图:手工清掉所有邻居指向自己的边,再清空自己的边链表;
//   有向图:直接调 CullInvalidEdges 统一清理。
//--------------------------------------------------------------------------------
void SparseGraph<node_type, edge_type>::RemoveNode(int node)                                   
{
  assert(node < (int)m_Nodes.size() && "<SparseGraph::RemoveNode>: invalid node index");

  //set this node's index to invalid_node_index
  m_Nodes[node].SetIndex(invalid_node_index);

  //if the graph is not directed remove all edges leading to this node and then
  //clear the edges leading from the node
  if (!m_bDigraph)
  {    
    //visit each neighbour and erase any edges leading to this node
    for (EdgeList::iterator curEdge = m_Edges[node].begin(); 
         curEdge != m_Edges[node].end();
         ++curEdge)
    {
      for (EdgeList::iterator curE = m_Edges[curEdge->To()].begin();
           curE != m_Edges[curEdge->To()].end();
           ++curE)
      {
         if (curE->To() == node)
         {
           curE = m_Edges[curEdge->To()].erase(curE);

           break;
         }
      }
    }

    //finally, clear this node's edges
    m_Edges[node].clear();
  }

  //if a digraph remove the edges the slow way
  else
  {
    CullInvalidEdges();
  }
}

//-------------------------- SetEdgeCost ---------------------------------
//
//  Sets the cost of a specific edge
//------------------------------------------------------------------------
template <class node_type, class edge_type>
  // SetEdgeCost:在 from 的出边链表中找到 to 边,把它的代价改成 NewCost。
void SparseGraph<node_type, edge_type>::SetEdgeCost(int from, int to, double NewCost)
{
  //make sure the nodes given are valid
  assert( (from < (int)m_Nodes.size()) && (to < (int)m_Nodes.size()) &&
        "<SparseGraph::SetEdgeCost>: invalid index");

  //visit each neighbour and erase any edges leading to this node
  for (EdgeList::iterator curEdge = m_Edges[from].begin(); 
       curEdge != m_Edges[from].end();
       ++curEdge)
  {
    if (curEdge->To() == to)
    {
      curEdge->SetCost(NewCost);
      break;
    }
  }
}

  //-------------------------------- UniqueEdge ----------------------------
//
//  returns true if the edge is not present in the graph. Used when adding
//  edges to prevent duplication
//------------------------------------------------------------------------
template <class node_type, class edge_type>
  // UniqueEdge:遍历 from 的出边,若已存在 to 边返回 false(不唯一),否则 true。
bool SparseGraph<node_type, edge_type>::UniqueEdge(int from, int to)const
{
  for (EdgeList::const_iterator curEdge = m_Edges[from].begin();
       curEdge != m_Edges[from].end();
       ++curEdge)
  {
    if (curEdge->To() == to)
    {
      return false;
    }
  }

  return true;
}

//-------------------------------- Save ---------------------------------------

template <class node_type, class edge_type>
  // Save/Load 各有两个重载:按文件名版(自己打开文件)和按文件流版(由调用者传入流)。
  // 存盘顺序:节点数 → 所有节点 → 边数 → 所有边;读盘时按同序恢复。
bool SparseGraph<node_type, edge_type>::Save(const char* FileName)const
{
  //open the file and make sure it's valid
  std::ofstream out(FileName);

  if (!out)
  {
    throw std::runtime_error("Cannot open file: " + std::string(FileName));
    return false;
  }

  return Save(out);
}

//-------------------------------- Save ---------------------------------------
template <class node_type, class edge_type>
bool SparseGraph<node_type, edge_type>::Save(std::ofstream& stream)const
{
  //save the number of nodes
  stream << m_Nodes.size() << std::endl;

  //iterate through the graph nodes and save them
  NodeVector::const_iterator curNode = m_Nodes.begin();
  for (curNode; curNode!=m_Nodes.end(); ++curNode)
  {
    stream << *curNode;
  }

  //save the number of edges
  stream << NumEdges() << std::endl;


  //iterate through the edges and save them
  for (unsigned int nodeIdx = 0; nodeIdx < m_Nodes.size(); ++nodeIdx)
  {
    for (EdgeList::const_iterator curEdge = m_Edges[nodeIdx].begin();
         curEdge!=m_Edges[nodeIdx].end(); ++curEdge)
    {
      stream << *curEdge;
    }  
  }

  return true;
}

//------------------------------- Load ----------------------------------------
//-----------------------------------------------------------------------------
template <class node_type, class edge_type>
bool SparseGraph<node_type, edge_type>::Load(const char* FileName)
{
  //open file and make sure it's valid
  std::ifstream in(FileName);

  if (!in)
  {
    throw std::runtime_error("Cannot open file: " + std::string(FileName));
    return false;
  }

  return Load(in);
}

//------------------------------- Load ----------------------------------------
//-----------------------------------------------------------------------------
template <class node_type, class edge_type>
bool SparseGraph<node_type, edge_type>::Load(std::ifstream& stream)
{
  Clear();

  //get the number of nodes and read them in
  int NumNodes, NumEdges;

  stream >> NumNodes;

  for (int n=0; n<NumNodes; ++n)
  {
    NodeType NewNode(stream);
  
    //when editing graphs it's possible to end up with a situation where some
    //of the nodes have been invalidated (their id's set to invalid_node_index). Therefore
    //when a node of index invalid_node_index is encountered, it must still be added.
    if (NewNode.Index() != invalid_node_index)
    {
      AddNode(NewNode);
    }
    else
    {
      m_Nodes.push_back(NewNode);

      //make sure an edgelist is added for each node
      m_Edges.push_back(EdgeList());
      
      ++m_iNextNodeIndex;
    }
  }

  //now add the edges
  stream >> NumEdges;
  for (int e=0; e<NumEdges; ++e)
  {
    EdgeType NextEdge(stream);

    AddEdge(NextEdge);
  }

  return true;
}
   

#endif