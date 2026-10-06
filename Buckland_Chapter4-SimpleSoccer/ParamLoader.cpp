//==============================================================================================
//【文件说明】ParamLoader.cpp —— 单例入口 Instance() 的实现
//
//【这个文件是干什么的?】
//  ParamLoader.h 声明了静态函数 Instance(),本文件给出它的函数体。
//  它用"函数内 static 局部变量"的写法,保证参数对象在程序里只被创建一次。
//
//【本文件包含了谁?】
//  "ParamLoader.h" —— 自己的类声明。
//
//【C++ 小课堂:函数内 static 局部变量 = 惰性单例】
//  static ParamLoader instance; 写在函数体里:第一次调用本函数时才构造 instance,
//  之后所有调用都复用同一个对象(程序结束时自动析构)。return &instance 返回它的地址。
//==============================================================================================
#include "ParamLoader.h"

// 单例入口:返回全局唯一 ParamLoader 对象的指针。
// ParamLoader:: 是作用域解析符,表示这个 Instance 属于 ParamLoader 类。
ParamLoader* ParamLoader::Instance()
{
// static 局部变量:首次进入本函数时构造一次(这时才真正打开 Params.ini 读参数),
// 以后每次调用都返回同一个对象。这就是"懒汉式单例"。
  static ParamLoader instance;

// &instance:取 instance 的地址(指针),返回给调用者(对应函数返回类型 ParamLoader*)。
  return &instance;
}