//==============================================================================================
//【文件说明】MessageDispatcher.h —— 消息系统的"邮局"(消息分发器,单例)
//
//【这个文件是干什么的?】
//  角色之间不直接调用对方,而是"寄信"——发一条消息,由本类(邮局)负责投递。
//  本类管理两种投递方式:
//    ① 即时消息(延迟 0):立刻把消息交给接收者;
//    ② 延迟消息(延迟 > 0):先放进"待发队列"(按时间排序),到点再投递。
//  这就是游戏 AI 里常用的"消息分发器"(Message Dispatcher)模式。
//
//【与相关文件的关系】
//  1. MessageDispatcher.cpp —— 本类所有方法在那里实现(投递逻辑);
//  2. messaging/Telegram.h   —— 消息"信封"结构:本类的队列里存的就是 Telegram;
//  3. misc/ConsoleUtils.h    —— 控制台工具(打印消息日志要用);
//  4. main.cpp               —— 主循环每帧调用 Dispatch->DispatchDelayedMessages()
//                               处理到期消息;
//  5. MinerOwnedStates.cpp / MinersWifeOwnedStates.cpp —— 通过宏 Dispatch
//                               发消息:Dispatch->DispatchMessage(...);
//  6. BaseGameEntity.h       —— 接收者类型(Discharge 用 BaseGameEntity* 调用)。
//
//【单例模式】邮局全游戏只需要一家,所以也是单例(三件套见下)。
// 宏 Dispatch = MessageDispatcher::Instance() 让发消息变得简短。
//
//【消息投递流程图】
//   发信人调用 DispatchMessage(延迟,发送方,接收方,消息,附加信息)
//      │
//      ├── 延迟 <= 0 → Discharge() → 直接调接收者的 HandleMessage()
//      └── 延迟 > 0  → 存入 PriorityQ(按投递时间排序)
//                          │
//                          └── 主循环每帧调 DispatchDelayedMessages()
//                              → 把到期的消息取出 → Discharge() → HandleMessage()
//==============================================================================================

//------------------------------------------------------------------------------------------------
// 包含保护(原理详解见 Locations.h):防止本文件被重复包含。
//------------------------------------------------------------------------------------------------
#ifndef MESSAGE_DISPATCHER_H
#define MESSAGE_DISPATCHER_H
//------------------------------------------------------------------------
//
//  Name:   MessageDispatcher.h
//
//  Desc:   A message dispatcher. Manages messages of the type Telegram.
//          Instantiated as a singleton.
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:MessageDispatcher.h
//   描述  :A message dispatcher. Manages messages of the type Telegram.
//          Instantiated as a singleton.
//          —— 消息分发器:管理 Telegram 类型的消息,以单例方式实例化。
//   作者  :Mat Buckland,2002 年
//------------------------------------------------------------------------
// 关闭 4786 号警告(旧 VC 对标准库容器调试信息的无意义警告,详见 EntityManager.h)。
#pragma warning (disable:4786)

// <set>:C++ 标准库的"有序集合"容器。下面 PriorityQ 用它存放延迟消息,
// 好处:自动按"投递时间"排序,且自动去重。
#include <set>


// 控制台工具:SetTextColor(改文字颜色,打印日志时用)。
#include "misc/ConsoleUtils.h"
// 消息"信封"结构 Telegram 的完整定义(队列里存的就是它)。
#include "messaging/Telegram.h"

// 【前置声明】class BaseGameEntity;
//   下面 Discharge 的参数用 BaseGameEntity* 指针,只需要知道类型存在。
//   完整定义在 BaseGameEntity.h。
class BaseGameEntity;


//to make code easier to read
//(原文注释:为了让代码更好读)

//--------------------------------------------------------------------------------
//【常量】const double SEND_MSG_IMMEDIATELY = 0.0f;
//  const:常量;double:双精度浮点数;
//  语义:给"延迟 0 秒"起个好名字,发即时消息时直接写这个名字。
//  注意:0.0f 里的 f 表示 float 字面量,赋给 double 会自动转成 0.0。
//--------------------------------------------------------------------------------
const double SEND_MSG_IMMEDIATELY = 0.0f;
// 【常量】const int NO_ADDITIONAL_INFO = 0;
//  语义:表示"本条消息不带附加信息"(Telegram 里有个 void* ExtraInfo 字段,
//  不需要时传这个 0 即可,接收方知道 0 = 没有附加信息)。
const int   NO_ADDITIONAL_INFO   = 0;

//to make life easier...
//(原文注释:为了让生活更轻松...)

// 【宏】#define Dispatch MessageDispatcher::Instance()
//  以后代码里写 Dispatch 就等于写 MessageDispatcher::Instance()。
//  例如 Dispatch->DispatchMessage(...) 就是"给唯一邮局发命令"。
#define Dispatch MessageDispatcher::Instance()


//--------------------------------------------------------------------------------
//【类 MessageDispatcher】消息分发器(邮局)的类定义。
//--------------------------------------------------------------------------------
class MessageDispatcher
{
// private: 私有区。
private:  
  
  //a std::set is used as the container for the delayed messages
  //because of the benefit of automatic sorting and avoidance
  //of duplicates. Messages are sorted by their dispatch time.
  //(原文注释:用 std::set 作为延迟消息的容器,因为它能自动排序、
  //  自动去重。消息按"投递时间"排序。)

  // 【成员】std::set<Telegram> PriorityQ —— 延迟消息的"待发队列"。
  //   set(集合):元素自动按 < 运算符排序、且不允许重复。
  //   Telegram 的 < 运算符在 Telegram.h 里定义(按 DispatchTime 排序),
  //   所以这个队列天然按"投递时间"从早到晚排好;
  //   队列头(begin())就是"最早该投递"的消息。
  //   Q 后缀:Queue(队列)的缩写。
  std::set<Telegram> PriorityQ;

  //this method is utilized by DispatchMessage or DispatchDelayedMessages.
  //This method calls the message handling member function of the receiving
  //entity, pReceiver, with the newly created telegram
  //(原文注释:这个方法被 DispatchMessage 或 DispatchDelayedMessages 使用。
  //  它调用接收实体 pReceiver 的消息处理成员函数,并把新创建的 telegram
  //  传给它)

  // 【私有函数】void Discharge(BaseGameEntity* pReceiver, const Telegram& msg);
  //   真正"把消息交给接收者"的动作:调用 pReceiver->HandleMessage(msg)。
  //   声明为私有 → 只能由本类的两个公有方法内部调用,外部不能直接投递。
  void Discharge(BaseGameEntity* pReceiver, const Telegram& msg);

  // 【私有构造函数】MessageDispatcher(){} —— 单例三件套第 1 件。
  MessageDispatcher(){}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造和赋值运算符应当私有)

  // 拷贝构造/赋值运算符私有:单例三件套第 2 件。
  MessageDispatcher(const MessageDispatcher&);
  MessageDispatcher& operator=(const MessageDispatcher&);

// public: 公有区。
public:

  //this class is a singleton
  //(原文注释:这个类是单例)

  // 【静态 Instance】单例三件套第 3 件:拿唯一邮局的入口(实现见 .cpp)。
  static MessageDispatcher* Instance();

  //send a message to another agent. Receiving agent is referenced by ID.
  //(原文注释:给另一个角色发消息。接收者用编号(ID)来引用)

  // 【公有方法】void DispatchMessage(double delay, int sender, int receiver,
  //                                  int msg, void* ExtraInfo);
  //   发消息的"对外入口",参数逐个解释:
  //     delay    :延迟秒数(0 = 立即发);
  //     sender   :发送方编号;
  //     receiver :接收方编号;
  //     msg      :消息编号(MessageTypes.h 里的枚举);
  //     ExtraInfo:附加信息指针(没有就传 NO_ADDITIONAL_INFO=0)。
  //   内部逻辑(实现见 .cpp):延迟<=0 直接投递,否则存入 PriorityQ。
  void DispatchMessage(double  delay,
                       int    sender,
                       int    receiver,
                       int    msg,
                       void*  ExtraInfo);

  //send out any delayed messages. This method is called each time through   
  //the main game loop.
  //(原文注释:发送所有到期的延迟消息。这个方法在每次主游戏循环时被调用)

  // 【公有方法】void DispatchDelayedMessages();
  //   检查队列头部:把所有"投递时间已到"的消息取出并投递(实现见 .cpp)。
  //   main.cpp 的主循环每帧调用它一次。
  void DispatchDelayedMessages();
};



// 包含保护结束:与文件开头的 #ifndef MESSAGE_DISPATCHER_H 配对。
#endif