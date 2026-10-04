//==============================================================================================
//【文件说明】Miner.h —— 矿工 Bob 的"人设档案"(类声明)
//
//【这个文件是干什么的?】
//  定义游戏主角"矿工 Bob"这个类。一个角色 = 一堆数据(位置、金块、存款、口渴、
//  疲劳、当前状态)+ 一堆行为(更新、换状态、收消息、各种小动作)。
//  本文件只写"档案"(声明:有哪些数据、哪些函数),具体行为写在 Miner.cpp。
//
//【与相关文件的关系】
//  1. BaseGameEntity.h      —— 被继承的父类(角色共同点:ID、Update、HandleMessage);
//  2. Locations.h           —— m_Location 成员的类型 location_type(地点枚举)在这里;
//  3. MinerOwnedStates.h    —— 矿工的所有状态类(挖矿/存钱/睡觉/喝酒/吃炖菜)声明;
//  4. fsm/StateMachine.h    —— Common 共享的"状态机"模板类(管理当前/上一个/全局状态);
//  5. Miner.cpp             —— 本文件声明的方法在那里实现;
//  6. main.cpp              —— 创建 Bob 的地方:new Miner(ent_Miner_Bob);
//  7. MinerOwnedStates.cpp  —— 状态类通过 Miner* 指针操作矿工数据(需包含本文件)。
//
//【状态机小课堂:矿工 = 数据 + 一台状态机】
//  矿工自己并不直接写"下一步做什么",而是把决策权交给成员 m_pStateMachine
//  (StateMachine 模板类):矿工只提供数据(位置/金块/口渴…),状态机负责
//  根据当前状态调用对应的"行为函数"。详见 MinerOwnedStates.h 的状态切换全景图。
//==============================================================================================

//------------------------------------------------------------------------------------------------
// 包含保护(原理详解见 Locations.h):防止本文件被重复包含。
//------------------------------------------------------------------------------------------------
#ifndef MINER_H
#define MINER_H
//------------------------------------------------------------------------
//
//  Name:   Miner.h
//
//  Desc:   A class defining a goldminer.
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:Miner.h
//   描述  :A class defining a goldminer —— 定义矿工的类
//   作者  :Mat Buckland,2002 年
//------------------------------------------------------------------------

// <string>:标准库字符串(本文件未直接用,保留是原作者习惯)。
#include <string>
// <cassert>:断言宏(StateMachine.h 内部用,这里保留也是原作者写法)。
#include <cassert>
// <iostream>:标准输入输出流(提供 cout 打印,状态实现文件里会用)。
#include <iostream>

// 父类头文件:要写": public BaseGameEntity"继承它,必须先见到它的完整定义。
#include "BaseGameEntity.h"
// 地点枚举头文件:下面成员 m_Location 的类型 location_type 定义在这里。
#include "Locations.h"
// 控制台小工具(文字颜色 SetTextColor 等),Common\misc\ 目录。
#include "misc/ConsoleUtils.h"
// 矿工的状态类声明:下面构造函数里要用 GoHomeAndSleepTilRested::Instance()。
#include "MinerOwnedStates.h"
// 状态机模板类:成员 m_pStateMachine 的类型 StateMachine<Miner> 来自这里。
#include "fsm/StateMachine.h"

// 【模板前向声明】template <class entity_type> class State;
//   告诉编译器"存在一个模板类 State,实体类型由尖括号参数指定"。
//   有了这行,下面声明参数 State<Miner>* 时编译器才认识 State。
//   真正的定义在 Common\FSM\State.h(由 MinerOwnedStates.h 包含进来)。
template <class entity_type> class State; //pre-fixed with "template <class entity_type> " for vs8 compatibility

// 【结构体前向声明】struct Telegram;
//   消息"信封"结构。只声明名字,不给出内容(指针参数只需要知道类型存在)。
//   完整定义在 Common\Messaging\Telegram.h。
struct Telegram;

//the amount of gold a miner must have before he feels he can go home
//(原文注释:矿工觉得"可以回家了"所需的存款量)

//--------------------------------------------------------------------------------
//【全局常量 const】const int ComfortLevel = 5;
//  const:常量,值一旦定下就不能修改;
//  不在类里、在文件顶部 → 全局常量,本文件包含它的地方都能用;
//  语义:存款达到 5 块,矿工就满足,决定回家休息(见 VisitBankAndDepositGold 状态)。
//--------------------------------------------------------------------------------
const int ComfortLevel       = 5;
//the amount of nuggets a miner can carry
//(原文注释:矿工最多能携带的金块数)

// MaxNuggets = 3:口袋最多装 3 块天然金块,装满就得去银行存。
const int MaxNuggets         = 3;
//above this value a miner is thirsty
//(原文注释:口渴值超过这个数,矿工就渴了)

// ThirstLevel = 5:口渴值达到 5,矿工就丢下矿铲去酒吧(见 QuenchThirst 状态)。
const int ThirstLevel        = 5;
//above this value a miner is sleepy
//(原文注释:疲劳值超过这个数,矿工就困了)

// TirednessThreshold = 5:疲劳值超过 5,矿工就回家睡觉(见 GoHomeAndSleepTilRested 状态)。
const int TirednessThreshold = 5;



//--------------------------------------------------------------------------------
//【类 Miner : public BaseGameEntity】定义矿工类,公有继承父类 BaseGameEntity。
//  继承的含义:矿工自动获得父类的 ID、Update 接口、HandleMessage 接口,
//  下面只需补充"矿工自己的"数据和函数。
//--------------------------------------------------------------------------------
class Miner : public BaseGameEntity
{
// private: 私有区——只有矿工自己能碰这些数据。
private:

  //an instance of the state machine class
  //(原文注释:状态机类的一个实例)

  // 【成员】m_pStateMachine —— 指向矿工状态机的指针。
  //   类型 StateMachine<Miner>*:给 Miner 用的状态机(尖括号里是"谁的状态机")。
  //   m_ 前缀:成员变量;p 前缀:pointer(指针)。
  //   矿工的行为决策全交给它:Update 时它调用当前状态的 Execute,
  //   消息来时它先问当前状态、再问全局状态。
  StateMachine<Miner>*  m_pStateMachine;
  
  // 【成员】m_Location —— 矿工当前所在地点。
  //   类型 location_type(来自 Locations.h 的枚举:shack/goldmine/bank/saloon)。
  location_type         m_Location;

  //how many nuggets the miner has in his pockets
  //(原文注释:矿工口袋里有多少金块)

  // 【成员】m_iGoldCarried —— 口袋里随身携带的天然金块数量(0~MaxNuggets)。
  //   i 前缀:int(整数)。
  int                   m_iGoldCarried;

  // 【成员】m_iMoneyInBank —— 银行里的存款(卖掉金块换来的钱)。
  int                   m_iMoneyInBank;

  //the higher the value, the thirstier the miner
  //(原文注释:值越大,矿工越渴)

  // 【成员】m_iThirst —— 口渴值(每 Update 一次 +1;到 ThirstLevel=5 就想喝酒)。
  int                   m_iThirst;

  //the higher the value, the more tired the miner
  //(原文注释:值越大,矿工越累)

  // 【成员】m_iFatigue —— 疲劳值(挖矿 +1;睡觉 -1;超过 TirednessThreshold=5 就困)。
  int                   m_iFatigue;

// public: 公有区——外部(main、状态类)可以调用。
public:

  //--------------------------------------------------------------------------------
  //【构造函数】Miner(int id):m_Location(shack), ... , BaseGameEntity(id)
  //  创建矿工对象时自动执行,做三件事:
  //    a) 用"初始化列表"(冒号后面的一串)给成员赋初值:
  //       m_Location(shack)     —— 出生在小屋;
  //       m_iGoldCarried(0)     —— 口袋空空;
  //       m_iMoneyInBank(0)     —— 存款为零;
  //       m_iThirst(0)          —— 不渴;
  //       m_iFatigue(0)         —— 不累;
  //       BaseGameEntity(id)    —— 先调用父类构造函数,给矿工分配唯一编号。
  //       (初始化列表语法:成员名(初值),多个用逗号分隔;
  //       比在函数体里赋值更早执行,是 C++ 推荐的初始化方式。)
  //    b) 函数体里 new 出状态机、设置初始状态。
  //  (int id):外部传入的编号(main 里传 ent_Miner_Bob=0)。
  //--------------------------------------------------------------------------------
  Miner(int id):m_Location(shack),
                          m_iGoldCarried(0),
                          m_iMoneyInBank(0),
                          m_iThirst(0),
                          m_iFatigue(0),
                          BaseGameEntity(id)
                               
  {
    //set up state machine
    //(原文注释:搭建状态机)

    // new StateMachine<Miner>(this):在堆上创建状态机对象,
    // 参数 this 是"指向矿工自己的指针"——状态机需要知道自己服务谁。
    // new 出来的对象要记得在析构函数里 delete(见下面 ~Miner)。
    m_pStateMachine = new StateMachine<Miner>(this);
    
    // SetCurrentState(...):设置初始状态 = 回家睡觉。
    // GoHomeAndSleepTilRested::Instance():拿到该状态类的唯一实例(单例)。
    // 即矿工一出生就在家睡觉,睡醒才开始挖矿人生。
    m_pStateMachine->SetCurrentState(GoHomeAndSleepTilRested::Instance());

    /* NOTE, A GLOBAL STATE HAS NOT BEEN IMPLEMENTED FOR THE MINER */
    //(原文注释:注意,矿工没有实现全局状态。
    //  与妻子不同——妻子有全局状态 WifesGlobalState,矿工没有。)
  }

  // 【析构函数】~Miner(){delete m_pStateMachine;}
  //   对象销毁时自动调用;delete 释放 new 出来的状态机内存,防止泄漏。
  ~Miner(){delete m_pStateMachine;}

  //this must be implemented
  //(原文注释:这个方法必须实现)

  // 【重写父类纯虚函数】void Update();
  //   矿工的"心跳":父类规定所有角色必须有,矿工给出自己的实现(见 Miner.cpp):
  //   口渴 +1 → 让状态机更新 → 当前状态执行它的动作。
  void Update();

  //so must this
  //(原文注释:这个也必须实现)

  // 【重写父类纯虚函数】virtual bool HandleMessage(const Telegram& msg);
  //   矿工收消息的入口(转发给状态机处理)。virtual 可省(父类已标虚)。
  virtual bool  HandleMessage(const Telegram& msg);

  
  // 【取状态机】StateMachine<Miner>* GetFSM()const{return m_pStateMachine;}
  //   给外部(状态类)提供访问矿工状态机的窗口,以便调用 ChangeState 等。
  //   const:本函数不修改矿工数据;函数体直接返回成员指针。
  StateMachine<Miner>* GetFSM()const{return m_pStateMachine;}


  
  //-------------------------------------------------------------accessors
  //(原文注释:以下是一批"存取器"——读取/修改矿工私有数据的公有小函数)

  // 【读地点】Location()const{return m_Location;}
  //   返回矿工当前所在地点(状态类用它判断要不要换地方)。
  location_type Location()const{return m_Location;}
  // 【改地点】ChangeLocation(location_type loc){m_Location=loc;}
  //   把矿工搬到新地点(状态类进入新地点时调用)。
  void          ChangeLocation(location_type loc){m_Location=loc;}
    
  // 【读金块】GoldCarried()const —— 返回口袋里的金块数。
  int           GoldCarried()const{return m_iGoldCarried;}
  // 【设金块】SetGoldCarried(int val) —— 直接设置口袋金块数(如存完钱清零)。
  void          SetGoldCarried(int val){m_iGoldCarried = val;}
  // 【加金块】AddToGoldCarried(int val) —— 增加金块数(实现见 Miner.cpp,
  //   有"不得为负"的保护)。挖矿时调用 AddToGoldCarried(1)。
  void          AddToGoldCarried(int val);
  // 【口袋满了吗?】PocketsFull()const{return m_iGoldCarried >= MaxNuggets;}
  //   比较运算符 >= 返回 bool;金块数达到上限(3)就返回 true。
  bool          PocketsFull()const{return m_iGoldCarried >= MaxNuggets;}

  // 【累了吗?】Fatigued()const —— 疲劳值是否超过阈值(实现见 Miner.cpp)。
  bool          Fatigued()const;
  // 【减疲劳】DecreaseFatigue(){m_iFatigue -= 1;}
  //   -= 是"自减赋值":m_iFatigue = m_iFatigue - 1 的简写。睡觉时调用。
  void          DecreaseFatigue(){m_iFatigue -= 1;}
  // 【增疲劳】IncreaseFatigue(){m_iFatigue += 1;}
  //   += 是"自增赋值"。挖矿时调用。
  void          IncreaseFatigue(){m_iFatigue += 1;}

  // 【读存款】Wealth()const —— 返回银行里的存款。
  int           Wealth()const{return m_iMoneyInBank;}
  // 【设存款】SetWealth(int val) —— 直接设置存款。
  void          SetWealth(int val){m_iMoneyInBank = val;}
  // 【加存款】AddToWealth(int val) —— 增加存款(实现见 Miner.cpp,同样防负)。
  void          AddToWealth(int val);

  // 【渴了吗?】Thirsty()const —— 口渴值是否达到阈值(实现见 Miner.cpp)。
  bool          Thirsty()const; 
  // 【买酒喝】BuyAndDrinkAWhiskey(){m_iThirst = 0; m_iMoneyInBank-=2;}
  //   一口气喝完:口渴清零,存款扣 2 块(威士忌的价格)。
  //   两个语句用分号分隔写在同一函数体里。
  void          BuyAndDrinkAWhiskey(){m_iThirst = 0; m_iMoneyInBank-=2;}

};



// 包含保护结束:与文件开头的 #ifndef MINER_H 配对。
#endif
