//==============================================================================================
//【程序总览】WestWorldWithMessaging —— 《Programming Game AI by Example》
//  (中文译名《游戏人工智能编程案例精粹》)第 2 章"状态机"的第三个示例程序:
//  "带消息传递的西部世界"。本示例在 WestWorld1 的基础上,演示游戏 AI 里
//  第二个经典设计模式:★ 消息传递(Message Passing)★。
//
//【游戏剧情】矿工 Bob 和妻子 Elsa 住在一个小屋。Bob 过着循环人生:
//   睡觉 → 挖矿 → 存钱 → 回家睡觉……;而这次,角色之间会"互相传话":
//   · Bob 回家时给 Elsa 发消息"亲爱的我回来了" → Elsa 放下家务去炖菜;
//   · Elsa 炖菜需要时间,她给自己发一条"延迟 1.5 秒"的"炖菜好了"消息;
//   · 到点后 Elsa 把"炖菜好了"发给 Bob → 正在睡觉的 Bob 起床吃炖菜,
//     吃完再接着睡/干活。程序每 0.8 秒打印一屏"小剧场",共演 30 幕。
//
//【整个工程的文件地图及相互关系】(带 ▶ 的箭头表示"包含/使用"方向)
//   main.cpp(本文件:程序唯一入口——创建 Bob 和 Elsa,上户口,循环驱动)
//      │
//      ├────▶ Miner.h ── Miner.cpp 是它的实现
//      │           │ 矿工类:数据(地点/金块/存款/口渴/疲劳)+ 状态机指针;
//      │           └ 继承 BaseGameEntity.h;包含 Locations.h;持有
//      │             StateMachine<Miner> 状态机(来自 Common\fsm\)
//      ├────▶ MinersWife.h ── MinersWife.cpp 是它的实现
//      │           妻子类:数据(地点/在做饭吗)+ 状态机指针;
//      │           继承 BaseGameEntity.h;含"全局状态"(矿工没有)。
//      ├────▶ BaseGameEntity.h ── BaseGameEntity.cpp 是它的实现
//      │           所有角色的"祖宗类":发唯一编号 ID,强制实现 Update()
//      │           和 HandleMessage()(本示例新加:收消息接口)。
//      ├────▶ EntityManager.h ── EntityManager.cpp 是它的实现
//      │           ★ 本示例新加:角色的"户口本"(编号→指针),单例 EntityMgr;
//      ├────▶ MessageDispatcher.h ── MessageDispatcher.cpp 是它的实现
//      │           ★ 本示例新加:消息"邮局",单例 Dispatch;
//      │           管即时消息 + 延迟消息队列;
//      ├────▶ Locations.h    —— 地点枚举:小屋/金矿/银行/酒吧;
//      ├────▶ EntityNames.h  —— 角色编号(0=Bob,1=Elsa)+ 编号→名字;
//      ├────▶ MessageTypes.h —— ★ 新加:消息编号(我回来了/炖菜好了);
//      │
//      │      状态接口与状态实现(由 Miner.h / MinersWife.h 间接包含使用):
//      │      Common\fsm\State.h —— 状态接口(Enter/Execute/Exit/OnMessage);
//      │      Common\fsm\StateMachine.h —— 状态机模板(当前/上一/全局状态);
//      │      Common\Messaging\Telegram.h —— ★ 消息"信封"结构;
//      │      MinerOwnedStates.h ─ .cpp —— 矿工 5 个状态(挖矿/存钱/睡觉/喝酒/吃炖菜);
//      │      MinersWifeOwnedStates.h ─ .cpp —— 妻子 4 个状态(全局/家务/厕所/炖菜);
//      │      Common\Time\CrudeTimer.h ─ .cpp —— 时钟单例 Clock(消息时间戳);
//      │      Common\misc\ConsoleUtils.h —— 控制台工具(文字颜色、按任意键);
//      │      Common\misc\Utils.h —— 随机数工具(RandFloat/RandInt)。
//
//【一次完整执行的调用流程】(建议按此顺序阅读各文件)
//   ① main() 创建 Bob(new Miner(0))和 Elsa(new Miner(1)),两人通过
//      构造函数搭好各自的状态机:Bob 初始状态=回家睡觉;Elsa 初始状态=
//      做家务 + 全局状态=全局状态;
//   ② 用 EntityMgr->RegisterEntity() 给两人"上户口"(凭编号可查到指针);
//   ③ 主循环 30 次:Bob->Update()(口渴+1,状态机跑当前状态)→
//      Elsa->Update()(全局状态先跑,再跑当前状态)→
//      Dispatch->DispatchDelayedMessages()(把到期的延迟消息投递出去)→
//      Sleep(800)(暂停 0.8 秒);
//   ④ Bob 回到家(Enter 睡觉状态)时:给 Elsa 发"我回来了"(即时消息)→
//      邮局直接投递 → Elsa 的全局状态 OnMessage 收到 → 切到炖菜状态;
//   ⑤ Elsa 炖菜:给自己发"延迟 1.5 秒"的"炖菜好了"→ 邮局存进队列;
//   ⑥ 若干帧后,③ 里的 DispatchDelayedMessages 发现消息到期 → 投递给
//      Elsa 的炖菜状态 OnMessage → 她把"炖菜好了"即时发给 Bob;
//   ⑦ Bob 在睡觉状态 OnMessage 收到 → 切到吃炖菜 → 吃完回上一个状态;
//   ⑧ 30 幕演完,delete 两人,按任意键退出。
//==============================================================================================

// <fstream>:文件输入输出流(配合 TEXTOUTPUT 把输出写进 output.txt)。
#include <fstream>
// <time.h>:C 标准库时间头文件。下面 srand((unsigned)time(NULL)) 用它
// 取当前时间当作随机数种子。
#include <time.h>

// 本工程自己的头文件(关系见上面的文件地图):
// 地点枚举(习惯性包含)。
#include "Locations.h"
// 矿工类:下面 new Miner(ent_Miner_Bob) 要用。
#include "Miner.h"
// 妻子类:下面 new MinersWife(ent_Elsa) 要用。
#include "MinersWife.h"
// 户口本单例:RegisterEntity() 上户口、宏 EntityMgr。
#include "EntityManager.h"
// 邮局单例:DispatchDelayedMessages()、宏 Dispatch。
#include "MessageDispatcher.h"
// 控制台工具:PressAnyKeyToContinue()、SetTextColor()。
#include "misc/ConsoleUtils.h"
// 角色编号:ent_Miner_Bob(=0)、ent_Elsa(=1)。
#include "EntityNames.h"


// 【全局文件流对象】std::ofstream os;
//   ofstream = output file stream(输出文件流);
//   仅当 TEXTOUTPUT 被定义时,os.open("output.txt") 打开文件,
//   各 .cpp 里 #define cout os 把输出重定向到文件。
std::ofstream os;

//--------------------------------------------------------------------------------
//【int main() —— 程序的入口函数】
//   int  :返回整数给操作系统(0 = 正常结束);
//   main :C++ 规定死的入口名字,操作系统启动程序后从这里开始;
//   ()   :空参数表(不接收命令行参数);
//   { }  :函数体,逐行执行。
//--------------------------------------------------------------------------------
int main()
{
//define this to send output to a text file (see locations.h)
//(原文注释:定义这个宏,把输出发送到文本文件(见 locations.h))

// #ifdef TEXTOUTPUT:如果 Locations.h 里取消 //#define TEXTOUTPUT 的注释,
// 就执行 os.open("output.txt") 打开输出文件;否则整段跳过。
#ifdef TEXTOUTPUT
  os.open("output.txt");
#endif

  //seed random number generator
  //(原文注释:播种随机数生成器)

  // srand(...):设置随机数种子。time(NULL) 取当前秒数;
  // (unsigned) 是强制类型转换,把秒数转成无符号整数;NULL 表示"取当前时间"。
  // 种子不同 → 后面的 RandFloat()/RandInt() 产生的随机序列就不同,
  // 这样每次运行剧情会略有差异。
  srand((unsigned) time(NULL));

  //create a miner
  //(原文注释:创建一个矿工)

  // new Miner(ent_Miner_Bob):在堆上创建矿工对象,参数=编号 0。
  // Miner* 是指向矿工的指针;Bob 是给指针起的名字。
  // 构造函数里自动:分配编号、搭好状态机、初始状态=回家睡觉。
  Miner* Bob = new Miner(ent_Miner_Bob);

  //create his wife
  //(原文注释:创建他的妻子)

  // new MinersWife(ent_Elsa):创建妻子对象,参数=编号 1。
  // 构造函数里自动:分配编号、搭好状态机、初始状态=做家务+全局状态。
  MinersWife* Elsa = new MinersWife(ent_Elsa);

  //register them with the entity manager
  //(原文注释:把他们注册到实体管理器)

  // EntityMgr:宏,即 EntityManager::Instance()(户口本单例);
  // ->RegisterEntity(Bob):给 Bob 上户口(编号 0 → Bob 指针)。
  // 之后消息系统凭编号 0 就能找到 Bob。
  EntityMgr->RegisterEntity(Bob);
  // 给 Elsa 上户口(编号 1 → Elsa 指针)。
  EntityMgr->RegisterEntity(Elsa);

  //run Bob and Elsa through a few Update calls
  //(原文注释:让 Bob 和 Elsa 跑过几次 Update 调用)

  // 【for 循环 30 次】int i=0 初始化;i<30 条件;++i 每轮后自增。
  // 30 轮 = 30 个"时间片",每片 0.8 秒,共约 24 秒的"小剧场"。
  for (int i=0; i<30; ++i)
  { 
    // Bob 的心跳:口渴 +1 → 状态机跑当前状态(可能切换状态/发消息)。
    Bob->Update();
    // Elsa 的心跳:全局状态先跑(可能随机去厕所)→ 当前状态跑。
    Elsa->Update();

    //dispatch any delayed messages
    //(原文注释:派发任何延迟消息)

    // Dispatch:宏,即 MessageDispatcher::Instance()(邮局单例);
    // ->DispatchDelayedMessages():把队列里"到期的延迟消息"投递出去
    // (比如 Elsa 的"炖菜好了"延迟消息,到点就在这里被派发)。
    Dispatch->DispatchDelayedMessages();

    // Sleep:Windows 系统函数(经 ConsoleUtils.h 间接包含 windows.h)。
    // 暂停 800 毫秒,让输出像逐幕播放的小剧场。
    Sleep(800);
  }

  //tidy up
  //(原文注释:收拾干净)

  // delete Bob:销毁矿工对象,释放 new 分配的内存。
  // 会自动调用 Miner 的析构函数(delete 状态机)。
  delete Bob;
  // delete Elsa:销毁妻子对象,释放内存(同样先走析构)。
  delete Elsa;

  //wait for a keypress before exiting
  //(原文注释:退出前等待按键)

  // 来自 ConsoleUtils.h:打印 "Press any key to continue",
  // 然后等待用户按任意键,防止命令行窗口一闪而过。
  PressAnyKeyToContinue();


  // 返回 0 给操作系统:约定"程序正常结束"。
  return 0;
}






