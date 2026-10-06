//==============================================================================================
//【文件说明】Goal_FindTarget.cpp —— (历史遗留副本,见同名 .h 说明)
//
//【这个文件是干什么的?】
//  内容是 Goal_Wander(闲逛)类的早期实现副本:Activate 开游荡;
//  Process 未激活就激活并置 active;Terminate 关游荡并置 completed。
//  注意这里用旧成员名 m_Status(新版用 m_iStatus),是原工程自带的历史副本。
//==============================================================================================
#include "Goal_Wander.h"
#include "..\Raven_Bot.h"
#include "..\Raven_SteeringBehaviors.h"





//---------------------------- Initialize -------------------------------------
//-----------------------------------------------------------------------------  
// Activate:开游荡转向。
void Goal_Wander::Activate()
{
  m_pOwner->GetSteering()->WanderOn();
}

//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
// Process:每帧调用,返回状态码。
int Goal_Wander::Process()
{
// m_Status:旧版状态成员名(新版叫 m_iStatus);== inactive=未激活。
  if (m_Status == inactive)
  {
    Activate();
    m_Status = active;
  }

  return m_Status;
}

//---------------------------- Terminate --------------------------------------
//-----------------------------------------------------------------------------
// Terminate:关游荡转向,状态=完成。
void Goal_Wander::Terminate()
{
  m_pOwner->GetSteering()->WanderOff();

  m_Status = completed;
}

