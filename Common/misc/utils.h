//==============================================================================================
//【文件说明】utils.h —— 全书共用的小工具函数与数学常量
//
//【这个文件是干什么的?】
//  集中放了一堆"哪里都能用"的小函数:
//   · 数学常量(Pi/TwoPi/...);
//   · 角度↔弧度转换、浮点判零、范围判断;
//   · 随机数(RandInt/RandFloat/RandGaussian 高斯分布);
//   · 钳制 Clamp、四舍五入 Rounded、Sigmoid;
//   · 统计(Average/StandardDeviation)、STL 容器批量 delete 工具。
//
//【谁在使用这个文件?】
//  几乎所有工程(Vector2D、MovingEntity、Raven 行为等都 include 它)。
//==============================================================================================
#ifndef UTILS_H
#define UTILS_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//------------------------------------------------------------------------
//
//  Name: utils.h
//
//  Desc: misc utility functions and constants
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <math.h>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <cassert>
#include <iomanip>



  // 常量:最大值;Pi/TwoPi/HalfPi/QuarterPi 弧度常用值。
//a few useful constants
const int     MaxInt    = (std::numeric_limits<int>::max)();
const double  MaxDouble = (std::numeric_limits<double>::max)();
const double  MinDouble = (std::numeric_limits<double>::min)();
const float   MaxFloat  = (std::numeric_limits<float>::max)();
const float   MinFloat  = (std::numeric_limits<float>::min)();

const double   Pi        = 3.14159;
const double   TwoPi     = Pi * 2;
const double   HalfPi    = Pi / 2;
const double   QuarterPi = Pi / 4;

//returns true if the value is a NaN
template <typename T>
  // isNaN:NaN 是唯一"不等于自己"的浮点值,用 val != val 判断。
inline bool isNaN(T val)
{
  return val != val;
}

  // DegsToRads:角度转弧度(游戏内部全用弧度)。
inline double DegsToRads(double degs)
{
  return TwoPi * (degs/360.0);
}



//returns true if the parameter is equal to zero
  // IsZero:浮点判零——在 ±MinDouble 之间就算 0(直接 == 0 不准)。
inline bool IsZero(double val)
{
  return ( (-MinDouble < val) && (val < MinDouble) );
}

//returns true is the third parameter is in the range described by the
//first two
  // InRange:判断 val 是否落在 start..end 之间(顺序无所谓)。
inline bool InRange(double start, double end, double val)
{
  if (start < end)
  {
    if ( (val > start) && (val < end) ) return true;
    else return false;
  }

  else
  {
    if ( (val < start) && (val > end) ) return true;
    else return false;
  }
}

template <class T>
  // Maximum:模板版 max。
T Maximum(const T& v1, const T& v2)
{
  return v1 > v2 ? v1 : v2;
}



//----------------------------------------------------------------------------
//  some random number functions.
//----------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// 随机数一组:RandInt[x,y]、RandFloat[0,1]、RandInRange[x,y]、
// RandBool(50/50)、RandomClamped(-1,1);RandGaussian 是 Box-Muller 法产生正态分布样本。
//--------------------------------------------------------------------------------
//returns a random integer between x and y
inline int   RandInt(int x,int y)
{
  assert(y>=x && "<RandInt>: y is less than x");
  return rand()%(y-x+1)+x;
}

//returns a random double between zero and 1
inline double RandFloat()      {return ((rand())/(RAND_MAX+1.0));}

inline double RandInRange(double x, double y)
{
  return x + RandFloat()*(y-x);
}

//returns a random bool
inline bool   RandBool()
{
  if (RandFloat() > 0.5) return true;

  else return false;
}

//returns a random double in the range -1 < n < 1
inline double RandomClamped()    {return RandFloat() - RandFloat();}


//returns a random number with a normal distribution. See method at
//http://www.taygeta.com/random/gaussian.html
inline double RandGaussian(double mean = 0.0, double standard_deviation = 1.0)
{				        
	double x1, x2, w, y1;
	static double y2;
	static int use_last = 0;

	if (use_last)		        /* use value from previous call */
	{
		y1 = y2;
		use_last = 0;
	}
	else
	{
		do 
    {
			x1 = 2.0 * RandFloat() - 1.0;
			x2 = 2.0 * RandFloat() - 1.0;
			w = x1 * x1 + x2 * x2;
		}
    while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}

	return( mean + y1 * standard_deviation );
}



//-----------------------------------------------------------------------
//  
//  some handy little functions
//-----------------------------------------------------------------------


  // Sigmoid:1/(1+e^(-x/response)),把任意实数压到 0..1(神经网络/行为权重用)。
inline double Sigmoid(double input, double response = 1.0)
{
	return ( 1.0 / ( 1.0 + exp(-input / response)));
}


//returns the maximum of two values
template <class T>
  // MaxOf/MinOf:模板版 max/min。
inline T MaxOf(const T& a, const T& b)
{
  if (a>b) return a; return b;
}

//returns the minimum of two values
template <class T>
inline T MinOf(const T& a, const T& b)
{
  if (a<b) return a; return b;
}


//clamps the first argument between the second two
template <class T, class U, class V>
  // Clamp:把 arg 钳制到 [minVal, maxVal] 区间(速度/位置越界保护)。
inline void Clamp(T& arg, const U& minVal, const V& maxVal)
{
  assert ( ((double)minVal < (double)maxVal) && "<Clamp>MaxVal < MinVal!");

  if (arg < (T)minVal)
  {
    arg = (T)minVal;
  }

  if (arg > (T)maxVal)
  {
    arg = (T)maxVal;
  }
}


//rounds a double up or down depending on its value
  // Rounded:四舍五入(小数 >=0.5 进 1);RoundUnderOffset 可自定义阈值。
inline int Rounded(double val)
{
  int    integral = (int)val;
  double mantissa = val - integral;

  if (mantissa < 0.5)
  {
    return integral;
  }

  else
  {
    return integral + 1;
  }
}

//rounds a double up or down depending on whether its 
//mantissa is higher or lower than offset
inline int RoundUnderOffset(double val, double offset)
{
  int    integral = (int)val;
  double mantissa = val - integral;

  if (mantissa < offset)
  {
    return integral;
  }

  else
  {
    return integral + 1;
  }
}

//compares two real numbers. Returns true if they are equal
  // isEqual:浮点相等判断——差的绝对值 < 1E-12 就算相等(重载了 float/double 两版)。
inline bool isEqual(float a, float b)
{
  if (fabs(a-b) < 1E-12)
  {
    return true;
  }

  return false;
}

inline bool isEqual(double a, double b)
{
  if (fabs(a-b) < 1E-12)
  {
    return true;
  }

  return false;
}


template <class T>
  // Average:向量平均值;StandardDeviation:标准差(统计实验用)。
inline double Average(const std::vector<T>& v)
{
  double average = 0.0;
  
  for (unsigned int i=0; i < v.size(); ++i)
  {    
    average += (double)v[i];
  }

  return average / (double)v.size();
}


inline double StandardDeviation(const std::vector<double>& v)
{
  double sd      = 0.0;
  double average = Average(v);

  for (unsigned int i=0; i<v.size(); ++i)
  {     
    sd += (v[i] - average) * (v[i] - average);
  }

  sd = sd / v.size();

  return sqrt(sd);
}


template <class container>
  // DeleteSTLContainer:遍历 vector/list 把每个指针 delete 并置 NULL;
  // DeleteSTLMap:遍历 map,delete 它的 value。
inline void DeleteSTLContainer(container& c)
{
  for (container::iterator it = c.begin(); it!=c.end(); ++it)
  {
    delete *it;
    *it = NULL;
  }
}

template <class map>
inline void DeleteSTLMap(map& m)
{
  for (map::iterator it = m.begin(); it!=m.end(); ++it)
  {
    delete it->second;
    it->second = NULL;
  }
}





#endif