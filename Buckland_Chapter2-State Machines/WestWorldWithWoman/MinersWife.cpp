//==============================================================================================
//【文件说明】MinersWife.cpp —— "矿工妻子"类的实现(兑现 MinersWife.h 的承诺)
//
//【这个文件是干什么的?】
//  只有一个函数:MinersWife::Update()——妻子的"心跳"。每被 main 调用一次
//  就活一步:给台词染上绿色(与矿工的红色区分),然后把决策权交给状态机。
//  【对比矿工】Miner::Update 里还有"口渴值 +1"这种固定消耗,妻子没有——
//  她的行为完全由状态类(全局状态 + 当前状态)自己决定。
//
//【与相关文件的关系】
//  MinersWife.h        —— 本文件实现它声明的类,第一件事是包含它;
//  misc/ConsoleUtils.h —— (经 MinersWife.h 间接包含)SetTextColor 来自这里;
//  StateMachine.h      —— m_pStateMachine->Update() 驱动妻子版状态机:
//                        先执行全局状态(WifesGlobalState:掷骰子决定要不要
//                        上厕所),再执行当前状态(DoHouseWork:随机做家务),
//                        两个状态类的实现在 MinersWifeOwnedStates.cpp;
//  main.cpp            —— 循环里每圈调用 Elsa.Update(),驱动妻子的状态机。
//==============================================================================================

// 包含自己的头文件:实现前必须先见到"妻子类长什么样"。
#include "MinersWife.h"


//--------------------------------------------------------------------------------
//【妻子的"心跳":每步更新】实现在类外,故用"MinersWife::"标明归属
// (双冒号原理详见 Miner.cpp 的文件头注释)。
//--------------------------------------------------------------------------------
void MinersWife::Update()
{
  //set text color to green
  //(原文注释:把文字颜色设为绿色)

  // SetTextColor:设置控制台文字颜色(工具函数见 ConsoleUtils.h)。
  // FOREGROUND_GREEN=绿色,FOREGROUND_INTENSITY=高亮加粗,"|"按位或
  // 把两个开关标志合并(按位或的详解见 Miner.cpp::Update 的注释)。
  // 妻子台词=亮绿色,矿工台词=亮红色,屏幕上一眼可分。
  SetTextColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
 
  // 把决策权交给状态机:先全局状态(1/10 概率想上厕所)、再当前状态
  // (做家务)——每步各执行一次 Execute(流程详见 StateMachine.h::Update)。
  m_pStateMachine->Update();
}






