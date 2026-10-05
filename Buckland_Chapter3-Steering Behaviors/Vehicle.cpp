//==============================================================================================
//【文件说明】Vehicle.cpp —— "机器人(车辆)"类的实现:转向力 → 运动
//
//【这个文件是干什么的?】
//  把 Vehicle.h 声明的接口全部落地:
//    ① 构造函数:初始化基类运动属性,创建转向控制器和朝向平滑器;
//    ② Update():每帧执行——
//       转向力(Calculate) ÷ 质量 = 加速度 → 更新速度 → 截断限速 → 更新位置
//       → 更新朝向/侧向 → 屏幕环绕 → 更新网格 → 平滑朝向;
//    ③ Render():把三角形小车画到屏幕(支持邻居着色、平滑车头等);
//    ④ InitializeBuffer():填小车造型的三个顶点。
//
//【与相关文件的关系】
//  Vehicle.h            —— 本文件实现它声明的类;
//  SteeringBehaviors.h  —— Calculate() 算转向力;
//  GameWorld.h          —— 世界指针:WrapAround 环绕屏幕用窗口尺寸、
//                           CellSpace() 更新网格;
//  2D/Transformations.h —— WorldTransform:本地坐标 → 世界坐标;
//  misc/Smoother.h      —— 朝向平滑器;
//  misc/Cgdi.h          —— 绘图单例 gdi。
//
//【物理小课堂:牛顿第二定律 F = ma】
//  转向力(所有行为合成)是"力"F;把 F 除以质量 m 得到"加速度"a;
//  加速度乘时间间隔得"速度增量"——这就是 Update 里那三行公式的来源。
//  这是"转向行为"体系最核心的物理循环:力 → 加速度 → 速度 → 位置。
//==============================================================================================
#include "Vehicle.h"
// 包含 2D 矩阵工具(Common 目录 2D/C2DMatrix.h,习惯性包含)。
#include "2d/C2DMatrix.h"
// 包含 2D 几何工具(Common 目录 2D/Geometry.h,习惯性包含)。
#include "2d/Geometry.h"
// 包含转向行为控制器:构造函数里 new SteeringBehavior(this) 需要完整定义。
#include "SteeringBehaviors.h"
// 包含坐标变换工具(Common 目录):WorldTransform(本地→世界)在这里。
#include "2d/Transformations.h"
// 包含世界类:Update 里要用 m_pWorld 的 cxClient()/CellSpace()。
#include "GameWorld.h"
// 包含网格空间划分(Common 目录):CellSpace() 返回的类型定义在这里。
#include "misc/CellSpacePartition.h"
// 包含绘图单例 gdi(misc/cgdi.h,注意小写文件名,原样保留):Render 画图用。
#include "misc/cgdi.h"

// 声明"用 std:: 的 vector/list 时省略 std::"。using 声明只是省打字。
using std::vector;
// 同上:list 也省略 std::。
using std::list;


// ↓↓↓ 原文注释翻译:构造函数(ctor = constructor)。
//----------------------------- ctor -------------------------------------
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//【构造函数】参数:世界指针、位置、旋转角、速度、质量、最大推力、最大速度、
//  最大转向速率、缩放。
//  冒号后分两部分:
//  ① MovingEntity(...) —— 先初始化父类:
//     Vector2D(sin(rotation), -cos(rotation)) —— 用旋转角算出初始朝向向量
//       (三角函数的极坐标:角度→单位向量),负号表示屏幕坐标系 Y 轴向下;
//     scale 同时传给"半径"和"缩放":小车按缩放值画;
//  ② 自己的成员:
//     m_pWorld(world) —— 记住所属世界;
//     m_vSmoothedHeading(Vector2D(0,0)) —— 平滑朝向先置零;
//     m_bSmoothingOn(false) —— 默认不开平滑;
//     m_dTimeElapsed(0.0) —— 帧时间先置 0。
//--------------------------------------------------------------------------------
Vehicle::Vehicle(GameWorld* world,
               Vector2D position,
               double    rotation,
               Vector2D velocity,
               double    mass,
               double    max_force,
               double    max_speed,
               double    max_turn_rate,
               double    scale):    MovingEntity(position,
                                                 scale,
                                                 velocity,
                                                 max_speed,
                                                 Vector2D(sin(rotation),-cos(rotation)),
                                                 mass,
                                                 Vector2D(scale,scale),
                                                 max_turn_rate,
                                                 max_force),

                                       m_pWorld(world),
                                       m_vSmoothedHeading(Vector2D(0,0)),
                                       m_bSmoothingOn(false),
                                       m_dTimeElapsed(0.0)
{  
// 调用 InitializeBuffer():把小车三角形的三个顶点填进缓冲(下面有实现)。
  InitializeBuffer();

  //set up the steering behavior class
// 创建转向行为控制器,并把 this(自己)交给它当"主人"——
// 从这一刻起,每帧 Calculate() 都知道"为谁服务"。
  m_pSteering = new SteeringBehavior(this);    

  //set up the smoother
// 创建朝向平滑器:样本数取 Prm.NumSamplesForSmoothing(ini 里 10),
// 初始值 Vector2D(0.0, 0.0)。
  m_pHeadingSmoother = new Smoother<Vector2D>(Prm.NumSamplesForSmoothing, Vector2D(0.0, 0.0)); 
  
 
}


// ↓↓↓ 原文注释翻译:析构函数(dtor = destructor)。
//---------------------------- dtor -------------------------------------
//-----------------------------------------------------------------------
// 析构:释放 new 出来的两个对象(delete 配对 new)。
// 顺序无所谓,两个都释放即可。
Vehicle::~Vehicle()
{
  delete m_pSteering;
//--------------------------------------------------------------------------------
//【Update:车辆的心跳】每帧由 GameWorld::Update 调用一次。
//  执行顺序(下面的代码逐段对应):
//  记录时间 → 记旧位置 → 算转向力 → 求加速度 → 更新速度 → 限速 →
//  更新位置 → 更新朝向 → 屏幕环绕 → 更新网格 → 平滑朝向。
//--------------------------------------------------------------------------------
  delete m_pHeadingSmoother;
}

//------------------------------ Update ----------------------------------
//
//  Updates the vehicle's position from a series of steering behaviors
//------------------------------------------------------------------------
// Update 函数定义开始。
void Vehicle::Update(double time_elapsed)
{    
  //update the time elapsed
// 记下这一帧的时间间隔:徘徊等行为要按它缩放,保证"时间无关"。
  m_dTimeElapsed = time_elapsed;

  //keep a record of its old position so we can update its cell later
  //in this method
// 记下旧位置:稍后若开着网格划分,要用旧位置把车从旧格子挪到新格子。
// (Pos() 是继承自 BaseGameEntity 的读位置函数。)
  Vector2D OldPos = Pos();


  Vector2D SteeringForce;

// ↓↓↓ 原文注释翻译:计算车辆行为列表里各转向行为合成出的总力。
  //calculate the combined force from each steering behavior in the 
  //vehicle's list
// 问转向控制器要"总转向力"(见 SteeringBehaviors::Calculate)。
  SteeringForce = m_pSteering->Calculate();
    
  //Acceleration = Force/Mass
// 牛顿第二定律:加速度 = 力 ÷ 质量。(质量是 1.0 时,加速度 = 力。)
// Vector2D 支持除法:每个分量除以质量。
  Vector2D acceleration = SteeringForce / m_dMass;

  //update velocity
// 速度 += 加速度 × 时间:速度随时间累积(初速 0,每帧被加速一点)。
// += 读作"自身加上";× 是标量与向量相乘。
  m_vVelocity += acceleration * time_elapsed; 

  //make sure vehicle does not exceed maximum velocity
// 限速:速度向量超过最大速度就把长度截断(Truncate)到 MaxSpeed。
// 模拟"引擎极限"——再大的力,速度也上不去。
  m_vVelocity.Truncate(m_dMaxSpeed);

  //update the position
// 位置 += 速度 × 时间:新位置 = 旧位置 + 这一帧走过的路程。
  m_vPos += m_vVelocity * time_elapsed;

  //update the heading if the vehicle has a non zero velocity
// 只要速度不为零(长度平方 > 极小值,避免除零),就更新朝向:
  if (m_vVelocity.LengthSq() > 0.00000001)
  {    
// 新朝向 = 速度方向的单位向量:车头自动指向前进方向。
// Vec2DNormalize(速度) = 把速度向量归一化成单位长度。
    m_vHeading = Vec2DNormalize(m_vVelocity);

// 侧向 = 新朝向的垂直方向(Perp)。
    m_vSide = m_vHeading.Perp();
  }

// 原代码注释掉的一行:防穿模约束(见 EntityFunctionTemplates.h),
// 本工程用"屏幕环绕"替代它,所以被注释掉。原样保留。
  //EnforceNonPenetrationConstraint(this, World()->Agents());

  //treat the screen as a toroid
// 屏幕环绕(WrapAround):把屏幕当成"甜甜圈",小车飞出左边界就从右边
// 钻回来(上下同理),这样 300 个机器人永远在画面里。
  WrapAround(m_vPos, m_pWorld->cxClient(), m_pWorld->cyClient());

  //update the vehicle's current cell if space partitioning is turned on
// 如果开着网格空间划分:把车辆从"旧格子"更新到"新格子"。
// (网格加速找邻居,见 Common/misc/CellSpacePartition.h。)
  if (Steering()->isSpacePartitioningOn())
  {
    World()->CellSpace()->UpdateEntity(this, OldPos);
  }

// 如果开着平滑:把当前朝向喂给平滑器,取回平均值存进 m_vSmoothedHeading。
// 之后渲染就画"平滑朝向"而不是"瞬时朝向"。
  if (isSmoothingOn())
  {
    m_vSmoothedHeading = m_pHeadingSmoother->Update(Heading());
  }
}


//--------------------------------------------------------------------------------
//【Render:把小车画到屏幕上】
//  流程:按需选画笔颜色 → 把本地顶点变换到世界坐标 → 画闭合多边形 →
//        需要时再画调试辅助(RenderAids)。
//--------------------------------------------------------------------------------
//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
// Render 函数定义开始。
void Vehicle::Render()
{ 
  //a vector to hold the transformed vertices
// 静态容器:保存"变换后"的顶点(static 表示函数级共享,只建一次,
// 每次调用复用,避免反复分配内存)。
  static std::vector<Vector2D>  m_vecVehicleVBTrans;

  //render neighboring vehicles in different colors if requested
// ↓↓↓ 原文注释翻译:如果要求"给邻居着色",就按编号/标记换颜色。
  if (m_pWorld->RenderNeighbors())
  {
// 编号为 0 的"头车"用红笔画;
    if (ID() == 0) gdi->RedPen();
// 被打过标记(在视野内)的邻居用绿笔画;
    else if(IsTagged()) gdi->GreenPen();
// 其他车用蓝笔画。
    else gdi->BluePen();
  }

  else
  {
// 否则(不要求邻居着色):所有车统一蓝笔。
    gdi->BluePen();
  }

// 开着"插到中间"行为时,该车画成红色(便于观察演示)。
  if (Steering()->isInterposeOn())
  {
    gdi->RedPen();
  }

// 开着"躲藏"行为时,该车画成绿色。
  if (Steering()->isHideOn())
  {
    gdi->GreenPen();
  }

// 开着平滑:用"平滑朝向"及其垂直方向建立坐标系,把顶点变换到世界坐标;
// WorldTransform(顶点, 位置, 朝向, 侧向, 缩放) —— 本地坐标 → 世界坐标。
  if (isSmoothingOn())
  { 
    m_vecVehicleVBTrans = WorldTransform(m_vecVehicleVB,
                                         Pos(),
                                         SmoothedHeading(),
                                         SmoothedHeading().Perp(),
                                         Scale());
  }

  else
  {
// 否则:用"瞬时朝向"做同样的变换。
    m_vecVehicleVBTrans = WorldTransform(m_vecVehicleVB,
                                         Pos(),
                                         Heading(),
                                         Side(),
                                         Scale());
  }


// 把变换后的顶点画成闭合多边形(三角形小车)。
// gdi->ClosedShape(顶点列表):按顶点顺序连线并闭合。
  gdi->ClosedShape(m_vecVehicleVBTrans);
 
  //render any visual aids / and or user options
// ↓↓↓ 原文注释翻译:渲染任何可视化辅助/用户选项(按 ViewKeys 开关)。
  if (m_pWorld->ViewKeys())
  {
// 如果用户开着"显示按键帮助"(ViewKeys),就调转向控制器的 RenderAids()
// 画各种调试信息(触须/检测盒/徘徊圆/参数面板等)。
    Steering()->RenderAids();
  }
}


//--------------------------------------------------------------------------------
//【InitializeBuffer:填小车造型的顶点】
//  小车的本地造型是三个顶点组成的三角形:
//   (-1, 0.6) 左上 / (1, 0) 车头 / (-1, -0.6) 左下。
//  渲染时经 WorldTransform 平移到实际位置、按朝向旋转、按缩放放大。
//--------------------------------------------------------------------------------
//----------------------------- InitializeBuffer -----------------------------
//
//  fills the vehicle's shape buffer with its vertices
//-----------------------------------------------------------------------------
// InitializeBuffer 函数定义开始。
void Vehicle::InitializeBuffer()
{
// 小车造型顶点数:3(一个三角形)。const int:固定不变。
  const int NumVehicleVerts = 3;

// 定义数组:三个顶点的本地坐标(以车头为原点,车头朝 +X)。
// 数组初始化用花括号 {元素, 元素, 元素}。
  Vector2D vehicle[NumVehicleVerts] = {Vector2D(-1.0f,0.6f),
                                        Vector2D(1.0f,0.0f),
                                        Vector2D(-1.0f,-0.6f)};

  //setup the vertex buffers and calculate the bounding radius
// 逐个把顶点拷贝进 m_vecVehicleVB(渲染时遍历它)。
// 循环 vtx 从 0 到 2,共 3 次;push_back 追加到 vector 末尾。
  for (int vtx=0; vtx<NumVehicleVerts; ++vtx)
  {
    m_vecVehicleVB.push_back(vehicle[vtx]);
  }
}
