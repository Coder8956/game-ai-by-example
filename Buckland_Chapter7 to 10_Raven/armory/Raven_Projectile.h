//==============================================================================================
//【文件说明】Raven_Projectile.h —— 子弹/抛射物的基类
//
//【这个文件是干什么的?】
//  武器开火时会造出一颗"抛射物"(子弹)。Raven 有四种:轨道炮弹(Slug)、
//  霰弹枪弹丸(Pellet)、火箭(Rocket)、爆能枪弹(Bolt)。它们都继承本基类。
//  本类记录:谁开的枪、瞄准点、出发位置、伤害值、是否命中/是否死亡等;
//  Update()/Render() 是纯虚函数,由各种子弹自己实现。
//
//【谁在使用这个文件?】—— 四种子弹类(Projectile_Bolt/Pellet/Rocket/Slug)都继承它;
//   Raven_WeaponSystem 开火时 new 出具体子弹。
//【本文件包含了谁?】—— game/MovingEntity.h(移动物体基类)、2d/Vector2D.h、
//   time/CrudeTimer.h、<list>(链表容器)。
//==============================================================================================
#ifndef PROJECTILE_H
#define PROJECTILE_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Projectile.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   Base class to define a projectile type. A projectile of the correct
//          type is created whnever a weapon is fired. In Raven there are four
//          types of projectile: Slugs (railgun), Pellets (shotgun), Rockets
//          (rocket launcher ) and Bolts (Blaster) 
//-----------------------------------------------------------------------------
#include "game/MovingEntity.h"
#include "2d/Vector2D.h"
#include "time/CrudeTimer.h"
#include <list>

// 前置声明:Raven_Game(世界数据)、Raven_Bot(机器人)在别处定义。
class Raven_Game;
class Raven_Bot;


//--------------------------------------------------------------------------------
// class Raven_Projectile : public MovingEntity:抛射物是一种"移动物体"。
class Raven_Projectile : public MovingEntity
{
protected:

  //the ID of the entity that fired this
// m_iShooterID:开枪者编号(避免子弹打中开枪者自己)。
//(原文注释:发射这颗子弹的实体 ID)
  int           m_iShooterID;

  //the place the projectile is aimed at
// m_vTarget:瞄准点坐标。
//(原文注释:子弹瞄准的位置)
  Vector2D      m_vTarget;

  //a pointer to the world data
//(原文注释:指向世界数据的指针)
// m_pWorld:世界对象(查所有机器人用)。
  Raven_Game*   m_pWorld;

  //where the projectile was fired from
// m_vOrigin:发射原点。
//(原文注释:子弹射出的位置)
  Vector2D      m_vOrigin;

  //how much damage the projectile inflicts
// m_iDamageInflicted:伤害值。
//(原文注释:子弹造成的伤害)
  int           m_iDamageInflicted;

  //is it dead? A dead projectile is one that has come to the end of its
  //trajectory and cycled through any explosion sequence. A dead projectile
  //can be removed from the world environment and deleted.
// m_bDead:是否已死亡(可删除)。
//(原文注释:死子弹=飞完轨迹并播完爆炸动画;可从世界删除)
  bool          m_bDead;

  //this is set to true as soon as a projectile hits something
// m_bImpacted:是否已命中(但可能还在播爆炸动画,未死)。
//(原文注释:子弹一撞到东西就置 true)
  bool          m_bImpacted;

  //the position where this projectile impacts an object
// m_vImpactPoint:命中点。
//(原文注释:子弹撞上物体的位置)
  Vector2D      m_vImpactPoint;

  //this is stamped with the time this projectile was instantiated. This is
  //to enable the shot to be rendered for a specific length of time
// m_dTimeOfCreation:创建时刻。
//(原文注释:记录子弹创建时刻,用于让它在屏幕上画固定时长)
  double       m_dTimeOfCreation;

// 两个辅助函数:找"弹道线撞到的最近机器人" / "撞到的所有机器人列表"。
  Raven_Bot*            GetClosestIntersectingBot(Vector2D From,
                                                  Vector2D To)const;

  std::list<Raven_Bot*> GetListOfIntersectingBots(Vector2D From,
                                                  Vector2D To)const;


// public:对外接口。
public:

// 构造函数:参数很多(目标点/世界/开枪者ID/原点/朝向/伤害/缩放/最大速度/质量/最大力)。
// 初始化列表先调父类 MovingEntity 的构造,再存自己的成员;m_dTimeOfCreation 在花括号里记时间。
  Raven_Projectile(Vector2D  target,   //the target's position
                   Raven_Game* world,  //a pointer to the world data
                   int      ShooterID, //the ID of the bot that fired this shot
                   Vector2D origin,  //the start position of the projectile
                   Vector2D heading,   //the heading of the projectile
                   int      damage,    //how much damage it inflicts
                   double    scale,    
                   double    MaxSpeed, 
                   double    mass,
                   double    MaxForce):  MovingEntity(origin,
                                                     scale,
                                                     Vector2D(0,0),
                                                     MaxSpeed,
                                                     heading,
                                                     mass,
                                                     Vector2D(scale, scale),
                                                     0, //max turn rate irrelevant here, all shots go straight
                                                     MaxForce),

                                        m_vTarget(target),
                                        m_bDead(false),
                                        m_bImpacted(false),
                                        m_pWorld(world),
                                        m_iDamageInflicted(damage),
                                        m_vOrigin(origin),
                                        m_iShooterID(ShooterID)
                

  {m_dTimeOfCreation = Clock->GetCurrentTime();}

  //unimportant for this class unless you want to implement a full state 
  //save/restore (which can be useful for debugging purposes)
// Write/Read:存档/读档接口(此处空实现)。
//(原文注释:写档/读档;本类留空 {}，除非要做完整存档调试)
  void Write(std::ostream&  os)const{}
  void Read (std::ifstream& is){}

  //must be implemented
// 纯虚函数:各种子弹自己实现"每帧更新"和"画出来"。
//(原文注释:必须由子类实现)—— Update()/Render() 纯虚函数(=0)。
  virtual void Update() = 0;
  virtual void Render() = 0;
  
  //set to true if the projectile has impacted and has finished any explosion 
  //sequence. When true the projectile will be removed from the game
// isDead:是否可删除;HasImpacted:是否已命中。
//(原文注释:命中且播完爆炸动画后置 true,此时子弹从游戏删除)
  bool isDead()const{return m_bDead;}
  
  //true if the projectile has impacted but is not yet dead (because it
  //may be exploding outwards from the point of impact for example)
  bool HasImpacted()const{return m_bImpacted;}



};






#endif