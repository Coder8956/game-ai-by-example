//==============================================================================================
//【文件说明】Vector2D.h —— 全书的数学地基:"二维向量"工具库
//
//【这个文件是干什么的?】
//  游戏里每个角色都有"位置"和"移动方向",在程序里都用二元组 (x, y) 表示。本文件把它
//  封装成 Vector2D 结构体,并配齐全套数学运算:向量加减、数乘、点积、长度、归一化、
//  反射(撞墙反弹)、两点距离、视野判断等。第 2~10 章几乎所有类都建立在它之上。
//
//【谁在使用这个文件?】(几乎全书都用,举代表)
//  Game/BaseGameEntity.h、Game/MovingEntity.h —— 游戏实体的位置/速度/朝向;
//  Game/Region.h、Graph/GraphNodeTypes.h —— 区域与图节点的坐标;
//  第 4 章 Goal.h / SteeringBehaviors.h / SoccerPitch.h —— 足球与操控行为;
//  misc/Cgdi.h —— 绘图(把向量坐标画到屏幕)。
//
//【本文件包含了谁?】
//  <math.h>       —— C 数学库(sqrt 开平方、cos 余弦);
//  <windows.h>    —— Windows 头文件(POINTS / POINT 屏幕坐标类型);
//  <iosfwd>       —— 输入输出流的前置声明(std::ostream / std::ifstream);
//  <limits>       —— 数值极限(std::numeric_limits,求最小精度 epsilon);
//  "misc/utils.h" —— 工具库(isEqual 浮点近似相等、MinDouble 极小数)。
//
//【C++ 小课堂:struct 与运算符重载】
//  struct(结构体)和 class 几乎一样,唯一区别:struct 的成员默认 public(公开),
//  class 默认 private。这里 x、y 直接公开最方便。
//  运算符重载:让自定义类型也能用 + - * / == 这种符号。operator+=(const Vector2D&)
//  就是在定义"向量 += 向量"这个操作具体怎么做,写法上看着像给 C++ 加新功能。
//==============================================================================================
#ifndef S2DVECTOR_H
#define S2DVECTOR_H
//------------------------------------------------------------------------
//
//  Name:   Vector2D.h
//
//  Desc:   2D vector struct
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Vector2D —— 二维向量】:
//   描述:一个 2D 向量结构体。作者:Mat Buckland。
//------------------------------------------------------------------------
#include <math.h>
#include <windows.h>
#include <iosfwd>
#include <limits>
#include "misc/utils.h"


//--------------------------------------------------------------------------------
// struct Vector2D —— 二维向量结构体。
//  同一个 (x,y) 有两种用法:① 当作"坐标点"(角色站在哪);② 当作"方向箭头"
//  (往哪走、走多快)。加减速、转弯、两点距离全都在这一个结构上算。
//--------------------------------------------------------------------------------
struct Vector2D
{
  // x:横坐标(向右为正)。double = 双精度小数类型。
  double x;
  double y;
  // y:纵坐标。注意 Windows 窗口坐标系里 y 轴向下为正(屏幕左上角是原点)。

  // 两个构造函数,都是"初始化列表"写法(冒号后逐项给 x、y 赋初值):
  // 第 1 个:不带参数 → 默认把 x、y 都设成 0.0(零点);
  // 第 2 个:给两个参数 a、b → x=a、y=b(造在指定点)。{} 是空函数体。
  Vector2D():x(0.0),y(0.0){}
  Vector2D(double a, double b):x(a),y(b){}

  //sets x and y to zero
  void Zero(){x=0.0; y=0.0;}
  //(原文注释:把 x 和 y 置为零)
  // Zero():把当前向量清零(位置归原点/速度清零)。

  //returns true if both x and y are zero
  bool isZero()const{return (x*x + y*y) < MinDouble;}
  //(原文注释:若 x、y 都为零则返回 true)
  // isZero():判断是不是零向量。用 x²+y² 与极小数 MinDouble 比较,而不直接 == 0,
  // 是为了容忍浮点误差。末尾 const = 只读函数,不修改成员。

  //returns the length of the vector
  inline double    Length()const;
  //(原文注释:返回向量的长度)—— 即箭头有多长(勾股定理 √(x²+y²));实现见文件下方。

  //returns the squared length of the vector (thereby avoiding the sqrt)
  inline double    LengthSq()const;
  //(原文注释:返回向量长度的平方,从而省去开方 sqrt)
  // 只比较大小、不需要真实长度时,用长度平方更快(开方很慢)。

  // Normalize():归一化——把向量长度缩成 1,只保留"方向"。
  inline void      Normalize();

  // Dot(v2):点积。结果 = x*v2.x + y*v2.y,可判断两向量夹角(同向为正、反向为负、垂直为 0)。
  // 参数 const Vector2D& = 把另一个向量"常引用"传进来(不复制、不允许改)。
  inline double    Dot(const Vector2D& v2)const;

  //returns positive if v2 is clockwise of this vector,
  //negative if anticlockwise (assuming the Y axis is pointing down,
  //X axis to right like a Window app)
  // Sign(v2):判断 v2 在本向量的顺时针还是逆时针方向,返回 ±1(配合上面的枚举)。
  // ↓↓↓ 上面这段英文注释的翻译:若 v2 在本向量的顺时针方向,返回正数;
  //   逆时针方向返回负数(前提:y 轴朝下、x 轴朝右,像 Windows 窗口那样)。
  inline int       Sign(const Vector2D& v2)const;

  //returns the vector that is perpendicular to this one.
  // Perp():求垂直向量(左转 90°),实现是 (x,y) → (-y,x)。
  //(原文注释:返回与本向量垂直的那个向量)
  inline Vector2D  Perp()const;

  //adjusts x and y so that the length of the vector does not exceed max
  //(原文注释:调整 x、y,使向量长度不超过 max)
  // Truncate(max):把太长的向量"砍短"到最长 max(限速用)。
  inline void      Truncate(double max);

  //returns the distance between this vector and th one passed as a parameter
  //(原文注释:返回本向量与参数向量之间的距离)
  // Distance(v2):两点间直线距离(欧氏距离)。
  inline double    Distance(const Vector2D &v2)const;

  //squared version of above.
  //(原文注释:上面那个的"平方"版本)—— DistanceSq:距离的平方,省去开方。
  inline double    DistanceSq(const Vector2D &v2)const;

  // Reflect(norm):反射——让向量沿法线 norm"镜像反弹"(像球撞墙),norm 须是单位法线。
  inline void      Reflect(const Vector2D& norm);

  //returns the vector that is the reverse of this vector
  //(原文注释:返回与本向量方向相反的向量)
  // GetReverse():取反向量(箭头调头)。
  inline Vector2D  GetReverse()const;


  //we need some overloaded operators
  // ↓↓↓ 下面一组是"运算符重载"(原文注释:we need some overloaded operators =
  //   我们需要一些重载运算符)。
  // operator+=( rhs ):定义"v1 += v2"。rhs = right hand side(右手边操作数)。
  // 做法:本向量的 x、y 分别加上 rhs 的 x、y,最后 return *this。
  // 返回类型 const Vector2D&:返回自身引用(链式写法 a += b += c 可行);
  // *this:this 是指向"当前对象"的指针,*this 就是对象本身。
  const Vector2D& operator+=(const Vector2D &rhs)
  {
    x += rhs.x;
    y += rhs.y;

    return *this;
  }

  // operator-=( rhs ):定义"v1 -= v2"——x、y 各自减去 rhs(向量减法)。
  const Vector2D& operator-=(const Vector2D &rhs)
  {
    x -= rhs.x;
    y -= rhs.y;

    return *this;
  }

  // operator*=( rhs ):定义"v *= 数字"——向量乘一个标量(放大/缩短箭头)。
  const Vector2D& operator*=(const double& rhs)
  {
    x *= rhs;
    y *= rhs;

    return *this;
  }

  // operator/=( rhs ):定义"v /= 数字"——向量除以一个标量。
  const Vector2D& operator/=(const double& rhs)
  {
    x /= rhs;
    y /= rhs;

    return *this;
  }

  // operator==:定义"v1 == v2"。用 isEqual 做浮点近似相等(不直接 ==,避免误差)。
  bool operator==(const Vector2D& rhs)const
  {
    return (isEqual(x, rhs.x) && isEqual(y,rhs.y) );
  }

  // operator!=:定义"v1 != v2"。|| 是"逻辑或":x、y 任一不等就不等。
  bool operator!=(const Vector2D& rhs)const
  {
    return (x != rhs.x) || (y != rhs.y);
  }
  
};

//-----------------------------------------------------------------------some more operator overloads
//-----------------------------------------------------------------------
// 下面是一组"非成员"运算符重载的声明(写在类外,供 "向量+向量"、"向量*数"用)。
// lhs = left hand side(左手边操作数)。实现见文件最底部。
//   operator<<:把向量打印进输出流(调试时 cout << v 直接看坐标);
//   operator>>:从输入流读进一个向量。
//-----------------------------------------------------------------------
inline Vector2D operator*(const Vector2D &lhs, double rhs);
inline Vector2D operator*(double lhs, const Vector2D &rhs);
inline Vector2D operator-(const Vector2D &lhs, const Vector2D &rhs);
inline Vector2D operator+(const Vector2D &lhs, const Vector2D &rhs);
inline Vector2D operator/(const Vector2D &lhs, double val);
std::ostream& operator<<(std::ostream& os, const Vector2D& rhs);
std::ifstream& operator>>(std::ifstream& is, Vector2D& lhs);


//------------------------------------------------------------------------member functions

//------------------------- Length ---------------------------------------
//
//  returns the length of a 2D vector
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// 下面是上面那些成员函数的具体实现(都标 inline,直接写在头文件里)。
// Vector2D:: 表示它们属于 Vector2D 类。
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Length —— 求长度】:返回二维向量的长度(√(x²+y²))。
inline double Vector2D::Length()const
{
  return sqrt(x * x + y * y);
}


//------------------------- LengthSq -------------------------------------
//
//  returns the squared length of a 2D vector
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【LengthSq —— 求长度平方】:返回长度平方(不开方)。
inline double Vector2D::LengthSq()const
{
  return (x * x + y * y);
}


//------------------------- Vec2DDot -------------------------------------
//
//  calculates the dot product
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Dot —— 点积】:计算点积 x*v2.x + y*v2.y。
inline double Vector2D::Dot(const Vector2D &v2)const
{
  return x*v2.x + y*v2.y;
}

//------------------------ Sign ------------------------------------------
//
//  returns positive if v2 is clockwise of this vector,
//  minus if anticlockwise (Y axis pointing down, X axis to right)
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Sign —— 转向】:若 v2 在本向量顺时针方向返回正数,逆时针返回负数。
// 下面这行匿名枚举:定义两个有名字的整数常量 clockwise=1、anticlockwise=-1,
// 供 Sign 返回。匿名 = 不给枚举类型起名字,只借用它"定义一批整数常量"的能力。
enum {clockwise = 1, anticlockwise = -1};

inline int Vector2D::Sign(const Vector2D& v2)const
{
  if (y*v2.x > x*v2.y)
  { 
    return anticlockwise;
  }
  else 
  {
    return clockwise;
  }
}

//------------------------------ Perp ------------------------------------
//
//  Returns a vector perpendicular to this vector
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Perp —— 垂直向量】:返回与本向量垂直的向量。
// 做法:原 (x,y) 左转 90° 变成 (-y, x)。
inline Vector2D Vector2D::Perp()const
{
  return Vector2D(-y, x);
}

//------------------------------ Distance --------------------------------
//
//  calculates the euclidean distance between two vectors
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Distance —— 距离】:计算两个向量(两点)之间的欧氏直线距离。
inline double Vector2D::Distance(const Vector2D &v2)const
{
  double ySeparation = v2.y - y;
  double xSeparation = v2.x - x;

  return sqrt(ySeparation*ySeparation + xSeparation*xSeparation);
}


//------------------------------ DistanceSq ------------------------------
//
//  calculates the euclidean distance squared between two vectors 
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【DistanceSq —— 距离平方】:两点距离的平方(省去开方)。
inline double Vector2D::DistanceSq(const Vector2D &v2)const
{
  double ySeparation = v2.y - y;
  double xSeparation = v2.x - x;

  return ySeparation*ySeparation + xSeparation*xSeparation;
}

//----------------------------- Truncate ---------------------------------
//
//  truncates a vector so that its length does not exceed max
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Truncate —— 截断】:若向量长度超过 max,就砍到 max。
// 三步:① 比长度 ② 超长则归一化(变单位向量)③ 乘 max(拉长到正好 max)。
inline void Vector2D::Truncate(double max)
{
  if (this->Length() > max)
  {
    this->Normalize();

    *this *= max;
  } 
}

//--------------------------- Reflect ------------------------------------
//
//  given a normalized vector this method reflects the vector it
//  is operating upon. (like the path of a ball bouncing off a wall)
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Reflect —— 反射】:给定一个归一化的法线,把本向量沿它镜像反弹
//   (像球撞墙后弹出去的路线)。公式:新向量 = 原向量 + 2×点积×法线反向。
inline void Vector2D::Reflect(const Vector2D& norm)
{
  *this += 2.0 * this->Dot(norm) * norm.GetReverse();
}

//----------------------- GetReverse ----------------------------------------
//
//  returns the vector that is the reverse of this vector
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【GetReverse —— 取反】:返回方向完全相反的向量(-x,-y)。
inline Vector2D Vector2D::GetReverse()const
{
  return Vector2D(-this->x, -this->y);
}


//------------------------- Normalize ------------------------------------
//
//  normalizes a 2D Vector
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Normalize —— 归一化】:把向量长度缩成 1,只留方向。
// 做法:x、y 各除以长度。先判断长度大于最小精度 epsilon 再除,避免除以零。
// std::numeric_limits<double>::epsilon() = double 能分辨的最小正数(几乎 0 的界)。
inline void Vector2D::Normalize()
{ 
  double vector_length = this->Length();

  if (vector_length > std::numeric_limits<double>::epsilon())
  {
    this->x /= vector_length;
    this->y /= vector_length;
  }
}


//------------------------------------------------------------------------non member functions

//------------------------------------------------------------------------
// 下面是一组"自由函数"(不属于任何类,直接用函数名调用;参数是向量本身)。
// 与上面成员函数功能相同,只是写法不同:Vec2DNormalize(v) 等价于让 v 归一化。
//------------------------------------------------------------------------
inline Vector2D Vec2DNormalize(const Vector2D &v)
{
  Vector2D vec = v;

  double vector_length = vec.Length();

  if (vector_length > std::numeric_limits<double>::epsilon())
  {
    vec.x /= vector_length;
    vec.y /= vector_length;
  }

  return vec;
}


// Vec2DDistance(v1,v2):两点直线距离(成员函数 Distance 的自由函数版)。
inline double Vec2DDistance(const Vector2D &v1, const Vector2D &v2)
{

  double ySeparation = v2.y - v1.y;
  double xSeparation = v2.x - v1.x;

  return sqrt(ySeparation*ySeparation + xSeparation*xSeparation);
}

// Vec2DDistanceSq:两点距离的平方。
inline double Vec2DDistanceSq(const Vector2D &v1, const Vector2D &v2)
{

  double ySeparation = v2.y - v1.y;
  double xSeparation = v2.x - v1.x;

  return ySeparation*ySeparation + xSeparation*xSeparation;
}

// Vec2DLength(v):向量长度;Vec2DLengthSq(v):长度平方。成员函数的自由函数版。
inline double Vec2DLength(const Vector2D& v)
{
  return sqrt(v.x*v.x + v.y*v.y);
}

inline double Vec2DLengthSq(const Vector2D& v)
{
  return (v.x*v.x + v.y*v.y);
}


//------------------------------------------------------------------------
// 下面 4 个是"互转"函数:在 Windows 的屏幕坐标类型(POINT/POINTS)和我们的
// Vector2D 之间换算。POINT = 长整型坐标,POINTS = 短整型坐标。
inline Vector2D POINTStoVector(const POINTS& p)
{
  return Vector2D(p.x, p.y);
}

inline Vector2D POINTtoVector(const POINT& p)
{
  return Vector2D((double)p.x, (double)p.y);
}

inline POINTS VectorToPOINTS(const Vector2D& v)
{
  POINTS p;
  p.x = (short)v.x;
  p.y = (short)v.y;

  return p;
}

inline POINT VectorToPOINT(const Vector2D& v)
{
  POINT p;
  p.x = (long)v.x;
  p.y = (long)v.y;

  return p;
}



//========================================================================
// 下面是类外的二元运算符实现。做法都一样:先复制一份 lhs 到 result,
// 再对 result 做运算,最后返回新结果(不改动原向量)。
//------------------------------------------------------------------------operator overloads
inline Vector2D operator*(const Vector2D &lhs, double rhs)
{
  Vector2D result(lhs);
  result *= rhs;
  return result;
}

inline Vector2D operator*(double lhs, const Vector2D &rhs)
{
  Vector2D result(rhs);
  result *= lhs;
  return result;
}

//overload the - operator
inline Vector2D operator-(const Vector2D &lhs, const Vector2D &rhs)
{
  Vector2D result(lhs);
  result.x -= rhs.x;
  result.y -= rhs.y;
  
  return result;
}

//overload the + operator
inline Vector2D operator+(const Vector2D &lhs, const Vector2D &rhs)
{
  Vector2D result(lhs);
  result.x += rhs.x;
  result.y += rhs.y;
  
  return result;
}

//overload the / operator
inline Vector2D operator/(const Vector2D &lhs, double val)
{
  Vector2D result(lhs);
  result.x /= val;
  result.y /= val;

  return result;
}

///////////////////////////////////////////////////////////////////////////////


//treats a window as a toroid
// ↓↓↓ 原文注释翻译:把窗口当成一个"圆环面"(toroid)——物体走出右边界就从左边
//   钻出来,走出下边界就从上边钻出来(像贪吃蛇穿墙)。
inline void WrapAround(Vector2D &pos, int MaxX, int MaxY)
{
  if (pos.x > MaxX) {pos.x = 0.0;}

  if (pos.x < 0)    {pos.x = (double)MaxX;}

  if (pos.y < 0)    {pos.y = (double)MaxY;}

  if (pos.y > MaxY) {pos.y = 0.0;}
}

//returns true if the point p is not inside the region defined by top_left
//and bot_rgt
  // 下面三个函数都在判断"点 p 是否落在某矩形范围内":
  //   NotInsideRegion:点在矩形"外"返回 true(任一坐标越界即在外);
  //   InsideRegion(两个重载):点在矩形"内"返回 true,就是对 NotInsideRegion 取反
  //     (! 是逻辑非)。第一个用左上/右下两个向量定矩形,第二个直接给四个整数边界。
  // ↓↓↓ 上面两行英文注释的翻译:若点 p 不在 top_left(左上角)与 bot_rgt(右下角)
  //   定义的矩形范围内,返回 true。
inline bool NotInsideRegion(Vector2D p,
                            Vector2D top_left,
                            Vector2D bot_rgt)
{
  return (p.x < top_left.x) || (p.x > bot_rgt.x) || 
         (p.y < top_left.y) || (p.y > bot_rgt.y);
}

inline bool InsideRegion(Vector2D p,
                         Vector2D top_left,
                         Vector2D bot_rgt)
{
  return !((p.x < top_left.x) || (p.x > bot_rgt.x) || 
         (p.y < top_left.y) || (p.y > bot_rgt.y));
}

inline bool InsideRegion(Vector2D p, int left, int top, int right, int bottom)
{
  return !( (p.x < left) || (p.x > right) || (p.y < top) || (p.y > bottom) );
}

//------------------ isSecondInFOVOfFirst -------------------------------------
//
//  returns true if the target position is in the field of view of the entity
//  positioned at posFirst facing in facingFirst
//-----------------------------------------------------------------------------
// ↓↓↓ 原文翻译【isSecondInFOVOfFirst —— 第二点是否在第一者视野内】:
//   给定站在 posFirst、面朝 facingFirst 的实体,若目标 posSecond 在它的视野
//   张角 fov 之内,返回 true。
// 原理:把"指向目标"的向量归一化后,与"朝向"做点积;点积 ≥ cos(半张角) 即在内。
inline bool isSecondInFOVOfFirst(Vector2D posFirst,
                                 Vector2D facingFirst,
                                 Vector2D posSecond,
                                 double    fov)
{
  Vector2D toTarget = Vec2DNormalize(posSecond - posFirst);

  return facingFirst.Dot(toTarget) >= cos(fov/2.0);
}





#endif