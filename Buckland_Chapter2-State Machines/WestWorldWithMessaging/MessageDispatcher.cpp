//==============================================================================================
//【文件说明】MessageDispatcher.cpp —— 消息系统的"邮局办事流程"(实现文件)
//
//【这个文件是干什么的?】
//  头文件 MessageDispatcher.h 声明的三个方法在这里实现:
//    ① Instance()           —— 单例入口:全游戏唯一一家邮局;
//    ② Discharge()          —— 内部投递员:把消息直接交给接收者的 HandleMessage();
//    ③ DispatchMessage()    —— 发消息:延迟<=0 立即投递,否则存入待发队列;
//    ④ DispatchDelayedMessages() —— 主循环每帧调用:把到期的延迟消息取出投递。
//
//【与相关文件的关系】
//  1. MessageDispatcher.h —— 本文件第一行就包含它(类声明、宏 Dispatch);
//  2. BaseGameEntity.h    —— 接收者类型:Discharge 要调用它的 HandleMessage();
//  3. Time/CrudeTimer.h   —— 时钟单例 Clock(记录/判断消息投递时间);
//  4. EntityManager.h     —— 户口本单例 EntityMgr:按编号找发送者/接收者;
//  5. Locations.h         —— 地点枚举(本文件未直接用,包含是原作者习惯);
//  6. MessageTypes.h      —— MsgToStr():把消息编号翻译成文字(打印日志);
//  7. EntityNames.h       —— GetNameOfEntity():把编号翻译成角色名。
//
//【消息的一生(完整流程)】
//  ① 某状态类调用 Dispatch->DispatchMessage(延迟,发送方,接收方,消息,附加);
//  ② DispatchMessage 用 EntityMgr 把"编号"换成"角色指针";
//  ③ 延迟<=0 → 直接 Discharge → 接收者 HandleMessage 当场处理;
//    延迟>0  → 计算投递时间,塞进 PriorityQ(自动按时间排序);
//  ④ main 每帧调用 DispatchDelayedMessages:
//    队头消息的投递时间已到 → Discharge → 从队列删除;
//  ⑤ 接收者(如矿工的睡觉状态)的 OnMessage 决定怎么响应。
//==============================================================================================

// 类声明、宏 Dispatch 都来自这里。
#include "MessageDispatcher.h"
// 接收者类型:Discharge 里要调用 pReceiver->HandleMessage(...)。
#include "BaseGameEntity.h"
// 时钟单例:Clock->GetCurrentTime() 取当前游戏时间(秒)。
#include "Time/CrudeTimer.h"
// 户口本单例:EntityMgr->GetEntityFromID(编号) 把编号换成角色指针。
#include "EntityManager.h"
// 地点枚举(习惯性包含,本文件未直接使用)。
#include "Locations.h"
// MsgToStr():消息编号→文字(打印日志用)。
#include "MessageTypes.h"
// GetNameOfEntity():角色编号→名字(打印日志用)。
#include "EntityNames.h"

#include <iostream>
using std::cout;

// using std::set:声明"我要直接用标准库里的 set 这个名字"。
// 否则每次都要写 std::set。set(集合)是延迟消息队列 PriorityQ 的类型。
using std::set;

//--------------------------------------------------------------------------------
//【可选重定向输出】与状态实现文件相同:取消 TEXTOUTPUT 定义后,
// 消息日志改写入 output.txt(由 main.cpp 打开)。
//--------------------------------------------------------------------------------
#ifdef TEXTOUTPUT
#include <fstream>
extern std::ofstream os;
#define cout os
#endif



//------------------------------ Instance -------------------------------------
// ↓↓↓ 原文标题翻译:Instance(单例入口)
//--------------------------------------------------------------------------------
//【Instance —— 取唯一邮局】函数内 static 局部变量,只在第一次调用时创建,
// 之后每次返回同一个地址——全游戏只有一家邮局。
//--------------------------------------------------------------------------------
MessageDispatcher* MessageDispatcher::Instance()
{
  static MessageDispatcher instance;

  return &instance;
}


//----------------------------- Dispatch ---------------------------------
//  
//  see description in header
//------------------------------------------------------------------------
// ↓↓↓ 原文标题与说明翻译:Dispatch —— 投递
//   说明:见头文件中的描述(即"调用接收者的消息处理函数")。
//--------------------------------------------------------------------------------
//【Discharge —— 内部投递员(把消息交给接收者)】
//  参数:
//    BaseGameEntity* pReceiver :接收者的指针;
//    const Telegram& telegram  :消息"信封"的常量引用。
//  逻辑:调用接收者的 HandleMessage(telegram);
//        若返回 false(接收者没处理掉),打印"消息未被处理"。
//--------------------------------------------------------------------------------
void MessageDispatcher::Discharge(BaseGameEntity* pReceiver,
                                  const Telegram& telegram)
{
  // ! 是"非":HandleMessage 返回 false(没人处理)时进入 if。
  if (!pReceiver->HandleMessage(telegram))
  {
    //telegram could not be handled
    //(原文注释:这条消息没能被处理)

    // 打印提示(括号里是中文译注,原有改动,保留)。
    cout << "Message not handled(消息未被处理)";
  }
}

//---------------------------- DispatchMessage ---------------------------
//
//  given a message, a receiver, a sender and any time delay , this function
//  routes the message to the correct agent (if no delay) or stores
//  in the message queue to be dispatched at the correct time
//------------------------------------------------------------------------
// ↓↓↓ 原文标题与说明翻译:DispatchMessage —— 派发消息
//   说明:给定一条消息、接收者、发送者以及任意时间延迟,本函数
//   要么把消息直接路由给正确的角色(无延迟时),要么存进消息队列,
//   等到正确的时间再派发。
//--------------------------------------------------------------------------------
//【DispatchMessage —— 发消息(对外入口)】
//  参数逐个解释:
//    double delay  :延迟秒数(0 = 立即发);
//    int    sender :发送方编号;
//    int    receiver :接收方编号;
//    int    msg    :消息编号(MessageTypes.h 的枚举);
//    void*  ExtraInfo:附加信息指针(没有传 NO_ADDITIONAL_INFO)。
//--------------------------------------------------------------------------------
void MessageDispatcher::DispatchMessage(double  delay,
                                        int    sender,
                                        int    receiver,
                                        int    msg,
                                        void*  ExtraInfo)
{
  // 红底白字:消息日志用醒目的颜色打印。
  SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);

  //get pointers to the sender and receiver
  //(原文注释:获取发送者和接收者的指针)

  // 凭编号去户口本查人:把"发送方编号"换成"发送者指针"。
  BaseGameEntity* pSender   = EntityMgr->GetEntityFromID(sender);
  // 把"接收方编号"换成"接收者指针"。若编号没登记,GetEntityFromID 会断言。
  BaseGameEntity* pReceiver = EntityMgr->GetEntityFromID(receiver);

  //make sure the receiver is valid
  //(原文注释:确保接收者是有效的)

  // NULL:空指针(哪里都不指向)。== 是"等于"。
  // 防御:万一接收者是空(理论上 GetEntityFromID 已保证非空),就报错退出。
  if (pReceiver == NULL)
  {
    cout << "\nWarning! No Receiver with ID of (警告！未找到ID为) " << receiver << " found(的接收者)";

    return;
  }
  
  //create the telegram
  //(原文注释:创建消息信封)

  // 【构造 Telegram 对象】Telegram telegram(0, sender, receiver, msg, ExtraInfo);
  //   调用 Telegram 的构造函数,打包一封"信":
  //     0         = DispatchTime(投递时间,先填 0,延迟情况下面再改);
  //     sender    = 发信人编号;receiver = 收信人编号;
  //     msg       = 消息编号;ExtraInfo = 附加信息。
  //   (构造函数定义见 Common\Messaging\Telegram.h。)
  Telegram telegram(0, sender, receiver, msg, ExtraInfo);
  
  //if there is no delay, route telegram immediately                       
  //(原文注释:如果没有延迟,立即投递消息)

  // delay <= 0.0f:延迟小于等于 0 → 即时消息,马上投递。
  // <= 是"小于等于";0.0f 是 float 类型的 0。
  if (delay <= 0.0f)                                                        
  {
    // 打印即时消息日志:当前时间、发送方、接收方、消息内容。
    // MsgToStr(msg):消息编号→文字(MessageTypes.h)。
    cout << "\nInstant telegram dispatched at time: (即时电报已派发，时间：) " << Clock->GetCurrentTime()
         << " by (发送方：) " << GetNameOfEntity(pSender->ID()) << " for (接收方：) " << GetNameOfEntity(pReceiver->ID()) 
         << ". Msg is (.消息为：) "<< MsgToStr(msg);

    //send the telegram to the recipient
    //(原文注释:把消息发送给接收者)

    // 立即投递:调用内部投递员 Discharge。
    Discharge(pReceiver, telegram);
  }

  //else calculate the time when the telegram should be dispatched
  //(原文注释:否则,计算消息应该被投递的时间)

  // else:延迟 > 0 → 延迟消息。
  else
  {
    // 取当前游戏时间(秒,自程序启动起算)。
    double CurrentTime = Clock->GetCurrentTime(); 

    // 投递时间 = 当前时间 + 延迟秒数(例如现在 2.0 秒,延迟 1.5,
    // 则投递时间 = 3.5 秒)。
    telegram.DispatchTime = CurrentTime + delay;

    //and put it in the queue
    //(原文注释:把它放进队列)

    // 塞进待发队列 PriorityQ。set 自动按 DispatchTime 从小到大排序,
    // 所以队头永远是"最早该投递"的消息。
    PriorityQ.insert(telegram);   

    // 打印延迟消息日志(登记进队列的信息)。
    cout << "\nDelayed telegram from (延迟电报，发送方：) " << GetNameOfEntity(pSender->ID()) << " recorded at time (记录时间：) " 
            << Clock->GetCurrentTime() << " for (接收方：) " << GetNameOfEntity(pReceiver->ID())
            << ". Msg is (.消息为：) "<< MsgToStr(msg);
            
  }
}


//---------------------- DispatchDelayedMessages -------------------------
//
//  This function dispatches any telegrams with a timestamp that has
//  expired. Any dispatched telegrams are removed from the queue
//------------------------------------------------------------------------
// ↓↓↓ 原文标题与说明翻译:DispatchDelayedMessages —— 派发到期消息
//   说明:本函数派发所有"时间戳已过期"的消息;已派发的消息会从队列中移除。
//--------------------------------------------------------------------------------
//【DispatchDelayedMessages —— 处理到期的延迟消息(主循环每帧调用)】
//  逻辑:只要队头消息的投递时间已到(且 > 0),就取出、投递、删除,
//  循环直到队头不再到期为止。
//--------------------------------------------------------------------------------
void MessageDispatcher::DispatchDelayedMessages()
{
  // 红底白字:消息日志醒目。
  SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
  
  //get current time
  //(原文注释:获取当前时间)

  // 当前游戏时间。
  double CurrentTime = Clock->GetCurrentTime();

  //now peek at the queue to see if any telegrams need dispatching.
  //remove all telegrams from the front of the queue that have gone
  //past their sell by date
  //(原文注释:现在查看队列,看是否有需要派发的消息。把队列头部所有
  //  "过了保质期"(投递时间已到)的消息全部取出。)

  // while 循环:条件为真就一直执行。
  //   ① !PriorityQ.empty()  :队列不为空(还有待发消息);
  //   ② 队头投递时间 < 当前时间:该投递了;
  //   ③ 队头投递时间 > 0    :投递时间有效(>0 才是真延迟消息,
  //                           避免误处理时间戳为 0 的占位消息)。
  // 三者用 &&(并且)连接,同时满足才进入循环体。
  while( !PriorityQ.empty() &&
         (PriorityQ.begin()->DispatchTime < CurrentTime) && 
         (PriorityQ.begin()->DispatchTime > 0) )
  {
    //read the telegram from the front of the queue
    //(原文注释:读取队列头部的消息)

    // *PriorityQ.begin():取队头元素的"引用"(不复制)。
    // const Telegram&:常量引用,只读不修改。
    // begin() 返回指向队头(最早投递)的迭代器;* 是解引用,取出元素本身。
    const Telegram& telegram = *PriorityQ.begin();

    //find the recipient
    //(原文注释:找到接收者)

    // 按消息里登记的接收者编号,去户口本找到接收者指针。
    BaseGameEntity* pReceiver = EntityMgr->GetEntityFromID(telegram.Receiver);

    // 打印"队列消息已就绪"日志。
    cout << "\nQueued telegram ready for dispatch: Sent to (队列中的电报已就绪，派发给：) " 
         << GetNameOfEntity(pReceiver->ID()) << ". Msg is (.消息为：) " << MsgToStr(telegram.Msg);

    //send the telegram to the recipient
    //(原文注释:把消息发送给接收者)

    // 投递给接收者(调用它的 HandleMessage)。
    Discharge(pReceiver, telegram);

    //remove it from the queue
    //(原文注释:把它从队列中移除)

    // 投递完成,从队列中删除这条消息(erase 按迭代器位置删除)。
    PriorityQ.erase(PriorityQ.begin());
  }
}



