//==============================================================================================
//【文件说明】MinersWifeOwnedStates.cpp —— 妻子 4 个状态的"具体剧本"(实现文件)
//
//【这个文件是干什么的?】
//  头文件 MinersWifeOwnedStates.h 里声明的 4 个状态类
//  (全局状态/做家务/去厕所/炖菜),在这里给出全部实现:
//  每个状态的 Instance()(单例)、Enter/Execute/Exit/OnMessage。
//  其中"全局状态"是妻子独有的——她每帧都会先执行它。
//
//【与相关文件的关系】
//  1. MinersWifeOwnedStates.h —— 4 个状态类的声明;
//  2. MinerOwnedStates.h       —— 会用到(炖菜消息发完后切回 DoHouseWork,
//                                   而 DoHouseWork 定义在 MinersWifeOwnedStates.h,
//                                   这里包含它是为保险/习惯);
//  3. MinersWife.h             —— 妻子的完整定义(wife-> 操作数据);
//  4. Locations.h              —— 地点枚举(妻子基本在小屋);
//  5. Time/CrudeTimer.h        —— 时钟单例 Clock(打印消息处理时间);
//  6. MessageDispatcher.h      —— 发消息:Dispatch->DispatchMessage(...);
//  7. MessageTypes.h           —— 消息编号(Msg_HiHoneyImHome、Msg_StewReady);
//  8. EntityNames.h            —— GetNameOfEntity(编号→名字);
//  9. misc/Utils.h(间接)       —— RandFloat()/RandInt()(随机数)。
//
//【本文件的"剧情主线"】
//  丈夫回家 → 妻子收"我回来了"消息(全局状态) → 切到炖菜 → 放菜进烤箱、
//  给自己发"延迟 1.5 秒"的炖菜好了消息 → 期间每帧执行"忙着做吃的" →
//  1.5 秒后收到自己的消息(炖菜状态) → 告诉丈夫"炖菜好了"、关火、
//  切回做家务 → 丈夫收到消息去吃炖菜。与此同时,全局状态每帧有 10%
//  概率让妻子溜去上厕所,上完再回来接着干。
//==============================================================================================

// 妻子的状态类声明(4 个类都在这里)。
#include "MinersWifeOwnedStates.h"
// 矿工的状态类声明(本文件会引用矿工相关符号,包含它是保险写法)。
#include "MinerOwnedStates.h"
// 妻子的完整定义:下面要用 wife->GetFSM()、SetCooking() 等,必须看到类内容。
#include "MinersWife.h"
// 地点枚举(妻子用 shack 等)。
#include "Locations.h"
// 时钟单例:Clock->GetCurrentTime() 打印当前游戏时间。
#include "Time/CrudeTimer.h"
// 消息分发器:发消息用 Dispatch 宏。
#include "MessageDispatcher.h"
// 消息编号:Msg_HiHoneyImHome、Msg_StewReady、SEND_MSG_IMMEDIATELY。
#include "MessageTypes.h"
// 角色编号→名字:GetNameOfEntity()。
#include "EntityNames.h"

#include <iostream>
using std::cout;

//--------------------------------------------------------------------------------
//【可选重定向输出】与 MinerOwnedStates.cpp 相同:
//  若在 Locations.h 取消 //#define TEXTOUTPUT 的注释,输出改写入 output.txt。
//--------------------------------------------------------------------------------
#ifdef TEXTOUTPUT
#include <fstream>
extern std::ofstream os;
#define cout os
#endif

//-----------------------------------------------------------------------Global state
// ↓↓↓ 原文分段标题翻译:以下是"全局状态"的方法实现

// 【Instance —— 单例入口】全游戏只有一个"妻子的全局状态"。
WifesGlobalState* WifesGlobalState::Instance()
{
  static WifesGlobalState instance;

  return &instance;
}


//--------------------------------------------------------------------------------
//【Execute —— 全局状态每帧执行(随机念头)】
//  逻辑:有 10% 概率"想去厕所",且此刻她不在厕所 → 切换到"去厕所"状态。
//  (全局状态的特点:不管当前在干嘛,每帧都先跑这个函数。)
//--------------------------------------------------------------------------------
void WifesGlobalState::Execute(MinersWife* wife)
{
  //1 in 10 chance of needing the bathroom (provided she is not already
  //in the bathroom)
  //(原文注释:有十分之一的概率需要上厕所(前提是她还没在厕所里))

  // RandFloat():Common 工具,返回 [0,1) 的随机小数;
  // < 0.1:随机值小于 0.1 = 10% 概率触发;
  // && 是"并且":两个条件同时满足才进入 if。
  if ( (RandFloat() < 0.1) && 
  // ! 是"非";isInState(*VisitBathroom::Instance()):判断"当前状态是否是去厕所";
  // 整句 = "她当前不在厕所"(避免已经在厕所又切去厕所的死循环)。
       !wife->GetFSM()->isInState(*VisitBathroom::Instance()) )
  {
    // 切换状态 → 去厕所。上完厕所会 RevertToPreviousState 回到之前状态。
    wife->GetFSM()->ChangeState(VisitBathroom::Instance());
  }
}

//--------------------------------------------------------------------------------
//【OnMessage —— 全局状态收消息(重点:妻子处理"丈夫回来了")】
//  收到 Msg_HiHoneyImHome("亲爱的我回来了")→
//    ① 打印消息处理日志(含处理者和时间);
//    ② 打印台词"让我给你做点炖菜";
//    ③ 切换到"炖菜"状态;④ 返回 true(消息已被处理)。
//  其他消息一律 return false。
//  注意:矿工处理"炖菜好了"消息用的是"当前状态"(睡觉状态)的 OnMessage;
//  妻子处理"我回来了"消息用的是"全局状态"的 OnMessage——两种用法都展示了。
//--------------------------------------------------------------------------------
bool WifesGlobalState::OnMessage(MinersWife* wife, const Telegram& msg)
{
  // 红底白字:让消息日志醒目。
  SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);

  // switch(msg.Msg):按消息编号分派。
  switch(msg.Msg)
  {
  case Msg_HiHoneyImHome:
   {
       // 收到"我回来了":打印日志(处理者 + 时间)。
       // Clock:宏,即 CrudeTimer::Instance()。
       cout << "\nMessage handled by (消息已处理，处理者：) " << GetNameOfEntity(wife->ID()) << " at time: (时间：) " 
       << Clock->GetCurrentTime();

     // 把文字颜色改回妻子的绿色。
     SetTextColor(FOREGROUND_GREEN|FOREGROUND_INTENSITY);

     // 打印台词:要给丈夫做炖菜。
     cout << "\n" << GetNameOfEntity(wife->ID()) << 
          ": Hi honey. Let me make you some of mah fine country stew(嗨，亲爱的。让我给你做些我拿手的乡村炖菜)";

     // 切换到"炖菜"状态(进入炖菜状态后会给"自己"发延迟消息)。
     wife->GetFSM()->ChangeState(CookStew::Instance());
   }

   return true;

  }//end switch
  //(原文注释:switch 结束)

  // 其他消息不处理。
  return false;
}

//-------------------------------------------------------------------------DoHouseWork
// ↓↓↓ 原文分段标题翻译:以下是"做家务"状态的方法实现

// 【Instance —— 单例入口】全游戏只有一个"做家务状态"。
DoHouseWork* DoHouseWork::Instance()
{
  static DoHouseWork instance;

  return &instance;
}


//--------------------------------------------------------------------------------
//【Enter —— 进入"做家务"状态】打印台词:开始干活。
//--------------------------------------------------------------------------------
void DoHouseWork::Enter(MinersWife* wife)
{
  cout << "\n" << GetNameOfEntity(wife->ID()) << ": Time to do some more housework!(是时候再做点家务了！)";
}


//--------------------------------------------------------------------------------
//【Execute —— 每帧执行(随机挑一件家务)】
//  RandInt(0,2):返回 0~2 的随机整数;switch 按结果三选一:
//    0 → 拖地;1 → 洗碗;2 → 铺床。每帧都随机换着干。
//  (switch 里每个 case 末尾的 break:跳出 switch,防止继续执行下面的 case。)
//--------------------------------------------------------------------------------
void DoHouseWork::Execute(MinersWife* wife)
{
  switch(RandInt(0,2))
  {
  case 0:

    cout << "\n" << GetNameOfEntity(wife->ID()) << ": Moppin' the floor(正在拖地)";

    break;

  case 1:

    cout << "\n" << GetNameOfEntity(wife->ID()) << ": Washin' the dishes(正在洗碗)";

    break;

  case 2:

    cout << "\n" << GetNameOfEntity(wife->ID()) << ": Makin' the bed(正在铺床)";

    break;
  }
}

//--------------------------------------------------------------------------------
//【Exit —— 离开"做家务"状态】空实现:不需要特别动作。
//--------------------------------------------------------------------------------
void DoHouseWork::Exit(MinersWife* wife)
{
}

//--------------------------------------------------------------------------------
//【OnMessage —— 做家务时不处理消息】返回 false(交给全局状态)。
//--------------------------------------------------------------------------------
bool DoHouseWork::OnMessage(MinersWife* wife, const Telegram& msg)
{
  return false;
}

//------------------------------------------------------------------------VisitBathroom
// ↓↓↓ 原文分段标题翻译:以下是"去厕所"状态的方法实现

// 【Instance —— 单例入口】全游戏只有一个"去厕所状态"。
VisitBathroom* VisitBathroom::Instance()
{
  static VisitBathroom instance;

  return &instance;
}


//--------------------------------------------------------------------------------
//【Enter —— 进入"去厕所"状态】打印台词。
//--------------------------------------------------------------------------------
void VisitBathroom::Enter(MinersWife* wife)
{  
  cout << "\n" << GetNameOfEntity(wife->ID()) << ": Walkin' to the can. Need to powda mah pretty li'lle nose(正走向厕所。需要给我的漂亮小鼻子补补粉)"; 
}


//--------------------------------------------------------------------------------
//【Execute —— 每帧执行(上厕所)】
//  打印"真舒服"后,立刻 RevertToPreviousState() 回到上一个状态
//  (通常是做家务)——"去厕所"是个临时插曲,上完就回去。
//--------------------------------------------------------------------------------
void VisitBathroom::Execute(MinersWife* wife)
{
  cout << "\n" << GetNameOfEntity(wife->ID()) << ": Ahhhhhh! Sweet relief!(啊——！真舒服！)";

  wife->GetFSM()->RevertToPreviousState();
}

//--------------------------------------------------------------------------------
//【Exit —— 离开"去厕所"状态】打印台词。
//--------------------------------------------------------------------------------
void VisitBathroom::Exit(MinersWife* wife)
{
  cout << "\n" << GetNameOfEntity(wife->ID()) << ": Leavin' the Jon(正离开厕所)";
}


//--------------------------------------------------------------------------------
//【OnMessage —— 上厕所时不处理消息】返回 false。
//--------------------------------------------------------------------------------
bool VisitBathroom::OnMessage(MinersWife* wife, const Telegram& msg)
{
  return false;
}


//------------------------------------------------------------------------CookStew
// ↓↓↓ 原文分段标题翻译:以下是"炖菜"状态的方法实现

// 【Instance —— 单例入口】全游戏只有一个"炖菜状态"。
CookStew* CookStew::Instance()
{
  static CookStew instance;

  return &instance;
}


//--------------------------------------------------------------------------------
//【Enter —— 进入"炖菜"状态(重点:给自己发延迟消息!)】
//  步骤:① 若还没在炖 → 放菜进烤箱、打印台词;
//        ② 给自己发一条"延迟 1.5 秒"的"炖菜好了"消息(炖菜需要时间);
//        ③ 打开 m_bCooking 开关(防止重复放菜)。
//  消息机制:自己给自己发延迟消息 = "1.5 秒后提醒我"。到点后消息分发器
//  会调用她的 OnMessage(见 MessageDispatcher.cpp)。
//--------------------------------------------------------------------------------
void CookStew::Enter(MinersWife* wife)
{
  //if not already cooking put the stew in the oven
  //(原文注释:如果还没在炖菜,就把炖菜放进烤箱)

  // Cooking():是否已在炖菜(m_bCooking 开关)。! 表示"还没在炖"。
  if (!wife->Cooking())
  {
    cout << "\n" << GetNameOfEntity(wife->ID()) << ": Putting the stew in the oven(把炖菜放进烤箱)";
  
    //send a delayed message myself so that I know when to take the stew
    //out of the oven
    //(原文注释:给自己发一条延迟消息,好知道什么时候该把炖菜从烤箱取出)

    // Dispatch->DispatchMessage(...):发消息。参数逐行:
    Dispatch->DispatchMessage(1.5,                  //time delay
    // ① 1.5:延迟 1.5 秒后送达;
                              wife->ID(),           //sender ID
    // ② 发送方 = 妻子自己;
                              wife->ID(),           //receiver ID
    // ③ 接收方 = 还是妻子自己(自己提醒自己);
                              Msg_StewReady,        //msg
    // ④ 消息类型 = "炖菜好了";
                              NO_ADDITIONAL_INFO); 
    // ⑤ 不带额外信息。到点后她会在 CookStew::OnMessage 里收到这条消息。
    // 延迟消息的排队、到点派发逻辑见 MessageDispatcher.cpp。

    // SetCooking(true):打开"在炖菜"开关,防止重复进入重复放菜。
    wife->SetCooking(true);
  }
}


//--------------------------------------------------------------------------------
//【Execute —— 每帧执行(等炖菜)】
//  炖菜需要时间:在收到延迟消息前的每一帧,她都"忙着做吃的"。
//--------------------------------------------------------------------------------
void CookStew::Execute(MinersWife* wife)
{
  cout << "\n" << GetNameOfEntity(wife->ID()) << ": Fussin' over food(正忙着做吃的)";
}

//--------------------------------------------------------------------------------
//【Exit —— 离开"炖菜"状态】打印"把炖菜端上桌"。
//--------------------------------------------------------------------------------
void CookStew::Exit(MinersWife* wife)
{
  // 文字颜色改回妻子的绿色。
  SetTextColor(FOREGROUND_GREEN|FOREGROUND_INTENSITY);
  
  cout << "\n" << GetNameOfEntity(wife->ID()) << ": Puttin' the stew on the table(把炖菜端上桌)";
}


//--------------------------------------------------------------------------------
//【OnMessage —— 炖菜时收到消息(重点:处理"炖菜好了")】
//  收到 Msg_StewReady(自己 1.5 秒前发的延迟消息)→
//    ① 打印"消息已收到"和当前时间;
//    ② 打印"炖菜好了!我们来吃吧";
//    ③ 立即给丈夫发"炖菜好了"消息(通知他回家吃饭);
//    ④ 关闭 m_bCooking 开关;⑤ 切回"做家务"状态;⑥ 返回 true。
//  其他消息一律 return false。
//--------------------------------------------------------------------------------
bool CookStew::OnMessage(MinersWife* wife, const Telegram& msg)
{
  // 红底白字:消息日志醒目。
  SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);

  switch(msg.Msg)
  {
    case Msg_StewReady:
    {
      // 收到自己的延迟消息:打印日志(接收者 + 时间)。
      cout << "\nMessage received by (消息已收到，接收者：) " << GetNameOfEntity(wife->ID()) <<
           " at time: (时间：) " << Clock->GetCurrentTime();

      // 文字颜色改回妻子的绿色。
      SetTextColor(FOREGROUND_GREEN|FOREGROUND_INTENSITY);
      cout << "\n" << GetNameOfEntity(wife->ID()) << ": StewReady! Lets eat(炖菜好了！我们来吃吧)";

      //let hubby know the stew is ready
      //(原文注释:让丈夫知道炖菜好了)

      // 立即给丈夫发"炖菜好了"消息(丈夫在睡觉状态收到后会去吃炖菜)。
      // SEND_MSG_IMMEDIATELY = 0:延迟 0 秒;
      // 发送方=妻子;接收方=矿工 Bob;消息=炖菜好了;无附加信息。
      Dispatch->DispatchMessage(SEND_MSG_IMMEDIATELY,
                                wife->ID(),
                                ent_Miner_Bob,
                                Msg_StewReady,
                                NO_ADDITIONAL_INFO);

      // 关闭"在炖菜"开关(这锅炖菜结束了)。
      wife->SetCooking(false);

      // 切回"做家务"状态(不是回上一个状态,而是回到日常)。
      wife->GetFSM()->ChangeState(DoHouseWork::Instance());               
    }

    return true;

  }//end switch
  //(原文注释:switch 结束)

  // 其他消息不处理。
  return false;
}