//==============================================================================================
//【文件说明】Goal_SeekToPosition.h —— "直奔某坐标点"目标
//
//【这个文件是干什么的?】
//  让机器人开启"追逐(seek)"转向,直线跑向指定坐标 m_vPosition。
//  它会估算"跑到那里该花多久",超时没到就判定卡住(failed);到了就 completed。
//  继承自通用目标模板 Goal<Raven_Bot>(父类见 Common\Goals\Goal.h)。
//
//【触发条件】—— 由上层目标需要"直奔某点"时创建(传入目标坐标)。
//【完成】到达目标点;【失败】超时卡住。
//
//【本文件包含了谁?】
//  "Goals/Goal.h"、"2d/Vector2D.h"(2D向量)、"Raven_Goal_Types.h"、"../Raven_Bot.h"。
//==============================================================================================
#ifndef GOAL_SEEK_TO_POSITION_H
#define GOAL_SEEK_TO_POSITION_H
#pragma warning (disable:4786)

#include "Goals/Goal.h"
#include "2d/Vector2D.h"
#include "Raven_Goal_Types.h"
#include "../Raven_Bot.h"


//--------------------------------------------------------------------------------
// class Goal_SeekToPosition : public Goal<Raven_Bot>:直奔点是一种目标。
class Goal_SeekToPosition : public Goal<Raven_Bot>
{
private:

  //the position the bot is moving to
// 目标点坐标(m_v=向量成员)。
//(原文注释:机器人正在前往的位置)
  Vector2D  m_vPosition;

  //the approximate time the bot should take to travel the target location
// 预计到达所需时间(秒),超时即视为卡住。
//(原文注释:机器人到达目标位置大致应花的时间)
  double     m_dTimeToReachPos;
  
  //this records the time this goal was activated
// 激活时刻。
//(原文注释:记录本目标被激活的时刻)
  double     m_dStartTime;

  //returns true if a bot gets stuck
// isStuck:是否卡住(末尾 const=只读函数,不改成员)。
//(原文注释:若机器人卡住则返回 true)
  bool      isStuck()const;

// public:对外接口。
public:

// 构造:pBot=所属机器人,target=目标坐标。
  Goal_SeekToPosition(Raven_Bot* pBot, Vector2D target);

  //the usual suspects
//(原文注释:照例的三个函数)—— Activate/Process/Terminate 标准动作,Render 画目标点。
  void Activate();
  int  Process();
  void Terminate();

  void Render();
};




#endif

