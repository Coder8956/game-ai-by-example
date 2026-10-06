//==============================================================================================
//【文件说明】Projectile_Bolt.cpp —— Bolt 子弹的实现
//
//【这个文件是干什么的?】
//  构造:把开枪者的世界/ID/位置/朝向和脚本里读的伤害/速度/质量交给父类;
//  Update:直线飞行,检测弹道线段是否撞到机器人(命中就发消息扣血)或墙(命中就停);
//  Render:画一条绿色粗线表示光束。
#include "Projectile_Bolt.h"
#include "../lua/Raven_Scriptor.h"
#include "misc/cgdi.h"
#include "../Raven_Bot.h"
#include "../Raven_Game.h"
#include "../constants.h"
#include "2d/WallIntersectionTests.h"
#include "../Raven_Map.h"

#include "../Raven_Messages.h"
#include "Messaging/MessageDispatcher.h"


//-------------------------- ctor ---------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 构造函数:冒号后调父类 Raven_Projectile 的构造。
// script->GetInt/GetDouble:从 Lua 脚本读参数(伤害、缩放、速度、质量、最大力)。
Bolt::Bolt(Raven_Bot* shooter, Vector2D target):

        Raven_Projectile(target,
                         shooter->GetWorld(),
                         shooter->ID(),
                         shooter->Pos(),
                         shooter->Facing(),
                         script->GetInt("Bolt_Damage"),
                         script->GetDouble("Bolt_Scale"),
                         script->GetDouble("Bolt_MaxSpeed"),
                         script->GetDouble("Bolt_Mass"),
                         script->GetDouble("Bolt_MaxForce"))
{
// assert:断言——目标点不能是 (0,0) 空向量,否则报错。
   assert (target != Vector2D());
}


//------------------------------ Update ---------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Update:每帧调用。
void Bolt::Update()
{
// ! :取反——还没命中才继续飞(已命中就停下,不再更新)。
  if (!m_bImpacted)
  {
// 速度 = 最大速度 × 朝向单位向量(让子弹沿直线飞)。
    m_vVelocity = MaxSpeed() * Heading();

    //make sure vehicle does not exceed maximum velocity
// Truncate:把速度向量长度截断到上限(防止超速)。
//(原文注释:确保不超过最大速度)
    m_vVelocity.Truncate(m_dMaxSpeed);

    //update the position
// 位置 += 速度(每帧前进一段)。
//(原文注释:更新位置)
    m_vPosition += m_vVelocity;

    
    //if the projectile has reached the target position or it hits an entity
    //or wall it should explode/inflict damage/whatever and then mark itself
    //as dead
//(原文注释:子弹到目标点或撞到实体/墙时,爆炸/扣血并标记自己死亡)


    //test to see if the line segment connecting the bolt's current position
    //and previous position intersects with any bots.
// 取弹道线段上最近的被撞机器人。
//(原文注释:检测"当前位置→上一位置"这段线段是否撞到机器人)
    Raven_Bot* hit = GetClosestIntersectingBot(m_vPosition - m_vVelocity,
                                               m_vPosition);
    
    //if hit
//(原文注释:若命中)
    if (hit)
// 标记:已命中、已死亡(Bolt 一击即毁)。
    {
      m_bImpacted = true;
      m_bDead     = true;

      //send a message to the bot to let it know it's been hit, and who the
      //shot came from
// 消息派发器:立刻发 Msg_TakeThatMF(受伤)消息,附带伤害值指针。
//(原文注释:给被打中的机器人发消息,告诉它被谁打了)
      Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                              m_iShooterID,
                              hit->ID(),
                              Msg_TakeThatMF,
                              (void*)&m_iDamageInflicted);
    }

    //test for impact with a wall
//(原文注释:检测是否撞到墙)
    double dist;
// 找弹道线段与墙的最近交点;命中则把子弹移到交点并停住。
     if( FindClosestPointOfIntersectionWithWalls(m_vPosition - m_vVelocity,
                                                 m_vPosition,
                                                 dist,
                                                 m_vImpactPoint,
                                                 m_pWorld->GetMap()->GetWalls()))
     {
       m_bDead     = true;
       m_bImpacted = true;

// 子弹位置 = 命中点(贴墙停下)。
       m_vPosition = m_vImpactPoint;

       return;
     }
  }
}


//-------------------------- Render -------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:画子弹。
void Bolt::Render()
{
// 用绿色粗笔画从当前位置到上一位置的线段(光束)。
  gdi->ThickGreenPen();
  gdi->Line(Pos(), Pos()-Velocity());
}