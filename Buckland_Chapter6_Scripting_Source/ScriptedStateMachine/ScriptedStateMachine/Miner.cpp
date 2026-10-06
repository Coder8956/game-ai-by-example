//==============================================================================================
//【文件说明】Miner.cpp —— Miner 矿工类的函数实现
//
//【这个文件是干什么的?】
//  实现矿工的构造、加金块、每帧 Update、判断是否疲劳。注意:真正的"行为"不在这儿——
//  Update 只是转发给脚本状态机 m_pStateMachine->Update(),由 Lua 脚本决定矿工这步干什么。
//==============================================================================================
#include "Miner.h"


// 构造函数:先把金块、疲劳清零,再把 name 交给父类 Entity;函数体内 new 一个
// 脚本化状态机,并把 this(矿工自己)传进去——状态机就知道自己管的是哪个矿工。
Miner::Miner(std::string    name):m_iGoldCarried(0),
                                  m_iFatigue(0),
                                  Entity(name)
                               
{
  m_pStateMachine = new ScriptedStateMachine<Miner>(this);
}

// AddToGoldCarried:金块数加 val;若被扣成负数则归零(不许欠金块)。
void Miner::AddToGoldCarried(int val)
{
  m_iGoldCarried += val;

  if (m_iGoldCarried < 0) m_iGoldCarried = 0;
}


// Update:每帧被 main 调用,直接转发给脚本状态机(Lua 状态逻辑在那里)。
void Miner::Update()
{
  m_pStateMachine->Update(); 
}

// Fatigued:疲劳值超过 TirednessThreshold(2)就返回 true,该回家睡觉了。
bool Miner::Fatigued()const
{
  if (m_iFatigue > TirednessThreshold)
  {
    return true;
  }

  return false;
}