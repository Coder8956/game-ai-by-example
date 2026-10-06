//==============================================================================================
//【文件说明】WallIntersectionTests.h —— "线段 vs 一整墙"的相交测试
//
//【这个文件是干什么的?】
//  游戏地图里有很多墙。本文件提供一组模板函数:给定一条视线/路线,逐个跟所有墙
//  做相交测试,回答"这条路被墙挡住了吗?""最近的交点在哪?""这个圆有没有压到墙?"。
//  Raven 的视线遮挡、足球的传球可达性判断都靠它。
//
//【谁在使用这个文件?】Raven 游戏世界、足球足球世界等——判断视线/路线是否被墙挡。
//
//【本文件包含了谁?】2d/Vector2D.h(向量)、2d/Wall2d.h(墙类)。
//
//【C++ 小课堂:模板(template)】
//  template <class ContWall> 的意思:"下面这个函数先留一个占位类型 ContWall",
//  调用时才由编译器根据你传入的容器类型(比如装墙指针的 vector)把它具体化一份。
//  这样一套函数能通吃各种"装墙的容器"。const_iterator 是容器的"只读遍历器",
//  像游标一样逐个指向容器里的元素。
//==============================================================================================
//-----------------------------------------------------------------------------
//
//  Name:   WallIntersectionTests.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   a few functions for testing line segments against containers of
//          walls
//-----------------------------------------------------------------------------

#include "2d/Vector2D.h"
#include "2d/Wall2d.h"


//----------------------- doWallsObstructLineSegment --------------------------
//
//  given a line segment defined by the points from and to, iterate through all
//  the map objects and walls and test for any intersection. This method
//  returns true if an intersection occurs.
// ↓↓↓ 原文翻译【doWallsObstructLineSegment】:给定线段 from→to,遍历所有墙,
//   只要有一面墙与它相交就返回 true(被挡住了)。
//-----------------------------------------------------------------------------
template <class ContWall>
inline bool doWallsObstructLineSegment(Vector2D from,
                                       Vector2D to,
                                       const ContWall& walls)
{
  //test against the walls
  ContWall::const_iterator curWall = walls.begin();

  for (curWall; curWall != walls.end(); ++curWall)
  {
    //do a line segment intersection test
    if (LineIntersection2D(from, to, (*curWall)->From(), (*curWall)->To()))
    {
      return true;
    }
  }
                                                                           
  return false;
}


//----------------------- doWallsObstructCylinderSides -------------------------
//
//  similar to above except this version checks to see if the sides described
//  by the cylinder of length |AB| with the given radius intersect any walls.
//  (this enables the trace to take into account any the bounding radii of
//  entity objects)
//-----------------------------------------------------------------------------
template <class ContWall>
// ↓↓↓ 原文翻译【doWallsObstructCylinderSides】:与上类似,但把"角色"看成一个
//   半径 BoundingRadius 的圆,测试这个"圆管"的两条侧边线段是否撞墙
inline bool doWallsObstructCylinderSides(Vector2D        A,
                                         Vector2D        B,
                                         double           BoundingRadius,
                                         const ContWall& walls)
{
  //the line segments that make up the sides of the cylinder must be created
  Vector2D toB = Vec2DNormalize(B-A);

  //A1B1 will be one side of the cylinder, A2B2 the other.
  Vector2D A1, B1, A2, B2;

  Vector2D radialEdge = toB.Perp() * BoundingRadius;

  //create the two sides of the cylinder
  A1 = A + radialEdge;
  B1 = B + radialEdge;

  A2 = A - radialEdge;
  B2 = B - radialEdge;

  //now test against them
  if (!doWallsObstructLineSegment(A1, B1, walls))
  {
    return doWallsObstructLineSegment(A2, B2, walls);
  }
  
  return true;
}

//------------------ FindClosestPointOfIntersectionWithWalls ------------------
//
//  tests a line segment against the container of walls  to calculate
//  the closest intersection point, which is stored in the reference 'ip'. The
//  distance to the point is assigned to the reference 'distance'
//
//  returns false if no intersection point found
// ↓↓↓ 原文翻译【FindClosestPointOfIntersectionWithWalls】:线段 vs 所有墙,求出
//   最近的交点(存进引用 ip)与距离(存进引用 distance);没交点返回 false。
//   MaxDouble 是 float 最大值,初始当"无穷大"用。
//-----------------------------------------------------------------------------

template <class ContWall>
inline bool FindClosestPointOfIntersectionWithWalls(Vector2D        A,
                                                    Vector2D        B,
                                                    double&          distance,
                                                    Vector2D&       ip,
                                                    const ContWall& walls)
{
  distance = MaxDouble;

  ContWall::const_iterator curWall = walls.begin();
  for (curWall; curWall != walls.end(); ++curWall)
  {
    double dist = 0.0;
    Vector2D point;

    if (LineIntersection2D(A, B, (*curWall)->From(), (*curWall)->To(), dist, point))
    {
      if (dist < distance)
      {
        distance = dist;
        ip = point;
      }
    }
  }

  if (distance < MaxDouble) return true;

  return false;
}

//------------------------ doWallsIntersectCircle -----------------------------
//
//  returns true if any walls intersect the circle of radius at point p
// ↓↓↓ 原文翻译【doWallsIntersectCircle】:若任一堵墙与点 p 处、半径 r 的圆相交,返回 true。
//-----------------------------------------------------------------------------
template <class ContWall>
inline bool doWallsIntersectCircle(const ContWall& walls, Vector2D p, double r)
{
  //test against the walls
  ContWall::const_iterator curWall = walls.begin();

  for (curWall; curWall != walls.end(); ++curWall)
  {
    //do a line segment intersection test
    if (LineSegmentCircleIntersection((*curWall)->From(), (*curWall)->To(), p, r))
    {
      return true;
    }
  }
                                                                           
  return false;
}


