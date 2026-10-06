//==============================================================================================
//【文件说明】Weapon_Blaster.h —— 爆能枪(Blaster)
//
//【这个文件是干什么的?】
//  Blaster 是机器人的默认武器,继承武器基类 Raven_Weapon。
//  开火时射出 Bolt(光束子弹);用模糊逻辑评估"当前距离下该不该用这把枪"。
//
//【谁在使用这个文件?】—— 机器人自带一把 Blaster;Raven_WeaponSystem 持有武器列表。
//【本文件包含了谁?】—— Raven_Weapon.h(父类)。
#ifndef BLASTER_H
#define BLASTER_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Weapon_Blaster.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   
//-----------------------------------------------------------------------------
#include "Raven_Weapon.h"


class  Raven_Bot;



//--------------------------------------------------------------------------------
// class Blaster : public Raven_Weapon:Blaster 是一种武器。
class Blaster : public Raven_Weapon
{
private:

// InitializeFuzzyModule:初始化模糊规则(父类纯虚函数的实现)。
  void  InitializeFuzzyModule();
  
// public:对外接口。
public:

// 构造:owner=持枪机器人。
  Blaster(Raven_Bot*   owner);


// Render:画枪;ShootAt:开火;GetDesirability:评估用枪欲望。
  void  Render();

  void  ShootAt(Vector2D pos);

  double GetDesirability(double DistToTarget);
};



#endif