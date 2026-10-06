//==============================================================================================
//【文件说明】Goal_Composite.h —— 组合目标基类(目标容器:可挂多个子目标)
//
//【这个文件是干什么的?】
//  父类 Goal 是"原子目标"(一件事),Goal_Composite 是"组合目标"——
//  它内部维护一个子目标链表 m_SubGoals,可以把几个目标按顺序串起来执行。
//  典型用法:Raven 的"取最近武器目标"= [走到武器旁, 捡起武器] 两个子目标。
//
//【组合目标的执行规则】
//   ProcessSubgoals():① 先把链表前端"已完成/已失败"的子目标 Terminate 并 delete 掉;
//   ② 再处理最前面那个子目标的 Process();③ 若它刚完成且后面还有子目标,
//   返回 active(表示父目标还没完);子目标全处理完才返回 completed。
//   析构时自动 RemoveAllSubgoals,防止子目标内存泄漏。
//
//【谁在使用这个文件?】
//  Raven 的 goals/ 目录下所有组合目标(如 GetWeapon, MoveToPosition 等)继承它。
//
//【本文件包含了谁?】
//  <list>      —— 标准库链表(子目标列表);
//  "Goal.h"   —— 父类 Goal。
//==============================================================================================
#ifndef GOAL_COMPOSITE_H
#define GOAL_COMPOSITE_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Name:   Goal_Composite.h      
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   Base composite goal class
//-----------------------------------------------------------------------------
#include <list>
#include "Goal.h"


template <class entity_type>
//--------------------------------------------------------------------------------
// template <class entity_type> —— 同父类,是模板。
// class Goal_Composite : public Goal<entity_type> —— 组合目标,公有继承 Goal。
// typedef std::list<Goal<entity_type>*> SubgoalList; —— 给子目标链表起短名。
//--------------------------------------------------------------------------------
class Goal_Composite : public Goal<entity_type>
{
private:

    typedef std::list<Goal<entity_type>* > SubgoalList;

protected:

  // m_SubGoals:子目标链表(新目标 push_front 到最前面,先执行最前的)。
  // ProcessSubgoals 处理子目标;ForwardMessageToFrontMostSubgoal 把消息转交给最前子目标。
  //composite goals may have any number of subgoals
  SubgoalList   m_SubGoals;


  //processes any subgoals that may be present
  int  ProcessSubgoals();

  //passes the message to the front-most subgoal
  bool ForwardMessageToFrontMostSubgoal(const Telegram& msg);


public:

  // 构造:转发给父类。析构 virtual ~Goal_Composite() 自动调 RemoveAllSubgoals。
  Goal_Composite(entity_type* pE, int type):Goal<entity_type>(pE,type){}

  //when this object is destroyed make sure any subgoals are terminated
  //and destroyed.
  virtual ~Goal_Composite(){RemoveAllSubgoals();}

  // 三个纯虚函数 Activate/Process/Terminate 同父类;HandleMessage 默认转发给最前子目标;
  //logic to run when the goal is activated.
  virtual void Activate() = 0;

  //logic to run each update-step.
  virtual int  Process() = 0;

  //logic to run prior to the goal's destruction
  virtual void Terminate() = 0;

  //if a child class of Goal_Composite does not define a message handler
  //the default behavior is to forward the message to the front-most
  //subgoal
  virtual bool HandleMessage(const Telegram& msg)
  { return ForwardMessageToFrontMostSubgoal(msg);}

  // AddSubgoal 加子目标;RemoveAllSubgoals 全部终止并删除。
  //adds a subgoal to the front of the subgoal list
  void         AddSubgoal(Goal<entity_type>* g);

  //this method iterates through the subgoals and calls each one's Terminate
  //method before deleting the subgoal and removing it from the subgoal list
  void         RemoveAllSubgoals();


  virtual void RenderAtPos(Vector2D& pos, TypeToString* tts)const;
  //this is only used to render information for debugging purposes
  virtual void Render();
};





//---------------------- RemoveAllSubgoals ------------------------------------
//-----------------------------------------------------------------------------
template <class entity_type>
  // RemoveAllSubgoals:遍历子目标,先 Terminate 再 delete,最后清空链表。
void Goal_Composite<entity_type>::RemoveAllSubgoals()
{
  for (SubgoalList::iterator it = m_SubGoals.begin();
       it != m_SubGoals.end();
       ++it)
  {  
    (*it)->Terminate();
    
    delete *it;
  }

  m_SubGoals.clear();
}
 

//-------------------------- ProcessSubGoals ----------------------------------
//
//  this method first removes any completed goals from the front of the
//  subgoal list. It then processes the next goal in the list (if there is one)
//-----------------------------------------------------------------------------
template <class entity_type>
//--------------------------------------------------------------------------------
// ProcessSubgoals:组合目标的核心调度函数。
//   ① 把链表前端已完成/已失败的子目标 Terminate+delete+pop_front 掉;
//   ② 对最前面那个子目标调 Process();
//   ③ 若它刚完成但后面还有子目标 → 返回 active(父目标继续,下一帧处理下一个);
//   ④ 子目标全没了 → 返回 completed。
//--------------------------------------------------------------------------------
int Goal_Composite<entity_type>::ProcessSubgoals()
{ 
  //remove all completed and failed goals from the front of the subgoal list
  while (!m_SubGoals.empty() &&
         (m_SubGoals.front()->isComplete() || m_SubGoals.front()->hasFailed()))
  {    
    m_SubGoals.front()->Terminate();
    delete m_SubGoals.front(); 
    m_SubGoals.pop_front();
  }

  //if any subgoals remain, process the one at the front of the list
  if (!m_SubGoals.empty())
  { 
    //grab the status of the front-most subgoal
    int StatusOfSubGoals = m_SubGoals.front()->Process();

    //we have to test for the special case where the front-most subgoal
    //reports 'completed' *and* the subgoal list contains additional goals.When
    //this is the case, to ensure the parent keeps processing its subgoal list
    //we must return the 'active' status.
    if (StatusOfSubGoals == completed && m_SubGoals.size() > 1)
    {
      return active;
    }

    return StatusOfSubGoals;
  }
  
  //no more subgoals to process - return 'completed'
  else
  {
    return completed;
  }
}

//----------------------------- AddSubgoal ------------------------------------
template <class entity_type>
  // AddSubgoal:push_front 加到链表最前面(后加的先执行)。
void Goal_Composite<entity_type>::AddSubgoal(Goal<entity_type>* g)
{   
  //add the new goal to the front of the list
  m_SubGoals.push_front(g);
}



//---------------- ForwardMessageToFrontMostSubgoal ---------------------------
//
//  passes the message to the goal at the front of the queue
//-----------------------------------------------------------------------------
template <class entity_type>
  // ForwardMessageToFrontMostSubgoal:把消息直接交给最前面的子目标处理。
bool Goal_Composite<entity_type>::ForwardMessageToFrontMostSubgoal(const Telegram& msg)
{
  if (!m_SubGoals.empty())
  {
    return m_SubGoals.front()->HandleMessage(msg);
  }

  //return false if the message has not been handled
  return false;
}


//-------------------------- RenderAtPos --------------------------------------
template <class entity_type>
  // RenderAtPos:先画父目标名,再向右缩进而后递归画每个子目标(调试缩进显示)。
void  Goal_Composite<entity_type>::RenderAtPos(Vector2D& pos, TypeToString* tts)const
{
  Goal<entity_type>::RenderAtPos(pos, tts);

  pos.x += 10;

  gdi->TransparentText();
  SubgoalList::const_reverse_iterator it;
  for (it=m_SubGoals.rbegin(); it != m_SubGoals.rend(); ++it)
  {
    (*it)->RenderAtPos(pos, tts);
  }

  pos.x -= 10;
}

template <class entity_type>
void  Goal_Composite<entity_type>::Render()
{
  if (!m_SubGoals.empty())
  {
    m_SubGoals.front()->Render();
  }
}



#endif

