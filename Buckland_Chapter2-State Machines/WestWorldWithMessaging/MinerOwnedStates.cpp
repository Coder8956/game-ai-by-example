//==============================================================================================
//【文件说明】MinerOwnedStates.cpp —— 矿工 5 个状态的"具体剧本"(实现文件)
//
//【这个文件是干什么的?】
//  头文件 MinerOwnedStates.h 里声明的 5 个状态类(挖矿/存钱/睡觉/喝酒/吃炖菜),
//  在这里给出全部实现:每个状态的 Instance()(单例)、Enter/Execute/Exit/OnMessage。
//  这是整个程序"矿工的行为逻辑"最集中的地方。
//
//【与相关文件的关系】
//  1. MinerOwnedStates.h   —— 本文件第一行就包含它(5 个状态类的声明);
//  2. fsm/State.h          —— 状态接口(父类),模板实现;
//  3. Miner.h              —— 矿工类完整定义:状态类要通过 pMiner-> 操作矿工数据;
//  4. Locations.h          —— 地点枚举(goldmine/bank/shack/saloon);
//  5. messaging/Telegram.h —— 消息"信封"结构;
//  6. MessageDispatcher.h  —— 发消息用 Dispatch->DispatchMessage(...);
//  7. MessageTypes.h       —— 消息编号(Msg_StewReady、Msg_HiHoneyImHome);
//  8. Time/CrudeTimer.h    —— 时钟单例 Clock(打印消息处理时间);
//  9. EntityNames.h        —— GetNameOfEntity(编号→名字)。
//
//【状态切换是怎么发生的?(核心机制)】
//  状态类里写 pMiner->GetFSM()->ChangeState(下一个状态::Instance()):
//    ① ChangeState 是 StateMachine.h 里的方法;
//    ② 它先调用"旧状态"的 Exit() → 再切换指针 → 再调用"新状态"的 Enter();
//    ③ Instance() 保证每个状态全局只有一份,切换只是换指针,非常轻量。
//==============================================================================================

// 状态类的声明(5 个类都在这里)。
#include "MinerOwnedStates.h"
// 状态接口模板(父类定义),状态类要继承它。
#include "fsm/State.h"
// 矿工的完整定义:下面要调用 pMiner->GetFSM()、Location() 等,必须看到 Miner 类。
#include "Miner.h"
// 地点枚举:goldmine/bank/shack/saloon 这些名字来自这里。
#include "Locations.h"
// 消息"信封"结构:OnMessage 的参数 const Telegram& 需要它。
#include "messaging/Telegram.h"
// 消息分发器:发消息要用 Dispatch 宏(即 MessageDispatcher::Instance())。
#include "MessageDispatcher.h"
// 消息编号:Msg_StewReady、Msg_HiHoneyImHome、SEND_MSG_IMMEDIATELY 等。
#include "MessageTypes.h"
// 时钟单例:Clock->GetCurrentTime() 打印"当前游戏时间"。
#include "Time/CrudeTimer.h"
// 角色编号→名字:GetNameOfEntity() 打印角色名。
#include "EntityNames.h"

#include <iostream>
using std::cout;

//--------------------------------------------------------------------------------
//【可选重定向输出】#ifdef TEXTOUTPUT ... #endif
//  #ifdef 是"如果已定义"的条件编译:若在 Locations.h 里取消
//  //#define TEXTOUTPUT 那行的注释,则本文件把 cout 重定向到文件输出流 os,
//  程序输出会写进 output.txt 而不是屏幕。否则整段被跳过。
//--------------------------------------------------------------------------------
#ifdef TEXTOUTPUT
#include <fstream>
extern std::ofstream os;
#define cout os
#endif


//------------------------------------------------------------------------methods for EnterMineAndDigForNugget
// ↓↓↓ 原文分段标题翻译:以下是"进金矿挖金子"状态的方法实现
//--------------------------------------------------------------------------------
//【Instance —— 单例入口】EnterMineAndDigForNugget* EnterMineAndDigForNugget::Instance()
//  返回类型:指向该状态类的指针;函数名前加"类名::"表示"这是该类的方法"。
//  实现:函数内 static 局部变量 instance —— 只在第一次调用时创建一份,
//  之后每次都返回同一个对象的地址。全游戏只有一个"挖矿状态"。
//--------------------------------------------------------------------------------
EnterMineAndDigForNugget* EnterMineAndDigForNugget::Instance()
{
  static EnterMineAndDigForNugget instance;

  return &instance;
}


//--------------------------------------------------------------------------------
//【Enter —— 进入"挖矿"状态时执行一次】
//  (Miner* pMiner):指向矿工的指针(矿工自己调用时传入 this)。
//  逻辑:如果矿工当前不在金矿,就打印台词并把他搬到金矿。
//--------------------------------------------------------------------------------
void EnterMineAndDigForNugget::Enter(Miner* pMiner)
{
  //if the miner is not already located at the goldmine, he must
  //change location to the gold mine
  //(原文注释:如果矿工还没到金矿,他必须先换到金矿)

  // != 是"不等于":当前地点不是金矿,才需要搬家(已经在金矿就跳过)。
  if (pMiner->Location() != goldmine)
  {
    // cout << :把内容输出到屏幕(流插入运算符 << 把右侧文本送进 cout 流);
    // "\n":换行;GetNameOfEntity(pMiner->ID()):矿工编号→名字;
    // 末尾的 (正走向金矿) 是英文台词的中文译注(原有改动,保留)。
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Walkin' to the goldmine(正走向金矿)";

    // ChangeLocation(goldmine):把矿工的地点改成金矿。
    pMiner->ChangeLocation(goldmine);
  }
}


//--------------------------------------------------------------------------------
//【Execute —— 每帧执行(挖矿的核心逻辑)】
//  步骤:① 金块 +1;② 疲劳 +1;③ 打印"捡起一块金块";
//        ④ 口袋满了 → 去银行存钱;⑤ 口渴了 → 去酒吧解渴。
//  注意:④⑤ 两个 if 顺序执行,若都满足,会先切到银行(④ 先执行)。
//--------------------------------------------------------------------------------
void EnterMineAndDigForNugget::Execute(Miner* pMiner)
{  
  //Now the miner is at the goldmine he digs for gold until he
  //is carrying in excess of MaxNuggets. If he gets thirsty during
  //his digging he packs up work for a while and changes state to
  //gp to the saloon for a whiskey.
  //(原文注释:矿工在金矿一直挖,直到携带的金块超过 MaxNuggets。
  //  挖矿期间若口渴,就暂停工作一段时间,切换状态去酒吧喝杯威士忌。
  //  注意原文最后一行 gp 是 go 的笔误,不影响编译。)

  // AddToGoldCarried(1):口袋金块数 +1。
  pMiner->AddToGoldCarried(1);

  // IncreaseFatigue():疲劳值 +1(挖矿是体力活)。
  pMiner->IncreaseFatigue();

  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Pickin' up a nugget(捡起一块金块)";

  //if enough gold mined, go and put it in the bank
  //(原文注释:如果挖够了金子,就去银行存起来)

  // PocketsFull():口袋是否满了(金块 >= MaxNuggets=3)。
  // 满了 → ChangeState 切换到"去银行存钱"状态。
  if (pMiner->PocketsFull())
  {
    pMiner->GetFSM()->ChangeState(VisitBankAndDepositGold::Instance());
  }

  // Thirsty():是否渴了(口渴 >= ThirstLevel=5)。
  // 渴了 → 切换到"去酒吧解渴"状态。
  if (pMiner->Thirsty())
  {
    pMiner->GetFSM()->ChangeState(QuenchThirst::Instance());
  }
}


//--------------------------------------------------------------------------------
//【Exit —— 离开"挖矿"状态时执行一次】
//  打印告别台词(矿工满载而归)。台词中的 (…) 是中文译注(原有改动,保留)。
//--------------------------------------------------------------------------------
void EnterMineAndDigForNugget::Exit(Miner* pMiner)
{
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " 
       << "Ah'm leavin' the goldmine with mah pockets full o' sweet gold(我正离开金矿，口袋里装满了香甜的金子)";
}


//--------------------------------------------------------------------------------
//【OnMessage —— 矿工在"挖矿"状态收到消息时调用】
//  挖矿时矿工不处理任何消息,直接 return false(表示"我没处理,请转给全局状态")。
//  矿工没有全局状态,所以消息最终无人处理(见 StateMachine::HandleMessage)。
//--------------------------------------------------------------------------------
bool EnterMineAndDigForNugget::OnMessage(Miner* pMiner, const Telegram& msg)
{
  //send msg to global message handler
  //(原文注释:把消息发给全局消息处理器)

  return false;
}

//------------------------------------------------------------------------methods for VisitBankAndDepositGold
// ↓↓↓ 原文分段标题翻译:以下是"去银行存钱"状态的方法实现

// 【Instance —— 单例入口】全游戏只有一个"存钱状态"。
VisitBankAndDepositGold* VisitBankAndDepositGold::Instance()
{
  static VisitBankAndDepositGold instance;

  return &instance;
}

//--------------------------------------------------------------------------------
//【Enter —— 进入"存钱"状态时执行一次】
//  如果矿工不在银行,打印台词并把他搬到银行。
//--------------------------------------------------------------------------------
void VisitBankAndDepositGold::Enter(Miner* pMiner)
{  
  //on entry the miner makes sure he is located at the bank
  //(原文注释:进入状态时,矿工先确保自己位于银行)

  if (pMiner->Location() != bank)
  {
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Goin' to the bank. Yes siree(正走向银行。没错，伙计！)";

    pMiner->ChangeLocation(bank);
  }
}


//--------------------------------------------------------------------------------
//【Execute —— 每帧执行(存钱的核心逻辑)】
//  步骤:① 把口袋里的金块全部转为存款;② 口袋清零;
//        ③ 打印"正在存钱";④ 存款够了(>=5)→ 回家睡觉;否则 → 回去继续挖。
//  (else 分支:存款没到 5,回金矿继续挖。)
//--------------------------------------------------------------------------------
void VisitBankAndDepositGold::Execute(Miner* pMiner)
{
  //deposit the gold
  //(原文注释:存入金子)

  // AddToWealth(口袋金块数):存款 += 口袋金块数(把金块全部换成钱)。
  pMiner->AddToWealth(pMiner->GoldCarried());
    
  // 口袋里的金块清零(已经换成存款了)。
  pMiner->SetGoldCarried(0);

  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " 
       << "Depositing gold. Total savings now: (正在存入黄金。目前总存款：) "<< pMiner->Wealth();

  //wealthy enough to have a well earned rest?
  //(原文注释:存款够多,可以好好休息一下了吗?)

  // Wealth() >= ComfortLevel:存款达到 5(ComfortLevel)→ 该享受生活了。
  if (pMiner->Wealth() >= ComfortLevel)
  {
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " 
         << "WooHoo! Rich enough for now. Back home to mah li'lle lady(哇呼！现在够有钱了。回家找我的小娘子)";
      
    // 切换到"回家睡觉"状态(进家门时会给妻子发"我回来了"消息)。
    pMiner->GetFSM()->ChangeState(GoHomeAndSleepTilRested::Instance());      
  }

  //otherwise get more gold
  //(原文注释:否则,再去挖更多的金子)

  // else:存款还不够 5,回到金矿继续挖(人生循环)。
  else 
  {
    pMiner->GetFSM()->ChangeState(EnterMineAndDigForNugget::Instance());
  }
}


//--------------------------------------------------------------------------------
//【Exit —— 离开"存钱"状态时执行一次】打印告别台词。
//--------------------------------------------------------------------------------
void VisitBankAndDepositGold::Exit(Miner* pMiner)
{
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Leavin' the bank(正离开银行)";
}


//--------------------------------------------------------------------------------
//【OnMessage —— 存钱时不处理消息】直接返回 false。
//--------------------------------------------------------------------------------
bool VisitBankAndDepositGold::OnMessage(Miner* pMiner, const Telegram& msg)
{
  //send msg to global message handler
  //(原文注释:把消息发给全局消息处理器)

  return false;
}
//------------------------------------------------------------------------methods for GoHomeAndSleepTilRested
// ↓↓↓ 原文分段标题翻译:以下是"回家睡觉"状态的方法实现

// 【Instance —— 单例入口】全游戏只有一个"睡觉状态"。
GoHomeAndSleepTilRested* GoHomeAndSleepTilRested::Instance()
{
  static GoHomeAndSleepTilRested instance;

  return &instance;
}

//--------------------------------------------------------------------------------
//【Enter —— 进入"睡觉"状态时执行一次(重点:发消息!)】
//  步骤:① 若不在小屋 → 搬回家;
//        ② 立刻给妻子 Elsa 发一条"亲爱的,我回来了"消息(即时发送)。
//  这正是本工程与 WestWorld1 的最大区别:角色之间用消息通信!
//--------------------------------------------------------------------------------
void GoHomeAndSleepTilRested::Enter(Miner* pMiner)
{
  if (pMiner->Location() != shack)
  {
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Walkin' home(正走回家)";

    pMiner->ChangeLocation(shack); 

    //let the wife know I'm home
    //(原文注释:让妻子知道我回家了)

    // Dispatch:宏,即 MessageDispatcher::Instance()(消息分发器单例);
    // ->DispatchMessage(...):发一条消息。各参数逐行解释:
    Dispatch->DispatchMessage(SEND_MSG_IMMEDIATELY, //time delay
    // ① SEND_MSG_IMMEDIATELY = 0.0:延迟 0 秒,立即发送;
                              pMiner->ID(),        //ID of sender
    // ② 发送方编号 = 矿工自己;
                              ent_Elsa,            //ID of recipient
    // ③ 接收方编号 = 妻子 Elsa(来自 EntityNames.h);
                              Msg_HiHoneyImHome,   //the message
    // ④ 消息类型 = "亲爱的,我回来了"(来自 MessageTypes.h);
                              NO_ADDITIONAL_INFO);    
    // ⑤ NO_ADDITIONAL_INFO = 0:不带额外信息。
    // 消息的后续流程见 MessageDispatcher.cpp 的注释。
  }
}

//--------------------------------------------------------------------------------
//【Execute —— 每帧执行(睡觉的核心逻辑)】
//  逻辑:① 不累了(疲劳 <= 阈值)→ 起床去挖矿;
//        ② 还累 → 睡觉:疲劳 -1,打印呼噜声。
//--------------------------------------------------------------------------------
void GoHomeAndSleepTilRested::Execute(Miner* pMiner)
{ 
  //if miner is not fatigued start to dig for nuggets again.
  //(原文注释:如果矿工不累了,就再次开始挖金块)

  // ! 是"逻辑非":!Fatigued() = 不累。不累就起床挖矿。
  if (!pMiner->Fatigued())
  {
     cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " 
          << "All mah fatigue has drained away. Time to find more gold!(我的疲劳已经全部消除了。是时候去找更多金子了！)";

     pMiner->GetFSM()->ChangeState(EnterMineAndDigForNugget::Instance());
  }

  // else:还累,继续睡。
  else 
  {
    //sleep
    //(原文注释:睡觉)

    // DecreaseFatigue():疲劳值 -1(睡一步,解乏一点)。
    pMiner->DecreaseFatigue();

    // 打印呼噜声。注意分号前有个空格,是原有写法,保留不动。
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "ZZZZ... (打呼噜...)" ; 
  } 
}

//--------------------------------------------------------------------------------
//【Exit —— 离开"睡觉"状态】空实现:起床不需要特别动作。
//--------------------------------------------------------------------------------
void GoHomeAndSleepTilRested::Exit(Miner* pMiner)
{ 
}


//--------------------------------------------------------------------------------
//【OnMessage —— 睡觉时收到消息(重点:唯一处理消息的矿工状态!)】
//  矿工在睡觉时,会响应妻子发来的"炖菜好了"(Msg_StewReady)消息:
//    ① 打印"消息已处理"和当前时间;
//    ② 切换状态 → 去吃炖菜(EatStew),并返回 true(消息已被处理)。
//  其他消息一律 return false。
//--------------------------------------------------------------------------------
bool GoHomeAndSleepTilRested::OnMessage(Miner* pMiner, const Telegram& msg)
{
   // SetTextColor:把文字颜色设为"红底白字",让消息日志醒目。
   // BACKGROUND_RED:背景红色;FOREGROUND_RED|GREEN|BLUE:前景白(三色叠加);
   // | 是按位或,把多个颜色标志合并成一个数。
   SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);

   // switch(msg.Msg):按消息编号分派(点号 . 访问结构体成员 Msg)。
   switch(msg.Msg)
   {
   case Msg_StewReady:

     // 收到"炖菜好了"消息:打印处理日志(含处理者和当前游戏时间)。
     // Clock:宏,即 CrudeTimer::Instance()(计时器单例)。
     cout << "\nMessage handled by (消息已处理，处理者：) " << GetNameOfEntity(pMiner->ID()) 
     << " at time: (时间：) " << Clock->GetCurrentTime();

     // 把文字颜色改回矿工的红色。
     SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);

     cout << "\n" << GetNameOfEntity(pMiner->ID()) 
          << ": Okay Hun, ahm a comin'!(好的亲爱的，我这就来！)";

     // 切换状态 → 吃炖菜(吃完会 RevertToPreviousState 回到睡觉状态)。
     pMiner->GetFSM()->ChangeState(EatStew::Instance());
      
     return true;

   }//end switch
   //(原文注释:switch 结束)

   // 其他消息:不处理,交给全局消息处理器(矿工没有全局状态,所以最终没人管)。
   return false; //send message to global message handler
}

//------------------------------------------------------------------------QuenchThirst
// ↓↓↓ 原文分段标题翻译:以下是"去酒吧解渴"状态的方法实现

// 【Instance —— 单例入口】全游戏只有一个"喝酒状态"。
QuenchThirst* QuenchThirst::Instance()
{
  static QuenchThirst instance;

  return &instance;
}

//--------------------------------------------------------------------------------
//【Enter —— 进入"喝酒"状态时执行一次】
//  若不在酒吧,先搬到酒吧,再打印台词。
//--------------------------------------------------------------------------------
void QuenchThirst::Enter(Miner* pMiner)
{
  if (pMiner->Location() != saloon)
  {    
    pMiner->ChangeLocation(saloon);

    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Boy, ah sure is thusty! Walking to the saloon(天哪，我确实渴了！正走向酒馆)";
  }
}

//--------------------------------------------------------------------------------
//【Execute —— 每帧执行(喝酒)】
//  步骤:① 买酒喝(口渴清零、存款 -2);② 打印台词;
//        ③ 喝完了 → 回金矿继续挖矿。
//--------------------------------------------------------------------------------
void QuenchThirst::Execute(Miner* pMiner)
{
  // BuyAndDrinkAWhiskey():口渴值清零,存款扣 2(一杯威士忌的价钱)。
  pMiner->BuyAndDrinkAWhiskey();

  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "That's mighty fine sippin' liquer(这酒喝起来可真不错)";

  // 解渴完成,回金矿上班。
  pMiner->GetFSM()->ChangeState(EnterMineAndDigForNugget::Instance());  
}


//--------------------------------------------------------------------------------
//【Exit —— 离开"喝酒"状态时执行一次】打印告别台词。
//--------------------------------------------------------------------------------
void QuenchThirst::Exit(Miner* pMiner)
{ 
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Leaving the saloon, feelin' good(离开酒馆，感觉很好)";
}


//--------------------------------------------------------------------------------
//【OnMessage —— 喝酒时不处理消息】直接返回 false。
//--------------------------------------------------------------------------------
bool QuenchThirst::OnMessage(Miner* pMiner, const Telegram& msg)
{
  //send msg to global message handler
  //(原文注释:把消息发给全局消息处理器)

  return false;
}

//------------------------------------------------------------------------EatStew
// ↓↓↓ 原文分段标题翻译:以下是"吃炖菜"状态的方法实现

// 【Instance —— 单例入口】全游戏只有一个"吃炖菜状态"。
EatStew* EatStew::Instance()
{
  static EatStew instance;

  return &instance;
}


//--------------------------------------------------------------------------------
//【Enter —— 进入"吃炖菜"状态】打印台词:夸赞妻子的手艺。
//--------------------------------------------------------------------------------
void EatStew::Enter(Miner* pMiner)
{
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Smells Reaaal goood Elsa!(闻起来真香啊，艾尔莎！)";
}

//--------------------------------------------------------------------------------
//【Execute —— 每帧执行(吃炖菜)】
//  吃完后调用 RevertToPreviousState():切回"上一个状态"。
//  这就是"状态闪回"(state blip)——吃完就回去接着睡/接着干活。
//  上一个状态由 StateMachine 在 ChangeState 时自动记录(见 StateMachine.h)。
//--------------------------------------------------------------------------------
void EatStew::Execute(Miner* pMiner)
{
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Tastes real good too!(尝起来也真不错！)";

  // RevertToPreviousState():回到上一个状态(睡觉/挖矿……)。
  pMiner->GetFSM()->RevertToPreviousState();
}

//--------------------------------------------------------------------------------
//【Exit —— 离开"吃炖菜"状态】打印感谢台词。
//--------------------------------------------------------------------------------
void EatStew::Exit(Miner* pMiner)
{ 
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Thankya li'lle lady. Ah better get back to whatever ah wuz doin'(谢谢你，小娘子。我最好回去继续刚才在做的事)";
}


//--------------------------------------------------------------------------------
//【OnMessage —— 吃炖菜时不处理消息】直接返回 false。
//--------------------------------------------------------------------------------
bool EatStew::OnMessage(Miner* pMiner, const Telegram& msg)
{
  //send msg to global message handler
  //(原文注释:把消息发给全局消息处理器)

  return false;
}


