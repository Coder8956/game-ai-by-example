//==============================================================================================
//【文件说明】HandyGraphFunctions.h —— 图论模块的"工具箱"(建网格图、画图、加权表)
//
//【这个文件是干什么的?】
//  图(SparseGraph)本身只是"节点+边"的数据结构;本文件提供配套的实用函数:
//    ValidNeighbour                  —— 判断网格坐标有没有越界;
//    GraphHelper_AddAllNeighboursToGridNode —— 给网格节点连上周围 8 个邻居;
//    GraphHelper_CreateGrid          —— 一键生成一张铺满矩形区域的网格图;
//    GraphHelper_DrawUsingGDI        —— 用 GDI 把图画到屏幕上(调试用);
//    WeightNavGraphNodeEdges         —— 按地形权重缩放某节点所有边的代价;
//    CreateAllPairsTable             —— 预计算"任意两节点间"的最短路径(全对表);
//    CreateAllPairsCostsTable        —— 预计算"任意两节点间"的最短代价(全对代价表);
//    CalculateAverageGraphEdgeLength / GetCostliestGraphEdge —— 图统计。
//
//【谁在使用这个文件?】
//  第 5 章 Pathfinder 工程的 Pathfinder.cpp/main.cpp 用它建网格图、画图;
//  Raven(第 7~10 章)也用 WeightNavGraphNodeEdges 给地形加权。
//
//【本文件包含了谁?】
//  <iostream>                       —— 标准输入输出;
//  "misc/Cgdi.h"                   —— 绘图单例 gdi(画线/圆/文字);
//  "misc/utils.h"                  —— 工具函数(Vec2DDistance、MinDouble 等);
//  "misc/Stream_Utility_Functions.h" —— ttos(整数转字符串)等;
//  "Graph/GraphAlgorithms.h"        —— Dijkstra/A* 搜索算法类;
//  "Graph/AStarHeuristicPolicies.h" —— A* 启发函数策略。
//==============================================================================================
#ifndef GRAPH_FUNCS
#define GRAPH_FUNCS
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Name:   HandyGraphFunctions.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   As the name implies, some useful functions you can use with your
//          graphs. 

//          For the function templates, make sure your graph interface complies
//          with the SparseGraph class
//-----------------------------------------------------------------------------
#include <iostream>

#include "misc/Cgdi.h"
#include "misc/utils.h"
#include "misc/Stream_Utility_Functions.h"
#include "Graph/GraphAlgorithms.h"
#include "Graph/AStarHeuristicPolicies.h"





//--------------------------- ValidNeighbour -----------------------------
//
//  returns true if x,y is a valid position in the map
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// ValidNeighbour:判断网格坐标 (x,y) 是否在 [0,NumCellsX)×[0,NumCellsY) 范围内。
//   !((x<0)||...) —— 逻辑或 || 连接四种越界可能,整体取反 = 在界内返回 true。
//--------------------------------------------------------------------------------
bool ValidNeighbour(int x, int y, int NumCellsX, int NumCellsY)
{
  return !((x < 0) || (x >= NumCellsX) || (y < 0) || (y >= NumCellsY));
}
  
//------------ GraphHelper_AddAllNeighboursToGridNode ------------------
//
//  use to add he eight neighboring edges of a graph node that 
//  is positioned in a grid layout
//------------------------------------------------------------------------
template <class graph_type>
//--------------------------------------------------------------------------------
// GraphHelper_AddAllNeighboursToGridNode:给网格里的某个节点(row,col)连上周围 8 个格子。
//   双重 for(i=-1..1, j=-1..1) 遍历 3×3 邻域;(i==0 && j==0) 跳过自己;
//   对每个合法邻居:算两点距离 dist → 建边(row*NumCellsX+col → nodeY*NumCellsX+nodeX);
//   节点编号算法:二维数组 [row][col] 展平成一维 = row*NumCellsX + col。
//   若不是有向图(!graph.isDigraph()),还要再加一条反方向的边(无向=双向)。
//--------------------------------------------------------------------------------
void GraphHelper_AddAllNeighboursToGridNode(graph_type& graph,
                                            int         row,
                                            int         col,
                                            int         NumCellsX,
                                            int         NumCellsY)
{   
  for (int i=-1; i<2; ++i)
  {
    for (int j=-1; j<2; ++j)
    {
      int nodeX = col+j;
      int nodeY = row+i;

      //skip if equal to this node
      if ( (i == 0) && (j==0) ) continue;

      //check to see if this is a valid neighbour
      if (ValidNeighbour(nodeX, nodeY, NumCellsX, NumCellsY))
      {
        //calculate the distance to this node
        Vector2D PosNode      = graph.GetNode(row*NumCellsX+col).Pos();
        Vector2D PosNeighbour = graph.GetNode(nodeY*NumCellsX+nodeX).Pos();

        double dist = PosNode.Distance(PosNeighbour);

        //this neighbour is okay so it can be added
        graph_type::EdgeType NewEdge(row*NumCellsX+col,
                                     nodeY*NumCellsX+nodeX,
                                     dist);
        graph.AddEdge(NewEdge);

        //if graph is not a diagraph then an edge needs to be added going
        //in the other direction
        if (!graph.isDigraph())
        {
          graph_type::EdgeType NewEdge(nodeY*NumCellsX+nodeX,
                                       row*NumCellsX+col,
                                       dist);
          graph.AddEdge(NewEdge);
        }
      }
    }
  }
}


//--------------------------- GraphHelper_CreateGrid --------------------------
//
//  creates a graph based on a grid layout. This function requires the 
//  dimensions of the environment and the number of cells required horizontally
//  and vertically 
//-----------------------------------------------------------------------------
template <class graph_type>
//--------------------------------------------------------------------------------
// GraphHelper_CreateGrid:按环境尺寸 (cySize×cxSize) 和格子数 (NumCellsY×NumCellsX)
//   生成一张完整的网格图。步骤:① 算每格宽高;② 双重循环 AddNode 铺所有节点;
//   ③ 逐格调用上面的 AddAllNeighboursToGridNode 连边。
//   NavGraphNode<>(...) 里的 <> 空模板尖括号:用缺省模板参数(void*)实例化导航节点。
//--------------------------------------------------------------------------------
void GraphHelper_CreateGrid(graph_type& graph,
                             int cySize,
                             int cxSize,
                             int NumCellsY,
                             int NumCellsX)
{ 
  //need some temporaries to help calculate each node center
  double CellWidth  = (double)cySize / (double)NumCellsX;
  double CellHeight = (double)cxSize / (double)NumCellsY;

  double midX = CellWidth/2;
  double midY = CellHeight/2;

  
  //first create all the nodes
  for (int row=0; row<NumCellsY; ++row)
  {
    for (int col=0; col<NumCellsX; ++col)
    {
      graph.AddNode(NavGraphNode<>(graph.GetNextFreeNodeIndex(),
                                   Vector2D(midX + (col*CellWidth),
                                   midY + (row*CellHeight))));

    }
  }
  //now to calculate the edges. (A position in a 2d array [x][y] is the
  //same as [y*NumCellsX + x] in a 1d array). Each cell has up to eight
  //neighbours.
  for (int row=0; row<NumCellsY; ++row)
  {
    for (int col=0; col<NumCellsX; ++col)
    {
      GraphHelper_AddAllNeighboursToGridNode(graph, row, col, NumCellsX, NumCellsY);
    }
  }
}  


//--------------------------- GraphHelper_DrawUsingGDI ------------------------
//
//  draws a graph using the GDI
//-----------------------------------------------------------------------------
template <class graph_type>
//--------------------------------------------------------------------------------
// GraphHelper_DrawUsingGDI:用 GDI 把图调试画出来。
//   遍历所有节点:在节点位置画小圆;若 DrawNodeIDs 为真,再用灰字印出节点编号;
//   再遍历该节点的所有边,从节点画直线到边的终点。
//   ConstNodeIterator / ConstEdgeIterator 是图提供的"只读迭代器",
//   begin() 取起点、end() 判断是否遍历完、next() 走到下一个(避免直接暴露内部容器)。
//--------------------------------------------------------------------------------
void GraphHelper_DrawUsingGDI(const graph_type& graph, int color, bool DrawNodeIDs = false)
{	

  //just return if the graph has no nodes
  if (graph.NumNodes() == 0) return;
  
  gdi->SetPenColor(color);

  //draw the nodes 
  graph_type::ConstNodeIterator NodeItr(graph);
  for (const graph_type::NodeType* pN=NodeItr.begin();
      !NodeItr.end();
       pN=NodeItr.next())
  {
    gdi->Circle(pN->Pos(), 2);

    if (DrawNodeIDs)
    {
      gdi->TextColor(200,200,200);
      gdi->TextAtPos((int)pN->Pos().x+5, (int)pN->Pos().y-5, ttos(pN->Index()));
    }

    graph_type::ConstEdgeIterator EdgeItr(graph, pN->Index());
    for (const graph_type::EdgeType* pE=EdgeItr.begin();
        !EdgeItr.end();
        pE=EdgeItr.next())
    {
      gdi->Line(pN->Pos(), graph.GetNode(pE->To()).Pos());
    }
  }
}


//--------------------------- WeightNavGraphNodeEdges -------------------------
//
//  Given a cost value and an index to a valid node this function examines 
//  all a node's edges, calculates their length, and multiplies
//  the value with the weight. Useful for setting terrain costs.
//------------------------------------------------------------------------
template <class graph_type>
//--------------------------------------------------------------------------------
// WeightNavGraphNodeEdges:把某节点所有边的代价 × 权重 weight。
//   典型用途:让"沼泽"这类地形上的边代价变大,AI 就会绕路走。
//   assert(节点<总数) 调试期防越界;非有向图时反向边也同步加权。
//--------------------------------------------------------------------------------
void WeightNavGraphNodeEdges(graph_type& graph, int node, double weight)
{
  //make sure the node is present
  assert(node < graph.NumNodes());

  //set the cost for each edge
  graph_type::ConstEdgeIterator ConstEdgeItr(graph, node);
  for (const graph_type::EdgeType* pE=ConstEdgeItr.begin();
       !ConstEdgeItr.end();
       pE=ConstEdgeItr.next())
  {
    //calculate the distance between nodes
    double dist = Vec2DDistance(graph.GetNode(pE->From()).Pos(),
                               graph.GetNode(pE->To()).Pos());

    //set the cost of this edge
    graph.SetEdgeCost(pE->From(), pE->To(), dist * weight);

    //if not a digraph, set the cost of the parallel edge to be the same
    if (!graph.isDigraph())
    {      
      graph.SetEdgeCost(pE->To(), pE->From(), dist * weight);
    }
  }
}


//----------------------- CreateAllPairsTable ---------------------------------
//
// creates a lookup table encoding the shortest path info between each node
// in a graph to every other
//-----------------------------------------------------------------------------
template <class graph_type>
//--------------------------------------------------------------------------------
// CreateAllPairsTable:构建"全对最短路径表"——一张二维表 ShortestPaths[src][dst],
//   存"从 src 出发走到 dst,下一步该往哪个节点走"。
//   做法:对每个 source 跑一次 Dijkstra 得到最短路径树(SPT),再从每个 target 沿着
//   SPT 反向回溯出"下一步"。建一次表,之后任意两点寻路 O(1) 查表。
//   vector<vector<int> > 是"动态数组的动态数组"(标准库二维表)。
//--------------------------------------------------------------------------------
std::vector<std::vector<int> > CreateAllPairsTable(const graph_type& G)
{
  enum {no_path = -1};
  
  std::vector<int> row(G.NumNodes(), no_path);
  
  std::vector<std::vector<int> > ShortestPaths(G.NumNodes(), row);

  for (int source=0; source<G.NumNodes(); ++source)
  {
    //calculate the SPT for this node
    Graph_SearchDijkstra<graph_type> search(G, source);

    std::vector<const graph_type::EdgeType*> spt = search.GetSPT();

    //now we have the SPT it's easy to work backwards through it to find
    //the shortest paths from each node to this source node
    for (int target = 0; target<G.NumNodes(); ++target)
    {
      //if the source node is the same as the target just set to target
      if (source == target)
      {
        ShortestPaths[source][target] = target;
      }

      else
      {
        int nd = target;

        while ((nd != source) && (spt[nd] != 0))
        {
          ShortestPaths[spt[nd]->From][target]= nd;

          nd = spt[nd]->From;
        }
      }
    }//next target node
  }//next source node

  return ShortestPaths;
}


//----------------------- CreateAllPairsCostsTable -------------------------------
//
//  creates a lookup table of the cost associated from traveling from one
//  node to every other
//-----------------------------------------------------------------------------
template <class graph_type>
//--------------------------------------------------------------------------------
// CreateAllPairsCostsTable:与上一个类似,但存的是"最短代价"而不是"下一步走哪"。
//--------------------------------------------------------------------------------
std::vector<std::vector<double> > CreateAllPairsCostsTable(const graph_type& G)
{
  //create a two dimensional vector
  std::vector<double> row(G.NumNodes(), 0.0);
  std::vector<std::vector<double> > PathCosts(G.NumNodes(), row);

  for (int source=0; source<G.NumNodes(); ++source)
  {
    //do the search
    Graph_SearchDijkstra<graph_type> search(G, source);

    //iterate through every node in the graph and grab the cost to travel to
    //that node
    for (int target = 0; target<G.NumNodes(); ++target)
    {
      if (source != target)
      {
        PathCosts[source][target]= search.GetCostToNode(target);
      }
    }//next target node
    
  }//next source node

  return PathCosts;
}

//---------------------- CalculateAverageGraphEdgeLength ----------------------
//
//  determines the average length of the edges in a navgraph (using the 
//  distance between the source & target node positions (not the cost of the 
//  edge as represented in the graph, which may account for all sorts of 
//  other factors such as terrain type, gradients etc)
//------------------------------------------------------------------------------
template <class graph_type>
//--------------------------------------------------------------------------------
// CalculateAverageGraphEdgeLength:遍历所有边,累加真实几何距离(不是边里存的 cost),
//   最后除以边数,得到平均边长——用来评估图的疏密程度。
//--------------------------------------------------------------------------------
double CalculateAverageGraphEdgeLength(const graph_type& G)
{
  double TotalLength = 0;
  int NumEdgesCounted = 0;

  graph_type::ConstNodeIterator NodeItr(G);
  const graph_type::NodeType* pN;
  for (pN = NodeItr.begin(); !NodeItr.end(); pN=NodeItr.next())
  {
    graph_type::ConstEdgeIterator EdgeItr(G, pN->Index());
    for (const graph_type::EdgeType* pE = EdgeItr.begin(); !EdgeItr.end(); pE=EdgeItr.next())
    {
      //increment edge counter
      ++NumEdgesCounted;

      //add length of edge to total length
      TotalLength += Vec2DDistance(G.GetNode(pE->From()).Pos(), G.GetNode(pE->To()).Pos());
    }
  }

  return TotalLength / (double)NumEdgesCounted;
}

//----------------------------- GetCostliestGraphEdge -------------------
//
//  returns the cost of the costliest edge in the graph
//-----------------------------------------------------------------------------
template <class graph_type>
//--------------------------------------------------------------------------------
// GetCostliestGraphEdge:遍历所有边,返回代价最大的那条边的 cost。
//   greatest 初值取 MinDouble(double 最小值),保证任何真实代价都比它大。
//--------------------------------------------------------------------------------
double GetCostliestGraphEdge(const graph_type& G)
{
  double greatest = MinDouble;

  graph_type::ConstNodeIterator NodeItr(G);
  const graph_type::NodeType* pN;
  for (pN = NodeItr.begin(); !NodeItr.end(); pN=NodeItr.next())
  {
    graph_type::ConstEdgeIterator EdgeItr(G, pN->Index());
    for (const graph_type::EdgeType* pE = EdgeItr.begin(); !EdgeItr.end(); pE=EdgeItr.next())
    {
      if (pE->Cost() > greatest)greatest = pE->Cost();
    }
  }

  return greatest;
}

#endif
