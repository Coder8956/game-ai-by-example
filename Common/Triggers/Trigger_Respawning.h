//==============================================================================================
//【文件说明】Trigger_Respawning.h —— 可重生的触发器(触发后冷却一段时间再激活)
//
//【这个文件是干什么的?】
//  继承 Trigger,加两个冷却计数:
//    m_iNumUpdatesBetweenRespawns     —— 冷却多少帧后复活;
//    m_iNumUpdatesRemainingUntilRespawn —— 还剩多少帧。
//  被触发时 Deactivate() 关闭并开始倒计时;Update 每帧倒计时,到 0 时 SetActive。
//  典型用法:回血包/弹药包,捡走后过几秒重生。
//==============================================================================================
#ifndef Trigger_Respawning_H
#define Trigger_Respawning_H
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786) 原理详见 SoccerPitch.h;包含保护原理详见 Goal.h。
//--------------------------------------------------------------------------------
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:     Trigger_Respawning.h
//
//  Author:   Mat Buckland
//
//  Desc:     base class to create a trigger that is capable of respawning
//            after a period of inactivity
//
//-----------------------------------------------------------------------------
#include "Trigger.h"
#include <iosfwd>
#include <cassert>


template <class entity_type>
  // 继承 Trigger;Deactivate 关触发器并开始冷却;Update 倒计时到 0 重新激活。
class Trigger_Respawning : public Trigger<entity_type>
{
protected:

  //When a bot comes within this trigger's area of influence it is triggered
  //but then becomes inactive for a specified amount of time. These values
  //control the amount of time required to pass before the trigger becomes 
  //active once more.
  int   m_iNumUpdatesBetweenRespawns;
  int   m_iNumUpdatesRemainingUntilRespawn;

  //sets the trigger to be inactive for m_iNumUpdatesBetweenRespawns 
  //update-steps
  void Deactivate()
  {
    SetInactive();
    m_iNumUpdatesRemainingUntilRespawn = m_iNumUpdatesBetweenRespawns;
  }

public:

  Trigger_Respawning(int id):Trigger<entity_type>(id),
                             m_iNumUpdatesBetweenRespawns(0),
                             m_iNumUpdatesRemainingUntilRespawn(0)
  {}

  virtual ~Trigger_Respawning(){}

  //to be implemented by child classes
  virtual void  Try(entity_type*) = 0;

  //this is called each game-tick to update the trigger's internal state
  virtual void Update()
  {
    if ( (--m_iNumUpdatesRemainingUntilRespawn <= 0) && !isActive())
    {
      SetActive();
    }
  }
  
  // SetRespawnDelay:设置冷却帧数。
  void SetRespawnDelay(unsigned int numTicks)
  {m_iNumUpdatesBetweenRespawns = numTicks;}
};




#endif