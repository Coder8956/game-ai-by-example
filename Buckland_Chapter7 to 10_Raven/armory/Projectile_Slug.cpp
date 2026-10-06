//==============================================================================================
//【文件说明】Projectile_Slug.cpp —— Slug 轨道炮弹的实现
//
//【这个文件是干什么的?】
//  Update:未命中时按转向力飞向目标并 TestForImpact;已命中但轨迹还可见时,等可见时长过了再死亡。
//  TestForImpact:轨道弹极快、瞬发,直接算原点到当前点的射线,取撞到的所有机器人列表,
//  逐个发消息扣血(可穿多个目标)。Render:命中后画绿线表示弹迹。
#include "Projectile_Slug.h"
#include "../lua/Raven_Scriptor.h"
#include "misc/cgdi.h"
#include "../Raven_Bot.h"
#include "../Raven_Game.h"
#include "game/EntityFunctionTemplates.h"
#include "2d/WallIntersectionTests.h"
#include "../Raven_Map.h"

#include "../Raven_Messages.h"
#include "Messaging/MessageDispatcher.h"

#include <list>


//-------------------------- ctor ---------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 构造函数:调父类构造;从脚本读 Slug_Persistance(轨迹可见时长)。
Slug::Slug(Raven_Bot* shooter, Vector2D target):

        Raven_Projectile(target,
                         shooter->GetWorld(),
                         shooter->ID(),
                         shooter->Pos(),
                         shooter->Facing(),
                         script->GetInt("Slug_Damage"),
                         script->GetDouble("Slug_Scale"),
                         script->GetDouble("Slug_MaxSpeed"),
                         script->GetDouble("Slug_Mass"),
                         script->GetDouble("Slug_MaxForce")),

        m_dTimeShotIsVisible(script->GetDouble("Slug_Persistance"))
{
  
}

//------------------------------ Update ---------------------------------------
        

//--------------------------------------------------------------------------------
// Update:每帧调用。
void Slug::Update()
{
// ! :取反——还没命中时计算飞行。
  if (!HasImpacted())
  {
     //calculate the steering force
// 期望速度 = (目标点-当前位置)归一化 × 最大速度。
//(原文注释:计算转向力)
    Vector2D DesiredVelocity = Vec2DNormalize(m_vTarget - Pos()) * MaxSpeed();

// sf:转向力 = 期望速度 - 当前速度。
    Vector2D sf = DesiredVelocity - Velocity();

    //update the position
//(原文注释:更新位置)—— 加速度 = 转向力 / 质量。
    Vector2D accel = sf / m_dMass;

// 速度 += 加速度。
    m_vVelocity += accel;

    //make sure the slug does not exceed maximum velocity
//(原文注释:确保弹丸不超过最大速度)
    m_vVelocity.Truncate(m_dMaxSpeed);

    //update the position
//(原文注释:更新位置)—— 位置 += 速度。
    m_vPosition += m_vVelocity; 

// 检测命中。
    TestForImpact();
  }
// 已命中、轨迹该消失了 → 标记死亡。
  else if (!isVisibleToPlayer())
  {
// 死亡。
    m_bDead = true;
  }

}

//----------------------------------- TestForImpact ---------------------------
//--------------------------------------------------------------------------------
// TestForImpact:检测命中。
void Slug::TestForImpact()
{
  // a rail gun slug travels VERY fast. It only gets the chance to update once 
// 标记已命中(瞬发)。
//(原文注释:轨道弹极快,只更新一次)—— 直接标记已命中。
  m_bImpacted = true;

  //first find the closest wall that this ray intersects with. Then we
  //can test against all entities within this range.
// 查射线与墙交点,得命中点。
//(原文注释:先找射线与最近墙交点,再在这段距离内查所有实体)
  double DistToClosestImpact;
  FindClosestPointOfIntersectionWithWalls(m_vOrigin,
                                          m_vPosition,
                                          DistToClosestImpact,
                                          m_vImpactPoint,
                                          m_pWorld->GetMap()->GetWalls());

  //test to see if the ray between the current position of the slug and 
  //the start position intersects with any bots.
// 取撞到的机器人列表(可多个)。
//(原文注释:检测"弹丸起点→当前位置"射线撞到的所有机器人)
  std::list<Raven_Bot*> hits = GetListOfIntersectingBots(m_vOrigin, m_vPosition);

  //if no bots hit just return;
// 列表为空 → 直接返回。
//(原文注释:没撞到机器人就直接返回)
  if (hits.empty()) return;

  //give some damage to the hit bots
// 遍历命中列表,逐个发受伤消息(轨道弹可穿多目标)。
//(原文注释:给被撞到的机器人造成伤害)
  std::list<Raven_Bot*>::iterator it;
  for (it=hits.begin(); it != hits.end(); ++it)
  {
    //send a message to the bot to let it know it's been hit, and who the
    //shot came from
// 发受伤消息,附伤害值。
//(原文注释:告诉被打者它被谁打了)
    Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                            m_iShooterID,
                            (*it)->ID(),
                            Msg_TakeThatMF,
                            (void*)&m_iDamageInflicted);
    
  }
}

//-------------------------- Render -------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:画弹迹。
void Slug::Render()
{
// && :逻辑与——仍可见且已命中才画绿线(原点→命中点)。
  if (isVisibleToPlayer() && m_bImpacted)
  {
    gdi->GreenPen();
    gdi->Line(m_vOrigin, m_vImpactPoint);
  }
}