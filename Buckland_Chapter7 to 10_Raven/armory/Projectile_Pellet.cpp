//==============================================================================================
//【文件说明】Projectile_Pellet.cpp —— Pellet 弹丸的实现
//
//【这个文件是干什么的?】
//  Update:未命中时按转向力飞向目标,然后 TestForImpact;已命中但轨迹还可见时,
//   等可见时长过了再标记死亡。TestForImpact:霰弹是瞬发的,直接算原点到命中点的射线,
//   查有没有撞到机器人,撞到就发消息扣血。Render:命中后画一条黄线+棕点表示弹迹。
#include "Projectile_Pellet.h"
#include "../lua/Raven_Scriptor.h"
#include "misc/cgdi.h"
#include "../Raven_Bot.h"
#include "../Raven_Game.h"
#include "game/EntityFunctionTemplates.h"
#include "../constants.h"
#include "2d/WallIntersectionTests.h"
#include "../Raven_Map.h"
#include <list>

#include "../Raven_Messages.h"
#include "Messaging/MessageDispatcher.h"



//-------------------------- ctor ---------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 构造函数:调父类构造,并从脚本读 Pellet_Persistance(轨迹可见时长)。
Pellet::Pellet(Raven_Bot* shooter, Vector2D target):

        Raven_Projectile(target,
                         shooter->GetWorld(),
                         shooter->ID(),
                         shooter->Pos(),
                         shooter->Facing(),
                         script->GetInt("Pellet_Damage"),
                         script->GetDouble("Pellet_Scale"),
                         script->GetDouble("Pellet_MaxSpeed"),
                         script->GetDouble("Pellet_Mass"),
                         script->GetDouble("Pellet_MaxForce")),

        m_dTimeShotIsVisible(script->GetDouble("Pellet_Persistance"))
{
  
}

//------------------------------ Update ---------------------------------------
        

//--------------------------------------------------------------------------------
// Update:每帧调用。
void Pellet::Update()
{
// ! :取反——还没命中时才计算飞行。
  if (!HasImpacted())
  {
     //calculate the steering force
// 期望速度 = (目标点-当前位置)归一化后 × 最大速度(朝目标全速飞)。
//(原文注释:计算转向力)
    Vector2D DesiredVelocity = Vec2DNormalize(m_vTarget - Pos()) * MaxSpeed();

// sf:转向力 = 期望速度 - 当前速度。
    Vector2D sf = DesiredVelocity - Velocity();

    //update the position
//(原文注释:更新位置)
// 加速度 = 转向力 / 质量(牛顿第二定律 F=ma)。
    Vector2D accel = sf / m_dMass;

// 速度 += 加速度。
    m_vVelocity += accel;

    //make sure vehicle does not exceed maximum velocity
// 速度截断到上限。
//(原文注释:确保不超过最大速度)
    m_vVelocity.Truncate(m_dMaxSpeed);

    //update the position
// 位置 += 速度。
    m_vPosition += m_vVelocity; 

// 检测命中。
    TestForImpact();
  }
// 已命中、且轨迹该消失了 → 标记死亡(可删除)。
  else if (!isVisibleToPlayer())
  {
// 死亡。
    m_bDead = true;
  }
}

//----------------------------------- TestForImpact ---------------------------
//--------------------------------------------------------------------------------
// TestForImpact:检测命中。
void Pellet::TestForImpact()
{
  //a shot gun shell is an instantaneous projectile so it only gets the chance
  //to update once 
// 直接标记已命中(霰弹瞬发,不逐段飞)。
//(原文注释:霰弹是瞬发抛射物,只更新一次)
  m_bImpacted = true;

  //first find the closest wall that this ray intersects with. Then we
  //can test against all entities within this range.
// DistToClosestImpact:到最近墙的距离;查射线与墙交点。
//(原文注释:先找这条射线与最近墙的交点,再在这段距离内查所有实体)
  double DistToClosestImpact;
  FindClosestPointOfIntersectionWithWalls(m_vOrigin,
                                          m_vPosition,
                                          DistToClosestImpact,
                                          m_vImpactPoint,
                                          m_pWorld->GetMap()->GetWalls());

  //test to see if the ray between the current position of the shell and 
  //the start position intersects with any bots.
// 取射线上最近的被撞机器人。
//(原文注释:检测"弹丸起点→当前位置"这段射线是否撞到机器人)
  Raven_Bot* hit = GetClosestIntersectingBot(m_vOrigin, m_vImpactPoint);
  
  //if no bots hit just return;
// !hit = 没打到人,直接返回。
//(原文注释:没撞到机器人就直接返回)
  if (!hit) return;

  //determine the impact point with the bot's bounding circle so that the
  //shell can be rendered properly
// 求线段与机器人碰撞圆的最近交点,更新命中点。
//(原文注释:算出弹丸与机器人碰撞圆的交点,便于正确画弹迹)
  GetLineSegmentCircleClosestIntersectionPoint(m_vOrigin,
                                               m_vImpactPoint,
                                               hit->Pos(),
                                               hit->BRadius(),
                                               m_vImpactPoint);

  //send a message to the bot to let it know it's been hit, and who the
  //shot came from
// 发受伤消息,附带伤害值。
//(原文注释:给被打中的机器人发消息,告诉它被谁打了)
  Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                              m_iShooterID,
                              hit->ID(),
                              Msg_TakeThatMF,
                              (void*)&m_iDamageInflicted);
}

//-------------------------- Render -------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:画弹迹。
void Pellet::Render()
{
// && :逻辑与——仍可见且已命中才画。
  if ( isVisibleToPlayer() && m_bImpacted)
  {
// 黄笔画原点→命中点的线段;棕色画笔画命中点的小圆。
    gdi->YellowPen();
    gdi->Line(m_vOrigin, m_vImpactPoint);

    gdi->BrownBrush();
    gdi->Circle(m_vImpactPoint, 3);
  }
}