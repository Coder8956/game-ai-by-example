//==============================================================================================
//【文件说明】Path.h —— "路径"类声明:一串路标点组成的行军路线
//
//【这个文件是干什么的?】
//  定义一条"路径":由一串路标点(waypoint,本质是 2D 坐标)按顺序排成。
//  "跟随路径"(FollowPath)行为会让机器人依次朝每个路标点飞,走完整个路线。
//  本类还能生成"随机路径"(在给定矩形区域内画一圈点),并支持"循环路径"
//  (最后一点连回第一点,机器人绕圈走)。
//
//【与相关文件的关系】
//  2d/Vector2D.h —— 路径点类型 Vector2D 的定义(Common 目录);
//  <list>        —— C++ 标准库"双向链表"容器:装路标点用(可快速插入删除);
//  <cassert>     —— 断言工具:调试期检查错误条件(SetNextWaypoint 用);
//  SteeringBehaviors.h/.cpp —— 机器人持有 Path* 指针,FollowPath() 行为
//                    每次询问"当前该去哪个路标点";
//  GameWorld.h/.cpp —— 创建路径对象并塞给每个机器人(按 U 键换新路径);
//  Path.cpp      —— 随机路径生成 CreateRandomPath 与画线 Render 的实现。
//
//【C++ 小课堂:标准库容器 std::list 与迭代器 iterator】
//  std::list<Vector2D> —— 一个"链表",每个元素是一个 Vector2D。
//  iterator(迭代器)    —— 像"书签":可以指向链表里的某个元素,++ 移到下一个,
//                          * 取出它指向的元素。curWaypoint 就是"当前书签"。
//==============================================================================================
#ifndef PATH_H
#define PATH_H
//------------------------------------------------------------------------
//
//  Name:   Path.h
//
//  Desc:   class to define, manage, and traverse a path (defined by a series of 2D vectors)
//          
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// 包含 C++ 标准库的 list(链表)容器:m_WayPoints 用它存储一串路标点。
#include <list>
// 包含断言工具 <cassert>:提供 assert(条件) 宏——条件为假时程序立刻中断,
// 报出错文件与行号(调试期抓 bug 用)。
#include <cassert>

// 包含 2D 向量工具(Common 目录):路标点的类型 Vector2D 定义在这里。
#include "2d/Vector2D.h"




//【类定义开始】class Path —— 路径类。私有区先声明三个数据成员。
class Path
{
// private: —— 私有区:下面的成员只有 Path 自己的函数能用,外界不能直接碰。
private:
  
// 路标点链表:m_WayPoints。m_ 前缀是原作者命名习惯:"member"(成员变量)。
// 链表里的每个元素是一个 Vector2D(一个 2D 坐标)。
  std::list<Vector2D>            m_WayPoints;

// ↓↓↓ 原文注释翻译:指向"当前路标点"的迭代器(书签)。
// 机器人每次问 CurrentWaypoint(),都从它这里取当前该去的点。
  //points to the current waypoint
  std::list<Vector2D>::iterator  curWaypoint;

// ↓↓↓ 原文注释翻译:标记路径是否"循环"(最后一个路标点连回第一个)。
// m_b 前缀:m_ = member(成员), b = bool(布尔)。
  //flag to indicate if the path should be looped
  //(The last waypoint connected to the first)
  bool                           m_bLooped;

// public: —— 公开区:下面的函数任何代码都能调用。
public:
  
//【构造函数①:默认构造】Path():m_bLooped(false){}
// 冒号后初始化列表把 m_bLooped 设为 false(默认不循环);函数体为空 {}。
// 注意:此时链表还是空的,要手动 Set/CreateRandomPath 才有路标点。
  Path():m_bLooped(false){}

// ↓↓↓ 原文注释翻译:
//    用于创建带"随机路标点"路径的构造函数。MinX/Y 与 MaxX/Y 限定路径的
//    活动范围(路径点只在这个矩形内生成)。
  //constructor for creating a path with initial random waypoints. MinX/Y
  //& MaxX/Y define the bounding box of the path.
//【构造函数②:随机路径构造】参数:路标点个数 + 生成范围的四个边界值 + 是否循环。
// 函数体:先调 CreateRandomPath 生成一串随机点(实现见 Path.cpp),
// 再把"当前书签"指向链表开头——从第一个路标点开始走。
  Path(int    NumWaypoints,
       double MinX,
       double MinY,
       double MaxX,
       double MaxY,
       bool   looped):m_bLooped(looped)
  {
    CreateRandomPath(NumWaypoints, MinX, MinY, MaxX, MaxY);

    curWaypoint = m_WayPoints.begin();
  }


// ↓↓↓ 原文注释翻译:返回当前路标点。
// 返回 *curWaypoint:用 * 取出"书签"指向的那个 Vector2D(值拷贝返回)。
// const 结尾:只读,不修改成员。
  //returns the current waypoint
  Vector2D    CurrentWaypoint()const{return *curWaypoint;}

// ↓↓↓ 原文注释翻译:走到链表末尾时返回 true(路径走完了)。
// 写法 !(curWaypoint != m_WayPoints.end()):先判断书签是否还没到末尾,
// 再取反。等价于 curWaypoint == m_WayPoints.end()(书签到了末尾)。
  //returns true if the end of the list has been reached
  bool        Finished(){return !(curWaypoint != m_WayPoints.end());}
  
// ↓↓↓ 原文注释翻译:把迭代器移到链表里的下一个路标点。
// inline:建议编译器"内联展开"(把小函数直接嵌进调用处,省一次函数调用开销)。
// 实现写在文件底部(本文件内)。
  //moves the iterator on to the next waypoint in the list
  inline void SetNextWaypoint();

// ↓↓↓ 原文注释翻译:创建一条被 min/max 矩形边界约束的随机路径。
// 返回整条路径的链表(实现见 Path.cpp)。
  //creates a random path which is bound by rectangle described by
  //the min/max values
  std::list<Vector2D> CreateRandomPath(int    NumWaypoints,
                                       double MinX,
                                       double MinY,
                                       double MaxX,
                                       double MaxY);


// 打开循环:把 m_bLooped 设为 true——走完最后一点后绕回第一点。
// 函数体只有一条赋值语句,写在花括号里。
  void LoopOn(){m_bLooped = true;}
// 关闭循环:设为 false——走完最后一点即结束。
  void LoopOff(){m_bLooped = false;}
 
// ↓↓↓ 原文注释翻译:在路径末尾追加一个路标点。
  //adds a waypoint to the end of the path
// 声明:追加路标点(注意:本工程未提供 .cpp 实现,声明保留是为了接口完整)。
  void AddWayPoint(Vector2D new_point);

// ↓↓↓ 原文注释翻译:用另一条 Path 或一个路标点链表来设置本路径的方法。
  //methods for setting the path with either another Path or a list of vectors
// 用"一串坐标"整体替换路径:先整表赋值,再把书签拨回开头。
// 两句写在同一行,用分号隔开。
  void Set(std::list<Vector2D> new_path){m_WayPoints = new_path;curWaypoint = m_WayPoints.begin();}
// 用"另一条 Path"复制路径:通过 GetPath() 取对方的链表来赋值,书签拨回开头。
// const Path&:只读借用对方(别名),不拷贝对象本身。
  void Set(const Path& path){m_WayPoints=path.GetPath(); curWaypoint = m_WayPoints.begin();}
  

// 清空路径:调用链表的 clear() 删除全部路标点。
  void Clear(){m_WayPoints.clear();}

// 返回整条路径链表(按值拷贝返回;const 结尾只读)。
// 调用方拿它可以重新 Set 给别的机器人。
  std::list<Vector2D> GetPath()const{return m_WayPoints;}

// ↓↓↓ 原文注释翻译:用橙色渲染(画)这条路径。
  //renders the path in orange
// 声明 Render:画路径(实现见 Path.cpp)。const:只读,不修改成员。
  void Render()const; 
};




//--------------------------------------------------------------------------------
//【行内函数实现区】下面这些函数写在头文件里、加了 inline,原因是它们太短,
// 直接内联能省去函数调用的开销。SetNextWaypoint 也实现在这里(而非 Path.cpp)。
//--------------------------------------------------------------------------------
//-------------------- Methods -------------------------------------------

//【SetNextWaypoint:把书签移到下一个路标点】
//   ++curWaypoint —— 先让书签向后挪一位,再取结果(前置自增);
//   == m_WayPoints.end() —— 判断是否已经越过最后一个元素(end 是"结尾哨兵",
//                            不是真实元素);
//   若到结尾:如果开启了循环,就把书签拨回链表开头(begin),实现"绕圈走";
//   若没开循环:什么都不做,书签停在 end——之后 Finished() 会返回 true。
inline void Path::SetNextWaypoint()
{
// assert(条件):条件"链表非空"必须成立,否则程序中断报错。
// 空链表没有路标点可移动,这是提前拦截逻辑错误。
  assert (m_WayPoints.size() > 0);
    
// ++curWaypoint 先自增再比较:书签移一位;若移到了 end(末尾哨兵)进入 if 体。
// (end() 不是最后一个元素,而是"最后一个再往后一位"的哨兵。)
  if (++curWaypoint == m_WayPoints.end())
  {
// 到末尾后,如果开启了循环模式……
    if (m_bLooped)
    {
// ……就把书签拨回链表开头(begin),形成首尾相连的环。
      curWaypoint = m_WayPoints.begin(); 
    }
  }
// 函数结束。
}  



#endif