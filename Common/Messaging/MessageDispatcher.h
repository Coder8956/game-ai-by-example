//==============================================================================================
//【文件说明】MessageDispatcher.h —— 全局消息调度器(单例,负责发信/排队/投递)
//
//【这个文件是干什么的?】
//  它是 AI 之间消息的"邮局":任何实体想给别的实体发消息,都交给它。
//  两种发送方式:
//    立刻发(delay=0)        —— 当场调用接收者的消息处理函数;
//    延迟发(delay>0)        —— 把电报存进 std::set(自动按时间排序),
//                              游戏每帧调 DispatchDelayedMessages(),到点才投递。
//
//【发送→排队→投递 全流程】
//   发送方:Dispatcher->DispatchMsg(delay, sender, receiver, msg, extra);
//   调度器:delay=0 当场 Discharge;delay>0 把 Telegram 插入 PriorityQ 按时间排好;
//   每帧:DispatchDelayedMessages() 检查队首,凡到点的电报逐个 Discharge 给接收者;
//   接收者:自身的 HandleMessage(const Telegram&) 函数被调用,决定怎么反应。
//
//【谁在使用这个文件?】
//  第 2 章 WestWorld1(main.cpp/Miner.cpp/Wife.cpp)、第 4 章 SimpleSoccer、
//  第 7~10 章 Raven —— 所有 AI 发消息都走 Dispatcher 这个宏。
//
//【本文件包含了谁?】
//  <set>/<string>        —— 标准库集合(自动排序去重)/字符串;
//  "Messaging/Telegram.h" —— 电报结构定义。
//  class BaseGameEntity;   —— 前置声明(只用到指针,不需要完整定义)。
//
//【C++ 小课堂:单例(Singleton)与宏】
//  单例 = 全程序只有一个实例,私有构造函数禁止别人 new,通过 Instance() 拿唯一指针。
//  #define Dispatcher MessageDispatcher::Instance() 是宏:代码里写 Dispatcher
//  会被预处理器原样替换成 MessageDispatcher::Instance(),写起来更短。
//==============================================================================================
#ifndef MESSAGE_DISPATCHER_H
#define MESSAGE_DISPATCHER_H
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786) 原理详见 SoccerPitch.h;包含保护原理详见 Goal.h。
//--------------------------------------------------------------------------------
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   MessageDispatcher.h
//
//  Desc:   A message dispatcher. Manages messages of the type Telegram.
//          Instantiated as a singleton.
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <set>
#include <string>


#include "Messaging/Telegram.h"


class BaseGameEntity;


//--------------------------------------------------------------------------------
// 下面 3 个常量是"魔法数字"的可读别名:
//   SEND_MSG_IMMEDIATELY = 0.0  —— 延迟为 0 = 立刻发;
//   NO_ADDITIONAL_INFO   = 0    —— 没有附带数据;
//   SENDER_ID_IRRELEVANT = -1   —— 发送者无关(广播类消息用)。
//--------------------------------------------------------------------------------
//to make life easier...
#define Dispatcher MessageDispatcher::Instance()

//to make code easier to read
const double SEND_MSG_IMMEDIATELY = 0.0;
const int    NO_ADDITIONAL_INFO   = 0;
const int    SENDER_ID_IRRELEVANT = -1;


//--------------------------------------------------------------------------------
// class MessageDispatcher —— 消息调度器(单例)。
//--------------------------------------------------------------------------------
class MessageDispatcher
{
private:  
  
  // PriorityQ:延迟消息队列。用 std::set(自动排序+去重),按投递时间从早到晚排好。
  //a std::set is used as the container for the delayed messages
  //because of the benefit of automatic sorting and avoidance
  //of duplicates. Messages are sorted by their dispatch time.
  std::set<Telegram> PriorityQ;

  // Discharge:真正"送信"——调用接收者 pReceiver 的 HandleMessage 函数。
  //this method is utilized by DispatchMsg or DispatchDelayedMessages.
  //This method calls the message handling member function of the receiving
  //entity, pReceiver, with the newly created telegram
  void Discharge(BaseGameEntity* pReceiver, const Telegram& msg);

//--------------------------------------------------------------------------------
// 私有构造 + 私有拷贝构造/赋值:外部无法 new 或复制,保证全程序只有一个实例(单例)。
//--------------------------------------------------------------------------------
  MessageDispatcher(){}

  //copy ctor and assignment should be private
  MessageDispatcher(const MessageDispatcher&);
  MessageDispatcher& operator=(const MessageDispatcher&);

public:

  // Instance():唯一的全局访问入口(静态成员函数,用 类名::Instance() 调用)。
  static MessageDispatcher* Instance();

  // DispatchMsg:发消息(delay>0 排队,delay=0 立刻);DispatchDelayedMessages:每帧调一次,投递到期消息。
  //send a message to another agent. Receiving agent is referenced by ID.
  void DispatchMsg(double      delay,
                   int         sender,
                   int         receiver,
                   int         msg,
                   void*       ExtraInfo);

  //send out any delayed messages. This method is called each time through   
  //the main game loop.
  void DispatchDelayedMessages();
};



#endif