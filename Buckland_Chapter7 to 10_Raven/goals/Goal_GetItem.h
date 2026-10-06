//==============================================================================================
//【文件说明】Goal_GetItem.h —— "去捡某件物品"组合目标(血包/武器)
//
//【这个文件是干什么的?】
//  继承自组合目标 Goal_Composite<Raven_Bot>。让机器人去捡一件指定物品
//  (m_iItemToGet:血包/霰弹/轨道炮/火箭筒)。它内部:先请求到物品的路径;
//  路算好前先闲逛;路算好后换成"沿路径走";发现物品被对手捡走就终止。
//
//【完成】走到物品旁触发拾取;【失败】无路可达 / 物品被抢。
//【它包含谁?】—— Goals/Goal_Composite.h、Raven_Goal_Types.h、../Raven_Bot.h、
//   triggers/trigger.h(触发器,血包/武器在地图上就是一种触发器)。
//==============================================================================================
#ifndef GOAL_GET_ITEM_H
#define GOAL_GET_ITEM_H
#pragma warning (disable:4786)

#include "Goals/Goal_Composite.h"
#include "Raven_Goal_Types.h"
#include "../Raven_Bot.h"
#include "triggers/trigger.h"


//helper function to change an item type enumeration into a goal type
// 自由函数:物品编号→目标编号(如 type_health→goal_get_health)。
//(原文注释:辅助函数,把"物品类型编号"翻成"目标类型编号")
int ItemTypeToGoalType(int gt);


//--------------------------------------------------------------------------------
// class Goal_GetItem : public Goal_Composite<Raven_Bot>:捡物品是一种组合目标。
class Goal_GetItem : public Goal_Composite<Raven_Bot>
{
private:

// m_iItemToGet:要捡的物品编号。
  int                     m_iItemToGet;

// m_pGiverTrigger:指向该物品在地图上的"触发器"对象(血包/武器就是触发器)。
  Trigger<Raven_Bot>*     m_pGiverTrigger;

  //true if a path to the item has been formulated
// m_bFollowingPath:是否正在沿路径走(布尔)。
//(原文注释:若到物品的路径已算好,则为 true)
  bool                    m_bFollowingPath;

  //returns true if the bot sees that the item it is heading for has been
  //picked up by an opponent
// hasItemBeenStolen:物品是否已被抢走;const=只读函数。
//(原文注释:若机器人看到它要捡的物品已被对手捡走,则返回 true)
  bool hasItemBeenStolen()const;

// public:对外接口。
public:

// 构造:pBot=机器人,item=要捡的物品;m_pGiverTrigger 先置 0(空指针),
// m_bFollowingPath 先置 false。
  Goal_GetItem(Raven_Bot* pBot,
               int        item):Goal_Composite<Raven_Bot>(pBot,
                                                   ItemTypeToGoalType(item)),
                                m_iItemToGet(item),
                                m_pGiverTrigger(0),
                                m_bFollowingPath(false)
  {}


  void Activate();

  int  Process();

  bool HandleMessage(const Telegram& msg);

// Terminate 内联在类里:直接把状态置为完成。
  void Terminate(){m_iStatus = completed;}
};






#endif
