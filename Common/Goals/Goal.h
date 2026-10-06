//==============================================================================================
//【文件说明】Goal.h —— 目标导向行为(GOAP)的"目标基类"
//
//【这个文件是干什么的?】
//  Raven(第 7~10 章)的 AI 用"目标"来组织行为。每个目标有完整的生命周期:
//    激活(Activate) → 每帧处理(Process) → 满足或失败时终止(Terminate)。
//  目标有 4 种状态:active(激活中)、inactive(未激活)、completed(完成)、failed(失败)。
//  本类是抽象基类(纯虚函数 Activate/Process/Terminate),具体目标(如"走到弹药包")
//  由子类实现这三个函数。
//
//【目标生命周期全景】
//   新建目标 → 状态 inactive;
//   每帧 Process:若 inactive 先调 Activate() 激活;返回 completed/failed/active;
//   完成或失败 → 调 Terminate() 收尾(关掉转向行为等);
//   失败 → ReactivateIfFailed 把状态改回 inactive,下一帧重新 Activate(重新规划)。
//
//【谁在使用这个文件?】
//  Goal_Composite.h(组合目标基类)、Raven 中的 goals/ 目录下所有具体目标类。
//
//【本文件包含了谁?】
//  struct Telegram;       —— 前置声明;
//  "misc/cgdi.h"           —— gdi 绘图单例(调试渲染目标名);
//  "misc/TypeToString.h"   —— 目标编号 → 名字字符串(调试显示)。
//==============================================================================================
#ifndef GOAL_H
#define GOAL_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Name:   Goal.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   Base goal class.
//-----------------------------------------------------------------------------

struct Telegram;
#include "misc/cgdi.h"
#include "misc/TypeToString.h"



template <class entity_type>
//--------------------------------------------------------------------------------
// 上面 template <class entity_type> 表示这是模板目标:目标挂在哪个实体上由使用方指定。
// class Goal —— 目标基类。
//--------------------------------------------------------------------------------
class Goal
{
public:

  // 匿名枚举:目标的 4 种状态(依次取值 0/1/2/3),m_iStatus 用它记状态。
   enum {active, inactive, completed, failed};
  
protected:

//--------------------------------------------------------------------------------
// 受保护成员:m_iType(目标类型编号)、m_pOwner(拥有者实体指针)、
//  m_iStatus(当前状态)。ActivateIfInactive/ReactivateIfFailed 是状态切换辅助函数。
//--------------------------------------------------------------------------------
  //an enumerated type specifying the type of goal
  int             m_iType;

  //a pointer to the entity that owns this goal
  entity_type*    m_pOwner;

  //an enumerated value indicating the goal's status (active, inactive,
  //completed, failed)
  int             m_iStatus;


  /* the following methods were created to factor out some of the commonality
     in the implementations of the Process method() */

  //if m_iStatus = inactive this method sets it to active and calls Activate()
  void ActivateIfInactive();

  //if m_iStatus is failed this method sets it to inactive so that the goal
  //will be reactivated (and therefore re-planned) on the next update-step.
  void ReactivateIfFailed();

public:

  //note how goals start off in the inactive state
  // 构造函数:记下类型和拥有者;状态默认 inactive(目标刚建好不立刻执行)。
  Goal(entity_type*  pE, int type):m_iType(type),
                                   m_pOwner(pE),
                                   m_iStatus(inactive)
  {}

  virtual ~Goal(){}

//--------------------------------------------------------------------------------
// 三个纯虚函数(子类必须实现)= 目标生命周期三件套:
//   Activate()   —— 激活时(从 inactive 切到 active)做的事;
//   Process()    —— 每帧调一次,返回当前状态(active/completed/failed);
//   Terminate()  —— 目标结束时的清理。
//  HandleMessage 默认返回 false(不处理消息);AddSubgoal 对普通目标抛异常
//  (普通目标不能加子目标,能加子目标的是 Goal_Composite)。
//--------------------------------------------------------------------------------
  //logic to run when the goal is activated.
  virtual void Activate() = 0;

  //logic to run each update-step
  virtual int  Process() = 0;

  //logic to run when the goal is satisfied. (typically used to switch
  //off any active steering behaviors)
  virtual void Terminate() = 0;

  //goals can handle messages. Many don't though, so this defines a default
  //behavior
  virtual bool HandleMessage(const Telegram& msg){return false;}


  //a Goal is atomic and cannot aggregate subgoals yet we must implement
  //this method to provide the uniform interface required for the goal
  //hierarchy.
  virtual void AddSubgoal(Goal<entity_type>* g)
  {throw std::runtime_error("Cannot add goals to atomic goals");}


  // 状态查询:isComplete/isActive/isInactive/hasFailed;GetType 取目标类型编号。
  bool         isComplete()const{return m_iStatus == completed;} 
  bool         isActive()const{return m_iStatus == active;}
  bool         isInactive()const{return m_iStatus == inactive;}
  bool         hasFailed()const{return m_iStatus == failed;}
  int          GetType()const{return m_iType;}

  
  
  //this is used to draw the name of the goal at the specific position
  //used for debugging
  virtual void RenderAtPos(Vector2D& pos, TypeToString* tts)const;
  
  //used to render any goal specific information
  virtual void Render(){}
  
};




//if m_iStatus is failed this method sets it to inactive so that the goal
//will be reactivated (replanned) on the next update-step.
template <class entity_type>
  // ReactivateIfFailed:失败时把状态改回 inactive,下一帧会重新 Activate(重新规划)。
void  Goal<entity_type>::ReactivateIfFailed()
{
  if (hasFailed())
  {
     m_iStatus = inactive;
  }
}

  
template <class entity_type>
  // ActivateIfInactive:未激活时调 Activate() 把它激活。
void  Goal<entity_type>::ActivateIfInactive()
{
  if (isInactive())
  {
    Activate();   
  }
}

template <class entity_type>
  // RenderAtPos:调试用——按状态给目标名上颜色(绿=完成,黑=未激活,红=失败,蓝=激活)。
void  Goal<entity_type>::RenderAtPos(Vector2D& pos, TypeToString* tts)const
{
  pos.y += 15;
  gdi->TransparentText();
  if (isComplete()) gdi->TextColor(0,255,0);
  if (isInactive()) gdi->TextColor(0,0,0);
  if (hasFailed()) gdi->TextColor(255,0,0);
  if (isActive()) gdi->TextColor(0,0,255);

  gdi->TextAtPos(pos.x, pos.y, tts->Convert(GetType())); 
}

#endif