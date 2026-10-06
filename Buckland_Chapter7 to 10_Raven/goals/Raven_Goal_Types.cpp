//==============================================================================================
//【文件说明】Raven_Goal_Types.cpp —— GoalTypeToString 单例与 Convert 的实现
//
//【这个文件是干什么的?】
//  头文件 Raven_Goal_Types.h 只"声明"了 GoalTypeToString 类;真正的函数体写在这里:
//  Instance() 负责造出全程序唯一的转换器对象;Convert(编号) 用 switch 把
//  每个 goal_xxx 编号翻译成对应的英文字符串(调试画屏、日志时看)。
//
//【谁在使用本文件?】—— 编译后和 Raven_Goal_Types.h 配套,谁包含那个 .h 就用到本实现。
//==============================================================================================
// 包含本类自己的头文件(函数声明在里面)。
#include "Raven_Goal_Types.h"


//--------------------------------------------------------------------------------
// Instance():返回单例指针。GoalTypeToString:: 表示"这个函数属于 GoalTypeToString 类"。
// 函数内部 static 局部对象:程序第一次走到这行时才创建,之后一直复用同一个,
// 从而保证全程序只有一个实例(单例)。&instance 取对象地址(指针)返回。
//--------------------------------------------------------------------------------
GoalTypeToString* GoalTypeToString::Instance()
{
  static GoalTypeToString instance;
  return &instance;
}

//--------------------------------------------------------------------------------
// Convert(int gt):把目标编号 gt 翻成字符串。std:: 是命名空间前缀
// (string 属于标准库 std 命名空间)。下面 switch 按编号分支,命中哪个就返回哪个名字。
//--------------------------------------------------------------------------------
std::string GoalTypeToString::Convert(int gt)
{
  switch(gt)
  {
  case goal_explore:

    return "explore";

  case goal_think:

    return "think";

  case goal_arrive_at_position:

    return "arrive_at_position";

  case goal_seek_to_position:

    return "seek_to_position";

  case goal_follow_path:

    return "follow_path";

  case goal_traverse_edge:

    return "traverse_edge";

  case goal_move_to_position:

    return "move_to_position";

  case goal_get_health:

    return "get_health";

  case goal_get_shotgun:

    return "get_shotgun";

  case goal_get_railgun:

    return "get_railgun";

  case goal_get_rocket_launcher:

    return "get_rocket_launcher";

  case goal_wander:

    return "wander";

  case goal_negotiate_door:

    return "negotiate_door";

  case goal_attack_target:

    return "attack_target";

  case goal_hunt_target:

    return "hunt_target";

  case goal_strafe:

    return "strafe";

  case goal_adjust_range:

    return "adjust_range";

  case goal_say_phrase:

    return "say_phrase";

// default:前面所有 case 都没命中时走这里——编号不认识,返回警告字符串。
  default:

    return "UNKNOWN GOAL TYPE!";

// switch 语句结束。
  }//end switch
}