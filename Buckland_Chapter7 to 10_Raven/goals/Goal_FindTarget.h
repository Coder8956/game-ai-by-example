//==============================================================================================
//【文件说明】Goal_FindTarget.h —— (注意:这是一份"重复/历史遗留"文件)
//
//【这个文件是干什么的?】
//  按文件名本应是"寻找目标"目标,但磁盘上这个文件里的内容实际是
//  Goal_Wander(闲逛)类的一个早期副本(用旧头路径 ai/Goal.h、旧成员名 m_Status)。
//  它与同目录 Goal_Wander.h 内容重复,工程里真正使用的是 Goal_Wander.h。
//  本文件只加注释说明,不改任何代码(这是原工程自带的历史遗留副本)。
//
//【C++ 小课堂】:类、继承、构造函数的解释详见 Goal_Wander.h 中的注释。
//==============================================================================================
#ifndef GOAL_WANDER_H
#define GOAL_WANDER_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_Wander.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   Causes a bot to wander until terminated
//-----------------------------------------------------------------------------
#include "ai/Goal.h"
#include "Raven_Goal_Types.h"
#include "../Raven_Bot.h"


// 注意:这里类名仍叫 Goal_Wander(历史副本,未改名)。
class Goal_Wander : public Goal<Raven_Bot>
{
private:

public:

  Goal_Wander(Raven_Bot* pBot):Goal<Raven_Bot>(pBot,goal_wander)
  {}

// Activate/Process/Terminate 标准动作(实现在同名 .cpp)。
  void Activate();

  int  Process();

  void Terminate();
};





#endif
