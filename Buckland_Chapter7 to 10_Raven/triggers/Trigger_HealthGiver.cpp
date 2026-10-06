//==============================================================================================
//【文件说明】triggers\Trigger_HealthGiver.cpp —— 加血包触发器的实现
//==============================================================================================
#include "Trigger_HealthGiver.h"
#include "misc/Cgdi.h"
#include "misc/Stream_Utility_Functions.h"
#include <fstream>
#include "../lua/Raven_Scriptor.h"
#include "../constants.h"
#include "../Raven_ObjectEnumerations.h"


///////////////////////////////////////////////////////////////////////////////
// 构造函数:调基类,Read 从文件读参数。
Trigger_HealthGiver::Trigger_HealthGiver(std::ifstream& datafile):
      
     Trigger_Respawning<Raven_Bot>(GetValueFromStream<int>(datafile))
{
  Read(datafile);
}


// Try:激活状态且 pBot 踩到?就加血、然后 Deactivate(等重生)。
void Trigger_HealthGiver::Try(Raven_Bot* pBot)
{
  if (isActive() && isTouchingTrigger(pBot->Pos(), pBot->BRadius()))
  {
    pBot->IncreaseHealth(m_iHealthGiven);

    Deactivate();
  } 
}


// Render:激活时画个白底红十字。
void Trigger_HealthGiver::Render()
{
  if (isActive())
  {
    gdi->BlackPen();
    gdi->WhiteBrush();
    const int sz = 5;
    gdi->Rect(Pos().x-sz, Pos().y-sz, Pos().x+sz+1, Pos().y+sz+1);
    gdi->RedPen();
    gdi->Line(Pos().x, Pos().y-sz, Pos().x, Pos().y+sz+1);
    gdi->Line(Pos().x-sz, Pos().y, Pos().x+sz+1, Pos().y);
  }
}


// Read:从文件读位置/半径/加血量/导航图节点索引,设触发区和重生延迟。
void Trigger_HealthGiver::Read(std::ifstream& in)
{
  double x, y, r;
  int GraphNodeIndex;
  
  in >> x >> y  >> r >> m_iHealthGiven >> GraphNodeIndex;

  SetPos(Vector2D(x,y)); 
  SetBRadius(r);
  SetGraphNodeIndex(GraphNodeIndex);

  //create this trigger's region of fluence
  AddCircularTriggerRegion(Pos(), script->GetDouble("DefaultGiverTriggerRange"));

  SetRespawnDelay((unsigned int)(script->GetDouble("Health_RespawnDelay") * FrameRate));
  SetEntityType(type_health);
}
