//==============================================================================================
//【文件说明】ParamLoader.h —— 全局参数单例(从 Params.ini 读入全场比赛参数)
//
//【这个文件是干什么的?】
//  球多大、球员跑多快、传球力度、射门精准度、各种距离阈值……本章所有"可调旋钮"
//  都写在文本文件 Params.ini 里。本文件定义 ParamLoader 类:它在程序启动时一次性
//  打开 Params.ini,按顺序读出每个数字,存进自己的成员变量,供全程序随时取用。
//  它是"单例(singleton)"——全程序只有这一份,谁都能访问,且不会重复读文件。
//
//【谁在使用这个文件?】
//  main.cpp / SoccerPitch.cpp / PlayerBase.cpp / FieldPlayer.cpp / Goalkeeper.cpp
//  / SoccerBall.cpp / SoccerTeam.cpp / SteeringBehaviors.cpp / SupportSpotCalculator.cpp
//  / FieldPlayerStates.cpp / GoalKeeperStates.cpp —— 几乎全场都通过宏 Prm.XXX 读参数;
//  ParamLoader.cpp —— 单例 Instance() 的实现就在那里。
//
//【本文件包含了谁?】
//  <fstream>/<string>/<cassert> —— C++ 标准库(文件流、字符串、断言);
//  "constants.h" —— 本章窗口尺寸/队伍人数常量;
//  "misc/iniFileLoaderBase.h" —— Common\misc 里的 INI 读取基类(提供 GetNextParameter* 函数)。
//
//【C++ 小课堂:单例 Singleton 回顾】
//  原理与 WestWorld1/MinerOwnedStates.h 的状态单例相同:私有构造 + 公有静态 Instance()。
//  区别在于:这里用宏 #define Prm (*ParamLoader::Instance()) 让代码里写 Prm.GoalWidth
//  就等于 (*ParamLoader::Instance()).GoalWidth,省去每次写一大串调用。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护(include guard):原理详见 Goal.h。
//--------------------------------------------------------------------------------
#ifndef PARAMLOADER
#define PARAMLOADER
//--------------------------------------------------------------------------------
// #pragma warning(disable:4800):让 MSVC 关闭 4800 号警告("bool 转 int 性能/转换提示")。
// #pragma 是给编译器下的专用指令,不生成代码。原作者嫌它吵,直接关掉。
//--------------------------------------------------------------------------------
#pragma warning(disable:4800)
//------------------------------------------------------------------------
//
//Name:  ParamLoader.h
//
//Desc:  singleton class to handle the loading of default parameter
//       values from an initialization file: 'params.ini'
//
//Author: Mat Buckland 2003 (fup@ai-junkie.com)
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:ParamLoader.h
//   描述  :单例类,负责从初始化文件 'params.ini' 加载默认参数值。
//   作者  :Mat Buckland,2003 年(本书作者)
//
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 下面是本文件需要的头文件:
//   <fstream>  —— 文件输入输出流(读取 Params.ini 用);
//   <string>   —— 标准库字符串;
//   <cassert>  —— 断言宏 assert();
//   "constants.h"        —— 本章窗口/队伍人数常量;
//   "misc/iniFileLoaderBase.h" —— INI 读取基类(Common\misc,提供 GetNextParameter* 系列函数)。
//--------------------------------------------------------------------------------
#include <fstream>
#include <string>
#include <cassert>


#include "constants.h"
#include "misc/iniFileLoaderBase.h"


//--------------------------------------------------------------------------------
// #define Prm (*ParamLoader::Instance()) —— 全程序最常用的一个宏:
//   凡是写 Prm.XXX 的地方,预处理时一律替换为 (*ParamLoader::Instance()).XXX;
//   即"拿到那唯一一份参数对象的指针,解引用(*)后取它的成员 XXX"。
//   例如 SoccerBall.cpp 里的 Prm.Friction 就是读球场地面摩擦这个参数。
//--------------------------------------------------------------------------------
#define Prm (*ParamLoader::Instance())

//--------------------------------------------------------------------------------
// class ParamLoader : public iniFileLoaderBase —— 定义参数加载类,
// 并以"公有继承"方式继承 INI 读取基类:本类白得基类的 GetNextParameter* 函数,
// 自己只管把读到的值存进对应的成员变量。
class ParamLoader : public iniFileLoaderBase
{
private:

//--------------------------------------------------------------------------------
// 私有构造函数:在 private 区,外界无法 new 第二份(单例第 1 件)。
//   :iniFileLoaderBase("Params.ini") —— 冒号初始化列表:先调用父类构造,
//   把文件名 "Params.ini" 交给父类去打开;随后按文件里的行顺序,
//   逐行 GetNextParameterDouble()/Int()/Bool() 读出并存入对应成员。
//   注意:读取顺序必须与 Params.ini 里参数出现的先后完全一致。
//--------------------------------------------------------------------------------
  ParamLoader():iniFileLoaderBase("Params.ini")
  {    
       
    GoalWidth                   = GetNextParameterDouble(); 
    
    NumSupportSpotsX            = GetNextParameterInt();    
    NumSupportSpotsY            = GetNextParameterInt();  
    
    Spot_PassSafeScore                     = GetNextParameterDouble();
    Spot_CanScoreFromPositionScore         = GetNextParameterDouble();
    Spot_DistFromControllingPlayerScore     = GetNextParameterDouble();
    Spot_ClosenessToSupportingPlayerScore  = GetNextParameterDouble();
    Spot_AheadOfAttackerScore              = GetNextParameterDouble();

    SupportSpotUpdateFreq       = GetNextParameterDouble(); 
    
    ChancePlayerAttemptsPotShot = GetNextParameterDouble();
    ChanceOfUsingArriveTypeReceiveBehavior = GetNextParameterDouble();
    
    BallSize                    = GetNextParameterDouble();    
    BallMass                    = GetNextParameterDouble();    
    Friction                    = GetNextParameterDouble(); 
    
    KeeperInBallRange           = GetNextParameterDouble();    
    PlayerInTargetRange         = GetNextParameterDouble(); 
    PlayerKickingDistance       = GetNextParameterDouble(); 
    PlayerKickFrequency         = GetNextParameterDouble();


    PlayerMass                  = GetNextParameterDouble(); 
    PlayerMaxForce              = GetNextParameterDouble();    
    PlayerMaxSpeedWithBall      = GetNextParameterDouble();   
    PlayerMaxSpeedWithoutBall   = GetNextParameterDouble();   
    PlayerMaxTurnRate           = GetNextParameterDouble();   
    PlayerScale                 = GetNextParameterDouble();      
    PlayerComfortZone           = GetNextParameterDouble();  
    PlayerKickingAccuracy       = GetNextParameterDouble();

    NumAttemptsToFindValidStrike = GetNextParameterInt();


    
    MaxDribbleForce             = GetNextParameterDouble();    
    MaxShootingForce            = GetNextParameterDouble();    
    MaxPassingForce             = GetNextParameterDouble();  
    
    WithinRangeOfHome           = GetNextParameterDouble();    
    WithinRangeOfSupportSpot    = GetNextParameterDouble();    
    
    MinPassDist                 = GetNextParameterDouble();
    GoalkeeperMinPassDist       = GetNextParameterDouble();
    
    GoalKeeperTendingDistance   = GetNextParameterDouble();    
    GoalKeeperInterceptRange    = GetNextParameterDouble();
    BallWithinReceivingRange    = GetNextParameterDouble();
    
    bStates                     = GetNextParameterBool();    
    bIDs                        = GetNextParameterBool(); 
    bSupportSpots               = GetNextParameterBool();     
    bRegions                    = GetNextParameterBool();
    bShowControllingTeam        = GetNextParameterBool();
    bViewTargets                = GetNextParameterBool();
    bHighlightIfThreatened      = GetNextParameterBool();

    FrameRate                   = GetNextParameterInt();

    SeparationCoefficient       = GetNextParameterDouble(); 
    ViewDistance                = GetNextParameterDouble(); 
    bNonPenetrationConstraint   = GetNextParameterBool(); 


    BallWithinReceivingRangeSq = BallWithinReceivingRange * BallWithinReceivingRange;
    KeeperInBallRangeSq      = KeeperInBallRange * KeeperInBallRange;
    PlayerInTargetRangeSq    = PlayerInTargetRange * PlayerInTargetRange;   
    PlayerKickingDistance   += BallSize;
    PlayerKickingDistanceSq  = PlayerKickingDistance * PlayerKickingDistance;
    PlayerComfortZoneSq      = PlayerComfortZone * PlayerComfortZone;
    GoalKeeperInterceptRangeSq     = GoalKeeperInterceptRange * GoalKeeperInterceptRange;
    WithinRangeOfSupportSpotSq = WithinRangeOfSupportSpot * WithinRangeOfSupportSpot;
  }
  
public:

//--------------------------------------------------------------------------------
// public 区开始。下面全是对外(只读为主)开放的参数字段,以及单例入口:
//   static ParamLoader* Instance() —— 公有静态函数,返回唯一实例指针(实现见 .cpp)。
//--------------------------------------------------------------------------------
  static ParamLoader* Instance();

  double GoalWidth;

  int   NumSupportSpotsX;
  int   NumSupportSpotsY;

  //these values tweak the various rules used to calculate the support spots
//(原文注释:下面这组数值用来微调"接应点(support spot)"评分的各项规则)
  double Spot_PassSafeScore;
  double Spot_CanScoreFromPositionScore;
  double Spot_DistFromControllingPlayerScore;
  double Spot_ClosenessToSupportingPlayerScore;
  double Spot_AheadOfAttackerScore;  
  
  double SupportSpotUpdateFreq ;

  double ChancePlayerAttemptsPotShot; 
  double ChanceOfUsingArriveTypeReceiveBehavior;

  double BallSize;
  double BallMass;
  double Friction;

  double KeeperInBallRange;
  double KeeperInBallRangeSq;

  double PlayerInTargetRange;
  double PlayerInTargetRangeSq;
  
  double PlayerMass;
  
  //max steering force
//(原文注释:最大转向力)下面是球员运动相关的一组参数(质量/最大力/速度/转向/尺寸等)。
  double PlayerMaxForce; 
  double PlayerMaxSpeedWithBall;
  double PlayerMaxSpeedWithoutBall;
  double PlayerMaxTurnRate;
  double PlayerScale;
  double PlayerComfortZone;

  double PlayerKickingDistance;
  double PlayerKickingDistanceSq;

  double PlayerKickFrequency; 

  double  MaxDribbleForce;
  double  MaxShootingForce;
  double  MaxPassingForce;

  double  PlayerComfortZoneSq;

  //in the range zero to 1.0. adjusts the amount of noise added to a kick,
  //the lower the value the worse the players get
//(原文注释:取值范围 0~1.0,调节射门时叠加的随机误差;值越低球员脚法越差)
  double  PlayerKickingAccuracy;

  //the number of times the SoccerTeam::CanShoot method attempts to find
  //a valid shot
//(原文注释:SoccerTeam::CanShoot 方法寻找一次有效射门时尝试的次数)
  int    NumAttemptsToFindValidStrike;

  //the distance away from the center of its home region a player
  //must be to be considered at home
//(原文注释:球员距离自己"老家区域"中心多近才算"已回到家")
  double WithinRangeOfHome;

  //how close a player must get to a sweet spot before he can change state
//(原文注释:球员要离接应甜区多近,才允许切换状态)
  double WithinRangeOfSupportSpot;
  double WithinRangeOfSupportSpotSq;
 
  
  //the minimum distance a receiving player must be from the passing player
//(原文注释:接球者与传球者之间必须保持的最小距离)
  double   MinPassDist;
  double   GoalkeeperMinPassDist;

  //this is the distance the keeper puts between the back of the net
  //and the ball when using the interpose steering behavior
//(原文注释:守门员使用"拦截(interpose)"转向行为时,在球门线后与球之间留出的距离)
  double  GoalKeeperTendingDistance;

  //when the ball becomes within this distance of the goalkeeper he
  //changes state to intercept the ball
//(原文注释:当球进入守门员这个距离内,他就切换状态去拦截球)
  double  GoalKeeperInterceptRange;
  double  GoalKeeperInterceptRangeSq;

  //how close the ball must be to a receiver before he starts chasing it
//(原文注释:球离接球者多近时,接球者才开始追球)
  double  BallWithinReceivingRange;
  double  BallWithinReceivingRangeSq;


  //these values control what debug info you can see
//--------------------------------------------------------------------------------
// 下面 b 开头的 bool 布尔量都是"调试显示开关":true=画面上画出对应辅助信息;
// 由菜单(main.cpp)和 Params.ini 控制,不影响比赛逻辑。
  bool  bStates;
  bool  bIDs;
  bool  bSupportSpots;
  bool  bRegions;
  bool  bShowControllingTeam;
  bool  bViewTargets;
  bool  bHighlightIfThreatened;

  int FrameRate;

  
  double SeparationCoefficient;

  //how close a neighbour must be before an agent perceives it
//(原文注释:邻居距离多近时,一个智能体才"感知"到它的存在)
  double ViewDistance;

  //zero this to turn the constraint off
//(原文注释:把它设为 0 即可关闭这个约束——防止球员互相穿透的约束开关)
  bool bNonPenetrationConstraint;

};

#endif