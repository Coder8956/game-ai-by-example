//==============================================================================================
//【文件说明】Projectile_Bolt.h —— 爆能枪(Blaster)子弹:Bolt
//
//【这个文件是干什么的?】
//  Bolt 是爆能枪射出的直线光束子弹,继承抛射物基类 Raven_Projectile。
//  它负责:每帧更新位置、检测是否撞到机器人或墙(命中即停并造成伤害)、画出自己。
//
//【谁在使用这个文件?】—— Weapon_Blaster(爆能枪)开火时 new 出一颗 Bolt。
//【本文件包含了谁?】—— Raven_Projectile.h(父类)。
#ifndef BOLT_H
#define BOLT_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Bolt.h
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:   class to implement a bolt type projectile
//
//-----------------------------------------------------------------------------
#include "Raven_Projectile.h"

class Raven_Bot;



//--------------------------------------------------------------------------------
// class Bolt : public Raven_Projectile:Bolt 是一种抛射物。
class Bolt : public Raven_Projectile
{
private:

  //tests the trajectory of the shell for an impact
//(原文注释:检测子弹弹道是否命中)—— TestForImpact 私有辅助(本类里未单独使用,更新时直接测)。
  void TestForImpact();
  
public:

// 构造:shooter=开枪者,target=瞄准点。
  Bolt(Raven_Bot* shooter, Vector2D target);
  
// Render:画子弹;Update:每帧更新(父类纯虚函数的实现)。
  void Render();

  void Update();
  
};


#endif