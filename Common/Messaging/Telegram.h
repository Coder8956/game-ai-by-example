//==============================================================================================
//【文件说明】Telegram.h —— AI 之间传递的"电报"数据结构(消息包)
//
//【这个文件是干什么的?】
//  游戏里的 AI 角色要互相发消息(比如"我看到敌人了"、"球给你")。
//  本文件定义 Telegram(电报)结构:一条消息 = 谁发的 + 给谁 + 消息编号 +
//  投递时间 + 附带数据。它不负责送信,只负责把消息打包好;
//  真正的送信逻辑在 MessageDispatcher(.h/.cpp)里。
//
//【谁在使用这个文件?】
//  MessageDispatcher.cpp/h —— 用 Telegram 做消息排队与投递;
//  第 2 章 WestWorld1 的 Miner/Wife、第 4 章 SimpleSoccer 的球员状态、
//  Raven(第 7~10 章)的 AI,都通过 Telegram 收发消息。
//
//【本文件包含了谁?】
//  <iostream> —— 标准输出流(operator<< 打印电报用);
//  <math.h>  —— C 数学库(fabs 取绝对值,比较投递时间用)。
//
//【C++ 小课堂:struct 与 class 的区别、运算符重载、模板函数】
//  struct 在 C++ 里和 class 几乎一样,唯一区别是 struct 的成员默认 public。
//  operator== / operator< / operator<< 是"运算符重载":给自定义类型定义
//    "相等比较""大小比较""打印"的规则,这样 std::set 才能给电报排序。
//  template <class T> T DereferenceToType(void* p):把无类型指针 void* 强制
//    转回具体类型 T 并解引用(取它指向的值),附消息数据时用。
//==============================================================================================
#ifndef TELEGRAM_H
#define TELEGRAM_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//------------------------------------------------------------------------
//
//  Name:   Telegram.h
//
//  Desc:   This defines a telegram. A telegram is a data structure that
//          records information required to dispatch messages. Messages 
//          are used by game agents to communicate with each other.
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <iostream>
#include <math.h>


//--------------------------------------------------------------------------------
// struct Telegram —— 电报(消息包)。5 个成员:
//   Sender/Receiver —— 发送方/接收方实体的 ID(整数编号);
//   Msg            —— 消息编号(各工程在 MessageTypes.h 里枚举);
//   DispatchTime   —— 计划投递时间(秒);-1 表示立刻投递;
//   ExtraInfo      —— 附带数据的无类型指针(void*,可指向任意东西)。
//--------------------------------------------------------------------------------
struct Telegram
{
  //(原文注释:发这条电报的实体 / 收这条电报的实体 / 消息本身)
  //the entity that sent this telegram
  int          Sender;

  //the entity that is to receive this telegram
  int          Receiver;

  //the message itself. These are all enumerated in the file
  //"MessageTypes.h"
  int          Msg;

  //messages can be dispatched immediately or delayed for a specified amount
  //of time. If a delay is necessary this field is stamped with the time 
  //the message should be dispatched.
  double       DispatchTime;

  //any additional information that may accompany the message
  void*        ExtraInfo;


  // 两个构造函数:空参(全填 -1 默认值);带参(指定时间/谁发/给谁/什么消息/附带数据)。
  //  info = NULL 表示没有附带数据(NULL = 空指针)。
  Telegram():DispatchTime(-1),
                  Sender(-1),
                  Receiver(-1),
                  Msg(-1)
  {}


  Telegram(double time,
           int    sender,
           int    receiver,
           int    msg,
           void*  info = NULL): DispatchTime(time),
                               Sender(sender),
                               Receiver(receiver),
                               Msg(msg),
                               ExtraInfo(info)
  {}
 
};


//--------------------------------------------------------------------------------
// SmallestDelay = 0.25:两条电报的投递时间差若小于 0.25 秒,就视为"同一时刻",
// 用于判重(避免消息队列里塞太多时间几乎相同的电报)。
//--------------------------------------------------------------------------------
//these telegrams will be stored in a priority queue. Therefore the >
//operator needs to be overloaded so that the PQ can sort the telegrams
//by time priority. Note how the times must be smaller than
//SmallestDelay apart before two Telegrams are considered unique.
const double SmallestDelay = 0.25;


  // operator==:两条电报相同 = 时间接近(<0.25)且 发送方/接收方/消息号 都一样。
inline bool operator==(const Telegram& t1, const Telegram& t2)
{
  return ( fabs(t1.DispatchTime-t2.DispatchTime) < SmallestDelay) &&
          (t1.Sender == t2.Sender)        &&
          (t1.Receiver == t2.Receiver)    &&
          (t1.Msg == t2.Msg);
}

  // operator<:按投递时间排序(早的排前面)。std::set 自动用它给电报按时间排好队。
inline bool operator<(const Telegram& t1, const Telegram& t2)
{
  if (t1 == t2)
  {
    return false;
  }

  else
  {
    return  (t1.DispatchTime < t2.DispatchTime);
  }
}

  // operator<<:让 cout << telegram 能直接打印一条电报的内容(调试用)。
inline std::ostream& operator<<(std::ostream& os, const Telegram& t)
{
  os << "time: " << t.DispatchTime << "  Sender: " << t.Sender
     << "   Receiver: " << t.Receiver << "   Msg: " << t.Msg;

  return os;
}

//handy helper function for dereferencing the ExtraInfo field of the Telegram 
//to the required type.
template <class T>
//--------------------------------------------------------------------------------
// DereferenceToType<T>:把 void* 指针 p 转回 T* 再解引用,取出 T 类型的数据。
//  用法:int x = DereferenceToType<int>(msg.ExtraInfo);
//--------------------------------------------------------------------------------
inline T DereferenceToType(void* p)
{
  return *(T*)(p);
}


#endif