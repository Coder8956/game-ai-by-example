//==============================================================================================
//【文件说明】SoccerPitch.h —— 整个足球赛场的"总调度室"(场地类)
//
//【这个文件是干什么的?】
//  一场比赛里所有东西的"总管家":它持有红队、蓝队两支球队的指针、两座球门、
//  一个足球、一圈边界墙,以及把场地切分出来的若干"区域"(Region)。
//  每个游戏帧,主程序(main.cpp)调用它的 Update() 更新全场、Render() 绘制全场。
//
//【谁在使用这个文件?】(= 哪些文件用 #include 包含了它)
//  main.cpp                  —— 创建球场对象,驱动整个游戏循环;
//  SoccerPitch.cpp           —— 本文件的实现:构造函数里真正 new 出球队/球门/足球;
//  FieldPlayerStates.cpp / GoalKeeperStates.cpp / TeamStates.cpp /
//  SupportSpotCalculator.cpp / Goalkeeper.cpp —— 状态逻辑要访问球场,
//                              拿"球、球门、区域"等数据。
//
//【本文件包含了谁?】
//  <windows.h>     —— Windows 系统头文件(窗口、绘图、消息等);
//  <vector>        —— 标准库"动态数组"容器(C++ 标准库用尖括号 <>);
//  <cassert>       —— 断言工具 assert()(GetRegionFromIndex 里检查下标用);
//  "2D/Wall2D.h"   —— 边界墙类(墙 = 线段 + 法线);
//  "2D/Vector2D.h" —— 2D 向量类;
//  "constants.h"   —— 本章常量(球场宽 700、高 400 等)。
//
//【C++ 小课堂:前置声明(forward declaration)】
//  只写 "class Region;" 这样的"空壳声明",不给类的内容。
//  好处:本文件里这些类只以"指针"形式出现(如 Region* m_pPlayingArea),
//  而"用指针"只需要知道"有这么个类",不需要知道它内部长什么样;
//  这样头文件之间不会互相 #include 成一团乱麻,编译也更快。
//  这些类的完整定义由实现文件(SoccerPitch.cpp 等)去 #include。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护:原理详解见 Goal.h(本工程第 1 个详述处),或 WestWorld1/Locations.h。
//--------------------------------------------------------------------------------
#ifndef SOCCERPITCH_H
#define SOCCERPITCH_H
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786) —— 让 MSVC 编译器"闭嘴",不报 4786 号警告。
//   #pragma  :编译器的"专用指令"(给编译器下命令,不生成任何代码);
//   warning  :跟警告有关的指令;
//   disable  :关闭(禁用)某条警告;
//   4786     :老版 VC6 时代的一个警告编号(与标准库调试符号长度有关),
//             今天早已无意义,保留它纯粹是历史遗留习惯。
//--------------------------------------------------------------------------------
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   SoccerPitch.h
//
//  Desc:   A SoccerPitch is the main game object. It owns instances of
//          two soccer teams, two goals, the playing area, the ball
//          etc. This is the root class for all the game updates and
//          renders etc
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 下面三条尖括号 <>:到编译器的标准/系统目录里找的"系统头文件"。
//   <windows.h> :Windows 系统头文件(窗口、绘图、消息);
//   <vector>    :标准库"动态数组"容器(m_vecWalls、m_Regions 用它装多个对象);
//   <cassert>   :断言宏 assert() 的定义处。
//--------------------------------------------------------------------------------
#include <windows.h>
#include <vector>
#include <cassert>

// ↓↓↓ 原作者说明的翻译:
//   文件名:SoccerPitch.h
//   描述  :SoccerPitch 是游戏的主对象。它拥有两支球队、两座球门、比赛区域、
//          足球等的实例。所有游戏更新(Update)和渲染(Render)的根都在这。
//   作者  :Mat Buckland,2003 年(本书作者)
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 下面三条双引号 "":工程自己的头文件("" 表示先到本工程及附加包含目录里找)。
//   "2D/Wall2D.h"   —— 边界墙类(墙 = 线段 + 法线);
//   "2D/Vector2D.h" —— 2D 向量类;
//   "constants.h"   —— 本章常量(球场宽高、队伍人数等)。
//--------------------------------------------------------------------------------
#include "2D/Wall2D.h"
#include "2D/Vector2D.h"
#include "constants.h"

//--------------------------------------------------------------------------------
// 前置声明:只报"有这么个类",不给内容。本文件里这些类只以指针形式出现,
// "用指针"只需要知道类名即可(原理详见文件头【C++ 小课堂】)。
// 注意下面第 28 行 SoccerTeam 被声明了两次——这是原作者的无害笔误,
// 重复声明同一个类完全合法,我们原样保留不修改。
//--------------------------------------------------------------------------------
class Region;
class Goal;
class SoccerTeam;
class SoccerBall;
class SoccerTeam;
class PlayerBase;


//--------------------------------------------------------------------------------
// class SoccerPitch —— 定义"球场"类。
// 注意:下面的成员变量全部放在 public:(公有)区而不是 private:(私有)区——
// 这是原作者风格:球场是全场的"公共数据仓库",各对象频繁访问,直接公开最省事。
//--------------------------------------------------------------------------------
class SoccerPitch
{ 
public:

  // 足球指针。m_p = 成员变量中的"指针"(pointer)类型。
  SoccerBall*          m_pBall;

  // 红队、蓝队指针(每队 TeamSize=5 名球员,见 constants.h)。
  SoccerTeam*          m_pRedTeam;
  SoccerTeam*          m_pBlueTeam;

  // 红队球门、蓝队球门指针(Goal 类见 Goal.h)。
  Goal*                m_pRedGoal;
  Goal*                m_pBlueGoal;
   
  //container for the boundary walls
  //(原文注释:存放边界墙的容器)

  // 球场四周的"墙"容器。m_vec = 成员变量中的"向量容器"(std::vector)。
  std::vector<Wall2D>  m_vecWalls;

  //defines the dimensions of the playing area
  //(原文注释:定义比赛区域的尺寸)

  // 比赛区域指针(Region 类:一块矩形场地,见 Common\Game\Region.h)。
  Region*              m_pPlayingArea;

  //the playing field is broken up into regions that the team
  //can make use of to implement strategies.
  //(原文注释:比赛场地被切分成若干区域,球队可以利用这些区域实现战术)

  // 区域容器:整块场地切成一小块一小块 Region,球员据此站位/跑位。
  std::vector<Region*> m_Regions;

  //true if a goal keeper has possession
  //(原文注释:true 表示守门员正控着球)

  // 守门员是否控球。m_b = 成员变量中的"布尔"(bool)类型。
  bool                 m_bGoalKeeperHasBall;

  //true if the game is in play. Set to false whenever the players
  //are getting ready for kickoff
  //(原文注释:true 表示比赛进行中;球员准备开球时置为 false)

  // 比赛是否正在进行(false = 开球前的准备状态)。
  bool                 m_bGameOn;

  //set true to pause the motion
  //(原文注释:设为 true 可暂停所有运动)

  // 暂停标志:true = 画面冻结。
  bool                 m_bPaused;

  //local copy of client window dimensions
  //(原文注释:客户端窗口尺寸的本地副本)

  // 窗口客户区的宽(m_cxClient)与高(m_cyClient);c 表示 client(窗口客户区)。
  int                  m_cxClient,
                       m_cyClient;  
  
  //this instantiates the regions the players utilize to  position
  //themselves
  //(原文注释:创建球员用来定位自己的那些区域)

  // 创建区域的函数声明:把场地按宽高切成若干 Region(实现见 SoccerPitch.cpp)。
  void CreateRegions(double width, double height);


public:

  // 构造函数:创建球场对象时自动调用(参数:窗口客户区的宽和高)。
  SoccerPitch(int cxClient, int cyClient);

  // 析构函数:~ 开头,对象销毁时自动调用,负责释放 new 出来的内存。
  ~SoccerPitch();

  // 更新函数:每帧调用一次,驱动全场(球员、球、门将)前进一个时间片。
  void  Update();

  // 渲染函数:每帧调用一次,把全场画到屏幕上(返回 false 表示窗口被关闭)。
  bool  Render();

  // 暂停/继续切换:! 是"取反",m_bPaused = !m_bPaused 即在 true/false 间翻转。
  void  TogglePause(){m_bPaused = !m_bPaused;}
  // 只读查询:当前是否暂停。
  bool  Paused()const{return m_bPaused;}

  // 只读查询:返回窗口客户区宽度。
  int   cxClient()const{return m_cxClient;}
  // 只读查询:返回窗口客户区高度。
  int   cyClient()const{return m_cyClient;}

  // 只读查询:守门员是否控球。
  bool  GoalKeeperHasBall()const{return m_bGoalKeeperHasBall;}
  // 设置:守门员控球标志(参数 b 就是要设成的值)。
  void  SetGoalKeeperHasBall(bool b){m_bGoalKeeperHasBall = b;}

  // 返回比赛区域指针。const Region*const = 对象只读 + 指针本身只读;
  // 末尾的 const 表示这是个只读成员函数。
  const Region*const         PlayingArea()const{return m_pPlayingArea;}
  // 返回墙容器。const std::vector<Wall2D>& = "常引用":只把容器"借"给
  // 调用者看,不复制一份(复制大容器很浪费)。
  const std::vector<Wall2D>& Walls(){return m_vecWalls;}                      
  // 返回足球指针(同样只读)。
  SoccerBall*const           Ball()const{return m_pBall;}

  // 按编号取区域指针。const Region* const:对象与指针都只读。
  const Region* const GetRegionFromIndex(int idx)                                
  {
    // assert(条件):断言——条件为假时程序立刻终止并报错(调试期抓越界下标)。
    // (int)m_Regions.size():把 size_t 类型强制转成 int,再与 int 型 idx 比较。
    assert ( (idx >= 0) && (idx < (int)m_Regions.size()) );

    // 下标 [] 访问容器第 idx 个元素,返回该区域的指针。
    return m_Regions[idx];
  }

  // 查询:比赛是否进行中。
  bool  GameOn()const{return m_bGameOn;}
  // 设置:开球(比赛开始)。
  void  SetGameOn(){m_bGameOn = true;}
  // 设置:停赛(比赛暂停/结束)。
  void  SetGameOff(){m_bGameOn = false;}

};

#endif