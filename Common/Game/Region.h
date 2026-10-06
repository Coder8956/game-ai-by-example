//==============================================================================================
//【文件说明】Region.h —— 矩形区域类(场地切分块、战术区域)
//
//【这个文件是干什么的?】
//  把游戏场地切成若干矩形小块(Region)。每个区域记下四条边(上下左右)、
//  中心、宽高和编号。主要回答两个问题:
//    Inside(pos, mode)       —— 某点是不是落在这个矩形里(mode=normal 整块,
//                                mode=halfsize 向内缩 1/4 当"中央地带");
//    GetRandomPosition()     —— 在矩形内随机取一个点(让 AI 随机跑到区域里)。
//
//【谁在使用这个文件?】
//  SimpleSoccer 的 SoccerPitch(把球场切成 Region 供球员站位)、
//  Raven 的战斗区域切分、第 5 章寻路的网格定义。
//
//【本文件包含了谁?】
//  <math.h>             —— fabs 取绝对值;
//  "2D/Vector2D.h"     —— 2D 向量;
//  "misc/Cgdi.h"        —— gdi 绘图单例(Render 画矩形);
//  "misc/utils.h"       —— RandInRange 随机数;
//  "misc/Stream_Utility_Functions.h" —— ttos 数字转字符串。
//==============================================================================================
#ifndef REGION_H
#define REGION_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//------------------------------------------------------------------------
//
//  Name:   Region.h
//
//  Desc:   Defines a rectangular region. A region has an identifying
//          number, and four corners.
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------

#include <math.h>

#include "2D/Vector2D.h"
#include "misc/Cgdi.h"
#include "misc/utils.h"
#include "misc/Stream_Utility_Functions.h"


//--------------------------------------------------------------------------------
// class Region —— 矩形区域。
//--------------------------------------------------------------------------------
class Region
{
public:

  // region_modifier 枚举:Inside() 用 normal=整块判定;halfsize=向内缩 1/4 判定。
  enum region_modifier{halfsize, normal};
  
//--------------------------------------------------------------------------------
// 私有数据:四条边 m_dTop/Left/Right/Bottom、宽高、中心 m_vCenter、编号 m_iID。
//--------------------------------------------------------------------------------
protected:

  double        m_dTop;
  double        m_dLeft;
  double        m_dRight;
  double        m_dBottom;

  double        m_dWidth;
  double        m_dHeight;

  Vector2D     m_vCenter;
  
  int          m_iID;

public:

  Region():m_dTop(0),m_dBottom(0),m_dLeft(0),m_dRight(0)
  {}


  // 带参构造:给出左、上、右、下四边和编号;自动算中心与宽高(fabs 防负)。
  Region(double left,
         double top,
         double right,
         double bottom,
         int id = -1):m_dTop(top),
                        m_dRight(right),
                        m_dLeft(left),
                        m_dBottom(bottom),
                        m_iID(id)
  {
    //calculate center of region
    m_vCenter = Vector2D( (left+right)*0.5, (top+bottom)*0.5 );

    m_dWidth  = fabs(right-left);
    m_dHeight = fabs(bottom-top);
  }

  virtual ~Region(){}

  // 三个关键接口:Render 画矩形;Inside 判定点在不在区域内;GetRandomPosition 随机取点。
  virtual inline void     Render(bool ShowID)const;

  //returns true if the given position lays inside the region. The
  //region modifier can be used to contract the region bounderies
  inline bool     Inside(Vector2D pos, region_modifier r)const;

  //returns a vector representing a random location
  //within the region
  inline Vector2D GetRandomPosition()const;

  // 下面一堆访问器:Top/Bottom/Left/Right/Width/Height/Length(长边)/Breadth(短边)/Center/ID。
  //-------------------------------
  double     Top()const{return m_dTop;}
  double     Bottom()const{return m_dBottom;}
  double     Left()const{return m_dLeft;}
  double     Right()const{return m_dRight;}
  double     Width()const{return fabs(m_dRight - m_dLeft);}
  double     Height()const{return fabs(m_dTop - m_dBottom);}
  double     Length()const{return max(Width(), Height());}
  double     Breadth()const{return min(Width(), Height());}

  Vector2D  Center()const{return m_vCenter;}
  int       ID()const{return m_iID;}

};



  // GetRandomPosition:在左右、上下范围内各取一个随机数,组合成随机坐标。
inline Vector2D Region::GetRandomPosition()const
{
  return Vector2D(RandInRange(m_dLeft, m_dRight),
                   RandInRange(m_dTop, m_dBottom));
}

//--------------------------------------------------------------------------------
// Inside:normal 模式直接比四边;halfsize 模式先算出 1/4 边距 margin,
// 再判定点是否在"向内缩了一圈"的矩形里。
//--------------------------------------------------------------------------------
inline bool Region::Inside(Vector2D pos, region_modifier r=normal)const
{
  if (r == normal)
  {
    return ((pos.x > m_dLeft) && (pos.x < m_dRight) &&
         (pos.y > m_dTop) && (pos.y < m_dBottom));
  }
  else
  {
    const double marginX = m_dWidth * 0.25;
    const double marginY = m_dHeight * 0.25;

    return ((pos.x > (m_dLeft+marginX)) && (pos.x < (m_dRight-marginX)) &&
         (pos.y > (m_dTop+marginY)) && (pos.y < (m_dBottom-marginY)));
  }

}

  // Render:用空心绿笔画矩形;ShowID 为真时在中心印上区域编号。
inline void Region::Render(bool ShowID = 0)const
{
  gdi->HollowBrush();
  gdi->GreenPen();
  gdi->Rect(m_dLeft, m_dTop, m_dRight, m_dBottom);

  if (ShowID)
  { 
    gdi->TextColor(Cgdi::green);
    gdi->TextAtPos(Center(), ttos(ID()));
  }
}


#endif
