//==============================================================================================
//【文件说明】Raven_Projectile.cpp —— 抛射物基类的两个辅助函数实现
//
//【这个文件是干什么的?】
//  GetClosestIntersectingBot:沿弹道线段找撞到的最近机器人;
//  GetListOfIntersectingBots:沿弹道线段找撞到的所有机器人(返回列表)。
//  两者都遍历世界所有机器人,跳过开枪者自己,用"点到线段距离 < 碰撞半径"判定命中。
#include "Raven_Projectile.h"
#include "../Raven_Game.h"
#include <list>

//------------------ GetClosestIntersectingBot --------------------------------

//--------------------------------------------------------------------------------
// GetClosestIntersectingBot:找弹道(From→To 线段)上最近的被撞机器人。
// 下面用只读迭代器遍历世界所有机器人,逐台检查是否被弹道穿过。
Raven_Bot* Raven_Projectile::GetClosestIntersectingBot(Vector2D    From,
                                                       Vector2D    To)const
{
// ClosestIntersectingBot=最近命中者(先置空);ClosestSoFar=目前最近距离(先设无穷大 MaxDouble)。
  Raven_Bot* ClosestIntersectingBot = 0;
  double ClosestSoFar = MaxDouble;

  //iterate through all entities checking against the line segment FromTo
  std::list<Raven_Bot*>::const_iterator curBot;
  for (curBot =  m_pWorld->GetAllBots().begin();
       curBot != m_pWorld->GetAllBots().end();
       ++curBot)
  {
    //make sure we don't check against the shooter of the projectile
    if ( ((*curBot)->ID() != m_iShooterID))
    {
      //if the distance to FromTo is less than the entity's bounding radius then
      //there is an intersection
      if (DistToLineSegment(From, To, (*curBot)->Pos()) < (*curBot)->BRadius())
      {
        //test to see if this is the closest so far
// 算这个机器人到子弹原点的距离平方(用平方比较,省开方)。
        double Dist = Vec2DDistanceSq((*curBot)->Pos(), m_vOrigin);

// 更近就更新记录。
        if (Dist < ClosestSoFar)
        {
          Dist = ClosestSoFar;
// 记下这个最近命中者。
          ClosestIntersectingBot = *curBot;
        }
      }
    }

  }

// 返回最近命中者(没命中则为空指针)。
  return ClosestIntersectingBot;
}


//---------------------- GetListOfIntersectingBots ----------------------------
//--------------------------------------------------------------------------------
// GetListOfIntersectingBots:找弹道上撞到的所有机器人,返回列表(逻辑同上,只是全部收集而非取最近)。
std::list<Raven_Bot*> Raven_Projectile::GetListOfIntersectingBots(Vector2D From,
                                                                  Vector2D To)const
{
  //this will hold any bots that are intersecting with the line segment
// hits:命中者列表。
//(原文注释:hits 用来装所有与线段相交的机器人)
  std::list<Raven_Bot*> hits;

  //iterate through all entities checking against the line segment FromTo
  std::list<Raven_Bot*>::const_iterator curBot;
  for (curBot =  m_pWorld->GetAllBots().begin();
       curBot != m_pWorld->GetAllBots().end();
       ++curBot)
  {
    //make sure we don't check against the shooter of the projectile
    if ( ((*curBot)->ID() != m_iShooterID))
    {
      //if the distance to FromTo is less than the entities bounding radius then
      //there is an intersection so add it to hits
      if (DistToLineSegment(From, To, (*curBot)->Pos()) < (*curBot)->BRadius())
      {
// 把命中的机器人加入列表。
        hits.push_back(*curBot);
      }
    }

  }

// 返回命中列表。
  return hits;
}

