//==============================================================================================
//【文件说明】Pathfinder.cpp —— Pathfinder 类的全部函数实现
//
//【这个文件是干什么的?】
//  Pathfinder.h 只"声明"了类有哪些函数(签名);本文件把这些函数"实现"出来:
//  建网格图、把鼠标坐标换算成格子、涂地形、调四种搜索算法(DFS/BFS/Dijkstra/A*)、
//  存档读档、把一切画到屏幕。第 5 章的核心算法思想在这里落地调用。
//
//【谁调用本文件?】本文件是被 main.cpp 通过 Pathfinder.h 间接使用的实现单元。
//
//【C++ 小课堂:声明(.h)与实现(.cpp)分离】
//  头文件写"有什么",cpp 写"怎么做"。每个实现函数名都带 类名:: 前缀,
//  如 Pathfinder::CreateGraph —— :: 是作用域解析符,表示"这个函数属于 Pathfinder 类"。
//==============================================================================================
// ---- 本文件用到的头文件 ----
//  Pathfinder.h               —— 自己类的声明(必带);
//  Graph/HandyGraphFunctions.h —— 建网格图/加邻居边等图工具函数;
//  misc/Cgdi.h                —— 全局绘图对象 gdi;Time/PrecisionTimer.h —— 高精度计时;
//  constants.h                —— 窗口尺寸常数;graph/AStarHeuristicPolicies.h —— A* 的启发式;
//  misc/Stream_Utility_Functions.h —— 数字转字符串等小工具。
#include "Pathfinder.h"
#include "Graph/HandyGraphFunctions.h"
#include "misc/Cgdi.h"
#include "Time/PrecisionTimer.h"
#include "constants.h"
#include "graph/AStarHeuristicPolicies.h"
#include "misc/Stream_Utility_Functions.h"


// 标准库输入输出(Save/Load 文件流用到)。using namespace std; = 后面可直接写
// ofstream/ifstream/string,而不用每次写 std:: 前缀。
#include <iostream>
using namespace std;

// ---- extern 声明:这几个全局变量定义在 main.cpp 里,本文件只是"引用"它们 ----
// extern = 告诉编译器"变量在别处已定义,这里直接用"。g_hwndToolbar 是工具栏窗口句柄。
extern HWND g_hwndToolbar;
extern const char*  g_szApplicationName;
extern const char*	g_szWindowClassName;

//----------------------- CreateGraph ------------------------------------
//
//------------------------------------------------------------------------
//-------------------------- CreateGraph -----------------------------------
// 建图:按纵向 CellsUp 格、横向 CellsAcross 格造出整张导航网络图。
// 步骤:量客户区大小 → 算每格像素宽高 → new 一张空图 → GraphHelper_CreateGrid
// 把节点和相邻边铺成网格 → 把起点放在底部中间、终点放在顶部中间。
void Pathfinder::CreateGraph(int CellsUp,
                              int CellsAcross)
{
  //get the height of the toolbar
  RECT rectToolbar;
  GetWindowRect(g_hwndToolbar, &rectToolbar);
  
  //get the dimensions of the client area
  HWND hwndMainWindow = FindWindow(g_szWindowClassName, g_szApplicationName); 

  RECT rect;
  GetClientRect(hwndMainWindow, &rect);
  m_icxClient = rect.right;
  m_icyClient = rect.bottom - abs(rectToolbar.bottom - rectToolbar.top) - InfoWindowHeight;

  //initialize the terrain vector with normal terrain
  m_TerrainType.assign(CellsUp * CellsAcross, normal);

  m_iCellsX     = CellsAcross;
  m_iCellsY     = CellsUp;
  m_dCellWidth  = (double)m_icxClient / (double)CellsAcross;
  m_dCellHeight = (double)m_icyClient / (double)CellsUp;

  //delete any old graph
  delete m_pGraph;

  //create the graph
  m_pGraph = new NavGraph(false);//not a digraph

  GraphHelper_CreateGrid(*m_pGraph, m_icxClient, m_icyClient, CellsUp, CellsAcross);

  //initialize source and target indexes to mid top and bottom of grid 
  PointToIndex(VectorToPOINTS(Vector2D(m_icxClient/2, m_dCellHeight*2)), m_iTargetCell);
  PointToIndex(VectorToPOINTS(Vector2D(m_icxClient/2, m_icyClient -m_dCellHeight*2)), m_iSourceCell);

  m_Path.clear();
  m_SubTree.clear();

  m_CurrentAlgorithm = non;
  m_dTimeTaken = 0;
}

//--------------------- PointToIndex -------------------------------------
//
//  converts a POINTS into an index into the graph
//------------------------------------------------------------------------
//-------------------------- PointToIndex ----------------------------------
// 把鼠标像素坐标 p 换算成格子编号:横向格子号 x = 像素x ÷ 格宽,纵向 y = 像素y ÷ 格高,
// 再按 "行×总列数 + 列" 拼成一维编号 NodeIndex。越界返回 false。
bool Pathfinder::PointToIndex(POINTS p, int& NodeIndex)
{
  //convert p to an index into the graph
  int x = (int)((double)(p.x)/m_dCellWidth);  
  int y = (int)((double)(p.y)/m_dCellHeight); 
  
  //make sure the values are legal
  if ( (x>m_iCellsX) || (y>m_iCellsY) )
  {
    NodeIndex = -1;

    return false;
  }

  NodeIndex = y*m_iCellsX+x;

  return true;
}

//----------------- GetTerrainCost ---------------------------------------
//
//  returns the cost of the terrain represented by the current brush type
//------------------------------------------------------------------------
//-------------------------- GetTerrainCost --------------------------------
// 返回某种地形的"每步移动代价":平地 1.0、泥 1.5、水 2.0(越难走越贵);
// 障碍等其它情况返回 MaxDouble(几乎无穷大)= 等于禁止通过。
double Pathfinder::GetTerrainCost(const brush_type brush)
{
  const double cost_normal = 1.0;
  const double cost_water  = 2.0;
  const double cost_mud    = 1.5;

  switch (brush)
  {
    case normal: return cost_normal;
    case water:  return cost_water;
    case mud:    return cost_mud;
    default:     return MaxDouble;
  };
}
  
//----------------------- PaintTerrain -----------------------------------
//
//  this either changes the terrain at position p to whatever the current
//  terrain brush is set to, or it adjusts the source/target cell
//------------------------------------------------------------------------
//-------------------------- PaintTerrain ----------------------------------
// 鼠标在某格按下/拖动时调用:若是"起点/终点笔刷",就改 m_iSourceCell/m_iTargetCell;
// 否则把该格涂成当前地形笔刷;最后调 UpdateAlgorithm 用当前算法重算一遍路径。
void Pathfinder::PaintTerrain(POINTS p)
{
  //convert p to an index into the graph
  int x = (int)((double)(p.x)/m_dCellWidth);  
  int y = (int)((double)(p.y)/m_dCellHeight); 
  
  //make sure the values are legal
  if ( (x>m_iCellsX) || (y>(m_iCellsY-1)) ) return;

  //reset path and tree records
  m_SubTree.clear();
  m_Path.clear();

  //if the current terrain brush is set to either source or target we
  //should change the appropriate node
  if ( (m_CurrentTerrainBrush == source) || (m_CurrentTerrainBrush == target) )
  {
    switch (m_CurrentTerrainBrush)
    {
    case source:

      m_iSourceCell = y*m_iCellsX+x; break;

    case target:

      m_iTargetCell = y*m_iCellsX+x; break;
      
    }//end switch
  }

  //otherwise, change the terrain at the current mouse position
  else
  {
    UpdateGraphFromBrush(m_CurrentTerrainBrush, y*m_iCellsX+x);
  }

  //update any currently selected algorithm
  UpdateAlgorithm();
}

//--------------------------- UpdateGraphFromBrush ----------------------------
//
//  given a brush and a node index, this method updates the graph appropriately
//  (by removing/adding nodes or changing the costs of the node's edges)
//-----------------------------------------------------------------------------
//----------------------- UpdateGraphFromBrush ------------------------------
// 把某个格子的地形改动真正写进图:若是障碍(1)就 RemoveNode 把节点从图中删掉;
// 否则若节点之前被删过就先 AddNode 恢复,再用 WeightNavGraphNodeEdges 把它的
// 各条边代价设成该地形的 GetTerrainCost —— 代价变了,A* 才会绕开贵的路。
void Pathfinder::UpdateGraphFromBrush(int brush, int CellIndex)
{
  //set the terrain type in the terrain index
  m_TerrainType[CellIndex] = brush;

  //if current brush is an obstacle then this node must be removed
  //from the graph
  if (brush == 1)
  {
    m_pGraph->RemoveNode(CellIndex);
  }

  else
  {
    //make the node active again if it is currently inactive
    if (!m_pGraph->isNodePresent(CellIndex))
    {
      int y = CellIndex / m_iCellsY;
      int x = CellIndex - (y*m_iCellsY);

      m_pGraph->AddNode(NavGraph::NodeType(CellIndex, Vector2D(x*m_dCellWidth + m_dCellWidth/2.0,
                                                               y*m_dCellHeight+m_dCellHeight/2.0)));

      GraphHelper_AddAllNeighboursToGridNode(*m_pGraph, y, x, m_iCellsX, m_iCellsY);
    }

    //set the edge costs in the graph
    WeightNavGraphNodeEdges(*m_pGraph, CellIndex, GetTerrainCost((brush_type)brush));                            
  }
}

//--------------------------- UpdateAlgorithm ---------------------------------
//-------------------------- UpdateAlgorithm --------------------------------
// 地形改了之后,按 m_CurrentAlgorithm 重新跑一遍对应的搜索(地形一变旧路径就失效)。
void Pathfinder::UpdateAlgorithm()
{
  //update any current algorithm
  switch(m_CurrentAlgorithm)
  {
  case non:

    break;

  case search_dfs:

    CreatePathDFS(); break;

  case search_bfs:
    
    CreatePathBFS(); break;

  case search_dijkstra:

    CreatePathDijkstra(); break;

  case search_astar:
    
    CreatePathAStar(); break;

  default: break;
  }
}

//------------------------- CreatePathDFS --------------------------------
//
//  uses DFS to find a path between the start and target cells.
//  Stores the path as a series of node indexes in m_Path.
//------------------------------------------------------------------------
//==========================================================================
// 下面四个函数是四种搜索算法的入口。它们套路几乎一样:
//   ① 记下当前算法;② 开秒表 PrecisionTimer;③ new 一个算法对象,传入图+起点+终点;
//   ④ 停表记录耗时;⑤ 若搜到(Found())就把路径取到 m_Path、搜索树取到 m_SubTree。
// 区别只在算法对象类型与是否有"代价"概念。
void Pathfinder::CreatePathDFS()
{
  //set current algorithm
  m_CurrentAlgorithm = search_dfs;

  //clear any existing path
  m_Path.clear();
  m_SubTree.clear();

  //create and start a timer
  PrecisionTimer timer; timer.Start();

  //do the search
  Graph_SearchDFS<NavGraph> DFS(*m_pGraph, m_iSourceCell, m_iTargetCell);

  //record the time taken  
  m_dTimeTaken = timer.TimeElapsed();

  //now grab the path (if one has been found)
  if (DFS.Found())
  {
    m_Path = DFS.GetPathToTarget();
  }

  m_SubTree = DFS.GetSearchTree();

  m_dCostToTarget = 0.0;
}


//------------------------- CreatePathBFS --------------------------------
//
//  uses BFS to find a path between the start and target cells.
//  Stores the path as a series of node indexes in m_Path.
//------------------------------------------------------------------------
//---- CreatePathDFS / CreatePathBFS ----
// 深度优先(DFS):一条路走到黑再回头;广度优先(BFS):一圈圈向外扩散。
// 二者都不考虑地形代价,只回答"能不能到、走哪条",所以 m_dCostToTarget 记 0。
void Pathfinder::CreatePathBFS()
{
  //set current algorithm
  m_CurrentAlgorithm = search_bfs;

  //clear any existing path
  m_Path.clear();
  m_SubTree.clear();

  //create and start a timer
  PrecisionTimer timer; timer.Start();

  //do the search
  Graph_SearchBFS<NavGraph> BFS(*m_pGraph, m_iSourceCell, m_iTargetCell);

    //record the time taken  
  m_dTimeTaken = timer.TimeElapsed();

  //now grab the path (if one has been found)
  if (BFS.Found())
  {
    m_Path = BFS.GetPathToTarget();
  }

  m_SubTree = BFS.GetSearchTree();

  m_dCostToTarget = 0.0;
}

//-------------------------- CreatePathDijkstra --------------------------
//
//  creates a path from m_iSourceCell to m_iTargetCell using Dijkstra's algorithm
//------------------------------------------------------------------------
//---- CreatePathDijkstra ----
// Dijkstra:按代价从小到大扩展节点,能得到真正的最短(最便宜)路径,但它"不看方向"——
// 不管终点在东边还是西边,都把整个图均匀地搜一遍。m_SubTree 取它的最短路径树 SPT。
void Pathfinder::CreatePathDijkstra()
{
  //set current algorithm
  m_CurrentAlgorithm = search_dijkstra;

  //create and start a timer
  PrecisionTimer timer; timer.Start();
    
  Graph_SearchDijkstra<NavGraph> djk(*m_pGraph, m_iSourceCell, m_iTargetCell);

  //record the time taken  
  m_dTimeTaken = timer.TimeElapsed();

  m_Path = djk.GetPathToTarget();

  m_SubTree = djk.GetSPT();

  m_dCostToTarget = djk.GetCostToTarget();
}

//--------------------------- CreatePathAStar ---------------------------
//------------------------------------------------------------------------
//---- CreatePathAStar(A* ★ 第 5 章核心)----
// A* = Dijkstra + "启发式估计 h"。每个待扩展节点有两个分:
//   g = 已经从起点走到它的真实代价(过海/泥会累加);
//   h = 它到终点的"直线距离估计"(Heuristic_Euclid = 欧氏直线距离);
//   f = g + h,A* 每次优先扩展 f 最小的节点,既保证最短,又尽量朝终点方向走,
//   因此比 Dijkstra 快得多。开放列表/关闭列表、父节点回溯都封装在 Graph_SearchAStar 里。
void Pathfinder::CreatePathAStar()
{
  //set current algorithm
  m_CurrentAlgorithm = search_astar;
      
  //create and start a timer
  PrecisionTimer timer; timer.Start();
  
  //create a couple of typedefs so the code will sit comfortably on the page   
  typedef Graph_SearchAStar<NavGraph, Heuristic_Euclid> AStarSearch;

  //create an instance of the A* search using the Euclidean heuristic
  AStarSearch AStar(*m_pGraph, m_iSourceCell, m_iTargetCell);
  

  //record the time taken  
  m_dTimeTaken = timer.TimeElapsed();

  m_Path = AStar.GetPathToTarget();

  m_SubTree = AStar.GetSPT();

  m_dCostToTarget = AStar.GetCostToTarget();

}

//---------------------------Load n save methods ------------------------------
//-----------------------------------------------------------------------------
//---- Save / Load:把地图(格子尺寸 + 每格地形)读写成 .map 文本文件 ----
void Pathfinder::Save( char* FileName)
{
  ofstream save(FileName);
  assert (save && "Pathfinder::Save< bad file >");

  //save the size of the grid
  save << m_iCellsX << endl;
  save << m_iCellsY << endl;

  //save the terrain
  for (unsigned int t=0; t<m_TerrainType.size(); ++t)
  {
    if (t==m_iSourceCell)
    {
      save << source << endl;
    }
    else if (t==m_iTargetCell)
    {
      save << target << endl;
    }
    else
    {
      save << m_TerrainType[t] << endl;
    }
  }
}

//-------------------------------- Load ---------------------------------------
//-----------------------------------------------------------------------------
void Pathfinder::Load( char* FileName)
{
  ifstream load(FileName);
  assert (load && "Pathfinder::Save< bad file >");

  //load the size of the grid
  load >> m_iCellsX;
  load >> m_iCellsY;

  //create a graph of the correct size
  CreateGraph(m_iCellsY, m_iCellsX);

  int terrain;

  //save the terrain
  for (int t=0; t<m_iCellsX*m_iCellsY; ++t)
  {
    load >> terrain;
    
    if (terrain == source)
    {
      m_iSourceCell = t;
    }

    else if (terrain == target)
    {
      m_iTargetCell = t;
    }

    else
    {
      m_TerrainType[t] = terrain;

      UpdateGraphFromBrush(terrain, t);
    }
  }
}

//------------------------ GetNameOfCurrentSearchAlgorithm --------------------
//-----------------------------------------------------------------------------
// 返回当前算法的英文名,画在窗口底部(如 "A Star")。
std::string Pathfinder::GetNameOfCurrentSearchAlgorithm()const
{
  switch(m_CurrentAlgorithm)
  {
  case non: return "";
  case search_astar: return "A Star";
  case search_bfs: return "Breadth First";
  case search_dfs: return "Depth First";
  case search_dijkstra: return "Dijkstras";
  default: return "UNKNOWN!";
  }
}

//---------------------------- Render ------------------------------------
//
//------------------------------------------------------------------------
//-------------------------- Render ----------------------------------------
// 把整个场景画到屏幕:逐个格子按地形填色(白=平地/黑=障碍/浅蓝=水/棕=泥),
// 终点画红叉、起点画绿框;若开了网络图就画灰线;搜索树画红线;最终路径画粗蓝线;
// 底部再写一行"本次耗时"和(A*/Dijkstra)"总代价"。gdi 是全局绘图对象。
void Pathfinder::Render()
{
  gdi->TransparentText();
  
  //render all the cells
  for (int nd=0; nd<m_pGraph->NumNodes(); ++nd)
  {
    int left   = (int)(m_pGraph->GetNode(nd).Pos().x - m_dCellWidth/2.0);
    int top    = (int)(m_pGraph->GetNode(nd).Pos().y - m_dCellHeight/2.0);
    int right  = (int)(1+m_pGraph->GetNode(nd).Pos().x + m_dCellWidth/2.0);
    int bottom = (int)(1+m_pGraph->GetNode(nd).Pos().y + m_dCellHeight/2.0);

    gdi->GreyPen();

    switch (m_TerrainType[nd])
    {
    case 0:
      gdi->WhiteBrush();
      if (!m_bShowTiles)gdi->WhitePen();
      break;

    case 1:
      gdi->BlackBrush();
      if (!m_bShowTiles)gdi->BlackPen();
      break;
      
    case 2:
      gdi->LightBlueBrush();
      if (!m_bShowTiles)gdi->LightBluePen();
      break;
      
    case 3:
      gdi->BrownBrush();
      if (!m_bShowTiles)gdi->BrownPen();
      break;

    default:
      gdi->WhiteBrush();
      if (!m_bShowTiles)gdi->WhitePen();
      break;
      
    }//end switch


    if (nd == m_iTargetCell)
    {
      gdi->RedBrush();
      if (!m_bShowTiles)gdi->RedPen();
    }

    if (nd == m_iSourceCell)
    {
      gdi->GreenBrush();
      if (!m_bShowTiles)gdi->GreenPen();
    }
   
    gdi->Rect(left, top, right, bottom);  

    if (nd == m_iTargetCell)
    {
      gdi->ThickBlackPen();
      gdi->Cross(Vector2D(m_pGraph->GetNode(nd).Pos().x-1, m_pGraph->GetNode(nd).Pos().y-1),
                (int)((m_dCellWidth*0.6)/2.0));
    }

    if (nd == m_iSourceCell)
    {
      gdi->ThickBlackPen();
      gdi->HollowBrush();
      gdi->Rect(left+7,top+7,right-6,bottom-6);
    }

    //render dots at the corners of the cells
    gdi->DrawDot(left, top, RGB(0,0,0));
    gdi->DrawDot(right-1, top, RGB(0,0,0));
    gdi->DrawDot(left, bottom-1, RGB(0,0,0));
    gdi->DrawDot(right-1, bottom-1, RGB(0,0,0));
  }  
  //draw the graph nodes and edges if rqd
  if (m_bShowGraph)
  {
    GraphHelper_DrawUsingGDI<NavGraph>(*m_pGraph, Cgdi::light_grey, false);  //false = don't draw node IDs
  }

  //draw any tree retrieved from the algorithms
  gdi->RedPen();

  for (unsigned int e=0; e<m_SubTree.size(); ++e)
  {   
    if (m_SubTree[e])
    {
      Vector2D from = m_pGraph->GetNode(m_SubTree[e]->From()).Pos();
      Vector2D to   = m_pGraph->GetNode(m_SubTree[e]->To()).Pos();

      gdi->Line(from, to);
    }
  }

  //draw the path (if any)  
  if (m_Path.size() > 0)
  {
    gdi->ThickBluePen();

    std::list<int>::iterator it = m_Path.begin();
    std::list<int>::iterator nxt = it; ++nxt;

    for (it; nxt != m_Path.end(); ++it, ++nxt)
    {
      gdi->Line(m_pGraph->GetNode(*it).Pos(), m_pGraph->GetNode(*nxt).Pos());
    }
  }
  
  if (m_dTimeTaken)
  {
    //draw time taken to complete algorithm
    string time = ttos(m_dTimeTaken, 8);
    string s = "Time Elapsed for " + GetNameOfCurrentSearchAlgorithm() + " is " + time;
    gdi->TextAtPos(1,m_icyClient + 3,s); 
  }

  //display the total path cost if appropriate
  if (m_CurrentAlgorithm == search_astar || m_CurrentAlgorithm == search_dijkstra)
  {
    gdi->TextAtPos(m_icxClient-110, m_icyClient + 3, "Cost is " + ttos(m_dCostToTarget));
  }
}
