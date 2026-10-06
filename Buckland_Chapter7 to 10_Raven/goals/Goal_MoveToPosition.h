//==============================================================================================
//【文件说明】Goal_MoveToPosition.h —— "移动到某坐标"组合目标
//
//【这个文件是干什么的?】
//  让机器人走到指定坐标 m_vDestination。它是"组合目标"(Goal_Composite),
//  会自己拆成子目标:先向路径规划器请求一条路;路算好前先直奔目标;
//  路算好后收到消息 Msg_PathReady,再换成"沿路径走"(Goal_FollowPath)子目标。
//
//【完成】到达目标点;【失败】路径规划器说无路可达。
//【它包含谁?】—— Goals/Goal_Composite.h(组合目标基类)、2D/Vector2D.h、
//   ../Raven_Bot.h、Raven_Goal_Types.h。
//==============================================================================================
#ifndef GOAL_MOVE_POS_H
#define GOAL_MOVE_POS_H
#pragma warning (disable:4786)

#include "Goals/Goal_Composite.h"
#include "2D/Vector2D.h"
#include "../Raven_Bot.h"
#include "Raven_Goal_Types.h"



//--------------------------------------------------------------------------------
// class Goal_MoveToPosition : public Goal_Composite<Raven_Bot>:
// 组合目标=可以挂一串子目标的目标(基类见 Common\Goals\Goal_Composite.h)。
class Goal_MoveToPosition : public Goal_Composite<Raven_Bot>
{
private:

  //the position the bot wants to reach
// 目的地坐标。
//(原文注释:机器人想要到达的位置)
  Vector2D m_vDestination;

// public:对外接口。
public:

// 构造:pBot=所属机器人,pos=目的地;编号 goal_move_to_position 交给父类。
  Goal_MoveToPosition(Raven_Bot* pBot,
                      Vector2D   pos):
  
            Goal_Composite<Raven_Bot>(pBot,
                                      goal_move_to_position),
            m_vDestination(pos)
  {}

 //the usual suspects
//(原文注释:照例的函数)—— Activate/Process/Terminate;Terminate 此处空实现 {}。
  void Activate();
  int  Process();
  void Terminate(){}

  //this goal is able to accept messages
// HandleMessage:处理消息。const Telegram& = 只读引用(不拷贝对象,高效)。
//(原文注释:本目标能接收消息)—— HandleMessage 处理路径规划器发来的消息。
  bool HandleMessage(const Telegram& msg);

  void Render();
};





#endif
