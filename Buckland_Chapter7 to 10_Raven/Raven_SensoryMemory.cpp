//==============================================================================================
//【文件说明】Raven_SensoryMemory.cpp —— 感官记忆的实现
//==============================================================================================
#include "Raven_SensoryMemory.h"
#include "Raven_Game.h"
#include "time/crudetimer.h"
#include "misc/cgdi.h"
#include "misc/Stream_Utility_Functions.h"

//------------------------------- ctor ----------------------------------------
//-----------------------------------------------------------------------------
// 构造函数:记下 owner 和记忆时长。
 Raven_SensoryMemory:: Raven_SensoryMemory(Raven_Bot* owner,
                                           double MemorySpan):m_pOwner(owner),
                                                            m_dMemorySpan(MemorySpan)
                                                          
{}

//--------------------- MakeNewRecordIfNotAlreadyPresent ----------------------

//--------------------------------------------------------------------------------
// MakeNewRecordIfNotAlreadyPresent:在 map 里查 pOpponent,没有就 new 一条空记录。
void Raven_SensoryMemory::MakeNewRecordIfNotAlreadyPresent(Raven_Bot* pOpponent)
{
  //else check to see if this Opponent already exists in the memory. If it doesn't,
  //create a new record
  if (m_MemoryMap.find(pOpponent) == m_MemoryMap.end())
  {
    m_MemoryMap[pOpponent] = MemoryRecord();
  }
}

//------------------------ RemoveBotFromMemory --------------------------------
//
//  this removes a bot's record from memory
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// RemoveBotFromMemory:从 map 里 erase 掉某个 bot 的记录。
void Raven_SensoryMemory::RemoveBotFromMemory(Raven_Bot* pBot)
{
  MemoryMap::iterator record = m_MemoryMap.find(pBot);
  
  if (record != m_MemoryMap.end())
  {
    m_MemoryMap.erase(record);
  }
}
  
//----------------------- UpdateWithSoundSource -------------------------------
//
// this updates the record for an individual opponent. Note, there is no need to
// test if the opponent is within the FOV because that test will be done when the
// UpdateVision method is called
//(原文注释翻译:更新某个对手的记录;不用测是否在视野内,那是 UpdateVision 的事)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// UpdateWithSoundSource:听到某个 bot 发出声音时,记一笔时间/位置;
//   如果彼此间无遮挡线(LOS),把 bShootable 设 true。
void Raven_SensoryMemory::UpdateWithSoundSource(Raven_Bot* pNoiseMaker)
{
  //make sure the bot being examined is not this bot
  if (m_pOwner != pNoiseMaker)
  {
    //if the bot is already part of the memory then update its data, else
    //create a new memory record and add it to the memory
    MakeNewRecordIfNotAlreadyPresent(pNoiseMaker);

    MemoryRecord& info = m_MemoryMap[pNoiseMaker];

    //test if there is LOS between bots 
    if (m_pOwner->GetWorld()->isLOSOkay(m_pOwner->Pos(), pNoiseMaker->Pos()))
    {
      info.bShootable = true;
      
     //record the position of the bot
      info.vLastSensedPosition = pNoiseMaker->Pos();
    }
    else
    {
      info.bShootable = false;
    }
    
    //record the time it was sensed
    info.fTimeLastSensed = (double)Clock->GetCurrentTime();
  }
}

//----------------------------- UpdateVision ----------------------------------
//
//  this method iterates through all the bots in the game world to test if
//  they are in the field of view. Each bot's memory record is updated
//  accordingly
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// UpdateVision:每帧遍历所有 bot,判断:有没有遮挡?在不在视野锥里?
//   在视野内就刷新时间/位置;刚进视野就记下 fTimeBecameVisible。
void Raven_SensoryMemory::UpdateVision()
{
  //for each bot in the world test to see if it is visible to the owner of
  //this class
  const std::list<Raven_Bot*>& bots = m_pOwner->GetWorld()->GetAllBots();
  std::list<Raven_Bot*>::const_iterator curBot;
  for (curBot = bots.begin(); curBot!=bots.end(); ++curBot)
  {
    //make sure the bot being examined is not this bot
    if (m_pOwner != *curBot)
    {
      //make sure it is part of the memory map
      MakeNewRecordIfNotAlreadyPresent(*curBot);

      //get a reference to this bot's data
      MemoryRecord& info = m_MemoryMap[*curBot];

      //test if there is LOS between bots 
      if (m_pOwner->GetWorld()->isLOSOkay(m_pOwner->Pos(), (*curBot)->Pos()))
      {
        info.bShootable = true;

              //test if the bot is within FOV
        if (isSecondInFOVOfFirst(m_pOwner->Pos(),
                                 m_pOwner->Facing(),
                                 (*curBot)->Pos(),
                                  m_pOwner->FieldOfView()))
        {
          info.fTimeLastSensed     = Clock->GetCurrentTime();
          info.vLastSensedPosition = (*curBot)->Pos();
          info.fTimeLastVisible    = Clock->GetCurrentTime();

          if (info.bWithinFOV == false)
          {
            info.bWithinFOV           = true;
            info.fTimeBecameVisible    = info.fTimeLastSensed;
          
          }
        }

        else
        {
          info.bWithinFOV = false;         
        }
      }

      else
      {
        info.bShootable = false;
        info.bWithinFOV = false;
      }
    }
  }//next bot
}


//------------------------ GetListOfRecentlySensedOpponents -------------------
//
//  returns a list of the bots that have been sensed recently
//(原文注释:返回最近感知过的 bot 列表)
//-----------------------------------------------------------------------------
std::list<Raven_Bot*> 
//--------------------------------------------------------------------------------
// GetListOfRecentlySensedOpponents:遍历记忆表,最近记忆时长内更新过的挑出来。
Raven_SensoryMemory::GetListOfRecentlySensedOpponents()const
{
  //this will store all the opponents the bot can remember
  std::list<Raven_Bot*> opponents;

  double CurrentTime = Clock->GetCurrentTime();

  MemoryMap::const_iterator curRecord = m_MemoryMap.begin();
  for (curRecord; curRecord!=m_MemoryMap.end(); ++curRecord)
  {
    //if this bot has been updated in the memory recently, add to list
    if ( (CurrentTime - curRecord->second.fTimeLastSensed) <= m_dMemorySpan)
    {
      opponents.push_back(curRecord->first);
    }
  }

  return opponents;
}

//----------------------------- isOpponentShootable --------------------------------
//
//  returns true if the bot given as a parameter can be shot (ie. its not
//  obscured by walls)
//(原文注释翻译:返回 true 表示这个 bot 没被墙挡着、可以射击)
//-----------------------------------------------------------------------------
// isOpponentShootable/isOpponentWithinFOV:查记忆表里的标志位。
bool Raven_SensoryMemory::isOpponentShootable(Raven_Bot* pOpponent)const
{
  MemoryMap::const_iterator it = m_MemoryMap.find(pOpponent);
 
  if (it != m_MemoryMap.end())
  {
    return it->second.bShootable;
  }

  return false;
}

//----------------------------- isOpponentWithinFOV --------------------------------
//
//  returns true if the bot given as a parameter is within FOV
//-----------------------------------------------------------------------------
bool  Raven_SensoryMemory::isOpponentWithinFOV(Raven_Bot* pOpponent)const
{
  MemoryMap::const_iterator it = m_MemoryMap.find(pOpponent);
 
  if (it != m_MemoryMap.end())
  {
    return it->second.bWithinFOV;
  }

  return false;
}

//---------------------------- GetLastRecordedPositionOfOpponent -------------------
//
//  returns the last recorded position of the bot
//-----------------------------------------------------------------------------
// GetLastRecordedPositionOfOpponent:返回最后位置;没记录就抛异常。
Vector2D  Raven_SensoryMemory::GetLastRecordedPositionOfOpponent(Raven_Bot* pOpponent)const
{
  MemoryMap::const_iterator it = m_MemoryMap.find(pOpponent);
 
  if (it != m_MemoryMap.end())
  {
    return it->second.vLastSensedPosition;
  }

  throw std::runtime_error("< Raven_SensoryMemory::GetLastRecordedPositionOfOpponent>: Attempting to get position of unrecorded bot");
}

//----------------------------- GetTimeOpponentHasBeenVisible ----------------------
//
//  returns the amount of time the given bot has been visible
//-----------------------------------------------------------------------------
// GetTimeOpponentHasBeenVisible:已看见多久=现在时间-刚进视野时间。
double  Raven_SensoryMemory::GetTimeOpponentHasBeenVisible(Raven_Bot* pOpponent)const
{
  MemoryMap::const_iterator it = m_MemoryMap.find(pOpponent);
 
  if (it != m_MemoryMap.end() && it->second.bWithinFOV)
  {
    return Clock->GetCurrentTime() - it->second.fTimeBecameVisible;
  }

  return 0;
}

//-------------------- GetTimeOpponentHasBeenOutOfView ------------------------
//
//  returns the amount of time the given opponent has remained out of view
//  returns a high value if opponent has never been seen or not present
//-----------------------------------------------------------------------------
// GetTimeOpponentHasBeenOutOfView:离开视野多久=现在时间-最后看见时间。
double Raven_SensoryMemory::GetTimeOpponentHasBeenOutOfView(Raven_Bot* pOpponent)const
{
  MemoryMap::const_iterator it = m_MemoryMap.find(pOpponent);
 
  if (it != m_MemoryMap.end())
  {
    return Clock->GetCurrentTime() - it->second.fTimeLastVisible;
  }

  return MaxDouble;
}

//------------------------ GetTimeSinceLastSensed ----------------------
//
//  returns the amount of time the given bot has been visible
//-----------------------------------------------------------------------------
// GetTimeSinceLastSensed:距上次感知过了多久。
double  Raven_SensoryMemory::GetTimeSinceLastSensed(Raven_Bot* pOpponent)const
{
  MemoryMap::const_iterator it = m_MemoryMap.find(pOpponent);
 
  if (it != m_MemoryMap.end() && it->second.bWithinFOV)
  {
    return Clock->GetCurrentTime() - it->second.fTimeLastSensed;
  }

  return 0;
}

//---------------------- RenderBoxesAroundRecentlySensed ----------------------
//
//  renders boxes around the opponents it has sensed recently.
//-----------------------------------------------------------------------------
// RenderBoxesAroundRecentlySensed:调试用,在最近感知过的敌人周围画橙框。
void  Raven_SensoryMemory::RenderBoxesAroundRecentlySensed()const
{
  std::list<Raven_Bot*> opponents = GetListOfRecentlySensedOpponents();
  std::list<Raven_Bot*>::const_iterator it;
  for (it = opponents.begin(); it != opponents.end(); ++it)
  {
    gdi->OrangePen();
    Vector2D p = (*it)->Pos();
    double   b = (*it)->BRadius();
      
    gdi->Line(p.x-b, p.y-b, p.x+b, p.y-b);
    gdi->Line(p.x+b, p.y-b, p.x+b, p.y+b);
    gdi->Line(p.x+b, p.y+b, p.x-b, p.y+b);
    gdi->Line(p.x-b, p.y+b, p.x-b, p.y-b);
  }

}