//==============================================================================================
//【文件说明】BaseGameEntity.cpp —— 基类"细节"的实现文件
//
//【这个文件是干什么的?】
//  头文件 BaseGameEntity.h 里只写了"有什么"(声明),本文件负责写"具体怎么做"(实现):
//    ① 给静态计数器 m_iNextValidID 赋初值 0;
//    ② 实现私有工具函数 SetID():检查编号合法、记录编号、推进计数器。
//
//【与相关文件的关系】
//  1. BaseGameEntity.h —— 本文件第一行就包含它(声明都来自这里);
//  2. Miner.h / MinersWife.h —— 它们的构造函数会通过"父类构造函数"
//     BaseGameEntity(id) 间接调用 SetID(),从而拿到各自唯一的编号;
//  3. main.cpp —— 创建 Bob(0) 和 Elsa(1) 时触发上面这条链。
//
//【执行顺序小剧场】程序一启动,创建 Bob:
//    main → new Miner(ent_Miner_Bob) → Miner 的构造函数 → 先调父类构造函数
//    BaseGameEntity(0) → 函数体 SetID(0) → 检查 0>=0 通过 → m_ID=0、
//    m_iNextValidID=1;接着创建 Elsa → SetID(1) → 1>=1 通过 → m_ID=1、
//    m_iNextValidID=2。这就是"每个角色编号唯一"的保障。
//==============================================================================================

// 包含头文件:拿到 BaseGameEntity 类的声明(以及 std::string、Telegram 等)。
#include "BaseGameEntity.h"
// <cassert>:C 标准库 assert(断言)宏的头文件。尖括号表示系统目录查找。
// 下面 SetID 里的 assert 就来自这里。
#include <cassert>



//--------------------------------------------------------------------------------
//【静态成员变量必须在 .cpp 里定义一次】int BaseGameEntity::m_iNextValidID = 0;
//  头文件里只写了 static int m_iNextValidID;(声明),C++ 规定静态成员
//  还必须在某个 .cpp 文件里"定义"一次并赋初值,否则链接时会报
//  "unresolved external symbol"(找不到符号)错误。
//  语法拆解:
//    int           :类型,整数;
//    BaseGameEntity::m_iNextValidID :域作用符 :: 表示"这是 BaseGameEntity 类
//                  里的 m_iNextValidID"(不是别的类的同名变量);
//    = 0           :初值,从 0 开始——第一个角色拿编号 0。
//  注意:整个工程只允许出现这一次定义,其他文件只能"用",不能再定义。
//--------------------------------------------------------------------------------
int BaseGameEntity::m_iNextValidID = 0;



//----------------------------- SetID -----------------------------------------
//
//  this must be called within each constructor to make sure the ID is set
//  correctly. It verifies that the value passed to the method is greater
//  or equal to the next valid ID, before setting the ID and incrementing
//  the next valid ID
//-----------------------------------------------------------------------------
// ↓↓↓ 原文翻译【SetID —— 设置编号】:
//   必须在每个构造函数里调用,确保编号设置正确。它会先检查传入的值
//   是否大于等于"下一个有效编号",然后再设置编号并递增下一个有效编号。
//   作用:保证每个角色拿到的编号都是新的、不重复的。
//-----------------------------------------------------------------------------
void BaseGameEntity::SetID(int val)
{
  //make sure the val is equal to or greater than the next available ID
  //(原文注释:确保 val 大于等于下一个可用的编号)

  // 【assert 断言】assert(条件) —— 调试期的"安全守卫":
  //   条件为真 → 什么都不做,继续往下走;
  //   条件为假 → 程序立刻报错并终止,并打印括号里的提示文字。
  //   (val >= m_iNextValidID):本角色要的编号必须不小于"下一个可用编号",
  //   否则说明有人乱填编号(比如重复用了 0),直接停下喊救命。
  //   && "字符串" :&& 是"并且"。这里字符串不是逻辑条件,而是"出错时显示
  //   的提示语"(assert 的第二个用法,纯为报错信息更友好)。
  assert ( (val >= m_iNextValidID) && "<BaseGameEntity::SetID>: invalid ID");

  // 把检查通过的编号存入本对象的私有成员 m_ID(以后用 ID() 读取)。
  m_ID = val;
    
  // 更新计数器:下一个可用编号 = 刚分配出去的编号 + 1。
  // 这样下次 new 角色时,val 必须 >= 这个新值,天然避免编号重复。
  m_iNextValidID = m_ID + 1;
}
