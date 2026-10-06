//==============================================================================================
//【文件说明】Raven_Messages.h —— 机器人之间「私信」的种类编号表
//
//【这个文件是干什么的?】
//  Raven 里的机器人(Bot)不是闷头自己跑,它们之间、以及子系统之间会互相发
//  「私信」(delayed message,延迟消息)。比如:导航器算好了路 → 给 bot 发
//  Msg_PathReady;听到枪声 → 给附近 bot 发 Msg_GunshotSound;子弹打中 →
//  给挨打的 bot 发 Msg_TakeThatMF。本文件就是这张「消息种类」编号表,
//  外加一个 MessageToString:把编号翻译成单词,方便调试打印。
//
//【谁在使用这个文件?】(= 哪些文件 #include 了它)
//  Raven_Bot.cpp(接收/分发消息)、navigation\Raven_PathPlanner.cpp(路算好发消息)、
//  Raven_Door.cpp(收到 Msg_OpenSesame 开门)、Raven_Game.cpp(管理消息收发);
//  triggers\Trigger_SoundNotify.cpp(发枪声消息);
//  armory\ 下 Projectile_*.cpp(子弹命中发消息)、goals\ 下各 Goal_*.cpp
//  (收到路径就绪/挨打等消息后调整行为)——这两类目录属于别的分片,不动。
//
//【本文件包含了谁?】
//  <string> —— 标准库字符串类(MessageToString 要返回字符串)。
//
//【C++ 小课堂:命名 enum(枚举类型)】
//   enum message_type { Msg_Blank, Msg_PathReady, ... };
//   与上一个文件 Raven_ObjectEnumerations.h 的「匿名 enum」几乎一样,区别只是
//   这里给枚举起了个名字 message_type。有了名字后,变量就可以声明成
//   message_type msg(而不是裸 int),类型更安全、可读性更好。
//   花括号里的名字同样从 0 开始自动编号:Msg_Blank=0、Msg_PathReady=1……
//==============================================================================================
//--------------------------------------------------------------------------------
// 原作者文件头注释(Name/Author/Desc),原样保留。
//--------------------------------------------------------------------------------
#ifndef RAVEN_MESSAGES_H
#define RAVEN_MESSAGES_H
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Messages.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   file to enumerate the messages a Raven_Bot must be able to handle
//(原文注释翻译:这个文件用来枚举 Raven_Bot 必须能够处理的那些消息种类)
//-----------------------------------------------------------------------------
#include <string>

//--------------------------------------------------------------------------------
// 命名枚举 message_type:所有合法消息的编号表。
// 含义速记:
//   Msg_Blank          = 空消息(占位,没含义);
//   Msg_PathReady      = 路径已就绪(导航器算好了一条路);
//   Msg_NoPathAvailable= 没有可达路径(算路失败);
//   Msg_TakeThatMF     = 你打中我了(子弹命中目标时发);
//   Msg_YouGotMeYouSOB = 你把我干掉了(我被打死时发);
//   Msg_GoalQueueEmpty = 目标队列空了(没事可干,该想想下一步干啥);
//   Msg_OpenSesame     = 开门(念咒一样,让滑动门开);
//   Msg_GunshotSound   = 枪声(听觉刺激,bot 会转头去看);
//   Msg_UserHasRemovedBot = 用户把某个 bot 删除了。
//--------------------------------------------------------------------------------
enum message_type
{
  Msg_Blank,
  Msg_PathReady,
  Msg_NoPathAvailable,
  Msg_TakeThatMF, 
  Msg_YouGotMeYouSOB,
  Msg_GoalQueueEmpty,
  Msg_OpenSesame,
  Msg_GunshotSound,
  Msg_UserHasRemovedBot
};

//(原文注释:used for outputting debug info = 用来输出调试信息)
//used for outputting debug info
//--------------------------------------------------------------------------------
// MessageToString —— 把消息编号翻译成英文字符串(调试打印用)。
//   int msg :传入的消息编号;返回对应英文名字。
// 函数体直接写在头文件里,所以用 inline 防止重复定义。
//--------------------------------------------------------------------------------
inline std::string MessageToString(int msg)
{
  switch(msg)
  {
  case Msg_PathReady:

    return "Msg_PathReady";

  case Msg_NoPathAvailable:

    return "Msg_NoPathAvailable";

  case Msg_TakeThatMF:

    return "Msg_TakeThatMF";

  case Msg_YouGotMeYouSOB:

    return "Msg_YouGotMeYouSOB";

  case Msg_GoalQueueEmpty:

    return "Msg_GoalQueueEmpty";

  case Msg_OpenSesame:

    return "Msg_OpenSesame";

  case Msg_GunshotSound:

    return "Msg_GunshotSound";

  case Msg_UserHasRemovedBot:

    return "Msg_UserHasRemovedBot";

// default:上面 case 都没匹配上 → 返回「未知消息!」。
  default:

    return "Undefined message!";
  }
}


#endif