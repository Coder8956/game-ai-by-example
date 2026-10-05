//==============================================================================================
//【文件说明】GameWorld.h —— "游戏世界"类声明:整个演示舞台的管理者
//
//【这个文件是干什么的?】
//  定义 GameWorld:一个"大容器",装着第三章世界里的一切:
//    ① 所有机器人 m_Vehicles(300 个);
//    ② 所有障碍物 m_Obstacles、所有墙壁 m_Walls;
//    ③ 一条路径 m_pPath(机器人沿它飞);
//    ④ 网格空间划分器 m_pCellSpace(加速"找邻居");
//    ⑤ 十字准星 m_vCrosshair(鼠标右键设目标点)、暂停开关、
//       各种"显示什么"的开关(墙/障碍/路径/检测盒/触须/FPS……)。
//  它也是"更新/渲染"的根:main.cpp 每帧调 world->Update() 和 world->Render()。
//
//【与相关文件的关系】
//  Vehicle.h/.cpp    —— m_Vehicles 的元素类型;Update() 里逐个驱动;
//  Obstacle.h/.cpp   —— 障碍物(生成/清除逻辑在 .cpp);
//  EntityFunctionTemplates.h —— 模板函数 EnforceNonPenetrationConstraint /
//                       TagNeighbors,由下面几个行内函数包装调用;
//  2d/Vector2D.h     —— 位置向量;
//  time/PrecisionTimer.h —— (被 GameWorld.cpp 用)帧率平滑;
//  misc/CellSpacePartition.h —— 网格空间划分器类型;
//  Path.h            —— 路径对象;
//  main.cpp          —— 创建本对象、每帧调 Update/Render、把键盘/菜单消息
//                       转交给 HandleKeyPresses/HandleMenuItems。
//
//【C++ 小课堂:typedef 别名】
//  typedef std::vector<BaseGameEntity*>::iterator ObIt; —— 给"实体指针容器的
//  迭代器类型"起个短名字 ObIt(Obstacle Iterator 障碍迭代器),写代码省字。
//  ObIt 就是 std::vector<BaseGameEntity*>::iterator 的别名,类型完全一样。
//==============================================================================================
#ifndef GameWorld_H
#define GameWorld_H
// #pragma warning (disable:4786)—— 屏蔽老编译器警告(见 SteeringBehaviors.h)。
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   GameWorld.h
//
//  Desc:   All the environment data and methods for the Steering
//          Behavior projects. This class is the root of the project's
//          update and render calls (excluding main of course)
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// 包含 Windows API 头文件:WPARAM/HWND/POINTS 等类型来自这里。
#include <windows.h>
// 包含 C++ 标准库 vector:下面三个成员容器都用它。
#include <vector>

// 包含 2D 向量工具(Common 目录):Vector2D 类型。
#include "2d/Vector2D.h"
// 包含高精度计时器(Common 目录 time/PrecisionTimer.h,习惯性包含)。
#include "time/PrecisionTimer.h"
// 包含网格空间划分器(Common 目录 misc/CellSpacePartition.h):
// m_pCellSpace 的类型 CellSpacePartition<Vehicle*> 定义在这里。
#include "misc/CellSpacePartition.h"
// 包含实体基类:TagNeighbors 等模板函数的操作对象继承自它。
#include "BaseGameEntity.h"
// 包含实体函数模板(防穿模、打标记等),下面几个行内函数要调用。
#include "EntityFunctionTemplates.h"
// 包含车辆类(注意原文件是小写 vehicle.h,保持原样):m_Vehicles 的类型。
#include "vehicle.h"


//【前置声明】class Obstacle; class Wall2D; class Path;
// 只报名字:下面成员只用指针/引用,不需要完整定义(完整定义由 .cpp 包含)。
class Obstacle;
class Wall2D;
class Path;


// typedef 别名:ObIt = 实体指针容器的迭代器(见文件头部小课堂)。
typedef std::vector<BaseGameEntity*>::iterator  ObIt;


//【类定义开始】class GameWorld —— 游戏世界。
// (注意原文件用的类名是 GameWorld,头文件保护宏是 GameWorld_H,保持原样。)
class GameWorld
{ 
// private: —— 私有区:数据成员与私有工具函数。
private:

// ↓↓↓ 原文注释翻译:所有移动实体(机器人)的容器。
  //a container of all the moving entities
// m_Vehicles —— 装所有 Vehicle* 的动态数组。
  std::vector<Vehicle*>         m_Vehicles;

// ↓↓↓ 原文注释翻译:(存放)任何障碍物。
  //any obstacles
// m_Obstacles —— 装所有障碍物指针(BaseGameEntity* 作基类指针用)。
  std::vector<BaseGameEntity*>  m_Obstacles;

// ↓↓↓ 原文注释翻译:环境中任何墙壁的容器。
  //container containing any walls in the environment
// m_Walls —— 墙(值存放,不是指针)。
  std::vector<Wall2D>           m_Walls;

// 网格空间划分器指针:把屏幕切格子,加速"找邻居"(菜单可开关)。
  CellSpacePartition<Vehicle*>* m_pCellSpace;

// ↓↓↓ 原文注释翻译:我们为机器人创建的、供其跟随的任何路径。
  //any path we may create for the vehicles to follow
// m_pPath —— 路径对象(按 U 键会换一条新的随机路径)。
  Path*                         m_pPath;

// ↓↓↓ 原文注释翻译:设为 true 时暂停运动。
  //set true to pause the motion
// m_bPaused —— 暂停开关(P 键切换)。
  bool                          m_bPaused;

// ↓↓↓ 原文注释翻译:客户端窗口尺寸的本地副本。
  //local copy of client window dimensions
// m_cxClient —— 窗口宽度(像素);
  int                           m_cxClient,
// m_cyClient —— 窗口高度(像素)。
                                m_cyClient;
// ↓↓↓ 原文注释翻译:十字准星的位置。
  //the position of the crosshair
// m_vCrosshair —— 十字准星(鼠标右键设它,作为 Seek 等行为的目标点)。
  Vector2D                      m_vCrosshair;

// ↓↓↓ 原文注释翻译:记录平均帧耗时。
  //keeps track of the average FPS
// m_dAvFrameTime —— 平均每帧秒数(取最近 10 帧平均,算 FPS 用)。
  double                         m_dAvFrameTime;


// ↓↓↓ 原文注释翻译:控制辅助显示/障碍等开关的标志。
  //flags to turn aids and obstacles etc on/off
// 显示墙壁?
  bool  m_bShowWalls;
// 显示障碍物?
  bool  m_bShowObstacles;
// 显示路径?
  bool  m_bShowPath;
// 显示检测盒?
  bool  m_bShowDetectionBox;
// 显示徘徊圆?
  bool  m_bShowWanderCircle;
// 显示触须?
  bool  m_bShowFeelers;
// 显示转向力?
  bool  m_bShowSteeringForce;
// 显示 FPS?
  bool  m_bShowFPS;
// 给邻居着色?
  bool  m_bRenderNeighbors;
// 显示按键帮助?
  bool  m_bViewKeys;
// 显示网格信息?
  bool  m_bShowCellSpaceInfo;


// 私有函数声明:创建障碍物(实现见 .cpp)。
  void CreateObstacles();

// 私有函数声明:创建墙壁(实现见 .cpp)。
  void CreateWalls();

  

// public: —— 公开区:对外接口。
public:
  
// 构造函数:参数是窗口宽、高(实现见 .cpp)。
  GameWorld(int cx, int cy);

// 析构函数:释放所有机器人/障碍物/网格/路径(实现见 .cpp)。
  ~GameWorld();

// Update:每帧更新整个世界(实现见 .cpp)。
  void  Update(double time_elapsed);

// Render:每帧渲染整个世界(实现见 .cpp)。
  void  Render();


// 防穿模包装:调模板函数 EnforceNonPenetrationConstraint(v, m_Vehicles)
// 把 v 与所有机器人之间的重叠推开(本工程注释掉了调用,但接口保留)。
  void  NonPenetrationContraint(Vehicle* v){EnforceNonPenetrationConstraint(v, m_Vehicles);}

// 给"视野范围内的机器人"打标记:包装 TagNeighbors(pVehicle, m_Vehicles, range)。
// 供转向行为的群体算法(分离/对齐/内聚)先圈邻居。
  void  TagVehiclesWithinViewRange(BaseGameEntity* pVehicle, double range)
  {
    TagNeighbors(pVehicle, m_Vehicles, range);
  }

// 给"视野范围内的障碍物"打标记:包装 TagNeighbors(pVehicle, m_Obstacles, range)。
// 供避障行为先圈出该检查的障碍。
  void  TagObstaclesWithinViewRange(BaseGameEntity* pVehicle, double range)
  {
    TagNeighbors(pVehicle, m_Obstacles, range);
  }

// 读墙壁容器(只读引用,避免拷贝)。
  const std::vector<Wall2D>&          Walls(){return m_Walls;}                          
// 读网格空间划分器指针(转向行为与渲染用)。
  CellSpacePartition<Vehicle*>*       CellSpace(){return m_pCellSpace;}
// 读障碍物容器(只读引用)。
  const std::vector<BaseGameEntity*>& Obstacles()const{return m_Obstacles;}
// 读机器人容器(只读引用)。
  const std::vector<Vehicle*>&        Agents(){return m_Vehicles;}


// ↓↓↓ 原文注释翻译:处理 WM_COMMAND 消息。
// 键盘消息(WPARAM 是消息参数)处理入口,实现见 .cpp。
  //handle WM_COMMAND messages
// HandleKeyPresses:处理按键(实现见 .cpp)。
  void        HandleKeyPresses(WPARAM wParam);
// HandleMenuItems:处理菜单项(实现见 .cpp)。
  void        HandleMenuItems(WPARAM wParam, HWND hwnd);
  
// 切换暂停开关。
  void        TogglePause(){m_bPaused = !m_bPaused;}
// 查询是否暂停。
  bool        Paused()const{return m_bPaused;}

// 读十字准星位置。
  Vector2D    Crosshair()const{return m_vCrosshair;}
// 声明:设置十字准星(鼠标右键点击,实现见 .cpp)。
  void        SetCrosshair(POINTS p);
// 设置十字准星(直接给 Vector2D 的版本)。
  void        SetCrosshair(Vector2D v){m_vCrosshair=v;}

// 读窗口宽度。
  int   cxClient()const{return m_cxClient;}
// 读窗口高度。
  int   cyClient()const{return m_cyClient;}
 
// 要渲染墙壁吗?
  bool  RenderWalls()const{return m_bShowWalls;}
// 要渲染障碍物吗?
  bool  RenderObstacles()const{return m_bShowObstacles;}
// 要渲染路径吗?
  bool  RenderPath()const{return m_bShowPath;}
// 要渲染检测盒吗?
  bool  RenderDetectionBox()const{return m_bShowDetectionBox;}
// 要渲染徘徊圆吗?
  bool  RenderWanderCircle()const{return m_bShowWanderCircle;}
// 要渲染触须吗?
  bool  RenderFeelers()const{return m_bShowFeelers;}
// 要渲染转向力吗?
  bool  RenderSteeringForce()const{return m_bShowSteeringForce;}

// 要显示 FPS 吗?
  bool  RenderFPS()const{return m_bShowFPS;}
// 切换 FPS 显示开关。
  void  ToggleShowFPS(){m_bShowFPS = !m_bShowFPS;}
  
// 切换"给邻居着色"开关。
  void  ToggleRenderNeighbors(){m_bRenderNeighbors = !m_bRenderNeighbors;}
// 查询邻居着色是否开启。
  bool  RenderNeighbors()const{return m_bRenderNeighbors;}
  
// 切换"按键帮助"显示。
  void  ToggleViewKeys(){m_bViewKeys = !m_bViewKeys;}
// 查询按键帮助是否显示。
  bool  ViewKeys()const{return m_bViewKeys;}

};



#endif