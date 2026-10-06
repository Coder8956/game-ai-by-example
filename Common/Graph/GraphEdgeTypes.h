//==============================================================================================
//【文件说明】GraphEdgeTypes.h —— 图论模块的"边(连接)"类定义
//
//【这个文件是干什么的?】
//  一张图由"节点(Node)"和"边(Edge)"组成。本文件定义两种边:
//    GraphEdge    —— 最基础的边:只记"从哪到哪"+"走这条边的代价(cost)";
//    NavGraphEdge —— 继承自 GraphEdge,多了"通行标志(flags)"和"穿过的实体 ID",
//                    用于第 5 章寻路:比如某条边要游泳/跳跃/穿过门,AI 才能判断
//                    自己能不能走这条路。
//
//【谁在使用这个文件?】
//  SparseGraph.h / GraphNodeTypes.h / GraphAlgorithms.h / HandyGraphFunctions.h
//  —— 第 5 章 Pathfinder 工程的整套图模块都要它;Raven(第 7~10 章)也用 NavGraphEdge。
//
//【本文件包含了谁?】
//  <ostream> —— 标准库输出流(operator<< 打印边用);
//  <fstream>  —— 文件流(从文件读边的构造函数用);
//  "graph/NodeTypeEnumerations.h" —— 借用里面的 invalid_node_index(-1)常量。
//
//【C++ 小课堂:继承(inheritance)】
//  class NavGraphEdge : public GraphEdge 表示 NavGraphEdge "是一种" GraphEdge,
//  自动拥有 GraphEdge 的全部成员(from/to/cost),只需再加自己的新成员。
//  "virtual ~GraphEdge()" 是虚析构函数:当用父类指针销毁子类对象时,
//  保证子类的析构也能被正确调用(防止内存泄漏)。
//==============================================================================================
#ifndef GRAPH_EDGE_TYPES_H
#define GRAPH_EDGE_TYPES_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Name:   GraphEdgeTypes.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   Class to define an edge connecting two nodes.
//          
//          An edge has an associated cost.
//-----------------------------------------------------------------------------
#include <ostream>
#include <fstream>

#include "graph/NodeTypeEnumerations.h"


//--------------------------------------------------------------------------------
// class GraphEdge —— 基础边类。protected 表示"自家子类能看见,外人不能直接碰":
//   m_iFrom = 起点节点编号;m_iTo = 终点节点编号;m_dCost = 走这条边的代价。
//   i = int 整数;d = double 浮点数(m_ 匈牙利前缀,详见 Goal.h 小课堂)。
//--------------------------------------------------------------------------------
class GraphEdge
{
protected:

  //(原文注释:一条边连接两个节点,节点编号恒为正数)
  //An edge connects two nodes. Valid node indices are always positive.
  int     m_iFrom;
  int     m_iTo;

  //the cost of traversing the edge
  double  m_dCost;

public:

//--------------------------------------------------------------------------------
// 三个构造函数(ctor = constructor):按不同参数组合初始化一条边。
//   带 from/to/cost :全参数版;
//   带 from/to      :省略代价,默认 cost=1.0;
//   空参()         :from/to 都设成 invalid_node_index(-1),表示"空边"。
// 冒号后是初始化列表(原理见 Goal.h)。
//--------------------------------------------------------------------------------
  //ctors
  GraphEdge(int from, int to, double cost):m_dCost(cost),
                                           m_iFrom(from),
                                           m_iTo(to)
  {}
  
  GraphEdge(int from, int  to):m_dCost(1.0),
                               m_iFrom(from),
                               m_iTo(to)
  {}
  
  GraphEdge():m_dCost(1.0),
              m_iFrom(invalid_node_index),
              m_iTo(invalid_node_index)
  {}

  //stream constructor
//--------------------------------------------------------------------------------
// 从文件流读边的构造函数:图可以预先存在文本文件里,程序启动时读进来。
//  char buffer[50] 是临时字符数组,用来"吞掉"文件里的括号等分隔符再读数值。
//--------------------------------------------------------------------------------
  GraphEdge(std::ifstream& stream)
  {
    char buffer[50];
    stream  >> buffer >> m_iFrom >> buffer >> m_iTo >> buffer >> m_dCost;
  }

  virtual ~GraphEdge(){}

  // 下面 6 个是访问器/设置器:外界只能通过这些函数读写 from/to/cost(封装原则)。
  int   From()const{return m_iFrom;}
  void  SetFrom(int NewIndex){m_iFrom = NewIndex;}

  int   To()const{return m_iTo;}
  void  SetTo(int NewIndex){m_iTo = NewIndex;}

  double Cost()const{return m_dCost;}
  void  SetCost(double NewCost){m_dCost = NewCost;}


//--------------------------------------------------------------------------------
// 重载 operator==(相等判断):两条边相等 = 起点、终点、代价都一样。
//   rhs = right-hand side,等号右边那个对象的简写;this 指向"自己"。
//  operator!= 直接用 == 的结果取反(!),省得再写一遍比较逻辑。
//--------------------------------------------------------------------------------
  //these two operators are required
  bool operator==(const GraphEdge& rhs)
  {
    return rhs.m_iFrom == this->m_iFrom &&
           rhs.m_iTo   == this->m_iTo   &&
           rhs.m_dCost == this->m_dCost;
  }

  bool operator!=(const GraphEdge& rhs)
  {
    return !(*this == rhs);
  }

//--------------------------------------------------------------------------------
// friend(友元)operator<<:让 cout << edge 能直接打印一条边。
//   友元 = 这个外部函数虽然不是类的成员,但被允许直接访问私有成员。
//--------------------------------------------------------------------------------
  //for reading and writing to streams.
  friend std::ostream& operator<<(std::ostream& os, const GraphEdge& e)
  {
    os << "m_iFrom: " << e.m_iFrom << " m_iTo: " << e.m_iTo 
       << " m_dCost: " << e.m_dCost << std::endl;
    
    return os;
  }

};


//--------------------------------------------------------------------------------
// class NavGraphEdge : public GraphEdge —— 导航边:在基础边上加"通行能力标志"。
//   : public 表示公有继承(父类的 public 在子类里仍是 public)。
//--------------------------------------------------------------------------------
class NavGraphEdge : public GraphEdge
{
public:
  
//--------------------------------------------------------------------------------
// 匿名枚举:通行标志位。用 1 << n(左移)让每个标志占二进制的一位,
// 这样多个标志可以用按位或 | 叠在一起(如 swim|jump),互不干扰。
//   例:swim=1(0001),crawl=2(0010)。注意原作者笔误 crawl 与 jump 都写成 1<<3,
//   值相同,这是原书代码,原样保留。
//--------------------------------------------------------------------------------
  //examples of typical flags
  enum
  {
    normal            = 0,
    swim              = 1 << 0,
    crawl             = 1 << 1,
    creep             = 1 << 3,
    jump              = 1 << 3,
    fly               = 1 << 4,
    grapple           = 1 << 5,
    goes_through_door = 1 << 6
  };

protected:

  // 新增两个成员:m_iFlags(通行标志组合);
  // m_iIDofIntersectingEntity(若这条边穿过门/电梯等物体,记录那个物体的 ID;-1 表示没穿)。
  int   m_iFlags;

  //if this edge intersects with an object (such as a door or lift), then
  //this is that object's ID. 
  int  m_iIDofIntersectingEntity;

public:
 
  
//--------------------------------------------------------------------------------
// 导航边构造函数:先调用父类 GraphEdge(from,to,cost) 把基础三参数初始化好,
// 再用初始化列表初始化自己的 flags 和 id(flags/id 缺省为 0 和 -1)。
//--------------------------------------------------------------------------------
  NavGraphEdge(int    from,
               int    to,
               double cost,
               int    flags = 0,
               int    id = -1):GraphEdge(from,to,cost),
                               m_iFlags(flags),
                               m_iIDofIntersectingEntity(id)

  {} 


  //stream constructor
  NavGraphEdge(std::ifstream& stream)
  {
    char buffer[50];
    stream  >> buffer >> m_iFrom >> buffer >> m_iTo >> buffer >> m_dCost;
    stream >> buffer >> m_iFlags >> buffer >> m_iIDofIntersectingEntity;
  }

  // 访问器:Flags 读写通行标志;IDofIntersectingEntity 读写穿过物体的 ID。
  int  Flags()const{return m_iFlags;}
  void SetFlags(int flags){m_iFlags = flags;}
  
  int  IDofIntersectingEntity()const{return m_iIDofIntersectingEntity;}
  void SetIDofIntersectingEntity(int id){m_iIDofIntersectingEntity = id;}

 
  friend std::ostream& operator<<(std::ostream& os, const NavGraphEdge& e)
  {
    os << "m_iFrom: " << e.m_iFrom << " m_iTo: " << e.m_iTo 
       << " m_dCost: " << e.m_dCost << " m_iFlags: " << e.m_iFlags
       << " ID: " << e.m_iIDofIntersectingEntity << std::endl;
    
    return os;
  }
};


#endif