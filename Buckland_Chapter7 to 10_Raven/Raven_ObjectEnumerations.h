//==============================================================================================
//【文件说明】Raven_ObjectEnumerations.h —— 地图上各种物体的「类型编号表」
//
//【这个文件是干什么的?】
//  Raven 地图里有墙、机器人、路点、血包、出生点、各种武器、障碍物、滑动门……
//  它们长得各不相同,但都要被系统统一管理。怎么区分?给每类物体发一个数字编号。
//  本文件就是这张编号表:把 0、1、2…… 起成看得懂的名字(type_wall、type_bot…),
//  另外还提供 GetNameOfType:把编号翻译成英文单词,调试时打印出来人能看懂。
//
//【谁在使用这个文件?】(= 哪些文件 #include 了它)
//  Raven_Game.h/.cpp、Raven_Map.cpp(读地图、建物体时要认编号);
//  triggers\Trigger_HealthGiver.cpp、Trigger_WeaponGiver.cpp(自己是哪种物体);
//  goals\ 下各 Goal_Evaluator、Goal_GetItem.cpp、Goal_Think.cpp、Raven_Feature.cpp
//      (目标层判断该去捡血/捡枪时要认物体类型;goals/ 下文件属别的分片,不动)
//      (目标层判断该去捡血/捡枪时要认物体类型)。
//
//【本文件包含了谁?】
//  <string> —— 标准库字符串类 std::string(GetNameOfType 要返回字符串)。
//
//【C++ 小课堂:匿名 enum(匿名枚举)】
//   enum { type_wall, type_bot, ... };
//   enum 是一种「给整数起名字」的语法。花括号里的名字从 0 开始自动编号:
//   type_wall=0、type_bot=1、type_unused=2……依次 +1。
//   省略了 enum 后面的名字(写成 enum { … }; 而不是 enum 名字 { … };),
//   所以叫「匿名枚举」——纯粹只是想拿一批常量,不需要造一个新类型。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护:Raven_ObjectEnumerations.h 的防重复包含标记。
// (包含保护原理详见 constants.h / WestWorld1\Locations.h 的注释。)
//--------------------------------------------------------------------------------
#ifndef RAVEN_OBJECTS_H
#define RAVEN_OBJECTS_H

//--------------------------------------------------------------------------------
// <string>:标准库字符串类。尖括号 <> 表示到标准库目录找。
// GetNameOfType 函数要返回一个字符串,所以先把它的说明书拿进来。
//--------------------------------------------------------------------------------
#include <string>


//--------------------------------------------------------------------------------
// 匿名枚举:下面花括号里每个名字都代表一个整数编号。
//   type_wall=0, type_bot=1, type_unused=2, type_waypoint=3,
//   type_health=4, type_spawn_point=5, type_rail_gun=6,
//   type_rocket_launcher=7, type_shotgun=8, type_blaster=9,
//   type_obstacle=10, type_sliding_door=11, type_door_trigger=12。
// 含义速记:
//   wall=墙,bot=机器人,waypoint=导航路点,health=血包,
//   spawn_point=出生点,rail_gun/rocket_launcher/shotgun/blaster=四种枪,
//   obstacle=障碍物,sliding_door=滑动门,door_trigger=门的触发点。
//--------------------------------------------------------------------------------
enum 
{
  type_wall,
  type_bot,
  type_unused,
  type_waypoint,
  type_health,
  type_spawn_point,
  type_rail_gun,
  type_rocket_launcher,
  type_shotgun,
  type_blaster,
  type_obstacle,
  type_sliding_door,
  type_door_trigger
};



//--------------------------------------------------------------------------------
// GetNameOfType —— 把物体编号翻译成英文名字(调试打印用)。
//   inline  :请求内联,省函数调用开销;
//   std::string :返回一个 C++ 字符串;
//   int w   :传入的物体编号(就是上面枚举里的某个值)。
// 函数体直接写在头文件里(所以才需要 inline)。
//--------------------------------------------------------------------------------
inline std::string GetNameOfType(int w)
{
  std::string s;
// 先造一个空字符串 s,等下根据编号往里面填单词。
  
// switch (w):多分支判断——根据 w 的值跳到对应的 case 分支。
// 每个 case 做的事:把对应英文单词赋给 s,然后 break 跳出 switch。
  switch (w)
  {
  case type_wall:
    
    s = "Wall"; break; 
    
  case type_waypoint:
    
    s = "Waypoint"; break;

  case type_obstacle:
    
    s = "Obstacle"; break;

  case type_health:
    
    s = "Health"; break;

  case type_spawn_point:
    
    s = "Spawn Point"; break;

  case type_rail_gun:
    
    s = "Railgun"; break;

  case type_blaster:
    
    s = "Blaster"; break;

  case type_rocket_launcher:
    
    s =  "rocket_launcher"; break;

  case type_shotgun:
    
    s =  "shotgun"; break;

  case type_unused:
    
    s =  "knife"; break;

  case type_bot:
    
    s =  "bot"; break;

  case type_sliding_door:
    
    s =  "sliding_door"; break;
    
  case type_door_trigger:
    
    s =  "door_trigger"; break;

// default:以上 case 都没匹配上时走这里(编号是个认识不了的怪值)。
  default:

    s = "UNKNOWN OBJECT TYPE"; break;

  }

  return s;
}


#endif