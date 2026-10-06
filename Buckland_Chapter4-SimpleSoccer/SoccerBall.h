//==============================================================================================
//【文件说明】SoccerBall.h —— 足球类的声明(SoccerBall)
//
//【这个文件是干什么的?】
//  声明足球对象有哪些数据和功能:位置/速度/朝向(继承自 MovingEntity)、上一帧位置、
  // 球场边界墙引用,以及踢球 Kick、每帧更新 Update、撞墙反弹 TestCollisionWithWalls、
// 预测未来位置 FuturePosition、把球停住 Trap 等。具体实现全在 SoccerBall.cpp。
//
//【谁在使用这个文件?】
//  Goal.h、PlayerBase.h/.cpp、SoccerTeam.h/.cpp、各状态文件、SteeringBehaviors.cpp
//  —— 球员判断"球离我多远/在哪/能不能踢"时都要用到 SoccerBall;
//  SoccerPitch.cpp —— 真正 new 出一个足球对象并持有它。
//
//【本文件包含了谁?】
//  <vector>              —— 标准库动态数组;
//  "Game/MovingEntity.h" —— Common\Game 的运动物体基类(足球也是会动的物体);
//  "constants.h"         —— 本章常量。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护 + #pragma warning(disable:4786):原理分别见 Goal.h 与 SoccerPitch.h。
//--------------------------------------------------------------------------------
#ifndef SOCCERBALL_H
#define SOCCERBALL_H
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name: SoccerBall.h
//
//  Desc: Class to implement a soccer ball. This class inherits from
//        MovingEntity and provides further functionality for collision
//        testing and position prediction.
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:SoccerBall.h
//   描述  :实现足球类。它继承自 MovingEntity,并额外提供碰撞检测与位置预测功能。
//   作者  :Mat Buckland,2003 年(本书作者)
//
//------------------------------------------------------------------------
#include <vector>

#include "Game/MovingEntity.h"
#include "constants.h"


// 前置声明:Wall2D(墙)、PlayerBase(球员)只以指针/引用形式出现在本文件,
// 故只报名字即可,完整定义由 .cpp 去包含(原理详见 SoccerPitch.h)。
class Wall2D;
class PlayerBase;


//--------------------------------------------------------------------------------
// class SoccerBall : public MovingEntity —— 足球类,公有继承运动物体基类
// (白得位置/速度/朝向等),再加上足球专属的碰撞与预测功能。
//--------------------------------------------------------------------------------
class SoccerBall : public MovingEntity
{
private:

  //keeps a record of the ball's position at the last update
//(原文注释:记录上一帧更新时球的位置)—— 进球判定要用到球"从哪飞到哪"。
  Vector2D                  m_vOldPos;

  //a local reference to the Walls that make up the pitch boundary
//(原文注释:指向构成球场边界的那些墙的本地引用)
// const std::vector<Wall2D>& 中的 & 是"引用"(别名):不另存一份墙列表,
// 只是借用球场那份;const 表示只读。
  const std::vector<Wall2D>& m_PitchBoundary;                                      


  

public:
    //tests to see if the ball has collided with a ball and reflects 
  //the ball's velocity accordingly
  void TestCollisionWithWalls(const std::vector<Wall2D>& walls);

//--------------------------------------------------------------------------------
// 构造函数:创建足球。冒号后先调用父类 MovingEntity 初始化位置/初速度(0,0)/质量等;
// 注释里写明 max speed、scale、turn rate、max force 这几个参数对足球用不上(unused)。
// 最后 m_PitchBoundary(PitchBoundary) 把球场墙列表的引用接住。
//--------------------------------------------------------------------------------
  SoccerBall(Vector2D           pos,            
             double               BallSize,
             double               mass,
             std::vector<Wall2D>& PitchBoundary):
  
      //set up the base class
      MovingEntity(pos,
                  BallSize,
                  Vector2D(0,0),
                  -1.0,                //max speed - unused
                  Vector2D(0,1),
                  mass,
                  Vector2D(1.0,1.0),  //scale     - unused
                  0,                   //turn rate - unused
                  0),                  //max force - unused
     m_PitchBoundary(PitchBoundary)
  {}
  
  //implement base class Update
  void      Update();

  //implement base class Render
  void      Render();

  //a soccer ball doesn't need to handle messages
//(原文注释:足球不需要处理消息)
// HandleMessage(const Telegram& msg){return false;} —— 继承来的消息处理函数,
// 足球一律不处理任何消息,直接内联返回 false。Telegram(电报)是 Common 消息系统的信封类型。
  bool      HandleMessage(const Telegram& msg){return false;}

  //this method applies a directional force to the ball (kicks it!)
  void      Kick(Vector2D direction, double force);

  //given a kicking force and a distance to traverse defined by start
  //and finish points, this method calculates how long it will take the
  //ball to cover the distance.
  double    TimeToCoverDistance(Vector2D from,
                               Vector2D to,
                               double     force)const;

  //this method calculates where the ball will in 'time' seconds
  Vector2D FuturePosition(double time)const;

  //this is used by players and goalkeepers to 'trap' a ball -- to stop
  //it dead. That player is then assumed to be in possession of the ball
//(原文注释:球员/守门员用它把球"控住"——让球立刻停下,随后视为该球员控球)
  //and m_pOwner is adjusted accordingly
  void      Trap(){m_vVelocity.Zero();}  

  Vector2D  OldPos()const{return m_vOldPos;}
  
  //this places the ball at the desired location and sets its velocity to zero
  void      PlaceAtPosition(Vector2D NewPos);
};



//this can be used to vary the accuracy of a player's kick.
// 自由函数声明(实现见 SoccerBall.cpp):给射门目标加一点随机误差,让脚法不是 100% 准。
Vector2D AddNoiseToKick(Vector2D BallPos, Vector2D BallTarget);



#endif