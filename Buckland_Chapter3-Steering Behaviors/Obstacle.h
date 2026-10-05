//==============================================================================================
//【文件说明】Obstacle.h —— "障碍物"类声明:屏幕上一动不动的圆圈
//
//【这个文件是干什么的?】
//  定义一个极其简单的"障碍物"类:它只有一个位置(Pos)和一个半径(BRadius),
//  画出来就是一个黑色圆圈。机器人(Vehicle)的"障碍物躲避"行为会避开它们。
//  注意:障碍物不运动,所以 Update(更新)函数是空的——它不需要"活"。
//
//【与相关文件的关系】
//  BaseGameEntity.h —— 父类:障碍物继承"游戏实体基类",白拿位置、半径、
//                       编号等数据和读写接口,只需补上自己的 Render 画法;
//  2d/Vector2D.h    —— 2D 向量工具(Common 目录),位置用 Vector2D 表示;
//  misc/Cgdi.h      —— 绘图工具单例 gdi(Common 目录):画黑笔、画圆;
//  windows.h        —— Windows API 头文件(Cgdi.h 间接需要);
//  GameWorld.cpp    —— 真正创建障碍物的地方(随机位置、随机大小);
//  Obstacle.cpp     —— 本类的 Write/Read(存盘/读盘)实现;
//  SteeringBehaviors.cpp —— ObstacleAvoidance() 读取障碍物列表来做躲避。
//
//【C++ 小课堂:继承(extends)】
//  class Obstacle : public BaseGameEntity —— Obstacle 是 BaseGameEntity 的
//  "子类"。子类自动拥有父类的全部公开/保护成员(位置、半径、编号、Tag 等),
//  只需再写"自己特别的部分"。父类里带 virtual 的函数,子类可以"覆写"
//  (override):同名函数、自己的实现,多态时优先调用子类版本。
//==============================================================================================
#ifndef OBSTACLE_H
#define OBSTACLE_H
//------------------------------------------------------------------------
//
//  Name:   Obstacle.h
//
//  Desc:   Simple obstacle class
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// 包含 2D 向量工具(Common 目录):构造函数参数 Vector2D 定义在这里。
// "2d/xxx.h" 表示"附加包含目录(Common)下的 2d 子文件夹"。
#include "2d/Vector2D.h"
// 包含父类"游戏实体基类":不包含它,编译器不认识 BaseGameEntity 是什么。
#include "BaseGameEntity.h"
// 包含绘图工具单例 gdi(Common 目录):下面 Render() 里的 gdi-> 都来自它。
#include "misc/Cgdi.h"


// 包含 Windows 系统头文件(尖括号 = 到编译器系统目录找)。
// Cgdi.h 等依赖它提供的 HWND/HDC 等类型。
#include <windows.h>


//【类定义开始】class Obstacle : public BaseGameEntity
//  公有继承 BaseGameEntity:障碍物自动拥有父类的成员与接口。
class Obstacle : public BaseGameEntity
{
// public: —— 公开区:下面的成员任何代码都能访问。
public:

//【构造函数①:按坐标创建】参数 x、y(横纵坐标)、r(半径),
// 冒号后调用父类构造函数 BaseGameEntity(0, Vector2D(x,y), r):
//   0 —— 实体类型编号(0 代表"普通实体");
//   Vector2D(x,y) —— 用两个坐标拼成一个 2D 向量,作为出生位置;
//   r —— 包围半径(障碍物是圆,r 就是圆的半径)。
  Obstacle(double x,
           double y,
           double r):BaseGameEntity(0, Vector2D(x,y), r)
  {}

//【构造函数②:按"位置向量 + 半径"创建】与①等价,只是参数直接给 Vector2D。
  Obstacle(Vector2D pos, double radius):BaseGameEntity(0, pos, radius)
  {}

//【构造函数③:从文件读入创建】参数是输入文件流 ifstream 的引用(&)。
// 函数体 {Read(in);} 直接把读盘工作交给下面声明的 Read()——障碍物可以
// 保存在文件里,再按需读出来。
  Obstacle(std::ifstream& in){Read(in);}

//【虚析构函数】对象被销毁时自动执行。virtual 关键字:父类指针删除子类对象时,
// 保证先调子类析构再调父类析构(这里两个都是空的,主要为了接口规范)。
  virtual ~Obstacle(){}

// ↓↓↓ 原文注释翻译:这个函数在 BaseGameEntity 里被定义为纯虚函数,所以
//    子类必须实现它。(纯虚 = 父类只声明不实现,强制子类补全。)
// 障碍物不动,所以 Update 什么都不做:函数体是空的 {}。
  //this is defined as a pure virtual function in BasegameEntity so
  //it must be implemented
  void      Update(double time_elapsed){}

//【Render:把自己画到屏幕上】
//   gdi —— 绘图工具单例(全局唯一的绘图对象,来自 misc/Cgdi.h);
//   ->  —— 指针的成员访问运算符(读作"指向"):调用 gdi 指向对象的成员;
//   BlackPen() —— 把画笔换成黑色;
//   Circle(Pos(), BRadius()) —— 以"位置"为圆心、"半径"为半径画圆。
//   (分号把两句写在一行,效果等于分开两行。)
  void      Render(){gdi->BlackPen();gdi->Circle(Pos(), BRadius());}

// 声明:把障碍物数据写入输出流(存盘用)。const:本函数不修改成员。
// 实现见 Obstacle.cpp。
  void      Write(std::ostream& os)const;
// 声明:从输入流读取障碍物数据(读盘用)。实现见 Obstacle.cpp。
  void      Read(std::ifstream& in);
};



#endif

