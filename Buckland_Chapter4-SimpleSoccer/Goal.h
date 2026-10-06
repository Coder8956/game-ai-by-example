//==============================================================================================
//【文件说明】Goal.h —— 足球场上的"球门"类
//
//【这个文件是干什么的?】
//  球场两端各有一个球门。本文件定义 Goal(球门)类:一个球门 = 左门柱 + 右门柱
//  (+ 朝向 + 球门线中心),并且负责回答一个问题——"球有没有飞进球门?"
//  回答方式:每帧调用 Scored(),用"线段相交"检测球的飞行轨迹是否穿过球门线。
//
//【谁在使用这个文件?】(= 哪些文件用 #include 包含了它)
//  SoccerPitch.cpp    —— 在构造函数里 new 出红、蓝两座球门;
//  SoccerTeam.cpp / PlayerBase.cpp / FieldPlayer.cpp / Goalkeeper.cpp
//  FieldPlayerStates.cpp / GoalKeeperStates.cpp / SupportSpotCalculator.cpp
//                     —— 球员踢球、判断射门时调用 Scored() 与各访问器;
//  main.cpp           —— 间接使用:它创建 SoccerPitch,场地里就带着球门。
//
//【本文件包含了谁?】
//  SoccerBall.h    —— 足球类声明(Scored 要用球的当前位置/上一位置);
//  2D/Vector2D.h   —— 2D 向量类(坐标、加减乘除运算);
//  2D/geometry.h   —— 2D 几何工具库(线段相交判断 LineIntersection2D)。
//
//【C++ 小课堂:类的封装(Encapsulation)与命名前缀】
//  类的成员分两区:
//    private:(私有)——只有本类自己的函数能读能写,外界碰不到(保护数据不被乱改);
//    public :(公有)——谁都能调用(对外提供的"操作按钮")。
//  匈牙利命名法前缀:
//    m_  = member,成员变量;m_v = 向量(Vector2D)成员;m_i = 整数(int)成员;
//    m_vLeftPost / m_vRightPost 中的 Left/Right 只是名字,用来区分左右门柱。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护 / include guard —— C++ 头文件的标准开场白三件套:
//   #ifndef GOAL_H :if not defined,若 "GOAL_H" 这个名字还没定义过 → 继续往下;
//   #define GOAL_H  :立刻把 GOAL_H 定义为"本文件已处理"的标记;
//   #endif         :结束条件区。
// 作用:#include 的本质是"把文件内容复制粘贴进来";若一条包含链把本文件复制两遍,
// 同样的定义会出现两次,编译器报"重复定义"错误。这三行保证第二遍复制被自动跳过。
// (详解亦可参见 WestWorld1/Locations.h 中的同名注释。)
//--------------------------------------------------------------------------------
#ifndef GOAL_H
#define GOAL_H
//------------------------------------------------------------------------
//
//Name:   Goal.h
//
//Desc:   class to define a goal for a soccer pitch. The goal is defined
//        by two 2D vectors representing the left and right posts.
//
//        Each time-step the method Scored should be called to determine
//        if a goal has been scored.
//
//Author: Mat Buckland 2003 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 三条 #include:把本文件需要的"零件说明书"先拿进来。
//   "SoccerBall.h"  —— 足球类声明(Scored 要用球的当前位置与上一位置);
//   "2D/Vector2D.h" —— 2D 向量类(坐标、加减乘除运算);
//   "2D/geometry.h" —— 2D 几何工具库(线段相交判断 LineIntersection2D)。
// 带 "2D/" 前缀是因为这些头文件位于公共目录 Common\2D\ 下
// (工程配置了附加包含目录,编译器才能找到)。
//--------------------------------------------------------------------------------
#include "SoccerBall.h"
#include "2D/Vector2D.h"
#include "2D/geometry.h"



//--------------------------------------------------------------------------------
// class Goal —— 定义"球门"类(类 = 图纸;本章的红队球门、蓝队球门
// 都是照这张图纸造出来的对象)。
//   class :C++ 关键字,表示"开始定义一个类";
//   Goal  :类名;
//   { };  :花括号内写类的成员;末尾的 ; 是类的固定句号。
//--------------------------------------------------------------------------------
class Goal 
{

private:

  // 左门柱的位置。m_ = 成员变量前缀;v = Vector2D 向量类型。
  Vector2D   m_vLeftPost;
  // 右门柱的位置。
  Vector2D   m_vRightPost;

  //a vector representing the facing direction of the goal
  //(原文注释:一个向量,表示球门的朝向)

  // 球门"朝向"向量:从球门线指向球场内部的方向,用来区分"正面/侧面"。
  Vector2D   m_vFacing;

  //the position of the center of the goal line
  //(原文注释:球门线中心点的位置)

  // 球门线(两根门柱之间的连线)的中点,即"球门正中心"。
  Vector2D   m_vCenter;

  //each time Scored() detects a goal this is incremented
  //(原文注释:每当 Scored() 检测到一次进球,这个数就加 1)

  // 累计进球数。
  int        m_iNumGoalsScored;

public:

  //--------------------------------------------------------------------------------
  // 构造函数:创建 Goal 对象时由 C++ 自动调用,把上面 5 个成员一次性初始化好。
  // 冒号 : 后面是"初始化列表":成员名(初值) 逐项初始化——比进函数体再赋值更高效。
  //   left/right/facing  —— 调用者传入的三个参数(左门柱、右门柱、朝向);
  //   m_vCenter((left+right)/2.0) —— 两门柱坐标相加再 ÷2 = 两点中点 = 球门中心;
  //   m_iNumGoalsScored(0) —— 新球门进球数从 0 算起。
  // 成员名(m_ 开头)与参数名(left 等)刻意不同名,避免"自己给自己赋值"的混淆。
  //--------------------------------------------------------------------------------
  Goal(Vector2D left, Vector2D right, Vector2D facing):m_vLeftPost(left),
                                                       m_vRightPost(right),
                                                       m_vCenter((left+right)/2.0),
                                                       m_iNumGoalsScored(0),
                                                       m_vFacing(facing)
  {  }

  //Given the current ball position and the previous ball position,
  //this method returns true if the ball has crossed the goal line 
  //and increments m_iNumGoalsScored
  //(原文注释:给定球的"当前位置"和"上一帧位置",若球越过了球门线则返回 true,
  //           并把 m_iNumGoalsScored 加 1)

  // 进球检测函数的"声明"(只有签名,没有函数体;实现写在本文件最下方的 Goal::Scored)。
  //   inline            —— 请求编译器把函数体"内联"进调用处,省掉函数调用开销;
  //   bool              —— 返回类型:true=进球,false=没进;
  //   const SoccerBall* —— 指向 SoccerBall 对象的指针,const 表示"只读借用,不许改球";
  //   *const            —— 指针本身也不能改指向(双保险);
  //   ;                 —— 声明以分号结尾,函数体另写在下面。
  inline bool Scored(const SoccerBall*const ball);

  //-----------------------------------------------------accessor methods
  //(原文注释分隔线:accessor methods = "访问器"——对外只读地暴露私有数据)

  // 下面 4 个是"访问器函数":末尾的 const 表示"只读函数,保证不修改任何成员",
  // 花括号里直接 return 对应的私有成员(最简写法,函数体就写在声明行内)。
  // 它们分别返回:球门中心 / 球门朝向 / 左门柱位置 / 右门柱位置。
  Vector2D Center()const{return m_vCenter;}
  Vector2D Facing()const{return m_vFacing;}
  Vector2D LeftPost()const{return m_vLeftPost;}
  Vector2D RightPost()const{return m_vRightPost;}

  // 返回目前进球数;ResetGoalsScored() 把进球数清零(开新场/重赛前调用)。
  int      NumGoalsScored()const{return m_iNumGoalsScored;}
  void     ResetGoalsScored(){m_iNumGoalsScored = 0;}
};


/////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------
// Goal::Scored —— 进球检测的实现(声明见上面 public 区)。
//   Goal:: —— "作用域解析符":说明下面这个 Scored 是 Goal 类的成员函数;
//   ball   —— 指向足球对象的指针(由调用者传入)。
// 原理:把"球从上一帧位置飞到现在位置"的轨迹看作一条线段;若它与
// "左门柱→右门柱"的球门线段相交,说明球穿过了球门线 → 进球!
//--------------------------------------------------------------------------------
bool Goal::Scored(const SoccerBall*const ball)
{
  // LineIntersection2D(线段A起点, 线段A终点, 线段B起点, 线段B终点):
  // geometry.h 提供的工具函数,判断两条线段是否相交(相交返回 true)。
  // 这里:线段A = 球的轨迹(OldPos→Pos),线段B = 球门线(LeftPost→RightPost)。
  // ball->Pos() 里的 "->" :指针指向对象的"成员访问符",等价于 (*ball).Pos()。
  if (LineIntersection2D(ball->Pos(), ball->OldPos(), m_vLeftPost, m_vRightPost))
  {
    // 进球了:计数先自增 1(++ 放前面 = 先加后用),再返回 true 通知调用者。
    ++m_iNumGoalsScored;

    return true;
  }

  // 轨迹没穿过球门线:没进球,返回 false。
  return false;
}


#endif