//==============================================================================================
//【文件说明】Projectile_Slug.h —— 轨道炮(RailGun)弹丸:Slug
//
//【这个文件是干什么的?】
//  Slug 是轨道炮射出的高速弹丸,继承抛射物基类 Raven_Projectile。
//  与 Pellet 几乎一样:命中后轨迹短暂可见,然后消失。区别在:轨道炮可能一发穿过多个目标。
//
//【谁在使用这个文件?】—— Weapon_RailGun(轨道炮)开火时 new 出一颗 Slug。
//【本文件包含了谁?】—— Raven_Projectile.h(父类)。
#ifndef SLUG_H
#define SLUG_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Slug.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to implement a railgun slug
//-----------------------------------------------------------------------------

#include "Raven_Projectile.h"

class Raven_Bot;
class Raven_Environment;

//--------------------------------------------------------------------------------
// class Slug : public Raven_Projectile:Slug 是一种抛射物。
class Slug : public Raven_Projectile
{
private:

  //when this projectile hits something it's trajectory is rendered
  //for this amount of time
// m_dTimeShotIsVisible:轨迹可见时长。
//(原文注释:子弹命中后,轨迹还要画这么长一段时间)
  double   m_dTimeShotIsVisible;

  //tests the trajectory of the shell for an impact
//(原文注释:检测弹丸弹道是否命中)—— TestForImpact。
  void  TestForImpact();

    //returns true if the shot is still to be rendered
// isVisibleToPlayer:当前时间 < 创建时刻+可见时长 → 仍可见。
//(原文注释:若这颗弹丸还该画出来就返回 true)
  bool  isVisibleToPlayer()const{return Clock->GetCurrentTime() < m_dTimeOfCreation + m_dTimeShotIsVisible;}
  
// public:对外接口。
public:

// 构造:shooter=开枪者,target=瞄准点。
  Slug(Raven_Bot* shooter, Vector2D target);
  
// Render:画弹丸;Update:每帧更新。
  void Render();

  void Update();
  
};


#endif