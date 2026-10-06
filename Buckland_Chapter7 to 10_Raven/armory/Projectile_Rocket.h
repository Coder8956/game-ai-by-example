//==============================================================================================
//【文件说明】Projectile_Rocket.h —— 火箭发射器(RocketLauncher)子弹:Rocket
//
//【这个文件是干什么的?】
//  Rocket 是火箭发射器射出的火箭,继承抛射物基类 Raven_Projectile。
//  特点:命中后会爆炸——以命中点为中心,在爆炸半径内对所有机器人造成范围伤害;
//  并在屏幕上画一个逐渐扩大的爆炸光圈,光圈扩到爆炸半径后消失。
#ifndef ROCKET_H
#define ROCKET_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Rocket.h
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:   class to implement a rocket
//
//-----------------------------------------------------------------------------

#include "Raven_Projectile.h"

class Raven_Bot;

//--------------------------------------------------------------------------------
// class Rocket : public Raven_Projectile:Rocket 是一种抛射物。
class Rocket : public Raven_Projectile
{
private:

  //the radius of damage, once the rocket has impacted
// m_dBlastRadius:爆炸半径。
//(原文注释:火箭命中后的伤害(爆炸)半径)
  double    m_dBlastRadius;

  //this is used to render the splash when the rocket impacts
// m_dCurrentBlastRadius:当前已画到的爆炸圈半径。
//(原文注释:画爆炸光圈用的当前半径(随时间扩大))
  double    m_dCurrentBlastRadius;

  //If the rocket has impacted we test all bots to see if they are within the 
  //blast radius and reduce their health accordingly
// InflictDamageOnBotsWithinBlastRadius:对爆炸范围内的机器人造成范围伤害。
//(原文注释:火箭命中后,检查所有机器人是否在爆炸半径内,据此扣血)
  void InflictDamageOnBotsWithinBlastRadius();

    //tests the trajectory of the shell for an impact
//(原文注释:检测弹道是否命中)—— TestForImpact。
  void TestForImpact();

// public:对外接口。
public:

// 构造:shooter=开枪者,target=瞄准点。
  Rocket(Raven_Bot* shooter, Vector2D target);
  
// Render:画火箭与爆炸圈;Update:每帧更新。
  void Render();

  void Update();
  
};


#endif