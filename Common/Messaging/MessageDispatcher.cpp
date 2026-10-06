//==============================================================================================
//【文件说明】MessageDispatcher.cpp —— 消息调度器的实现(单例、发信、延迟投递)
//
//【本文件是干什么的?】
//  实现头文件 MessageDispatcher.h 声明的 4 个函数:
//    Instance()                 —— 返回全局唯一实例的指针;
//    Discharge()               —— 把电报真正交给接收者的 HandleMessage;
//    DispatchMsg()             —— 发消息入口(delay=0 立刻发,delay>0 排队);
//    DispatchDelayedMessages()  —— 每帧调用,把到点的延迟消息投递出去。
//
//【本文件包含了谁?】
//  "MessageDispatcher.h"   —— 自己的声明;
//  "Game/BaseGameEntity.h" —— 实体基类(HandleMessage 定义在它里);
//  "misc/FrameCounter.h"   —— TickCounter 全局帧计数器(取当前时间);
//  "game/EntityManager.h"  —— EntityMgr 实体管理器(按 ID 找接收者);
//  "Debug/DebugConsole.h"   —— debug_con 调试窗口(可选打印消息日志)。
//  using std::set; —— 之后写 set 就等价于 std::set(省键盘)。
//==============================================================================================
#include "MessageDispatcher.h"
#include "Game/BaseGameEntity.h"
#include "misc/FrameCounter.h"
#include "game/EntityManager.h"
#include "Debug/DebugConsole.h"

using std::set;

//uncomment below to send message info to the debug window
//#define SHOW_MESSAGING_INFO

//--------------------------- Instance ----------------------------------------
//
//   this class is a singleton
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Instance:单例模式的经典写法——函数内定义一个 static 局部变量;
// 第一次调用时创建,之后每次都返回同一个对象的地址(&instance)。
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
//--------------------------------------------------------------------------------
// Discharge:送信。调用接收者的 HandleMessage(telegram);
// 若返回 false(接收者处理不了),可选地在调试窗口打一行日志。
//--------------------------------------------------------------------------------
void MessageDispatcher::Discharge(BaseGameEntity* pReceiver, const Telegram& telegram)
{
  if (!pReceiver->HandleMessage(telegram))
  {
    //telegram could not be handled
    #ifdef SHOW_MESSAGING_INFO
    debug_con << "Message not handled" << "";
    #endif
  }
}

//---------------------------- DispatchMsg ---------------------------
//
//  given a message, a receiver, a sender and any time delay, this function
//  routes the message to the correct agent (if no delay) or stores
//  in the message queue to be dispatched at the correct time
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// DispatchMsg:发消息入口。流程:
//   ① 用 EntityMgr 按 receiver ID 找接收者指针;找不到就警告并返回;
//   ② 造一封 Telegram(先把 DispatchTime 置 0);
//   ③ delay<=0:当场 Discharge;
//   ④ delay>0:DispatchTime = 当前帧 + delay,插入 PriorityQ 排队。
//--------------------------------------------------------------------------------
void MessageDispatcher::DispatchMsg(double       delay,
                                    int          sender,
                                    int          receiver,
                                    int          msg,
                                    void*        AdditionalInfo = NULL)
{

  //get a pointer to the receiver
  // EntityMgr 是实体管理器单例的宏(详见 Game/EntityManager.h)。
  BaseGameEntity* pReceiver = EntityMgr->GetEntityFromID(receiver);

  //make sure the receiver is valid
  if (pReceiver == NULL)
  {
    #ifdef SHOW_MESSAGING_INFO
    debug_con << "\nWarning! No Receiver with ID of " << receiver << " found" << "";
    #endif

    return;
  }
  
  //create the telegram
  Telegram telegram(0, sender, receiver, msg, AdditionalInfo);
  
  //if there is no delay, route telegram immediately                       
  if (delay <= 0.0)                                                        
  {
    #ifdef SHOW_MESSAGING_INFO
    debug_con << "\nTelegram dispatched at time: " << TickCounter->GetCurrentFrame()
         << " by " << sender << " for " << receiver 
         << ". Msg is " << msg << "";
    #endif

    //send the telegram to the recipient
    Discharge(pReceiver, telegram);
  }

  //else calculate the time when the telegram should be dispatched
  else
  {
    double CurrentTime = TickCounter->GetCurrentFrame(); 

    telegram.DispatchTime = CurrentTime + delay;

    //and put it in the queue
    PriorityQ.insert(telegram);   

    #ifdef SHOW_MESSAGING_INFO
    debug_con << "\nDelayed telegram from " << sender << " recorded at time " 
            << TickCounter->GetCurrentFrame() << " for " << receiver
            << ". Msg is " << msg << "";
    #endif
  }
}

//---------------------- DispatchDelayedMessages -------------------------
//
//  This function dispatches any telegrams with a timestamp that has
//  expired. Any dispatched telegrams are removed from the queue
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// DispatchDelayedMessages:每帧由游戏主循环调一次。
//  while 循环检查队首(std::set 已按时间排好序):凡到点且时间>0 的电报,
//  取出 → 找接收者 → Discharge → 从队列 erase 删除。
//--------------------------------------------------------------------------------
void MessageDispatcher::DispatchDelayedMessages()
{ 
  //first get current time
  double CurrentTime = TickCounter->GetCurrentFrame(); 

  //now peek at the queue to see if any telegrams need dispatching.
  //remove all telegrams from the front of the queue that have gone
  //past their sell by date
  while( !PriorityQ.empty() &&
	     (PriorityQ.begin()->DispatchTime < CurrentTime) && 
         (PriorityQ.begin()->DispatchTime > 0) )
  {
    //read the telegram from the front of the queue
    const Telegram& telegram = *PriorityQ.begin();

    //find the recipient
    BaseGameEntity* pReceiver = EntityMgr->GetEntityFromID(telegram.Receiver);

    #ifdef SHOW_MESSAGING_INFO
    debug_con << "\nQueued telegram ready for dispatch: Sent to " 
         << pReceiver->ID() << ". Msg is "<< telegram.Msg << "";
    #endif

    //send the telegram to the recipient
    Discharge(pReceiver, telegram);

	//remove it from the queue
    PriorityQ.erase(PriorityQ.begin());
  }
}



