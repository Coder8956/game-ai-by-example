//==============================================================================================
//【文件说明】Miner.h —— "矿工"类的声明:游戏主角矿工 Bob 的数据与行为清单
//
//【这个文件是干什么的?】
//  定义 class Miner(矿工类),即主角"矿工 Bob"的图纸:
//    ▸ 数据(成员变量):当前地点、口袋里的金块数、银行存款、口渴值、疲劳值,
//      以及最重要的——一台"状态机"指针 m_pStateMachine(矿工的大脑);
//    ▸ 行为(成员函数):Update()(矿工的心跳/每步更新),GetFSM()(取出状态机),
//      外加一组读写数据的小工具(GoldCarried()、Thirsty() 等)。
//
//【文件关系图:本文件在"状态机"中的位置】
//        Miner(矿工)──继承──▶ BaseGameEntity(基类:管编号、强制实现 Update)
//            │
//            │ 持有一个 m_pStateMachine(StateMachine<Miner>* 状态机指针)
//            ▼
//        StateMachine<Miner>(状态机:管当前/上一/全局三种状态)──使用──▶
//        State<Miner>(状态接口)◀──分别实现── 4 个具体状态
//        (具体状态类全部在 MinerOwnedStates.h/.cpp:挖矿/存钱/睡觉/喝酒)
//
//【逐个文件的关系】
//  BaseGameEntity.h  —— Miner 继承它:自动获得"唯一编号"功能,并必须实现
//                      基类的纯虚函数 Update()(实现写在 Miner.cpp);
//  Locations.h       —— 成员 m_Location 的类型 location_type(4 个地点枚举)来自它;
//  MinerOwnedStates.h —— 矿工的 4 个状态类声明。为什么要包含它?因为下面的
//                      构造函数要把初始状态设为"回家睡觉",必须先认识
//                      GoHomeAndSleepTilRested 这个类才能调用它的 Instance();
//  StateMachine.h    —— 成员 m_pStateMachine 的类型 StateMachine<Miner> 来自它;
//  Miner.cpp         —— Update() 等函数的具体实现都在那里;
//  MinerOwnedStates.cpp —— 4 个状态类通过本文件的公有函数操纵矿工
//                      (如 pMiner->AddToGoldCarried(1)),并通过
//                      pMiner->GetFSM()->ChangeState(...) 切换状态;
//  EntityNames.h     —— main.cpp 创建矿工时传入的编号 ent_Miner_Bob(=0)
//                      经基类登记进 m_ID;
//  main.cpp          —— 创建 Miner 对象并循环调用 Update(),驱动状态机运转。
//
//【游戏规则速查(常量区 + 成员含义)】
//  挖矿:金块 +1、疲劳 +1;口袋满(3 块)去银行;
//  银行:金块全换存款,存款 ≥ 5 回家睡觉,没存够继续去挖;
//  睡觉:每步疲劳 -1,疲劳降到 ≤ 5 就睡醒回金矿;
//  口渴:矿工每活一步口渴 +1,口渴 ≥ 5 去酒吧,喝酒清零口渴、花 2 元,喝完回金矿。
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
//   描述  :A class defining a goldminer —— 定义"矿工"的一个类
//   作者  :Mat Buckland,2002 年(本书作者)
//------------------------------------------------------------------------

// 标准库头文件用尖括号:<string> 是字符串库(本文件没直接用到,习惯性包含);
// <cassert> 是"断言"库(Miner.cpp 里用到的 assert 靠的是这里的包含)。
#include <string>
#include <cassert>

// 双引号 = 本工程自己的头文件:
// BaseGameEntity.h —— 要继承基类,必须先让编译器见到基类的完整定义;
#include "BaseGameEntity.h"
// Locations.h —— 地点枚举 location_type(shack/goldmine/bank/saloon),
// 下面成员 m_Location 的类型来自它。
#include "Locations.h"
// MinerOwnedStates.h —— 4 个状态类的声明:构造函数里要用
// GoHomeAndSleepTilRested::Instance()(初始状态:回家睡觉)。
#include "MinerOwnedStates.h"
// StateMachine.h —— 状态机模板类的定义:成员 m_pStateMachine 的类型
// StateMachine<Miner> 来自它。
#include "StateMachine.h"


//--------------------------------------------------------------------------------
//【全局常量:矿工的"性格参数"】
//  const :常量——变量一旦初始化就不能再修改,防止被代码误改;
//  int   :整数类型;
//  命名  :常量名首字母大写、单词首字母大写连写(如 TirednessThreshold = 疲劳门槛)。
//  这 4 个常量是 Miner 类和 4 个状态类共同遵守的游戏规则,改这里就能改矿工性格:
//--------------------------------------------------------------------------------

//the amount of gold a miner must have before he feels comfortable
//(原文注释:矿工的"银行存款"达到这个数,他才觉得舒服)
// 【舒适度门槛】:存款 ≥ 5 块就"心满意足",矿工会回家休息
// (使用处:MinerOwnedStates.cpp 里 VisitBankAndDepositGold::Execute)。
const int ComfortLevel       = 5;
//the amount of nuggets a miner can carry
//(原文注释:矿工一次最多能背几块天然金块)
// 【口袋容量】:金块 ≥ 3 就算装满(PocketsFull() 判断),必须先去银行存掉。
const int MaxNuggets         = 3;
//above this value a miner is thirsty
//(原文注释:口渴值超过这个数,矿工就渴了)
// 【口渴门槛】:口渴值 ≥ 5 就得去酒吧(Thirsty() 判断)。
const int ThirstLevel        = 5;
//above this value a miner is sleepy
//(原文注释:疲劳值超过这个数,矿工就困了)
// 【疲劳门槛】:疲劳值 > 5 就算困,要回家睡觉(Fatigued() 判断,注意是"严格大于")。
const int TirednessThreshold = 5;



//--------------------------------------------------------------------------------
//【类定义】class Miner : public BaseGameEntity —— "矿工"是一种"游戏实体"
//   class Miner : 定义一个名叫 Miner 的类;
//   : public BaseGameEntity : "公有继承"。Miner 自动拥有基类的全部成员
//     (编号 m_ID、访问器 ID() 等),基类的公有成员在 Miner 里仍然是公有的。
//     这是"is-a(是一种)"关系:矿工是一种游戏实体;
//   { ... };  :花括号内是类体,结尾分号不能丢。
//   另外因基类的 Update() 是纯虚函数(=0),Miner 必须实现它
//   (实现在 Miner.cpp),否则 Miner 也是抽象类、创建不了对象。
//--------------------------------------------------------------------------------
class Miner : public BaseGameEntity
{
// private:(私有区)下面这些数据只有本类自己的函数能访问(封装,详见
// BaseGameEntity.h 中 private 的注释);状态类想改这些数据,只能走
// 下面 public 区的"服务窗口"函数。
private:

  //an instance of the state machine class
  //(原文注释:状态机类的一个实例)

  // 【状态机指针——矿工的"大脑",整个状态机的核心】
  //   StateMachine<Miner>* :指向一台"矿工版状态机"的指针(模板造出的具体
  //   类型,原理见 StateMachine.h)。矿工自己从不决定"现在干什么",一切
  //   决策都交给这台机器——机器再转交给当前状态对象。
  //   与 WestWorld1 的区别:那时矿工只拿一个"当前状态指针";现在换成整台
  //   状态机,多了"上一状态/全局状态"档案和"切换/回退"的完整服务。
  StateMachine<Miner>*  m_pStateMachine;
  
  // 【当前位置】:类型 location_type 来自 Locations.h(小屋/金矿/银行/酒吧之一)。
  location_type         m_Location;

  //how many nuggets the miner has in his pockets
  //(原文注释:矿工口袋里现有几块金子)
  // 【口袋金块数】:挖矿每步 +1;去银行后清零。i_=int(整数)前缀。
  int                   m_iGoldCarried;

  // 【银行存款】:存金块时增加;买威士忌一杯 -2。
  int                   m_iMoneyInBank;

  //the higher the value, the thirstier the miner
  //(原文注释:这个值越高,矿工越渴)
  // 【口渴值】:矿工每活一步(每次 Update)+1;≥ ThirstLevel(5)就算渴了。
  int                   m_iThirst;

  //the higher the value, the more tired the miner
  //(原文注释:这个值越高,矿工越累)
  // 【疲劳值】:挖矿每步 +1;睡觉每步 -1;> TirednessThreshold(5)就算困了。
  int                   m_iFatigue;

// public:(公有区)类对外的"服务窗口":状态类与 main 都通过这里操纵矿工。
public:

  //--------------------------------------------------------------------------------
  //【构造函数】矿工出生时自动执行。冒号":"到花括号之间是"成员初始化列表",
  // 格式为"成员名(初始值)",逗号分隔,逐项发"出厂设置":
  //   BaseGameEntity(id) —— 先调用"基类的构造函数":把编号 id 交给它登记
  //                         (内部再调 SetID,见 BaseGameEntity.cpp);
  //   m_Location(shack)  —— 出生地点:小屋;
  //   m_iGoldCarried(0)  —— 口袋金块:0;
  //   m_iMoneyInBank(0)  —— 银行存款:0;
  //   m_iThirst(0)       —— 口渴值:0;
  //   m_iFatigue(0)      —— 疲劳值:0;
  // 初始化列表比在函数体里逐个赋值更高效,而且初始化基类只能用这一种写法。
  // 成员按"在类中声明的先后顺序"初始化。
  // 【小知识】本行 m_iFatigue(0) 之后的那一长串行尾空格是原作者排版残留,
  // 本工程遵循"源码零改动"原则,原样保留。
  //--------------------------------------------------------------------------------
  Miner(int id):BaseGameEntity(id),
                m_Location(shack),
                m_iGoldCarried(0),
                m_iMoneyInBank(0),
                m_iThirst(0),
                m_iFatigue(0)                                         
  {
    // 【new:在"堆"内存里造一台状态机】
    //   new StateMachine<Miner>(this) 的意思:在堆(程序自由使用的动态内存区)
    //   里创建一台"矿工版状态机",并返回它的地址。构造参数传 this——
    //   this 是 C++ 关键字,代表"正在执行本函数的那个矿工对象自己"的地址:
    //   状态机从此记住"我是这台矿工的大脑"(m_pOwner = this,见 StateMachine.h)。
    //   new 出来的东西不随矿工对象自动销毁,必须有人负责 delete(见下面的析构函数)。
    m_pStateMachine = new StateMachine<Miner>(this);
    
    // 【安装初始状态】:矿工一睁眼正处于"回家睡觉"状态。
    // GoHomeAndSleepTilRested 是 4 个状态类之一(声明在 MinerOwnedStates.h),
    // Instance() 取它的唯一实例(单例);SetCurrentState 把它装进状态机。
    // (出生时疲劳是 0,所以第一轮 Update 他立刻"睡醒"跑去挖矿。)
    m_pStateMachine->SetCurrentState(GoHomeAndSleepTilRested::Instance());
  }

  // 【析构函数】"~类名"= 对象销毁时自动执行的收尾函数。
  // delete m_pStateMachine:把构造函数里 new 出来的那台状态机内存归还系统。
  // 【黄金法则】new 和 delete 必须成对出现——只 new 不 delete = 内存泄漏;
  // 只 delete 不 new = 程序崩溃。矿工死了,他的"大脑"也要一并安葬。
  ~Miner(){delete m_pStateMachine;}

  //this must be implemented
  //(原文注释:这个函数必须实现)

  // 【每步更新/心跳】覆写基类的纯虚函数 BaseGameEntity::Update()。
  // 实现在 Miner.cpp:设置台词颜色、口渴值 +1,然后把行动决策权整个交给
  // 状态机的 Update()。main.cpp 的循环每转一圈就调用它一次,矿工就"活一步"。
  void Update();

  // 【取出状态机】状态类想切换状态时(如 pMiner->GetFSM()->ChangeState(...)),
  // 就是通过这个"服务窗口"拿到矿工脑子里的那台状态机。
  // ()const 的只读承诺原理详见 BaseGameEntity.h 中 ID() 的注释。
  StateMachine<Miner>*  GetFSM()const{return m_pStateMachine;}


  //【地点读/写小工具(getter/setter)】
  // getter:只读访问器,返回当前地点。
  location_type Location()const{return m_Location;}
  // setter:把矿工搬到地点 loc。参数前的 const 表示函数内不许修改这个参数。
  void          ChangeLocation(const location_type loc){m_Location=loc;}
    
  //【口袋与金块的小工具】
  // 读:口袋里现在有几块金子。
  int           GoldCarried()const{return m_iGoldCarried;}
  // 写:直接设置口袋金块数(本例在存钱后清零时用到)。
  void          SetGoldCarried(const int val){m_iGoldCarried = val;}
  // 增/减金块:val 可为负;实现不在类内而在 Miner.cpp(带"不能小于 0"的保护)。
  void          AddToGoldCarried(const int val);
  // 判断:口袋满了吗?bool 类型只有真(true)/假(false)两个值;
  // ">=" 读作"大于等于"。函数体写在类内 = 隐式内联函数。
  bool          PocketsFull()const{return m_iGoldCarried >= MaxNuggets;}

  //【疲劳相关的小工具】
  // 判断:困了吗(疲劳值 > TirednessThreshold=5)?实现在 Miner.cpp。
  bool          Fatigued()const;
  // 睡一觉:疲劳 -1。"-=" 读作"自身减去"(等价于 m_iFatigue = m_iFatigue - 1)。
  void          DecreaseFatigue(){m_iFatigue -= 1;}
  // 干活:疲劳 +1。"+=" 读作"自身加上"。
  void          IncreaseFatigue(){m_iFatigue += 1;}

  //【存款相关的小工具】
  // 读:银行存款。
  int           Wealth()const{return m_iMoneyInBank;}
  // 写:直接设置存款。
  void          SetWealth(const int val){m_iMoneyInBank = val;}
  // 增/减存款(存钱 +金块数;买酒 -2);实现同样在 Miner.cpp(有"不为负"保护)。
  void          AddToWealth(const int val);

  // 判断:渴了吗(口渴值 ≥ ThirstLevel=5)?实现在 Miner.cpp。
  bool          Thirsty()const; 
  // 【买一杯威士忌并喝下】:口渴值立刻归零,存款 -2。
  // 花括号里一行写了两个语句,用分号隔开(C++ 允许,一般为了清晰会分行写)。
  void          BuyAndDrinkAWhiskey(){m_iThirst = 0; m_iMoneyInBank-=2;}

};





// 配对文件上方 #ifndef MINER_H 的收尾(作用见 Locations.h 的"包含保护"注释)。
#endif
