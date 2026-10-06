//==============================================================================================
//【文件说明】Raven_Goal_Types.h —— 所有"目标类型"的编号表 + 编号转名字的工具类
//
//【这个文件是干什么的?】
//  机器人脑子里同时挂着一长串目标(思考/探索/寻路/捡血/捡枪/追敌……)。
//  程序内部用整数编号区分它们(goal_think=0、goal_explore=1……,见下面的 enum)。
//  本文件做两件事:① 用匿名 enum 给每种目标分配一个编号;
//  ② 提供 GoalTypeToString 单例,把编号翻译成英文名字(调试时打印用)。
//
//【谁在使用这个文件?】
//  Goal_Think.h/.cpp、Goal_Explore.h、Goal_AttackTarget.h、Goal_HuntTarget.h、
//  Goal_FindTarget.h、Goal_AdjustRange.h、Goal_DodgeSideToSide.h、Goal_GetItem.h、
//  Goal_FollowPath.h、Goal_MoveToPosition.h、Goal_NegotiateDoor.h、Goal_SeekToPosition.h、
//  Goal_TraverseEdge.cpp、Goal_Wander.h、Raven_Bot.cpp、Raven_Game.cpp
//  —— 它们都 #include 本文件来引用 goal_xxx 这些编号。
//
//【本文件包含了谁?】
//  <string>               —— C++ 标准字符串类(std::string);
//  "misc/TypeToString.h"  —— 公共基类 TypeToString(编号转名字的通用模板,
//                             位于 Common\misc\ 目录,由别的分片负责注释)。
//
//【C++ 小课堂:匿名 enum 与单例】
//   enum { a, b, c }; 不给 enum 起名字叫"匿名枚举":编译器自动让 a=0、b=1、c=2……
//   常用来定义一组"有名字的整数常量",比 #define 更安全(有类型、可调试)。
//   static 函数 + 函数内 static 对象 = "单例"模式:全程序只有一个实例。
//==============================================================================================
#ifndef GOAL_ENUMERATIONS_H
#define GOAL_ENUMERATIONS_H

//--------------------------------------------------------------------------------
// #include:拿进本文件需要的零件。<string> 用尖括号表示它是 C++ 标准库头文件;
// "misc/TypeToString.h" 用引号表示它是项目自己的头文件(在 Common\misc 下)。
//--------------------------------------------------------------------------------
#include <string>
#include "misc/TypeToString.h"

//--------------------------------------------------------------------------------
// 匿名枚举:给每种目标分配一个整数编号(从 0 开始依次 +1)。
// 后面 Goal_xxx 类在构造时就用这些编号登记自己属于哪种目标。
//--------------------------------------------------------------------------------
enum
{
  goal_think,
  goal_explore,
  goal_arrive_at_position,
  goal_seek_to_position,
  goal_follow_path,
  goal_traverse_edge,
  goal_move_to_position,
  goal_get_health,
  goal_get_shotgun,
  goal_get_rocket_launcher,
  goal_get_railgun,
  goal_wander,
  goal_negotiate_door,
  goal_attack_target,
  goal_hunt_target,
  goal_strafe,
  goal_adjust_range,
  goal_say_phrase
  
};

//--------------------------------------------------------------------------------
// class GoalTypeToString:编号 → 英文名字 的转换器。
//   ": public TypeToString" 表示它"继承"自公共基类 TypeToString
//   (即"是一个"类型转换器,复用父类的通用框架)。
//--------------------------------------------------------------------------------
class GoalTypeToString : public TypeToString
{

// private 构造函数:把构造函数藏起来=外界不能随便 new,只能通过下面的 Instance()
// 拿到全程序唯一的那个对象(单例模式,详见 WestWorld1\MinerOwnedStates.h 的注释)。
  GoalTypeToString(){}

// public:对外公开的接口。
public:

// static 成员函数:不依赖具体对象也能调用(直接用 类名::Instance())。
// 返回单例指针;Convert 把编号 gt 翻成可读字符串。
  static GoalTypeToString* Instance();
  
  std::string Convert(int gt);
};

#endif
