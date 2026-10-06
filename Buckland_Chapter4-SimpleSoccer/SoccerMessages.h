//==============================================================================================
//【文件说明】SoccerMessages.h —— 球员之间"喊话"的消息种类清单
//
//【这个文件是干什么的?】
//  足球场上球员不能真说话,AI 之间靠"发消息"协作。本文件把可能用到的消息种类
//  定义成一个枚举 enum MessageType:比如"把球传给我(Msg_PassToMe)"、
// "来支援进攻(Msg_SupportAttacker)"、"回自己位置(Msg_GoHome)"等。
//  另外声明了一个工具函数 MessageToString:把消息编号翻译成文字(调试打印用)。
//
//【谁在使用这个文件?】
//  PlayerBase.cpp / SoccerTeam.cpp / FieldPlayerStates.cpp / GoalKeeperStates.cpp
//  / TeamStates.cpp —— 球员发消息、收消息、翻译消息时用到这些枚举值;
//  SoccerMessages.cpp —— MessageToString 的实现就在那里。
//
//【本文件包含了谁?】
//  <string> —— C++ 标准库的字符串类(std::string),MessageToString 的返回类型要用它。
//
//【C++ 小课堂:enum 枚举(详见 WestWorld1/Locations.h)】
//  花括号里的 5 个枚举项,编译器自动从 0 开始编号:Msg_ReceiveBall=0、Msg_PassToMe=1、
//  Msg_SupportAttacker=2、Msg_GoHome=3、Msg_Wait=4。代码里写名字比写数字好读。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护(include guard):原理详见 Goal.h,此处不再展开。
//--------------------------------------------------------------------------------
#ifndef SOCCER_MESSAGES_H
#define SOCCER_MESSAGES_H

#include <string>
// <string>:标准库字符串头文件(尖括号=系统目录),下面 MessageToString 返回 std::string 要用。

// enum MessageType:定义"消息种类"枚举类型。下面 5 项即球员间可互发的全部消息。
enum MessageType
{
  Msg_ReceiveBall,
  Msg_PassToMe,
  Msg_SupportAttacker,
  Msg_GoHome,
  Msg_Wait
};

//converts an enumerated value to a string
//(原文注释:把一个枚举值转换成对应的字符串)
// inline std::string MessageToString(int msg); —— 函数声明(只有签名);
//   inline:建议内联(函数体很小,省去调用开销);std::string:返回文字;
//   int msg:入参是消息编号(用 int 接收枚举,二者本质都是整数);实现见 SoccerMessages.cpp。
inline std::string MessageToString(int msg);


#endif