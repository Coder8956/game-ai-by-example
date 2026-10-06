//==============================================================================================
//【文件说明】lua\Raven_Scriptor.cpp —— 脚本读取器的实现
//==============================================================================================
#include "Raven_Scriptor.h"

// Instance: Meyers 单例,函数内 static。
Raven_Scriptor* Raven_Scriptor::Instance()
{
  static Raven_Scriptor instance;

  return &instance;
}



// 构造函数:调基类,然后 RunScriptFile("Params.lua") 读参数脚本。
Raven_Scriptor::Raven_Scriptor():Scriptor()
{
  RunScriptFile("Params.lua");
}