//==============================================================================================
//【文件说明】SteeringBehaviors.h —— 第三章的"灵魂":转向行为控制器
//
//【这个文件是干什么的?】
//  把"怎么转向"这件事封装成一个类。每个机器人(Vehicle)持有一个
//  SteeringBehavior 对象,它负责:
//    ① 维护一张"行为开关表"(m_iFlags 二进制位):想开哪个行为就置哪个位;
//    ② 实现十几个转向行为算法:追击/逃跑/到达/徘徊/躲避障碍/躲避墙壁/
//       跟随路径/拦截/隐藏/偏移追击/分离/对齐/内聚……
//    ③ 按三种方式之一把"开着的"行为产生的力合成为一股"总转向力",
//       交给 Vehicle 去驱动运动。
//
//【行为全景图:每个行为的"本能"一句话】
//   Seek(追击目标)      —— 朝目标方向全速冲;
//   Flee(逃离目标)      —— 反方向全速跑;
//   Arrive(到达)        —— 像 Seek 但接近目标时减速,平稳停下;
//   Wander(徘徊)        —— 漫无目的地随机游荡;
//   Pursuit(拦截)       —— 预测猎物未来位置,去"堵"它;
//   Evade(逃命)         —— 预测追兵未来位置,反向逃;
//   Interpose(插到中间)  —— 站到两个移动者连线中点;
//   Hide(躲藏)          —— 躲在障碍物后面,不让猎人看见;
//   OffsetPursuit(编队)  —— 保持与"队长"的相对位置(编队阵型);
//   FollowPath(沿路径)   —— 依次经过一串路标点;
//   ObstacleAvoidance(避障) —— 躲开眼前的圆形障碍物;
//   WallAvoidance(避墙)  —— 用"触须"探墙,提前转向;
//   Separation(分离)     —— 别和邻居挤在一起;
//   Alignment(对齐)      —— 与邻居朝向保持一致;
//   Cohesion(内聚)       —— 往邻居的"重心"靠拢;
//   (Separation+Alignment+Cohesion 三者组合 = 经典的"群体飞行 flocking"。)
//
//【与相关文件的关系】
//  Vehicle.h/.cpp    —— 本类的"雇主":Vehicle 每帧调 Calculate() 拿转向力;
//  GameWorld.h/.cpp  —— 提供世界数据(障碍物/墙/机器人列表)供行为取用;
//  Path.h            —— 路径对象(m_pPath),FollowPath 行为消费它;
//  ParamLoader.h     —— 宏 Prm:各行为的权重、检测距离等参数都从它读;
//  constants.h       —— 习惯性包含;
//  SteeringBehaviors.cpp —— 全部行为算法的具体实现(本文件的"另一半")。
//
//【C++ 小课堂:位标志(bit flags)】
//  每个行为对应 m_iFlags 里的一个二进制位(如 seek = 0x00002)。开/关行为
//  就是置 1/清零对应位(|= 置位、^= 翻转、& 测试)。一个 int 能同时记录
//  十几个开关,非常省内存——见下方 On()/xxxOn()/xxxOff() 的注释。
//==============================================================================================
#ifndef STEERINGBEHAVIORS_H
#define STEERINGBEHAVIORS_H
// #pragma warning (disable:4786)—— 告诉编译器:屏蔽警告 4786(标识符被
// 截断到 255 字符的提示)。这是老 VC++ 时代对付长模板名的惯例写法。
// #pragma 是"编译器指令",不改代码逻辑,只调编译器行为。
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   SteeringBehaviors.h
//
//  Desc:   class to encapsulate steering behaviors for a Vehicle
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// 包含 C++ 标准库 vector(动态数组):触须 m_Feelers 等用它存储。
#include <vector>
// 包含 Windows API 头文件:下面的 RenderAids() 要用 GetAsyncKeyState 等
// 键盘查询函数(宏 KEYDOWN 依赖它)。
#include <windows.h>
// 包含 C++ 标准库 string(字符串),习惯性包含。
#include <string>
// 包含 C++ 标准库 list(链表):SetPath 等接口的参数类型用。
#include <list>

// 包含 2D 向量工具(Common 目录):Vector2D 类型与运算。
#include "2d/Vector2D.h"
// 包含窗口常量(实际未直接用,作者习惯)。
#include "constants.h"
// 包含路径类:成员 m_pPath 的类型 Path 定义在这里。
#include "Path.h"
// 包含参数加载器:宏 Prm 在这里定义,构造函数用它读各行为权重。
#include "ParamLoader.h"


//【前置声明】下面四行只"报名字、不给内容":
//  class Vehicle;         —— 本类大量使用 Vehicle*,指针只需名字即可编译;
//  class CController;     —— 游戏控制器(本工程未用,保留声明);
//  class Wall2D;          —— 墙(Common 目录),WallAvoidance 的参数类型;
//  class BaseGameEntity;  —— 实体基类(重复声明一行是原作者笔误,保留原样)。
// 完整定义由实现文件 SteeringBehaviors.cpp 去包含。
class Vehicle;
class CController;
class Wall2D;
class BaseGameEntity;
class BaseGameEntity;




//--------------------------------------------------------------------------------
//【游荡行为相关常量】(写在类外,全文件可见)
//  WanderRad / WanderDist / WanderJitterPerSec / WaypointSeekDist ——
//  "徘徊"和"沿路径"行为用到的基础数值,直接写死成常量(不进 ini)。
//--------------------------------------------------------------------------------
//--------------------------- Constants ----------------------------------

// ↓↓↓ 原文注释翻译:徘徊行为中"约束圆"的半径。
//the radius of the constraining circle for the wander behavior
// 徘徊圆半径 WanderRad = 1.2:徘徊目标点落在一个小圆上,圆越大晃得越开。
const double WanderRad    = 1.2;
// ↓↓↓ 原文注释翻译:徘徊圆被投影在 agent 前方多远处。
//distance the wander circle is projected in front of the agent
// 徘徊圆距车头的距离 WanderDist = 2.0:圆放在车前方 2 个"车身半径"处。
const double WanderDist   = 2.0;
// ↓↓↓ 原文注释翻译:每帧沿圆的最大位移量(抖动幅度)。
//the maximum amount of displacement along the circle each frame
// 抖动速度 WanderJitterPerSec = 80:目标点在圆上"乱跳"的剧烈程度。
const double WanderJitterPerSec = 80.0;

// ↓↓↓ 原文注释翻译:用于路径跟随。
//used in path following
// 路标点切换距离 WaypointSeekDist = 20:离当前路标点 20 像素内就换下一个。
const double WaypointSeekDist   = 20;                                          



//------------------------------------------------------------------------

//【类定义开始】class SteeringBehavior —— 转向行为控制器。
// 先 public(公开枚举),再 private(私有数据与算法),最后再 public(对外接口)。
class SteeringBehavior
{
// public: —— 第一段公开区。
public:
  
//【公开枚举:三种"合力方式"】
//   enum summing_method —— 枚举(只能取下面三个值之一):
//     weighted_average —— 加权平均:所有开着的行为按权重相加,再截断;
//     prioritized     —— 优先级法:按重要程度逐个加,力用完了就停;
//     dithered        —— 抖动法:每帧按概率随机挑一个行为出力。
//   m_SummingMethod 记录当前选哪种(默认 prioritized,见 .cpp)。
  enum summing_method{weighted_average, prioritized, dithered};

// private: —— 私有区:下面是内部数据与算法,外界不可见。
private:

//【私有枚举:行为类型 = 一个二进制位】
//   每个枚举项是 16 进制数,依次是第 1、2、3……位为 1(0x00002 = 二进制
//   0000 0000 0000 0010,即第 1 位;0x10000 是第 16 位)。
//   这样"行为开关集合"就能用一个 int(m_iFlags)记录:某位是 1 = 该行为开着。
  enum behavior_type
  {
// none = 0x00000:什么都不开(全 0)。
    none               = 0x00000,
// seek(追击)= 第 1 位。
    seek               = 0x00002,
// flee(逃离)= 第 2 位。
    flee               = 0x00004,
// arrive(到达)= 第 3 位。
    arrive             = 0x00008,
// wander(徘徊)= 第 4 位。
    wander             = 0x00010,
// cohesion(内聚)= 第 5 位。
    cohesion           = 0x00020,
// separation(分离)= 第 6 位。
    separation         = 0x00040,
// allignment(对齐;原书拼写如此,保留)= 第 7 位。
    allignment         = 0x00080,
// obstacle_avoidance(避障)= 第 8 位。
    obstacle_avoidance = 0x00100,
// wall_avoidance(避墙)= 第 9 位。
    wall_avoidance     = 0x00200,
// follow_path(沿路径)= 第 10 位。
    follow_path        = 0x00400,
// pursuit(拦截)= 第 11 位。
    pursuit            = 0x00800,
// evade(逃命)= 第 12 位。
    evade              = 0x01000,
// interpose(插到中间)= 第 13 位。
    interpose          = 0x02000,
// hide(躲藏)= 第 14 位。
    hide               = 0x04000,
// flock(群体飞行)= 第 15 位(是"分离+对齐+内聚+徘徊"的组合宏,见 FlockingOn)。
    flock              = 0x08000,
// offset_pursuit(编队追击)= 第 16 位。
    offset_pursuit     = 0x10000,
  };

// private: —— 第二段私有区:数据成员。
private:

  
// ↓↓↓ 原文注释翻译:指向本实例"主人"(拥有者)的指针。
  //a pointer to the owner of this instance
// m_pVehicle —— 指向"我服务的那个机器人"。所有行为都要读取它的位置、
// 朝向、速度等来算力。p = pointer(指针)。
  Vehicle*     m_pVehicle;   
  
// ↓↓↓ 原文注释翻译:由所有被选中的行为共同产生的转向力。
  //the steering force created by the combined effect of all
  //the selected behaviors
// m_vSteeringForce —— 合成后的总转向力:Calculate() 算完就存在这里,
// Vehicle 的 Update 通过 Force() 取走。
  Vector2D    m_vSteeringForce;
 
// ↓↓↓ 原文注释翻译:这两个可以用来记录朋友、追兵或猎物(目标代理)。
  //these can be used to keep track of friends, pursuers, or prey
// m_pTargetAgent1 —— 目标代理 1(如拦截对象/逃命对象/躲藏对象)。
  Vehicle*     m_pTargetAgent1;
// m_pTargetAgent2 —— 目标代理 2(如 Interpose 插到"这两个人"中间)。
  Vehicle*     m_pTargetAgent2;

// ↓↓↓ 原文注释翻译:当前目标(位置)。
  //the current target
// m_vTarget —— 当前目标点(Seek/Flee/Arrive 等用,鼠标右键设定)。
  Vector2D    m_vTarget;

// ↓↓↓ 原文注释翻译:障碍躲避用的"检测盒"长度。
  //length of the 'detection box' utilized in obstacle avoidance
// m_dDBoxLength —— 避障检测盒长度:随速度动态变化(见 .cpp)。
  double                 m_dDBoxLength;


// ↓↓↓ 原文注释翻译:存"触须"的顶点缓冲(rqd = required 的缩写,即所需)。
// 触须(天线):避墙用的 3 根射线,详见 WallAvoidance 与 CreateFeelers。
  //a vertex buffer to contain the feelers rqd for wall avoidance  
// m_Feelers —— 3 根触须端点(向量)的容器:正中、偏左、偏右各一根。
  std::vector<Vector2D> m_Feelers;
  
// ↓↓↓ 原文注释翻译:墙壁检测用的"触须"长度。
  //the length of the 'feeler/s' used in wall detection
// m_dWallDetectionFeelerLength —— 触须长度(从 ini 读,默认 40)。
  double                 m_dWallDetectionFeelerLength;



// ↓↓↓ 原文注释翻译:徘徊目标点在徘徊圆上的当前位置(agent 正试图驶向的点)。
  //the current position on the wander circle the agent is
  //attempting to steer towards
// m_vWanderTarget —— 徘徊圆上的"目标点"(每帧随机抖动一下)。
  Vector2D     m_vWanderTarget; 

// ↓↓↓ 原文注释翻译:(上面已解释)——徘徊三件套:
  //explained above
// m_dWanderJitter —— 抖动幅度(圆上点每帧随机位移量);
  double        m_dWanderJitter;
// m_dWanderRadius —— 徘徊圆半径;
  double        m_dWanderRadius;
// m_dWanderDistance —— 圆距车头距离。
  double        m_dWanderDistance;


// ↓↓↓ 原文注释翻译:权重(乘数)。可以调节以改变各行为的强弱——例如把
//    群体飞行调成你想要的样子。
  //multipliers. These can be adjusted to effect strength of the  
// 下面一组 m_dWeightXxx 是各行为的"权重"(从 ini 读,再乘以 SteeringForceTweaker):
// 合成的总力 = Σ(行为力 × 权重)。权重越大,该行为"说话越有分量"。
  //appropriate behavior. Useful to get flocking the way you require
  //for example.
// 分离行为权重。
  double        m_dWeightSeparation;
// 内聚行为权重。
  double        m_dWeightCohesion;
// 对齐行为权重。
  double        m_dWeightAlignment;
// 徘徊行为权重。
  double        m_dWeightWander;
// 避障行为权重。
  double        m_dWeightObstacleAvoidance;
// 避墙行为权重。
  double        m_dWeightWallAvoidance;
// 追击行为权重。
  double        m_dWeightSeek;
// 逃离行为权重。
  double        m_dWeightFlee;
// 到达行为权重。
  double        m_dWeightArrive;
// 拦截行为权重。
  double        m_dWeightPursuit;
// 编队追击权重。
  double        m_dWeightOffsetPursuit;
// 插到中间权重。
  double        m_dWeightInterpose;
// 躲藏权重。
  double        m_dWeightHide;
// 逃命权重。
  double        m_dWeightEvade;
// 沿路径权重。
  double        m_dWeightFollowPath;

// ↓↓↓ 原文注释翻译:agent 能"看"多远。
  //how far the agent can 'see'
// m_dViewDistance —— 视野距离:超出它就不算邻居(默认 50)。
  double        m_dViewDistance;

// ↓↓↓ 原文注释翻译:指向当前路径的指针。
  //pointer to any current path
// m_pPath —— 当前路径对象(FollowPath 用;构造函数里 new 一个并开启循环)。
  Path*          m_pPath;

// ↓↓↓ 原文注释翻译:车辆距离当前路标点多远(平方)就开始寻找下一个路标点。
  //the distance (squared) a vehicle has to be from a path waypoint before
  //it starts seeking to the next waypoint
// m_dWaypointSeekDistSq —— 切换路标点的距离阈值(存平方,省开方)。
  double        m_dWaypointSeekDistSq;


// ↓↓↓ 原文注释翻译:编队/偏移追击用的偏移量。
  //any offset used for formations or offset pursuit
// m_vOffset —— 相对"队长"的偏移位置(如"左后方 2 个身位")。
  Vector2D     m_vOffset;



// ↓↓↓ 原文注释翻译:二进制标志,指示哪些行为当前处于激活状态。
  //binary flags to indicate whether or not a behavior should be active
// m_iFlags —— 行为开关集合:一个 int 的 16 个位分别管 16 个行为。
  int           m_iFlags;

  
// ↓↓↓ 原文注释翻译:Arrive(到达)用它决定车辆应多快地减速到目标。
  //Arrive makes use of these to determine how quickly a vehicle
  //should decelerate to its target
// 减速档位枚举:slow=3(最慢档,早早减速)/ normal=2 / fast=1(临近才刹)。
// 数值越大减速越早、停得越稳。
  enum Deceleration{slow = 3, normal = 2, fast = 1};

// ↓↓↓ 原文注释翻译:(默认值)
  //default
// m_Deceleration —— 当前减速档,默认 normal(适中)。
  Deceleration m_Deceleration;

// ↓↓↓ 原文注释翻译:是否使用"网格空间划分"(cell space partitioning)?
  //is cell space partitioning to be used or not?
// m_bCellSpaceOn —— 找邻居是否走网格加速(默认关,菜单可切换)。
  bool          m_bCellSpaceOn;
 
// ↓↓↓ 原文注释翻译:当前用哪种方法合成激活行为的力。
  //what type of method is used to sum any active behavior
// m_SummingMethod —— 合力方式(加权/优先级/抖动),默认 prioritized。
  summing_method  m_SummingMethod;


// ↓↓↓ 原文注释翻译:这个函数测试 m_iFlags 的某一位是否被置位(是否开着)。
  //this function tests if a specific bit of m_iFlags is set
//【On:查询行为开关】On(behavior_type bt) 返回布尔:
//   m_iFlags & bt —— 按位与:只有 bt 那一位是 1 时结果才非 0;
//   == bt —— 再和 bt 自身比:相等说明该位确实是 1(行为开着)。
//   例:On(seek) 为 true ⇔ seek 位为 1 ⇔ 追击行为开着。
  bool      On(behavior_type bt){return (m_iFlags & bt) == bt;}

// 声明:把 ForceToAdd 累加进 sf,但不得超过机器人最大推力(实现见 .cpp)。
  bool      AccumulateForce(Vector2D &sf, Vector2D ForceToAdd);

// ↓↓↓ 原文注释翻译:创建避墙行为使用的"天线(触须)"。
  //creates the antenna utilized by the wall avoidance behavior
// 声明:生成 3 根触须端点(实现见 .cpp)。
  void      CreateFeelers();



// 下面的大块注释是原作者的分区标记,意思:"行为声明从这里开始"。原样保留。
   /* .......................................................

                    BEGIN BEHAVIOR DECLARATIONS

      .......................................................*/


// ↓↓↓ 原文注释翻译:这个行为让 agent 朝目标位置移动。
  //this behavior moves the agent towards a target position
//【追击 Seek】朝目标位置移动的力(实现见 .cpp)。
  Vector2D Seek(Vector2D TargetPos);

// ↓↓↓ 原文注释翻译:这个行为返回一个让 agent 远离目标位置的力。
  //this behavior returns a vector that moves the agent away
  //from a target position
//【逃离 Flee】与 Seek 相反:远离目标。
  Vector2D Flee(Vector2D TargetPos);

// ↓↓↓ 原文注释翻译:与 seek 类似,但试图以"零速度"到达目标位置(平稳停下)。
  //this behavior is similar to seek but it attempts to arrive 
  //at the target position with a zero velocity
//【到达 Arrive】参数:目标位置 + 减速档位。
  Vector2D Arrive(Vector2D     TargetPos,
                  Deceleration deceleration);

// ↓↓↓ 原文注释翻译:这个行为预测 agent 在时间 T 后会到哪,并朝那个点追击以
//    拦截它。
  //this behavior predicts where an agent will be in time T and seeks
  //towards that point to intercept it.
//【拦截 Pursuit】预测猎物未来位置去堵截。
  Vector2D Pursuit(const Vehicle* agent);

// ↓↓↓ 原文注释翻译:这个行为保持与目标车辆"偏移方向"上的相对位置(编队)。
  //this behavior maintains a position, in the direction of offset
  //from the target vehicle
//【编队追击 OffsetPursuit】参数:队长 + 偏移向量。
  Vector2D OffsetPursuit(const Vehicle* agent, const Vector2D offset);

// ↓↓↓ 原文注释翻译:这个行为试图逃离一个追击者。
  //this behavior attempts to evade a pursuer
//【逃命 Evade】预测追兵未来位置再反向逃。
  Vector2D Evade(const Vehicle* agent);

// ↓↓↓ 原文注释翻译:这个行为让 agent 随机地四处游荡。
  //this behavior makes the agent wander about randomly
//【徘徊 Wander】漫无目的随机游走。
  Vector2D Wander();

// ↓↓↓ 原文注释翻译:这个行为返回一个力,试图让 agent 远离可能遇到的障碍物。
  //this returns a steering force which will attempt to keep the agent 
  //away from any obstacles it may encounter
//【避障 ObstacleAvoidance】参数:障碍物指针容器(只读)。
  Vector2D ObstacleAvoidance(const std::vector<BaseGameEntity*>& obstacles);

// ↓↓↓ 原文注释翻译:这个行为返回一个力,让 agent 远离可能遇到的墙壁。
  //this returns a steering force which will keep the agent away from any
  //walls it may encounter
//【避墙 WallAvoidance】参数:墙容器。
  Vector2D WallAvoidance(const std::vector<Wall2D> &walls);

  
// ↓↓↓ 原文注释翻译:给定一串 Vector2D,这个行为产生一个按顺序沿路标点
//    移动的力。
  //given a series of Vector2Ds, this method produces a force that will
  //move the agent along the waypoints in order
//【沿路径 FollowPath】依次经过路标点。
  Vector2D FollowPath();

// ↓↓↓ 原文注释翻译:这个行为产生一个力,把车辆驶向"两个移动 agent 连线
//    的中点"。
  //this results in a steering force that attempts to steer the vehicle
  //to the center of the vector connecting two moving agents.
//【插到中间 Interpose】站到两个目标之间。
  Vector2D Interpose(const Vehicle* VehicleA, const Vehicle* VehicleB);

// ↓↓↓ 原文注释翻译:给定一个要躲避的 agent 位置和一组 BaseGameEntity,
//    这个方法试图把一个障碍物放在自己和对手之间。
  //given another agent position to hide from and a list of BaseGameEntitys this
  //method attempts to put an obstacle between itself and its opponent
//【躲藏 Hide】参数:猎人 + 障碍物列表。
  Vector2D Hide(const Vehicle* hunter, const std::vector<BaseGameEntity*>& obstacles);


// ↓↓↓ 原文注释翻译:-- 群体行为 --//(Group Behaviors)
  // -- Group Behaviors -- //

//【内聚 Cohesion】参数:机器人容器(邻居列表)。
  Vector2D Cohesion(const std::vector<Vehicle*> &agents);
  
//【分离 Separation】参数:机器人容器。
  Vector2D Separation(const std::vector<Vehicle*> &agents);

//【对齐 Alignment】参数:机器人容器。
  Vector2D Alignment(const std::vector<Vehicle*> &agents);

// ↓↓↓ 原文注释翻译:下面三个与上面三个相同,但用"网格空间划分"来找邻居
//    (邻居只查自己格子附近的,而不是全列表)。
  //the following three are the same as above but they use cell-space
  //partitioning to find the neighbors
// 网格版内聚。
  Vector2D CohesionPlus(const std::vector<Vehicle*> &agents);
// 网格版分离。
  Vector2D SeparationPlus(const std::vector<Vehicle*> &agents);
// 网格版对齐。
  Vector2D AlignmentPlus(const std::vector<Vehicle*> &agents);

// 原作者分区标记:"行为声明到此结束"。原样保留。
    /* .......................................................

                       END BEHAVIOR DECLARATIONS

      .......................................................*/

// ↓↓↓ 原文注释翻译:计算并合成所有激活行为的转向力。
  //calculates and sums the steering forces from any active behaviors
// 三种合力算法的声明:
//   CalculateWeightedSum() —— 加权平均法;
  Vector2D CalculateWeightedSum();
//   CalculatePrioritized() —— 优先级法;
  Vector2D CalculatePrioritized();
//   CalculateDithered() —— 抖动法。
  Vector2D CalculateDithered();

// ↓↓↓ 原文注释翻译:Hide 的辅助方法:返回一个位于障碍物"另一侧"(背对追兵)
//    的位置。
  //helper method for Hide. Returns a position located on the other
  //side of an obstacle to the pursuer
// 计算藏身点的辅助函数(实现见 .cpp)。
  Vector2D GetHidingPosition(const Vector2D& posOb,
                              const double     radiusOb,
                              const Vector2D& posHunter);



  
  
// public: —— 第三段公开区:对外接口。
public:

// 构造函数:绑定一个 Vehicle 作为"主人"(实现见 .cpp)。
  SteeringBehavior(Vehicle* agent);

// 虚析构(实现见 .cpp:释放路径对象)。
  virtual ~SteeringBehavior();

// ↓↓↓ 原文注释翻译:计算并合成所有激活行为的转向力(对外入口)。
  //calculates and sums the steering forces from any active behaviors
//【Calculate:对外总入口】Vehicle::Update 每帧调用它,拿总转向力(见 .cpp)。
  Vector2D Calculate();

// ↓↓↓ 原文注释翻译:计算转向力中与车辆朝向"平行"的分量(前进/后退分量)。
  //calculates the component of the steering force that is parallel
  //with the vehicle heading
// ForwardComponent():朝向点积转向力 = 沿车头方向的分量(实现见 .cpp)。
  double    ForwardComponent();

// ↓↓↓ 原文注释翻译:计算转向力中与车辆朝向"垂直"的分量(侧向分量)。
// (原注释 perpendicuar 是 perpendicular 的笔误,保留原样。)
  //calculates the component of the steering force that is perpendicuar
  //with the vehicle heading
// SideComponent():侧向点积转向力 = 横向分量。
  double    SideComponent();



// ↓↓↓ 原文注释翻译:渲染可视化辅助与信息,方便观察每个行为是怎么算出来的。
  //renders visual aids and info for seeing how each behavior is
  //calculated
// RenderAids():画调试信息(触须、检测盒、徘徊圆等,见 .cpp)。
  void      RenderAids();

// 设置目标点(鼠标右键点击的位置)。
  void      SetTarget(const Vector2D t){m_vTarget = t;}

// 设置目标代理 1(指针)。
  void      SetTargetAgent1(Vehicle* Agent){m_pTargetAgent1 = Agent;}
// 设置目标代理 2(指针)。
  void      SetTargetAgent2(Vehicle* Agent){m_pTargetAgent2 = Agent;}

// 设置编队偏移向量。
  void      SetOffset(const Vector2D offset){m_vOffset = offset;}
// 读回编队偏移向量。
  Vector2D  GetOffset()const{return m_vOffset;}

// 用一串新路标点替换当前路径(转交给 m_pPath 的 Set)。
  void      SetPath(std::list<Vector2D> new_path){m_pPath->Set(new_path);}
// 生成随机路径:参数是路标点数 + 矩形范围(注意参数名 mx/my/cx/cy 与
// 实际含义 MinX/MinY/MaxX/MaxY 对应,原书命名如此)。const:不修改本对象。
  void      CreateRandomPath(int num_waypoints, int mx, int my, int cx, int cy)const
            {m_pPath->CreateRandomPath(num_waypoints, mx, my, cx, cy);}

// Force():读回当前总转向力(Vehicle 的 Update 取走它)。
  Vector2D Force()const{return m_vSteeringForce;}

// 开关"网格空间划分"(菜单切换用)。
  void      ToggleSpacePartitioningOnOff(){m_bCellSpaceOn = !m_bCellSpaceOn;}
// 查询网格划分是否开启。
  bool      isSpacePartitioningOn()const{return m_bCellSpaceOn;}

// 设置合力方式(加权/优先级/抖动)。
  void      SetSummingMethod(summing_method sm){m_SummingMethod = sm;}


//【行为开关组:On 系列】|=(按位或)把对应位置 1 = 打开该行为。
//   例:FleeOn() → m_iFlags |= flee → flee 位变 1 → 逃离行为生效。
  void FleeOn(){m_iFlags |= flee;}
// 打开追击。
  void SeekOn(){m_iFlags |= seek;}
// 打开到达。
  void ArriveOn(){m_iFlags |= arrive;}
// 打开徘徊。
  void WanderOn(){m_iFlags |= wander;}
// 打开拦截,并记下拦截目标(同时赋值 m_pTargetAgent1)。
  void PursuitOn(Vehicle* v){m_iFlags |= pursuit; m_pTargetAgent1 = v;}
// 打开逃命,并记下要逃的对象。
  void EvadeOn(Vehicle* v){m_iFlags |= evade; m_pTargetAgent1 = v;}
// 打开内聚。
  void CohesionOn(){m_iFlags |= cohesion;}
// 打开分离。
  void SeparationOn(){m_iFlags |= separation;}
// 打开对齐(注意枚举名是 allignment,原书拼写如此)。
  void AlignmentOn(){m_iFlags |= allignment;}
// 打开避障。
  void ObstacleAvoidanceOn(){m_iFlags |= obstacle_avoidance;}
// 打开避墙。
  void WallAvoidanceOn(){m_iFlags |= wall_avoidance;}
// 打开沿路径。
  void FollowPathOn(){m_iFlags |= follow_path;}
// 打开插到中间,并记下两个目标代理。
  void InterposeOn(Vehicle* v1, Vehicle* v2){m_iFlags |= interpose; m_pTargetAgent1 = v1; m_pTargetAgent2 = v2;}
// 打开躲藏,记下猎人。
  void HideOn(Vehicle* v){m_iFlags |= hide; m_pTargetAgent1 = v;}
// 打开编队追击:记下队长和偏移量。
  void OffsetPursuitOn(Vehicle* v1, const Vector2D offset){m_iFlags |= offset_pursuit; m_vOffset = offset; m_pTargetAgent1 = v1;}  
// 一键"群体飞行":内聚+对齐+分离+徘徊 四个行为同时打开。
// (这是本演示程序的默认行为,见 GameWorld.cpp。)
  void FlockingOn(){CohesionOn(); AlignmentOn(); SeparationOn(); WanderOn();}

//【行为开关组:Off 系列】先确认开着(On(flee) 为真),再 ^=(按位异或)把该位
// 翻转成 0 = 关闭。例:On(flee) 为真时 m_iFlags ^= flee 把 flee 位清 0。
  void FleeOff()  {if(On(flee))   m_iFlags ^=flee;}
// 关闭追击。
  void SeekOff()  {if(On(seek))   m_iFlags ^=seek;}
// 关闭到达。
  void ArriveOff(){if(On(arrive)) m_iFlags ^=arrive;}
// 关闭徘徊。
  void WanderOff(){if(On(wander)) m_iFlags ^=wander;}
// 关闭拦截。
  void PursuitOff(){if(On(pursuit)) m_iFlags ^=pursuit;}
// 关闭逃命。
  void EvadeOff(){if(On(evade)) m_iFlags ^=evade;}
// 关闭内聚。
  void CohesionOff(){if(On(cohesion)) m_iFlags ^=cohesion;}
// 关闭分离。
  void SeparationOff(){if(On(separation)) m_iFlags ^=separation;}
// 关闭对齐。
  void AlignmentOff(){if(On(allignment)) m_iFlags ^=allignment;}
// 关闭避障。
  void ObstacleAvoidanceOff(){if(On(obstacle_avoidance)) m_iFlags ^=obstacle_avoidance;}
// 关闭避墙。
  void WallAvoidanceOff(){if(On(wall_avoidance)) m_iFlags ^=wall_avoidance;}
// 关闭沿路径。
  void FollowPathOff(){if(On(follow_path)) m_iFlags ^=follow_path;}
// 关闭插到中间。
  void InterposeOff(){if(On(interpose)) m_iFlags ^=interpose;}
// 关闭躲藏。
  void HideOff(){if(On(hide)) m_iFlags ^=hide;}
// 关闭编队追击。
  void OffsetPursuitOff(){if(On(offset_pursuit)) m_iFlags ^=offset_pursuit;}
// 一键关闭群体飞行(四个行为全关)。
  void FlockingOff(){CohesionOff(); AlignmentOff(); SeparationOff(); WanderOff();}

//【查询组:isXxxOn】返回布尔,询问某行为是否开着(= On(xxx) 的包装)。
  bool isFleeOn(){return On(flee);}
// 逃离开着吗?
  bool isSeekOn(){return On(seek);}
// 追击开着吗?
  bool isArriveOn(){return On(arrive);}
// 到达开着吗?
  bool isWanderOn(){return On(wander);}
// 徘徊开着吗?
  bool isPursuitOn(){return On(pursuit);}
// 拦截开着吗?
  bool isEvadeOn(){return On(evade);}
// 逃命开着吗?
  bool isCohesionOn(){return On(cohesion);}
// 内聚开着吗?
  bool isSeparationOn(){return On(separation);}
// 分离开着吗?
  bool isAlignmentOn(){return On(allignment);}
// 对齐开着吗?
  bool isObstacleAvoidanceOn(){return On(obstacle_avoidance);}
// 避障开着吗?
  bool isWallAvoidanceOn(){return On(wall_avoidance);}
// 避墙开着吗?
  bool isFollowPathOn(){return On(follow_path);}
// 沿路径开着吗?
  bool isInterposeOn(){return On(interpose);}
// 插到中间开着吗?
  bool isHideOn(){return On(hide);}
// 躲藏开着吗?
  bool isOffsetPursuitOn(){return On(offset_pursuit);}
// 编队追击开着吗?

// 读检测盒长度(RenderAids 画检测盒时用)。
  double DBoxLength()const{return m_dDBoxLength;}
// 读触须端点列表(RenderAids 画触须时用)。const 引用:只读借用,不拷贝。
  const std::vector<Vector2D>& GetFeelers()const{return m_Feelers;}
  
// 读徘徊抖动值(调试面板显示用)。
  double WanderJitter()const{return m_dWanderJitter;}
// 读徘徊圆距离。
  double WanderDistance()const{return m_dWanderDistance;}
// 读徘徊圆半径。
  double WanderRadius()const{return m_dWanderRadius;}

// 读分离权重(调试面板显示用)。
  double SeparationWeight()const{return m_dWeightSeparation;}
// 读对齐权重。
  double AlignmentWeight()const{return m_dWeightAlignment;}
// 读内聚权重。
  double CohesionWeight()const{return m_dWeightCohesion;}

// 类定义结束。
};




#endif