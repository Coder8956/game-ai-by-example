//==============================================================================================
//【文件说明】Raven_Target.h —— 机器人「当前目标」的信息记录结构
//
//【这个文件是干什么的?】
//  每个机器人的「目标系统」(Raven_TargetingSystem)会维护一个 Raven_Target 结构,
//  记录:我现在盯着谁(Instance)、它上次被我看见时在哪(LastVisiblePosition)、
//  它在不在我的视野里(isWithinFOV)、能不能打到它(isShootable)。
//
//【谁在使用这个文件?】
//  Raven_TargetingSystem.h/.cpp —— 持有一个 Raven_Target,每帧更新这些字段。
//==============================================================================================
#ifndef RAVEN_TARGET_H
#define RAVEN_TARGET_H
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Target
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:   struct to hold data about a target
//(原文注释翻译:记录目标相关数据的结构)
//
//-----------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// struct Raven_Target —— 目标信息结构。
struct Raven_Target
{
    //the current target (this will be null if there is no target assigned)
// Instance:指向目标机器人的指针(0=没目标)。
//(原文注释:当前目标;没分配目标时为 NULL)
  Raven_Bot*         Instance;

  //a vector marking the last visible position of the target.
// LastVisiblePosition:目标最后可见位置(目标躲墙后时,机器人凭这个位置追)。
//(原文注释:记录目标最后一次被看见时的位置)
  Vector2D           LastVisiblePosition;

  //true if target is within the field of view of the owner
// isWithinFOV:目标在视野内吗?
//(原文注释:true 表示目标在拥有者的视野锥内)
  bool               isWithinFOV;

  //true if there is no obstruction between the target and the owner, 
  //permitting a shot. (for example, a the target may be behind the owner
  //but in the open. In this situation m_bWithinFOV is false but m_bShootable
  //is true). This value is utilized by the owner to determine whether or not
  //to turn around and face the target when approached from behind
// isShootable:能射击吗?(中间无墙挡着)
//(原文注释翻译:true 表示目标和拥有者之间无遮挡、可以射击;
//           比如目标在背后但没挡着,这时 isWithinFOV=false 但 isShootable=true。
  bool               isShootable;

// 构造:初值全部清零;Reset() 把状态清空。
  Raven_Target():Instance(0), isWithinFOV(false), isShootable(false){}

  void Reset(){Instance = 0; isWithinFOV = false; isShootable = false;}
};


#endif