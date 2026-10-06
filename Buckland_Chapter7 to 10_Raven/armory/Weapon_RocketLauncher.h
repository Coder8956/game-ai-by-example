//==============================================================================================
//【文件说明】Weapon_RocketLauncher.h —— 火箭发射器(RocketLauncher)
//
//【这个文件是干什么的?】
//  RocketLauncher 是范围杀伤武器,继承武器基类 Raven_Weapon。
//  开火时射出 Rocket(命中后爆炸,范围伤害);用模糊逻辑按距离+弹药量评估用枪欲望。
//
//【谁在使用这个文件?】—— 地图上的火箭弹药用点被拾取后,机器人获得 RocketLauncher。
//【本文件包含了谁?】—— Raven_Weapon.h(父类)。
#ifndef ROCKETLAUNCHER_H
#define ROCKETLAUNCHER_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   RocketLauncher
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to implement a rocket launcher
//-----------------------------------------------------------------------------
#include "Raven_Weapon.h"



class  Raven_Bot;

//--------------------------------------------------------------------------------
// class RocketLauncher : public Raven_Weapon:火箭发射器是一种武器。
class RocketLauncher : public Raven_Weapon
{
private:

// InitializeFuzzyModule:初始化模糊规则。
  void     InitializeFuzzyModule();

// public:对外接口。
public:

// 构造:owner=持枪机器人。
  RocketLauncher(Raven_Bot* owner);


// Render:画枪;ShootAt:开火;GetDesirability:评估用枪欲望。
  void Render();

  void ShootAt(Vector2D pos);

  double GetDesirability(double DistToTarget);
};



#endif