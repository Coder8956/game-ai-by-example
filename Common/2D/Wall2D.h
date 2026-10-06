//==============================================================================================
//【文件说明】Wall2D.h —— 2D"墙":一条线段 + 一个垂直法线
//
//【这个文件是干什么的?】
//  一道墙用线段 A→B 表示,再配一个垂直墙面的"法线" m_vN(指向墙的哪一侧)。
//  角色撞墙、反弹、判断在墙的哪一边,都靠它。球场四周、Raven 地图边界都是这种墙。
//
//【谁在使用这个文件?】
//  SoccerPitch.cpp —— 球场四周的墙(SoccerPitch.h 里 m_vecWalls 装一墙 Wall2D);
//  Raven_Game.cpp / 各游戏世界 —— 地图边界墙。
//
//【本文件包含了谁?】
//  misc/Cgdi.h —— 绘图单例 gdi;2d/Vector2D.h —— 向量;<fstream> —— 文件读写。
//==============================================================================================
#ifndef WALL_H
#define WALL_H
//------------------------------------------------------------------------
//
//  Name:   Wall2D.h
//
//  Desc:   class to create and render 2D walls. Defined as the two 
//          vectors A - B with a perpendicular normal. 
//          
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include "misc/Cgdi.h"
#include "2d/Vector2D.h"
#include <fstream>


// ↓↓↓ 原文翻译【Wall2D —— 墙】:创建并渲染 2D 墙的类。定义为两端点 A-B 再加一条
//   垂直法线。作者:Mat Buckland。
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// class Wall2D —— 墙类。protected(保护区)成员:子类可见、外面不可见。
class Wall2D 
{
protected:

  // 三个向量:m_vA=起点、m_vB=终点、m_vN=单位法线(垂直墙面、朝外)。
  Vector2D    m_vA,
              m_vB,
              m_vN;

  // CalculateNormal:由 A、B 算出法线。先把墙方向归一化,再左转 90°((x,y)→(-y,x))。
  void CalculateNormal()
  {
    Vector2D temp = Vec2DNormalize(m_vB - m_vA);

    m_vN.x = -temp.y;
    m_vN.y = temp.x;
  }

public:

  // 三个构造函数:① 空构造;② 给 A、B,自动算法线;③ A、B、N 全给;④ 从文件读。
  Wall2D(){}

  Wall2D(Vector2D A, Vector2D B):m_vA(A), m_vB(B)
  {
    CalculateNormal();
  }

  Wall2D(Vector2D A, Vector2D B, Vector2D N):m_vA(A), m_vB(B), m_vN(N)
  { }

  Wall2D(std::ifstream& in){Read(in);}

  // Render:画墙(virtual 虚函数,留待子类重画);可选顺带画一条法线小箭头。
  virtual void Render(bool RenderNormals = false)const
  {
    gdi->Line(m_vA, m_vB);

    //render the normals if rqd
    if (RenderNormals)
    {
      int MidX = (int)((m_vA.x+m_vB.x)/2);
      int MidY = (int)((m_vA.y+m_vB.y)/2);

      gdi->Line(MidX, MidY, (int)(MidX+(m_vN.x * 5)), (int)(MidY+(m_vN.y * 5)));
    }
  }

  // 一组访问器/设置器:From/To/Normal/Center。注意 SetFrom/SetTo 改端点后会重算法线。
  Vector2D From()const  {return m_vA;}
  void     SetFrom(Vector2D v){m_vA = v; CalculateNormal();}

  Vector2D To()const    {return m_vB;}
  void     SetTo(Vector2D v){m_vB = v; CalculateNormal();}
  
  Vector2D Normal()const{return m_vN;}
  void     SetNormal(Vector2D n){m_vN = n;}
  
  Vector2D Center()const{return (m_vA+m_vB)/2.0;}

  // Write/Read:把墙的三个点写入文件 / 从文件读回(存档用)。
  std::ostream& Wall2D::Write(std::ostream& os)const
  {
    os << std::endl;
    os << From() << ",";
    os << To() << ",";
    os << Normal();
    return os;
  }

 void Read(std::ifstream& in)
  {
    double x,y;

    in >> x >> y;
    SetFrom(Vector2D(x,y));

    in >> x >> y;
    SetTo(Vector2D(x,y));

     in >> x >> y;
    SetNormal(Vector2D(x,y));
  }
  
};

#endif