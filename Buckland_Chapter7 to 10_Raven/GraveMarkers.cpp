//==============================================================================================
//【文件说明】GraveMarkers.cpp —— 墓碑管理类的实现
//==============================================================================================
#include "GraveMarkers.h"
#include "misc/cgdi.h"
#include "2D/Transformations.h"

//------------------------------- ctor ----------------------------------------
//-----------------------------------------------------------------------------
// 构造函数:记下寿命,并把墓碑多边形(RIP 字样轮廓)的 9 个顶点存进顶点缓冲。
GraveMarkers::GraveMarkers(double lifetime):m_dLifeTime(lifetime)
{
      //create the vertex buffer for the graves
// 9 个顶点连成墓碑轮廓(8 个点画形状 + 第 9 个闭合回起点)。
//(原文注释:创建墓碑的顶点缓冲)
    const int NumripVerts = 9;
    const Vector2D rip[NumripVerts] = {Vector2D(-4, -5),
                                       Vector2D(-4, 3),
                                       Vector2D(-3, 5),
                                       Vector2D(-1, 6),
                                       Vector2D(1, 6),
                                       Vector2D(3, 5),
                                       Vector2D(4, 3),
                                       Vector2D(4, -5),
                                       Vector2D(-4, -5)};
  for (int i=0; i<NumripVerts; ++i)
  {
    m_vecRIPVB.push_back(rip[i]);
  }
}


//--------------------------------------------------------------------------------
// Update:遍历墓碑链表,超时的 erase 删掉,没超时的 it++ 下一个。
void GraveMarkers::Update()
{
  GraveList::iterator it = m_GraveList.begin();
  while (it != m_GraveList.end())
  {
    if (Clock->GetCurrentTime() - it->TimeCreated > m_dLifeTime)
    {
      it = m_GraveList.erase(it);
    }
    else
    {
      ++it;
    }
  }
}
    

//--------------------------------------------------------------------------------
// Render:对每块墓碑,把多边形顶点用 WorldTransform 搬到该位置,再用 gdi 画出来,
//   并在上方写 RIP 三个字。
void GraveMarkers::Render()
{
  GraveList::iterator it = m_GraveList.begin();
  Vector2D facing(-1,0);
  for (it; it != m_GraveList.end(); ++it)
  {
    
    m_vecRIPVBTrans = WorldTransform(m_vecRIPVB,
                                   it->Position,
                                   facing,
                                   facing.Perp(),
                                   Vector2D(1,1));

    gdi->BrownPen();
    gdi->ClosedShape(m_vecRIPVBTrans);
    gdi->TextColor(133,90,0);
    gdi->TextAtPos(it->Position.x - 10, it->Position.y - 5, "RIP");
  }
}

// AddGrave:在指定位置 push_back 一条新墓碑记录(自动记下当前时间)。
void GraveMarkers::AddGrave(Vector2D pos)
{
  m_GraveList.push_back(GraveRecord(pos));
}