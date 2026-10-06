//==============================================================================================
//【文件说明】Goal_MoveToItem.cpp —— (旧框架遗留文件,见同名 .h 说明)
//
//【这个文件是干什么的?】
//  Initialize:向导航模块请求到物品的路径;算路期间先让机器人朝前挪一小步;
//  然后把本目标标记为已满足(m_bSatisfied=true)。
#include "Goal_MoveToItem.h"

#include "..\Raven_Bot.h"
#include "..\Raven_NavModule.h"

#include "Raven_GoalQ.h"



//--------------------------------------------------------------------------------
// Initialize:初始化。
void Goal_MoveToItem::Initialize()
{
  //request a path to the item
// NavModule()=导航模块;CreatePathToItem(物品类型)=发起寻路。
//(原文注释:请求一条到物品的路径)
  m_pOwner->NavModule()->CreatePathToItem(m_iItemType);

  //the bot may have to wait a few update cycles before a path is calculated
  //so for appearances sake it just moves forward a little
// 加一个"直奔某点"子目标:目标点=当前位置+朝向×20(朝前 20 像素)。
//(原文注释:算路要等几帧,这期间让机器人先往前走一小步)
  m_pOwner->GoalQ()->AddGoal_SeekToPosition(m_pOwner->Pos() + m_pOwner->Facing()*20);

  //this goal is now satisfied
// 标记本目标已满足。
//(原文注释:本目标至此视为已完成)
  m_bSatisfied = true;
}
