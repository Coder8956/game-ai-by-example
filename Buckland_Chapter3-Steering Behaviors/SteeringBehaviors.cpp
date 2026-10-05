//==============================================================================================
//【文件说明】SteeringBehaviors.cpp —— 转向行为控制器的"算法库"实现
//
//【这个文件是干什么的?】
//  把 SteeringBehaviors.h 里声明的全部接口落地:
//    ① 构造函数:绑定主人 Vehicle,按 ini 读权重,准备徘徊目标点和路径;
//    ② Calculate() 家族:按三种方式之一,把所有"开着的"行为的力合成
//       一股总转向力;
//    ③ 行为算法库:Seek/Flee/Arrive/Pursuit/Evade/Wander/避障/避墙/
//       分离/对齐/内聚(及网格版)/Interpose/Hide/FollowPath/OffsetPursuit;
//    ④ 工具:AccumulateForce(限量累加)、CreateFeelers(造触须)、
//       GetHidingPosition(算藏身点)、RenderAids(画调试信息)。
//
//【核心思想:转向力 = 行为 → 权重 → 合成 → 截断】
//  每个行为函数都返回一个"转向力向量"(想让车往哪使劲、使多大劲);
//  乘上该行为的权重(从 ini 读)后,由 Calculate 家族合成为总力;
//  总力不能超过 MaxForce(引擎极限),超了就截断(Truncate)。
//  这辆车的 Update(见 Vehicle.cpp)再把力转成加速度驱动运动。
//
//【与相关文件的关系】
//  SteeringBehaviors.h —— 本文件实现它的全部声明;
//  Vehicle.h/.cpp    —— 通过 m_pVehicle 读位置/朝向/速度,并拿世界数据;
//  GameWorld.h/.cpp  —— World() 提供障碍物/墙/机器人列表;
//  2D/Wall2D.h、2D/Transformations.h、2D/geometry.h —— 墙、坐标变换、几何;
//  misc/utils.h      —— RandFloat/RandomClamped/Clamp/TwoPi/HalfPi 等;
//  misc/Cgdi.h       —— 绘图单例 gdi(RenderAids 画图);
//  misc/CellSpacePartition.h —— 网格邻居查询(网格版行为);
//  EntityFunctionTemplates.h —— (习惯性包含)。
//
//【C++ 小课堂:坐标空间】
//  世界坐标 = 屏幕绝对坐标;本地坐标 = 以车辆自身为原点的坐标。
//  PointToLocalSpace(世界点) → 本地;PointToWorldSpace/VectorToWorldSpace(本地) → 世界。
//  避障算法先把障碍物坐标变换到"车头系",算起来才简单(见避障注释)。
//==============================================================================================
#include "SteeringBehaviors.h"
// 包含车辆类:要通过 m_pVehicle 访问机器人的各项数据。
#include "Vehicle.h"
// 包含墙壁类(Common 目录 2D/Wall2D.h):WallAvoidance 的参数类型。
#include "2d/Wall2D.h"
// 包含坐标变换工具(Common 目录):PointToLocalSpace/PointToWorldSpace/
// Vec2DRotateAroundOrigin 等。
#include "2d/Transformations.h"
// 包含通用工具(Common 目录 misc/utils.h):随机数、常量、Clamp 等。
#include "misc/utils.h"
// 包含绘图单例 gdi(misc/Cgdi.h):RenderAids 画调试图形。
#include "misc/Cgdi.h"
// 包含世界类:World() 取障碍物/墙/邻居。
#include "GameWorld.h"
// 包含 2D 几何工具(Common 目录):TwoCirclesOverlapped、LineIntersection2D、
// DistToLineSegment 等。
#include "2d/geometry.h"
// 包含实体基类(习惯性包含)。
#include "BaseGameEntity.h"
// 包含网格空间划分器:网格版行为(CellSpace()->begin/next 遍历邻居)。
#include "misc/CellSpacePartition.h"
// 包含流工具(Common 目录):ttos(数字转字符串,RenderAids 显示数值用)。
#include "misc/Stream_Utility_Functions.h"
// 包含实体函数模板(习惯性包含)。
#include "EntityFunctionTemplates.h"

// 包含断言工具 <cassert>:assert(条件) 调试期拦截逻辑错误。
#include <cassert>


// using std::string / using std::vector:省去 std:: 前缀。
using std::string;
// vector 也省略前缀(下面大量使用)。
using std::vector;


// ↓↓↓ 原文注释翻译:构造函数(ctor)。
//------------------------- ctor -----------------------------------------
//
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//【构造函数】绑定主人 Vehicle*,并按 ini 参数初始化所有权重与检测值。
//  初始化列表很长,但模式统一:"成员(取自 Prm.xxx 或常量)"。要点:
//    m_pVehicle(agent) —— 记住"我服务谁";
//    m_iFlags(0) —— 初始一个行为都不开;
//    m_dDBoxLength / 各权重 / ViewDistance / 触须长度 —— 全从 Prm 读;
//    m_Feelers(3) —— 触须容器先占 3 个元素(3 根触须);
//    m_Deceleration(normal) —— 默认中等减速档;
//    m_dWanderDistance/Jitter/Radius —— 徘徊三参数用文件头部常量;
//    m_dWaypointSeekDistSq —— 路标点切换阈值(平方);
//    m_bCellSpaceOn(false) —— 默认不用网格;
//    m_SummingMethod(prioritized) —— 默认优先级合成法。
//--------------------------------------------------------------------------------
SteeringBehavior::SteeringBehavior(Vehicle* agent):
                                  
             
             m_pVehicle(agent),
             m_iFlags(0),
             m_dDBoxLength(Prm.MinDetectionBoxLength),
             m_dWeightCohesion(Prm.CohesionWeight),
             m_dWeightAlignment(Prm.AlignmentWeight),
             m_dWeightSeparation(Prm.SeparationWeight),
             m_dWeightObstacleAvoidance(Prm.ObstacleAvoidanceWeight),
             m_dWeightWander(Prm.WanderWeight),
             m_dWeightWallAvoidance(Prm.WallAvoidanceWeight),
             m_dViewDistance(Prm.ViewDistance),
             m_dWallDetectionFeelerLength(Prm.WallDetectionFeelerLength),
             m_Feelers(3),
             m_Deceleration(normal),
             m_pTargetAgent1(NULL),
             m_pTargetAgent2(NULL),
             m_dWanderDistance(WanderDist),
             m_dWanderJitter(WanderJitterPerSec),
             m_dWanderRadius(WanderRad),
             m_dWaypointSeekDistSq(WaypointSeekDist*WaypointSeekDist),
             m_dWeightSeek(Prm.SeekWeight),
             m_dWeightFlee(Prm.FleeWeight),
             m_dWeightArrive(Prm.ArriveWeight),
             m_dWeightPursuit(Prm.PursuitWeight),
             m_dWeightOffsetPursuit(Prm.OffsetPursuitWeight),
             m_dWeightInterpose(Prm.InterposeWeight),
             m_dWeightHide(Prm.HideWeight),
             m_dWeightEvade(Prm.EvadeWeight),
             m_dWeightFollowPath(Prm.FollowPathWeight),
             m_bCellSpaceOn(false),
             m_SummingMethod(prioritized)


{
// ↓↓↓ 原文注释翻译:(下面两段是)徘徊行为要用的东西。
  //stuff for the wander behavior
// 随机取一个角度 theta(0~2π):RandFloat() 返回 0~1 随机数,乘 2π。
  double theta = RandFloat() * TwoPi;

  //create a vector to a target position on the wander circle
// 把徘徊目标点放在"徘徊圆"上的一个随机位置:
// (半径×cosθ, 半径×sinθ) —— 极坐标转直角坐标,圆上均匀分布。
  m_vWanderTarget = Vector2D(m_dWanderRadius * cos(theta),
                              m_dWanderRadius * sin(theta));

// ↓↓↓ 原文注释翻译:创建一条路径。
  //create a Path
// new 一个路径对象(沿路径行为用);
  m_pPath = new Path();
// 把路径设为"循环"模式(走完一圈绕回来)。
  m_pPath->LoopOn();

}

//--------------------------------------------------------------------------------
//【析构函数】释放构造函数里 new 的路径对象(delete 配对 new)。
//--------------------------------------------------------------------------------
//---------------------------------dtor ----------------------------------
SteeringBehavior::~SteeringBehavior(){delete m_pPath;}


// ↓↓↓ 原文注释翻译:下面是"合力计算方法"部分(CALCULATE METHODS)。
// (////// 是原作者的分节装饰线,原样保留。)
/////////////////////////////////////////////////////////////////////////////// CALCULATE METHODS 


//--------------------------------------------------------------------------------
//【Calculate:对外总入口】按 m_SummingMethod 指定的方式,合成总转向力。
//  步骤:清零旧力 → (若开群体行为)先圈邻居 → 按三种方式之一算力。
//--------------------------------------------------------------------------------
//----------------------- Calculate --------------------------------------
//
//  calculates the accumulated steering force according to the method set
//  in m_SummingMethod
//------------------------------------------------------------------------
// Calculate 函数定义开始。
Vector2D SteeringBehavior::Calculate()
{ 
  //reset the steering force
// 清零旧力:上一帧的力作废,重新从头累加。Zero() 把向量各分量设 0。
  m_vSteeringForce.Zero();

// ↓↓↓ 原文注释翻译:如果开了网格划分,就用网格算邻居;否则用标准打标记法。
  //use space partitioning to calculate the neighbours of this vehicle
  //if switched on. If not, use the standard tagging system
// 网格划分没开:
  if (!isSpacePartitioningOn())
  {
    //tag neighbors if any of the following 3 group behaviors are switched on
// 只要分离/对齐/内聚任一开着,先给"视野内"的机器人打标记(圈邻居)。
// On(separation) 等:查询行为开关(见 .h 的 On 注释)。
// || 读作"或者"。
    if (On(separation) || On(allignment) || On(cohesion))
    {
// 调世界帮忙:把 m_pVehicle 视野半径内的机器人打上标记,
// 之后 Separation/Alignment/Cohesion 只处理打了钩的(效率高)。
      m_pVehicle->World()->TagVehiclesWithinViewRange(m_pVehicle, m_dViewDistance);
    }
  }
// 网格划分开着:
  else
  {
    //calculate neighbours in cell-space if any of the following 3 group
    //behaviors are switched on
// 任一群体行为开着时,让网格算出 m_pVehicle 的邻居(存进网格迭代器),
// 网格版行为(CohesionPlus 等)再遍历它。
    if (On(separation) || On(allignment) || On(cohesion))
    {
      m_pVehicle->World()->CellSpace()->CalculateNeighbors(m_pVehicle->Pos(), m_dViewDistance);
    }
  }

// switch:按合力方式分支。
  switch (m_SummingMethod)
  {
// 方式一:加权平均——全部行为的力×权重相加,最后截断(见下方函数);
  case weighted_average:
    
    m_vSteeringForce = CalculateWeightedSum(); break;

// 方式二:优先级——按重要程度逐个累加,力用尽即停(默认方式);
  case prioritized:

    m_vSteeringForce = CalculatePrioritized(); break;

// 方式三:抖动——每帧按概率随机挑行为(见下方函数);
  case dithered:
    
    m_vSteeringForce = CalculateDithered();break;

// 其他(理论上不会走到):返回零向量兜底。
  default:m_vSteeringForce = Vector2D(0,0); 

  }//end switch
// switch 结束。

// 返回合成好的总转向力。
  return m_vSteeringForce;
}

//--------------------------------------------------------------------------------
//【ForwardComponent:前进分量】总转向力在"车头方向"上的投影。
//  用途:调试面板可分别显示"往前/往侧"的力。
//--------------------------------------------------------------------------------
//------------------------- ForwardComponent -----------------------------
//
//  returns the forward oomponent of the steering force
//------------------------------------------------------------------------
// 点积:朝向(单位向量)· 转向力 = 力在朝向上的分量(带正负号)。
// m_pVehicle->Heading():读车头单位向量;.Dot() 是向量点积。
double SteeringBehavior::ForwardComponent()
{
  return m_pVehicle->Heading().Dot(m_vSteeringForce);
}

//--------------------------------------------------------------------------------
//【SideComponent:侧向分量】总转向力在"车身侧向"上的投影。
//--------------------------------------------------------------------------------
//--------------------------- SideComponent ------------------------------
//  returns the side component of the steering force
//------------------------------------------------------------------------
// 点积:侧向(单位向量)· 转向力 = 横向分量。
double SteeringBehavior::SideComponent()
{
  return m_pVehicle->Side().Dot(m_vSteeringForce);
}


//--------------------------------------------------------------------------------
//【AccumulateForce:限量累加力】
//  把 ForceToAdd 加进 RunningTot,但保证总量不超过车辆最大推力 MaxForce:
//  若剩余额度不够,只加"能装下的"部分(方向不变、长度截到剩余额度)。
//  返回值:false 表示"额度已用尽"(调用方应停止继续加力)。
//--------------------------------------------------------------------------------
//--------------------- AccumulateForce ----------------------------------
//
//  This function calculates how much of its max steering force the 
//  vehicle has left to apply and then applies that amount of the
//  force to add.
//------------------------------------------------------------------------
// AccumulateForce 函数定义开始。参数:RunningTot(已累加的总力,按引用
// 修改)+ ForceToAdd(想加进来的力)。
bool SteeringBehavior::AccumulateForce(Vector2D &RunningTot,
                                       Vector2D ForceToAdd)
{
  
// ↓↓↓ 原文注释翻译:计算车辆已经用掉了多少转向力。
  //calculate how much steering force the vehicle has used so far
// 已用掉的力 = 当前总量的长度(Length() 开平方得大小)。
  double MagnitudeSoFar = RunningTot.Length();

// ↓↓↓ 原文注释翻译:计算还剩多少转向力可用。
  //calculate how much steering force remains to be used by this vehicle
// 剩余额度 = 最大推力 - 已用掉的量。
  double MagnitudeRemaining = m_pVehicle->MaxForce() - MagnitudeSoFar;

  //return false if there is no more force left to use
// ↓↓↓ 原文注释翻译:如果没有剩余额度,返回 false(别再加了)。
  if (MagnitudeRemaining <= 0.0) return false;
// 剩余 ≤ 0:一点额度都没有,返回 false 拒绝本次累加。

// ↓↓↓ 原文注释翻译:计算想加的力的大小。
  //calculate the magnitude of the force we want to add
// 待加入力的大小。
  double MagnitudeToAdd = ForceToAdd.Length();
  
// ↓↓↓ 原文注释翻译:如果"待加入 + 已用"不超过最大推力,就整个加进去;
//    否则只加不超限的那部分。
  //if the magnitude of the sum of ForceToAdd and the running total
  //does not exceed the maximum force available to this vehicle, just
  //add together. Otherwise add as much of the ForceToAdd vector is
  //possible without going over the max.
// 放得下:直接加(+= 自身加上)。
  if (MagnitudeToAdd < MagnitudeRemaining)
  {
    RunningTot += ForceToAdd;
  }

  else
  {
    //add it to the steering force
// 放不下:先归一化 ForceToAdd(只留方向),再乘以剩余额度——
// 方向照旧、长度正好塞满剩余空间。
    RunningTot += (Vec2DNormalize(ForceToAdd) * MagnitudeRemaining); 
  }

// 成功加入,返回 true。
  return true;
}



//--------------------------------------------------------------------------------
//【CalculatePrioritized:优先级合成法】(默认方式)
//  按"重要性"从高到低逐个处理开着的行为:避墙→避障→逃命→逃离→
//  群体三件套→追击→到达→徘徊→拦截→编队→插中→躲藏→沿路径。
//  每加一个力就检查额度:一旦力用尽(AccumulateForce 返回 false),
//  立刻返回——后面的低优先级行为"轮不到"。
//--------------------------------------------------------------------------------
//---------------------- CalculatePrioritized ----------------------------
//
//  this method calls each active steering behavior in order of priority
//  and acumulates their forces until the max steering force magnitude
//  is reached, at which time the function returns the steering force 
//  accumulated to that  point
//------------------------------------------------------------------------
// CalculatePrioritized 函数定义开始。
Vector2D SteeringBehavior::CalculatePrioritized()
{       
// 局部变量 force:临时存放单个行为的力。
  Vector2D force;
  
// 开着避墙(最高优先级:命比什么都重要):
   if (On(wall_avoidance))
  {
// 算避墙力 × 权重(见 WallAvoidance);
    force = WallAvoidance(m_pVehicle->World()->Walls()) *
            m_dWeightWallAvoidance;

// 累加;额度用尽就立刻返回当前总力。
    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }
   
// 开着避障:算避障力 × 权重,累加,额度用尽即返回。
  if (On(obstacle_avoidance))
  {
    force = ObstacleAvoidance(m_pVehicle->World()->Obstacles()) * 
            m_dWeightObstacleAvoidance;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

// 开着逃命:断言目标已指定(没有目标就中断报错——开发期抓 bug);
// 算逃命力 × 权重,累加。
  if (On(evade))
  {
    assert(m_pTargetAgent1 && "Evade target not assigned");
    
    force = Evade(m_pTargetAgent1) * m_dWeightEvade;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  
// 开着逃离:朝"十字准星"(鼠标目标)反方向逃 × 权重。
  if (On(flee))
  {
    force = Flee(m_pVehicle->World()->Crosshair()) * m_dWeightFlee;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }


 
// ↓↓↓ 原文注释翻译:下面这三个可以组合成群体飞行(徘徊也适合加进来)。
  //these next three can be combined for flocking behavior (wander is
  //also a good behavior to add into this mix)
// 网格划分没开时,依次处理群体三件套:分离 → 对齐 → 内聚(各 × 权重累加)。
  if (!isSpacePartitioningOn())
  {
    if (On(separation))
    {
      force = Separation(m_pVehicle->World()->Agents()) * m_dWeightSeparation;

      if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
    }

    if (On(allignment))
    {
      force = Alignment(m_pVehicle->World()->Agents()) * m_dWeightAlignment;

      if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
    }

    if (On(cohesion))
    {
      force = Cohesion(m_pVehicle->World()->Agents()) * m_dWeightCohesion;

      if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
    }
  }

// 网格划分开着时,用网格版:分离Plus → 对齐Plus → 内聚Plus。
  else
  {

    if (On(separation))
    {
      force = SeparationPlus(m_pVehicle->World()->Agents()) * m_dWeightSeparation;

      if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
    }

    if (On(allignment))
    {
      force = AlignmentPlus(m_pVehicle->World()->Agents()) * m_dWeightAlignment;

      if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
    }

    if (On(cohesion))
    {
      force = CohesionPlus(m_pVehicle->World()->Agents()) * m_dWeightCohesion;

      if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
    }
  }

// 开着追击:朝准星全速飞 × 权重。
  if (On(seek))
  {
    force = Seek(m_pVehicle->World()->Crosshair()) * m_dWeightSeek;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }


// 开着到达:朝准星减速停 × 权重。
  if (On(arrive))
  {
    force = Arrive(m_pVehicle->World()->Crosshair(), m_Deceleration) * m_dWeightArrive;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

// 开着徘徊:随机游荡力 × 权重。
  if (On(wander))
  {
    force = Wander() * m_dWeightWander;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

// 开着拦截:断言拦截目标已指定;算拦截力 × 权重。
  if (On(pursuit))
  {
    assert(m_pTargetAgent1 && "pursuit target not assigned");

    force = Pursuit(m_pTargetAgent1) * m_dWeightPursuit;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

// 开着编队追击:断言队长与偏移已指定;算编队力(不带权重,原书如此)。
  if (On(offset_pursuit))
  {
    assert (m_pTargetAgent1 && "pursuit target not assigned");
    assert (!m_vOffset.isZero() && "No offset assigned");

    force = OffsetPursuit(m_pTargetAgent1, m_vOffset);

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

// 开着插到中间:断言两个目标都在;算插中力 × 权重。
  if (On(interpose))
  {
    assert (m_pTargetAgent1 && m_pTargetAgent2 && "Interpose agents not assigned");

    force = Interpose(m_pTargetAgent1, m_pTargetAgent2) * m_dWeightInterpose;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

// 开着躲藏:断言猎人已指定;算躲藏力 × 权重。
  if (On(hide))
  {
    assert(m_pTargetAgent1 && "Hide target not assigned");

    force = Hide(m_pTargetAgent1, m_pVehicle->World()->Obstacles()) * m_dWeightHide;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }


// 开着沿路径:算沿路径力 × 权重。
  if (On(follow_path))
  {
    force = FollowPath() * m_dWeightFollowPath;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

// 全部加完(或提前用完额度),返回总转向力。
  return m_vSteeringForce;
}


//--------------------------------------------------------------------------------
//【CalculateWeightedSum:加权平均合成法】
//  把所有开着的行为的力 × 权重全部相加(不设优先级、不中途退出),
//  最后用 Truncate 把总长度截到最大推力以内。
//  缺点:所有行为"抢同一份额度",极端情况下低权重行为可能互相抵消。
//--------------------------------------------------------------------------------
//---------------------- CalculateWeightedSum ----------------------------
//
//  this simply sums up all the active behaviors X their weights and 
//  truncates the result to the max available steering force before 
//  returning
//------------------------------------------------------------------------
// CalculateWeightedSum 函数定义开始。
Vector2D SteeringBehavior::CalculateWeightedSum()
{        
// 开着避墙:总力 += 避墙力 × 权重(以下每个分支同一模式,不再逐行展开)。
  if (On(wall_avoidance))
  {
    m_vSteeringForce += WallAvoidance(m_pVehicle->World()->Walls()) *
                         m_dWeightWallAvoidance;
  }
   
// 开着避障:相加。
  if (On(obstacle_avoidance))
  {
    m_vSteeringForce += ObstacleAvoidance(m_pVehicle->World()->Obstacles()) * 
            m_dWeightObstacleAvoidance;
  }

// 开着逃命:断言目标,相加。
  if (On(evade))
  {
    assert(m_pTargetAgent1 && "Evade target not assigned");
    
    m_vSteeringForce += Evade(m_pTargetAgent1) * m_dWeightEvade;
  }


// ↓↓↓ 原文注释翻译:这三个可组合成群体飞行(徘徊也适合加进来)。
  //these next three can be combined for flocking behavior (wander is
  //also a good behavior to add into this mix)
// 网格没开:分离/对齐/内聚 依次相加;
  if (!isSpacePartitioningOn())
  {
    if (On(separation))
    {
      m_vSteeringForce += Separation(m_pVehicle->World()->Agents()) * m_dWeightSeparation;
    }

    if (On(allignment))
    {
      m_vSteeringForce += Alignment(m_pVehicle->World()->Agents()) * m_dWeightAlignment;
    }

    if (On(cohesion))
    {
      m_vSteeringForce += Cohesion(m_pVehicle->World()->Agents()) * m_dWeightCohesion;
    }
  }
// 网格开着:网格版三件套相加。
  else
  {
    if (On(separation))
    {
      m_vSteeringForce += SeparationPlus(m_pVehicle->World()->Agents()) * m_dWeightSeparation;
    }

    if (On(allignment))
    {
      m_vSteeringForce += AlignmentPlus(m_pVehicle->World()->Agents()) * m_dWeightAlignment;
    }

    if (On(cohesion))
    {
      m_vSteeringForce += CohesionPlus(m_pVehicle->World()->Agents()) * m_dWeightCohesion;
    }
  }


// 徘徊力相加;
  if (On(wander))
  {
    m_vSteeringForce += Wander() * m_dWeightWander;
  }

// 追击力相加;
  if (On(seek))
  {
    m_vSteeringForce += Seek(m_pVehicle->World()->Crosshair()) * m_dWeightSeek;
  }

// 逃离力相加;
  if (On(flee))
  {
    m_vSteeringForce += Flee(m_pVehicle->World()->Crosshair()) * m_dWeightFlee;
  }

// 到达力相加;
  if (On(arrive))
  {
    m_vSteeringForce += Arrive(m_pVehicle->World()->Crosshair(), m_Deceleration) * m_dWeightArrive;
  }

// 拦截力相加(断言目标);
  if (On(pursuit))
  {
    assert(m_pTargetAgent1 && "pursuit target not assigned");

    m_vSteeringForce += Pursuit(m_pTargetAgent1) * m_dWeightPursuit;
  }

// 编队力相加(断言队长与偏移);
  if (On(offset_pursuit))
  {
    assert (m_pTargetAgent1 && "pursuit target not assigned");
    assert (!m_vOffset.isZero() && "No offset assigned");

    m_vSteeringForce += OffsetPursuit(m_pTargetAgent1, m_vOffset) * m_dWeightOffsetPursuit;
  }

// 插中力相加(断言两目标);
  if (On(interpose))
  {
    assert (m_pTargetAgent1 && m_pTargetAgent2 && "Interpose agents not assigned");

    m_vSteeringForce += Interpose(m_pTargetAgent1, m_pTargetAgent2) * m_dWeightInterpose;
  }

// 躲藏力相加(断言猎人);
  if (On(hide))
  {
    assert(m_pTargetAgent1 && "Hide target not assigned");

    m_vSteeringForce += Hide(m_pTargetAgent1, m_pVehicle->World()->Obstacles()) * m_dWeightHide;
  }

// 沿路径力相加。
  if (On(follow_path))
  {
    m_vSteeringForce += FollowPath() * m_dWeightFollowPath;
  }

// 总力截断:长度超过 MaxForce 就砍到 MaxForce(方向不变)——
// 引擎极限,无论开多少行为,总推力就这么多。
  m_vSteeringForce.Truncate(m_pVehicle->MaxForce());
 
// 返回总力。
  return m_vSteeringForce;
}


//--------------------------------------------------------------------------------
//【CalculateDithered:抖动合成法】
//  每帧给每个开着的行为一次"中奖机会":按 Prm.prXxx 概率决定它本轮
//  是否出力;第一个出力且力非零的行为胜出,直接返回(其余行为这帧不参与)。
//  效果:整体行为"概率性混合"——长期看每个行为出力的比例≈它的概率。
//  注意:原作者只实现了部分行为(够演示思路即可)。
//--------------------------------------------------------------------------------
//---------------------- CalculateDithered ----------------------------
//
//  this method sums up the active behaviors by assigning a probabilty
//  of being calculated to each behavior. It then tests the first priority
//  to see if it should be calcukated this simulation-step. If so, it
//  calculates the steering force resulting from this behavior. If it is
//  more than zero it returns the force. If zero, or if the behavior is
//  skipped it continues onto the next priority, and so on.
//
//  NOTE: Not all of the behaviors have been implemented in this method,
//        just a few, so you get the general idea
//------------------------------------------------------------------------
// CalculateDithered 函数定义开始。
Vector2D SteeringBehavior::CalculateDithered()
{  
// ↓↓↓ 原文注释翻译:重置转向力。
  //reset the steering force
// 清零总力,从头来。
   m_vSteeringForce.Zero();

// 开着避墙 且 随机数 < 概率 prWallAvoidance(抽中了):
// RandFloat() 返回 0~1;概率越大越容易抽中。
  if (On(wall_avoidance) && RandFloat() < Prm.prWallAvoidance)
  {
// 算避墙力 × 权重 ÷ 概率(除以概率是数学上的"期望补偿",保持平均力度);
    m_vSteeringForce = WallAvoidance(m_pVehicle->World()->Walls()) *
                         m_dWeightWallAvoidance / Prm.prWallAvoidance;

// 力非零(有实际效果):截断到最大推力……
    if (!m_vSteeringForce.isZero())
    {
      m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
// ……直接返回,本轮结束(后面的行为这帧不出力)。
      return m_vSteeringForce;
    }
  }
   
// 避障:抽中才出力,非零即截断返回(与上面模式相同)。
  if (On(obstacle_avoidance) && RandFloat() < Prm.prObstacleAvoidance)
  {
    m_vSteeringForce += ObstacleAvoidance(m_pVehicle->World()->Obstacles()) * 
            m_dWeightObstacleAvoidance / Prm.prObstacleAvoidance;

    if (!m_vSteeringForce.isZero())
    {
      m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }

// 网格没开时:分离 抽中才出力。
  if (!isSpacePartitioningOn())
  {
    if (On(separation) && RandFloat() < Prm.prSeparation)
    {
      m_vSteeringForce += Separation(m_pVehicle->World()->Agents()) * 
                          m_dWeightSeparation / Prm.prSeparation;

      if (!m_vSteeringForce.isZero())
      {
        m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
        return m_vSteeringForce;
      }
    }
  }

  else
  {
// 网格开着时:分离Plus 抽中才出力。
    if (On(separation) && RandFloat() < Prm.prSeparation)
    {
      m_vSteeringForce += SeparationPlus(m_pVehicle->World()->Agents()) * 
                          m_dWeightSeparation / Prm.prSeparation;

      if (!m_vSteeringForce.isZero())
      {
        m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
        return m_vSteeringForce;
      }
    }
  }


// 逃离:抽中才出力。
  if (On(flee) && RandFloat() < Prm.prFlee)
  {
    m_vSteeringForce += Flee(m_pVehicle->World()->Crosshair()) * m_dWeightFlee / Prm.prFlee;

    if (!m_vSteeringForce.isZero())
    {
      m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }

// 逃命:抽中才出力(断言目标)。
  if (On(evade) && RandFloat() < Prm.prEvade)
  {
    assert(m_pTargetAgent1 && "Evade target not assigned");
    
    m_vSteeringForce += Evade(m_pTargetAgent1) * m_dWeightEvade / Prm.prEvade;

    if (!m_vSteeringForce.isZero())
    {
      m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }


// 网格没开:对齐 抽中才出力;
  if (!isSpacePartitioningOn())
  {
    if (On(allignment) && RandFloat() < Prm.prAlignment)
    {
      m_vSteeringForce += Alignment(m_pVehicle->World()->Agents()) *
                          m_dWeightAlignment / Prm.prAlignment;

      if (!m_vSteeringForce.isZero())
      {
        m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
        return m_vSteeringForce;
      }
    }

// 内聚 抽中才出力;
    if (On(cohesion) && RandFloat() < Prm.prCohesion)
    {
      m_vSteeringForce += Cohesion(m_pVehicle->World()->Agents()) * 
                          m_dWeightCohesion / Prm.prCohesion;

      if (!m_vSteeringForce.isZero())
      {
        m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
        return m_vSteeringForce;
      }
    }
  }
// 网格开着:对齐Plus 抽中才出力;
  else
  {
    if (On(allignment) && RandFloat() < Prm.prAlignment)
    {
      m_vSteeringForce += AlignmentPlus(m_pVehicle->World()->Agents()) *
                          m_dWeightAlignment / Prm.prAlignment;

      if (!m_vSteeringForce.isZero())
      {
        m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
        return m_vSteeringForce;
      }
    }

// 内聚Plus 抽中才出力。
    if (On(cohesion) && RandFloat() < Prm.prCohesion)
    {
      m_vSteeringForce += CohesionPlus(m_pVehicle->World()->Agents()) *
                          m_dWeightCohesion / Prm.prCohesion;

      if (!m_vSteeringForce.isZero())
      {
        m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
        return m_vSteeringForce;
      }
    }
  }

// 徘徊:抽中才出力。
  if (On(wander) && RandFloat() < Prm.prWander)
  {
    m_vSteeringForce += Wander() * m_dWeightWander / Prm.prWander;

    if (!m_vSteeringForce.isZero())
    {
      m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }

// 追击:抽中才出力。
  if (On(seek) && RandFloat() < Prm.prSeek)
  {
    m_vSteeringForce += Seek(m_pVehicle->World()->Crosshair()) * m_dWeightSeek / Prm.prSeek;

    if (!m_vSteeringForce.isZero())
    {
      m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }

// 到达:抽中才出力。
  if (On(arrive) && RandFloat() < Prm.prArrive)
  {
    m_vSteeringForce += Arrive(m_pVehicle->World()->Crosshair(), m_Deceleration) * 
                        m_dWeightArrive / Prm.prArrive;

    if (!m_vSteeringForce.isZero())
    {
      m_vSteeringForce.Truncate(m_pVehicle->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }
// 谁都没抽中/抽中了但力为零:返回当前总力(可能为零向量)。
 
  return m_vSteeringForce;
}



// ↓↓↓ 原文注释翻译:行为算法从这里开始(START OF BEHAVIORS)。
/////////////////////////////////////////////////////////////////////////////// START OF BEHAVIORS

//--------------------------------------------------------------------------------
//【Seek 追击】
//  原理:想要的速度 = 指向目标的单位向量 × 最大速度;
//  转向力 = 想要的速度 - 当前速度(速度差)。
//  直觉:速度差就是"该补/该减"的力——方向朝目标、大小与当前速度有关。
//--------------------------------------------------------------------------------
//------------------------------- Seek -----------------------------------
//
//  Given a target, this behavior returns a steering force which will
//  direct the agent towards the target
//------------------------------------------------------------------------
// Seek 函数定义开始。参数:目标位置。
Vector2D SteeringBehavior::Seek(Vector2D TargetPos)
{
// 想要的速度:先算"从自己指向目标"的向量,归一化成单位向量,再乘最大速度。
// Vec2DNormalize(向量) —— 长度变 1、方向不变。
  Vector2D DesiredVelocity = Vec2DNormalize(TargetPos - m_pVehicle->Pos())
                            * m_pVehicle->MaxSpeed();

// 返回"想要的速度 - 当前速度":差向量 = 该施加的转向力。
// 当前速度越偏,纠正力越大;完全朝目标飞时差为零,无需转向。
  return (DesiredVelocity - m_pVehicle->Velocity());
}

//--------------------------------------------------------------------------------
//【Flee 逃离】与 Seek 正好相反:朝"远离目标"的方向全速跑。
//--------------------------------------------------------------------------------
//----------------------------- Flee -------------------------------------
//
//  Does the opposite of Seek
//------------------------------------------------------------------------
// Flee 函数定义开始。
Vector2D SteeringBehavior::Flee(Vector2D TargetPos)
{
  //only flee if the target is within 'panic distance'. Work in distance
  //squared space.
// 原作者注释掉的一段:可选的"恐慌距离"(目标太远就不逃)。
// 被注释的代码不参与编译,原样保留。
 /* const double PanicDistanceSq = 100.0f * 100.0;
  if (Vec2DDistanceSq(m_pVehicle->Pos(), target) > PanicDistanceSq)
  {
    return Vector2D(0,0);
  }
  */

// 想要的速度:自己位置 - 目标位置(反方向)归一化 × 最大速度;
  Vector2D DesiredVelocity = Vec2DNormalize(m_pVehicle->Pos() - TargetPos) 
                            * m_pVehicle->MaxSpeed();

// 返回速度差。
  return (DesiredVelocity - m_pVehicle->Velocity());
}

//--------------------------------------------------------------------------------
//【Arrive 到达】像 Seek 但会提前减速,以"接近零速度"停到目标上。
//  减速幅度由 deceleration 档位决定(slow 最稳/fast 最急)。
//--------------------------------------------------------------------------------
//--------------------------- Arrive -------------------------------------
//
//  This behavior is similar to seek but it attempts to arrive at the
//  target with a zero velocity
//------------------------------------------------------------------------
// Arrive 函数定义开始。参数:目标位置 + 减速档位。
Vector2D SteeringBehavior::Arrive(Vector2D     TargetPos,
                                  Deceleration deceleration)
{
// 从自己指向目标的向量。
  Vector2D ToTarget = TargetPos - m_pVehicle->Pos();

  //calculate the distance to the target
// ↓↓↓ 原文注释翻译:计算到目标的距离。
  double dist = ToTarget.Length();
// 距离 = 该向量的长度(开平方)。

// 距离大于 0(还没到目标):
  if (dist > 0)
  {
// ↓↓↓ 原文注释翻译:因为 Deceleration 是枚举(整数),这个值用来精细调节减速。
    //because Deceleration is enumerated as an int, this value is required
    //to provide fine tweaking of the deceleration..
// 减速微调常量 0.3:数值越小,减速越晚、越猛。
    const double DecelerationTweaker = 0.3;

    //calculate the speed required to reach the target given the desired
    //deceleration
// ↓↓↓ 原文注释翻译:在给定减速档位下,计算到达目标所需的速度。
    double speed =  dist / ((double)deceleration * DecelerationTweaker);     
// 想要的速度 = 距离 ÷ (档位值 × 微调):档位越大(slow=3)速度越小,早早减速;
// 档位越小(fast=1)速度越大,临近目标才刹。
// (double)deceleration:枚举转 double 参与除法。

    //make sure the velocity does not exceed the max
// ↓↓↓ 原文注释翻译:确保速度不超过最大值。
    speed = min(speed, m_pVehicle->MaxSpeed());
// 速度封顶:min(想要的速度, 最大速度) 取较小者。

    //from here proceed just like Seek except we don't need to normalize 
// ↓↓↓ 原文注释翻译:从这里开始和 Seek 一样,只是无需再归一化 ToTarget,因为
//    已经算过它的长度 dist 了。
    //the ToTarget vector because we have already gone to the trouble
    //of calculating its length: dist. 
// 想要的速度 = 方向(ToTarget/dist,单位向量)× 速度大小;
    Vector2D DesiredVelocity =  ToTarget * speed / dist;

// 返回速度差 = 转向力。
    return (DesiredVelocity - m_pVehicle->Velocity());
  }

// 已经在目标上(距离 0):无需转向,返回零向量。
  return Vector2D(0,0);
}

//--------------------------------------------------------------------------------
//【Pursuit 拦截】预测猎物未来的位置,去"堵"它(而不是追当前位置)。
//  若猎物正迎面撞来且方向几乎相反(相对朝向 < -0.95,约 18°内),
//  直接追击其当前位置即可;否则估算"到达所需时间",预测未来位置再追。
//--------------------------------------------------------------------------------
//------------------------------ Pursuit ---------------------------------
//
//  this behavior creates a force that steers the agent towards the 
//  evader
//------------------------------------------------------------------------
// Pursuit 函数定义开始。参数:猎物(逃命者)指针。
Vector2D SteeringBehavior::Pursuit(const Vehicle* evader)
{
// ↓↓↓ 原文注释翻译:如果猎物在前方且正对着自己,直接追击其当前位置。
  //if the evader is ahead and facing the agent then we can just seek
  //for the evader's current position.
// 从自己指向猎物的向量。
  Vector2D ToEvader = evader->Pos() - m_pVehicle->Pos();

// 相对朝向:自己的朝向 · 猎物的朝向(点积 = 两方向夹角的余弦)。
// 点积接近 -1 = 两方向几乎相反(对面相遇)。
  double RelativeHeading = m_pVehicle->Heading().Dot(evader->Heading());

// 两个条件同时成立:
//   ToEvader.Dot(自己朝向) > 0 —— 猎物在我前方;
//   RelativeHeading < -0.95 —— 猎物几乎正对我冲来(acos(0.95)≈18°)。
  if ( (ToEvader.Dot(m_pVehicle->Heading()) > 0) &&  
       (RelativeHeading < -0.95))  //acos(0.95)=18 degs
  {
// 条件成立:直接追它当前位置(Seek)。
    return Seek(evader->Pos());
  }

// ↓↓↓ 原文注释翻译:(不算前方时)预测猎物将来会在哪。
  //Not considered ahead so we predict where the evader will be.
 
// ↓↓↓ 原文注释翻译:前瞻时间与"我与猎物之间的距离"成正比,与"两者速度之和"
//    成反比。
  //the lookahead time is propotional to the distance between the evader
  //and the pursuer; and is inversely proportional to the sum of the
  //agent's velocities
// 前瞻时间 = 距离 ÷ (我的最大速度 + 猎物当前速度)。
// 直觉:离得越远越要提前看;双方越快,相遇越快,看得越短。
  double LookAheadTime = ToEvader.Length() / 
                        (m_pVehicle->MaxSpeed() + evader->Speed());
  
  //now seek to the predicted future position of the evader
// 追"猎物当前位置 + 它的速度×时间"= 猎物未来位置(Seek 到那)。
// 速度 × 时间 = 未来一段时间的位移。
  return Seek(evader->Pos() + evader->Velocity() * LookAheadTime);
}


//--------------------------------------------------------------------------------
//【Evade 逃命】与拦截相反:预测追兵未来位置,朝反方向逃。
//  注意:追兵在我身后时也在逃(这里不做面朝判断,原书如此)。
//--------------------------------------------------------------------------------
//----------------------------- Evade ------------------------------------
//
//  similar to pursuit except the agent Flees from the estimated future
//  position of the pursuer
//------------------------------------------------------------------------
// Evade 函数定义开始。参数:追兵指针。
Vector2D SteeringBehavior::Evade(const Vehicle* pursuer)
{
// ↓↓↓ 原文注释翻译:这次不需要检查朝向(原书说明,保留)。
  /* Not necessary to include the check for facing direction this time */

// 从自己指向追兵的向量。
  Vector2D ToPursuer = pursuer->Pos() - m_pVehicle->Pos();

// ↓↓↓ 原文注释翻译:(取消下面两行的注释,可以让 Evade 只考虑"威胁范围"内的追兵。)
  //uncomment the following two lines to have Evade only consider pursuers 
  //within a 'threat range'
// 威胁范围 = 100 像素:追兵在 100 像素外就不逃(力为零)。
  const double ThreatRange = 100.0;
// 距离平方 > 威胁范围平方 → 追兵太远,返回零向量(Vector2D() 即 (0,0))。
  if (ToPursuer.LengthSq() > ThreatRange * ThreatRange) return Vector2D();
 
  //the lookahead time is propotional to the distance between the pursuer
  //and the pursuer; and is inversely proportional to the sum of the
  //agents' velocities
// 前瞻时间 = 距离 ÷ (我的最大速度 + 追兵速度)(与 Pursuit 同理)。
  double LookAheadTime = ToPursuer.Length() / 
                         (m_pVehicle->MaxSpeed() + pursuer->Speed());
  
  //now flee away from predicted future position of the pursuer
// 朝"追兵未来位置"的反方向逃(Flee 到那)。
  return Flee(pursuer->Pos() + pursuer->Velocity() * LookAheadTime);
}


//--------------------------------------------------------------------------------
//【Wander 徘徊】漫无目的地随机游荡(转向行为里最"活"的一个)。
//  模型:车头前方 WanderDist 处放一个"徘徊圆",圆上有一个目标点;
//  每帧让目标点在圆上随机抖动一下,再朝它转向——于是走出一条
//  "随机但不突兀"的游荡轨迹(不会像纯随机那样抽搐)。
//--------------------------------------------------------------------------------
//--------------------------- Wander -------------------------------------
//
//  This behavior makes the agent wander about randomly
//------------------------------------------------------------------------
// Wander 函数定义开始。
Vector2D SteeringBehavior::Wander()
{ 
// ↓↓↓ 原文注释翻译:这个行为依赖更新频率,所以用"时间无关帧率"时必须加这行。
  //this behavior is dependent on the update rate, so this line must
  //be included when using time independent framerate.
// 本帧抖动幅度 = 抖动速度 × 距上一帧的时间:帧率高低不影响抖动快慢。
  double JitterThisTimeSlice = m_dWanderJitter * m_pVehicle->TimeElapsed();

  //first, add a small random vector to the target's position
// 目标点 += 一个随机小向量(RandomClamped() 返回 -1~1):在圆上"抖"一下。
  m_vWanderTarget += Vector2D(RandomClamped() * JitterThisTimeSlice,
                              RandomClamped() * JitterThisTimeSlice);

  //reproject this new vector back on to a unit circle
// 把抖完的点重新归一化:投影回单位圆上(只留方向)。
  m_vWanderTarget.Normalize();

  //increase the length of the vector to the same as the radius
  //of the wander circle
// 放大到徘徊圆半径:单位向量 × 半径 = 圆上的点。
  m_vWanderTarget *= m_dWanderRadius;

  //move the target into a position WanderDist in front of the agent
// 把目标点平移到"车头前方 WanderDist 处":先造一个本地坐标点
// (WanderDist, 0),加上圆上偏移。
  Vector2D target = m_vWanderTarget + Vector2D(m_dWanderDistance, 0);

  //project the target into world space
// ↓↓↓ 原文注释翻译:把目标点投影到世界坐标。
  Vector2D Target = PointToWorldSpace(target,
// PointToWorldSpace(本地点, 朝向, 侧向, 位置):把车头系的点变换到屏幕坐标——
// 朝前 WanderDist、左右按徘徊圆偏移的那个世界位置。
                                       m_pVehicle->Heading(),
                                       m_pVehicle->Side(), 
                                       m_pVehicle->Pos());

  //and steer towards it
// 转向力 = 目标世界位置 - 自己位置(朝那个点飞的方向向量)。
  return Target - m_pVehicle->Pos(); 
}


//--------------------------------------------------------------------------------
//【ObstacleAvoidance 避障】返回一个避开"最近相交障碍物"的转向力。
//  算法:在车头前方放一个"检测盒"(长度随速度变化),把盒内障碍物变换到
//  车头本地坐标;找"盒轴线上最先被穿过的"那个;算侧向推离力 + 制动力。
//  直觉:像开车时"看前方一段路",只躲最近挡路的那个,其他不管。
//--------------------------------------------------------------------------------
//---------------------- ObstacleAvoidance -------------------------------
//
//  Given a vector of CObstacles, this method returns a steering force
//  that will prevent the agent colliding with the closest obstacle
//------------------------------------------------------------------------
// ObstacleAvoidance 函数定义开始。参数:障碍物指针容器(只读)。
Vector2D SteeringBehavior::ObstacleAvoidance(const std::vector<BaseGameEntity*>& obstacles)
{
  //the detection box length is proportional to the agent's velocity
// ↓↓↓ 原文注释翻译:检测盒长度与车辆速度成正比。
  m_dDBoxLength = Prm.MinDetectionBoxLength + 
// 检测盒长度 = 基础长度 + (当前速度÷最大速度)×基础长度:
// 开得越快,看得越远(刹车距离更长,需要更早发现障碍)。
                  (m_pVehicle->Speed()/m_pVehicle->MaxSpeed()) *
                  Prm.MinDetectionBoxLength;

  //tag all obstacles within range of the box for processing
// 先给"检测盒范围内"的障碍打标记:只处理打了钩的(见 TagNeighbors)。
  m_pVehicle->World()->TagObstaclesWithinViewRange(m_pVehicle, m_dDBoxLength);

// ↓↓↓ 原文注释翻译:这个变量记录"最近的相交障碍物"(CIB = Closest
//    Intersecting Obstacle)。
  //this will keep track of the closest intersecting obstacle (CIB)
// 最近相交障碍物指针,先置空。
  BaseGameEntity* ClosestIntersectingObstacle = NULL;
 
  //this will be used to track the distance to the CIB
// 记录"到最近相交点"的距离,先置为极大值(第一轮必然被替换)。
  double DistToClosestIP = MaxDouble;

  //this will record the transformed local coordinates of the CIB
// 记录最近障碍物在"车头本地坐标"里的位置(后面算推力要用)。
  Vector2D LocalPosOfClosestObstacle;

// 遍历障碍物容器的迭代器(书签),从开头开始。
  std::vector<BaseGameEntity*>::const_iterator curOb = obstacles.begin();

// while 循环:书签没到末尾就继续检查。
  while(curOb != obstacles.end())
  {
    //if the obstacle has been tagged within range proceed
// 只处理"被打过标记"(在检测盒范围内)的障碍物。
    if ((*curOb)->IsTagged())
    {
      //calculate this obstacle's position in local space
// PointToLocalSpace(世界坐标, 朝向, 侧向, 位置):把障碍物位置变换到
// "车头系"——车头为原点、车头方向为 +X 轴。
      Vector2D LocalPos = PointToLocalSpace((*curOb)->Pos(),
                                             m_pVehicle->Heading(),
                                             m_pVehicle->Side(),
                                             m_pVehicle->Pos());

// ↓↓↓ 原文注释翻译:如果本地位置的 x 为负,说明障碍在车后面,可以忽略。
      //if the local position has a negative x value then it must lay
      //behind the agent. (in which case it can be ignored)
// 车头系下 x ≥ 0(障碍在前方)才值得处理。
      if (LocalPos.x >= 0)
      {
        //if the distance from the x axis to the object's position is less
// ↓↓↓ 原文注释翻译:如果障碍到 X 轴(车头线)的距离小于"它的半径 + 检测盒
//    半宽",就是潜在相交。
        //than its radius + half the width of the detection box then there
        //is a potential intersection.
// 扩张半径 = 障碍物半径 + 自己半径:两个圆都要算进去。
        double ExpandedRadius = (*curOb)->BRadius() + m_pVehicle->BRadius();

// fabs(本地y) < 扩张半径:障碍物横向离车头线够近,可能撞上。
// fabs = 绝对值(取正)。
        if (fabs(LocalPos.y) < ExpandedRadius)
        {
// ↓↓↓ 原文注释翻译:现在做"直线/圆"相交测试。圆心是 (cX, cY),交点公式为
//    x = cX ± sqrt(r²-cY²)(y=0 时)。只需看最小的正 x,那就是最近的交点。
          //now to do a line/circle intersection test. The center of the 
          //circle is represented by (cX, cY). The intersection points are 
          //given by the formula x = cX +/-sqrt(r^2-cY^2) for y=0. 
          //We only need to look at the smallest positive value of x because
          //that will be the closest point of intersection.
// 圆心本地坐标的 x(障碍物在车头前方的距离);
          double cX = LocalPos.x;
// 圆心本地坐标的 y(横向偏移)。
          double cY = LocalPos.y;
          
          //we only need to calculate the sqrt part of the above equation once
// ↓↓↓ 原文注释翻译:上面的公式里 sqrt 部分只需算一次。
          double SqrtPart = sqrt(ExpandedRadius*ExpandedRadius - cY*cY);
// sqrt(半径² - y²):圆与车头线(X 轴)交点横坐标的偏移量。
// sqrt = 开平方根(勾股定理:r² = y² + x²)。

// 最近的交点 ip = 圆心 x - sqrtPart(圆"迎向车头"的那一侧)。
          double ip = cX - SqrtPart;

// 如果 ip ≤ 0(圆已经盖过车头/在身后)……
          if (ip <= 0.0)
          {
// ……改用另一侧交点:ip = 圆心 x + sqrtPart。
            ip = cX + SqrtPart;
          }

// ↓↓↓ 原文注释翻译:看看这是不是目前最近的。是的话,记下该障碍与它的本地坐标。
          //test to see if this is the closest so far. If it is keep a
          //record of the obstacle and its local coordinates
// 这个交点比之前记录的更近:
          if (ip < DistToClosestIP)
          {
// 更新最近交点距离;
            DistToClosestIP = ip;

// 记下这个"最近相交障碍物";
            ClosestIntersectingObstacle = *curOb;

// 记下它的本地坐标。
            LocalPosOfClosestObstacle = LocalPos;
          }         
        }
      }
    }

    ++curOb;
  }

// ↓↓↓ 原文注释翻译:如果找到了相交障碍物,算一个推离它的转向力。
  //if we have found an intersecting obstacle, calculate a steering 
  //force away from it
// 转向力向量,先默认零(没障碍就不转向)。
  Vector2D SteeringForce;

// 确实有最近相交障碍物:
  if (ClosestIntersectingObstacle)
  {
// ↓↓↓ 原文注释翻译:离障碍越近,转向力应该越强。
    //the closer the agent is to an object, the stronger the 
    //steering force should be
// 倍率 = 1 + (检测盒长度 - 障碍本地x) ÷ 检测盒长度:
// 障碍越靠近车头(本地x 越小),倍率越大,推得越狠。
    double multiplier = 1.0 + (m_dDBoxLength - LocalPosOfClosestObstacle.x) /
                        m_dDBoxLength;

// ↓↓↓ 原文注释翻译:计算侧向(横向)推力。
    //calculate the lateral force
// 侧向力 = (障碍半径 - 本地y) × 倍率:把车往"远离障碍横向位置"的方向推;
    SteeringForce.y = (ClosestIntersectingObstacle->BRadius()-
                       LocalPosOfClosestObstacle.y)  * multiplier;   
// 本地坐标里 y 是横向,所以转向力 y 分量 = 横向推力(车头系)。

// ↓↓↓ 原文注释翻译:加一个与"障碍到车距离"成正比的制动力。
    //apply a braking force proportional to the obstacles distance from
    //the vehicle. 
// 制动力权重 0.2:给推力留一点"减速"分量,避免擦着障碍飞过。
    const double BrakingWeight = 0.2;

// 制动力 x 分量 = (障碍半径 - 本地x) × 0.2:离得越近刹车越重。
    SteeringForce.x = (ClosestIntersectingObstacle->BRadius() - 
                       LocalPosOfClosestObstacle.x) * 
                       BrakingWeight;
  }

// ↓↓↓ 原文注释翻译:最后把转向向量从本地坐标变回世界坐标。
  //finally, convert the steering vector from local to world space
// VectorToWorldSpace(本地方, 朝向, 侧向):把车头系算出的力转成屏幕坐标系的力。
  return VectorToWorldSpace(SteeringForce,
                            m_pVehicle->Heading(),
                            m_pVehicle->Side());
}


//--------------------------------------------------------------------------------
//【WallAvoidance 避墙】返回一个让车远离墙壁的转向力。
//  方法:从车头伸出 3 根"触须"(天线):正中 1 根长、左右 2 根半长;
//  检测触须与墙壁的交点;若有交,则沿"墙的法线"方向施加一个
//  大小等于"触须末端超出交点距离"的推力(把车头推离墙)。
//--------------------------------------------------------------------------------
//--------------------------- WallAvoidance --------------------------------
//
//  This returns a steering force that will keep the agent away from any
//  walls it may encounter
//------------------------------------------------------------------------
// WallAvoidance 函数定义开始。参数:墙壁容器(只读)。
Vector2D SteeringBehavior::WallAvoidance(const std::vector<Wall2D>& walls)
{
  //the feelers are contained in a std::vector, m_Feelers
// 先调用 CreateFeelers() 生成 3 根触须端点(存在 m_Feelers 里)。
  CreateFeelers();
  
// 记录"本触须最近交点距离",初始 0;
  double DistToThisIP    = 0.0;
// 记录"所有触须中最近交点距离",初始极大值。
  double DistToClosestIP = MaxDouble;

  //this will hold an index into the vector of walls
// 记录最近交点的墙下标,初始 -1(表示没找到)。
  int ClosestWall = -1;

// 转向力 / 临时点 / 最近交点 三个向量变量。
  Vector2D SteeringForce,
// point:临时存放交点;
            point,         //used for storing temporary info
// ClosestPoint:最近交点。
            ClosestPoint;  //holds the closest intersection point

// ↓↓↓ 原文注释翻译:逐一检查每根触须。
  //examine each feeler in turn
// 外层循环:遍历 3 根触须(下标 flr)。
  for (unsigned int flr=0; flr<m_Feelers.size(); ++flr)
  {
// ↓↓↓ 原文注释翻译:逐墙检查是否有交点。
    //run through each wall checking for any intersection points
// 内层循环:遍历所有墙段(下标 w)。
    for (unsigned int w=0; w<walls.size(); ++w)
    {
// LineIntersection2D(线段1起点, 线段1终点, 线段2起点, 线段2终点,
//   输出距离, 输出交点):判断"车位置→触须端点"线段是否穿过"墙段"。
// 相交时函数返回 true,并把交点到车位置的距离写进 DistToThisIP,
// 交点坐标写进 point。
      if (LineIntersection2D(m_pVehicle->Pos(),
                             m_Feelers[flr],
                             walls[w].From(),
                             walls[w].To(),
                             DistToThisIP,
                             point))
      {
// 这个交点比已记录的最远点更近:
        //is this the closest found so far? If so keep a record
        if (DistToThisIP < DistToClosestIP)
// 更新最近距离;
        {
          DistToClosestIP = DistToThisIP;
// 记录墙的下标;

          ClosestWall = w;
// 记录交点坐标。

          ClosestPoint = point;
        }
      }
    }//next wall

  
// ↓↓↓ 原文注释翻译:如果检测到交点,计算一个把车推离墙的力。
    //if an intersection point has been detected, calculate a force  
    //that will direct the agent away
// 触须末端到交点的向量 = "车头冲出去"超出的部分(OverShoot 越界量)。
    if (ClosestWall >=0)
    {
      //calculate by what distance the projected position of the agent
      //will overshoot the wall
// ↓↓↓ 原文注释翻译:沿墙法线方向、大小等于越界量,创建推力。
      Vector2D OverShoot = m_Feelers[flr] - ClosestPoint;

      //create a force in the direction of the wall normal, with a 
      //magnitude of the overshoot
// 推力 = 墙的法线(单位向量)× 越界长度:冲出去多少,就推回来多少。
// walls[ClosestWall].Normal():墙段的法线(垂直墙面的方向)。
      SteeringForce = walls[ClosestWall].Normal() * OverShoot.Length();
    }

  }//next feeler

// 返回转向力(没有撞墙时是零向量)。
  return SteeringForce;
}

//--------------------------------------------------------------------------------
//【CreateFeelers:生成 3 根触须】
//  触须是"车位置 + 方向×长度"的端点:
//    触须0:正前方,全长(用来探正前方的墙);
//    触须1:左前方 135°(HalfPi×3.5),半长;
//    触须2:右前方 45°(HalfPi×0.5),半长。
//--------------------------------------------------------------------------------
//------------------------------- CreateFeelers --------------------------
//
//  Creates the antenna utilized by WallAvoidance
//------------------------------------------------------------------------
// CreateFeelers 函数定义开始。
void SteeringBehavior::CreateFeelers()
{
  //feeler pointing straight in front
// 触须0 = 车位置 + 触须长度 × 朝向:正前方最远的那根。
// (朝向是单位向量,乘长度得到"车头方向 length 像素处"的点。)
  m_Feelers[0] = m_pVehicle->Pos() + m_dWallDetectionFeelerLength * m_pVehicle->Heading();

// ↓↓↓ 原文注释翻译:朝左的触须。
  //feeler to left
// 先把朝向向量拷进临时变量 temp;
  Vector2D temp = m_pVehicle->Heading();
// 把 temp 绕原点旋转 HalfPi×3.5 = 315°(即左偏 45°);
  Vec2DRotateAroundOrigin(temp, HalfPi * 3.5f);
// 触须1 = 车位置 + (半长)× 旋转后的方向。
  m_Feelers[1] = m_pVehicle->Pos() + m_dWallDetectionFeelerLength/2.0f * temp;

// ↓↓↓ 原文注释翻译:朝右的触须。
  //feeler to right
// 再拷朝向;
  temp = m_pVehicle->Heading();
// 旋转 HalfPi×0.5 = 45°(右偏 45°);
  Vec2DRotateAroundOrigin(temp, HalfPi * 0.5f);
// 触须2 = 车位置 + (半长)× 旋转后的方向。
  m_Feelers[2] = m_pVehicle->Pos() + m_dWallDetectionFeelerLength/2.0f * temp;
}


//--------------------------------------------------------------------------------
//【Separation 分离】与邻居保持距离:每个邻居推我一把,离得越近推得越狠。
//  力 = Σ(背向邻居的单位向量 ÷ 距离):距离做分母 → 越近力越大(1/距离)。
//  只处理"被打标记(视野内)且不是我、也不是逃命目标"的邻居。
//--------------------------------------------------------------------------------
//---------------------------- Separation --------------------------------
//
// this calculates a force repelling from the other neighbors
//------------------------------------------------------------------------
// Separation 函数定义开始。参数:机器人容器(邻居列表)。
Vector2D SteeringBehavior::Separation(const vector<Vehicle*> &neighbors)
{  
// 转向力,初始零。
  Vector2D SteeringForce;

// 遍历所有机器人(下标 a)。
  for (unsigned int a=0; a<neighbors.size(); ++a)
  {
// ↓↓↓ 原文注释翻译:确保不算自己、只算够近的(打了标记的)。
//    ***同时确保不把逃命目标算进来(否则逃命力会被分离抵消)***
    //make sure this agent isn't included in the calculations and that
    //the agent being examined is close enough. ***also make sure it doesn't
    //include the evade target ***
// 三个条件同时成立才算:不是自己 && 打过标记 && 不是逃命目标。
// != 比较指针地址。
    if((neighbors[a] != m_pVehicle) && neighbors[a]->IsTagged() &&
      (neighbors[a] != m_pTargetAgent1))
    {
// 从邻居指向自己的向量(背向邻居的方向)。
      Vector2D ToAgent = m_pVehicle->Pos() - neighbors[a]->Pos();

// ↓↓↓ 原文注释翻译:按"与邻居距离成反比"缩放力。
      //scale the force inversely proportional to the agents distance  
      //from its neighbor.
// 力 += 归一化(背向向量) ÷ 距离:方向背向邻居,大小 1/距离。
// 邻居贴脸(距离≈0)时力趋近无穷大——完美"别挤我"。
      SteeringForce += Vec2DNormalize(ToAgent)/ToAgent.Length();
    }
  }

// 返回分离力。
  return SteeringForce;
}


//--------------------------------------------------------------------------------
//【Alignment 对齐】让自己的朝向与邻居们的"平均朝向"一致(群飞排成一线)。
//  力 = (邻居平均朝向 - 我的朝向):差多少,就转多少。
//--------------------------------------------------------------------------------
//---------------------------- Alignment ---------------------------------
//
//  returns a force that attempts to align this agents heading with that
//  of its neighbors
//------------------------------------------------------------------------
// Alignment 函数定义开始。
Vector2D SteeringBehavior::Alignment(const vector<Vehicle*>& neighbors)
{
  //used to record the average heading of the neighbors
// 累加器:邻居朝向向量之和(初始零);
  Vector2D AverageHeading;
// 计数器:有效的邻居数量。

  //used to count the number of vehicles in the neighborhood
  int    NeighborCount = 0;

// 遍历所有机器人:把有效邻居的朝向向量累加;
  //iterate through all the tagged vehicles and sum their heading vectors  
  for (unsigned int a=0; a<neighbors.size(); ++a)
  {
    //make sure *this* agent isn't included in the calculations and that
    //the agent being examined  is close enough ***also make sure it doesn't
    //include any evade target ***
// 条件:不是自己 && 打过标记 && 不是逃命目标(同 Separation);
    if((neighbors[a] != m_pVehicle) && neighbors[a]->IsTagged() &&
      (neighbors[a] != m_pTargetAgent1))
    {
// 累加它的朝向向量;
      AverageHeading += neighbors[a]->Heading();

// 计数 +1。
      ++NeighborCount;
    }
  }

// ↓↓↓ 原文注释翻译:如果邻居数 ≥ 1,对朝向向量取平均。
  //if the neighborhood contained one or more vehicles, average their
  //heading vectors.
// 平均朝向 = 总和 ÷ 邻居数;
  if (NeighborCount > 0)
  {
// 力 = 平均朝向 - 我的朝向:朝"大家的平均方向"转。
    AverageHeading /= (double)NeighborCount;

    AverageHeading -= m_pVehicle->Heading();
  }
  
// 返回对齐力。
  return AverageHeading;
}

//--------------------------------------------------------------------------------
//【Cohesion 内聚】朝邻居们的"重心"靠拢(群飞聚成团)。
//  力 = Seek(重心):用追击行为飞向所有邻居位置的平均点。
//  最后归一化:内聚力通常比分离/对齐大得多,归一化防它一家独大。
//--------------------------------------------------------------------------------
//-------------------------------- Cohesion ------------------------------
//
//  returns a steering force that attempts to move the agent towards the
//  center of mass of the agents in its immediate area
//------------------------------------------------------------------------
// Cohesion 函数定义开始。
Vector2D SteeringBehavior::Cohesion(const vector<Vehicle*> &neighbors)
{
  //first find the center of mass of all the agents
// 重心累加器 + 转向力,初始零;
  Vector2D CenterOfMass, SteeringForce;

// 计数器。
  int NeighborCount = 0;

  //iterate through the neighbors and sum up all the position vectors
// 遍历所有机器人:把有效邻居的位置累加;
  for (unsigned int a=0; a<neighbors.size(); ++a)
  {
    //make sure *this* agent isn't included in the calculations and that
    //the agent being examined is close enough ***also make sure it doesn't
    //include the evade target ***
// 条件:不是自己 && 打过标记 && 不是逃命目标;
    if((neighbors[a] != m_pVehicle) && neighbors[a]->IsTagged() &&
      (neighbors[a] != m_pTargetAgent1))
    {
// 累加位置;
      CenterOfMass += neighbors[a]->Pos();

// 计数 +1。
      ++NeighborCount;
    }
  }

// 有邻居才处理:
  if (NeighborCount > 0)
  {
    //the center of mass is the average of the sum of positions
// 重心 = 位置总和 ÷ 邻居数(平均点);
    CenterOfMass /= (double)NeighborCount;

// ↓↓↓ 原文注释翻译:现在朝那个位置追击(Seek)。
    //now seek towards that position
// 内聚力 = Seek(重心):朝重心全速飞。
    SteeringForce = Seek(CenterOfMass);
  }

// ↓↓↓ 原文注释翻译:内聚力通常远大于分离/对齐,最好归一化它。
  //the magnitude of cohesion is usually much larger than separation or
  //allignment so it usually helps to normalize it.
// 返回归一化的内聚力(方向照旧、长度 1)。
  return Vec2DNormalize(SteeringForce);
}


// ↓↓↓ 原文注释翻译:注意:下面三个行为与上面三个相同,只是用网格空间划分
//    来找邻居(只遍历自己附近格子的实体,而不是整个列表)。
//  邻居来源不同(网格迭代器),算法本身一致。
/* NOTE: the next three behaviors are the same as the above three, except
          that they use a cell-space partition to find the neighbors
*/


//--------------------------------------------------------------------------------
//【SeparationPlus 网格版分离】与 Separation 算法相同,但邻居来自网格查询。
//  遍历网格迭代器(begin/next)而非整个机器人列表——大场景下快得多。
//--------------------------------------------------------------------------------
//---------------------------- Separation --------------------------------
//
// this calculates a force repelling from the other neighbors
//
//  USES SPACIAL PARTITIONING
//------------------------------------------------------------------------
// SeparationPlus 函数定义开始。
Vector2D SteeringBehavior::SeparationPlus(const vector<Vehicle*> &neighbors)
{  
// 转向力,初始零。
  Vector2D SteeringForce;

  //iterate through the neighbors and sum up all the position vectors
// for 循环:用网格迭代器遍历"附近格子里的实体":
//   begin() 取第一个;!end() 判断没结束;next() 取下一个。
// 网格划分器在 Calculate() 里已按视野算好邻居,这里只消费。
  for (BaseGameEntity* pV = m_pVehicle->World()->CellSpace()->begin();
                         !m_pVehicle->World()->CellSpace()->end();     
                       pV = m_pVehicle->World()->CellSpace()->next())
  {    
    //make sure this agent isn't included in the calculations and that
    //the agent being examined is close enough
// 不是自己才算(网格只装了附近的,无需再查距离/标记):
    if(pV != m_pVehicle)
    {
// 背向邻居的向量;
      Vector2D ToAgent = m_pVehicle->Pos() - pV->Pos();

      //scale the force inversely proportional to the agents distance  
      //from its neighbor.
// 力 += 归一化 ÷ 距离(与普通版相同)。
      SteeringForce += Vec2DNormalize(ToAgent)/ToAgent.Length();
    }

  }

// 返回分离力。
  return SteeringForce;
}
//--------------------------------------------------------------------------------
//【AlignmentPlus 网格版对齐】与 Alignment 相同,邻居来自网格。
//--------------------------------------------------------------------------------
//---------------------------- Alignment ---------------------------------
//
//  returns a force that attempts to align this agents heading with that
//  of its neighbors
//
//  USES SPACIAL PARTITIONING
//------------------------------------------------------------------------
// AlignmentPlus 函数定义开始。
Vector2D SteeringBehavior::AlignmentPlus(const vector<Vehicle*> &neighbors)
{
  //This will record the average heading of the neighbors
// 平均朝向累加器;
  Vector2D AverageHeading;

  //This count the number of vehicles in the neighborhood
// 邻居计数(注意这里是 double,后面直接当除数)。
  double    NeighborCount = 0.0;

  //iterate through the neighbors and sum up all the position vectors
// 用网格迭代器遍历邻居(注意这里迭代器类型是 MovingEntity*,因为网格模板
// 统一按移动实体存储);
  for (MovingEntity* pV = m_pVehicle->World()->CellSpace()->begin();
                         !m_pVehicle->World()->CellSpace()->end();     
                     pV = m_pVehicle->World()->CellSpace()->next())
  {
    //make sure *this* agent isn't included in the calculations and that
    //the agent being examined  is close enough
// 不是自己:累加它的朝向、计数 +1;
    if(pV != m_pVehicle)
    {
      AverageHeading += pV->Heading();

      ++NeighborCount;
    }

  }

// ↓↓↓ 原文注释翻译:如果邻居数 > 0,对朝向取平均。
  //if the neighborhood contained one or more vehicles, average their
  //heading vectors.
// 有邻居:平均朝向 = 总和 ÷ 数量;
  if (NeighborCount > 0.0)
  {
// 力 = 平均朝向 - 我的朝向。
    AverageHeading /= NeighborCount;

    AverageHeading -= m_pVehicle->Heading();
  }
  
// 返回对齐力。
  return AverageHeading;
}


//--------------------------------------------------------------------------------
//【CohesionPlus 网格版内聚】与 Cohesion 相同,邻居来自网格。
//--------------------------------------------------------------------------------
//-------------------------------- Cohesion ------------------------------
//
//  returns a steering force that attempts to move the agent towards the
//  center of mass of the agents in its immediate area
//
//  USES SPACIAL PARTITIONING
//------------------------------------------------------------------------
// CohesionPlus 函数定义开始。
Vector2D SteeringBehavior::CohesionPlus(const vector<Vehicle*> &neighbors)
{
  //first find the center of mass of all the agents
// 重心累加器 + 转向力;
  Vector2D CenterOfMass, SteeringForce;

// 计数器。
  int NeighborCount = 0;

  //iterate through the neighbors and sum up all the position vectors
// 网格迭代器遍历邻居:累加位置、计数;
  for (BaseGameEntity* pV = m_pVehicle->World()->CellSpace()->begin();
                         !m_pVehicle->World()->CellSpace()->end();     
                       pV = m_pVehicle->World()->CellSpace()->next())
  {
    //make sure *this* agent isn't included in the calculations and that
    //the agent being examined is close enough
// 不是自己才算;
    if(pV != m_pVehicle)
    {
      CenterOfMass += pV->Pos();

      ++NeighborCount;
    }
  }

// 有邻居:重心 = 总和 ÷ 数量;
  if (NeighborCount > 0)
  {
    //the center of mass is the average of the sum of positions
// 内聚力 = Seek(重心);
    CenterOfMass /= (double)NeighborCount;

    //now seek towards that position
    SteeringForce = Seek(CenterOfMass);
  }

  //the magnitude of cohesion is usually much larger than separation or
  //allignment so it usually helps to normalize it.
// 归一化返回。
  return Vec2DNormalize(SteeringForce);
}


//--------------------------------------------------------------------------------
//【Interpose 插到中间】站到两个移动目标"未来"的中点。
//  做法:先算两点当前中点;估算自己飞到那的时间 T;用 T 预测两个目标
//  的未来位置;取"未来中点"再用 Arrive 驶过去。
//--------------------------------------------------------------------------------
//--------------------------- Interpose ----------------------------------
//
//  Given two agents, this method returns a force that attempts to 
//  position the vehicle between them
//------------------------------------------------------------------------
// Interpose 函数定义开始。参数:两个目标机器人。
Vector2D SteeringBehavior::Interpose(const Vehicle* AgentA,
                                     const Vehicle* AgentB)
{
  //first we need to figure out where the two agents are going to be at 
  //time T in the future. This is approximated by determining the time
  //taken to reach the mid way point at the current time at at max speed.
// 当前中点 = (A位置 + B位置) ÷ 2。
  Vector2D MidPoint = (AgentA->Pos() + AgentB->Pos()) / 2.0;

// 估算飞到中点的用时 = 到中点的距离 ÷ 我的最大速度。
  double TimeToReachMidPoint = Vec2DDistance(m_pVehicle->Pos(), MidPoint) /
                               m_pVehicle->MaxSpeed();

  //now we have T, we assume that agent A and agent B will continue on a
  //straight trajectory and extrapolate to get their future positions
// 预测 A 的未来位置 = A 现在位置 + A速度 × 用时(直线外推);
  Vector2D APos = AgentA->Pos() + AgentA->Velocity() * TimeToReachMidPoint;
// 预测 B 的未来位置。
  Vector2D BPos = AgentB->Pos() + AgentB->Velocity() * TimeToReachMidPoint;

  //calculate the mid point of these predicted positions
// 未来中点 = (A未来 + B未来) ÷ 2;
  MidPoint = (APos + BPos) / 2.0;

  //then steer to Arrive at it
// 用"快速到达"(fast 档)飞向未来中点——尽量准时到位。
  return Arrive(MidPoint, fast);
}

//--------------------------------------------------------------------------------
//【Hide 躲藏】找一个"障碍物背对猎人"的位置躲起来。
//  对每个障碍物算一个"藏身点"(在障碍远离猎人那侧 30 像素处),
//  选离自己最近的藏身点,用 Arrive 飞过去;没有合适障碍就 Evade 逃。
//--------------------------------------------------------------------------------
//--------------------------- Hide ---------------------------------------
//
//------------------------------------------------------------------------
// Hide 函数定义开始。参数:猎人 + 障碍物列表。
Vector2D SteeringBehavior::Hide(const Vehicle*           hunter,
                                 const vector<BaseGameEntity*>& obstacles)
{
// 最近距离,初始极大值;
  double    DistToClosest = MaxDouble;
// 最佳藏身点。
  Vector2D BestHidingSpot;

// 障碍迭代器(书签);
  std::vector<BaseGameEntity*>::const_iterator curOb = obstacles.begin();
// 最近障碍的迭代器(记录用)。
  std::vector<BaseGameEntity*>::const_iterator closest;

// 遍历每个障碍物:
  while(curOb != obstacles.end())
  {
    //calculate the position of the hiding spot for this obstacle
// 用 GetHidingPosition 算这个障碍的藏身点(障碍位置、半径、猎人位置);
    Vector2D HidingSpot = GetHidingPosition((*curOb)->Pos(),
                                             (*curOb)->BRadius(),
                                              hunter->Pos());
            
// ↓↓↓ 原文注释翻译:用"距离平方"找离自己最近的藏身点。
    //work in distance-squared space to find the closest hiding
    //spot to the agent
// 藏身点到自己的距离平方;
    double dist = Vec2DDistanceSq(HidingSpot, m_pVehicle->Pos());

// 比已记录的最远距离还近:
    if (dist < DistToClosest)
    {
// 更新最近距离;
      DistToClosest = dist;

// 更新最佳藏身点;
      BestHidingSpot = HidingSpot;

// 记录这个障碍。
      closest = curOb;
    }  
            
    ++curOb;

// 下一个障碍(end while)。
  }//end while
  
  //if no suitable obstacles found then Evade the hunter
// ↓↓↓ 原文注释翻译:如果没有合适的障碍,就 Evade(逃)开猎人。
  if (DistToClosest == MaxFloat)
// 距离还是极大值(MaxFloat)= 一个藏身点都没找到:
  {
// 改为"逃命"(Evade)直接跑。
    return Evade(hunter);
  }
      
  //else use Arrive on the hiding spot
// 有藏身点:用"快速到达"飞过去躲好。
  return Arrive(BestHidingSpot, fast);
}

//--------------------------------------------------------------------------------
//【GetHidingPosition 算藏身点】
//  给定猎人位置与障碍(位置+半径),返回一个"障碍背对猎人那一侧"的点:
//  沿"猎人→障碍"的方向,从障碍圆心再往外推 半径+30 像素。
//--------------------------------------------------------------------------------
//------------------------- GetHidingPosition ----------------------------
//
//  Given the position of a hunter, and the position and radius of
//  an obstacle, this method calculates a position DistanceFromBoundary 
//  away from its bounding radius and directly opposite the hunter
//------------------------------------------------------------------------
// GetHidingPosition 函数定义开始。参数:障碍位置、障碍半径、猎人位置。
Vector2D SteeringBehavior::GetHidingPosition(const Vector2D& posOb,
                                              const double     radiusOb,
                                              const Vector2D& posHunter)
{
  //calculate how far away the agent is to be from the chosen obstacle's
  //bounding radius
// 藏身点离障碍边界的距离:30 像素(再近容易被发现,再远不够隐蔽);
  const double DistanceFromBoundary = 30.0;
// 总推离距离 = 障碍半径 + 30。
  double       DistAway    = radiusOb + DistanceFromBoundary;

// ↓↓↓ 原文注释翻译:计算从猎人指向障碍物的方向。
  //calculate the heading toward the object from the hunter
// 方向向量 = 归一化(障碍位置 - 猎人位置):从猎人看向障碍的方向。
  Vector2D ToOb = Vec2DNormalize(posOb - posHunter);
  
// ↓↓↓ 原文注释翻译:按大小缩放,再加到障碍位置,得到藏身点。
  //scale it to size and add to the obstacles position to get
  //the hiding spot.
// 藏身点 = 障碍位置 + 方向 × 推离距离:在障碍"远离猎人"的那一侧。
  return (ToOb * DistAway) + posOb;
}


//--------------------------------------------------------------------------------
//【FollowPath 沿路径】依次飞向每个路标点。
//  离当前路标点够近(阈值 m_dWaypointSeekDistSq)就切换到下一个;
//  还有路标点用 Seek 飞过去;走完最后一个用 Arrive 平稳停住。
//--------------------------------------------------------------------------------
//------------------------------- FollowPath -----------------------------
//
//  Given a series of Vector2Ds, this method produces a force that will
//  move the agent along the waypoints in order. The agent uses the
// 'Seek' behavior to move to the next waypoint - unless it is the last
//  waypoint, in which case it 'Arrives'
//------------------------------------------------------------------------
// FollowPath 函数定义开始。
Vector2D SteeringBehavior::FollowPath()
{ 
  //move to next target if close enough to current target (working in
  //distance squared space)
// 距离平方比较:到当前路标点的距离平方 < 切换阈值平方 → 够近了;
  if(Vec2DDistanceSq(m_pPath->CurrentWaypoint(), m_pVehicle->Pos()) <
     m_dWaypointSeekDistSq)
// 切换到下一个路标点(SetNextWaypoint,见 Path.h;循环路径会自动绕回)。
  {
    m_pPath->SetNextWaypoint();
  }

// 还没走完(书签没到末尾):
  if (!m_pPath->Finished())
  {
// 用 Seek 飞向当前路标点。
    return Seek(m_pPath->CurrentWaypoint());
  }

// 走完了:
  else
  {
// 用 Arrive(正常档)停在最后一点上,不冲过去。
    return Arrive(m_pPath->CurrentWaypoint(), normal);
  }
}

//--------------------------------------------------------------------------------
//【OffsetPursuit 编队追击】保持与"队长"的一个固定偏移位置(编队阵型)。
//  做法:把偏移向量变换到队长的世界坐标(跟着队长转);再预测队长未来
//  位置,用 Arrive 保持相对位置。
//--------------------------------------------------------------------------------
//------------------------- Offset Pursuit -------------------------------
//
//  Produces a steering force that keeps a vehicle at a specified offset
//  from a leader vehicle
//------------------------------------------------------------------------
// OffsetPursuit 函数定义开始。参数:队长 + 偏移向量。
Vector2D SteeringBehavior::OffsetPursuit(const Vehicle*  leader,
                                          const Vector2D offset)
{
// ↓↓↓ 原文注释翻译:计算偏移在世界坐标中的位置。
  //calculate the offset's position in world space
// PointToWorldSpace(偏移, 队长朝向, 队长侧向, 队长位置):把"队长本地系"
// 里的偏移(如左后方)变换成世界坐标——队长转身,偏移点跟着转。
  Vector2D WorldOffsetPos = PointToWorldSpace(offset,
                                                  leader->Heading(),
                                                  leader->Side(),
                                                  leader->Pos());

// 从我指向偏移点的向量;
  Vector2D ToOffset = WorldOffsetPos - m_pVehicle->Pos();

// ↓↓↓ 原文注释翻译:前瞻时间与"到偏移点的距离"成正比,与"两车速度之和"
//    成反比。
  //the lookahead time is propotional to the distance between the leader
  //and the pursuer; and is inversely proportional to the sum of both
  //agent's velocities
// 前瞻时间 = 距离 ÷ (我的最大速度 + 队长速度);
  double LookAheadTime = ToOffset.Length() / 
                        (m_pVehicle->MaxSpeed() + leader->Speed());
  
  //now Arrive at the predicted future position of the offset
// 用 Arrive 飞向"偏移点 + 队长速度×时间"(队长未来的偏移位置),fast 档。
  return Arrive(WorldOffsetPos + leader->Velocity() * LookAheadTime, fast);
}



//【宏 KEYDOWN】定义"按键是否按下"的检测:GetAsyncKeyState(键码) 返回的
// 高位为 1 表示该键当前被按下,与 0x8000 按位与后取真/假。
//for receiving keyboard input from user
// 键码(vk_code)如 VK_INSERT、'F' 等;?: 三元运算符 = "条件?真值:假值"。
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
//--------------------------------------------------------------------------------
//【RenderAids 渲染调试辅助】把各种行为的工作过程画出来:
//  参数面板(按 Insert/Delete 调最大推力、Home/End 调最大速度)、
//  转向力箭头、徘徊圆与目标点、检测盒、触须、路径、以及
//  分离/对齐/内聚权重的实时调节(S/X、A/Z、D/C 键)。
//--------------------------------------------------------------------------------
//----------------------------- RenderAids -------------------------------
//
//------------------------------------------------------------------------
// RenderAids 函数定义开始。
void SteeringBehavior::RenderAids( )
{ 
// 文字背景透明、颜色灰(参数面板文字)。
  
// 面板行号与行高:从第 0 行开始,每行 20 像素高。
  gdi->TransparentText();
  gdi->TextColor(Cgdi::grey);

// 按 Insert 键:最大推力每秒 +1000(实时调参);
  int NextSlot = 0; int SlotSize = 20;
// 按 Delete 键:最大推力每秒 -1000(不低于 0.2);

// 按 Home 键:最大速度每秒 +50;
  if (KEYDOWN(VK_INSERT)){m_pVehicle->SetMaxForce(m_pVehicle->MaxForce() + 1000.0f*m_pVehicle->TimeElapsed());} 
// 按 End 键:最大速度每秒 -50(不低于 0.2)。
  if (KEYDOWN(VK_DELETE)){if (m_pVehicle->MaxForce() > 0.2f) m_pVehicle->SetMaxForce(m_pVehicle->MaxForce() - 1000.0f*m_pVehicle->TimeElapsed());}
  if (KEYDOWN(VK_HOME)){m_pVehicle->SetMaxSpeed(m_pVehicle->MaxSpeed() + 50.0f*m_pVehicle->TimeElapsed());}
// 保护:推力、速度不允许为负(万一调过头就清零)。
  if (KEYDOWN(VK_END)){if (m_pVehicle->MaxSpeed() > 0.2f) m_pVehicle->SetMaxSpeed(m_pVehicle->MaxSpeed() - 50.0f*m_pVehicle->TimeElapsed());}

  if (m_pVehicle->MaxForce() < 0) m_pVehicle->SetMaxForce(0.0f);
// 头车(ID==0)才在面板上显示数值(避免 300 辆车都刷面板);
  if (m_pVehicle->MaxSpeed() < 0) m_pVehicle->SetMaxSpeed(0.0f);
  
  if (m_pVehicle->ID() == 0){ gdi->TextAtPos(5,NextSlot,"MaxForce(Ins/Del):"); gdi->TextAtPos(160,NextSlot,ttos(m_pVehicle->MaxForce()/Prm.SteeringForceTweaker)); NextSlot+=SlotSize;}
// 显示 MaxForce 与 MaxSpeed 两行参数。
  if (m_pVehicle->ID() == 0){ gdi->TextAtPos(5,NextSlot,"MaxSpeed(Home/End):"); gdi->TextAtPos(160,NextSlot,ttos(m_pVehicle->MaxSpeed()));NextSlot+=SlotSize;}

// ↓↓↓ 原文注释翻译:渲染转向力。
  //render the steering force
// 开着"显示转向力"时:
  if (m_pVehicle->World()->RenderSteeringForce())
  {  
// 红笔;把总力除以 Tweaker 再乘 VehicleScale(缩放到可见大小);
    gdi->RedPen();
    Vector2D F = (m_vSteeringForce / Prm.SteeringForceTweaker) * Prm.VehicleScale ;
// 从车位置画到"车位置 + 力",一条箭头线。
    gdi->Line(m_pVehicle->Pos(), m_pVehicle->Pos() + F);
  }

// ↓↓↓ 原文注释翻译:如果相关,渲染徘徊相关的信息。
  //render wander stuff if relevant
// 开着徘徊 且 开着"显示徘徊圆":
  if (On(wander) && m_pVehicle->World()->RenderWanderCircle())
  {    
// 按 F 键:抖动 +1/秒(限 0~100);按 V:抖动 -1/秒;
    if (KEYDOWN('F')){m_dWanderJitter+=1.0f*m_pVehicle->TimeElapsed(); Clamp(m_dWanderJitter, 0.0f, 100.0f);}
    if (KEYDOWN('V')){m_dWanderJitter-=1.0f*m_pVehicle->TimeElapsed(); Clamp(m_dWanderJitter, 0.0f, 100.0f );}
// 按 G 键:圆距离 +2/秒(限 0~50);按 B:圆距离 -2/秒;
    if (KEYDOWN('G')){m_dWanderDistance+=2.0f*m_pVehicle->TimeElapsed(); Clamp(m_dWanderDistance, 0.0f, 50.0f);}
    if (KEYDOWN('B')){m_dWanderDistance-=2.0f*m_pVehicle->TimeElapsed(); Clamp(m_dWanderDistance, 0.0f, 50.0f);}
// 按 H 键:圆半径 +2/秒(限 0~100);按 N:圆半径 -2/秒。
    if (KEYDOWN('H')){m_dWanderRadius+=2.0f*m_pVehicle->TimeElapsed(); Clamp(m_dWanderRadius, 0.0f, 100.0f);}
    if (KEYDOWN('N')){m_dWanderRadius-=2.0f*m_pVehicle->TimeElapsed(); Clamp(m_dWanderRadius, 0.0f, 100.0f);}

 
// 头车显示 Jitter/Distance/Radius 三行参数;
    if (m_pVehicle->ID() == 0){ gdi->TextAtPos(5,NextSlot, "Jitter(F/V): "); gdi->TextAtPos(160, NextSlot, ttos(m_dWanderJitter));NextSlot+=SlotSize;}
    if (m_pVehicle->ID() == 0) {gdi->TextAtPos(5,NextSlot,"Distance(G/B): "); gdi->TextAtPos(160, NextSlot, ttos(m_dWanderDistance));NextSlot+=SlotSize;}
    if (m_pVehicle->ID() == 0) {gdi->TextAtPos(5,NextSlot,"Radius(H/N): ");gdi->TextAtPos(160, NextSlot,  ttos(m_dWanderRadius));NextSlot+=SlotSize;}

    
    //calculate the center of the wander circle
// 徘徊圆圆心 = 车头前方 WanderDist×半径 处(变换到世界坐标);
    Vector2D m_vTCC = PointToWorldSpace(Vector2D(m_dWanderDistance*m_pVehicle->BRadius(), 0),
                                         m_pVehicle->Heading(),
                                         m_pVehicle->Side(),
                                         m_pVehicle->Pos());
    //draw the wander circle
// 绿笔、空心笔刷画徘徊圆;
    gdi->GreenPen();
    gdi->HollowBrush();
// 画圆:圆心 m_vTCC,半径 WanderRadius×车身半径。
    gdi->Circle(m_vTCC, m_dWanderRadius*m_pVehicle->BRadius()); 

    //draw the wander target
// 红笔画徘徊目标点:目标世界位置画个小圆(半径 3)。
    gdi->RedPen();
    gdi->Circle(PointToWorldSpace((m_vWanderTarget + Vector2D(m_dWanderDistance,0))*m_pVehicle->BRadius(),
                                  m_pVehicle->Heading(),
                                  m_pVehicle->Side(),
                                  m_pVehicle->Pos()), 3);                                  
  }

// ↓↓↓ 原文注释翻译:如果相关,渲染检测盒。
  //render the detection box if relevant
// 开着"显示检测盒":用灰色画避障的检测盒,并标红"盒内会撞上的障碍"。
  if (m_pVehicle->World()->RenderDetectionBox())
  {
// 灰笔;
    gdi->GreyPen();

    //a vertex buffer rqd for drawing the detection box
// 静态顶点缓冲(4 个点):只分配一次,复用。
    static std::vector<Vector2D> box(4);

// 检测盒长度(与避障算法同样的公式:速度越快越长);
    double length = Prm.MinDetectionBoxLength + 
                  (m_pVehicle->Speed()/m_pVehicle->MaxSpeed()) *
                  Prm.MinDetectionBoxLength;

    //verts for the detection box buffer
// 盒子的 4 个顶点(车头本地坐标):左侧上下、右侧上下;
    box[0] = Vector2D(0,m_pVehicle->BRadius());
    box[1] = Vector2D(length, m_pVehicle->BRadius());
    box[2] = Vector2D(length, -m_pVehicle->BRadius());
    box[3] = Vector2D(0, -m_pVehicle->BRadius());
 
  
// 没开平滑:按实时朝向变换盒子;
    if (!m_pVehicle->isSmoothingOn())
    {
// 画闭合的检测盒;
      box = WorldTransform(box,m_pVehicle->Pos(),m_pVehicle->Heading(),m_pVehicle->Side());
      gdi->ClosedShape(box);
    }
// 开了平滑:按平滑朝向变换盒子再画。
    else
    {
      box = WorldTransform(box,m_pVehicle->Pos(),m_pVehicle->SmoothedHeading(),m_pVehicle->SmoothedHeading().Perp());
      gdi->ClosedShape(box);
    } 


// ↓↓↓ 原文注释翻译:检测盒长度与车辆速度成正比(与避障算法相同的检测逻辑,
//    用来把"会撞上的"障碍标红)。
    //////////////////////////////////////////////////////////////////////////
    //the detection box length is proportional to the agent's velocity
// 计算检测盒长度;
  m_dDBoxLength = Prm.MinDetectionBoxLength + 
                  (m_pVehicle->Speed()/m_pVehicle->MaxSpeed()) *
                  Prm.MinDetectionBoxLength;

  //tag all obstacles within range of the box for processing
// 给盒内障碍打标记;
  m_pVehicle->World()->TagObstaclesWithinViewRange(m_pVehicle, m_dDBoxLength);

  //this will keep track of the closest intersecting obstacle (CIB)
// 找最近相交障碍(与 ObstacleAvoidance 相同的遍历逻辑,见上文注释);
  BaseGameEntity* ClosestIntersectingObstacle = NULL;
 
  //this will be used to track the distance to the CIB
  double DistToClosestIP = MaxDouble;

  //this will record the transformed local coordinates of the CIB
  Vector2D LocalPosOfClosestObstacle;

// 遍历障碍;
  std::vector<BaseGameEntity*>::const_iterator curOb = m_pVehicle->World()->Obstacles().begin();

  while(curOb != m_pVehicle->World()->Obstacles().end())
  {
    //if the obstacle has been tagged within range proceed
// 只处理打了标记的;
    if ((*curOb)->IsTagged())
    {
      //calculate this obstacle's position in local space
// 变换到车头本地坐标;
      Vector2D LocalPos = PointToLocalSpace((*curOb)->Pos(),
                                             m_pVehicle->Heading(),
                                             m_pVehicle->Side(),
                                             m_pVehicle->Pos());

      //if the local position has a negative x value then it must lay
      //behind the agent. (in which case it can be ignored)
// 在前方(x≥0)且横向够近的……
      if (LocalPos.x >= 0)
      {
        //if the distance from the x axis to the object's position is less
        //than its radius + half the width of the detection box then there
        //is a potential intersection.
// ……用粗红笔再画一遍检测盒(提示"这里有障碍要躲")。
        if (fabs(LocalPos.y) < ((*curOb)->BRadius() + m_pVehicle->BRadius()))
        {
          gdi->ThickRedPen();
          gdi->ClosedShape(box);        
        }
      }
    }

    ++curOb;
  }


/////////////////////////////////////////////////////
// 检测盒渲染结束。
  }

// ↓↓↓ 原文注释翻译:渲染避墙用的触须。
  //render the wall avoidnace feelers
// 开着避墙 且 开着"显示触须":
  if (On(wall_avoidance) && m_pVehicle->World()->RenderFeelers())
  {
// 橙笔;
    gdi->OrangePen();

// 从车位置到每根触须端点画线。
    for (unsigned int flr=0; flr<m_Feelers.size(); ++flr)
    {

      gdi->Line(m_pVehicle->Pos(), m_Feelers[flr]);
    }            
  }  

// ↓↓↓ 原文注释翻译:渲染路径信息。
  //render path info
// 开着沿路径 且 开着"显示路径":把路径画出来(见 Path::Render)。
  if (On(follow_path) && m_pVehicle->World()->RenderPath())
  {
    m_pPath->Render();
  }

  
// 开着分离:头车显示"分离权重(S/X 调)",并按 S/X 实时增减权重(限 0~50×Tweaker);
  if (On(separation))
  {
    if (m_pVehicle->ID() == 0){ gdi->TextAtPos(5, NextSlot, "Separation(S/X):");gdi->TextAtPos(160,NextSlot, ttos(m_dWeightSeparation/Prm.SteeringForceTweaker));NextSlot+=SlotSize;}

    if (KEYDOWN('S')){m_dWeightSeparation += 200*m_pVehicle->TimeElapsed(); Clamp(m_dWeightSeparation, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
    if (KEYDOWN('X')){m_dWeightSeparation -= 200*m_pVehicle->TimeElapsed();Clamp(m_dWeightSeparation, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
  }

// 开着对齐:头车显示"对齐权重(A/Z 调)",A/Z 实时增减;
  if (On(allignment))
  {
    if (m_pVehicle->ID() == 0) {gdi->TextAtPos(5, NextSlot, "Alignment(A/Z):"); gdi->TextAtPos(160, NextSlot, ttos(m_dWeightAlignment/Prm.SteeringForceTweaker));NextSlot+=SlotSize;}

    if (KEYDOWN('A')){m_dWeightAlignment += 200*m_pVehicle->TimeElapsed();Clamp(m_dWeightAlignment, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
    if (KEYDOWN('Z')){m_dWeightAlignment -= 200*m_pVehicle->TimeElapsed();Clamp(m_dWeightAlignment, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
  }

// 开着内聚:头车显示"内聚权重(D/C 调)",D/C 实时增减;
  if (On(cohesion))
  {
    if (m_pVehicle->ID() == 0) {gdi->TextAtPos(5, NextSlot, "Cohesion(D/C):"); gdi->TextAtPos(160, NextSlot, ttos(m_dWeightCohesion/Prm.SteeringForceTweaker));NextSlot+=SlotSize;}
    if (KEYDOWN('D')){m_dWeightCohesion += 200*m_pVehicle->TimeElapsed();Clamp(m_dWeightCohesion, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
    if (KEYDOWN('C')){m_dWeightCohesion -= 200*m_pVehicle->TimeElapsed();Clamp(m_dWeightCohesion, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
  }

// 开着沿路径:头车显示"路标点切换距离(D/C 调)",D/C 实时增减(限 0~400)。
  if (On(follow_path))
  { 
    double sd = sqrt(m_dWaypointSeekDistSq);
    if (m_pVehicle->ID() == 0){ gdi->TextAtPos(5, NextSlot, "SeekDistance(D/C):");gdi->TextAtPos(160, NextSlot,ttos(sd));NextSlot+=SlotSize;}
    
    if (KEYDOWN('D')){m_dWaypointSeekDistSq += 1.0;}
    if (KEYDOWN('C')){m_dWaypointSeekDistSq -= 1.0; Clamp(m_dWaypointSeekDistSq, 0.0f, 400.0f);}
  }  

}





