//==============================================================================================
//【文件说明】GraveMarkers.h —— 机器人死后的「墓碑」记录与绘制
//
//【这个文件是干什么的?】
//  一个机器人被打死时,在它死掉的地方画一个小墓碑(记录位置、出现时间),
//  过一会儿(寿命)自动消失。本类负责记这些墓碑、每帧更新(过期就删)、画出来。
//
//【谁在使用这个文件?】
//  Raven_Game.h/.cpp —— 游戏类持有一个 GraveMarkers,bot 死亡时 AddGrave,
//      每帧调 Update/Render。
//
//【本文件包含了谁?】
//  <list>/<vector>  —— 标准库容器;
//  2d/vector2d.h    —— 2D 向量;
//  time/crudetimer.h —— 简易计时器 Clock(全局单例)。
//==============================================================================================
#ifndef GRAVE_MARKERS_H
#define GRAVE_MARKERS_H
#pragma warning (disable : 4786)
//-----------------------------------------------------------------------------
//
//  Name:   GraveMarkers.h
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:   Class to record and render graves at the site of a bot's death
//(原文注释翻译:在机器人死亡地点记录并绘制墓碑的类)
//
//-----------------------------------------------------------------------------
#include <list>
#include <vector>
#include "2d/vector2d.h"
#include "time/crudetimer.h"

//--------------------------------------------------------------------------------
// class GraveMarkers —— 墓碑管理类。
class GraveMarkers
{
private:
  
//--------------------------------------------------------------------------------
// 嵌套结构 GraveRecord:一条墓碑记录=位置 + 出现时刻。
//   struct 和 class 几乎一样,只是 struct 默认 public。
//--------------------------------------------------------------------------------
  struct GraveRecord
  {
    Vector2D Position;
    double    TimeCreated;

    GraveRecord(Vector2D pos):Position(pos),
                              TimeCreated(Clock->GetCurrentTime())
    {}
  };

private:
  
  typedef std::list<GraveRecord> GraveList;

private:

  //how long a grave remains on screen
// m_dLifeTime:墓碑寿命(秒);超时就被 Update 删除。
//(原文注释:墓碑在屏幕上停留多久)
  double m_dLifeTime;

  //when a bot dies, a grave is rendered to mark the spot.
// m_vecRIPVB/RIPVBTrans:墓碑「RIP」字样的多边形顶点(原始/变换后);
// m_GraveList:所有现存墓碑的链表。
//(原文注释:bot 死时,画一个墓碑标记那个位置)
  std::vector<Vector2D>   m_vecRIPVB;
  std::vector<Vector2D>   m_vecRIPVBTrans;
  GraveList               m_GraveList;


public:

// 构造(给寿命)、Update(每帧清过期)、Render(画)、AddGrave(加新墓碑)。
  GraveMarkers(double lifetime);

  void Update();
  void Render();
  void AddGrave(Vector2D pos);

};

#endif