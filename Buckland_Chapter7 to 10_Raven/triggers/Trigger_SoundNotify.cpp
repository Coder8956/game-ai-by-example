//==============================================================================================
//【文件说明】triggers\Trigger_SoundNotify.cpp —— 声音通知触发器的实现
//==============================================================================================
#include "Trigger_SoundNotify.h"
#include "Triggers/TriggerRegion.h"
#include "../Raven_Game.h"
#include "../lua/Raven_Scriptor.h"
#include "../constants.h"
#include "Messaging/MessageDispatcher.h"
#include "../Raven_Messages.h"

#include "misc/cgdi.h"

//------------------------------ ctor -----------------------------------------
//-----------------------------------------------------------------------------

// 构造函数:记下发声者,位置=发声者位置,半径=range,建圆形触发区。
Trigger_SoundNotify::Trigger_SoundNotify(Raven_Bot* source,
                                     double      range):Trigger_LimitedLifetime<Raven_Bot>(FrameRate /script->GetInt("Bot_TriggerUpdateFreq")),
                                                       m_pSoundSource(source)
{
  //set position and range
  SetPos(m_pSoundSource->Pos());

  SetBRadius(range);

  //create and set this trigger's region of fluence
  AddCircularTriggerRegion(Pos(), BRadius());
}


//------------------------------ Try ------------------------------------------
//
//  when triggered this trigger adds the bot that made the source of the sound 
//  to the triggering bot's perception.
//(原文注释翻译:触发时把发声者加进触发它的那个 bot 的感知里)
//-----------------------------------------------------------------------------
// Try:pBot 在声音范围内?在就给它发 Msg_GunshotSound 消息,带上发声者指针。
void Trigger_SoundNotify::Try(Raven_Bot* pBot)
{
  //is this bot within range of this sound
  if (isTouchingTrigger(pBot->Pos(), pBot->BRadius()))
  {
    Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                            SENDER_ID_IRRELEVANT,
                            pBot->ID(),
                            Msg_GunshotSound,
                            m_pSoundSource);
  }   
}

