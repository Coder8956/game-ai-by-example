//==============================================================================================
//【文件说明】MinerOwnedStates.cpp —— 矿工 4 个状态的"剧本正文"(本示例的核心)
//
//【这个文件是干什么的?】
//  矿工人生每一幕"谁说了什么台词、干了什么事、什么时候切到下一幕",
//  全部写在这个文件里。每个状态分三段(都来自 State.h 接口的规定):
//    Enter()   —— 进入状态时执行一次:通常负责"走到对应地点"+ 念开场白;
//    Execute() —— 处于该状态期间,矿工每 Update() 一次就执行一次:
//                 干活 + 判断要不要切换到别的状态;
//    Exit()    —— 离开状态时执行一次:念告别词。
//
//【与相关文件的关系】
//  MinerOwnedStates.h —— 本文件实现其中声明的 4 个状态类(必须先包含它);
//  State.h            —— 状态接口(经 MinerOwnedStates.h 间接包含,再写一次是
//                        明确表达依赖,无害);
//  Miner.h / Miner.cpp —— 参数 pMiner 就是"正经历这个状态的矿工"。各状态通过
//                        矿工的公有函数读改数据,并通过 pMiner->ChangeState(...)
//                        触发切换(切换流程的实现见 Miner.cpp);
//  Locations.h        —— goldmine/bank/shack/saloon 四个地点枚举;
//  EntityNames.h      —— GetNameOfEntity(编号) 把编号翻译成名字 "Miner Bob";
//  misc/ConsoleUtils.h —— SetTextColor(...) 设置控制台文字颜色(位于工程
//                        上上级的公共目录 Common\misc\,靠工程"附加包含目录
//                        ..\..\Common"的设置才能被找到)。
//
//【一次典型循环】main 调 miner.Update() → 口渴 +1 → 当前状态 Execute(this)
//  → Execute 里干活并可能调用 ChangeState → ChangeState 里:
//  旧状态 Exit → 换指针 → 新状态 Enter —— 如此往复,状态机不停转。
//==============================================================================================

// 本文件要实现的 4 个状态类的"说明书"(类声明)。
#include "MinerOwnedStates.h"
// State 基类的定义(4 个类的共同父类)。
#include "State.h"
// 矿工类的完整定义:本文件要调用矿工的公有函数(ChangeLocation、
// AddToGoldCarried、ChangeState 等),光有前置声明不够,必须包含完整头文件。
#include "Miner.h"
// 地点枚举(goldmine/bank/shack/saloon)。
#include "Locations.h"
// 控制台工具函数 SetTextColor(位于 ..\..\Common\misc\ConsoleUtils.h)。
#include "misc/ConsoleUtils.h"
// 编号→名字函数 GetNameOfEntity(打印台词时显示 "Miner Bob")。
#include "EntityNames.h"

// <iostream>:C++ 标准输入输出库。cout(控制台输出流)定义在这里。
#include <iostream>
// using 声明:把"std 命名空间里的 cout"引进本文件,
// 以后直接写 cout,不用处处写 std::cout。
using std::cout;

//define this to output to a file
//(原文注释:定义这个宏,输出就改写进文件里)

//--------------------------------------------------------------------------------
//【条件编译】#ifdef XXX ... #endif:仅当编译前定义过 XXX 宏时,中间的代码才
// 会被真正编译,否则整段当作不存在。默认 TEXTOUTPUT 并未定义 → 下面 4 行
// 全部跳过,程序照常打印到屏幕。想要"输出到文件"时,在工程设置的
// "预处理器定义"里加上 TEXTOUTPUT 再编译即可。
//--------------------------------------------------------------------------------
#ifdef TEXTOUTPUT
// <fstream>:文件流库。ofstream = output file stream,"往文件里写"的流。
#include <fstream>
// extern:声明"os 这个全局变量在别的 .cpp 文件里定义,这里只借用不重复定义"。
extern std::ofstream os;
// #define cout os:文本宏替换——此后本文件里所有 cout 都会在预处理阶段
// 被替换成 os,于是输出全部改道写进文件。
#define cout os
#endif





//--------------------------------------methods for EnterMineAndDigForNugget

//(原文分隔注释:下面开始【状态①进金矿挖金子】的各个方法实现)

//--------------------------------------------------------------------------------
//【单例入口】获取"挖矿状态"的唯一实例(矿工和别的状态都这样用它):
//   EnterMineAndDigForNugget*           :返回类型——指向本类的指针;
//   EnterMineAndDigForNugget::Instance():作用域"类名::"表明这是
//     MinerOwnedStates.h 里声明的那个静态函数的"实现";
//   static 局部变量 instance:函数体内的 static 变量,只在程序"第一次"
//     执行到这一行时构造一次,之后每次调用都返回同一个对象——
//     "全游戏只有一份挖矿状态"由此保证;
//   return &instance:"&"是取地址运算符,返回那个唯一对象的内存地址。
//--------------------------------------------------------------------------------
EnterMineAndDigForNugget* EnterMineAndDigForNugget::Instance()
{
  static EnterMineAndDigForNugget instance;

  return &instance;
}

//--------------------------------------------------------------------------------
//【Enter:进入"挖矿"状态时执行一次】
//   void:函数不返回任何值;参数 pMiner(p=pointer 指针):刚切换进本状态的矿工。
//   职责:若矿工还不在金矿,让他走过去并念一句台词。
//--------------------------------------------------------------------------------
void EnterMineAndDigForNugget::Enter(Miner* pMiner)
{
  //if the miner is not already located at the goldmine, he must
  //change location to the gold mine
  //(原文注释大意:如果矿工还不在金矿,就必须把位置改成金矿)

  // if(条件){...}:条件为"真"才执行花括号内的代码。
  // "!="读作"不等于";pMiner->Location() 用箭头运算符(指针版点号)
  // 调用矿工的只读访问器,返回他当前所在的地点。
  // 条件含义:矿工此刻不在金矿 → 需要先赶路。
  if (pMiner->Location() != goldmine)
  {
    // SetTextColor:设置控制台文字颜色(定义在 ConsoleUtils.h)。
    // FOREGROUND_RED=红色,FOREGROUND_INTENSITY=高亮加粗;
    // "|"是"按位或"运算符:把两个开关标志合并成"又红又亮"。
    SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
    // cout << x:流插入运算符"<<"把右侧内容依次送进输出流打印。
    // "\n"是换行符;GetNameOfEntity(pMiner->ID()) 把矿工编号翻译成
    // 名字"Miner Bob";台词:Walkin' to the goldmine(走去金矿)。
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Walkin' to the goldmine";

    // 把矿工的地点成员改成 goldmine(赶路一步到位)。
    pMiner->ChangeLocation(goldmine);
  }
}


//--------------------------------------------------------------------------------
//【Execute:处于"挖矿"状态期间的每步行动】矿工每次 Update() 都会走到这里:
//   挖 1 块金子、更累 1 分、报一句台词,然后做两个"要不要切换状态"的判断。
//--------------------------------------------------------------------------------
void EnterMineAndDigForNugget::Execute(Miner* pMiner)
{  
  //the miner digs for gold until he is carrying in excess of MaxNuggets. 
  //If he gets thirsty during his digging he packs up work for a while and 
  //changes state to go to the saloon for a whiskey.
  //(原文注释大意:矿工一直挖到口袋装不下为止;挖矿期间若口渴了,
  //  就暂时收工,切换到去酒吧喝威士忌的状态)

  // 挖到 1 块天然金块:口袋金块 +1(AddToGoldCarried 实现在 Miner.cpp)。
  pMiner->AddToGoldCarried(1);

  // 干活很累人:疲劳值 +1(IncreaseFatigue 实现在 Miner.h 类内)。
  pMiner->IncreaseFatigue();

  // 打印台词:Pickin' up a nugget(捡起一块金子)。
  SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Pickin' up a nugget";

  //if enough gold mined, go and put it in the bank
  //(原文注释:挖够了金子就去存进银行)

  // 判断①:口袋满了吗?(金块数 ≥ MaxNuggets=3,见 Miner.h 的 PocketsFull)
  if (pMiner->PocketsFull())
  {
    // 满 → 切换到"去银行存钱"状态。VisitBankAndDepositGold::Instance()
    // 取银行状态的唯一实例;ChangeState 的三步流程见 Miner.cpp。
    pMiner->ChangeState(VisitBankAndDepositGold::Instance());
  }

  // 判断②:口渴了吗?(口渴值 ≥ ThirstLevel=5,见 Miner.cpp 的 Thirsty)
  if (pMiner->Thirsty())
  {
    // 渴 → 切换到"去酒吧解渴"状态。
    pMiner->ChangeState(QuenchThirst::Instance());
  }
}


//--------------------------------------------------------------------------------
//【Exit:离开"挖矿"状态时执行一次】由 ChangeState() 在切换前自动调用。
// 注意:这里输出的字符串被拆成了两行"<<"链式书写,效果和一行完全相同。
//--------------------------------------------------------------------------------
void EnterMineAndDigForNugget::Exit(Miner* pMiner)
{
  SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " 
       << "Ah'm leavin' the goldmine with mah pockets full o' sweet gold";
}



//----------------------------------------methods for VisitBankAndDepositGold
//(原文分隔注释:下面开始【状态②去银行存金子】的方法实现)

//--------------------------------------------------------------------------------
//【单例入口】获取"银行状态"的唯一实例。原理与挖矿状态相同:
// 函数内 static 局部变量只构造一次,返回它的地址(详见上方注释)。
//--------------------------------------------------------------------------------
VisitBankAndDepositGold* VisitBankAndDepositGold::Instance()
{
  static VisitBankAndDepositGold instance;

  return &instance;
}


//--------------------------------------------------------------------------------
//【Enter:进入"银行"状态时执行一次】若矿工还不在银行,走过去并念台词。
//--------------------------------------------------------------------------------
void VisitBankAndDepositGold::Enter(Miner* pMiner)
{  
  //on entry the miner makes sure he is located at the bank
  //(原文注释大意:一进本状态,矿工先确保自己身处银行)

  // 不在银行 → 赶路。写法与挖矿状态的 Enter 相同,只是地点换成 bank。
  if (pMiner->Location() != bank)
  {
    SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Goin' to the bank. Yes siree";

    pMiner->ChangeLocation(bank);
  }
}


//--------------------------------------------------------------------------------
//【Execute:处于"银行"状态期间的每步行动】存金子、报存款余额,
// 然后按存款多少决定:回家睡觉 or 回金矿继续挖。
//--------------------------------------------------------------------------------
void VisitBankAndDepositGold::Execute(Miner* pMiner)
{

  //deposit the gold
  //(原文注释:存金子)

  // AddToWealth(参数):存款加上"参数"这么多钱。参数是
  // pMiner->GoldCarried() —— 口袋里现有的金块数。一句话:
  // "把口袋里的金块全部换算成存款"。存钱动作一步完成。
  pMiner->AddToWealth(pMiner->GoldCarried());
    
  // 口袋清零(金子都进了银行)。
  pMiner->SetGoldCarried(0);

  // 打印台词:Depositing gold. Total savings now: 后面接上存款数字
  // (Wealth() 读出存款,数字会自动转换成文字打印出来)。
  SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " 
       << "Depositing gold. Total savings now: "<< pMiner->Wealth();

  //wealthy enough to have a well earned rest?
  //(原文注释大意:钱够多了,值得好好休息一下?)

  // 判断:存款 ≥ ComfortLevel(5,常量在 Miner.h)?
  if (pMiner->Wealth() >= ComfortLevel)
  {
    SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " 
         << "WooHoo! Rich enough for now. Back home to mah li'lle lady";
      
    // 够富有 → 切换到"回家睡觉"状态。
    pMiner->ChangeState(GoHomeAndSleepTilRested::Instance());      
  }

  //otherwise get more gold
  //(原文注释:否则就去挖更多金子)

  // else:if 条件不成立时走这里(和 if 二选一)。
  // 没存够 → 切回"挖矿"状态,继续搬砖。
  else 
  {
    pMiner->ChangeState(EnterMineAndDigForNugget::Instance());
  }
}


//--------------------------------------------------------------------------------
//【Exit:离开"银行"状态时执行一次】告别银行。
//--------------------------------------------------------------------------------
void VisitBankAndDepositGold::Exit(Miner* pMiner)
{
  SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Leavin' the bank";
}



//----------------------------------------methods for GoHomeAndSleepTilRested
//(原文分隔注释:下面开始【状态③回家睡觉】的方法实现)

//--------------------------------------------------------------------------------
//【单例入口】获取"回家睡觉状态"的唯一实例(原理同上)。
//--------------------------------------------------------------------------------
GoHomeAndSleepTilRested* GoHomeAndSleepTilRested::Instance()
{
  static GoHomeAndSleepTilRested instance;

  return &instance;
}

//--------------------------------------------------------------------------------
//【Enter:进入"睡觉"状态时执行一次】若矿工还不在小屋,走回家并念台词。
//--------------------------------------------------------------------------------
void GoHomeAndSleepTilRested::Enter(Miner* pMiner)
{
  // 不在小屋(shack) → 赶路回家。
  if (pMiner->Location() != shack)
  {
    SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Walkin' home";

    pMiner->ChangeLocation(shack); 
  }
}

//--------------------------------------------------------------------------------
//【Execute:处于"睡觉"状态期间的每步行动】
// 不困了就精神百倍地回金矿;还困就继续睡(疲劳 -1)。
//--------------------------------------------------------------------------------
void GoHomeAndSleepTilRested::Execute(Miner* pMiner)
{ 
  //if miner is not fatigued start to dig for nuggets again.
  //(原文注释大意:矿工若不再疲劳,就重新开始去挖金子)

  // "!"读作"逻辑非":把真变假、假变真。Fatigued()= 还困吗?
  // !Fatigued() = 不困了吗?→ 不困(疲劳值 ≤ 5)→ 睡醒了!
  if (!pMiner->Fatigued())
  {
    SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY); 
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " 
          << "What a God darn fantastic nap! Time to find more gold";

    // 睡醒 → 切换回"挖矿"状态。
     pMiner->ChangeState(EnterMineAndDigForNugget::Instance());
  }

  // 还困 → 继续睡:else 分支(if 条件不成立时执行)。
  else 
  {
    //sleep
    //(原文注释:睡觉)

    // 睡一觉:疲劳 -1(DecreaseFatigue 实现在 Miner.h 类内)。
    pMiner->DecreaseFatigue();

    SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "ZZZZ... ";
  } 
}

//--------------------------------------------------------------------------------
//【Exit:离开"睡觉"状态时执行一次】告别小屋。
//--------------------------------------------------------------------------------
void GoHomeAndSleepTilRested::Exit(Miner* pMiner)
{ 
  SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Leaving the house";
}




//------------------------------------------------methods for QuenchThirst
//(原文分隔注释:下面开始【状态④去酒吧解渴】的方法实现)

//--------------------------------------------------------------------------------
//【单例入口】获取"解渴状态"的唯一实例(原理同上)。
//--------------------------------------------------------------------------------
QuenchThirst* QuenchThirst::Instance()
{
  static QuenchThirst instance;

  return &instance;
}

//--------------------------------------------------------------------------------
//【Enter:进入"解渴"状态时执行一次】若矿工还不在酒吧,搬过去并念台词。
// (注意本函数里"搬位置"写在"念台词"之前,与前面几个状态相反——顺序无碍。)
//--------------------------------------------------------------------------------
void QuenchThirst::Enter(Miner* pMiner)
{
  // 不在酒吧(saloon) → 先搬位置:
  if (pMiner->Location() != saloon)
  {    
    pMiner->ChangeLocation(saloon);

    SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Boy, ah sure is thusty! Walking to the saloon";
  }
}

//--------------------------------------------------------------------------------
//【Execute:处于"解渴"状态期间的每步行动】买威士忌喝下、回金矿。
// 若走进本状态时根本不渴(理论上不可能——只有 Thirsty() 才会切进来),
// 打印 ERROR! 提醒出 bug 了。
//--------------------------------------------------------------------------------
void QuenchThirst::Execute(Miner* pMiner)
{
   // 防御性检查:真的渴吗?(按状态机逻辑,此时必然渴)
   if (pMiner->Thirsty())
   {
     // 买一杯威士忌喝下:口渴值清零、存款 -2
     // (BuyAndDrinkAWhiskey 实现在 Miner.h 类内)。
     pMiner->BuyAndDrinkAWhiskey();

     SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
     cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "That's mighty fine sippin liquer";

     // 喝爽了 → 切回"挖矿"状态,继续搬砖。
     pMiner->ChangeState(EnterMineAndDigForNugget::Instance());
  }

  // 不渴却进了本状态 = 逻辑错误,打印醒目提示。
  // 字符串里的 "\n" 是换行符,所以会竖着连打三行 ERROR!。
  else 
  {
    SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
    cout << "\nERROR!\nERROR!\nERROR!";
  } 
}

//--------------------------------------------------------------------------------
//【Exit:离开"解渴"状态时执行一次】告别酒吧。
//--------------------------------------------------------------------------------
void QuenchThirst::Exit(Miner* pMiner)
{ 
  SetTextColor(FOREGROUND_RED| FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Leaving the saloon, feelin' good";
}

