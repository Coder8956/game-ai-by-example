//==============================================================================================
//【文件说明】Miner.cpp —— "矿工"类的实现(兑现 Miner.h 里的全部承诺)
//
//【这个文件是干什么的?】
//  1. 构造函数 Miner::Miner()  —— 矿工出生时的初始值(出生在小屋、身无分文、
//     初始状态为"回家睡觉");
//  2. Miner::ChangeState()     —— 切换状态的标准三步曲(旧状态退出→换指针→新状态进入);
//  3. Miner::Update()          —— 矿工的"心跳":每被调用一次就活一步;
//  4. 一组小工具的实现:加金块(AddToGoldCarried)、加存款(AddToWealth)、
//     判断口渴(Thirsty)、判断困倦(Fatigued)。
//
//【与相关文件的关系】
//  Miner.h            —— 本文件实现它声明的类,第一件事就是包含它;
//  MinerOwnedStates.h —— 构造函数要把初始状态设为"回家睡觉"状态,必须包含
//                        4 个状态类的声明,才能调用 GoHomeAndSleepTilRested::Instance();
//  main.cpp           —— main() 创建矿工后,循环调用这里的 Update(),驱动状态机;
//  State.h / MinerOwnedStates.cpp —— Update() 和 ChangeState() 最终会调用
//                        具体状态对象的 Execute/Enter/Exit(代码在 MinerOwnedStates.cpp);
//  Locations.h        —— (经 Miner.h 间接包含)出生地点 shack(小屋)来自它。
//
//【C++ 语法:"类名::函数名"中的双冒号 "::"】
//  叫"作用域解析运算符"。写在类外面的函数实现,必须用 Miner::xxx 说明
//  "实现的是 Miner 类里的那个 xxx",否则编译器以为是个不相干的散函数。
//==============================================================================================

// 包含自己的头文件:实现前必须先见到"矿工类长什么样"(有哪些成员和声明)。
#include "Miner.h"
// 包含矿工 4 个状态类的声明:构造函数里要用到
// GoHomeAndSleepTilRested::Instance()(回家睡觉状态)。
#include "MinerOwnedStates.h"

//--------------------------------------------------------------------------------
//【构造函数】矿工出生时自动执行。冒号":"到花括号之间的内容叫"成员初始化列表",
// 格式为"成员名(初始值)",逗号分隔,逐项给成员变量发"出厂设置":
//   BaseGameEntity(id)      —— 先调用"基类的构造函数":把编号 id 传给
//                              BaseGameEntity::BaseGameEntity(id),它内部再调用
//                              SetID(id) 登记编号(见 BaseGameEntity.cpp);
//   m_Location(shack)       —— 出生地点:小屋;
//   m_iGoldCarried(0)       —— 口袋金块:0;
//   m_iMoneyInBank(0)       —— 银行存款:0;
//   m_iThirst(0)            —— 口渴值:0;
//   m_iFatigue(0)           —— 疲劳值:0;
//   m_pCurrentState(GoHomeAndSleepTilRested::Instance())
//                           —— 【初始状态】:矿工一睁眼正处于"回家睡觉"状态。
//                              GoHomeAndSleepTilRested 是 4 个状态类之一(声明在
//                              MinerOwnedStates.h),Instance() 取它的唯一实例(单例)。
// 最后的 {} 是空函数体:所有初始化工作都由初始化列表完成了。
// 【小知识】成员初始化列表比在函数体里逐个赋值更高效,而且初始化基类
// 只能用这一种写法。成员按"在类中声明的先后顺序"初始化。
//--------------------------------------------------------------------------------
Miner::Miner(int id):BaseGameEntity(id),
                     m_Location(shack),
                     m_iGoldCarried(0),
                     m_iMoneyInBank(0),
                     m_iThirst(0),
                     m_iFatigue(0),
                     m_pCurrentState(GoHomeAndSleepTilRested::Instance())
                                                                  
{}

//--------------------------- ChangeState -------------------------------------
//-----------------------------------------------------------------------------
//【切换状态:状态机的"换脑"标准流程】
//  参数 State* pNewState:要切换到的新状态(通常传"某状态类::Instance()")。
//  固定三步(顺序不能乱):
//    ① 旧状态->Exit(this)        —— 让旧状态收尾告别;
//    ② m_pCurrentState = 新状态   —— 把"大脑"指针换掉;
//    ③ 新状态->Enter(this)       —— 让新状态登场准备(通常包括赶路)。
//-----------------------------------------------------------------------------
void Miner::ChangeState(State* pNewState)
{
  //make sure both states are both valid before attempting to 
  //call their methods
  //(原文注释大意:调用状态的方法之前,先确保两个状态指针都有效)

  // assert:断言(来自 <cassert>,经 Miner.h 包含)。条件为"假"时程序
  // 立即中断并报出错文件和行号——开发阶段抓逻辑错误的工具。
  // 条件:m_pCurrentState(旧状态)和 pNewState(新状态)都非空。
  // "&&"读作"并且";指针直接当条件用:空指针(NULL,地址 0)算"假",
  // 非空算"真"。空指针去调用函数会让程序崩溃,不如提前拦下。
  assert (m_pCurrentState && pNewState);

  //call the exit method of the existing state
  //(原文注释:调用现有状态的退出方法)

  // 第①步:this 是 C++ 关键字,代表"正在执行本函数的那个矿工对象自己"的地址。
  // "->"是指针的成员访问运算符(读作"指向"),this 是指针,所以用 ->
  // 而不是点号。把矿工自己交给旧状态的 Exit() 做收尾。
  m_pCurrentState->Exit(this);

  //change state to the new state
  //(原文注释:把状态换成新状态)

  // 第②步:指针改指新状态——从这一行起,矿工的"大脑"就是新状态了。
  m_pCurrentState = pNewState;

  //call the entry method of the new state
  //(原文注释:调用新状态的进入方法)

  // 第③步:调用新状态的 Enter() 做登场准备。
  m_pCurrentState->Enter(this);
}


//-----------------------------------------------------------------------------
//【加金块】给口袋增加 val 块(挖矿时传 1;负数表示减少)。
// 注意:这个函数在 Miner.h 里只有"声明"(行尾带分号),具体实现就在这里;
// 实际调用它的是 MinerOwnedStates.cpp 里的挖矿状态。
//-----------------------------------------------------------------------------
void Miner::AddToGoldCarried(const int val)
{
  // "+=" 读作"自身加上":等价于 m_iGoldCarried = m_iGoldCarried + val。
  m_iGoldCarried += val;

  // 保护:金块数不允许为负。if(条件) 后只有一条语句时,花括号可以省略——
  // 条件成立时只执行紧跟的那一句。
  if (m_iGoldCarried < 0) m_iGoldCarried = 0;
}


//-----------------------------------------------------------------------------
//【加存款】给银行存款增加 val(存金块时传金块数;买酒时传 -2)。
//-----------------------------------------------------------------------------
void Miner::AddToWealth(const int val)
{
  m_iMoneyInBank += val;

  // 同样的"不为负"保护。
  if (m_iMoneyInBank < 0) m_iMoneyInBank = 0;
}


//-----------------------------------------------------------------------------
//【判断口渴】返回 bool(真/假):口渴值 ≥ ThirstLevel(5,常量在 Miner.h)
// 返回 true(渴了),否则返回 false。括号后的 const 表示本函数不修改任何成员。
//-----------------------------------------------------------------------------
bool Miner::Thirsty()const
{
  if (m_iThirst >= ThirstLevel){return true;}

  return false;
}


//-----------------------------------------------------------------------------
//【矿工的"心跳":每步更新】——整个状态机的发动机。
// main.cpp 的 for 循环每圈调用一次 miner.Update()。
// 注意:矿工自己只做两件事:口渴值 +1、把行动决策权交给"当前状态"的
// Execute()。矿工从不用 if-else 判断"我该干什么"——一切交给状态,
// 这正是"行为跟着状态走"的状态机精髓。
//-----------------------------------------------------------------------------
void Miner::Update()
{
  // 每活一步,口渴值 +1(时间流逝,人越来越渴)。
  m_iThirst += 1;
  
  // if(指针):判断指针非空。指针直接作条件:空(NULL)算"假"、非空算"真"。
  // 防御式编程:万一将来有人忘了给矿工设初始状态,也不至于空指针崩溃。
  if (m_pCurrentState)
  {
    // 让"当前状态"执行一步。this 传的是矿工自己;
    // 实际调用到哪个 Execute,取决于 m_pCurrentState 此刻指向
    // 哪个具体状态类——这就是"多态"(同一句话,不同对象不同行为)。
    m_pCurrentState->Execute(this);
  }
}


//-----------------------------------------------------------------------------
//【判断困倦】疲劳值 > TirednessThreshold(5,常量在 Miner.h;注意是
// "严格大于"而非"大于等于")返回 true。与 Thirsty() 的写法风格不同
// (花括号分行),但逻辑同理,返回类型 bool。
//-----------------------------------------------------------------------------
bool Miner::Fatigued()const
{
  if (m_iFatigue > TirednessThreshold)
  {
    return true;
  }

  return false;
}