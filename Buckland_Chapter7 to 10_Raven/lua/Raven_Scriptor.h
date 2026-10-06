//==============================================================================================
//【文件说明】lua\Raven_Scriptor.h —— 脚本读取器(单例)
//
//【这个文件是干什么的?】
//  游戏里大量参数(机器人速度/血/视野/权重等)不写死在代码里,而是放在 .lua 脚本文件里。
//  本类是个单例,封装了 Common 下的第三方 lua 解释器(lua-5.1.5 + luabind),
//  提供 script->GetDouble("Bot_MaxSpeed") 这种按名字读参数的接口。
//  全工程用宏 script 直接访问,见下面 #define。
//
//【注意】Common\lua-5.1.5 和 Common\luabind 是第三方库,不在本分片注释范围。
#ifndef RAVEN_SCRIPTOR_H
#define RAVEN_SCRIPTOR_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Scriptor
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   A Singleton Scriptor class for use with the Raven project
//(原文注释翻译:Raven 工程用的单例脚本读取器类)
//-----------------------------------------------------------------------------
#include "Script/scriptor.h"



#define script Raven_Scriptor::Instance()

//--------------------------------------------------------------------------------
// class Raven_Scriptor —— 继承 Common 下的 Scriptor 基类,只加了单例 Instance()。
class Raven_Scriptor : public Scriptor
{
private:
  
  Raven_Scriptor();

  //copy ctor and assignment should be private
  Raven_Scriptor(const Raven_Scriptor&);
  Raven_Scriptor& operator=(const Raven_Scriptor&);

public:

// Instance:拿单例指针。
  static Raven_Scriptor* Instance();

};

#endif

 
  

