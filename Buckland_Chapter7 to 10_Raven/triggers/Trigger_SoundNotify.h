//==============================================================================================
//【文件说明】triggers\Trigger_SoundNotify.h —— 声音通知触发器
//
//【这个文件是干什么的?】
//  bot 开枪/走路发出声音时,在它位置上临时放一个「声音触发器」,
//  周围一定距离内的 bot 走过这个圆时,就会在感官记忆里记下「听见谁在哪发声」。
//  这个触发器只活一帧(本帧 Update 完就没了)。
//
//【谁在使用这个文件?】
//  Raven_Map.cpp 的 AddSoundTrigger —— bot 开枪时 new 一个本触发器。
#ifndef TRIGGER_SOUNDNOTIFY_H
#define TRIGGER_SOUNDNOTIFY_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:     Trigger_SoundNotify.h
//
//  Author:   Mat Buckland
//
//  Desc:     whenever an agent makes a sound -- such as when a weapon fires --
//            this trigger can be used to notify other bots of the event.
//
//            This type of trigger has a circular trigger region and a lifetime
//            of 1 update-step
//(原文注释翻译:这种触发器是圆形区域,寿命只有一帧)
//
//-----------------------------------------------------------------------------
#include "Triggers/Trigger_LimitedLifetime.h"
#include "../Raven_Bot.h"



//--------------------------------------------------------------------------------
// class Trigger_SoundNotify —— 继承有限寿命触发器基类。
class Trigger_SoundNotify : public Trigger_LimitedLifetime<Raven_Bot>
{
private:

  //a pointer to the bot that has made the sound
// m_pSoundSource:发声者是谁。
//(原文注释:发出声音的那个 bot 指针)
  Raven_Bot*  m_pSoundSource;

public:

// 构造:给发声者和半径;Try:某个 bot 在声音范围内就通知它;Render 空。
  Trigger_SoundNotify(Raven_Bot* source, double range);


  void  Try(Raven_Bot*);

  void  Render(){}

};




#endif