//==============================================================================================
//【文件说明】SoccerBall.cpp —— 足球的"物理引擎"(SoccerBall 类的实现)
//
//【这个文件是干什么的?】
//  足球在场上怎么飞,全由本文件说了算:被踢(Kick)、每帧运动(Update)、
//  撞墙反弹(TestCollisionWithWalls)、预测未来位置(TimeToCoverDistance /
//  FuturePosition,球员算"什么时候传球能接到"就靠它)、画出来(Render)。
//  另外文件开头还有一个"全局函数"AddNoiseToKick:给射门加一点随机误差,
//  让球员的脚法不是 100% 精准,比赛才真实。
//
//【谁在使用本文件定义的函数?】
//  SoccerPitch.cpp —— 每帧调用 ball->Update() 和 ball->Render();
//  PlayerBase.cpp / FieldPlayer.cpp / FieldPlayerStates.cpp —— 射门、传球时
//       调用 SoccerBall::Kick()、TimeToCoverDistance()、FuturePosition()、AddNoiseToKick();
//  SteeringBehaviors.cpp —— 追球/踢球时也要预测球的位置。
//  (它们包含的是头文件 SoccerBall.h;本 .cpp 是这些函数的"实现"。)
//
//【本文件包含了谁?】
//  "SoccerBall.h"        —— 足球类自己的声明(成员变量/函数签名);
//  "2D/geometry.h"       —— 2D 几何工具(碰撞、线段相交等);
//  "Debug/DebugConsole.h"—— 调试控制台(Common\Debug\ 目录,本项目自研);
//  "misc/Cgdi.h"         —— 绘图工具单例 gdi(Common\misc\ 目录);
//  "ParamLoader.h"       —— 全局参数单例 Prm(本章比赛参数的唯一来源);
//  "2D/Wall2D.h"         —— 边界墙类。
//
//【C++ 小课堂:自由函数 vs 成员函数】
//  AddNoiseToKick 是"自由函数"(不属于任何类,直接写函数名调用);
//  其余都是"SoccerBall::"开头的成员函数(属于足球对象,用 ball-> 调用)。
//==============================================================================================
//--------------------------------------------------------------------------------
// #include:把本文件需要的"零件说明书"先拿进来(详见文件头【本文件包含了谁?】)。
//--------------------------------------------------------------------------------
#include "SoccerBall.h"
#include "2D/geometry.h"
#include "Debug/DebugConsole.h"
#include "misc/Cgdi.h"
#include "ParamLoader.h"
#include "2D/Wall2D.h"


//----------------------------- AddNoiseToKick --------------------------------
//
//  this can be used to vary the accuracy of a player's kick. Just call it 
//  prior to kicking the ball using the ball's position and the ball target as
//  parameters.
//-----------------------------------------------------------------------------
// ↓↓↓ 原文翻译【AddNoiseToKick —— 给射门加随机噪声】:
//   这个函数用来改变球员射门的精准度:在踢球前调用它,把球的当前位置
//   和目标位置作为参数传入,它返回一个被加了随机偏差的新目标点。
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 自由函数:计算一个"带随机误差的射门目标点"。
//   Vector2D    —— 返回类型:一个 2D 向量(坐标点);
//   BallPos     —— 球当前的位置;
//   BallTarget  —— 球员原本想踢到的目标点。
// 它不属于任何类,直接用函数名调用(与下面 SoccerBall:: 开头的成员函数不同)。
//--------------------------------------------------------------------------------
Vector2D AddNoiseToKick(Vector2D BallPos, Vector2D BallTarget)
{

  // 随机偏转角:误差大小由"球员踢球不准的程度"决定。
  //   Pi                  :圆周率 3.14159(定义在 Common\misc\utils.h);
  //   Prm                 :全局参数单例的宏,定义在 ParamLoader.h 第 23 行
  //                        (#define Prm (*ParamLoader::Instance()));
  //   PlayerKickingAccuracy:踢球精准度(0~1,越大越准);
  //   RandomClamped()     :返回 [-1, 1] 之间的随机数(定义在 utils.h)。
  // 精准度越高误差越小:准确度=1 → 角度=0,完全不偏。
  double displacement = (Pi - Pi*Prm.PlayerKickingAccuracy) * RandomClamped();

  // 从球指向目标点的向量 = 目标点坐标 - 球坐标(向量减法)。
  Vector2D toTarget = BallTarget - BallPos;

  // 把 toTarget 向量绕原点旋转 displacement 弧度(给方向加上随机偏转)。
  Vec2DRotateAroundOrigin(toTarget, displacement);

  // 旋转后的向量 + 球的位置 = 最终"带误差的射门目标点"。
  return toTarget + BallPos;
}

  

//-------------------------- Kick ----------------------------------------
//                                                                        
//  applys a force to the ball in the direction of heading. Truncates
//  the new velocity to make sure it doesn't exceed the max allowable.
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Kick —— 踢球】:
//   沿 direction(方向)给球施加一个力,并把新速度截断,
//   确保它不超过最大允许值。
//----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 成员函数:踢球。direction = 踢球方向,force = 踢球力度(越大球越快)。
// 本函数只设速度,不设位置——球从哪出发由调用者决定。
//--------------------------------------------------------------------------------
void SoccerBall::Kick(Vector2D direction, double force)
{  
  //ensure direction is normalized
  //(原文注释:确保方向向量是归一化的)

  // Normalize():Vector2D 的成员函数,把向量长度缩成 1(方向不变)。
  direction.Normalize();
  
  //calculate the acceleration
  //(原文注释:计算加速度)

  // 加速度 = 力 ÷ 质量(牛顿第二定律 F = ma → a = F/m)。
  // (direction * force):向量 × 标量 = 把向量按力度拉长;
  // m_dMass:球的质量(构造时设定)。
  Vector2D acceleration = (direction * force) / m_dMass;

  //update the velocity
  //(原文注释:更新速度)

  // 把速度直接设为算出的加速度(本帧"起脚瞬间"的速度)。
  m_vVelocity = acceleration;
}

//----------------------------- Update -----------------------------------
//
//  updates the ball physics, tests for any collisions and adjusts
//  the ball's velocity accordingly
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Update —— 每帧更新】:
//   更新球的物理状态:测试是否撞墙,并相应调整球的速度。
//----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 成员函数:每帧调用一次,推进球的运动(撞墙检测 + 摩擦处理 + 位移)。
//--------------------------------------------------------------------------------
void SoccerBall::Update()
{
  //keep a record of the old position so the goal::scored method
  //can utilize it for goal testing
  //(原文注释:记下旧位置,供 Goal::Scored 做进球判定用)

  // 把当前帧的"起点位置"存档——下一帧它就是"上一帧位置"。
  m_vOldPos = m_vPosition;

      //Test for collisions
  //(原文注释:测试碰撞)

  // 与球场边界墙做碰撞检测,必要时反弹(实现见本文件下面)。
    TestCollisionWithWalls(m_PitchBoundary);

  //Simulate Prm.Friction. Make sure the speed is positive 
  //first though
  //(原文注释:模拟 Prm.Friction 摩擦力。先确保速度是正的)

  // LengthSq():速度长度的平方(勾股定理 |v|²,用平方比较省去开方)。
  // 条件:速度还足够大(大于摩擦阈值)才做摩擦处理;球快停时不再动它,
  // 免得它"原地来回抖"。
  if (m_vVelocity.LengthSq() > Prm.Friction * Prm.Friction)
  {
    // 把"速度方向 × 摩擦值"这一小段量叠加到速度上。
    // Friction 是本章"地面摩擦"的简化参数(数值由参数文件经 ParamLoader
    // 读入,见 ParamLoader.h);配合上面的阈值判断,当 Friction 取负值时
    // 即表现为逐帧减速,直到速度低于阈值后不再处理。
    m_vVelocity += Vec2DNormalize(m_vVelocity) * Prm.Friction;

    // 位置按当前速度前进一步(速度单位:像素/帧)。
    m_vPosition += m_vVelocity;



    //update heading
    //(原文注释:更新朝向)

    // 朝向 = 速度方向(球往哪飞,就"面向"哪)。
    m_vHeading = Vec2DNormalize(m_vVelocity);
  }   
}

//---------------------- TimeToCoverDistance -----------------------------
//
//  Given a force and a distance to cover given by two vectors, this
//  method calculates how long it will take the ball to travel between
//  the two points
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【TimeToCoverDistance —— 到达某点所需时间】:
//   给定一个力,和由两个向量点(起点 A、终点 B)确定的距离,
//   这个函数计算球从 A 飞到 B 需要多少时间。
//----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 成员函数:预测"用 force 的力度踢球,球从 A 飞到 B 需要多少帧"。
// 球员传球前用它算"这脚球多久能到",好决定跑位时机。
// 返回 -1.0 表示:以这个力度根本踢不到 B(球会中途停下)。
//--------------------------------------------------------------------------------
double SoccerBall::TimeToCoverDistance(Vector2D A,
                                      Vector2D B,
                                      double force)const
{
  //this will be the velocity of the ball in the next time step *if*
  //the player was to make the pass. 
  //(原文注释:这是球在下一个时间步的速度——如果球员传出这一脚的话)

  // 初速度 = 力 ÷ 质量(起脚第一帧的速度)。
  double speed = force / m_dMass;

  //calculate the velocity at B using the equation
  //
  //  v^2 = u^2 + 2as
  //

  //first calculate s (the distance between the two positions)
  // ↓↓↓ 上面两段英文注释的翻译:
  //   "calculate the velocity at B using the equation" —— 用下面的公式算球到达
  //     B 点时的速度:v² = u² + 2as(匀减速运动的速度-位移公式);
  //   "first calculate s (the distance between the two positions)" —— 先算 s(两点距离)。

  // Vec2DDistance(A, B):求两点距离(定义在 2D\geometry.h)。
  double DistanceToCover =  Vec2DDistance(A, B);

  // 匀减速运动公式 v² = u² + 2as 的"中间量":
  //   speed*speed          = u²(初速度平方);
  //   2.0*距离*Prm.Friction = 2as(Friction 充当加速度 a);
  // 算出的 term = 球到达 B 点时的"速度平方 v²"。
  double term = speed*speed + 2.0*DistanceToCover*Prm.Friction;

  //if  (u^2 + 2as) is negative it means the ball cannot reach point B.
  //(原文注释:如果 (u² + 2as) 是负数,说明球到不了 B 点)

  // 速度平方为负 = 无实数解 = 球半路停下。返回 -1 表示"这脚踢不到"。
  if (term <= 0.0) return -1.0;

  // sqrt():开平方(数学库函数)。v = 球到达 B 点时的速度。
  double v = sqrt(term);

  //it IS possible for the ball to reach B and we know its speed when it
  //gets there, so now it's easy to calculate the time using the equation
  //
  //    t = v-u
  //        ---
  //         a
  //
  // ↓↓↓ 上面英文注释的翻译:球确实能到 B,而且我们知道它到时的速度;
  //   于是用公式 t = (v-u)/a 算时间(时间 = (末速度 - 初速度) ÷ 加速度)。

  // 时间 = (末速度 - 初速度) ÷ 摩擦加速度。单位:帧。
  return (v-speed)/Prm.Friction;
}

//--------------------- FuturePosition -----------------------------------
//
//  given a time this method returns the ball position at that time in the
//  future
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【FuturePosition —— 未来位置】:
//   给定一个时间,返回球在该时间之后所处的位置。
//----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 成员函数:预测"time 帧之后球会在哪"。传球配合时,球员要算
// "球过来时我应该跑到哪"。
//--------------------------------------------------------------------------------
Vector2D SoccerBall::FuturePosition(double time)const
{
  //using the equation s = ut + 1/2at^2, where s = distance, a = friction
  //u=start velocity

  //calculate the ut term, which is a vector
  // ↓↓↓ 上面英文注释的翻译:用公式 s = ut + ½at² 计算,其中 s=距离,
  //     a=摩擦(加速度),u=初速度。

  //(原文注释:先算 ut 项,它是一个向量)

  // 匀速部分:初速度 × 时间 = 若无摩擦,time 帧后球飞出的距离(向量)。
  Vector2D ut = m_vVelocity * time;

  //calculate the 1/2at^2 term, which is scalar
  //(原文注释:算 ½at² 项,它是一个标量)

  // 减速部分:½ × 摩擦 × 时间²(Friction 充当加速度 a)。
  double half_a_t_squared = 0.5 * Prm.Friction * time * time;

  //turn the scalar quantity into a vector by multiplying the value with
  //the normalized velocity vector (because that gives the direction)
  //(原文注释:把这个标量乘上归一化速度向量,变成向量——因为那给出了方向)

  // 把"减速距离"这个标量沿速度方向铺开,变成向量(减速方向与运动方向相反)。
  Vector2D ScalarToVector = half_a_t_squared * Vec2DNormalize(m_vVelocity);

  //the predicted position is the balls position plus these two terms
  //(原文注释:预测位置 = 球当前位置 + 这两项)

  // 最终位置 = 现在位置 + 匀速部分 + 减速部分。
  return Pos() + ut + ScalarToVector;
}


//----------------------------- Render -----------------------------------
//
//  Renders the ball
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Render —— 渲染】:
//   把球画出来。
//----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 成员函数:把球画到屏幕上。
//--------------------------------------------------------------------------------
void SoccerBall::Render()
{
  // gdi:全局绘图对象(单例,定义在 Common\misc\Cgdi.h,全程序只有一份)。
  // BlackBrush():把画刷设成黑色——接下来画的圆就用黑色填充。
  gdi->BlackBrush();

  // Circle(圆心, 半径):画一个实心圆。m_dBoundingRadius = 球的半径。
  gdi->Circle(m_vPosition, m_dBoundingRadius);

  // ↓↓↓ 下面这段 /* ... */ 是被原作者"注释掉"的调试代码(不参与编译):
  // 用绿色画刷把截球预测点(IPPoints)画成小圆点;暂时不需要,先雪藏。
  /*
  gdi->GreenBrush();
  for (int i=0; i<IPPoints.size(); ++i)
  {
    gdi->Circle(IPPoints[i], 3);
  }
  */
}


//----------------------- TestCollisionWithWalls -------------------------
//
// ↓↓↓(原文此处只有分隔线,没有说明文字)本函数的作用见下面的中文说明。
//--------------------------------------------------------------------------------
// 成员函数:与四周的墙逐条做碰撞检测;若球"将要撞上"某面墙,就反弹速度。
// 步骤:① 算球面上最靠近墙的点 → ② 求它与墙面的交点 →
//       ③ 确认交点落在墙线段内 → ④ 确认本帧能飞到 → ⑤ 取最近的墙反弹。
//--------------------------------------------------------------------------------
void SoccerBall::TestCollisionWithWalls(const std::vector<Wall2D>& walls)
{  
  //test ball against each wall, find out which is closest
  //(原文注释:逐条测试球与每面墙,找出最近的那面)

  // 记录"最近碰撞墙"的编号;-1 表示目前还没撞到任何墙。
  int idxClosest = -1;

  // 速度方向(归一化):后面用它代替速度向量做几何计算。
  Vector2D VelNormal = Vec2DNormalize(m_vVelocity);

  // 两个 2D 向量变量:IntersectionPoint = 球与墙的交点;
  // CollisionPoint = 记录"最近的那次"交点。
  Vector2D IntersectionPoint, CollisionPoint;

  // 目前找到的最近距离(平方)。MaxFloat:float 能表示的最大值
  // (定义在 Common\misc\utils.h)——初始当"无穷大",任何真实距离都比它小。
  double DistToIntersection = MaxFloat;

  //iterate through each wall and calculate if the ball intersects.
  //If it does then store the index into the closest intersecting wall
  //(原文注释:遍历每一面墙,计算球是否与之相交;若是,记下最近的相交墙编号)

  // for 循环:逐面墙处理,下标 w 从 0 数到 walls.size()-1。
  // unsigned int:无符号整数(不为负);++w 每轮结束后自增 1。
  for (unsigned int w=0; w<walls.size(); ++w)
  {
    //assuming a collision if the ball continued on its current heading 
    //calculate the point on the ball that would hit the wall. This is 
    //simply the wall's normal(inversed) multiplied by the ball's radius
    //and added to the balls center (its position)
  //(原文注释:假设球沿当前方向继续飞会撞墙,计算球上会撞到墙的那个点:
  //           即墙法线(反向)× 球半径,再加到球心上)

  // 球面上"最靠近这面墙"的点 = 球心位置 - 墙法线 × 球半径。
  // Normal():墙的法线(垂直墙面、朝外的单位向量);BRadius():球半径。
    Vector2D ThisCollisionPoint = Pos() - (walls[w].Normal() * BRadius());

    //calculate exactly where the collision point will hit the plane    
  //(原文注释:精确计算碰撞点会撞在平面上的哪里)

  // WhereIsPoint(点, 直线起点, 直线法线):判断点在直线的哪一侧,
  // 返回 plane_backside(背面)/plane_frontside(正面)/plane_onplane(线上)。
  // 若碰撞点在墙的"背面",说明球在墙外侧,要用墙法线方向去求交点。
    if (WhereIsPoint(ThisCollisionPoint,
                     walls[w].From(),
                     walls[w].Normal()) == plane_backside)
    {
      // DistanceToRayPlaneIntersection(点, 射线起点, 平面上的点, 平面法线):
      // 求"从该点沿某方向发出的射线"与平面的交点距离。
      // (背面分支:射线方向 = 墙的法线方向。)
      double DistToWall = DistanceToRayPlaneIntersection(ThisCollisionPoint,
                                                         walls[w].Normal(),
                                                         walls[w].From(),
                                                         walls[w].Normal());

      // 交点 = 球面碰撞点 + 距离 × 墙法线方向。
      IntersectionPoint = ThisCollisionPoint + (DistToWall * walls[w].Normal());
      
    }

    // 球在墙的"正面"(球场内侧):改用速度方向求交点。
    else
    {
      // (正面分支:射线方向 = 速度方向 VelNormal,公式与背面分支相同。)
      double DistToWall = DistanceToRayPlaneIntersection(ThisCollisionPoint,
                                                         VelNormal,
                                                         walls[w].From(),
                                                         walls[w].Normal());

      // 交点 = 球面碰撞点 + 距离 × 速度方向。
      IntersectionPoint = ThisCollisionPoint + (DistToWall * VelNormal);
    }
    
    //check to make sure the intersection point is actually on the line
    //segment
  //(原文注释:确认交点确实落在墙的线段范围上)

  // 标志:交点是否真的在"线段"上(墙是线段,不是无限长的直线)。
    bool OnLineSegment = false;

    // 判断"球面碰撞点前后 20 像素"与墙线段是否相交:
    // 相交 → 说明球会路过这面墙的范围内,OnLineSegment = true。
    if (LineIntersection2D(walls[w].From(), 
                           walls[w].To(),
                           ThisCollisionPoint - walls[w].Normal()*20.0,
                           ThisCollisionPoint + walls[w].Normal()*20.0))
    {

      // 在墙线段范围内,标记为真。
      OnLineSegment = true;                                               
    }

  
                                                                          //Note, there is no test for collision with the end of a line segment
    // ↑ 上面那行英文注释的翻译:注意,这里没有对"线段端点"做碰撞测试——
    // 球可能从墙的两端"漏"过去;这是原作者有意简化的地方。
    
    //now check to see if the collision point is within range of the
    //velocity vector. [work in distance squared to avoid sqrt] and if it
    //is the closest hit found so far. 
    // ↓↓↓ 上面这段英文注释的翻译:检查碰撞点是否在速度向量的作用范围内
    // (用距离平方比较,避免开方),并且是迄今为止最近的一次碰撞;若是,
    // 说明球会在本帧与下一帧之间撞墙。

    // 球心与交点的距离平方(平方比较,省开方)。
    //If it is that means the ball will collide with the wall sometime
    //between this time step and the next one.
    double distSq = Vec2DDistanceSq(ThisCollisionPoint, IntersectionPoint);

    // 三个条件都满足才算"真的会撞":
    // 1) 交点距离 ≤ 本帧球能飞出的距离(本帧内能到);
    // 2) 比之前记录的最近距离还近;
    // 3) 交点确实在墙线段上。
    if ((distSq <= m_vVelocity.LengthSq()) && (distSq < DistToIntersection) && OnLineSegment)            
    {        
      // 更新"最近碰撞"记录:距离、墙的编号、碰撞点。
      DistToIntersection = distSq;
      idxClosest = w;
      CollisionPoint = IntersectionPoint;
    }     
  }//next wall

    
  //to prevent having to calculate the exact time of collision we
  //can just check if the velocity is opposite to the wall normal
  //before reflecting it. This prevents the case where there is overshoot
  //and the ball gets reflected back over the line before it has completely
  //reentered the playing area.
  //(原文注释:为避免计算精确碰撞时刻,只需检查速度是否与墙法线方向相反,
  //           然后反射它。这能防止球"过冲"后,在还没完全回到场地内时
  //           又被错误地弹回线外)

  // 满足两个条件才反弹:确实撞到了墙(idxClosest >= 0),且球正在朝墙飞
  // (点积 < 0 = 速度方向与墙法线方向相反)。
  if ( (idxClosest >= 0 ) && VelNormal.Dot(walls[idxClosest].Normal()) < 0)
  {
    // Reflect(法线):Vector2D 的成员函数,把向量沿法线"镜像"——像光撞镜面那样反弹。
    m_vVelocity.Reflect(walls[idxClosest].Normal());   
  }
}

//----------------------- PlaceAtLocation -------------------------------------
//
//  positions the ball at the desired location and sets the ball's velocity to
//  zero
//-----------------------------------------------------------------------------
// ↓↓↓ 原文翻译【PlaceAtPosition —— 放到指定位置】:
//   把球放到指定位置,并把速度清零。
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 成员函数:把球摆到指定位置(开球、进球后重新发球时用)。
//--------------------------------------------------------------------------------
void SoccerBall::PlaceAtPosition(Vector2D NewPos)
{
  // 位置 = 目标位置。
  m_vPosition = NewPos;

  // 旧位置同步成新位置——否则下一帧会误以为球"飞"了一大段。
  m_vOldPos = m_vPosition;
  
  // 速度清零:球静止。Zero():把向量的各分量都置为 0。
  m_vVelocity.Zero();
}

