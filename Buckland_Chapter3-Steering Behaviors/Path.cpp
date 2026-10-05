//==============================================================================================
//【文件说明】Path.cpp —— 路径类的"随机生成"与"渲染"实现
//
//【这个文件是干什么的?】
//  1. CreateRandomPath():在一个矩形区域内,均匀地生成一串随机路标点,
//     看起来像一个"不规则的多边形路线"。随机路径+循环模式 → 机器人
//     会沿着这条路线一圈圈地飞(演示"跟随路径"行为);
//  2. Render():用橙色线段把相邻路标点连起来画到屏幕上(调试辅助)。
//
//【与相关文件的关系】
//  Path.h             —— 本文件实现它的两个函数;
//  misc/utils.h       —— 提供 min(取小)、TwoPi(2π)、RandInRange(随机数);
//  misc/Cgdi.h        —— 绘图单例 gdi:OrangePen(橙色笔)、Line(画线);
//  2d/transformations.h —— 提供 Vec2DRotateAroundOrigin(绕原点旋转向量);
//  SteeringBehaviors.cpp —— FollowPath() 行为消费这里的路标点。
//
//【算法思路:怎么"均匀随机"地生成一圈点?】
//  1) 算出矩形中心 midX/midY;2) 在半径(0.2×较小边 ~ 较小边)之间随机取
//     一个"径向距离";3) 把这个点绕中心按角度 i×(360°/数量)旋转——角度
//     是均匀分的,距离是随机的 → 得到一圈"围绕中心、疏密随机"的点。
//==============================================================================================
#include "Path.h"
// 包含 Common 目录的 misc/utils.h:min(取两者较小值)、TwoPi(2π≈6.28)、
// RandInRange(在区间内取随机数)都来自这里。
#include "misc/utils.h"
// 包含绘图单例 gdi(misc/Cgdi.h):Render() 里画线要用。
#include "misc/Cgdi.h"
// 包含 2D 变换工具(Common 目录):Vec2DRotateAroundOrigin(绕原点旋转向量)
// 来自这里。注意文件名是小写 transformations.h,与头文件引用大小写一致。
#include "2d/transformations.h"

//--------------------------------------------------------------------------------
//【CreateRandomPath:生成一条随机路径】
//  参数:路标点数量 + 矩形范围(MinX/MinY/MaxX/MaxY);
//  返回:填好路标点的链表。矩形范围通常来自窗口尺寸(见 GameWorld.cpp)。
//--------------------------------------------------------------------------------
std::list<Vector2D> Path::CreateRandomPath(int   NumWaypoints,
                                           double MinX,
                                           double MinY,
                                           double MaxX,
                                           double MaxY)
{
// 先把旧路标点全部清掉:从一张"白纸"开始画。
// clear() 是 list 的成员函数:删除所有元素。
    m_WayPoints.clear();

// 矩形中心的横坐标 = (最大X + 最小X) / 2。所有随机点都围绕这个中心分布。
    double midX = (MaxX+MinX)/2.0;
// 矩形中心的纵坐标 = (最大Y + 最小Y) / 2。
    double midY = (MaxY+MinY)/2.0;

// 取中心坐标里较小的那个值,记作 smaller:它决定"随机半径"的上限,保证
// 生成的点不会跑出矩形范围(半径最大 = 中心到边的距离)。
// min(midX, midY) 返回两者中较小者。
    double smaller = min(midX, midY);

// spacing = 角度间隔:一圈共 2π 弧度,平均分给 NumWaypoints 个点,每个点
// 相隔 spacing 弧度。TwoPi 是常量 2π(来自 utils.h)。
// (double)NumWaypoints —— 把整数强转成 double,保证除法结果是小数。
    double spacing = TwoPi/(double)NumWaypoints;

// for 循环:生成 NumWaypoints 个路标点。i 从 0 数到 NumWaypoints-1,每轮一个点。
    for (int i=0; i<NumWaypoints; ++i)
    {
// 随机取"径向距离":在 smaller×0.2 到 smaller 之间取随机数,单位是像素。
// 0.2f 是 0.2 的 float 写法(f 后缀);RandInRange 返回区间内随机值。
      double RadialDist = RandInRange(smaller*0.2f, smaller);

// 先做一个"躺在 X 轴上"的临时向量 (RadialDist, 0):半径朝右,角度为 0。
// Vector2D(横坐标, 纵坐标) 是它的构造函数。
      Vector2D temp(RadialDist, 0.0f);

// 把这个向量绕原点旋转 i×spacing 弧度:第 i 个点就落在"均匀分角"的位置上。
// Vec2DRotateAroundOrigin(向量, 角度) 直接修改传入的向量(按引用)。
// (传入的是 temp 本身,& 按引用——函数内改的就是它。)
      Vec2DRotateAroundOrigin(temp, i*spacing);

// 把点平移到矩形中心:横坐标加 midX、纵坐标加 midY(整行两句,分号分隔)。
// += 读作"自身加上":temp.x = temp.x + midX 的简写。
      temp.x += midX; temp.y += midY;
      
// 把这个路标点追加到链表末尾。push_back 是 list 的成员函数:尾部插入。
      m_WayPoints.push_back(temp);
                            
    }

// 生成完毕,把"当前书签"拨回链表开头——机器人从第一个路标点开始走。
// begin() 返回指向第一个元素的迭代器。
    curWaypoint = m_WayPoints.begin();

// 把整条路径(链表)返回给调用者(按值返回一份拷贝)。
    return m_WayPoints;
}


//--------------------------------------------------------------------------------
//【Render:把路径画到屏幕上】(const:只读,不修改成员)
//  做法:依次取相邻两个路标点,用橙色线段连起来;如果开启循环,再把
//  最后一个点连回第一个点,形成闭合图形。
//--------------------------------------------------------------------------------
void Path::Render()const
{
// 把画笔换成橙色(OrangePen = 橙色画笔,来自绘图单例 gdi)。
  gdi->OrangePen();

// 定义只读迭代器 it,指向链表开头。const_iterator:只能读、不能改元素。
  std::list<Vector2D>::const_iterator it = m_WayPoints.begin();

// 取出第一个路标点给 wp(线段起点),同时 *it++ 让书签移到下一个(起点前移)。
// *it 取当前元素,it++ 是"先取值、书签后移"(后置自增)。
  Vector2D wp = *it++;

// while 循环:只要书签还没到末尾哨兵(end),就一直画。
  while (it != m_WayPoints.end())
  {
// 从 wp(上一个点)到 *it(当前点)画一条线段。gdi->Line(起点, 终点)。
    gdi->Line(wp, *it);

// 把当前点记为新的起点 wp,书签再后移一位(*it++ 先取值再移动)。
    wp = *it++;
  }

// 如果开启循环(m_bLooped 为 true),把最后一个点连回第一个点,路径闭合。
// --it 先把书签退回最后那个真实元素,再取它的值;
// *m_WayPoints.begin() 取第一个路标点。
  if (m_bLooped) gdi->Line(*(--it), *m_WayPoints.begin());
}
