//==============================================================================================
//【程序总览】WestWorldWithWoman —— 《Programming Game AI by Example》
//  (中文译名《游戏人工智能编程案例精粹》)第 2 章"状态机"的第二个示例程序。
//
// 这是运行在黑色"控制台窗口"里的超简易游戏,演示游戏 AI 最经典的设计模式:
// ★ 有限状态机(Finite State Machine,简称 FSM)★
//
//【游戏剧情】西部世界里住着一对夫妻:矿工 Bob 与妻子 Elsa(艾尔莎)。
//   矿工 Bob 的"人生"(与第一个示例 WestWorld1 相同):
//     挖金矿 → 口袋装满(3 块)→ 去银行存钱 → 存款够 5 块就回家睡觉
//     → 睡醒了再去挖矿 → …… 挖矿挖到口渴(5 点)先去酒吧买威士忌。
//   妻子 Elsa 的"人生"(本示例的新增看点):
//     平时在家随机做三件家务(拖地/洗碗/铺床);每一步都有 1/10 的概率
//     突然想上厕所——上完厕所自动回去继续做家务(这一"短暂插播"
//     的玩法术语叫"状态 blip",靠状态机的"全局状态+退回上一状态"实现)。
// 程序每 0.8 秒打印一行台词:矿工红色、妻子绿色,总共演 20 幕小剧场。
//
//【与 WestWorld1 的两大进化】
//   1. 状态机的逻辑从矿工类里抽成了通用模板类 StateMachine(人人可用);
//   2. 新增"全局状态 + 上一状态档案",玩法升级为"状态 blip"。
//
//【整个工程的文件地图及相互关系】(带 ▶ 的箭头表示"包含/使用"方向)
//   main.cpp(本文件:程序唯一入口——创建矿工+妻子,循环驱动两人生活 20 步)
//      │
//      ├────▶ Locations.h      —— 地点枚举:小屋/金矿/银行/酒吧。
//      │                        (main 没直接用到,包含它只是习惯写法)
//      ├────▶ Miner.h ─ Miner.cpp 是它的实现
//      │           ↑ Miner.h 又包含 BaseGameEntity.h(基类)、Locations.h、
//      │           │ MinerOwnedStates.h、StateMachine.h
//      │           └ 矿工类:位置/金块/存款/口渴/疲劳 + 状态机指针。
//      ├────▶ MinersWife.h ─ MinersWife.cpp 是它的实现
//      │           ↑ MinersWife.h 又包含 BaseGameEntity.h、Locations.h、
//      │           │ MinersWifeOwnedStates.h、StateMachine.h、misc/Utils.h
//      │           └ 妻子类:位置 + 状态机指针(初始状态=做家务,
//             另安装了全局状态——本工程独有)。
//      ├────▶ misc/ConsoleUtils.h —— 控制台小工具(文字颜色、按任意键继续)。
//      │           注意:它不在本文件夹,而在上上级的 Common\misc\ 公共目录,
//      │           靠工程"附加包含目录 ..\..\Common"的设置才能被找到。
//      ├────▶ EntityNames.h    —— 角色编号枚举(0=Bob,1=Elsa)
//      │                        + 把编号翻译成名字的 GetNameOfEntity()。
//      │
//      │      BaseGameEntity.h ─ BaseGameEntity.cpp 是它的实现
//      │           所有角色的"祖宗类"(基类):发唯一编号 ID,
//      │           并强制所有子类必须实现 Update()。
//      │           Miner 与 MinersWife 都继承它。
//      │      State.h(状态接口模板:Enter/Execute/Exit 三件套)
//      │           ▲ 被 MinerOwnedStates.h(矿工 4 状态)与
//      │           │   MinersWifeOwnedStates.h(妻子 3 状态)继承实现
//      │      StateMachine.h(通用状态机模板:当前/上一/全局三种状态,
//      │           切换三步曲 + 退回上一状态;由 Miner.h / MinersWife.h 使用,
//      │           main.cpp 没直接包含它)
//      │      MinerOwnedStates.h ─ MinerOwnedStates.cpp 是它的实现
//      │           矿工 4 状态:挖矿/存钱/睡觉/喝酒(都是单例)。
//      │      MinersWifeOwnedStates.h ─ MinersWifeOwnedStates.cpp 是它的实现
//      │           妻子 3 状态:全局状态(掷骰子)/做家务/上厕所(都是单例)。
//      │      misc/Utils.h —— 随机数函数 RandFloat/RandInt(公共目录,
//      │           妻子的"1/10 概率"和"随机家务"靠它;经 MinersWife.h
//      │           间接包含,main 没直接包含)。
//
//【一次完整循环的调用流程】(建议按此顺序阅读各文件)
//   ① main() 创建 Miner Bob(编号 0):出生在小屋、初始状态=回家睡觉;
//      创建 MinersWife Elsa(编号 1):出生在小屋、当前状态=做家务、
//      全局状态=妻子的全局状态。
//   ② main 循环(共 20 轮):先调 Bob.Update()——口渴 +1,再交给状态机:
//      当前状态的 Execute 干活,适时 GetFSM()->ChangeState(...) 切换状态;
//   ③ 再调 Elsa.Update()——交给状态机:先执行全局状态(1/10 概率切去
//      上厕所),再执行当前状态(随机做家务;上厕所后 RevertToPreviousState
//      退回家务);
//   ④ Sleep(800) 暂停 0.8 秒,回到 ②;
//   ⑤ 20 步演完 → PressAnyKeyToContinue() 等用户按键 → return 0 结束。
//==============================================================================================

//--------------------------------------------------------------------------------
// #include 是"预处理指令":编译前的预处理阶段,把引号/尖括号里那个文件的
// 全部内容原样复制粘贴到这一行所在的位置。
// 双引号 "xxx.h":先到本工程目录及工程设置的"附加包含目录"里找;
// 尖括号 <xxx.h>:只到编译器的系统/标准库目录里找(标准库都用尖括号)。
//--------------------------------------------------------------------------------

// <fstream>:文件流库——下面那个全局变量 os(往文件写东西的"输出流")
// 的类型 std::ofstream 定义在这里。
#include <fstream>
// 本工程自己的头文件:定义了 location_type 地点枚举(shack/goldmine/
// bank/saloon)。main 本身没直接用到地点,包含它是原作者的习惯写法。
#include "Locations.h"
// 矿工类的"说明书":不包含它,编译器不认识下面的 Miner 是什么。
#include "Miner.h"
// 妻子类的"说明书":不包含它,编译器不认识下面的 MinersWife 是什么。
#include "MinersWife.h"
// Common\misc\ 目录里的控制台工具:提供本文件用到的 Sleep 与
// PressAnyKeyToContinue。"misc/xxx.h" 表示"附加包含目录下的 misc 子文件夹
// 里的 xxx.h"(Sleep 本身是 windows.h 里的系统函数,经它被间接包含)。
#include "misc/ConsoleUtils.h"
// 角色编号枚举:下面创建两个角色时用的编号 ent_Miner_Bob(0)、
// ent_Elsa(1)定义在这里。
#include "EntityNames.h"



//--------------------------------------------------------------------------------
//【全局变量:文件输出流 os】
//   std::ofstream —— 类型:标准库(std 命名空间)的"输出文件流",
//                   可以把它理解成"一个打开后就能往里写字的文件";
//   os           —— 变量名(输出流 output stream 的缩写);
//   定义在所有函数之外 = 全局变量:整个工程任何 .cpp 都能(经 extern 声明)
//   使用它。它平时闲置,只有定义了 TEXTOUTPUT 宏(见 Locations.h 的开关),
//   下面 os.open("output.txt") 执行后,各状态文件里的 "#define cout os"
//   才会让全部打印改道写进 output.txt 文件。
//--------------------------------------------------------------------------------
std::ofstream os;

//--------------------------------------------------------------------------------
//【int main() —— 程序的入口函数】
//   int   :返回类型——本函数结束后返回一个整数给操作系统;
//   main  :名字是 C++ 规定死的,操作系统启动程序后就从这里开始执行;
//   ()    :空的参数表,表示不接收命令行参数
//           (也可以写 int main(int argc, char* argv[]) 来接收,本程序不需要);
//   { }   :函数体,花括号内是逐行执行的全部代码。
//--------------------------------------------------------------------------------
int main()
{
//define this to send output to a text file (see locations.h)
//(原文注释:定义这个宏,就把输出发送到文本文件里(见 locations.h 的开关))

//【条件编译】仅当 TEXTOUTPUT 宏已定义时,中间两行才会被编译
// (原理详见 MinerOwnedStates.cpp 的注释;默认未定义,整段跳过)。
#ifdef TEXTOUTPUT
  // 打开(创建)output.txt 文件,并与 os 输出流接通,准备接收输出。
  os.open("output.txt");
#endif

  //create a miner
  //(原文注释:创建一个矿工)

  // 【创建(实例化)矿工对象】
  //   Miner         —— 类名(相当于"图纸");
  //   Bob           —— 对象名(按图纸造出的"实物",这次直接取名 Bob);
  //   ent_Miner_Bob —— 传给构造函数的参数:矿工编号(来自 EntityNames.h,值为 0)。
  // 创建时自动调用 Miner 的构造函数(见 Miner.h):出生在小屋、金块 0、
  // 存款 0、口渴 0、疲劳 0,并 new 出一台状态机、安装初始状态=回家睡觉。
  Miner Bob(ent_Miner_Bob);

  //create his wife
  //(原文注释:创建他的妻子)

  // 【创建妻子对象】编号 ent_Elsa(=1)。构造函数(见 MinersWife.h):
  // 出生在小屋,new 出状态机,当前状态=做家务,全局状态=妻子的全局状态
  // (每步 1/10 概率想上厕所)。
  MinersWife Elsa(ent_Elsa);

  //run Bob and Elsa through a few Update calls
  //(原文注释:让 Bob 和 Elsa 经历几次 Update 调用)

  // 【for 循环:让花括号里的代码重复执行 20 次】
  //   int i=0  :循环开始前执行一次——定义计数器 i,赋初值 0;
  //   i<20     :每轮开始前检查,条件为"真"才继续循环;
  //   ++i      :每轮结束后执行——让 i 加 1("++"读作"自增")。
  //   三段用分号隔开。i 从 0 涨到 19,共执行 20 轮,模拟两人"生活"的
  //   20 个时间片(每人每片各走一步)。
  for (int i=0; i<20; ++i)
  { 
    // "点号"是成员访问运算符:对象名.函数名() 表示调用这个对象的成员函数。
    // 矿工的"心跳"(实现在 Miner.cpp):设红色、口渴 +1、把决策交给
    // 状态机的当前状态(挖矿/存钱/睡觉/喝酒,可能触发切换)。
    Bob.Update();

    // 妻子的"心跳"(实现在 MinersWife.cpp):设绿色、把决策交给状态机:
    // 先全局状态(1/10 概率切上厕所),再当前状态(随机家务)。
    Elsa.Update();

    // Sleep:Windows 系统函数(经 ConsoleUtils.h 间接包含的 windows.h 声明)。
    // 暂停当前程序 800 毫秒(0.8 秒)。加上这句,输出像逐幕播放的小剧场;
    // 没有它,几十行台词会瞬间刷完。
    Sleep(800);
  }

  //wait for a keypress before exiting
  //(原文注释:退出前等待按键)

  // 来自 ConsoleUtils.h 的工具函数:打印 "Press any key to continue",
  // 然后原地等待,直到用户按下键盘上任意一个键。
  // 作用:让命令行窗口在程序结束前停住,不会一闪而过。
  PressAnyKeyToContinue();

  // 把整数 0 返回给操作系统。约定俗成:0 表示"程序正常结束",非 0 表示出错。
  return 0;
}






