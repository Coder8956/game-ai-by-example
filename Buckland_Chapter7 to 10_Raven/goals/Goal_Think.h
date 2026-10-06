//==============================================================================================
//【文件说明】Goal_Think.h —— 机器人的"决策大脑"(最高级组合目标)
//
//【这个文件是干什么的?】
//  它是每个机器人脑子里的最高目标。继承自组合目标 Goal_Composite<Raven_Bot>。
//  内部装着一串"评估器"(Goal_Evaluator*),每个评估器给一个候选目标打分;
//  Arbitrate() 挑出分数最高的目标,让它接管机器人。Process() 每帧执行当前目标。
//
//【决策全景图(谁给谁打分、选中后挂哪个目标)】:
//
//        Goal_Think(决策大脑,持有一串评估器 m_Evaluators)
//        |
//        +-- GetHealthGoal_Evaluator   :血少且近血包 → 分高 → 选中 Goal_GetItem(血包)
//        +-- ExploreGoal_Evaluator     :无事可做       → 分高 → 选中 Goal_Explore
//        +-- AttackTargetGoal_Evaluator:锁了敌人       → 分高 → 选中 Goal_AttackTarget
//        +-- GetWeaponGoal_Evaluator×3:缺某武器       → 分高 → 选中 Goal_GetItem(武器)
//        |
//        Arbitrate() 遍历评估器,取最高分 MostDesirable,调 SetGoal() 挂目标
//
//        Goal_AttackTarget 内部再拆:DodgeSideToSide(横移)/SeekToPosition/HuntTarget
//        Goal_GetItem 内部再拆:FollowPath→TraverseEdge/NegotiateDoor
//
//【一次决策的调用流程】:
//   Raven_Bot 每帧 → GetBrain()->Process() → Goal_Think::Process()
//   → 若当前子目标已完成/失败 → Activate() → Arbitrate()
//   → 各评估器 CalculateDesirability() 打分 → 最高分 SetGoal() → 挂新子目标。
//
//【谁在使用这个文件?】—— Raven_Bot.h/.cpp(每个机器人 new 一个 Goal_Think 当大脑)。
//【本文件包含了谁?】—— <vector>、<string>、2d/Vector2D.h、Goals/Goal_Composite.h、
//   ../Raven_Bot.h、Goal_Evaluator.h。
//==============================================================================================
#ifndef GOAL_THINK_H
#define GOAL_THINK_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_Think .h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to arbitrate between a collection of high level goals, and
//          to process those goals.
//-----------------------------------------------------------------------------
#include <vector>
#include <string>
#include "2d/Vector2D.h"
#include "Goals/Goal_Composite.h"
#include "../Raven_Bot.h"
#include "Goal_Evaluator.h"



//--------------------------------------------------------------------------------
// class Goal_Think : public Goal_Composite<Raven_Bot>:决策大脑是最高级组合目标。
class Goal_Think : public Goal_Composite<Raven_Bot>
{
private:
  
// typedef:给类型起别名。GoalEvaluators = "一串评估器指针"的 vector 容器。
  typedef std::vector<Goal_Evaluator*>   GoalEvaluators;

private:
  
// m_Evaluators:本大脑持有的全部评估器(构造时 new 出来)。
  GoalEvaluators  m_Evaluators;

// public:对外接口。
public:

// 构造:new 出各评估器;~Goal_Think() 析构:把它们 delete 掉(防内存泄漏)。
  Goal_Think(Raven_Bot* pBot);
  ~Goal_Think();

  //this method iterates through each goal evaluator and selects the one
  //that has the highest score as the current goal
// Arbitrate:仲裁/决策——挑分最高的目标。
//(原文注释:遍历每个目标评估器,选分最高的作为当前目标)
  void Arbitrate();

  //returns true if the given goal is not at the front of the subgoal list
// notPresent:检查某目标是否正在执行(避免重复添加);const=只读函数。
//(原文注释:若给定目标不在子目标链最前面,返回 true)
  bool notPresent(unsigned int GoalType)const;

  //the usual suspects
//(原文注释:照例的函数)—— Process/Activate;Terminate 空实现。
  int  Process();
  void Activate();
  void Terminate(){}
  
  //top level goal types
// AddGoal_*:把对应目标加到子目标链最前面(立刻执行)。
//(原文注释:顶层目标的添加接口)—— AddGoal_* 系列:外部直接塞目标用。
  void AddGoal_MoveToPosition(Vector2D pos);
  void AddGoal_GetItem(unsigned int ItemType);
  void AddGoal_Explore();
  void AddGoal_AttackTarget();

  //this adds the MoveToPosition goal to the *back* of the subgoal list.
// QueueGoal_MoveToPosition:排队移动目标。
//(原文注释:把"移动到某点"目标加到子目标链末尾(排队,不插队))
  void QueueGoal_MoveToPosition(Vector2D pos);

  //this renders the evaluations (goal scores) at the specified location
// RenderEvaluations:画分数榜;Render:画各子目标。
//(原文注释:在指定位置画出各评估器的分数(调试用))
  void  RenderEvaluations(int left, int top)const;
  void  Render();


};


#endif