//==============================================================================================
//【文件说明】triggers\Trigger_HealthGiver.h —— 加血包触发器
//
//【这个文件是干什么的?】
//  地图上一个加血包:bot 跑上去就加血;加完包要等一阵才刷新(Respawning)。
#ifndef HEALTH_GIVER_H
#define HEALTH_GIVER_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:     Trigger_HealthGiver.h
//
//  Author:   Mat Buckland
//
//  Desc:     If a bot runs over an instance of this class its health is
//            increased. 
//(原文注释翻译:bot 踩到本类的实例就加血)
//
//-----------------------------------------------------------------------------
#include "Triggers/Trigger_Respawning.h"
#include "Triggers/TriggerRegion.h"
#include <iosfwd>
#include "../Raven_Bot.h"



//--------------------------------------------------------------------------------
// class Trigger_HealthGiver —— 继承可重生触发器基类。
class Trigger_HealthGiver : public Trigger_Respawning<Raven_Bot>
{
private:

  //the amount of health an entity receives when it runs over this trigger
// m_iHealthGiven:加血量。
//(原文注释:踩到本触发器时加多少血)
  int   m_iHealthGiven;
  
public:

// 构造(从文件读);Try:加血;Render:画红十字;Read:从文件读。
  Trigger_HealthGiver(std::ifstream& datafile);

  //if triggered, the bot's health will be incremented
  void Try(Raven_Bot* pBot);
  
  //draws a box with a red cross at the trigger's location
  void Render();

  void Read (std::ifstream& is);
};



#endif