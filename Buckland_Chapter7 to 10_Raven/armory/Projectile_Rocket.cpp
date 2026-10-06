//==============================================================================================
//【文件说明】Projectile_Rocket.cpp —— Rocket 火箭的实现
//
//【这个文件是干什么的?】
//  Update:未命中时直线飞行并测命中;已命中后让爆炸圈半径逐渐增大,到爆炸半径就标记死亡。
//  TestForImpact:测弹道撞到机器人/墙/到达目标点,任一命中就爆炸并范围扣血。
//  InflictDamageOnBotsWithinBlastRadius:遍历所有机器人,在爆炸半径内就发消息扣血。
//  Render:画红色小火箭;命中后再画一个空心爆炸圈。
#include "Projectile_Rocket.h"
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
// 构造函数:调父类构造;爆炸圈当前半径先设 0.0,爆炸半径从脚本读 Rocket_BlastRadius。
Rocket::Rocket(Raven_Bot* shooter, Vector2D target):

        Raven_Projectile(target,
                         shooter->GetWorld(),
                         shooter->ID(),
                         shooter->Pos(),
                         shooter->Facing(),
                         script->GetInt("Rocket_Damage"),
                         script->GetDouble("Rocket_Scale"),
                         script->GetDouble("Rocket_MaxSpeed"),
                         script->GetDouble("Rocket_Mass"),
                         script->GetDouble("Rocket_MaxForce")),

       m_dCurrentBlastRadius(0.0),
       m_dBlastRadius(script->GetDouble("Rocket_BlastRadius"))
{
// 断言:目标点不能是 (0,0)。
   assert (target != Vector2D());
}


//------------------------------ Update ---------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Update:每帧调用。
void Rocket::Update()
{
// ! :取反——还没命中时直线飞行。
  if (!m_bImpacted)
  {
// 速度 = 最大速度 × 朝向(直线飞)。
    m_vVelocity = MaxSpeed() * Heading();

    //make sure vehicle does not exceed maximum velocity
//(原文注释:确保不超过最大速度)
    m_vVelocity.Truncate(m_dMaxSpeed);

    //update the position
// 位置 += 速度。
    m_vPosition += m_vVelocity;

// 检测命中(命中就爆炸)。
    TestForImpact();  
  }

  else
  {
// 已命中:让爆炸圈半径按衰减速率逐渐扩大。
    m_dCurrentBlastRadius += script->GetDouble("Rocket_ExplosionDecayRate");

    //when the rendered blast circle becomes equal in size to the blast radius
    //the rocket can be removed from the game
// 爆炸圈画够大了 → 标记死亡。
//(原文注释:当画出的爆炸圈扩大到等于爆炸半径时,火箭即可从游戏删除)
    if (m_dCurrentBlastRadius > m_dBlastRadius)
    {
// 死亡。
      m_bDead = true;
    }
  }
}

//--------------------------------------------------------------------------------
// TestForImpact:检测命中(三种情况:撞机器人/撞墙/飞到目标点)。
void Rocket::TestForImpact()
{
   
    //if the projectile has reached the target position or it hits an entity
    //or wall it should explode/inflict damage/whatever and then mark itself
    //as dead


    //test to see if the line segment connecting the rocket's current position
    //and previous position intersects with any bots.
// 取弹道线段上最近的被撞机器人。
//(原文注释:检测"当前位置→上一位置"这段线段是否撞到机器人)
    Raven_Bot* hit = GetClosestIntersectingBot(m_vPosition - m_vVelocity, m_vPosition);
    
    //if hit
// 撞到机器人:标记已命中、发消息告诉被打者是谁打的、范围扣血。
//(原文注释:若命中)
    if (hit)
// 发受伤消息(附伤害值),并对爆炸范围内机器人范围扣血。
    {
      m_bImpacted = true;

      //send a message to the bot to let it know it's been hit, and who the
      //shot came from
      Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                              m_iShooterID,
                              hit->ID(),
                              Msg_TakeThatMF,
                              (void*)&m_iDamageInflicted);

      //test for bots within the blast radius and inflict damage
      InflictDamageOnBotsWithinBlastRadius();
    }

    //test for impact with a wall
//(原文注释:检测是否撞墙)
    double dist;
// 撞墙:标记已命中、范围扣血、把火箭移到交点。
     if( FindClosestPointOfIntersectionWithWalls(m_vPosition - m_vVelocity,
                                                 m_vPosition,
                                                 dist,
                                                 m_vImpactPoint,
                                                 m_pWorld->GetMap()->GetWalls()))
     {
        m_bImpacted = true;
      
        //test for bots within the blast radius and inflict damage
        InflictDamageOnBotsWithinBlastRadius();

// 移到墙交点。
        m_vPosition = m_vImpactPoint;

        return;
    }
    
    //test to see if rocket has reached target position. If so, test for
     //all bots in vicinity
// tolerance:容差 5.0(用距离平方 < 容差平方 判断"到了")。
//(原文注释:测试火箭是否到达目标点;到了就检查附近所有机器人)
    const double tolerance = 5.0;   
// 离目标点足够近也算命中,爆炸范围扣血。
    if (Vec2DDistanceSq(Pos(), m_vTarget) < tolerance*tolerance)
    {
      m_bImpacted = true;

      InflictDamageOnBotsWithinBlastRadius();
    }
}

//--------------- InflictDamageOnBotsWithinBlastRadius ------------------------
//
//  If the rocket has impacted we test all bots to see if they are within the 
//  blast radius and reduce their health accordingly
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// InflictDamageOnBotsWithinBlastRadius:遍历世界所有机器人,
// 若它离爆炸点的距离 < 爆炸半径 + 它自身碰撞半径,就发消息扣血。
void Rocket::InflictDamageOnBotsWithinBlastRadius()
{
  std::list<Raven_Bot*>::const_iterator curBot = m_pWorld->GetAllBots().begin();

  for (curBot; curBot != m_pWorld->GetAllBots().end(); ++curBot)
  {
// 距离 < 爆炸半径+机器人半径 → 在爆炸范围内。
    if (Vec2DDistance(Pos(), (*curBot)->Pos()) < m_dBlastRadius + (*curBot)->BRadius())
    {
      //send a message to the bot to let it know it's been hit, and who the
      //shot came from
      Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                              m_iShooterID,
                              (*curBot)->ID(),
                              Msg_TakeThatMF,
                              (void*)&m_iDamageInflicted);
      
    }
  }  
}


//-------------------------- Render -------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:画火箭与爆炸圈。
void Rocket::Render()
{
  
// 红笔橙刷画一个小圆(火箭本体)。
  gdi->RedPen();
  gdi->OrangeBrush();
  gdi->Circle(Pos(), 2);

// 命中后:空心刷画一个半径为当前爆炸半径的圈(爆炸光圈)。
  if (m_bImpacted)
  {
    gdi->HollowBrush();
    gdi->Circle(Pos(), m_dCurrentBlastRadius);
  }
}