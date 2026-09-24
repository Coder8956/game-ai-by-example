//==============================================================================================
//【文件说明】Miner.h —— "矿工"类的声明:游戏主角矿工 Bob 的数据与行为清单
//
//【这个文件是干什么的?】
//  定义 class Miner(矿工类),即本游戏唯一主角"矿工 Bob"的图纸:
//    ▸ 数据(成员变量):当前地点、口袋里的金块数、银行存款、口渴值、疲劳值,
//      以及最重要的——"当前状态"指针 m_pCurrentState(矿工的大脑此刻由哪个状态接管);
//    ▸ 行为(成员函数):Update()(矿工的心跳/每步更新)、ChangeState()(切换状态),
//      外加一组读写数据的小工具(GoldCarried()、Thirsty() 等)。
//
//【文件关系图:本文件是整个"状态机"的中枢】
//        Miner(矿工)──继承──▶ BaseGameEntity(基类:管编号、强制实现 Update)
//            │
//            │ 恰好持有一个 m_pCurrentState(State* 类型,当前状态指针)
//            ▼
//        State(状态接口:Enter/Execute/Exit)◀──分别实现── 4 个具体状态
//        (具体状态类全部在 MinerOwnedStates.h/.cpp:挖矿/存钱/睡觉/喝酒)
//
//【逐个文件的关系】
//  BaseGameEntity.h —— Miner 继承它(class Miner : public BaseGameEntity):
//                     自动获得"唯一编号"功能,并必须实现基类的纯虚函数 Update();
//  Locations.h      —— 成员 m_Location 的类型 location_type(4 个地点枚举)来自它;
//  State.h          —— 本文件只用"前置声明"(class State;)介绍 State 这个名字,
//                     因为 m_pCurrentState 只是个"指针",不必包含 State 的完整定义;
//  Miner.cpp        —— 本类所有函数的具体实现都在那里(构造函数、Update、
//                     ChangeState、Thirsty 等);
//  MinerOwnedStates.h/.cpp —— 4 个状态类通过本文件的公有函数操纵矿工
//                     (如 pMiner->AddToGoldCarried(1))并调用 ChangeState() 切换;
//  EntityNames.h    —— main.cpp 创建矿工时传入的编号 ent_Miner_Bob(=0)
//                     经基类登记进 m_ID;
//  main.cpp         —— 创建 Miner 对象并循环调用 Update(),驱动整个状态机运转。
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
// <cassert> 是"断言"库——注意 Miner.cpp 里用到的 assert 靠的是这里的包含
// (Miner.cpp 包含了本文件,就没再单独包含 <cassert>)。
#include <string>
#include <cassert>

// 双引号 = 本工程自己的头文件:
// BaseGameEntity.h —— 要继承基类,必须先让编译器见到基类的完整定义;
#include "BaseGameEntity.h"
// Locations.h —— 地点枚举 location_type(shack/goldmine/bank/saloon),
// 下面成员 m_Location 的类型来自它。
#include "Locations.h"


// 前置声明(forward declaration):只告诉编译器"State 是一个类",不给具体内容。
// 为什么可以?下面成员 m_pCurrentState 是"指针"(State*):指针只存一个地址、
// 大小固定,编译器不需要知道 State 类里有什么。真身在 State.h,
// 由 Miner.cpp(经 MinerOwnedStates.h)拿到完整定义。
class State;

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
private:

  // 【当前状态指针——矿工的"大脑",整个状态机的核心】
  //   State* 类型:可以指向任何一个"State 的子类"对象——运行时它指向
  //   4 个具体状态(挖矿/存钱/睡觉/喝酒)之一。
  //   矿工自己从不决定"现在干什么",一切决策都交给这个指针指向的状态对象
  //   ——这就是"状态机"思想的落点(多态:同一句代码,指向谁就执行谁的行为)。
  //   命名:m_=member(成员)、p_=pointer(指针),匈牙利命名习惯。
  State*                m_pCurrentState;
  
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

public:

  // 【构造函数】:与类同名、无返回类型。这里只写"声明"(末尾有分号),
  // 具体实现在 Miner.cpp。创建矿工时(main.cpp 写的 Miner miner(ent_Miner_Bob);)
  // 自动被调用,给矿工"出生初始值"(位置=小屋、金块/存款/口渴/疲劳=0、
  // 初始状态=回家睡觉状态)。
  Miner(int id);

  //this must be implemented
  //(原文注释:这个函数必须实现)

  // 【每步更新/心跳】覆写基类的纯虚函数 BaseGameEntity::Update()。
  // 实现在 Miner.cpp:口渴值 +1,然后把行动决策交给当前状态的 Execute()。
  // main.cpp 的循环每转一圈就调用它一次,矿工就"活一步"。
  void Update();

  //this method changes the current state to the new state. It first
  //calls the Exit() method of the current state, then assigns the
  //new state to m_pCurrentState and finally calls the Entry()
  //method of the new state.
  //(原文注释大意:本方法把当前状态换成新状态。它先调用当前状态的
  //  Exit(),再把新状态赋给 m_pCurrentState,最后调用新状态的
  //  Enter()。注:原文把进入函数写作 Entry,实际代码里叫 Enter。)

  // 【切换状态入口】参数 State* new_state:新状态的指针(通常传
  // "某状态类::Instance()"的返回值)。三个步骤的实现在 Miner.cpp:
  // ① 旧状态->Exit(this)  ② m_pCurrentState=新状态  ③ 新状态->Enter(this)。
  void ChangeState(State* new_state);

  

  //【地点读/写小工具(getter/setter)】
  // getter:只读访问器,返回当前地点。括号后的 const 表示
  // "本函数保证不修改对象的任何成员"(只读承诺),const 对象也能调用。
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




#endif
