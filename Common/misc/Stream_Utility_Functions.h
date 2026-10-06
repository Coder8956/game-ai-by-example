//==============================================================================================
//【文件说明】Stream_Utility_Functions.h —— 流(stream)上的小工具:类型转字符串等
//
//【这个文件是干什么的?】
//  4 个通用函数:
//    ttos<T>            —— 把任意类型 T 转成字符串(用 ostringstream 拼);
//    btos              —— bool 转 "true"/"false" 字符串;
//    GetValueFromStream<T> —— 从文件流读一个 T,读错类型抛异常;
//    WriteBitsToStream    —— 把数值按二进制 0/1 逐位打印(调试用)。
//
//【谁在使用这个文件?】
//  Region.h/Cgdi.h 等用 ttos 把数字转成文字画到屏幕上;SparseGraph 读盘用 GetValueFromStream。
//
//【本文件包含了谁?】
//  <sstream>/<string>/<iomanip> —— 字符串流/字符串/格式控制(setprecision)。
//==============================================================================================
#ifndef STREAM_UTILITY_FUNCTIONS
#define STREAM_UTILITY_FUNCTIONS
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Name:   Stream_Utility_Functions.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   various useful functions that operate on or with streams
//-----------------------------------------------------------------------------
#include <sstream>
#include <string>
#include <iomanip>


//------------------------------ ttos -----------------------------------------
//
//  convert a type to a string
//-----------------------------------------------------------------------------
template <class T>
  // ttos:用 ostringstream 把 t 按 fixed 小数精度转字符串,返回其内容。
inline std::string ttos(const T& t, int precision = 2)
{
  std::ostringstream buffer;

  buffer << std::fixed << std::setprecision(precision) << t;

  return buffer.str();
}

//------------------------------ ttos -----------------------------------------
//
//  convert a bool to a string
//-----------------------------------------------------------------------------
  // btos:bool → "true"/"false"。
inline std::string btos(bool b)
{
  if (b) return "true";
  return "false";
}

//--------------------------- GetValueFromStream ------------------------------
//
//  grabs a value of the specified type from an input stream
//-----------------------------------------------------------------------------
template <typename T>
  // GetValueFromStream:从文件流读 T;若流状态失败(类型不匹配)抛 runtime_error。
inline T GetValueFromStream(std::ifstream& stream)
{
  T val;

  stream >> val;

  //make sure it was the correct type
  if (!stream)
  {
    throw std::runtime_error("Attempting to retrieve wrong type from stream");
  }

  return val;
}

//--------------------------- WriteBitsToStream ------------------------------------
//
// writes the value as a binary string of bits
//-----------------------------------------------------------------------------
template <typename T>
  // WriteBitsToStream:从最高位到最低位逐位判断(val & mask),打 1/0;每 8 位插个空格。
void WriteBitsToStream(std::ostream& stream, const T& val)
{
  int iNumBits = sizeof(T) * 8;

  while (--iNumBits >= 0)
  {
    if ((iNumBits+1) % 8 == 0) stream << " ";
    unsigned long mask = 1 << iNumBits;
    if (val & mask) stream << "1";
    else stream << "0";
  }
}



#endif