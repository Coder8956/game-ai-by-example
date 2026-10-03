//==============================================================================================
//【文件说明】MinersWife.h —— "矿工妻子"类的声明:艾尔莎(Elsa)的数据与行为清单
//
//【这个文件是干什么的?】
//  定义 class MinersWife(矿工妻子类)——第二个登场的角色"艾尔莎"的图纸。
//  与矿工 Miner 相比她"无欲无求":没有金块/存款/口渴/疲劳,只有两样东西:
//    ▸ 数据(成员变量):当前地点 m_Location + 一台"状态机"指针 m_pStateMachine;
//    ▸ 行为(成员函数):Update()(每步更新,实现在 MinersWife.cpp)、
//      GetFSM()(取出状态机)和一对读写地点的小工具。
//  她的人生被拆成 3 个状态(在 MinersWifeOwnedStates.h/.cpp):
//    全局状态 WifesGlobalState(随时插播"想上厕所")、
//    当前状态 DoHouseWork(做家务:拖地/洗碗/铺床随机轮换)、
//    临时状态 VisitBathroom(上厕所,上完退回家务)。
//
//【逐个文件的关系】
//  State.h               —— 状态接口(模板),MinersWifeOwnedStates.h 要用它
//                           (本文件先包含,供下方 StateMachine 指针类型使用);
//  BaseGameEntity.h      —— 妻子继承它(与矿工同门:自动获得编号、必须实现
//                           Update());她的编号是 ent_Elsa(=1,见 EntityNames.h);
//  Locations.h           —— 成员 m_Location 的类型 location_type 来自它;
//  MinersWifeOwnedStates.h —— 妻子的 3 个状态类声明。构造函数里要用
//                           DoHouseWork::Instance() 和 WifesGlobalState::Instance(),
//                           必须先认识它们;
//  misc/ConsoleUtils.h   —— 控制台工具(MinersWife.cpp 的 Update 里用
//                           SetTextColor 给台词染绿色);
//  Miner.h               —— 矿工类的完整定义。本文件没直接用到矿工,
//                           包含它只是沿袭原作写法(无害);
//  StateMachine.h        —— 成员 m_pStateMachine 的类型 StateMachine<MinersWife>
//                           来自它;
//  misc/Utils.h          —— 公共小工具库:提供随机数函数 RandFloat()/RandInt()
//                           (minersWifeOwnedStates.cpp 里"1/10 概率想上厕所"
//                           和"随机做三件家务"都靠它。该文件包含本文件后即可使用)。
//  (注:misc/ 开头的两个头文件都在工程上上级的公共目录 Common\misc\ 里,
//   靠工程"附加包含目录 ..\..\Common"的设置才能被找到。)
//
//【矿工与妻子的"人生"对比——同一台状态机模板的两种用法】
//   矿工:初始状态=回家睡觉;不装全局状态(m_pGlobalState 一直是 NULL);
//         靠"口渴值"等数据触发切换。
//   妻子:初始状态=做家务;【额外安装了全局状态】——状态机每走一步都先执行
//         全局状态(掷骰子决定要不要插播"上厕所"),再执行当前状态(做家务)。
//         这就是 StateMachine.h 注释里介绍的"状态 blip"玩法在本工程的落地。
//==============================================================================================

// 包含保护(原理详解见 Locations.h):防止本文件被重复包含。
#ifndef MINERSWIFE_H
#define MINERSWIFE_H

// <string>:C++ 标准库字符串头文件(本文件没直接用到,习惯性包含)。
#include <string>

// State.h —— 状态接口(下面经 StateMachine.h 间接需要,先包含一次无妨)。
#include "State.h"
// BaseGameEntity.h —— 要继承基类,必须先见到基类的完整定义。
#include "BaseGameEntity.h"
// Locations.h —— 地点枚举 location_type,成员 m_Location 的类型。
#include "Locations.h"
// MinersWifeOwnedStates.h —— 妻子的 3 个状态类声明:构造函数里要用
// DoHouseWork::Instance() 和 WifesGlobalState::Instance()。
#include "MinersWifeOwnedStates.h"
// misc/ConsoleUtils.h —— 公共目录里的控制台工具(染台词颜色用)。
#include "misc/ConsoleUtils.h"
// Miner.h —— 矿工类定义(本文件没直接用到,沿袭原作写法)。
#include "Miner.h"
// StateMachine.h —— 状态机模板类,成员 m_pStateMachine 的类型来自它。
#include "StateMachine.h"
// misc/Utils.h —— 公共工具库:随机数函数 RandFloat/RandInt
// (真正用到它的是 MinersWifeOwnedStates.cpp,经包含本文件获得)。
#include "misc/Utils.h"



//--------------------------------------------------------------------------------
//【类定义】class MinersWife : public BaseGameEntity —— "矿工妻子"也是一种"游戏实体"
//   继承原理与 Miner 相同(详见 Miner.h 中 class Miner 一行的注释):
//   自动获得编号功能,并必须实现基类的纯虚函数 Update()
//   (实现在 MinersWife.cpp)。
//--------------------------------------------------------------------------------
class MinersWife : public BaseGameEntity
{
// private:(私有区)外界碰不到(详见 BaseGameEntity.h 的注释)。
private:

  //an instance of the state machine class
  //(原文注释:状态机类的一个实例)

  // 【状态机指针——妻子的大脑】StateMachine<MinersWife>* :
  //   指向一台"妻子版状态机"的指针(new 出来、析构时 delete,与 Miner 相同)。
  StateMachine<MinersWife>*  m_pStateMachine;

  // 【当前位置】:妻子只在家(shack)活动——但状态类不检查她的地点,
  // 这个成员更像"留档用"(她上厕所并不真的改变 Location 的值)。
  location_type              m_Location;


// public:(公有区)类对外的"服务窗口"。
public:

  //--------------------------------------------------------------------------------
  //【构造函数】成员初始化列表(原理详见 Miner.h 构造函数的注释):
  //   BaseGameEntity(id) —— 编号交给基类登记(main.cpp 传 ent_Elsa=1);
  //   m_Location(shack)  —— 出生地点:小屋。
  // 函数体里安装两样东西(与矿工的最大区别——多装了一个"全局状态"):
  //--------------------------------------------------------------------------------
  MinersWife(int id):BaseGameEntity(id),
                     m_Location(shack)
                                                                      
  {
    // 【new 一台状态机】把妻子自己(this)登记为机器的主人
    // (new/delete 成对使用的原理详见 Miner.h 构造/析构函数的注释)。
    m_pStateMachine = new StateMachine<MinersWife>(this);

    // 【安装当前状态】艾尔莎一睁眼就在"做家务"(拖地/洗碗/铺床随机轮换)。
    m_pStateMachine->SetCurrentState(DoHouseWork::Instance());

    // 【安装全局状态】——本工程唯一用到 SetGlobalState 的地方!
    // 全局状态 WifesGlobalState 每步都"插播"检查:1/10 的概率突然想上厕所
    // → ChangeState 切去 VisitBathroom → 上完 RevertToPreviousState 退回来
    // (机制详见 StateMachine.h 头部的"状态 blip"注释)。
    m_pStateMachine->SetGlobalState(WifesGlobalState::Instance());
  }

  // 【析构函数】把构造函数里 new 出来的状态机内存 delete 归还
  // (new/delete 成对的黄金法则,详见 Miner.h 的注释)。
  ~MinersWife(){delete m_pStateMachine;}


  // 【每步更新/心跳】覆写基类的纯虚函数。实现在 MinersWife.cpp:
  // 给台词染绿色,然后把决策权交给状态机(她没有口渴值之类的"固定消耗",
  // 一切行为都由状态类自己决定)。
  void Update();

  // 【取出状态机】状态类(如 VisitBathroom)通过 wife->GetFSM()->
  // RevertToPreviousState() 退回上一状态(详见 MinersWifeOwnedStates.cpp)。
  StateMachine<MinersWife>*  GetFSM()const{return m_pStateMachine;}

  //----------------------------------------------------accessors
  //(原文分隔注释:下面开始"访问器"——读写小工具)

  // 读:当前地点;写:把妻子搬到地点 loc(与 Miner 的同名工具一样)。
  location_type Location()const{return m_Location;}
  void          ChangeLocation(const location_type loc){m_Location=loc;}
   
};




// 配对文件上方 #ifndef MINERSWIFE_H 的收尾(作用见 Locations.h 的"包含保护"注释)。
#endif
