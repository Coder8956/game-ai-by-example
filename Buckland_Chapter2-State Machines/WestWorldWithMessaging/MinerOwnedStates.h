//==============================================================================================
//【文件说明】MinerOwnedStates.h —— 矿工的 5 个"具体状态"类声明(状态机的角色表)
//
//【这个文件是干什么的?】
//  把矿工的人生拆成 5 个状态类,每个类都继承 State<Miner> 接口,
//  各自实现 Enter(进入)/Execute(每步)/Exit(离开)/OnMessage(收消息):
//    ① EnterMineAndDigForNugget —— 进金矿挖金子(矿工的"上班"状态)
//    ② VisitBankAndDepositGold  —— 去银行把金块换成存款
//    ③ GoHomeAndSleepTilRested  —— 回家睡觉解乏(矿工出生时的初始状态)
//    ④ QuenchThirst             —— 去酒吧买威士忌解渴
//    ⑤ EatStew                  —— 吃妻子炖好的菜(收到"炖菜好了"消息才进入)
//
//【状态切换全景图】(切换代码在 MinerOwnedStates.cpp,切换流程在 StateMachine.h)
//      ③睡觉 ──睡醒(疲劳≤5)──▶ ①挖矿 ◀──喝完酒────── ④喝酒 ◀──口渴(≥5)──┐
//       ▲                            ││                                  │
//       │                       口袋满(≥3)→ ②银行                       │
//       └──存款≥5(ComfortLevel)────┘└──存款<5:回①挖矿──────────────────┘
//       │                                                            │
//       └──收到"炖菜好了"消息 → ⑤吃炖菜 ──吃完 → 回到上一个状态 ────┘
//  【口诀】挖满 3 块去银行;存款 5 块回家睡;口渴 5 点去喝酒;
//          睡醒回矿、喝完回矿、没存够也回矿;老婆喊吃饭就先去吃。
//
//【与相关文件的关系】
//  fsm/State.h          —— 5 个类的"共同父类":必须先包含它,才能写继承;
//  class Miner;(下面)   —— 本文件只对 Miner 做"前置声明",因为接口函数的
//                           参数是"Miner 指针",指针只需要知道类型存在;
//  struct Telegram;(下面)—— 消息"信封"的前置声明(参数 const Telegram& 需要);
//  MinerOwnedStates.cpp  —— 5 个类的具体实现(台词、干活、切换逻辑全在那边);
//  Miner.h / Miner.cpp   —— 矿工用成员 m_pStateMachine 指向这些状态;构造函数
//                           把初始状态设为 ③;消息入口是 Miner::HandleMessage()。
//
//【设计模式:单例(Singleton)——每个状态全游戏只造一个对象】
//  观察下面 5 个类:它们"有函数、无成员变量"——状态本身不保存任何数据,
//  矿工的数据全在 Miner 对象里。所以"挖矿状态"这种东西,全局只需要一份,
//  谁来挖都复用同一个状态对象即可(省内存,也方便管理)。
//  实现单例的固定三件套(5 个类的写法完全相同,以①为详解):
//    1) 构造函数私有(写在 private: 区)→ 外界不能创建第二份;
//    2) 拷贝构造函数、赋值运算符私有 → 禁止复制已有实例;
//    3) 公有的静态函数 Instance() → 全世界都用"类名::Instance()"拿唯一实例
//       (Instance() 的实现藏在 MinerOwnedStates.cpp 的函数内 static 局部变量里)。
//==============================================================================================

//------------------------------------------------------------------------------------------------
// 包含保护(原理详解见 Locations.h):防止本文件被重复包含。
//------------------------------------------------------------------------------------------------
#ifndef MINER_OWNED_STATES_H
#define MINER_OWNED_STATES_H
//------------------------------------------------------------------------
//
//  Name:   MinerOwnedStates.h
//
//  Desc:   All the states that can be assigned to the Miner class.
//          Note that a global state has not been implemented.
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:MinerOwnedStates.h
//   描述  :All the states that can be assigned to the Miner class
//          —— 能分配给"矿工"类的所有状态(即矿工专属的一组状态)。
//          Note that a global state has not been implemented
//          —— 注意:矿工没有实现全局状态(与妻子不同)。
//   作者  :Mat Buckland,2002 年(本书作者)
//------------------------------------------------------------------------

// State 接口的完整定义:下面 5 个类都要写": public State<Miner>"继承它,
// 编译器必须先见到父类的完整定义。路径 fsm/ 在 Common 共享目录。
#include "fsm/State.h"


// 【前置声明】class Miner;
//   只报"Miner 是个类"的名字,不给内容(参数只用 Miner* 指针)。
//   Miner 的完整定义在 Miner.h,由实现文件 MinerOwnedStates.cpp 去包含。
class Miner;
// 【前置声明】struct Telegram;
//   消息"信封"结构的前置声明(OnMessage 的参数要用)。
//   完整定义在 Common\Messaging\Telegram.h。
struct Telegram;




//------------------------------------------------------------------------
//
//  In this state the miner will walk to a goldmine and pick up a nugget
//  of gold. If the miner already has a nugget of gold he'll change state
//  to VisitBankAndDepositGold. If he gets thirsty he'll change state
//  to QuenchThirst
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【状态①:进金矿挖金子】:
//   此状态下,矿工会走到金矿并捡起一块天然金块。口袋装满后切换到
//   "去银行存钱"(VisitBankAndDepositGold)状态;挖矿期间若口渴了,
//   则切换到"解渴"(QuenchThirst)状态。
//------------------------------------------------------------------------
// ": public State<Miner>" —— 公有继承 State 接口(模板参数 Miner 表示
// 这个状态服务的实体类型是矿工):必须把父类的四个纯虚函数
// (Enter/Execute/Exit/OnMessage)全部实现,否则本类也是抽象类、无法创建对象。
class EnterMineAndDigForNugget : public State<Miner>
{
private:
  
  // 【私有构造函数】EnterMineAndDigForNugget(){}
  //   名字同类名、空参数表、空函数体。写在 private: 区,外界就写不出
  //   "EnterMineAndDigForNugget xxx;" 也 new 不出对象——想拿本类的对象,
  //   只能走下面的静态函数 Instance()。这是单例三件套的第 1 件。
  EnterMineAndDigForNugget(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  // 拷贝构造函数(私有、无实现)→ 禁止复制唯一实例。单例三件套第 2 件。
  EnterMineAndDigForNugget(const EnterMineAndDigForNugget&);
  // 赋值运算符(私有、无实现)→ 禁止用 = 覆盖唯一实例。
  EnterMineAndDigForNugget& operator=(const EnterMineAndDigForNugget&);
 
public:

  //this is a singleton
  //(原文注释:这是一个单例)

  // 【静态函数】static EnterMineAndDigForNugget* Instance();
  //   返回本状态唯一实例的指针,实现见 MinerOwnedStates.cpp。
  //   单例三件套第 3 件——全世界拿本对象的唯一入口。
  static EnterMineAndDigForNugget* Instance();

  // 【覆写父类纯虚函数】下面 4 个函数逐个实现:
  //   Enter    —— 进入本状态时执行一次(矿工赶路进金矿 + 开场白);
  //   Execute  —— 每一帧(每次 Update)执行(挖一块金、判断是否切换);
  //   Exit     —— 离开本状态时执行一次(说告别台词);
  //   OnMessage—— 收到消息时被调用,返回 true=处理了,false=不处理。
  //  virtual 在子类里可省略(父类已标虚),保留它是醒目写法。
  virtual void Enter(Miner* miner);

  virtual void Execute(Miner* miner);

  virtual void Exit(Miner* miner);

  virtual bool OnMessage(Miner* agent, const Telegram& msg);

};

//------------------------------------------------------------------------
//
//  Entity will go to a bank and deposit any nuggets he is carrying. If the 
//  miner is subsequently wealthy enough he'll walk home, otherwise he'll
//  keep going to get more gold
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【状态②:去银行存钱】:
//   角色会去银行,存下随身携带的金块。如果存款足够多(达到 ComfortLevel=5),
//   就回家;否则继续去挖更多的金子。
//------------------------------------------------------------------------
class VisitBankAndDepositGold : public State<Miner>
{
private:
  
  VisitBankAndDepositGold(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  VisitBankAndDepositGold(const VisitBankAndDepositGold&);
  VisitBankAndDepositGold& operator=(const VisitBankAndDepositGold&);
 
public:

  //this is a singleton
  //(原文注释:这是一个单例)

  static VisitBankAndDepositGold* Instance();

  virtual void Enter(Miner* miner);

  virtual void Execute(Miner* miner);

  virtual void Exit(Miner* miner);

  virtual bool OnMessage(Miner* agent, const Telegram& msg);
};


//------------------------------------------------------------------------
//
//  miner will go home and sleep until his fatigue is decreased
//  sufficiently
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【状态③:回家睡觉】:
//   矿工会回家睡觉,直到疲劳值降到足够低。
//   注意:这是矿工的"初始状态"(出生就在家睡);
//   进入本状态时会发消息"亲爱的我回来了"给妻子 Elsa。
//------------------------------------------------------------------------
class GoHomeAndSleepTilRested : public State<Miner>
{
private:
  
  GoHomeAndSleepTilRested(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  GoHomeAndSleepTilRested(const GoHomeAndSleepTilRested&);
  GoHomeAndSleepTilRested& operator=(const GoHomeAndSleepTilRested&);
 
public:

  //this is a singleton
  //(原文注释:这是一个单例)

  static GoHomeAndSleepTilRested* Instance();

  virtual void Enter(Miner* miner);

  virtual void Execute(Miner* miner);

  virtual void Exit(Miner* miner);

  virtual bool OnMessage(Miner* agent, const Telegram& msg);
};


//------------------------------------------------------------------------
//
//  miner changes location to the saloon and keeps buying Whiskey until
//  his thirst is quenched. When satisfied he returns to the goldmine
//  and resumes his quest for nuggets.
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【状态④:去酒吧解渴】:
//   矿工把地点换到酒吧,不停地买威士忌直到不渴了。
//   解渴完成后回到金矿,继续他的挖金大业。
//------------------------------------------------------------------------
class QuenchThirst : public State<Miner>
{
private:
  
  QuenchThirst(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  QuenchThirst(const QuenchThirst&);
  QuenchThirst& operator=(const QuenchThirst&);
 
public:

  //this is a singleton
  //(原文注释:这是一个单例)

  static QuenchThirst* Instance();

  virtual void Enter(Miner* miner);

  virtual void Execute(Miner* miner);

  virtual void Exit(Miner* miner);

  virtual bool OnMessage(Miner* agent, const Telegram& msg);
};


//------------------------------------------------------------------------
//
//  this is implemented as a state blip. The miner eats the stew, gives
//  Elsa some compliments and then returns to his previous state
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【状态⑤:吃炖菜】:
//   这是一个"状态闪回"(state blip):矿工吃炖菜、夸夸 Elsa,
//   然后立刻回到他之前的状态(靠 RevertToPreviousState 实现)。
//------------------------------------------------------------------------
class EatStew : public State<Miner>
{
private:
  
  EatStew(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  EatStew(const EatStew&);
  EatStew& operator=(const EatStew&);
 
public:

  //this is a singleton
  //(原文注释:这是一个单例)

  static EatStew* Instance();

  virtual void Enter(Miner* miner);

  virtual void Execute(Miner* miner);

  virtual void Exit(Miner* miner);

  virtual bool OnMessage(Miner* agent, const Telegram& msg);
};




// 包含保护结束:与文件开头的 #ifndef MINER_OWNED_STATES_H 配对。
#endif