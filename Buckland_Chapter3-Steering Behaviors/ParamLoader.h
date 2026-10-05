//==============================================================================================
//【文件说明】ParamLoader.h —— 游戏参数的"读卡器":把 params.ini 读进内存
//
//【这个文件是干什么的?】
//  第三章演示程序有 300 个机器人在屏幕上乱跑,它们的速度、质量、各种转向
//  行为的"权重"等几十个数值都写在配置文件 params.ini 里(方便玩家/开发者
//  不改代码就调参数)。本文件定义一个 ParamLoader 类:程序启动时自动
//  逐行读出 ini 文件里的数值,存进公开成员变量,供全工程随时取用。
//
//【与相关文件的关系】
//  misc/iniFileLoaderBase.h —— 本类的"父类"(基类):负责打开文件、跳过注释、
//                              按类型读数值。ParamLoader 只负责"按顺序取"。
//  params.ini               —— 被读取的配置文件(和 exe 同目录);
//  misc/utils.h             —— 提供常量 Pi(圆周率)等工具;
//  constants.h              —— 包含它纯属习惯,本文件未直接用到;
//  谁在使用本文件?
//    SteeringBehaviors.h/.cpp —— 通过宏 Prm 读取各种行为的权重与检测距离;
//    Vehicle.cpp / GameWorld.cpp —— 读车辆质量、速度、数量、格子数等。
//
//【C++ 小课堂:单例模式 + #define 宏】
//  本类只允许存在"一个"对象(全局唯一),取它的唯一写法是 ParamLoader::Instance()
//  (实现见 ParamLoader.cpp 的静态局部变量)。为了让代码更好写,作者又定义了
//  宏 Prm,使 Prm.xxx 等价于 (*ParamLoader::Instance()).xxx,详见下方 #define 行。
//==============================================================================================
#ifndef PARAMLOADER_H
#define PARAMLOADER_H
//-----------------------------------------------------------------------------
//
//  Name:   ParamLoader.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to parse a parameter file for the steering behavior project
//-----------------------------------------------------------------------------
// 包含 constants.h(窗口常量)。本文件未直接用到,属于原作者的习惯性包含。
#include "constants.h"
// 包含"ini 文件读取基类":ParamLoader 要继承它,才能调用
// GetNextParameterInt()/GetNextParameterFloat() 逐行读参数。
// 路径写法 misc/xxx.h 表示"附加包含目录 Common 下的 misc 子文件夹",
// 靠工程设置"附加包含目录 ..\..\Common"才能找到(原理见 main.cpp 文件地图)。
#include "misc/iniFileLoaderBase.h"
// 包含 misc/utils.h(Common 目录):下面构造函数里用到的 Pi(圆周率)来自这里。
#include "misc/utils.h"



//【宏 #define —— 编译前的"文本替换"】
//  作用:从此以后,代码里凡是出现 Prm 这个记号,预处理阶段都会被原样替换成
//  (*ParamLoader::Instance())。最外层括号是为了保证运算符优先级正确。
//  于是写 Prm.MaxSpeed 就等价于写 (*ParamLoader::Instance()).MaxSpeed:
//  "取唯一实例(它是一个指针),解引用得到对象,再取它的 MaxSpeed 成员"。
//  Prm 读作 "Params"(参数)。整个工程都用这个简称取参数,打字省事。
#define Prm (*ParamLoader::Instance())


//【类定义开始】class ParamLoader : public iniFileLoaderBase
//   class      —— C++ 关键字,定义一个"类"(模板/图纸);
//   ParamLoader —— 类名:"参数加载器";
//   : public iniFileLoaderBase —— 公有继承 iniFileLoaderBase:自动拥有父类的
//                                 读文件能力,并把父类的公开成员变成自己的。
//   { }       —— 类体,里面写成员。
class ParamLoader : public iniFileLoaderBase
{
private:
  
//【构造函数】创建对象时自动执行。冒号后是"成员初始化列表",这里只有一项:
//   iniFileLoaderBase("params.ini") —— 先调用父类构造函数,把要读的文件名
//     "params.ini"交给它,父类负责打开文件、准备好"读指针"。
// 函数体(下面的花括号)里就是"读参数"的主循环:按 ini 文件里的先后顺序,
// 逐个调用 GetNextParameterInt()(读整数)或 GetNextParameterFloat()(读小数)
// 存进对应成员。ini 里每行一个"名字+数值",读的顺序必须和 ini 里的顺序一致!
    ParamLoader():iniFileLoaderBase("params.ini")
  {
// 读整数:机器人数量(第 1 行 NumAgents = 300)。
// 【小知识】"="是赋值:把右边读出的值装进左边的成员变量。
// 从本行到第 74 行的写法完全一样:一读一行、一行一存,都是"按 ini 顺序"。
    NumAgents               = GetNextParameterInt();
    NumObstacles            = GetNextParameterInt();
    MinObstacleRadius       = GetNextParameterFloat();
    MaxObstacleRadius       = GetNextParameterFloat();

    NumCellsX               = GetNextParameterInt();
    NumCellsY               = GetNextParameterInt();

    NumSamplesForSmoothing  = GetNextParameterInt();

    SteeringForceTweaker    = GetNextParameterFloat();
    MaxSteeringForce        = GetNextParameterFloat() * SteeringForceTweaker;
    MaxSpeed                = GetNextParameterFloat();
    VehicleMass             = GetNextParameterFloat();
    VehicleScale            = GetNextParameterFloat();

    SeparationWeight        = GetNextParameterFloat() * SteeringForceTweaker;
    AlignmentWeight         = GetNextParameterFloat() * SteeringForceTweaker;
    CohesionWeight          = GetNextParameterFloat() * SteeringForceTweaker;
    ObstacleAvoidanceWeight = GetNextParameterFloat() * SteeringForceTweaker;
    WallAvoidanceWeight     = GetNextParameterFloat() * SteeringForceTweaker;
    WanderWeight            = GetNextParameterFloat() * SteeringForceTweaker;
    SeekWeight              = GetNextParameterFloat() * SteeringForceTweaker;
    FleeWeight              = GetNextParameterFloat() * SteeringForceTweaker;
    ArriveWeight            = GetNextParameterFloat() * SteeringForceTweaker;
    PursuitWeight           = GetNextParameterFloat() * SteeringForceTweaker;
    OffsetPursuitWeight     = GetNextParameterFloat() * SteeringForceTweaker;
    InterposeWeight         = GetNextParameterFloat() * SteeringForceTweaker;
    HideWeight              = GetNextParameterFloat() * SteeringForceTweaker;
    EvadeWeight             = GetNextParameterFloat() * SteeringForceTweaker;
    FollowPathWeight        = GetNextParameterFloat() * SteeringForceTweaker;

    ViewDistance            = GetNextParameterFloat();
    MinDetectionBoxLength   = GetNextParameterFloat();
    WallDetectionFeelerLength=GetNextParameterFloat();

    prWallAvoidance         = GetNextParameterFloat();
    prObstacleAvoidance     = GetNextParameterFloat();  
    prSeparation            = GetNextParameterFloat();
    prAlignment             = GetNextParameterFloat();
    prCohesion              = GetNextParameterFloat();
    prWander                = GetNextParameterFloat();
    prSeek                  = GetNextParameterFloat();
    prFlee                  = GetNextParameterFloat();
    prEvade                 = GetNextParameterFloat();
    prHide                  = GetNextParameterFloat();
    prArrive                = GetNextParameterFloat();

// 最大转向速率:每秒最多能转多少弧度。这里直接用常量 Pi(π≈3.14159)
// ——即每秒最多转半个圆。注意:它没有从 ini 读,而是写死的固定值。
    MaxTurnRatePerSecond    = Pi;
  }

public:
// public: —— 从这行开始,下面的成员"对外公开",任何代码都能访问。
// (上面 private: 区里的构造函数是私有的——单例模式第 1 件:禁止别人 new。)

  static ParamLoader* Instance();
// 静态成员函数声明:全世界获取本类唯一实例的唯一入口(实现见 ParamLoader.cpp)。
// static 表示它不属于某个对象,直接用"类名::Instance()"调用。

  int	NumAgents;
//--------------------------------------------------------------------------------
// 【公开数据成员:ini 读出来的全部游戏参数,供全工程读取】
//  下面几十个成员的名字与 params.ini 里的参数一一对应。它们全是"公有的"
//  简单变量,没有封装成 getter/setter——作者为了方便直接暴露,读者不要学
//  这种风格,这里仅作演示。使用示例:SteeringBehaviors 里写 Prm.MaxSpeed。
//--------------------------------------------------------------------------------
//  机器人(agent)数量。
  int	NumObstacles;
  double MinObstacleRadius;
  double MaxObstacleRadius;

// ↓↓↓ 原文注释翻译:空间划分(网格)用的水平格子数。
//  空间划分:把屏幕分成 N×N 的网格,加速"找邻居"(详见 misc/CellSpacePartition.h)。
  //number of horizontal cells used for spatial partitioning
  int   NumCellsX;
// ↓↓↓ 原文注释翻译:空间划分(网格)用的垂直格子数。
  //number of vertical cells used for spatial partitioning
  int   NumCellsY;

// ↓↓↓ 原文注释翻译:平滑器用多少个样本做平均(去抖)。
//  车辆朝向的"平滑器"用它决定取最近几帧做平均(见 Vehicle.cpp)。
  //how many samples the smoother will use to average a value
  int   NumSamplesForSmoothing;

// ↓↓↓ 原文注释翻译:
//    用来微调"合成后的转向力"(只改 MaxSteeringForce 是不行的!
//    这个调节因子会影响下面所有转向行为的权重系数)。
//  它的作用:MaxSteeringForce 实际值 = ini 的 SteeringForce(2.0) × 200 = 400。
  //used to tweak the combined steering force (simply altering the MaxSteeringForce
  //will NOT work!This tweaker affects all the steering force multipliers
  //too).
  double SteeringForceTweaker;

  double MaxSteeringForce;
  double MaxSpeed;
  double VehicleMass;

  double VehicleScale;
  double MaxTurnRatePerSecond;

  double SeparationWeight;
  double AlignmentWeight ;
  double CohesionWeight  ;
  double ObstacleAvoidanceWeight;
  double WallAvoidanceWeight;
  double WanderWeight    ;
  double SeekWeight      ;
  double FleeWeight      ;
  double ArriveWeight    ;
  double PursuitWeight   ;
  double OffsetPursuitWeight;
  double InterposeWeight ;
  double HideWeight      ;
  double EvadeWeight     ;
  double FollowPathWeight;

// ↓↓↓ 原文注释翻译:邻居必须多近,agent 才"感知"到它(把它算进自己的
//    邻里范围)。——即"视力范围":超过这个距离的机器人不算邻居。
  //how close a neighbour must be before an agent perceives it (considers it
  //to be within its neighborhood)
  double ViewDistance;

// ↓↓↓ 原文注释翻译:用于障碍物躲避(检测盒的长度基准)。
  //used in obstacle avoidance
  double MinDetectionBoxLength;

// ↓↓↓ 原文注释翻译:用于墙壁躲避(触须 feeler 的长度)。
  //used in wall avoidance
  double WallDetectionFeelerLength;

// ↓↓↓ 原文注释翻译:这些概率表示在使用"优先级抖动"(dithered)合成法时,
//    各转向行为被选中的概率(0~1)。
  //these are the probabilities that a steering behavior will be used
  //when the prioritized dither calculate method is used
  double prWallAvoidance;
  double prObstacleAvoidance;
  double prSeparation;
  double prAlignment;
  double prCohesion;
  double prWander;
  double prSeek;
  double prFlee;
  double prEvade;
  double prHide;
  double prArrive;
  
};





#endif