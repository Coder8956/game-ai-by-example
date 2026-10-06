//==============================================================================================
//【文件说明】Weapon_RailGun.h —— 轨道炮(RailGun)
//
//【这个文件是干什么的?】
//  RailGun 是远程狙击武器,继承武器基类 Raven_Weapon。
//  开火时射出高速 Slug(可穿多目标);子弹稀少,用模糊逻辑综合"距离+弹药量"评估用枪欲望。
//
//【谁在使用这个文件?】—— 地图上的轨道炮弹药点被拾取后,机器人获得 RailGun。
//【本文件包含了谁?】—— Raven_Weapon.h(父类)。
#ifndef RAILGUN_H
#define RAILGUN_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Weapon_RailGun.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to implement a rail gun
//-----------------------------------------------------------------------------
#include "Raven_Weapon.h"


class  Raven_Bot;



//--------------------------------------------------------------------------------
// class RailGun : public Raven_Weapon:RailGun 是一种武器。
class RailGun : public Raven_Weapon
{
private:

// InitializeFuzzyModule:初始化模糊规则。
  void  InitializeFuzzyModule();

// public:对外接口。
public:

// 构造:owner=持枪机器人。
  RailGun(Raven_Bot* owner);

// Render:画枪;ShootAt:开火;GetDesirability:评估用枪欲望。
  void  Render();

  void  ShootAt(Vector2D pos);

  double GetDesirability(double DistToTarget);
};



#endif