//==============================================================================================
//【文件说明】Projectile_Pellet.h —— 霰弹枪(ShotGun)弹丸:Pellet
//
//【这个文件是干什么的?】
//  Pellet 是霰弹枪打出的弹丸,继承抛射物基类 Raven_Projectile。
//  与 Bolt 不同:它命中后还会在屏幕上短暂留一段轨迹(画一瞬),然后才消失。
//
//【谁在使用这个文件?】—— Weapon_ShotGun(霰弹枪)开火时一次射出多颗 Pellet。
//【本文件包含了谁?】—— Raven_Projectile.h(父类)。
#ifndef PELLET_H
#define PELLET_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Pellet.h
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:   class to implement a pellet type projectile
//
//-----------------------------------------------------------------------------

#include "Raven_Projectile.h"

class Raven_Bot;
class Raven_Environment;

//--------------------------------------------------------------------------------
// class Pellet : public Raven_Projectile:Pellet 是一种抛射物。
class Pellet : public Raven_Projectile
{
private:

  //when this projectile hits something it's trajectory is rendered
  //for this amount of time
// m_dTimeShotIsVisible:轨迹可见时长。
//(原文注释:子弹命中后,轨迹还要画这么长一段时间)
  double   m_dTimeShotIsVisible;

  //tests the trajectory of the pellet for an impact
//(原文注释:检测弹丸弹道是否命中)—— TestForImpact 私有函数。
  void  TestForImpact();

  //returns true if the shot is still to be rendered
// isVisibleToPlayer:是否仍可见。
//(原文注释:若这颗弹丸还该画出来就返回 true)—— 当前时间 < 创建时刻+可见时长。
  bool  isVisibleToPlayer()const{return Clock->GetCurrentTime() < m_dTimeOfCreation + m_dTimeShotIsVisible;}
  
// public:对外接口。
public:

// 构造:shooter=开枪者,target=瞄准点。
  Pellet(Raven_Bot* shooter, Vector2D target);
  
// Render:画弹丸;Update:每帧更新。
  void Render();

  void Update();
  
};


#endif