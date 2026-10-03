# WestWorld1 阅读代码流程指南

WestWorld1 是《Programming Game AI by Example》第 2 章的第一个示例工程。

一个黑色控制台小程序，用最少的代码演示游戏 AI 最经典的设计：

> **有限状态机（FSM）**

剧情：西部世界里住着矿工 Bob。程序每 0.8 秒执行一步，总共 20 步。
Bob 每一步干什么，完全由他"当前的状态"决定。

本指南回答一个问题：

> **拿到这 10 个源文件，按什么顺序读、每一步看什么、读到什么程度算过关？**

---

# 一、开工前 1 分钟：工程里有什么

```text
WestWorld1/
│
├── main.cpp                 ← 程序唯一入口：创建矿工，循环驱动 20 步
│
├── BaseGameEntity.h         ← 实体基类（声明）：发唯一编号 ID
├── BaseGameEntity.cpp       ← 实体基类（实现）：编号计数器 + SetID()
│
├── State.h                  ← 状态接口：规定状态必须有 Enter/Execute/Exit
│
├── Miner.h                  ← 矿工类（声明）：数据 + Update + ChangeState
├── Miner.cpp                ← 矿工类（实现）：构造/Update/ChangeState/小工具
│
├── MinerOwnedStates.h       ← 4 个状态类的声明（挖矿/存钱/睡觉/喝酒）
├── MinerOwnedStates.cpp     ← 4 个状态类的实现（台词+干活+切换，工程核心）
│
├── Locations.h              ← 地点枚举：shack / goldmine / bank / saloon
└── EntityNames.h            ← 角色编号枚举 + GetNameOfEntity()（编号转名字）
```

每个文件都不长，全部读完约 1.5～2 小时。
真正要啃的其实只有两个思想：

```text
多态    —— 状态指针指向谁，就执行谁的行为
状态机  —— 行为跟着状态走，"切换状态"就是"换指针"
```

---

# 二、先跑一遍，再看代码（第 0 步）

读代码之前，先让程序跑起来。

```text
若已编译：直接运行 Debug\WestWorld1.exe
```

看输出时只留意一件事：

```text
每行台词前都会打印角色名字（Miner Bob）
台词大致按 挖矿 → 存钱 → 睡觉 → 挖矿 → 喝酒 …… 循环
```

目的：让后面读到的每一段代码，都能在脑子里"对上号"。
看不懂输出没关系，读完第五、六章自然全懂。

---

# 三、核心：推荐阅读顺序

## 0. 四条阅读原则

```text
原则一：入口优先      —— 先看 main.cpp 搭起"程序骨架"，再往骨架上填肉
原则二：被依赖者优先  —— 先读"合同"（.h 声明），再读"兑现"（.cpp 实现）
原则三：先 .h 后 .cpp —— 知道"有什么"再研究"怎么做"
原则四：每步设里程碑  —— 带着问题读，读完必须能回答，答不出就重读
```

## 1. 阅读流程总览

```text
第 0 步  跑一遍程序，混个脸熟
   ↓
第 1 步  main.cpp            程序骨架：谁在驱动矿工生活？
   ↓
第 2 步  Locations.h         世界设定：矿工住在哪几个地方？
        EntityNames.h       角色设定：Bob 是几号？
   ↓
第 3 步  State.h             状态合同：想当"状态"必须会什么？
   ↓
第 4 步  BaseGameEntity.h    基类：为什么需要"祖宗类"？
        BaseGameEntity.cpp
   ↓
第 5 步  Miner.h             主角：矿工长什么样？
        Miner.cpp           状态机骨架：Update / ChangeState
   ↓
第 6 步  MinerOwnedStates.h  角色表：4 个状态类 + 单例写法
   ↓
第 7 步  MinerOwnedStates.cpp 剧本正文：台词、干活、切换（核心，读最慢）
   ↓
第 8 步  回到 main.cpp       闭环验收：脑内推演前 3 步输出，与实际对照
```

## 2. 第 1 步：main.cpp —— 搭骨架（10 分钟）

整个文件只看三段，其他（#include 列表、头注释）扫一眼即可：

```text
① Miner miner(ent_Miner_Bob);    创建矿工
② for (int i=0; i<20; ++i)       循环 20 次
   {
     miner.Update();             矿工活一步
     Sleep(800);                 暂停 0.8 秒（剧场效果）
   }
③ PressAnyKeyToContinue();       结束前等按键
```

要回答的问题：

```text
Q：谁在驱动矿工的"人生"？
A：main 的 for 循环 —— 每圈调一次 miner.Update()。
```

里程碑（读完的标志）：

> 能用一句话说出：**程序 = 创建一个矿工，让他活 20 步，每步调一次 Update()。**

## 3. 第 2 步：Locations.h + EntityNames.h —— 世界设定（5 分钟）

全工程最简单的两个文件，各只有几十行。

Locations.h：一个枚举，列出矿工世界的 4 个地点。

```text
shack     小屋（家，睡觉）
goldmine  金矿（上班，挖金子）
bank      银行（存金子）
saloon    酒吧（买威士忌解渴）
```

EntityNames.h：角色编号枚举 + 一个 switch 函数。

```text
ent_Miner_Bob = 0   矿工 Bob 的编号
ent_Elsa     = 1    艾尔莎（本工程未登场，只占位）

GetNameOfEntity(0) → "Miner Bob"
```

要回答的问题：

```text
Q：为什么打印台词时能显示 "Miner Bob" 而不是 "0"？
A：状态代码里调 GetNameOfEntity(pMiner->ID()) 把编号翻译成名字。
```

## 4. 第 3 步：State.h —— 状态合同（全工程最重要的 30 行）

只有一个类 State，没有任何数据成员，只有三个"纯虚函数"：

```text
class State
{
public:
  virtual void Enter(Miner*)   = 0;   进入状态时执行一次
  virtual void Execute(Miner*) = 0;  状态期间每步执行（干活+决定切换）
  virtual void Exit(Miner*)    = 0;  离开状态时执行一次
};
```

这是所有状态的"合同"：4 个具体状态类都必须实现这三个函数，否则连对象都造不出来。

要回答的问题：

```text
Q1：三个函数各自在什么时机被调用？
Q2：State 自己保存数据吗？
A2：不保存。矿工的数据全在 Miner 对象里 —— 记住这点，第 6 步会用到。
```

里程碑：

> 背下三件套的分工：**Enter 进场布置一次 / Execute 每步干活 / Exit 离场告别一次。**

## 5. 第 4 步：BaseGameEntity.h → .cpp —— 基类（10 分钟）

先读 .h 再读 .cpp，两件事：

```text
① 每个实体有唯一编号
   int  m_ID;                       本对象的编号
   static int m_iNextValidID;       计数器（全类共享一份，真身在 .cpp）

② 强制子类实现 Update()
   virtual void Update() = 0;       纯虚函数：不实现就别想创建对象
```

.cpp 里只有：计数器初始化 + SetID()（检查编号合法 → 保存 → 计数器 +1）。

要回答的问题：

```text
Q1：为什么 Update() 写成 = 0（纯虚）？
A1：逼每种实体自己实现"每步更新"，漏实现会编译报错，而不是运行时出 bug。

Q2：为什么 m_iNextValidID 要加 static？
A2：它不属于某个矿工，属于整个类 —— 全局只有一份，保证编号永不重复。
```

里程碑：

> 知道 **"有编号"和"必须实现 Update()"是基类定下的规矩**，Miner 只是照办。

## 6. 第 5 步：Miner.h → Miner.cpp —— 主角与状态机骨架（20 分钟）

### 先读 Miner.h，抓三块

第一块：4 个游戏规则常量（读代码时随时回来查）：

```text
ComfortLevel        = 5   存款 ≥ 5 → 心满意足，回家睡觉
MaxNuggets          = 3   口袋金块 ≥ 3 → 装满了，去银行
ThirstLevel         = 5   口渴 ≥ 5 → 渴了，去酒吧
TirednessThreshold  = 5   疲劳 > 5 → 困了，回家睡觉
```

第二块：矿工的"身体数据"（全部私有，外界只能通过小函数读写）：

```text
State*        m_pCurrentState;   ★当前状态指针 —— 矿工的"大脑"，FSM 核心
location_type m_Location;        当前位置（4 个地点之一）
int           m_iGoldCarried;    口袋里的金块数
int           m_iMoneyInBank;    银行存款
int           m_iThirst;         口渴值（每步 +1）
int           m_iFatigue;        疲劳值（挖矿 +1，睡觉 -1）
```

第三块：行为入口，只有两个函数需要实现（都在 .cpp）：

```text
void Update();               心跳：每步更新
void ChangeState(State*);    切换状态：换"大脑"
```

### 再读 Miner.cpp，只精读两个函数

```text
Miner::Update()      状态机的发动机
{
  m_iThirst += 1;                       ① 每活一步，口渴 +1
  if (m_pCurrentState)                  ② 大脑还在吗？
  {
    m_pCurrentState->Execute(this);    ③ 决策权全交给当前状态
  }
}

Miner::ChangeState(State* pNewState)   换脑标准流程（顺序固定）
{
  assert(两个指针都非空);
  ① m_pCurrentState->Exit(this);       旧状态收尾告别
  ② m_pCurrentState = pNewState;       换指针（从这行起大脑就是新状态）
  ③ m_pCurrentState->Enter(this);      新状态登场准备
}
```

其余函数（AddToGoldCarried、Thirsty、Fatigued 等）扫一眼即可，
它们都只是"一行加减 + 一个大小比较"的小工具。

要回答的问题：

```text
Q：矿工自己写过一个 if 来决定"现在该挖矿还是睡觉"吗？
A：没有！矿工只把决策权交给 m_pCurrentState 指向的状态对象。
   这正是状态机的精髓：行为不在 Miner 的长 if-else 里，而是分散在状态类里。
```

里程碑：

> 能默写：**Update = 口渴 +1，然后当前状态->Execute(this)；
> ChangeState = Exit 旧 → 换指针 → Enter 新。**

## 7. 第 6 步：MinerOwnedStates.h —— 角色表 + 单例（10 分钟）

4 个状态类的"花名册"：

```text
① EnterMineAndDigForNugget   进金矿挖金子（矿工的"上班"状态）
② VisitBankAndDepositGold    去银行把金块换成存款
③ GoHomeAndSleepTilRested    回家睡觉解乏（矿工出生时的初始状态）
④ QuenchThirst               去酒吧买威士忌解渴
```

每个类长得一模一样，都在重复一个固定套路 —— **单例三件套**：

```text
1. 构造函数私有            → 外界造不出第二个实例
2. 拷贝构造/赋值私有       → 禁止复制唯一实例
3. public static Instance() → 全世界唯一的取货入口
```

要回答的问题：

```text
Q1：为什么这 4 个类敢用单例？
A1：因为它们"有函数、无成员变量"——状态本身不保存任何数据，
    矿工的数据全在 Miner 对象里。所以"挖矿状态"全局只需要一份。

Q2：代码里怎么拿到某个状态？
A2：永远写 类名::Instance()，如
    EnterMineAndDigForNugget::Instance()
```

里程碑：

> 理解 **"状态无数据 → 全局一份够用 → 单例合法"** 这个因果链。

## 8. 第 7 步：MinerOwnedStates.cpp —— 剧本正文（30 分钟，读最慢）

本工程的核心文件。每个状态按固定三段读：

```text
Enter    → 念"赶路"台词 + 搬位置（ChangeLocation）
Execute  → 干活（改数据）+ 判断要不要 ChangeState（切换逻辑全在这）
Exit     → 念告别台词
```

推荐读法：**一个状态一个状态地读，每个状态把 Execute 里的切换条件抄下来。**
顺序：先①挖矿（最常出现）→ ②银行 → ③睡觉 → ④喝酒。

读完后必须能独立填出这张表：

```text
状态        Execute 干的活                 切换条件            切到哪
─────────────────────────────────────────────────────────────────
①挖矿      金块+1、疲劳+1                 口袋满(金块≥3)      →②银行
                                           口渴(口渴≥5)        →④喝酒
②银行      存款+金块数、口袋清零           存款≥5              →③睡觉
                                           存款<5              →①挖矿
③睡觉      睡一觉疲劳-1                   不困(疲劳≤5)        →①挖矿
④喝酒      买酒：口渴清零、存款-2          喝完(必然)          →①挖矿
```

注意一个细节（很多人会漏）：

```text
①挖矿的 Execute 里有两个并排的 if（不是 else）：
   if (PocketsFull())  → 切②银行
   if (Thirsty())      → 切④喝酒
两个条件同一步都成立时，会连续两次 ChangeState：
   先切到②银行（触发②的 Enter），紧接着又切到④喝酒。
这解释了输出里"为什么有时刚走到银行就转身去了酒吧"。
```

里程碑：

> 不看代码也能画出 **四状态切换全景图**（见第四章）。

## 9. 第 8 步：回到 main.cpp —— 闭环验收（10 分钟）

从 for 循环开始，在脑内推演前 3 步，再运行程序对照。

推演需要的已知条件（来自 Miner 构造函数）：

```text
矿工出生时：位置=shack、金块=0、存款=0、口渴=0、疲劳=0、状态=③睡觉
```

脑内推演：

```text
第 1 步 Update：
  口渴 0→1
  ③睡觉 Execute：疲劳 0，不困 → 切①挖矿
      "What a God darn fantastic nap! Time to find more gold"
  ChangeState → ③Exit："Leaving the house"
              → ①Enter："Walkin' to the goldmine"

第 2 步 Update：
  口渴 1→2
  ①挖矿 Execute：金块 0→1、疲劳 0→1
      "Pickin' up a nugget"

第 3 步 Update：
  口渴 2→3
  ①挖矿 Execute：金块 1→2、疲劳 1→2
      "Pickin' up a nugget"
```

验收标准（三条全过 = 读懂了）：

```text
✓ 能说出第一幕三句台词的先后顺序，并解释为什么是这个顺序
✓ 能继续推演第 4、5 步（金块挖满 3 块 → 去银行存钱）
✓ 能解释第 6～8 步里"口袋满 + 口渴"同时成立时会发生什么
```

---

# 四、读完必须能复述的两张图

## 图 1：一次 Update 的完整调用链

```text
main 的 for 循环
   ↓
miner.Update()                          (Miner.cpp)
   ├─ m_iThirst += 1                     口渴 +1
   ↓
m_pCurrentState->Execute(this)           (多态：指向谁就执行谁)
   ├─ 干活：改矿工数据（金块/存款/疲劳/口渴）
   ├─ 念台词（cout + GetNameOfEntity）
   └─ 满足条件时：pMiner->ChangeState(目标状态::Instance())
                  (Miner.cpp)
                   ├─ 旧状态 Exit()      告别
                   ├─ 换指针 m_pCurrentState = 新状态
                   └─ 新状态 Enter()      赶路 + 开场白
   ↓
回到 for 循环，下一圈再重复
```

## 图 2：四状态切换全景图

```text
                睡醒(疲劳≤5)
      ┌──────────────────────────┐
      │                          ▼
 ③睡觉(shack)              ①挖矿(goldmine)
      ▲                    金块+1 疲劳+1
      │ 存款≥5                 │        │
      │                       口袋满    口渴≥5
      │                         │        │
 ②银行(bank)◀───────────────────┘        │
   存款+金块、口袋清零                     ▼
      │                            ④喝酒(saloon)
      │ 存款<5                       口渴清零、存款-2
      └────────→ ①挖矿  ◀─────────────┘
                 喝完回矿

【口诀】挖满 3 块去银行；存满 5 块回家睡；
         口渴 5 点去喝酒；睡醒、喝完、没存够，统统回金矿。
```

---

# 五、阅读自测清单

合上代码，口头回答。答不出的，回到对应章节重读。

```text
1. 程序一共 20 步，每一步谁被调用？起点是哪个函数？
   （main → miner.Update()）

2. Miner 自己写过"该挖矿还是该睡觉"的 if 吗？
   （没有，决策全在状态类的 Execute 里）

3. ChangeState 为什么是"Exit 旧 → 换指针 → Enter 新"这个顺序？
   （先让旧状态收尾；指针换完，新状态的 Enter 才能"以新状态自居"登场赶路）

4. 4 个状态类为什么敢做成单例？
   （状态类无成员变量，数据全在 Miner 里，全局一份就够）

5. State.h 里三个函数各在什么时机被调用、被谁调用？
   （Enter/Exit 由 ChangeState 调；Execute 由 Miner::Update 调）

6. BaseGameEntity 的 Update()=0 起什么作用？
   （强制子类实现，漏了直接编译不过）

7. Thirsty() 用 >=（≥5 算渴），Fatigued() 用 >（>5 算困），
   两处写法不一致，会影响行为吗？
   （会——疲劳=5 时"不困"也不"渴"，属于边界值差异；自己推演一次）

8. 矿工出生在小屋、疲劳为 0、初始状态是"睡觉"，第一步会发生什么？
   （立刻睡醒切去挖矿，见第三章第 8 步的推演）

9. Miner.h 里为什么写 class State; 就够了，不用 #include "State.h"？
   （成员只是指针，指针大小固定，编译器不需要 State 的完整定义）

10. TEXTOUTPUT 宏是干什么的？
    （#ifdef 条件编译：定义它后 cout 被宏替换成文件流 os，输出改写进文件）
```

---

# 六、常见疑问速答

```text
Q：main.cpp 为什么 include 了 Locations.h 却没直接用？
A：习惯写法，无害。真正用地点的是 Miner.h 和状态代码。

Q：misc/ConsoleUtils.h 不在本文件夹，编译器怎么找到的？
A：工程设置了附加包含目录 ..\..\Common，
   ConsoleUtils.h 位于仓库根目录的 Common\misc\ 下。

Q：为什么每个状态类的函数参数都是 Miner* 而不直接用全局变量？
A：状态对象通过指针操作"正在经历这个状态的那个矿工"，
   将来多个角色复用同一批状态类时依然成立（本书后续就这么干）。

Q：这是状态模式（State Pattern）吗？
A：是。FSM 是思想，状态模式是它在面向对象代码里的落地形态。
```

---

# 七、动手实验（选做，每个 5 分钟）

改常量重跑，观察行为变化，是检验理解的最快方式：

```text
实验 1：MaxNuggets 3 → 1
        矿工挖 1 块就跑银行，输出节奏明显变快

实验 2：ComfortLevel 5 → 20
        矿工几乎不再回家睡觉（存款很难攒够）

实验 3：ThirstLevel 5 → 1
        几乎每步都往酒吧跑（第 1 步就渴了）

实验 4：把 for 循环 20 → 40
        完整观察"挖矿→银行→睡觉→挖矿→喝酒"的循环周期

实验 5（进阶）：给 Miner 加一个 m_iHunger 成员和一个 EatState，
        照抄单例三件套 + 三个接口函数 —— 这是从"读懂"到"会用"的一步
```

---

# 八、下一步

```text
WestWorld1          ← 本工程：单角色、无交流、最简 FSM
   ↓
WestWorldWithWoman  同一章的第二个示例：加入妻子 Elsa，两个角色各自一套状态
   ↓
WestWorldWithMessaging  第三个示例：加入"消息系统"（Telegram/消息派发），
                          角色之间可以互相发消息触发状态切换
```

读完本工程后建议先回看 `2026-1002-00-FSM概念.md`，
把概念笔记里的 State / Transition / Condition 与本工程代码一一对应：

```text
State      → State.h 及其 4 个子类
Condition  → 各状态 Execute 里的 if（PocketsFull/Thirsty/Wealth>=ComfortLevel…）
Transition → Miner::ChangeState()（Exit → 换指针 → Enter）
CurrentState → Miner::m_pCurrentState
Enter/Update/Exit → Enter/Execute/Exit（Update 由 Miner::Update 代劳转发）
```

> **状态决定"现在做什么"，条件决定"什么时候换状态"——
> 在 WestWorld1 里，前者写在状态类的 Execute，后者也是。矿工自己什么都不决定。**
