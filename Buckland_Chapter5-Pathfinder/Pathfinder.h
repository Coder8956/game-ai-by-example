//==============================================================================================
//【文件说明】Pathfinder.h —— 第 5 章核心类"寻路器"的声明
//
//【这个文件是干什么的?】
//  本工程是一个"网格寻路"演示:一块 19x19 的格子地,你可以用鼠标涂出障碍、
//  水、泥等地形,再指定起点(绿)和终点(红),然后点工具栏按钮让程序自动算出
//  一条从起点到终点的最短(或某算法找到的)路径并画出来。
//  本文件定义的 Pathfinder 类就是这套逻辑的"大脑":它持有一张网络图(graph)、
//  当前地形、当前选的搜索算法,并提供建图、涂地形、跑算法、渲染、存档读档等功能。
//
//【谁在使用这个文件?】
//  main.cpp        —— new 出一个 Pathfinder 对象 g_Pathfinder,响应工具栏/菜单按钮,
//                      调用 CreateGraph、PaintTerrain、CreatePathAStar、Render 等;
//  Pathfinder.cpp  —— 本类所有成员函数的具体实现(声明在这里,实现在那里)。
//
//【本文件包含了谁?】
//  <windows.h>/<vector>/<fstream>/<string>/<list> —— 操作系统与 C++ 标准库;
//  2D/Vector2d.h            —— 2D 向量坐标类;
//  Graph/SparseGraph.h      —-- Common 库里的"稀疏图"类(寻路的核心数据结构);
//  Graph/GraphAlgorithms.h  —— Common 库里现成的 DFS/BFS/Dijkstra/A* 算法实现;
//  Graph/GraphEdgeTypes.h / GraphNodeTypes.h —— 图的边、节点类型;
//  misc/utils.h             —— 常用工具函数与常量(MaxDouble 等)。
//  (以上 Graph/misc 头文件都在公共目录 Common\ 下,靠工程附加包含目录找到。)
//==============================================================================================
//--------------------------------------------------------------------------------
//【包含保护】#ifndef Pathfinder_H / #define Pathfinder_H / #endif:防止本头文件
// 被重复包含两次而报"重复定义"。(写法详见 WestWorld1/Locations.h 注释)
//--------------------------------------------------------------------------------
#ifndef Pathfinder_H
#define Pathfinder_H
#pragma warning (disable:4786)
//(原文无注释)#pragma warning(disable:4786):让编译器对"调试信息名太长"的第 4786 号
// 警告闭嘴。这是老 STL(模板类)在旧编译器上常报的无害警告,作者在这里关掉它。
//------------------------------------------------------------------------
//
//  Name:   Pathfinder.h
//
//  Desc:   class enabling users to create simple environments consisting
//          of different terrain types and then to use various search algorithms
//          to find paths through them
//
//  Author: Mat Buckland  (fup@ai-junkie.com)
// ↓↓↓ 上面作者信息块的翻译:
//   Name : Pathfinder.h(文件名);
//   Desc : 这个类让用户创建由不同地形组成的简单环境,再用各种搜索算法在其中找路径;
//   Author: Mat Buckland 2003(本书作者邮箱 fup@ai-junkie.com)。
//
//------------------------------------------------------------------------
// ---- 第一批:操作系统与 C++ 标准库头文件(尖括号表示去系统目录找)----
//  windows.h —— Windows API 大全(窗口、消息、绘图句柄等);
//  vector/list —— 动态数组/链表容器(后面 m_TerrainType、m_Path 用它们);
//  fstream     —— 文件读写(存档 Save/读档 Load 用);string —— 字符串。
#include <windows.h>
#include <vector>
#include <fstream>
#include <string>
#include <list>


// ---- 第二批:本工程与公共库 Common\ 的头文件(双引号表示先在本工程/附加目录找)----
//  这里引入的 SparseGraph(稀疏图)和 GraphAlgorithms(图算法)是第 5 章寻路的
//  真正主角:它们在 Common\Graph\ 下,本书把图与搜索算法写成了可复用的模板。
#include "2D/Vector2d.h"
#include "Graph/SparseGraph.h"
#include "Graph/GraphAlgorithms.h"
#include "misc/utils.h"
#include "graph/GraphEdgeTypes.h"
#include "graph/GraphNodeTypes.h"



//==================================================================================
// class Pathfinder —— 开始定义"寻路器"类。花括号内分两大区:
//   public  (公有)—— 对外按钮,main.cpp 直接调用;
//   private (私有)—— 内部数据与辅助函数,外面碰不到(保护起来防误改)。
//==================================================================================
class Pathfinder
{
public:

// ---- 内嵌枚举 1:brush_type("笔刷类型",即格子地形种类)----
// enum 把一组相关整数起名字:下面 normal/obstacle/... 分别是 0/1/2/3/4/5。
// 用户点工具栏选哪种笔刷,鼠标涂到的格子就变成对应地形。
  enum brush_type
  {
    normal   = 0,
    obstacle = 1,
    water    = 2,
    mud      = 3,
    source   = 4,
    target   = 5
  };
      
// ---- 内嵌枚举 2:algorithm_type(当前选了哪种搜索算法)----
//   non        —— 还没选算法;
//   search_dfs —— 深度优先;search_bfs —— 广度优先;
//   search_dijkstra —— Dijkstra(只管代价、不管方向);
//   search_astar    —— A*(带"启发式估计"的 Dijkstra,最快找到最短路径)。
  enum algorithm_type
  {
    non,
    search_astar,
    search_bfs,
    search_dfs,
    search_dijkstra
  };

private:
  
//==================================================================================
// private 区:下面都是 Pathfinder 对象的"内部数据"与"内部辅助函数"。
// 命名前缀约定:m_ = member(成员变量);m_i=整型;m_d=double 浮点;m_p=指针;m_b=布尔。
//==================================================================================
  //the terrain type of each cell
  std::vector<int>              m_TerrainType;
//(原文注释:每个格子的地形种类)
// m_TerrainType:长度=格子总数的数组,第 i 格存它的地形编号(normal/water/...)。

  //this vector will store any path returned from a graph search
  std::list<int>                m_Path;
//(原文注释:这里存放图搜索返回的任意一条路径)
// m_Path:搜索结果——一条由一串格子编号(节点 index)组成的链表,渲染时把它们连成蓝线。

  //create a typedef for the graph type
  typedef SparseGraph<NavGraphNode<void*>, GraphEdge> NavGraph;
//(原文注释:为图类型起一个 typedef 别名)
// typedef = 给已有类型起别名。这里把"节点是 NavGraphNode<void*>、边是 GraphEdge 的
// 稀疏图 SparseGraph<...>"整体改名成 NavGraph(导航图),后面写起来短。
// <...> 尖括号 = 模板参数,相当于"给泛型容器指定它装什么东西"。

  NavGraph*                     m_pGraph;
// m_pGraph:指向"导航图"对象的指针(实际对象用 new 在堆上创建)。
// 图 = 一张由节点(格子)和边(相邻格子之间的通路)组成的网,寻路就是在这张网上搜。
  
  //this vector of edges is used to store any subtree returned from 
  //any of the graph algorithms (such as an SPT)
  std::vector<const GraphEdge*> m_SubTree;
//(原文注释:这个边数组存放任意图算法返回的"搜索子树"/最短路径树 SPT)
// m_SubTree:算法探索过的边集合,渲染时用红线画出来,让你看见算法"探索了哪些地方"。

  //the total cost of the path from target to source
  double                         m_dCostToTarget;
//(原文注释:从终点到起点这条路径的总代价)
// m_dCostToTarget:整条路径的加权长度(过水域要 ×2,所以绕水反而更长)。

  //the currently selected algorithm
  algorithm_type                m_CurrentAlgorithm;
//(原文注释:当前选中的算法)

  //the current terrain brush
  brush_type                    m_CurrentTerrainBrush;
//(原文注释:当前选中的地形笔刷)

  //the dimensions of the cells
  double                        m_dCellWidth;
  double                        m_dCellHeight;
//(原文注释:每个格子的宽/高,单位像素)

  //number of cells vertically and horizontally
  int                           m_iCellsX,
                                m_iCellsY;
//(原文注释:横向/纵向各有多少个格子)

  //local record of the client area
  int                           m_icxClient,
                                m_icyClient;
//(原文注释:客户区宽高的本地记录)

  //the indices of the source and target cells
  int                           m_iSourceCell,
                                m_iTargetCell;
//(原文注释:起点格、终点格的编号 index)

  //flags to indicate if the start and finish points have been added
  bool                          m_bStart,
                                m_bFinish;
//(原文注释:标记起点、终点是否已经设置好)

  //should the graph (nodes and GraphEdges) be rendered?
  bool                          m_bShowGraph;

  //should the tile outlines be rendered
  bool                          m_bShowTiles;
//(原文注释:是否显示格子边框)

  //holds the time taken for the most currently used algorithm to
  //complete
  double                        m_dTimeTaken;
//(原文注释:记录最近一次算法跑完整张图花了多少时间)
  
  //this calls the appropriate algorithm
  void  UpdateAlgorithm();
//(原文注释:根据 m_CurrentAlgorithm 调用对应的搜索函数)

  //helper function for PaintTerrain (see below)
  void  UpdateGraphFromBrush(int brush, int CellIndex);
//(原文注释:PaintTerrain 的辅助函数,按笔刷更新某个格子在图里的节点/边代价)
 
 std::string GetNameOfCurrentSearchAlgorithm()const;
//==================================================================================
// public 区:对外接口(构造/析构、建图、渲染、四种算法、开关、存取档)。
//==================================================================================

public:

  Pathfinder():m_bStart(false),
// Pathfinder()——构造函数:new Pathfinder() 时自动调用,用"初始化列表"把所有
// 成员先清零。冒号 : 后面逐项写 成员(初值);m_pGraph(NULL) 表示图指针先为空(还没建图)。
                m_bFinish(false),
                m_bShowGraph(false),
                m_bShowTiles(true),
                m_dCellWidth(0),
                m_dCellHeight(0),
                m_iCellsX(0),
                m_iCellsY(0),
                m_dTimeTaken(0.0),
                m_CurrentTerrainBrush(normal),
                m_iSourceCell(0),
                m_iTargetCell(0),
                m_icxClient(0),
                m_icyClient(0),
                m_dCostToTarget(0.0),
                m_pGraph(NULL)
  {}

  ~Pathfinder(){delete m_pGraph;}
// ~Pathfinder()——析构函数:对象被销毁时自动调用,delete 释放掉 new 出来的图,
// 防止内存泄漏。波浪号 ~ 是析构函数的固定记号,与类同名、无返回值无参数。

  void CreateGraph(int CellsUp, int CellsAcross);

  void Render();
// CreateGraph / Render:建图(按格子数造一张网络图)/ 把地形、路径、树画到屏幕。

  //this will paint whatever cell the cursor is currently over in the 
  //currently selected terrain brush
  void PaintTerrain(POINTS p);
//(原文注释:把鼠标当前所在的格子涂成当前选中的笔刷地形)
// POINTS p —— Windows 给出的鼠标坐标点;

  //the algorithms
  void CreatePathDFS();
  void CreatePathBFS();
  void CreatePathDijkstra();
  void CreatePathAStar();
  void MinSpanningTree();
//(原文注释:下面是各种搜索算法入口)——DFS/BFS/Dijkstra/A* 各建一条从起点到终点的路径;
// MinSpanningTree 是"最小生成树"(本章未在工具栏挂按钮,保留接口)。

  //if m_bShowGraph is true the graph will be rendered
  void ToggleShowGraph(){m_bShowGraph = !m_bShowGraph;}
  void SwitchGraphOn(){m_bShowGraph = true;}
  void SwitchGraphOff(){m_bShowGraph = false;}
  bool isShowGraphOn()const{return m_bShowGraph;}
//(原文注释:若 m_bShowGraph 为真则绘制网络图)
// 这几个是内联开关函数:{ 花括号里直接写函数体 }。Toggle=翻转(!取反),
// SwitchOn/Off=强制开/关,is...On=查询当前状态。const 表示不改任何成员。

  void ToggleShowTiles(){m_bShowTiles = !m_bShowTiles;}
  void SwitchTilesOn(){m_bShowTiles = true;}
  void SwitchTilesOff(){m_bShowTiles = false;}
  bool isShowTilesOn()const{return m_bShowTiles;}
// 上面是格子显示的一组开关(与上面图开关对称)。

  void ChangeBrush(const brush_type NewBrush){m_CurrentTerrainBrush = NewBrush;}

  void ChangeSource(const int cell){m_iSourceCell = cell;}
// ChangeBrush/ChangeSource/ChangeTarget:外界(工具栏按钮)告诉它换笔刷、换起点格、换终点格。
  void ChangeTarget(const int cell){m_iTargetCell = cell;}

  //converts a POINTS to an index into the graph. Returns false if p
  //is invalid
  bool PointToIndex(POINTS p, int& NodeIndex);
//(原文注释:把一个鼠标坐标 POINTS 换算成图里的节点编号;p 非法时返回 false)
// int& 的 & 是"引用":调用者把自己的变量传进来,函数可直接改写它(NodeIndex)。

  //returns the terrain cost of the brush type
  double GetTerrainCost(brush_type brush);
//(原文注释:返回某种笔刷地形的"移动代价")——平地 1.0、泥 1.5、水 2.0,障碍视为不可走。

  void Save( char* FileName);
  void Load( char* FileName);
// Save/Load:把当前地形地图存成 .map 文件 / 从 .map 文件读回来(char* = C 风格字符串)。

};


#endif