//==============================================================================================
//【文件说明】SoccerMessages.cpp —— MessageToString 的实现(把消息编号翻译成文字)
//
//【这个文件是干什么的?】
//  SoccerMessages.h 只声明了 MessageToString,本文件给出它的函数体:传入一个消息编号
//  (int msg),返回对应的英文名字符串。主要用于调试时打印"谁给谁发了什么"。
//
//【本文件包含了谁?】
//  "SoccerMessages.h" —— 自己的声明(枚举 MessageType 与 MessageToString 签名)。
//
//【C++ 小课堂:switch/case 多路分支】
//  switch(表达式) 会拿表达式的值,依次和各 case 后的值比较;命中哪个 case,
//  就从那里开始往下执行;return 立刻返回并结束函数。default:表示"以上都不匹配"
//  时的兜底分支(这里对未知消息号返回提示字符串)。
//==============================================================================================
#include "SoccerMessages.h"


// 函数实现:与头文件里的签名逐字对应。msg 是消息编号,函数体用 switch 把它映射成文字。
inline std::string MessageToString(int msg)
{
// switch(msg):拿消息编号 msg 做多路判断(见文件头【C++ 小课堂】)。
  switch (msg)
  {
  case Msg_ReceiveBall:
    
    return "Msg_ReceiveBall";

  case Msg_PassToMe:
    
    return "Msg_PassToMe";

  case Msg_SupportAttacker:

    return "Msg_SupportAttacker";

  case Msg_GoHome:

    return "Msg_GoHome";

  case Msg_Wait:

    return "Msg_Wait";

// default:兜底分支——msg 不是上面任何一种已知编号时走这里。
  default:

    return "INVALID MESSAGE!!";
  }
}