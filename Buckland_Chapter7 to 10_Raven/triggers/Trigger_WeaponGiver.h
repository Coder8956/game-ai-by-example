//==============================================================================================
//【文件说明】triggers\Trigger_WeaponGiver.h —— 武器包触发器
//
//【这个文件是干什么的?】
//  地图上一个武器箱:bot 跑上去就捡到一把指定武器(霰弹枪/ railgun / 火箭筒),
//  捡完等一阵刷新。
#ifndef WEAPON_GIVER_H
#define WEAPON_GIVER_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:     Trigger_WeaponGiver.h
//
//  Author:   Mat Buckland
//
//  Desc:     This trigger 'gives' the triggering bot a weapon of the
//            specified type 
//(原文注释翻译:给触发它的 bot 一把指定类型的武器)
//
//-----------------------------------------------------------------------------
#include "Triggers/Trigger_Respawning.h"
#include "../Raven_Bot.h"
#include <iosfwd>




//--------------------------------------------------------------------------------
// class Trigger_WeaponGiver —— 继承可重生触发器基类。
class Trigger_WeaponGiver : public Trigger_Respawning<Raven_Bot>
{
private:

  //vrtex buffers for rocket shape
// m_vecRLVB/RIPVBTrans:武器形状顶点。
//(原文注释:火箭形状的顶点缓冲)
  std::vector<Vector2D>         m_vecRLVB;
  std::vector<Vector2D>         m_vecRLVBTrans;
  
public:

  //this type of trigger is created when reading a map file
  Trigger_WeaponGiver(std::ifstream& datafile);

  //if triggered, this trigger will call the PickupWeapon method of the
  //bot. PickupWeapon will instantiate a weapon of the appropriate type.
// Try:给武器;Render:画武器图标;Read:从文件读。
//(原文注释翻译:触发时调 bot 的 PickupWeapon,new 对应武器)
  void Try(Raven_Bot*);
  
  //draws a symbol representing the weapon type at the trigger's location
  void Render();

  void Read (std::ifstream& is);
};




#endif