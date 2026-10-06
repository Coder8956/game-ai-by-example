//==============================================================================================
//【文件说明】AStarHeuristicPolicies.h —— A* 寻路的"启发函数"策略集
//
//【这个文件是干什么的?】
//  A* 寻路的核心是一个"启发函数(heuristic)":估算"从节点 nd1 到终点 nd2 还有多远"。
//  估得越准,A* 搜得越快。本文件给了 3 种现成策略类,任选一个配给 Graph_SearchA* 用:
//    Heuristic_Euclid       —— 直线距离(最常用,也是标准 A*);
//    Heuristic_Noisy_Euclidian —— 直线距离×随机扰动,让多个 AI 走出不完全一样的路,
//                                 避免大家排成一队跟在屁股后面;
//    Heuristic_Dijkstra     —— 永远返回 0;启发值恒为 0 的 A* 就退化成 Dijkstra 算法。
//
//【谁在使用这个文件?】
//  GraphAlgorithms.h(Graph_SearchA* 模板)在定义 A* 时把启发函数当模板参数传入;
//  HandyGraphFunctions.h 也直接 #include 它。第 5 章 Pathfinder 工程使用。
//
//【本文件包含了谁?】
//  "misc/utils.h" —— 工具函数库(Vec2DDistance 直线距离、RandInRange 随机范围数)。
//
//【C++ 小课堂:静态成员函数 + 模板成员函数】
//  static double Calculate(...) 表示这是"类级别的函数",不依赖任何对象,
//  直接用 类名::Calculate(...) 调用(这里类本身被当策略用,不 new 对象)。
//  template <class graph_type> 是嵌在类里面的模板方法,调用时由编译器从参数 G 的
//  类型自动推断 graph_type。
//==============================================================================================
#ifndef ASTAR_HEURISTIC_POLICIES_H
#define ASTAR_HEURISTIC_POLICIES_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Name:   AStarHeuristicPolicies.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class templates defining a heuristic policy for use with the A*
//          search algorithm
//-----------------------------------------------------------------------------
#include "misc/utils.h"

//-----------------------------------------------------------------------------
//the euclidian heuristic (straight-line distance)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 直线距离启发:返回两节点坐标间的欧几里得距离(Vec2DDistance,工具库函数)。
// 这是 A* 最经典的启发式——永不高估真实代价,保证找到最短路径。
//--------------------------------------------------------------------------------
class Heuristic_Euclid 
{
public:

  Heuristic_Euclid(){}

  //calculate the straight line distance from node nd1 to node nd2
  template <class graph_type>
  static double Calculate(const graph_type& G, int nd1, int nd2)
  {
    return Vec2DDistance(G.GetNode(nd1).Pos(), G.GetNode(nd2).Pos());
  }
};

//-----------------------------------------------------------------------------
//this uses the euclidian distance but adds in an amount of noise to the 
//result. You can use this heuristic to provide imperfect paths. This can
//be handy if you find that you frequently have lots of agents all following
//each other in single file to get from one place to another
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 带噪声的直线距离:距离 × RandInRange(0.9, 1.1),即打 9 折到 1.1 折的随机抖动。
// 让 AI 偶尔选一条稍远但不那么"整整齐齐"的路,避免群体行为太机械。
//--------------------------------------------------------------------------------
class Heuristic_Noisy_Euclidian
{
public:

  Heuristic_Noisy_Euclidian(){}

  //calculate the straight line distance from node nd1 to node nd2
  template <class graph_type>
  static double Calculate(const graph_type& G, int nd1, int nd2)
  {
    return Vec2DDistance(G.GetNode(nd1).Pos(), G.GetNode(nd2).Pos()) * RandInRange(0.9f, 1.1f);
  }
};

//-----------------------------------------------------------------------------
//you can use this class to turn the A* algorithm into Dijkstra's search.
//this is because Dijkstra's is equivalent to an A* search using a heuristic
//value that is always equal to zero.
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Dijkstra 策略:启发值永远返回 0。A* 的估价函数 = 已走代价 + 启发值;
// 启发值为 0 时,A* 只按已走代价扩展节点,就变成了 Dijkstra 最短路算法。
//--------------------------------------------------------------------------------
class Heuristic_Dijkstra 
{
public:

  template <class graph_type>
  static double Calculate(const graph_type& G, int nd1, int nd2)
  {
    return 0;
  }
};






#endif