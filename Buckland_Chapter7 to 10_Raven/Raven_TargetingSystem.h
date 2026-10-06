//==============================================================================================
//【文件说明】Raven_TargetingSystem.h —— 机器人的「选目标」子系统
//
//【这个文件是干什么的?】
//  机器人的感官记忆(SensoryMemory)里存着「最近见过哪些敌人」。
//  本系统每帧从这些敌人里挑一个最近的当当前目标,并回答一系列问题:
//  目标在不在视野里?能不能打?上次见在哪?被看见多久了?等等。
//
//【谁在使用这个文件?】
//  Raven_Bot.h/.cpp —— 每个 bot 持有一个 TargetingSystem,战斗决策时问它。
//==============================================================================================
#ifndef TARGETING_SYSTEM_H
#define TARGETING_SYSTEM_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Raven_TargetingSystem.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to select a target from the opponents currently in a bot's
//          perceptive memory.
//(原文注释翻译:从机器人感官记忆里的对手中挑一个当目标的类)
//-----------------------------------------------------------------------------
#include "2d/Vector2D.h"
#include <list>


class Raven_Bot;




//--------------------------------------------------------------------------------
// class Raven_TargetingSystem —— 选目标子系统。
class Raven_TargetingSystem
{
private:

  //the owner of this system
// m_pOwner:持有本系统的 bot。
//(原文注释:本系统的拥有者)
  Raven_Bot*  m_pOwner;

  //the current target (this will be null if there is no target assigned)
// m_pCurrentTarget:当前锁定的目标 bot。
//(原文注释:当前目标;没目标时为 NULL)
  Raven_Bot*  m_pCurrentTarget;


public:

  Raven_TargetingSystem(Raven_Bot* owner);

  //each time this method is called the opponents in the owner's sensory 
  //memory are examined and the closest  is assigned to m_pCurrentTarget.
  //if there are no opponents that have had their memory records updated
  //within the memory span of the owner then the current target is set
  //to null
// Update:每帧重新选最近目标。
//(原文注释翻译:每次调用 Update,检查感官记忆里的对手,最近的那个设为当前目标;
//           如果记忆里没有最近刷新过的对手,就把当前目标置空。)
  void       Update();

  //returns true if there is a currently assigned target
//(原文注释:返回 true 表示当前有目标)
  bool       isTargetPresent()const{return m_pCurrentTarget != 0;}

  //returns true if the target is within the field of view of the owner
//(原文注释:目标在拥有者视野内返回 true)
  bool       isTargetWithinFOV()const;

  //returns true if there is unobstructed line of sight between the target
  //and the owner
//(原文注释翻译:目标和拥有者之间无遮挡视线返回 true)
  bool       isTargetShootable()const;

  //returns the position the target was last seen. Throws an exception if
  //there is no target currently assigned
//(原文注释翻译:返回目标最后被看见的位置;没目标时抛异常)
  Vector2D   GetLastRecordedPosition()const;

  //returns the amount of time the target has been in the field of view
//(原文注释:返回目标在视野里待了多久)
  double      GetTimeTargetHasBeenVisible()const;

  //returns the amount of time the target has been out of view
//(原文注释:返回目标离开视野多久了)
  double      GetTimeTargetHasBeenOutOfView()const;
  
  //returns a pointer to the target. null if no target current.
//(原文注释:返回目标指针;没目标返回 NULL)
  Raven_Bot* GetTarget()const{return m_pCurrentTarget;}

  //sets the target pointer to null
//(原文注释:把目标指针置空)
  void       ClearTarget(){m_pCurrentTarget=0;}
};




#endif
