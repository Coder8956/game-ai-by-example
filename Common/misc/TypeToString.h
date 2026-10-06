//==============================================================================================
//【文件说明】TypeToString.h —— "整数编号转字符串"的抽象接口
//
//【这个文件是干什么的?】
//  纯虚接口类:只规定一个函数 Convert(int enumeration) → string。
//  各工程继承它,把自己的消息/目标编号枚举翻译成可读字符串,
//  主要用于调试时在屏幕上显示目标名。
//
//【谁在使用这个文件?】
//  Common/Goals/Goal.h 的 RenderAtPos 接收 TypeToString* 参数;
//  各工程(如 Raven)提供自己的子类实现具体翻译表。
//==============================================================================================
#ifndef TYPE_TO_STRING_H
#define TYPE_TO_STRING_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Name:   TypeToString.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   an interface for a class that has a static
//          method for converting an int into a string. (useful when debugging
//          to convert enumerations)
//-----------------------------------------------------------------------------
#include <string>

  // 纯虚函数 Convert:子类必须实现"编号→字符串"的具体翻译。
class TypeToString
{
public:

  virtual std::string Convert(int enumeration)=0;
};

#endif