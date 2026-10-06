//==============================================================================================
//【文件说明】TriggerRegion.h —— 触发器影响区域的几何形状
//
//【这个文件是干什么的?】
//  TriggerRegion 是抽象基类,只规定一个接口 isTouching(实体位置,半径) → 是否重叠。
//  两个具体子类:
//    TriggerRegion_Circle    —— 圆形区域(用距离平方判断);
//    TriggerRegion_Rectangle —— 矩形区域(用实体的包围盒和矩形的重叠判断)。
//
//【谁在使用这个文件?】
//  Trigger.h 持一个 TriggerRegion* 指向这两个子类之一。
//==============================================================================================
#ifndef TRIGGER_REGION_H
#define TRIGGER_REGION_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Name:   TriggerRegion.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to define a region of influence for a trigger. A 
//          TriggerRegion has one method, isTouching, which returns true if
//          a given position is inside the region
//-----------------------------------------------------------------------------
#include "2d/Vector2d.h"
  // 抽象基类:isTouching 纯虚,子类必须实现。
#include "2d/InvertedAABBox2D.h"

class TriggerRegion
{
public:

  virtual ~TriggerRegion(){}

  //returns true if an entity of the given size and position is intersecting
  //the trigger region.
  virtual bool isTouching(Vector2D EntityPos, double EntityRadius)const = 0;
};


  // 圆形区域:存圆心 m_vPos 和半径 m_dRadius;isTouching 用距离平方 < (r1+r2)^2 判断。
//--------------------------- TriggerRegion_Circle ----------------------------
//
//  class to define a circular region of influence
//-----------------------------------------------------------------------------
class TriggerRegion_Circle : public TriggerRegion
{
private:

  //the center of the region
  Vector2D m_vPos;
  
  //the radius of the region
  double    m_dRadius;

public:

  TriggerRegion_Circle(Vector2D pos, 
                       double    radius):m_dRadius(radius),
                                        m_vPos(pos)
  {}

  bool isTouching(Vector2D pos, double EntityRadius)const
  {
    return Vec2DDistanceSq(m_vPos, pos) < (EntityRadius + m_dRadius)*(EntityRadius + m_dRadius);
  }
};


  // 矩形区域:包一个 InvertedAABBox2D;isTouching 用实体外接盒与矩形重叠判断。
//--------------------------- TriggerRegion_Rectangle -------------------------
//
//  class to define a circular region of influence
//-----------------------------------------------------------------------------
class TriggerRegion_Rectangle : public TriggerRegion
{
private:

  InvertedAABBox2D* m_pTrigger;
  
public:

  TriggerRegion_Rectangle(Vector2D TopLeft, 
                          Vector2D BottomRight)
  {
    m_pTrigger = new InvertedAABBox2D(TopLeft, BottomRight);
  }

  ~TriggerRegion_Rectangle(){delete m_pTrigger;}

  //there's no need to do an accurate (and expensive) circle v
  //rectangle intersection test. Instead we'll just test the bounding box of
  //the given circle with the rectangle.
  bool isTouching(Vector2D pos, double EntityRadius)const
  {
    InvertedAABBox2D Box(Vector2D(pos.x-EntityRadius, pos.y-EntityRadius),
                         Vector2D(pos.x+EntityRadius, pos.y+EntityRadius));

    return Box.isOverlappedWith(*m_pTrigger);
  }
};


#endif