//==============================================================================================
//【文件说明】Goal_Wander.h —— "漫无目的闲逛"目标
//
//【这个文件是干什么的?】
//  让机器人开启"游荡"转向行为:随机晃晃悠悠地走,直到被上级目标终止。
//  它是一个"叶子目标"(自己不再拆子目标),继承自通用目标模板 Goal<Raven_Bot>
//  (父类定义在 Common\Goals\Goal.h,由另一个分片负责注释)。
//
//【触发条件】—— 由上层组合目标(如 Goal_Explore)按需创建。
//【激活动作】—— 开启游荡转向;【完成/失败】—— 只能被外部 Terminate 终止。
//
//【本文件包含了谁?】
//  "Goals/Goal.h"       —— 通用目标基类模板(Common\Goals\ 目录);
//  "Raven_Goal_Types.h" —— 目标编号表(用到 goal_wander);
//  "../Raven_Bot.h"      —— 机器人类。
//
//【C++ 小课堂:模板类继承】
//   Goal<Raven_Bot> 表示"以 Raven_Bot 为类型参数实例化的目标模板"。
//   每个目标都要实现三个标准动作:Activate(激活)、Process(每帧处理,返回状态)、
//   Terminate(终止)。m_iStatus 记录状态:active/completed/failed/inactive。
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
#include "Goals/Goal.h"
#include "Raven_Goal_Types.h"
#include "../Raven_Bot.h"


//--------------------------------------------------------------------------------
// class Goal_Wander : public Goal<Raven_Bot>:闲逛是一种目标。
class Goal_Wander : public Goal<Raven_Bot>
{
private:

public:

// 构造函数:pBot=所属机器人;把它和编号 goal_wander 一起交给父类。
  Goal_Wander(Raven_Bot* pBot):Goal<Raven_Bot>(pBot,goal_wander)
  {}

// Activate:激活(开游荡);Process:每帧处理返回状态码;Terminate:终止(关游荡)。
  void Activate();

  int  Process();

  void Terminate();
};





#endif
