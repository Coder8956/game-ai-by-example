//==============================================================================================
//【文件说明】Weapon_ShotGun.h —— 霰弹枪(ShotGun)
//
//【这个文件是干什么的?】
//  ShotGun 是近程散射武器,继承武器基类 Raven_Weapon。
//  一次开火打出多颗 Pellet 弹丸,带散射角(Spread);用模糊逻辑按距离+弹药量评估用枪欲望。
//
//【谁在使用这个文件?】—— 地图上的霰弹枪弹药用点被拾取后,机器人获得 ShotGun。
//【本文件包含了谁?】—— Raven_Weapon.h(父类)。
#ifndef SHOTGUN_H
#define SHOTGUN_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Weapon_ShotGun.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to implement a shot gun
//-----------------------------------------------------------------------------
#include "Raven_Weapon.h"


class  Raven_Bot;



//--------------------------------------------------------------------------------
// class ShotGun : public Raven_Weapon:霰弹枪是一种武器。
class ShotGun : public Raven_Weapon
{
private:

// InitializeFuzzyModule:初始化模糊规则。
  void     InitializeFuzzyModule();

  //how much shot the each shell contains
// m_iNumBallsInShell:每发弹丸数。
//(原文注释:每发子弹含多少颗小弹丸)
  int      m_iNumBallsInShell;

  //how much the shot spreads out when a cartridge is discharged
// m_dSpread:散射角。
//(原文注释:开火时弹丸散射的角度大小)
  double    m_dSpread;

// public:对外接口。
public:

// 构造:owner=持枪机器人。
  ShotGun(Raven_Bot* owner);

// Render:画枪;ShootAt:开火;GetDesirability:评估用枪欲望。
  void  Render();

  void  ShootAt(Vector2D pos);

  double GetDesirability(double DistToTarget);
};



#endif