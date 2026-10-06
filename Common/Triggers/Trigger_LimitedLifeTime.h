//==============================================================================================
//【文件说明】Trigger_LimitedLifetime.h —— 有寿命的触发器(若干帧后自动消失)
//
//【这个文件是干什么的?】
//  继承 Trigger,加一个寿命计数器 m_iLifetime。每帧 Update 时 --,到 0 就把自己标记删除。
//  典型用法:地上临时出现的弹药包,过一段时间自动消失。
//==============================================================================================
#ifndef TRIGGER_LIMITEDLIFETIME_H
#define TRIGGER_LIMITEDLIFETIME_H
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786) 原理详见 SoccerPitch.h;包含保护原理详见 Goal.h。
//--------------------------------------------------------------------------------
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:     Trigger_LimitedLifetime.h
//
//  Author:   Mat Buckland
//
//  Desc:     defines a trigger that only remains in the game for a specified
//            number of update steps
//
//-----------------------------------------------------------------------------
#include "Trigger.h"


template <class entity_type>
  // 继承 Trigger;构造时自动取新 ID;Update 里寿命减到 0 就 SetToBeRemovedFromGame。
class Trigger_LimitedLifetime : public Trigger<entity_type>
{
protected:

  //the lifetime of this trigger in update-steps
  int m_iLifetime;

public:

  Trigger_LimitedLifetime(int lifetime):Trigger<entity_type>(BaseGameEntity::GetNextValidID()),
                                        m_iLifetime(lifetime)
  {}

  virtual ~Trigger_LimitedLifetime(){}

  //children of this class should always make sure this is called from within
  //their own update method
  // Update:--m_iLifetime,<=0 时标记删除;Try 仍是纯虚,由更具体子类实现。
  virtual void Update()
  {
    //if the lifetime counter expires set this trigger to be removed from
    //the game
    if (--m_iLifetime <= 0)
    {
      SetToBeRemovedFromGame();
    }
  }

  //to be implemented by child classes
  virtual void  Try(entity_type*) = 0;
};




#endif