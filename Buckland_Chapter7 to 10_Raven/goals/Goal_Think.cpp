//==============================================================================================
//【文件说明】Goal_Think.cpp —— 决策大脑的实现
//
//【这个文件是干什么的?】
//  构造:给每个评估器随机性格偏置并 new 出 6 个评估器;
//  Arbitrate:遍历打分挑最高;Activate:触发决策;Process:执行当前子目标;
//  AddGoal_*:外部接口,直接加对应目标;Render:画调试信息。
//==============================================================================================
#include "Goal_Think.h"
#include <list>
#include "misc/Cgdi.h"
#include "../Raven_ObjectEnumerations.h"
#include "misc/utils.h"
#include "../lua/Raven_Scriptor.h"

#include "Goal_MoveToPosition.h"
#include "Goal_Explore.h"
#include "Goal_GetItem.h"
#include "Goal_Wander.h"
#include "Raven_Goal_Types.h"
#include "Goal_AttackTarget.h"


#include "GetWeaponGoal_Evaluator.h"
#include "GetHealthGoal_Evaluator.h"
#include "ExploreGoal_Evaluator.h"
#include "AttackTargetGoal_Evaluator.h"


//--------------------------------------------------------------------------------
// 构造函数:把 pBot、编号 goal_think 交给父类。
Goal_Think::Goal_Think(Raven_Bot* pBot):Goal_Composite<Raven_Bot>(pBot, goal_think)
{
  
  //these biases could be loaded in from a script on a per bot basis
  //but for now we'll just give them some random values
// 偏置随机范围:0.5~1.5。
//(原文注释:这些偏置本可按机器人从脚本读入,现在先给随机值)
  const double LowRangeOfBias = 0.5;
  const double HighRangeOfBias = 1.5;

// RandInRange(a,b):返回 [a,b] 间随机数;给每种评估器一个随机偏置。
  double HealthBias = RandInRange(LowRangeOfBias, HighRangeOfBias);
  double ShotgunBias = RandInRange(LowRangeOfBias, HighRangeOfBias);
  double RocketLauncherBias = RandInRange(LowRangeOfBias, HighRangeOfBias);
  double RailgunBias = RandInRange(LowRangeOfBias, HighRangeOfBias);
  double ExploreBias = RandInRange(LowRangeOfBias, HighRangeOfBias);
  double AttackBias = RandInRange(LowRangeOfBias, HighRangeOfBias);

  //create the evaluator objects
// push_back:往 vector 末尾追加;new:在堆上创建评估器。共 6 个评估器入列。
//(原文注释:创建各评估器对象)
  m_Evaluators.push_back(new GetHealthGoal_Evaluator(HealthBias));
  m_Evaluators.push_back(new ExploreGoal_Evaluator(ExploreBias));
  m_Evaluators.push_back(new AttackTargetGoal_Evaluator(AttackBias));
  m_Evaluators.push_back(new GetWeaponGoal_Evaluator(ShotgunBias,
                                                     type_shotgun));
  m_Evaluators.push_back(new GetWeaponGoal_Evaluator(RailgunBias,
                                                     type_rail_gun));
  m_Evaluators.push_back(new GetWeaponGoal_Evaluator(RocketLauncherBias,
                                                     type_rocket_launcher));
}

//----------------------------- dtor ------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 析构函数:~ 表示对象销毁时自动调用;用迭代器(遍历容器的指针式对象)遍历评估器,
// 逐个 delete(释放 new 出来的对象)。begin()=指向第一个元素。
Goal_Think::~Goal_Think()
{
  GoalEvaluators::iterator curDes = m_Evaluators.begin();
  for (curDes; curDes != m_Evaluators.end(); ++curDes)
  {
// *curDes:解引用取到评估器指针;delete 释放它。
    delete *curDes;
  }
}

//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Activate:激活。
void Goal_Think::Activate()
// ! :取反——机器人没被玩家接管时,才自动做决策(被玩家操控就不抢方向盘)。
{
  if (!m_pOwner->isPossessed())
  {
// 触发一次决策(挑分最高的目标)。
    Arbitrate();
  }

// 状态=进行中。
  m_iStatus = active;
}

//------------------------------ Process --------------------------------------
//
//  processes the subgoals
//(原文注释:处理各子目标)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Process:每帧调用,返回状态码。
int Goal_Think::Process()
{
// 未激活就先激活(从而触发 Arbitrate)。
  ActivateIfInactive();
  
// 跑当前子目标,取回它的状态。
  int SubgoalStatus = ProcessSubgoals();

// || :逻辑或——当前目标完成或失败时,把大脑置为 inactive,下帧重新决策。
  if (SubgoalStatus == completed || SubgoalStatus == failed)
  {
    if (!m_pOwner->isPossessed())
    {
// 大脑进入待决策状态(下一帧 Activate 会再 Arbitrate)。
      m_iStatus = inactive;
    }
  }

  return m_iStatus;
}

//----------------------------- Update ----------------------------------------
// 
//  this method iterates through each goal option to determine which one has
//  the highest desirability.
//(原文注释:遍历每个候选目标,找出渴望分最高的那个)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Arbitrate:仲裁决策。
void Goal_Think::Arbitrate()
{
// best=目前最高分;MostDesirable=目前最诱人的评估器指针。
  double best = 0;
  Goal_Evaluator* MostDesirable = 0;

  //iterate through all the evaluators to see which produces the highest score
//(原文注释:遍历所有评估器,看哪个打分最高)
  GoalEvaluators::iterator curDes = m_Evaluators.begin();
  for (curDes; curDes != m_Evaluators.end(); ++curDes)
  {
// 调该评估器给当前机器人打分(0~1)。
    double desirabilty = (*curDes)->CalculateDesirability(m_pOwner);

// >= :大于等于——比当前最高还高,就记下来。
    if (desirabilty >= best)
    {
      best = desirabilty;
// 暂存这个评估器。
      MostDesirable = *curDes;
    }
  }

// assert:断言——若 MostDesirable 为空(没选中任何目标)就报错中断。
  assert(MostDesirable && "<Goal_Think::Arbitrate>: no evaluator selected");

// 让最诱人的评估器把对应目标挂到机器人脑子里。
  MostDesirable->SetGoal(m_pOwner);
}


//---------------------------- notPresent --------------------------------------
//
//  returns true if the goal type passed as a parameter is the same as this
//  goal or any of its subgoals
//(原文注释:若传入的目标类型正是本目标或其子目标,返回 true)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// notPresent:检查某类型目标是否正在执行(链最前面的那个)。
bool Goal_Think::notPresent(unsigned int GoalType)const
{
// m_SubGoals:父类的子目标链表;empty()=是否为空。
  if (!m_SubGoals.empty())
  {
// front()=链首;GetType()=目标编号;!= 不等——不是该类型就返回 true。
    return m_SubGoals.front()->GetType() != GoalType;
  }

  return true;
}

//--------------------------------------------------------------------------------
// AddGoal_*:外部直接加目标。下面四个都是:先 notPresent 检查,
// 再清掉旧子目标,最后挂上新目标。
void Goal_Think::AddGoal_MoveToPosition(Vector2D pos)
{
  AddSubgoal( new Goal_MoveToPosition(m_pOwner, pos));
}

// AddGoal_Explore:加"探索"目标。
void Goal_Think::AddGoal_Explore()
{
  if (notPresent(goal_explore))
  {
    RemoveAllSubgoals();
    AddSubgoal( new Goal_Explore(m_pOwner));
  }
}

// AddGoal_GetItem:加"去捡某物品"目标(ItemTypeToGoalType 把物品编号转目标编号)。
void Goal_Think::AddGoal_GetItem(unsigned int ItemType)
{
  if (notPresent(ItemTypeToGoalType(ItemType)))
  {
    RemoveAllSubgoals();
    AddSubgoal( new Goal_GetItem(m_pOwner, ItemType));
  }
}

// AddGoal_AttackTarget:加"攻击目标"目标。
void Goal_Think::AddGoal_AttackTarget()
{
  if (notPresent(goal_attack_target))
  {
    RemoveAllSubgoals();
    AddSubgoal( new Goal_AttackTarget(m_pOwner));
  }
}

//-------------------------- Queue Goals --------------------------------------
//(原文注释:排队目标)—— QueueGoal_MoveToPosition 把移动目标追加到链尾排队。
//-----------------------------------------------------------------------------
// push_back:追加到子目标链末尾(排在现有目标后面执行)。
void Goal_Think::QueueGoal_MoveToPosition(Vector2D pos)
{
   m_SubGoals.push_back(new Goal_MoveToPosition(m_pOwner, pos));
}



//----------------------- RenderEvaluations -----------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// RenderEvaluations:在屏幕上逐个画出各评估器的分数(left/top 为坐标)。
void Goal_Think::RenderEvaluations(int left, int top)const
{
// gdi 绘图单例:设文字颜色为黑色。
  gdi->TextColor(Cgdi::black);
  
  std::vector<Goal_Evaluator*>::const_iterator curDes = m_Evaluators.begin();
  for (curDes; curDes != m_Evaluators.end(); ++curDes)
  {
// 让每个评估器在对应位置画自己的分数。
    (*curDes)->RenderInfo(Vector2D(left, top), m_pOwner);

// += 75:每个分数向右排开 75 像素。
    left += 75;
  }
}

//--------------------------------------------------------------------------------
// Render:遍历子目标链,让每个子目标自渲染。
void Goal_Think::Render()
{
  std::list<Goal<Raven_Bot>*>::iterator curG;
  for (curG=m_SubGoals.begin(); curG != m_SubGoals.end(); ++curG)
  {
    (*curG)->Render();
  }
}


   
