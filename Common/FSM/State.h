//==============================================================================================
//【文件说明】State.h —— 状态机里"一个状态"的抽象基类(模板)
//
//【这个文件是干什么的?】
//  有限状态机(FSM)把角色的行为拆成一个个"状态",比如矿工的"挖矿中/回家睡觉中/
//  去酒吧中"。每个状态都要回答四件事:进入时做什么(Enter)、每帧做什么(Execute)、
//  离开时做什么(Exit)、收到消息怎么办(OnMessage)。本文件把这四件事定成一份"规矩"
//  (纯虚接口),所有具体状态都照它来实现。
//
//【谁在使用这个文件?】
//  WestWorld1 的 MinerOwnedStates.h/.cpp —— 矿工的各个状态都继承 State<Miner>;
//  Raven、第 4 章等所有状态机工程同理。
//
//【本文件包含了谁?】只前置声明了 struct Telegram(消息体,真正定义在 Messaging/Telegram.h)。
//
//【C++ 小课堂:抽象基类与纯虚函数】
//  virtual ... =0 叫"纯虚函数":只声明规矩、不写具体内容。含纯虚函数的类叫"抽象类",
//  不能直接造对象,只能被继承;子类必须把这四个函数都实现出来。=0 就是"我只管定接口,
//  实现交给子类"。virtual 关键字让"用父类指针调子类函数"时能正确调到子类版本(多态)。
//==============================================================================================
#ifndef STATE_H
#define STATE_H
//------------------------------------------------------------------------
//
//  Name:   State.h
//
//  Desc:   abstract base class to define an interface for a state
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【State —— 状态基类】:定义"一个状态"接口的抽象基类。作者:Mat Buckland。
//------------------------------------------------------------------------
// 前置声明:struct Telegram;(只说"有这么个消息结构体",不展开)——因为下面 OnMessage
// 要用到它,但这里只需要它的引用,不必包含它的完整头文件。
struct Telegram;

// template <class entity_type>:模板——entity_type 是个占位类型,用的时候才指定
// (比如 State<Miner>、State<Raven_Bot>),这样同一份状态代码能服务不同角色。
template <class entity_type>
// class State —— 状态抽象基类。
class State
{
public:

  virtual ~State(){}
  // virtual ~State():虚析构函数。有虚函数的基类通常把析构函数也声明为 virtual,
  // 这样通过父类指针删除子类对象时,才会正确调用子类的析构函数(防止内存泄漏)。

  //this will execute when the state is entered
  //(原文注释:进入本状态时执行一次)—— Enter:刚切换到这个状态时的初始化。
  virtual void Enter(entity_type*)=0;

  //this is the states normal update function
  //(原文注释:本状态的常规更新函数)—— Execute:停在这个状态期间,每帧都执行。
  virtual void Execute(entity_type*)=0;

  //this will execute when the state is exited. 
  //(原文注释:离开本状态时执行一次)—— Exit:切换走之前的收尾清理。
  virtual void Exit(entity_type*)=0;

  //this executes if the agent receives a message from the 
  //message dispatcher
  // ↓↓↓ 上面两行英文注释的翻译:当角色从"消息分发器"收到一条消息时执行。
  // OnMessage:处理收到的电报(Telegram),返回是否处理成功。
  virtual bool OnMessage(entity_type*, const Telegram&)=0;
};

#endif