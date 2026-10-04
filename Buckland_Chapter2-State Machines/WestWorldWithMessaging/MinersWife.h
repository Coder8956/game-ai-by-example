//==============================================================================================
//【文件说明】MinersWife.h —— 矿工妻子 Elsa 的"人设档案"(类声明)
//
//【这个文件是干什么的?】
//  定义游戏里的第二个角色"矿工妻子 Elsa"。她比矿工多两个特点:
//    ① 有"全局状态"(WifesGlobalState)——每个时刻都要先跑一遍的"默认状态",
//       例如随机去上厕所的念头就写在全局状态里;
//    ② 有"在做饭吗?"这个开关(m_bCooking)——防止重复炖菜。
//
//【与相关文件的关系】
//  1. BaseGameEntity.h        —— 父类(角色共同点:ID、Update、HandleMessage);
//  2. fsm/State.h             —— 状态接口(State 模板类),状态文件要用;
//  3. Locations.h             —— 地点枚举(妻子的 m_Location 类型);
//  4. MinersWifeOwnedStates.h —— 妻子的状态类(做家务/上厕所/炖菜/全局状态);
//  5. Miner.h                 —— 丈夫的类(消息处理里需要知道丈夫的编号等);
//  6. fsm/StateMachine.h      —— 状态机模板(妻子的状态机);
//  7. misc/Utils.h            —— Common 的随机数工具(RandFloat/RandInt,
//                                 全局状态里"随机去厕所"要用);
//  8. MinersWife.cpp          —— 本文件声明的方法在那里实现;
//  9. main.cpp                —— 创建 Elsa 的地方:new MinersWife(ent_Elsa)。
//
//【丈夫 vs 妻子:两个角色的状态机差异】
//  矿工 Bob  :只设了"当前状态"(出生=回家睡觉),没有全局状态;
//  妻子 Elsa:既设"当前状态"(出生=做家务 DoHouseWork),
//            又设"全局状态"(WifesGlobalState)——每帧先跑全局、再跑当前。
//  这就是 StateMachine.h 里 m_pGlobalState 的用武之地。
//==============================================================================================

//------------------------------------------------------------------------------------------------
// 包含保护(原理详解见 Locations.h):防止本文件被重复包含。
//------------------------------------------------------------------------------------------------
#ifndef MINERSWIFE_H
#define MINERSWIFE_H
//------------------------------------------------------------------------
//
//  Name: MinersWife.h
//
//  Desc: class to implement Miner Bob's wife.
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:MinersWife.h
//   描述  :class to implement Miner Bob's wife —— 实现矿工 Bob 妻子的类
//   作者  :Mat Buckland,2003 年
//------------------------------------------------------------------------

// <string>:标准库字符串(保留是原作者习惯,本文件未直接用)。
#include <string>

// 状态接口:State<MinersWife> 类型定义在这里(妻子的状态类要继承它)。
// 注意路径:fsm/State.h 在 Common 共享目录,靠"附加包含目录 ..\..\Common"找到。
#include "fsm/State.h"
// 父类:要写": public BaseGameEntity"继承它。
#include "BaseGameEntity.h"
// 地点枚举:妻子也有 m_Location(她主要在小屋,偶尔去厕所)。
#include "Locations.h"
// 妻子的状态类声明:构造函数里要用 DoHouseWork::Instance()、
// WifesGlobalState::Instance()。
#include "MinersWifeOwnedStates.h"
// 控制台小工具(SetTextColor 等)。
#include "misc/ConsoleUtils.h"
// 丈夫的类:妻子要知道"矿工是谁"(处理消息时发消息给 ent_Miner_Bob)。
#include "Miner.h"
// 状态机模板:成员 m_pStateMachine 的类型 StateMachine<MinersWife>。
#include "fsm/StateMachine.h"
// Common 随机数工具:RandFloat()、RandInt() 在这里(全局状态里随机去厕所)。
#include "misc/Utils.h"



//--------------------------------------------------------------------------------
//【类 MinersWife : public BaseGameEntity】妻子类,公有继承父类。
//  与 Miner 的写法几乎一样,只是数据成员不同。
//--------------------------------------------------------------------------------
class MinersWife : public BaseGameEntity
{
// private: 私有区。
private:

  //an instance of the state machine class
  //(原文注释:状态机类的一个实例)

  // 【成员】m_pStateMachine —— 妻子的状态机指针(服务对象是 MinersWife)。
  StateMachine<MinersWife>* m_pStateMachine;

  // 【成员】m_Location —— 妻子当前地点(基本总在小屋)。
  location_type   m_Location;

  //is she presently cooking?
  //(原文注释:她现在在做饭吗?)

  // 【成员】m_bCooking —— "在做饭吗?"布尔开关。
  //   b 前缀:bool(布尔);true=正在炖菜,false=没有。
  //   作用:炖菜状态 Enter 时若发现已在炖,就不重复放菜、不重复发延迟消息。
  bool            m_bCooking;


// public: 公有区。
public:

  //--------------------------------------------------------------------------------
  //【构造函数】MinersWife(int id):m_Location(shack),
  //                               m_bCooking(false),
  //                               BaseGameEntity(id)
  //  初始化列表:出生在小屋、没在做饭、通过父类构造函数拿到唯一编号。
  //  函数体:搭好状态机,设置"当前状态=做家务"和"全局状态=全局状态"。
  //  (int id):外部传入的编号(main 里传 ent_Elsa=1)。
  //--------------------------------------------------------------------------------
  MinersWife(int id):m_Location(shack),
                     m_bCooking(false),
                     BaseGameEntity(id)
                                        
  {
    //set up the state machine
    //(原文注释:搭建状态机)

    // new 出妻子的状态机,参数 this 指向妻子自己。
    m_pStateMachine = new StateMachine<MinersWife>(this);

    // 设置当前状态 = 做家务(DoHouseWork):不干活时就扫地、洗碗、铺床。
    m_pStateMachine->SetCurrentState(DoHouseWork::Instance());

    // 设置全局状态 = WifesGlobalState:每帧最先执行,
    // 负责"有 10% 概率想去上厕所"这类时刻都该考虑的事。
    m_pStateMachine->SetGlobalState(WifesGlobalState::Instance());
  }

  // 【析构函数】释放 new 出来的状态机内存。
  ~MinersWife(){delete m_pStateMachine;}


  //this must be implemented
  //(原文注释:这个方法必须实现)

  // 【重写父类纯虚函数】void Update(); —— 妻子的心跳(实现见 MinersWife.cpp)。
  void          Update();

  //so must this
  //(原文注释:这个也必须实现)

  // 【重写父类纯虚函数】virtual bool HandleMessage(const Telegram& msg);
  //   收消息入口(转发给状态机,先问当前状态再问全局状态)。
  virtual bool  HandleMessage(const Telegram& msg);

  // 【取状态机】供状态类使用(调用 ChangeState、isInState 等)。
  StateMachine<MinersWife>* GetFSM()const{return m_pStateMachine;}

  //----------------------------------------------------accessors
  //(原文注释:以下是一批存取器)

  // 【读地点】Location()const —— 返回妻子当前地点。
  location_type Location()const{return m_Location;}
  // 【改地点】ChangeLocation(location_type loc) —— 设置妻子地点。
  void          ChangeLocation(location_type loc){m_Location=loc;}

  // 【在做饭吗?】Cooking()const —— 返回 m_bCooking 的值。
  bool          Cooking()const{return m_bCooking;}
  // 【设置做饭开关】SetCooking(bool val) —— 炖菜状态进出时开/关。
  void          SetCooking(bool val){m_bCooking = val;}
   
};

// 包含保护结束:与文件开头的 #ifndef MINERSWIFE_H 配对。
#endif
