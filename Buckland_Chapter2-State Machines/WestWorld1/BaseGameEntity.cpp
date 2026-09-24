//==============================================================================================
//【文件说明】BaseGameEntity.cpp —— "游戏实体基类"的实现(兑现 BaseGameEntity.h 的承诺)
//
//【这个文件是干什么的?】
//  .h 里只写了"有什么"(声明),这个 .cpp 负责"具体怎么做"(实现):
//    1. 给静态成员 m_iNextValidID 一个"真身"并初始化为 0;
//    2. 写出 SetID() 登记编号的具体代码(检查合法性 + 保存 + 计数器前进)。
//
//【与相关文件的关系】
//  BaseGameEntity.h —— 本文件实现的类就声明在那里,所以第一件事是包含它;
//  Miner.h/.cpp     —— 矿工继承本类,矿工构造时传来的编号最终在这里登记;
//  main.cpp         —— 创建矿工时传的 ent_Miner_Bob(=0)就是这样拿到编号 0 的。
//
//【C++ 小课堂:为什么函数前要写"BaseGameEntity::"?】
//  " :: " 叫作用域解析运算符(scope resolution operator)。
//  "BaseGameEntity::SetID" 读作"BaseGameEntity 类里的那个 SetID"——
//  在类外写成员函数的实现时,必须用"类名::函数名"说明"这是谁家的函数",
//  否则编译器以为你在写一个不相干的散函数。
//==============================================================================================

// 包含自己类的头文件:实现前必须先见到"类长什么样"(有哪些成员声明)。
#include "BaseGameEntity.h"
// <cassert>:标准库的"断言"头文件,提供下面用到的 assert 宏。
#include <cassert>



//【静态成员的定义】
//  BaseGameEntity.h 里只"声明"了 static int m_iNextValidID(类内 static 成员
//  只领"出生证",不分配真正的存储空间),必须在类外这样"定义"一次:
//      int BaseGameEntity::m_iNextValidID = 0;
//  含义:定义"BaseGameEntity 类的 m_iNextValidID",从 0 开始编号。
//  全程序只有这一份(所有实体共享),所以第一个创建的实体编号是 0
//  (main.cpp 里矿工 Bob 传的正是 ent_Miner_Bob = 0)。
//  注意:静态成员不能在构造函数里初始化——它不随对象走,程序一启动就存在。
int BaseGameEntity::m_iNextValidID = 0;



//----------------------------- SetID -----------------------------------------
//
//  this must be called within each constructor to make sure the ID is set
//  correctly. It verifies that the value passed to the method is greater
//  or equal to the next valid ID, before setting the ID and incrementing
//  the next valid ID
//-----------------------------------------------------------------------------
//(原文注释大意:每个构造函数都必须调用它,确保编号设置正确。
//  它先验证传来的编号 ≥ "下一个可用编号",然后才保存编号并把计数器加 1。)

//--------------------------------------------------------------------------------
//【SetID 的实现】void:无返回值;参数 int val:要登记的编号。
//--------------------------------------------------------------------------------
void BaseGameEntity::SetID(int val)
{
  //make sure the val is equal to or greater than the next available ID
  //(原文注释:确保 val 大于等于下一个可用编号)

  // assert(条件):断言。条件为"真"→ 什么都不做,程序继续;
  // 条件为"假"→ 程序立刻中断,报出错的文件名和行号——专门用来在
  // 开发阶段抓逻辑错误的工具(只检查,不负责修复)。
  // 这里的条件: (val >= m_iNextValidID) —— 新编号不能小于计数器
  // (否则可能和已存在的实体编号撞车);
  // && —— "并且";断言条件后面 && 的字符串是断言失败时显示的提示文字。
  // 断言只在 Debug 版编译时生效(用了 NDEBUG 宏),Release 版会自动剔除。
  assert ( (val >= m_iNextValidID) && "<BaseGameEntity::SetID>: invalid ID");

  // 把编号存进本对象的成员变量 m_ID。
  m_ID = val;
    
  // 计数器前进:下一个可用编号 = 本编号 + 1,保证之后的实体不会重复用号。
  m_iNextValidID = m_ID + 1;
}
