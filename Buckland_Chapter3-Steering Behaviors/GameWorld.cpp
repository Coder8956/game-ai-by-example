//==============================================================================================
//【文件说明】GameWorld.cpp —— "游戏世界"类的实现:创建、更新、渲染、交互
//
//【这个文件是干什么的?】
//  1. 构造函数:按 params.ini 的参数创建 300 个机器人(默认开启"群体飞行"),
//     建网格划分器、建一条循环路径;最后一只被改造成"鲨鱼"(大、会徘徊),
//     其余全部开启"逃命"——这就是"大鱼群"演示场景;
//  2. Update():暂停则不做事;否则用 10 帧平滑器算平均帧耗时,再逐个
//     驱动所有机器人 Update();
//  3. Render():画墙壁、障碍物、机器人(含网格信息、路径、FPS 等);
//  4. 交互:SetCrosshair(右键设目标)、HandleKeyPresses(键盘)、
//     HandleMenuItems(菜单:障碍/墙/网格/合力方式/平滑等);
//  5. CreateWalls/CreateObstacles:可选地生成围墙和障碍物。
//
//【与相关文件的关系】
//  GameWorld.h      —— 本文件实现它声明的所有接口;
//  Vehicle.h/.cpp   —— 创建/驱动/渲染机器人;
//  SteeringBehaviors.h —— 给机器人开行为(FlockingOn/EvadeOn/SetPath 等);
//  Obstacle.h/.cpp、2D/Wall2D.h —— 障碍物与墙;
//  ParamLoader.h    —— 宏 Prm:全部数值参数;
//  misc/WindowUtils.h —— ChangeMenuState/CheckMenuItemAppropriately(勾选菜单);
//  misc/Stream_Utility_Functions.h —— ttos(数字转字符串);
//  time/PrecisionTimer.h、misc/Smoother.h —— 帧率测量与平滑;
//  resource.h       —— 菜单资源 ID(ID_OB_OBSTACLES 等,由 VS 生成);
//  main.cpp         —— 创建世界、每帧调 Update/Render、转发键盘/菜单消息。
//
//【C++ 小课堂:#ifdef 条件编译】
//  #define SHOAL 定义宏 → #ifdef SHOAL 成立 → 下面一段代码参与编译。
//  把 #define 那行注释掉,这段"鲨鱼场景"就不编译——这是开关功能区的常用手法。
//==============================================================================================
#include "GameWorld.h"
// 包含车辆类:new Vehicle(...) 需要完整定义。
#include "Vehicle.h"
// 包含窗口常量(习惯性包含,实际未直接用)。
#include "constants.h"
// 包含障碍物类:CreateObstacles 里 new Obstacle 需要完整定义。
#include "Obstacle.h"
// 包含 2D 几何工具(Common 目录):PointInCircle(点在圆内)来自这里。
#include "2d/Geometry.h"
// 包含墙壁类(Common 目录 2D/Wall2D.h):CreateWalls 用。
#include "2d/Wall2D.h"
// 包含坐标变换工具(Common 目录,习惯性包含)。
#include "2d/Transformations.h"
// 包含转向行为控制器:给机器人开行为时要调它的函数。
#include "SteeringBehaviors.h"
// 包含高精度计时器(Common 目录):帧率测量(本文件用 Smoother 平滑帧耗时)。
#include "time/PrecisionTimer.h"
// 包含平滑器模板(Common 目录 misc/Smoother.h):平滑平均帧耗时。
#include "misc/Smoother.h"
// 包含参数加载器:宏 Prm 在这里定义,本文件大量使用。
#include "ParamLoader.h"
// 包含窗口工具(Common 目录 misc/WindowUtils.h):ChangeMenuState 等菜单函数。
#include "misc/WindowUtils.h"
// 包含流工具(Common 目录 misc/Stream_Utility_Functions.h):ttos(转字符串)。
#include "misc/Stream_Utility_Functions.h"


// 包含资源头(resource.h 由 VS 生成):菜单项 ID 常量(如 ID_OB_WALLS)。
#include "resource.h"

// 包含 C++ 标准库 list(习惯性包含)。
#include <list>
// using std::list:之后写 list 即 std::list(省前缀)。
using std::list;


// ↓↓↓ 原文注释翻译:构造函数(ctor)。
//------------------------------- ctor -----------------------------------
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//【构造函数】参数:窗口宽 cx、高 cy。
//  初始化列表逐项"出厂设置":
//    m_cxClient(cx) / m_cyClient(cy) —— 记住窗口尺寸;
//    m_bPaused(false) —— 默认不暂停;
//    m_vCrosshair(居中) —— 准星先放窗口中心;
//    m_bShowWalls 等一堆显示开关 —— 默认全关;
//    m_bShowFPS(true) —— 只有 FPS 默认开;
//    m_pPath(NULL) —— 路径先空着(下面马上建);
//  函数体:建网格划分器 → 建路径 → 批量造机器人 → (可选)造鲨鱼场景。
//--------------------------------------------------------------------------------
GameWorld::GameWorld(int cx, int cy):

            m_cxClient(cx),
            m_cyClient(cy),
            m_bPaused(false),
            m_vCrosshair(Vector2D(cxClient()/2.0, cxClient()/2.0)),
            m_bShowWalls(false),
            m_bShowObstacles(false),
            m_bShowPath(false),
            m_bShowWanderCircle(false),
            m_bShowSteeringForce(false),
            m_bShowFeelers(false),
            m_bShowDetectionBox(false),
            m_bShowFPS(true),
            m_dAvFrameTime(0),
            m_pPath(NULL),
            m_bRenderNeighbors(false),
            m_bViewKeys(false),
            m_bShowCellSpaceInfo(false)
{

  //setup the spatial subdivision class
// 创建网格空间划分器:把 cx×cy 的窗口切成 NumCellsX×NumCellsY 个格子,
// 预分配 NumAgents 个位置。网格用于加速"找邻居"(见 Common 目录)。
  m_pCellSpace = new CellSpacePartition<Vehicle*>((double)cx, (double)cy, Prm.NumCellsX, Prm.NumCellsY, Prm.NumAgents);

// border = 30:路径生成范围的边距(路径点不会贴到窗口边缘)。
  double border = 30;
// 创建循环路径:5 个路标点,范围 [30,30] ~ [cx-30, cy-30],looped=true(循环)。
// 这是默认路径;按 U 键会换成随机数量的新路径。
  m_pPath = new Path(5, border, border, cx-border, cy-border, true); 

// ↓↓↓ 原文注释翻译:创建(机器人)代理(agents)。
  //setup the agents
// for 循环:按参数创建 NumAgents(默认 300)个机器人。
  for (int a=0; a<Prm.NumAgents; ++a)
  {

// ↓↓↓ 原文注释翻译:确定一个随机出生位置。
    //determine a random starting position
// 随机出生点:屏幕中心 ± 随机偏移(RandomClamped() 返回 -1~1 随机数,乘半屏)。
// 结果:出生点散布在整个窗口,但越靠中心概率越高。
    Vector2D SpawnPos = Vector2D(cx/2.0+RandomClamped()*cx/2.0,
                                 cy/2.0+RandomClamped()*cy/2.0);


// new 一个机器人(对象),参数逐个注释在右侧:
//   this(世界指针) / 出生位置 / 随机朝向角(0~2π)/ 初速 0 /
//   质量 / 最大推力 / 最大速度 / 最大转向速率 / 缩放。
// new 返回指针,存进 pVehicle。
    Vehicle* pVehicle = new Vehicle(this,
                                    SpawnPos,                 //initial position
                                    RandFloat()*TwoPi,        //start rotation
                                    Vector2D(0,0),            //velocity
                                    Prm.VehicleMass,          //mass
                                    Prm.MaxSteeringForce,     //max force
                                    Prm.MaxSpeed,             //max velocity
                                    Prm.MaxTurnRatePerSecond, //max turn rate
                                    Prm.VehicleScale);        //scale

// 给这个机器人开启"群体飞行":内聚+对齐+分离+徘徊 四个行为同时激活
// (见 SteeringBehaviors::FlockingOn)。这是每个小鱼的默认行为。
    pVehicle->Steering()->FlockingOn();

// 把机器人指针追加进世界容器 m_Vehicles。
    m_Vehicles.push_back(pVehicle);

// ↓↓↓ 原文注释翻译:把它加进网格空间划分器。
    //add it to the cell subdivision
// 让网格划分器登记这个机器人(网格查询时它才会被找到)。
    m_pCellSpace->AddEntity(pVehicle);
  }


//【条件编译:鲨鱼场景】#define SHOAL 定义宏 → 下面整段参与编译。
// 把 #define 一行注释掉即可关闭此场景(全部机器人保持群体飞行)。
#define SHOAL
#ifdef SHOAL
// 最后一只机器人(下标 NumAgents-1)关掉群体飞行——它不再合群;
  m_Vehicles[Prm.NumAgents-1]->Steering()->FlockingOff();
// 把它放大 10 倍:SetScale(10,10),包围半径同步放大(见基类注释)——大块头;
  m_Vehicles[Prm.NumAgents-1]->SetScale(Vector2D(10, 10));
// 给它只开"徘徊"行为:它变成漫无目的游荡的"大鲨鱼";
  m_Vehicles[Prm.NumAgents-1]->Steering()->WanderOn();
// 把它的最大速度降到 70(比小鱼 150 慢,方便小鱼逃)。
  m_Vehicles[Prm.NumAgents-1]->SetMaxSpeed(70);


// 其余所有小鱼(下标 0 ~ NumAgents-2)……
   for (int i=0; i<Prm.NumAgents-1; ++i)
  {
// ……全部开启"逃命"行为,目标就是那只大鲨鱼:小鱼们会拼命远离它。
// 于是屏幕上出现"大鱼追、群鱼逃"的壮观场面。
    m_Vehicles[i]->Steering()->EvadeOn(m_Vehicles[Prm.NumAgents-1]);

  }
#endif
 
// ↓↓↓ 原文注释翻译:创建任何障碍物或墙壁。
// 下面两行被注释掉了:默认场景不生成障碍物/墙(可通过菜单/按键打开)。
  //create any obstacles or walls
  //CreateObstacles();
// (已注释)生成障碍物。
  //CreateWalls();
}


// ↓↓↓ 原文注释翻译:析构函数(dtor)。
//-------------------------------- dtor ----------------------------------
//------------------------------------------------------------------------
// 析构:逐个 delete 机器人(配对构造时的 new),再删障碍物、网格、路径。
// 顺序:先删孩子们(机器人),再删容器与工具对象,避免悬空指针。
GameWorld::~GameWorld()
{
  for (unsigned int a=0; a<m_Vehicles.size(); ++a)
  {
    delete m_Vehicles[a];
  }

  for (unsigned int ob=0; ob<m_Obstacles.size(); ++ob)
  {
    delete m_Obstacles[ob];
  }

  delete m_pCellSpace;
  
  delete m_pPath;
}


//--------------------------------------------------------------------------------
//【Update:每帧更新整个世界】(由 main.cpp 的消息循环调用)
//  暂停则直接返回;否则先平滑测量帧耗时,再驱动所有机器人各 Update 一次。
//--------------------------------------------------------------------------------
//----------------------------- Update -----------------------------------
//------------------------------------------------------------------------
// Update 函数定义开始。
void GameWorld::Update(double time_elapsed)
{ 
// 暂停开关开着就立刻返回(画面冻结,方便观察)。
  if (m_bPaused) return;

// ↓↓↓ 原文注释翻译:创建一个平滑器来平滑帧率。
  //create a smoother to smooth the framerate
// 采样数:取最近 10 帧。const int:固定。
  const int SampleRate = 10;
// 帧率平滑器:函数内静态变量(原理见 ParamLoader.cpp),只建一次。
// Smoother<double>:对 double 做滑动平均。
  static Smoother<double> FrameRateSmoother(SampleRate, 0.0);

// 把本帧耗时喂进平滑器,取回"平均帧耗时",存进 m_dAvFrameTime 显示 FPS 用。
  m_dAvFrameTime = FrameRateSmoother.Update(time_elapsed);
  

// ↓↓↓ 原文注释翻译:更新所有机器人。
  //update the vehicles
// for 循环:逐个调用机器人的 Update()(它们各自算转向力、移动)。
  for (unsigned int a=0; a<m_Vehicles.size(); ++a)
  {
    m_Vehicles[a]->Update(time_elapsed);
  }
}
  

//--------------------------------------------------------------------------------
//【CreateWalls:创建围墙】
//  在窗口边缘内侧建一圈"斜角围墙",用来演示避墙行为(菜单勾选"墙壁"时调用)。
//  8 个顶点组成一个"切角矩形":四角用 0.2 比例的斜边切掉,更像游戏关卡。
//--------------------------------------------------------------------------------
//--------------------------- CreateWalls --------------------------------
//
//  creates some walls that form an enclosure for the steering agents.
//  used to demonstrate several of the steering behaviors
//------------------------------------------------------------------------
// CreateWalls 函数定义开始。
void GameWorld::CreateWalls()
{
// ↓↓↓ 原文注释翻译:创建墙壁。
  //create the walls  
// 墙与窗口边缘的间距:20 像素。
  double bordersize = 20.0;
// 角部切角比例:0.2(四角斜边长度占边长的比例)。
  double CornerSize = 0.2;
// 竖直方向可用长度 = 窗口高 - 2×边距。
  double vDist = m_cyClient-2*bordersize;
// 水平方向可用长度 = 窗口宽 - 2×边距。
  double hDist = m_cxClient-2*bordersize;
  
// 围墙顶点数:8 个。const int:固定。
  const int NumWallVerts = 8;

// 8 个顶点坐标数组(按顺时针围一圈,四角带斜边):
//  第 0~3 个:上边、右边(依次是左上→上中→右上→右下);
//  第 4~7 个:下边、左边(右下→下中→左下→左上)。
//  每行 Vector2D(横坐标, 纵坐标),乘 CornerSize 实现切角。
  Vector2D walls[NumWallVerts] = {Vector2D(hDist*CornerSize+bordersize, bordersize),
                                   Vector2D(m_cxClient-bordersize-hDist*CornerSize, bordersize),
                                   Vector2D(m_cxClient-bordersize, bordersize+vDist*CornerSize),
                                   Vector2D(m_cxClient-bordersize, m_cyClient-bordersize-vDist*CornerSize),
                                         
                                   Vector2D(m_cxClient-bordersize-hDist*CornerSize, m_cyClient-bordersize),
                                   Vector2D(hDist*CornerSize+bordersize, m_cyClient-bordersize),
                                   Vector2D(bordersize, m_cyClient-bordersize-vDist*CornerSize),
                                   Vector2D(bordersize, bordersize+vDist*CornerSize)};
  
// 依次把"第 w 个顶点 → 第 w+1 个顶点"连成墙段:共 7 段。
// Wall2D(起点, 终点) 构造一个墙段(自带法线)。
  for (int w=0; w<NumWallVerts-1; ++w)
  {
    m_Walls.push_back(Wall2D(walls[w], walls[w+1]));
  }

// 最后一段:最后一个顶点连回第 0 个顶点,围墙闭合。
  m_Walls.push_back(Wall2D(walls[NumWallVerts-1], walls[0]));
}


//--------------------------------------------------------------------------------
//【CreateObstacles:创建障碍物】
//  按参数生成 NumObstacles 个随机大小、随机位置的圆障碍,并保证它们
//  互相不重叠(用 Overlapped 模板函数检测,失败就重试换位置)。
//--------------------------------------------------------------------------------
//--------------------------- CreateObstacles -----------------------------
//
//  Sets up the vector of obstacles with random positions and sizes. Makes
//  sure the obstacles do not overlap
//------------------------------------------------------------------------
// CreateObstacles 函数定义开始。
void GameWorld::CreateObstacles()
{
// ↓↓↓ 原文注释翻译:创建一批随机大小的小圆片(tiddlywinks 是书里对障碍物的昵称)。
    //create a number of randomly sized tiddlywinks
// 外层循环:共生成 NumObstacles(默认 7)个障碍物。
  for (int o=0; o<Prm.NumObstacles; ++o)
  {   
// bOverlapped:标记"当前位置是否与已有障碍重叠",先假设重叠(需要重试)。
    bool bOverlapped = true;

// ↓↓↓ 原文注释翻译:不断生成小圆片,直到找到不重叠的那个。有时可能陷入
//    死循环(障碍物没地方放了),我们检测这种情况并退出。
    //keep creating tiddlywinks until we find one that doesn't overlap
    //any others.Sometimes this can get into an endless loop because the
    //obstacle has nowhere to fit. We test for this case and exit accordingly

// 尝试计数 NumTrys / 最大尝试次数 NumAllowableTrys = 2000:防止死循环。
    int NumTrys = 0; int NumAllowableTrys = 2000;

// 内层 while 循环:只要"还重叠"就一直换位置重试。
    while (bOverlapped)
    {
// 尝试次数 +1(++ 自增)。
      NumTrys++;

// 试了 2000 次还放不下:说明窗口已塞满,放弃本次(return 退出函数)。
      if (NumTrys > NumAllowableTrys) return;
      
// 随机半径:在 MinObstacleRadius~MaxObstacleRadius(ini:10~30)之间取整数。
// (int) 强转:把 double 参数转成 int 供 RandInt 用。
      int radius = RandInt((int)Prm.MinObstacleRadius, (int)Prm.MaxObstacleRadius);

// 障碍物离窗口边缘的最小距离:10 像素;
      const int border                 = 10;
// 障碍物之间的最小间隔:20 像素。
      const int MinGapBetweenObstacles = 20;

// 随机位置:横坐标在 [半径+10, 窗口宽-半径-10] 之间随机,
// 纵坐标在 [半径+10, 窗口高-半径-30-10] 之间随机(底部多留 30 给文字)。
// new Obstacle(x, y, 半径) 建一个障碍物对象。
      Obstacle* ob = new Obstacle(RandInt(radius+border, m_cxClient-radius-border),
                                  RandInt(radius+border, m_cyClient-radius-30-border),
                                  radius);

// 用模板函数 Overlapped 检查它与现有障碍物是否重叠(间隔至少 20 像素)。
// ! 取反:不重叠才算成功。
      if (!Overlapped(ob, m_Obstacles, MinGapBetweenObstacles))
      {
// ↓↓↓ 原文注释翻译:它没有重叠,可以加进去。
        //its not overlapped so we can add it
// 加入障碍物容器。
        m_Obstacles.push_back(ob);

// 标记"不再重叠",跳出 while 循环,继续生成下一个障碍。
        bOverlapped = false;
      }

      else
      {
// 重叠了:释放这个刚 new 出来的对象(不能要,免得泄漏内存),再试一次。
        delete ob;
      }
    }
  }
}


//--------------------------------------------------------------------------------
//【SetCrosshair:设置十字准星】
//  用户右键点击时(main.cpp 的 WM_LBUTTONUP 转来),把准星移到点击处,
//  但要保证点不在任何障碍物内部(否则目标被挡住没意义)。
//--------------------------------------------------------------------------------
//------------------------- Set Crosshair ------------------------------------
//
//  The user can set the position of the crosshair by right clicking the
//  mouse. This method makes sure the click is not inside any enabled
//  Obstacles and sets the position appropriately
//------------------------------------------------------------------------
// SetCrosshair 函数定义开始。POINTS p 是鼠标点击的屏幕坐标。
void GameWorld::SetCrosshair(POINTS p)
{
// 把坐标转成 Vector2D,作为"候选位置"(先不直接改准星)。
// (double)p.x:把 short 类型强转成 double 参与浮点运算。
  Vector2D ProposedPosition((double)p.x, (double)p.y);

// ↓↓↓ 原文注释翻译:确保它不在障碍物内部。
  //make sure it's not inside an obstacle
// 用 ObIt 迭代器遍历所有障碍物(ObIt 是 GameWorld.h 里的 typedef 别名)。
  for (ObIt curOb = m_Obstacles.begin(); curOb != m_Obstacles.end(); ++curOb)
  {
// PointInCircle(圆心, 半径, 点):几何函数,判断点是否在圆内。
// 若点在某个障碍物圆内……
    if (PointInCircle((*curOb)->Pos(), (*curOb)->BRadius(), ProposedPosition))
    {
// ……直接 return:准星位置保持不变(拒绝把目标设在障碍物里)。
      return;
    }

  }
// 位置合法:更新准星的横坐标;
  m_vCrosshair.x = (double)p.x;
// ……和纵坐标。之后 Seek/Arrive 等行为就朝这个新目标使劲。
  m_vCrosshair.y = (double)p.y;
}


//--------------------------------------------------------------------------------
//【HandleKeyPresses:处理按键】(由 main.cpp 的 WM_KEYUP 消息转来)
//  U —— 换一条随机路径;P —— 暂停;O —— 邻居着色;
//  I —— 开关朝向平滑;Y —— 开关障碍物。
//--------------------------------------------------------------------------------
//------------------------- HandleKeyPresses -----------------------------
// HandleKeyPresses 函数定义开始。WPARAM wParam 是按键码。
void GameWorld::HandleKeyPresses(WPARAM wParam)
{

// switch:按按键码(wParam)分支处理。
  switch(wParam)
  {
// 按键 U(键码 'U'):换一条随机路径——
  case 'U':
    {
// 先删掉旧路径对象;
      delete m_pPath;
// 新路径边距 60;
      double border = 60;
// 建新路径:3~7 个随机路标点,循环模式;
      m_pPath = new Path(RandInt(3, 7), border, border, cxClient()-border, cyClient()-border, true); 
// 打开"显示路径"开关(让路径可见);
      m_bShowPath = true; 
// 把新路径同步给每一辆机器人(它们的 FollowPath 行为立刻换线)。
      for (unsigned int i=0; i<m_Vehicles.size(); ++i)
      {
        m_Vehicles[i]->Steering()->SetPath(m_pPath->GetPath());
      }
    }
    break;

// 按键 P:切换暂停(TogglePause)。
    case 'P':
      
      TogglePause(); break;

// 按键 O:切换"给邻居着色"(观察哪些车互为邻居)。
    case 'O':

      ToggleRenderNeighbors(); break;

// 按键 I:给所有机器人切换"朝向平滑"(对比顺滑/生硬两种转向)。
    case 'I':

      {
        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->ToggleSmoothing();
        }

      }

      break;

// 按键 Y:开关障碍物——
    case 'Y':

// 先翻转显示开关 m_bShowObstacles;
       m_bShowObstacles = !m_bShowObstacles;

// 如果要"隐藏障碍物":清空障碍物容器,并关掉所有机器人的避障行为;
        if (!m_bShowObstacles)
        {
          m_Obstacles.clear();

          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_Vehicles[i]->Steering()->ObstacleAvoidanceOff();
          }
        }
// 如果要"显示障碍物":生成一批新障碍物,并给所有机器人打开避障行为。
        else
        {
          CreateObstacles();

          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_Vehicles[i]->Steering()->ObstacleAvoidanceOn();
          }
        }
        break;

// switch 结束(原注释 end switch)。
  }//end switch
}



//--------------------------------------------------------------------------------
//【HandleMenuItems:处理菜单项】(由 main.cpp 的 WM_COMMAND 消息转来)
//  菜单 ID 来自 resource.h:
//   障碍物开关 / 墙壁开关 / 网格划分开关 / 查看网格邻居 /
//   合力方式(加权·优先级·抖动)/ 按键帮助 / FPS / 平滑。
//  菜单项要同步勾选状态(ChangeMenuState / CheckMenuItemAppropriately)。
//--------------------------------------------------------------------------------
//-------------------------- HandleMenuItems -----------------------------
// HandleMenuItems 函数定义开始。参数:菜单 ID + 窗口句柄 hwnd。
void GameWorld::HandleMenuItems(WPARAM wParam, HWND hwnd)
{
  switch(wParam)
  {
// 菜单"障碍物":与 Y 键逻辑相同(翻转开关、清空/生成、开关避障行为、
// 同步菜单勾选)。详见上面 Y 键注释。
    case ID_OB_OBSTACLES:

        m_bShowObstacles = !m_bShowObstacles;

        if (!m_bShowObstacles)
        {
          m_Obstacles.clear();

          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_Vehicles[i]->Steering()->ObstacleAvoidanceOff();
          }

          //uncheck the menu
         ChangeMenuState(hwnd, ID_OB_OBSTACLES, MFS_UNCHECKED);
        }
        else
        {
          CreateObstacles();

          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_Vehicles[i]->Steering()->ObstacleAvoidanceOn();
          }

          //check the menu
          ChangeMenuState(hwnd, ID_OB_OBSTACLES, MFS_CHECKED);
        }

       break;

// 菜单"墙壁":勾选时生成围墙并打开所有机器人的避墙行为;
// 取消勾选时清掉墙并关闭避墙行为。菜单勾选状态同步。
    case ID_OB_WALLS:

      m_bShowWalls = !m_bShowWalls;

      if (m_bShowWalls)
      {
        CreateWalls();

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->WallAvoidanceOn();
        }

        //check the menu
         ChangeMenuState(hwnd, ID_OB_WALLS, MFS_CHECKED);
      }

      else
      {
        m_Walls.clear();

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->WallAvoidanceOff();
        }

        //uncheck the menu
         ChangeMenuState(hwnd, ID_OB_WALLS, MFS_UNCHECKED);
      }

      break;


// 菜单"网格空间划分":给所有机器人切换网格开关;
    case IDR_PARTITIONING:
      {
        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->ToggleSpacePartitioningOnOff();
        }

        //if toggled on, empty the cell space and then re-add all the 
        //vehicles
// 如果刚打开:清空网格,再重新登记所有机器人(让网格数据与当前位置一致);
        if (m_Vehicles[0]->Steering()->isSpacePartitioningOn())
        {
          m_pCellSpace->EmptyCells();
       
          for (unsigned int i=0; i<m_Vehicles.size(); ++i)
          {
            m_pCellSpace->AddEntity(m_Vehicles[i]);
          }

          ChangeMenuState(hwnd, IDR_PARTITIONING, MFS_CHECKED);
        }
        else
        {
          ChangeMenuState(hwnd, IDR_PARTITIONING, MFS_UNCHECKED);
          ChangeMenuState(hwnd, IDM_PARTITION_VIEW_NEIGHBORS, MFS_UNCHECKED);
          m_bShowCellSpaceInfo = false;

        }
      }

      break;

// 菜单"查看网格邻居":显示/隐藏网格邻居信息;若网格没开,先自动打开它。
    case IDM_PARTITION_VIEW_NEIGHBORS:
      {
        m_bShowCellSpaceInfo = !m_bShowCellSpaceInfo;
        
        if (m_bShowCellSpaceInfo)
        {
          ChangeMenuState(hwnd, IDM_PARTITION_VIEW_NEIGHBORS, MFS_CHECKED);

          if (!m_Vehicles[0]->Steering()->isSpacePartitioningOn())
          {
            SendMessage(hwnd, WM_COMMAND, IDR_PARTITIONING, NULL);
          }
        }
        else
        {
          ChangeMenuState(hwnd, IDM_PARTITION_VIEW_NEIGHBORS, MFS_UNCHECKED);
        }
      }
      break;
        

// 菜单"加权平均合力":三选一——把每辆车的合力方式设为 weighted_average,
// 并同步三个菜单项的勾选状态。
    case IDR_WEIGHTED_SUM:
      {
        ChangeMenuState(hwnd, IDR_WEIGHTED_SUM, MFS_CHECKED);
        ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_DITHERED, MFS_UNCHECKED);

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->SetSummingMethod(SteeringBehavior::weighted_average);
        }
      }

      break;

// 菜单"优先级合力":设为 prioritized(默认方式)。
    case IDR_PRIORITIZED:
      {
        ChangeMenuState(hwnd, IDR_WEIGHTED_SUM, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_CHECKED);
        ChangeMenuState(hwnd, IDR_DITHERED, MFS_UNCHECKED);

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->SetSummingMethod(SteeringBehavior::prioritized);
        }
      }

      break;

// 菜单"抖动合力":设为 dithered。
    case IDR_DITHERED:
      {
        ChangeMenuState(hwnd, IDR_WEIGHTED_SUM, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_DITHERED, MFS_CHECKED);

        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->Steering()->SetSummingMethod(SteeringBehavior::dithered);
        }
      }

      break;


// 菜单"按键帮助":切换按键帮助显示,并同步勾选。
      case ID_VIEW_KEYS:
      {
        ToggleViewKeys();

        CheckMenuItemAppropriately(hwnd, ID_VIEW_KEYS, m_bViewKeys);
      }

      break;

// 菜单"FPS":切换 FPS 显示,并同步勾选。
      case ID_VIEW_FPS:
      {
        ToggleShowFPS();

        CheckMenuItemAppropriately(hwnd, ID_VIEW_FPS, RenderFPS());
      }

      break;

// 菜单"平滑":给所有机器人切换朝向平滑,按头车状态同步勾选。
      case ID_MENU_SMOOTHING:
      {
        for (unsigned int i=0; i<m_Vehicles.size(); ++i)
        {
          m_Vehicles[i]->ToggleSmoothing();
        }

        CheckMenuItemAppropriately(hwnd, ID_MENU_SMOOTHING, m_Vehicles[0]->isSmoothingOn());
      }

      break;
      
  }//end switch
}


//--------------------------------------------------------------------------------
//【Render:渲染整个世界】(由 main.cpp 的 WM_PAINT 消息转来)
//  顺序:墙壁 → 障碍物 → 机器人(含网格信息)→ 路径提示 → FPS → 网格。
//  全部通过绘图单例 gdi 画到"后备缓冲"上,再统一翻到屏幕(见 main.cpp)。
//--------------------------------------------------------------------------------
//------------------------------ Render ----------------------------------
//------------------------------------------------------------------------
// Render 函数定义开始。
void GameWorld::Render()
{
// 设置文字背景透明(文字不盖住底色)。
  gdi->TransparentText();

// ↓↓↓ 原文注释翻译:渲染任何墙壁。
  //render any walls
// 画笔换黑色;
  gdi->BlackPen();
// 遍历所有墙壁段……
  for (unsigned int w=0; w<m_Walls.size(); ++w)
  {
// 逐段渲染(true:同时画出墙的法线,便于观察)。
    m_Walls[w].Render(true);  //true flag shows normals
  }

// ↓↓↓ 原文注释翻译:渲染任何障碍物。
  //render any obstacles
  gdi->BlackPen();
  
// 遍历所有障碍物……
  for (unsigned int ob=0; ob<m_Obstacles.size(); ++ob)
  {
// 以圆心、半径画黑圈。
    gdi->Circle(m_Obstacles[ob]->Pos(), m_Obstacles[ob]->BRadius());
  }

// ↓↓↓ 原文注释翻译:渲染(机器人)代理。
  //render the agents
// 遍历所有机器人……
  for (unsigned int a=0; a<m_Vehicles.size(); ++a)
  {
// 让机器人自己渲染(它的 Render 会按状态选颜色、画三角形)。
    m_Vehicles[a]->Render();  
    
// ↓↓↓ 原文注释翻译:渲染网格空间划分的相关信息。
    //render cell partitioning stuff
// 开着"网格信息"且是头车(a==0)时:画视野方框、邻居圈、视野圈。
    if (m_bShowCellSpaceInfo && a==0)
    {
// 空心笔刷(画框不填充);
      gdi->HollowBrush();
// 以头车为中心、ViewDistance 为半边长,构造一个 AABB 包围盒(视野方框);
      InvertedAABBox2D box(m_Vehicles[a]->Pos() - Vector2D(Prm.ViewDistance, Prm.ViewDistance),
                           m_Vehicles[a]->Pos() + Vector2D(Prm.ViewDistance, Prm.ViewDistance));
// 画这个方框;
      box.Render();

// 红笔画邻居:
      gdi->RedPen();
// 按头车位置与视野距离,让网格算出它的邻居(存进网格的遍历器);
      CellSpace()->CalculateNeighbors(m_Vehicles[a]->Pos(), Prm.ViewDistance);
// 遍历网格算出的邻居(网格迭代器 begin/end/next),逐个……
      for (BaseGameEntity* pV = CellSpace()->begin();!CellSpace()->end();pV = CellSpace()->next())
      {
// ……画红圈标出邻居位置;
        gdi->Circle(pV->Pos(), pV->BRadius());
      }
      
// 绿笔画视野圈:
      gdi->GreenPen();
// 以头车为圆心、ViewDistance 为半径画绿圈 = 它的"视力范围"。
      gdi->Circle(m_Vehicles[a]->Pos(), Prm.ViewDistance);
    }
  }  

//【条件编译:十字准星】#ifdef CROSSHAIR 默认未定义,整段不编译。
// 想显示准星,把 #define CROSSHAIR 这行取消注释即可。
//#define CROSSHAIR
#ifdef CROSSHAIR
// ↓↓↓ 原文注释翻译:最后画十字准星。
  //and finally the crosshair
// 红笔;画准星圆圈、横竖线、提示文字(均在被条件编译的代码块内)。
  gdi->RedPen();
  gdi->Circle(m_vCrosshair, 4);
  gdi->Line(m_vCrosshair.x - 8, m_vCrosshair.y, m_vCrosshair.x + 8, m_vCrosshair.y);
  gdi->Line(m_vCrosshair.x, m_vCrosshair.y - 8, m_vCrosshair.x, m_vCrosshair.y + 8);
  gdi->TextAtPos(5, cyClient() - 20, "Click to move crosshair");
#endif


// (原作者注释掉的提示文字,原样保留。)
  //gdi->TextAtPos(cxClient() -120, cyClient() - 20, "Press R to reset");

// 文字颜色设为灰色。
  gdi->TextColor(Cgdi::grey);
// 如果开着"显示路径":在窗口底部提示"按 U 换随机路径",并画出路径。
  if (RenderPath())
  {
     gdi->TextAtPos((int)(cxClient()/2.0f - 80), cyClient() - 20, "Press 'U' for random path");

     m_pPath->Render();
  }

// 如果开着"显示 FPS":把 1÷平均帧耗时 = 帧率(FPS)显示在左下角。
// ttos:数字转字符串工具。
  if (RenderFPS())
  {
    gdi->TextColor(Cgdi::grey);
    gdi->TextAtPos(5, cyClient() - 20, ttos(1.0 / m_dAvFrameTime));
  } 

// 如果开着"网格信息":画出所有网格线(方便观察格子划分)。
  if (m_bShowCellSpaceInfo)
  {
    m_pCellSpace->RenderCells();
  }

}
