//==============================================================================================
//【文件说明】MinerOwnedStates.h —— 矿工的 4 个"具体状态"类声明(状态机的角色表)
//
//【这个文件是干什么的?】
//  把矿工的人生拆成 4 个状态类,每个类都继承 State.h 的 State 接口,
//  各自实现 Enter(进入)/Execute(每步)/Exit(离开)三个函数:
//    ① EnterMineAndDigForNugget —— 进金矿挖金子(矿工的"上班"状态)
//    ② VisitBankAndDepositGold  —— 去银行把金块换成存款
//    ③ GoHomeAndSleepTilRested  —— 回家睡觉解乏(矿工出生时的初始状态)
//    ④ QuenchThirst             —— 去酒吧买威士忌解渴
//
//【状态切换全景图】(切换代码在 MinerOwnedStates.cpp,切换流程在 Miner::ChangeState)
//      ③睡觉 ──睡醒(疲劳≤5)──▶ ①挖矿 ◀──喝完酒────── ④喝酒 ◀──口渴(≥5)──┐
//       ▲                            ││                                  │
//       │                       口袋满(≥3)→ ②银行                       │
//       └──存款≥5(ComfortLevel)────┘└──存款<5:回①挖矿──────────────────┘
//  【口诀】挖满 3 块去银行;存款 5 块回家睡;口渴 5 点去喝酒;
//          睡醒回矿、喝完回矿、没存够也回矿。
//
//【与相关文件的关系】
//  State.h              —— 4 个类的"共同父类":必须先包含它,才能写继承;
//  class Miner;(下面)   —— 本文件同样只对 Miner 做"前置声明",因为三个接口
//                           函数的参数是"Miner 指针",指针不需要完整定义;
//  MinerOwnedStates.cpp  —— 4 个类的具体实现(台词、干活、切换逻辑全在那边);
//  Miner.h / Miner.cpp   —— 矿工用成员 m_pCurrentState 指向这些状态;构造函数
//                           把初始状态设为 ③;切换入口是 Miner::ChangeState()。
//
//【设计模式:单例(Singleton)——每个状态全游戏只造一个对象】
//  观察下面 4 个类:它们"有函数、无成员变量"——状态本身不保存任何数据,
//  矿工的数据全在 Miner 对象里。所以"挖矿状态"这种东西,全局只需要一份,
//  谁来挖都复用同一个状态对象即可(省内存,也方便管理)。
//  实现单例的固定三件套(4 个类的写法完全相同,以①为详解):
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
//  Desc:   All the states that can be assigned to the Miner class
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:MinerOwnedStates.h
//   描述  :All the states that can be assigned to the Miner class
//          —— 能分配给"矿工"类的所有状态(即矿工专属的一组状态)
//   作者  :Mat Buckland,2002 年(本书作者)
//------------------------------------------------------------------------

// State 接口的完整定义:下面 4 个类都要写": public State"继承它,
// 编译器必须先见到父类的完整定义。
#include "State.h"


// 前置声明:只报"Miner 是个类"的名字,不给内容(参数只用 Miner* 指针)。
// Miner 的完整定义在 Miner.h,由实现文件 MinerOwnedStates.cpp 去包含。
class Miner;


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
// ": public State" —— 公有继承 State 接口:必须把父类的三个纯虚函数
// (Enter/Execute/Exit)全部实现,否则本类也是抽象类、无法创建对象。
class EnterMineAndDigForNugget : public State
{
private:

  // 私有构造函数:名字同类名、带空参数表"()"、函数体为空"{}"。
  // 写在 private: 区,外界(别的代码)就写不出"EnterMineAndDigForNugget xxx;"
  // 也 new 不出对象——想拿本类的对象,只能走下面的静态函数 Instance()。
  // 这是单例三件套的第 1 件。
  EnterMineAndDigForNugget(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  // 拷贝构造函数:参数是"const 本类&"(另一个同类对象的引用,const 表示
  // 只读借用不修改),作用是"照着入参对象复制出一个新对象"。
  // 声明在 private: 区且不提供实现 → 谁想复制唯一实例,编译器直接报错。
  // 这是单例三件套的第 2 件。& 读作"引用"(别名,给对象另起名字)。
  EnterMineAndDigForNugget(const EnterMineAndDigForNugget&);
  // 赋值运算符 operator=:把"="右边对象的内容复制给左边。同样私有化禁用,
  // 防止唯一实例被别的对象覆盖内容。
  EnterMineAndDigForNugget& operator=(const EnterMineAndDigForNugget&);

public:

  //this is a singleton
  //(原文注释:这是一个单例)

  // 静态成员函数:不属于某个对象,直接用"类名::Instance()"调用
  // (如 EnterMineAndDigForNugget::Instance())。返回本类型指针,
  // 指向全局唯一实例(实现见 MinerOwnedStates.cpp)。
  // 这是单例三件套的第 3 件——全世界获取本对象的唯一入口。
  static EnterMineAndDigForNugget* Instance();
  
  // 下面三个虚函数逐个实现(override/覆写)父类 State 的纯虚函数:
  // 进入本状态时执行一次(矿工赶路进金矿 + 开场白)。
  // virtual 在子类里可省略(父类已标),保留它是醒目写法。
  virtual void Enter(Miner* miner);

  // 处于本状态期间,矿工每 Update() 一次执行一次(挖金块 + 判断是否切换)。
  virtual void Execute(Miner* miner);

  // 离开本状态时执行一次(告别金矿)。
  virtual void Exit(Miner* miner);
};

//------------------------------------------------------------------------
//
//  Entity will go to a bank and deposit any nuggets he is carrying. If the 
//  miner is subsequently wealthy enough he'll walk home, otherwise he'll
//  keep going to get more gold
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【状态②:去银行存金子】:
//   实体(矿工)会去银行,把随身携带的金块全部存起来。存完若足够富有
//   (存款 ≥ ComfortLevel=5)就回家,否则继续去挖更多金子。
//------------------------------------------------------------------------
// 单例三件套与状态①完全相同(私有构造、私有拷贝/赋值、静态 Instance),
// 原理详见上面 EnterMineAndDigForNugget 中的注释,下面不再逐行展开。
class VisitBankAndDepositGold : public State
{
private:

  // 私有构造:禁止外界随意创建(单例第 1 件)。
  VisitBankAndDepositGold(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  // 私有拷贝构造 + 私有赋值:禁止复制(单例第 2 件)。
  VisitBankAndDepositGold(const VisitBankAndDepositGold&);
  VisitBankAndDepositGold& operator=(const VisitBankAndDepositGold&);
  
public:

  //this is a singleton
  //(原文注释:这是一个单例)

  // 静态取唯一实例的入口(单例第 3 件)。
  static VisitBankAndDepositGold* Instance();

  // 实现 State 接口的三个动作:
  // 进入银行状态时执行一次(走去银行)。
  virtual void Enter(Miner* miner);

  // 每步执行:存金块、看存款决定"回家睡觉"还是"回金矿继续挖"。
  virtual void Execute(Miner* miner);

  // 离开银行时执行一次(告别)。
  virtual void Exit(Miner* miner);
};


//------------------------------------------------------------------------
//
//  miner will go home and sleep until his fatigue is decreased
//  sufficiently
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【状态③:回家睡觉到睡够】:
//   矿工会回家睡觉,直到疲劳值降到足够低(睡醒)为止。
//   注意:矿工出生时被设为这个状态(Miner 构造函数里
//   m_pCurrentState(GoHomeAndSleepTilRested::Instance()))——
//   程序一开场,矿工正"睡在小屋",因为疲劳是 0,他立刻睡醒去挖矿。
//------------------------------------------------------------------------
// 单例三件套同上(详见状态①的注释)。
class GoHomeAndSleepTilRested : public State
{
private:

  GoHomeAndSleepTilRested(){}

    //copy ctor and assignment should be private
  GoHomeAndSleepTilRested(const GoHomeAndSleepTilRested&);
  GoHomeAndSleepTilRested& operator=(const GoHomeAndSleepTilRested&);

public:

  //this is a singleton
  //(原文注释:这是一个单例)

  static GoHomeAndSleepTilRested* Instance();

  // 实现 State 接口的三个动作:
  // 进入睡觉状态时执行一次(走回家)。
  virtual void Enter(Miner* miner);

  // 每步执行:睡一步疲劳 -1;疲劳降到不困(≤ TirednessThreshold)就回金矿。
  virtual void Execute(Miner* miner);

  // 离开家时执行一次(告别小屋)。
  virtual void Exit(Miner* miner);
};


//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
// ↓↓↓(原文此处只留了空的注释框,没有写说明文字)补上本文件风格的中文说明:
// 【状态④:去酒吧解渴】矿工口渴值达到 ThirstLevel(5)时进入本状态:
// 走进酒吧、买杯威士忌喝下(口渴清零、花 2 元),然后回金矿继续挖矿。
//------------------------------------------------------------------------
// 单例三件套同上(详见状态①的注释)。
class QuenchThirst : public State
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

  // 实现 State 接口的三个动作:
  // 进入解渴状态时执行一次(走去酒吧)。
  virtual void Enter(Miner* miner);

  // 每步执行:买酒喝下,喝完回金矿。
  virtual void Execute(Miner* miner);

  // 离开酒吧时执行一次(告别)。
  virtual void Exit(Miner* miner);
};





#endif