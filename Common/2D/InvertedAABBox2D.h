//==============================================================================================
//【文件说明】InvertedAABBox2D.h —— "轴对齐包围盒"(屏幕坐标系版)
//
//【这个文件是干什么的?】
//  用左上角、右下角两个点定义一个"正放的矩形"(边与坐标轴平行,不旋转)。
//  游戏里常把角色/节点包在这种矩形里,快速判断"两个矩形有没有重叠",
//  比逐像素判断快得多。名字里 Inverted(倒置)指:屏幕坐标 y 轴向下为正。
//
//【谁在使用这个文件?】第 5 章寻路工程:给图节点/区域套包围盒,做可视范围剔除。
//
//【本文件包含了谁?】
//  2d/Vector2D.h —— 二维向量;misc/cgdi.h —— 绘图单例 gdi(Render 画矩形用)。
//==============================================================================================
#ifndef INVAABBOX2D_H
#define INVAABBOX2D_H
//-----------------------------------------------------------------------------
//
//  Name:   InvertedAABBox2D.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   v simple inverted (y increases down screen) axis aligned bounding
//          box class
//-----------------------------------------------------------------------------

#include "2d/Vector2D.h"
#include "misc/cgdi.h"

//--------------------------------------------------------------------------------
// class InvertedAABBox2D —— 轴对齐包围盒类。
class InvertedAABBox2D
{
private:
  
  // 三个成员:m_vTopLeft 左上角、m_vBottomRight 右下角、m_vCenter 中心(两点中点)。
  Vector2D  m_vTopLeft;
  Vector2D  m_vBottomRight;

  Vector2D  m_vCenter;
  
public:

  // 构造函数:给左上角 tl、右下角 br,中心自动算成 (tl+br)/2。
  InvertedAABBox2D(Vector2D tl,
                   Vector2D br):m_vTopLeft(tl),
                                m_vBottomRight(br),
                                m_vCenter((tl+br)/2.0)
  {}

  //returns true if the bbox described by other intersects with this one
  //(原文注释:若另一个包围盒 other 与本盒相交则返回 true)
  // 思路:两个矩形"不相交"的条件是一方完全在另一方的左/右/上/下之外;
  // 把这些情况取反(!)就是"相交"。|| 是逻辑或。
  bool isOverlappedWith(const InvertedAABBox2D& other)const
  {
    return !((other.Top() > this->Bottom()) ||
           (other.Bottom() < this->Top()) ||
           (other.Left() > this->Right()) ||
           (other.Right() < this->Left()));
  }

  
  // 下面是一组访问器:返回四角坐标或顶/底/左/右边界值、中心点。
  Vector2D TopLeft()const{return m_vTopLeft;}
  Vector2D BottomRight()const{return m_vBottomRight;}

  double    Top()const{return m_vTopLeft.y;}
  double    Left()const{return m_vTopLeft.x;}
  double    Bottom()const{return m_vBottomRight.y;}
  double    Right()const{return m_vBottomRight.x;}
  Vector2D Center()const{return m_vCenter;}

  // Render:把矩形画出来(gdi->Line 画四条边);可选再画一个中心点小圆。
  void     Render(bool RenderCenter = false)const
  {
    gdi->Line((int)Left(), (int)Top(), (int)Right(), (int)Top() );
    gdi->Line((int)Left(), (int)Bottom(), (int)Right(), (int)Bottom() );
    gdi->Line((int)Left(), (int)Top(), (int)Left(), (int)Bottom() );
    gdi->Line((int)Right(), (int)Top(), (int)Right(), (int)Bottom() );


    if (RenderCenter)
    {
      gdi->Circle(m_vCenter, 5);
    }
  }

};
  
#endif