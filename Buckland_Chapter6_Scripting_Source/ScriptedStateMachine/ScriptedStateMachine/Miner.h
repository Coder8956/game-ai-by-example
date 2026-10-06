//==============================================================================================
//【文件说明】Miner.h —— 矿工类(其状态机由 Lua 脚本定义)
//
//【这个文件是干什么的?】
//  这是第 2 章矿工 Bob 的"脚本版":矿工继承 Entity,身上带金块数、疲劳度,
//  以及一个"脚本化状态机"指针 m_pStateMachine。与第 2 章不同:状态不再用 C++ 类写,
//  而是写在 Lua 脚本 StateMachineScript.lua 里,C++ 只负责每帧驱动它。
//
//【谁在使用这个文件?】Miner.cpp(实现);main.cpp(用 luabind 把矿工方法注册给 Lua)。
//【本文件包含了谁?】Entity.h(父类)、ScriptedStateMachine.h(状态机模板类)。
//==============================================================================================
#ifndef MINER_H
#define MINER_H
#pragma warning (disable : 4786)
//------------------------------------------------------------------------
//
//  Name:   Miner.h
//
//  Desc:   A class defining a goldminer. The miner has a FSM defined
//          by a Lua script.
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <string>
#include <cassert>

#include "Entity.h"
#include "ScriptedStateMachine.h"




//the amount of nuggets a miner can carry
const int MaxNuggets         = 3;

//above this value a miner is sleepy
const int TirednessThreshold = 2;
//(原文注释:超过这个值矿工就困了)
// MaxNuggets=口袋最多装几块金;TirednessThreshold=疲劳超过几就该回家睡觉。



// class Miner : public Entity —— 矿工继承自 Entity(是一种游戏对象)。
class Miner : public Entity
{
private:

  ScriptedStateMachine<Miner>* m_pStateMachine;

  //how many nuggets the miner has in his pockets
  int                   m_iGoldCarried;

  //the higher the value, the more tired the miner
  int                   m_iFatigue;
// m_pStateMachine:指向"脚本化状态机"对象的指针(模板参数填 Miner,表示管的是矿工);
// m_iGoldCarried:兜里金块数;m_iFatigue:疲劳值。

public:

  Miner(std::string name);

  ~Miner(){delete m_pStateMachine;}

  //this must be implemented
  void Update();

  int           GoldCarried()const{return m_iGoldCarried;}
  void          SetGoldCarried(int val){m_iGoldCarried = val;}
  void          AddToGoldCarried(int val);

  bool          Fatigued()const;
  void          DecreaseFatigue(){m_iFatigue -= 1;}
  void          IncreaseFatigue(){m_iFatigue += 1;}

// public 接口:构造/析构、Update(每帧)、金块增减、疲劳增减、查询疲劳;
// GetFSM() 返回状态机指针,Lua 脚本通过它 ChangeState(切换状态)。
  ScriptedStateMachine<Miner>* GetFSM()const{return m_pStateMachine;}
};



#endif
