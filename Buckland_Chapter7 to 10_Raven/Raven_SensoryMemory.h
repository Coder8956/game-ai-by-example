//==============================================================================================
//【文件说明】Raven_SensoryMemory.h —— 机器人的「感官记忆」
//
//【这个文件是干什么的?】
//  机器人不是全知的:它只能看见视野锥里的东西、听见周围的声音。
//  但它有「记忆」:即使敌人跑出视野,它还记得「多久前在哪见过它」。
//  本文件两个类:
//    MemoryRecord       —— 对某个敌人的一条记忆(上次见的时间/位置/能否打等);
//    Raven_SensoryMemory—— 一个 bot 的全部记忆表(map<敌人, 记录>),
//                          每帧 UpdateVision 更新视野、UpdateWithSoundSource 记录声音。
//
//【谁在使用这个文件?】
//  Raven_Bot.h/.cpp —— 持有一个 SensoryMemory;
//  Raven_TargetingSystem.cpp —— 问它「最近见过哪些敌人」。
#ifndef RAVEN_SENSORY_SYSTEM_H
#define RAVEN_SENSORY_SYSTEM_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:
//
//-----------------------------------------------------------------------------
#include <map>
#include <list>
#include "2d/vector2d.h"

class Raven_Bot;


//--------------------------------------------------------------------------------
// class MemoryRecord —— 对一个敌人的记忆记录。
class MemoryRecord
{
public:
  
  //records the time the opponent was last sensed (seen or heard). This
  //is used to determine if a bot can 'remember' this record or not. 
  //(if CurrentTime() - m_dTimeLastSensed is greater than the bot's
  //memory span, the data in this record is made unavailable to clients)
// fTimeLastSensed:上次感知时间。
//(原文注释翻译:记录上次感知(看见或听见)这个对手的时间;
//           当前时间-它 超过记忆时长,这条记录就视为失效。)
  double       fTimeLastSensed;

  //it can be useful to know how long an opponent has been visible. This 
  //variable is tagged with the current time whenever an opponent first becomes
  //visible. It's then a simple matter to calculate how long the opponent has
  //been in view (CurrentTime - fTimeBecameVisible)
// fTimeBecameVisible:刚进入视野的时间。
//(原文注释翻译:记录对手刚进入视野的时间;当前时间减它=已看见多久)
  double       fTimeBecameVisible;

  //it can also be useful to know the last time an opponent was seen
// fTimeLastVisible:最后一次被看见的时间。
//(原文注释:记下对手最后一次被看见的时间,也很有用)
  double       fTimeLastVisible;

  //a vector marking the position where the opponent was last sensed. This can
  // be used to help hunt down an opponent if it goes out of view
// vLastSensedPosition:最后感知到的位置。
//(原文注释翻译:记录对手最后被感知时的位置;对手跑出视野时,靠这个位置追它)
  Vector2D    vLastSensedPosition;

  //set to true if opponent is within the field of view of the owner
// bWithinFOV:在视野内吗?bShootable:能打吗?
//(原文注释:对手在拥有者视野内置 true)
  bool        bWithinFOV;

  //set to true if there is no obstruction between the opponent and the owner, 
  //permitting a shot.
  bool        bShootable;
  

  MemoryRecord():fTimeLastSensed(-999),
            fTimeBecameVisible(-999),
            fTimeLastVisible(0),
            bWithinFOV(false),
            bShootable(false)
  {}
};



//--------------------------------------------------------------------------------
// class Raven_SensoryMemory —— 感官记忆管理器。
class Raven_SensoryMemory
{
private:

  typedef std::map<Raven_Bot*, MemoryRecord> MemoryMap;

private:
  
  //the owner of this instance
  Raven_Bot* m_pOwner;

  //this container is used to simulate memory of sensory events. A MemoryRecord
  //is created for each opponent in the environment. Each record is updated 
  //whenever the opponent is encountered. (when it is seen or heard)
// m_MemoryMap:记忆表,key=敌人指针,value=记忆记录。
//(原文注释翻译:每个对手一条 MemoryRecord;每次看见/听见它就更新这条记录)
  MemoryMap  m_MemoryMap;

  //a bot has a memory span equivalent to this value. When a bot requests a 
  //list of all recently sensed opponents this value is used to determine if 
  //the bot is able to remember an opponent or not.
// m_dMemorySpan:记忆时长(秒)。
//(原文注释翻译:记忆时长;超过这个秒数没更新的记录视为遗忘)
  double      m_dMemorySpan;

  //this methods checks to see if there is an existing record for pBot. If
  //not a new MemoryRecord record is made and added to the memory map.(called
  //by UpdateWithSoundSource & UpdateVision)
// MakeNewRecordIfNotAlreadyPresent:没记录就新建。
//(原文注释翻译:查一下 pBot 有没有记录,没有就新建一条加进表)
  void       MakeNewRecordIfNotAlreadyPresent(Raven_Bot* pBot);

public:

  Raven_SensoryMemory(Raven_Bot* owner, double MemorySpan);

  //this method is used to update the memory map whenever an opponent makes
  //a noise
// UpdateWithSoundSource:听到敌人脚步声/枪声时记一笔。
//(原文注释翻译:对手发出声响时,用它更新记忆表)
  void     UpdateWithSoundSource(Raven_Bot* pNoiseMaker);

  //this removes a bot's record from memory
//(原文注释:从记忆里删掉某个 bot 的记录)
  void     RemoveBotFromMemory(Raven_Bot* pBot);

  //this method iterates through all the opponents in the game world and 
  //updates the records of those that are in the owner's FOV
// UpdateVision:每帧刷新视野内的敌人。
//(原文注释翻译:遍历世界里所有对手,把在视野内的那些更新记录)
  void     UpdateVision();

  bool     isOpponentShootable(Raven_Bot* pOpponent)const;
  bool     isOpponentWithinFOV(Raven_Bot* pOpponent)const;
  Vector2D GetLastRecordedPositionOfOpponent(Raven_Bot* pOpponent)const;
  double    GetTimeOpponentHasBeenVisible(Raven_Bot* pOpponent)const;
  double    GetTimeSinceLastSensed(Raven_Bot* pOpponent)const;
  double    GetTimeOpponentHasBeenOutOfView(Raven_Bot* pOpponent)const;

  //this method returns a list of all the opponents that have had their
  //records updated within the last m_dMemorySpan seconds.
// GetListOfRecentlySensedOpponents:取出「还在记忆里的敌人」列表。
//(原文注释翻译:返回最近记忆时长内更新过的所有敌人列表)
  std::list<Raven_Bot*> GetListOfRecentlySensedOpponents()const;

  void     RenderBoxesAroundRecentlySensed()const;

};


#endif