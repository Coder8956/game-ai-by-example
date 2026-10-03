//==============================================================================================
//【文件说明】MinersWifeOwnedStates.cpp —— 妻子 3 个状态的"剧本正文"
//
//【这个文件是干什么的?】
//  妻子艾尔莎的每一幕"谁说了什么台词、干了什么事、什么时候切到下一幕",
//  全部写在这里(与矿工的 MinerOwnedStates.cpp 地位相同)。3 个状态:
//    WifesGlobalState::Execute —— 全局状态:掷骰子,1/10 概率想上厕所;
//    DoHouseWork::Execute      —— 做家务:随机三选一(拖地/洗碗/铺床);
//    VisitBathroom::Execute    —— 上厕所(blip):上完 RevertToPreviousState
//                                 退回被打断的家务状态。
//  状态机的执行顺序:每步先全局状态、后当前状态(见 StateMachine.h::Update)。
//
//【与相关文件的关系】
//  MinersWifeOwnedStates.h —— 本文件实现其中声明的 3 个状态类(必须先包含它);
//  MinersWife.h / MinersWife.cpp —— 参数 wife 就是"正经历这个状态的妻子";
//                        切换/退回通过 wife->GetFSM()->...(状态机)完成;
//  Locations.h / EntityNames.h —— (Locations 是习惯性包含;EntityNames 提供
//                        GetNameOfEntity 把编号翻译成名字 "Elsa" 打印台词);
//  misc/Utils.h          —— (经 MinersWife.h 间接包含)随机数函数
//                        RandFloat()/RandInt() 来自这个公共目录的工具库;
//  <iostream>            —— cout(控制台输出流)。
//
//【一次典型的"上厕所 blip"】
//  main 调 Elsa.Update() → 状态机 Update() → ①全局状态 Execute:
//  RandFloat() 掷出 0~1 的随机数,< 0.1(即 1/10 概率)→ ChangeState 切到
//  ③上厕所 → ③的 Enter 念台词 → 下一步 ③的 Execute:"上完了"+退回上一状态
//  → ②做家务继续。整个过程家务状态"毫不知情"——这正是 blip 的妙处。
//==============================================================================================

// 本文件要实现的 3 个状态类的"说明书"(类声明)。
#include "MinersWifeOwnedStates.h"
// 妻子类的完整定义:本文件要调用妻子的公有函数(GetFSM 等)。
#include "MinersWife.h"
// 地点枚举(本文件没直接用到,习惯性包含)。
#include "Locations.h"
// 编号→名字函数 GetNameOfEntity(打印台词时显示 "Elsa")。
#include "EntityNames.h"

// <iostream>:C++ 标准输入输出库,cout 定义在这里。
#include <iostream>
// using 声明:引进 std 命名空间里的 cout,以后直接写 cout。
using std::cout;

//--------------------------------------------------------------------------------
//【条件编译】原理与矿工版完全相同(详见 MinerOwnedStates.cpp 的注释):
// 默认 TEXTOUTPUT 未定义,下面 4 行跳过;定义后输出改写进文件。
//--------------------------------------------------------------------------------
#ifdef TEXTOUTPUT
#include <fstream>
extern std::ofstream os;
#define cout os
#endif

//-----------------------------------------------------------------------Global state
//(原文分隔注释:下面开始【全局状态】的方法实现)

//--------------------------------------------------------------------------------
//【单例入口】获取"全局状态"的唯一实例(原理详见 MinerOwnedStates.cpp
// 中挖矿状态 Instance() 的注释:函数内 static 只构造一次,返回其地址)。
//--------------------------------------------------------------------------------
WifesGlobalState* WifesGlobalState::Instance()
{
  static WifesGlobalState instance;

  return &instance;
}


//--------------------------------------------------------------------------------
//【全局状态每步执行:掷骰子决定要不要上厕所】
//   参数 wife:正被本状态"监护"的妻子。这个函数每步都会被状态机调用
//  (先于当前状态),但大多数时候它"什么都不做"。
//--------------------------------------------------------------------------------
void WifesGlobalState::Execute(MinersWife* wife)
{
  //1 in 10 chance of needing the bathroom
  //(原文注释:十分之一的机会需要上厕所)

  // RandFloat():来自公共库 Common\misc\Utils.h 的随机数函数,
  // 返回 0.0~1.0 之间的随机小数(均匀分布)。
  // "<"读作"小于":随机数落在 [0, 0.1) 区间的概率正好是 1/10
  // ——这就是"1 in 10"的掷骰子写法。
  // (小知识:rand() 若不先 srand 播种,每次运行产生同一串"随机"数,
  //  所以每场演示的剧情其实完全一样——原作如此,零改动保留。)
  if (RandFloat() < 0.1)
  {
    // 掷中 → 切换到"上厕所"状态(blip 开始):
    // wife->GetFSM() 取出妻子的状态机,->ChangeState(...) 执行切换
    // (状态机会先把"做家务"存档为上一状态,见 StateMachine.h)。
    wife->GetFSM()->ChangeState(VisitBathroom::Instance());
  }
}

//---------------------------------------DoHouseWork

//(原文分隔注释:下面开始【做家务状态】的方法实现)

//--------------------------------------------------------------------------------
//【单例入口】获取"做家务状态"的唯一实例(原理同上)。
//--------------------------------------------------------------------------------
DoHouseWork* DoHouseWork::Instance()
{
  static DoHouseWork instance;

  return &instance;
}


//--------------------------------------------------------------------------------
//【Enter:进入"家务"状态】空函数体——家务是妻子的常态,无需开场白。
//--------------------------------------------------------------------------------
void DoHouseWork::Enter(MinersWife* wife)
{
}


//--------------------------------------------------------------------------------
//【Execute:家务状态每步执行】从三件家务里随机挑一件,念对应台词。
//--------------------------------------------------------------------------------
void DoHouseWork::Execute(MinersWife* wife)
{
  // RandInt(0,2):公共库的随机整数函数,返回 0~2 之间(含两端)的随机数。
  // switch(变量):多路分支——拿随机数与每个 case 逐个比较,命中哪段执行哪段
  // (原理详见 EntityNames.h 中 GetNameOfEntity 的注释)。
  switch(RandInt(0,2))
  {
  // 掷出 0:
  case 0:

    // 台词:Moppin' the floor(拖地板)。
    cout << "\n" << GetNameOfEntity(wife->ID()) << ": Moppin' the floor";

    // break:跳出 switch——干完这件就结束本步,绝不再往下穿到别的 case
    // (没有 break 的话会继续执行 case 1、case 2 的代码,叫"贯穿"陷阱)。
    break;

  // 掷出 1:
  case 1:

    // 台词:Washin' the dishes(洗碗)。
    cout << "\n" << GetNameOfEntity(wife->ID()) << ": Washin' the dishes";

    break;

  // 掷出 2:
  case 2:

    // 台词:Makin' the bed(铺床)。
    cout << "\n" << GetNameOfEntity(wife->ID()) << ": Makin' the bed";

    break;
  }
}

//--------------------------------------------------------------------------------
//【Exit:离开"家务"状态】空函数体——被上厕所"打断"无需告别词。
//--------------------------------------------------------------------------------
void DoHouseWork::Exit(MinersWife* wife)
{
}



//------------------------------------------------------------------------VisitBathroom
//(原文分隔注释:下面开始【上厕所状态】的方法实现)

//--------------------------------------------------------------------------------
//【单例入口】获取"上厕所状态"的唯一实例(原理同上)。
//--------------------------------------------------------------------------------
VisitBathroom* VisitBathroom::Instance()
{
  static VisitBathroom instance;

  return &instance;
}


//--------------------------------------------------------------------------------
//【Enter:进入"厕所"状态时执行一次】念台词:走去卫生间。
// 台词:Walkin' to the can. Need to powda mah pretty li'lle nose
//      (去趟卫生间。得给俺滴漂亮小鼻子补补粉啦)。
// 注:"can(罐子)"是英语俚语"厕所";"powda"是 powder(扑粉)的方言拼法。
//--------------------------------------------------------------------------------
void VisitBathroom::Enter(MinersWife* wife)
{  
  cout << "\n" << GetNameOfEntity(wife->ID()) << ": Walkin' to the can. Need to powda mah pretty li'lle nose"; 
}


//--------------------------------------------------------------------------------
//【Execute:处于"厕所"状态期间的每步行动】念一句"上完了",
// 然后立刻退回上一状态——blip 结束,回到被打断的家务。
//--------------------------------------------------------------------------------
void VisitBathroom::Execute(MinersWife* wife)
{
  // 台词:Ahhhhhh! Sweet relief!(啊——!真舒坦!)
  cout << "\n" << GetNameOfEntity(wife->ID()) << ": Ahhhhhh! Sweet relief!";

  // 【blip 的收尾】RevertToPreviousState():退回上一状态(实现见
  // StateMachine.h——它就是再调一次 ChangeState 切回存档的旧状态)。
  // 上厕所到此结束,家务继续,仿佛什么都没发生过。
  wife->GetFSM()->RevertToPreviousState();
}

//--------------------------------------------------------------------------------
//【Exit:离开"厕所"状态时执行一次】念台词:离开卫生间。
// 台词:Leavin' the Jon("Jon"是"john"的拼法,英语俚语"厕所")。
//--------------------------------------------------------------------------------
void VisitBathroom::Exit(MinersWife* wife)
{
  cout << "\n" << GetNameOfEntity(wife->ID()) << ": Leavin' the Jon";
}