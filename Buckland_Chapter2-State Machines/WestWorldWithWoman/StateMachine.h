//==============================================================================================
//【文件说明】StateMachine.h —— 通用"有限状态机"类(每个角色脑子里装的那台机器)
//
//【这个文件是干什么的?】
//  定义 template class StateMachine —— 一个模板类,负责替某个角色管理它的全部
//  状态。造出 StateMachine<Miner> 就是"矿工版状态机",造出
//  StateMachine<MinersWife> 就是"妻子版状态机"。它替角色保管三个状态指针:
//    ▸ m_pCurrentState  —— 当前状态:角色此刻"正在干什么"(最重要的一位);
//    ▸ m_pPreviousState —— 上一个状态:刚从哪个状态切换过来的(留档备查);
//    ▸ m_pGlobalState   —— 全局状态:无论当前状态是谁,每次更新都会"额外"
//                          执行一次的大管家(本工程只有妻子用到,详见下)。
//  同时提供一整套操作:Update()(驱动状态机走一步)、ChangeState()(切换状态
//  的标准三步曲)、RevertToPreviousState()(退回上一个状态)等。
//
//【与 WestWorld1 的对比——为什么多了这个文件?】
//  WestWorld1 里矿工类自己拿着一个 m_pCurrentState 指针,自己写 ChangeState,
//  "状态机"的功能是硬编码在 Miner 类里的。本工程把这套逻辑抽成了通用的
//  StateMachine 类:角色只要"拥有"一台状态机,再"告诉"它初始状态是谁,
//  剩下的一切(切换、回退、全局逻辑)全部由状态机代劳——这正是本书第 2 章
//  代码一步步"进化重构"的路线:先能跑(WestWorld1),再抽公共(WestWorldWithWoman)。
//
//【与相关文件的关系】
//  1. State.h —— 本文件包含它:三个状态指针的类型 State<entity_type>* 来自那里;
//  2. Miner.h / MinersWife.h —— 矿工、妻子各持有一个
//     "StateMachine<自己>" 的指针 m_pStateMachine(在各自构造函数里 new 出来,
//     在析构函数里 delete 回收),通过 GetFSM() 对外提供;
//  3. Miner.cpp / MinersWife.cpp —— 各自的 Update() 每一步都调用
//     m_pStateMachine->Update(),把决策权交给状态机;
//  4. MinerOwnedStates.cpp / MinersWifeOwnedStates.cpp —— 各状态类通过
//     "角色->GetFSM()->ChangeState(...)" 触发切换(不再像 WestWorld1 那样直接
//     调用角色自己的 ChangeState);
//  5. MinersWifeOwnedStates.cpp —— 妻子的"上厕所"状态用本文件的
//     RevertToPreviousState() 干完活后自动回到原来的家务状态。
//
//【新概念:全局状态 m_pGlobalState 与"状态 blip"】
//  有些行为不属于任何一个具体状态,而是"随时可能插播"的零星小事——比如妻子
//  每一步都有 1/10 的概率想去上厕所。为此状态机在"当前状态"之外再挂一个
//  全局状态:每次 Update() 先执行全局状态的 Execute,再执行当前状态的 Execute。
//  妻子的用法(WifesGlobalState):全局状态里掷骰子决定"要不要上厕所",想上就
//  ChangeState 切到 VisitBathroom;上完厕所用 RevertToPreviousState() 退回刚才
//  打断的家务——这种"短暂插播再返回"的模式,书里叫状态 blip(blink of state)。
//  (矿工没用全局状态——他的 StateMachine 的 m_pGlobalState 一直是 NULL 空指针,
//   Update() 里的 if 判断会自动跳过它。)
//
//【C++ 小课堂:模板类(template class)】
//  template <class entity_type> 的原理详解见 State.h 顶部的注释——把"状态机
//  服务哪种角色"留作类型参数。本文件里所有 entity_type 的位置,造出
//  StateMachine<Miner> 时都会被替换成 Miner。
//==============================================================================================

// 包含保护(原理详解见 Locations.h):防止本文件被重复包含。
#ifndef STATEMACHINE_H
#define STATEMACHINE_H

//------------------------------------------------------------------------
//
//  Name:   StateMachine.h
//
//  Desc:   State machine class. Inherit from this class and create some 
//          states to give your agents FSM functionality
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:StateMachine.h
//   描述  :State machine class. Inherit from this class and create some
//          states to give your agents FSM functionality
//          (状态机类。继承这个类并创建一些状态,就能给你的"智能体"
//           装上有限状态机的功能)
//   作者  :Mat Buckland,2002 年(本书作者)
//   注:原文 Desc 行的第二行结尾有一个行尾空格,系原作者排版残留,原样保留。
//------------------------------------------------------------------------
// <cassert>:标准库"断言",下面 ChangeState 里用到 assert。
#include <cassert>
// <string>:标准库字符串库(本文件没直接用到 std::string,习惯性包含)。
#include <string>

// State 接口的完整定义:下面三个状态指针的类型 State<entity_type>* 来自它。
#include "State.h"



// 模板头(template <class entity_type>):把"这台状态机服务哪种角色"留作
// 类型参数,原理详见 State.h 顶部注释。
template <class entity_type>
class StateMachine
{
// private:(私有区)三个状态指针和"机器的主人"都是内部机密,
// 外界只能通过下面 public 区的函数间接操作。
private:

  //a pointer to the agent that owns this instance
  //(原文注释:指向"拥有本状态机实例的那个智能体"的指针)

  // 【机器的主人】entity_type* —— "这台状态机属于哪个角色"的指针。
  // 造出 StateMachine<Miner> 后它就是 Miner*,指向那位矿工对象自己。
  // 有了它,状态机才能在驱动状态时把主人交给状态:
  // "Execute(主人指针)"。
  entity_type*          m_pOwner;

  // 【当前状态指针】State<entity_type>* —— 可以指向任何一个"该角色版状态"
  // 的对象。角色此刻"在干什么"完全由它决定。注意:它只是个指针,具体状态
  // 对象(单例)并不归本类所有,所以析构函数不 delete 它。
  State<entity_type>*   m_pCurrentState;
  
  //a record of the last state the agent was in
  //(原文注释:记录智能体刚才所在的那个状态)

  // 【上一个状态指针】ChangeState() 每次切换前都会把"旧状态"存档到这里,
  // 供 RevertToPreviousState()(退回上一状态)使用。
  State<entity_type>*   m_pPreviousState;

  //this is called every time the FSM is updated
  //(原文注释:每次更新状态机(FSM)时,这个(状态)都会被调用)

  // 【全局状态指针】无论当前状态是谁,每次 Update() 都先执行它的 Execute
  // 再执行当前状态的 Execute(概念详见本文件头部"全局状态与状态 blip"注释)。
  // 没有全局状态的角色把它保持为 NULL 即可。
  State<entity_type>*   m_pGlobalState;
  

// public:(公有区)外界(角色类、状态类)操作状态机的"服务窗口"。
public:

  // 【构造函数】创建状态机时必须告诉它"主人是谁"。
  // 冒号后面是"成员初始化列表"(格式:"成员名(初始值)",逗号分隔):
  //   m_pOwner(owner)       —— 主人指针 = 传进来的 owner(角色把自己的
  //                            this 指针传进来,见 Miner.h 构造函数);
  //   其余三个指针(NULL)   —— 出生时三个状态全部为"空"(NULL 即空指针,
  //                            表示"还不指向任何状态"),随后由角色用
  //                            SetCurrentState/SetGlobalState 逐个安装。
  // 函数体 {} 为空:全部初始化工作由初始化列表完成。
  StateMachine(entity_type* owner):m_pOwner(owner),
                                   m_pCurrentState(NULL),
                                   m_pPreviousState(NULL),
                                   m_pGlobalState(NULL)
  {}

  // 【虚析构函数】空函数体:本类自己没有需要清理的资源(三个状态指针指向的
  // 单例状态对象归全局所有,不归本类销毁)。virtual 的意义详见 State.h 中
  // ~State() 的注释。
  virtual ~StateMachine(){}

  //use these methods to initialize the FSM
  //(原文注释:用这些方法来初始化状态机)

  // 【安装当前状态】参数 s:要设为"当前状态"的状态对象指针。
  // 角色构造函数里调用(如 Miner 里 SetCurrentState(GoHomeAndSleepTilRested::Instance()))。
  void SetCurrentState(State<entity_type>* s){m_pCurrentState = s;}
  // 【安装全局状态】妻子用它安装 WifesGlobalState(矿工不调用,保持 NULL)。
  void SetGlobalState(State<entity_type>* s) {m_pGlobalState = s;}
  // 【安装上一状态】本示例没有用到,提供它只是让工具集完整。
  void SetPreviousState(State<entity_type>* s){m_pPreviousState = s;}
  
  //call this to update the FSM
  //(原文注释:调用这个来更新状态机)

  //--------------------------------------------------------------------------------
  //【Update —— 状态机的"心跳",每次驱动角色走一步】
  //  名字后面的 const:承诺本函数不修改对象的任何成员——但注意例外:
  //  Execute/ChangeState 会通过 m_pOwner 指针"隔着"修改主人对象(指针本身
  //  没变),这种"自己不改、改别人"的写法在 const 成员函数里是允许的。
  //  执行顺序:先全局状态,后当前状态(全局状态优先"插播")。
  //--------------------------------------------------------------------------------
  void  Update()const
  {
    //if a global state exists, call its execute method, else do nothing
    //(原文注释大意:若存在全局状态,就调用它的 Execute;否则什么都不做)

    // if(指针):指针直接当条件用——空指针(NULL)算"假",非空算"真"。
    // "->"是指针的成员访问运算符(读作"指向"):m_pGlobalState 是指针,
    // 所以用 -> 而不是点号。含义:若装了全局状态,就让它的 Execute 干活,
    // 并把主人 m_pOwner 交给他操纵。
    if(m_pGlobalState)   m_pGlobalState->Execute(m_pOwner);

    //same for the current state
    //(原文注释:对当前状态做同样的事)

    // 当前状态同样执行一步。(若当前状态也是 NULL 则跳过——防御式编程,
    // 万一忘了安装状态也不至于空指针崩溃。)
    if (m_pCurrentState) m_pCurrentState->Execute(m_pOwner);
  }

  //change to a new state
  //(原文注释:切换到新状态)

  //--------------------------------------------------------------------------------
  //【ChangeState —— 切换状态的"换脑"标准流程】
  //  参数 pNewState:新状态对象(通常传"某状态类::Instance()")。
  //  固定四步(顺序不能乱):
  //    ① m_pPreviousState = 旧状态  —— 先存档(供日后"退回"用);
  //    ② 旧状态->Exit(主人)          —— 让旧状态收尾告别;
  //    ③ m_pCurrentState = 新状态     —— 把"大脑"指针换掉;
  //    ④ 新状态->Enter(主人)         —— 让新状态登场准备(通常包括赶路)。
  //  与 WestWorld1 相比多了①存档一步——这正是为了支持
  //  RevertToPreviousState()(退回上一状态)的"状态 blip"玩法。
  //--------------------------------------------------------------------------------
  void  ChangeState(State<entity_type>* pNewState)
  {
    // 断言:新状态指针必须非空(空指针就出逻辑错误了,提前拦下)。
    // 注意原文这行结尾 && 后面的空行拆行写法:条件 && 提示文字——
    // 断言失败时显示 "<StateMachine::ChangeState>: trying to change to NULL state"
    // (试图切换到空状态)。
    assert(pNewState && 
           "<StateMachine::ChangeState>: trying to change to NULL state");

    //keep a record of the previous state
    //(原文注释:保留上一个状态的记录)

    // 第①步:存档旧状态。
    m_pPreviousState = m_pCurrentState;

    //call the exit method of the existing state
    //(原文注释:调用现有状态的退出方法)

    // 第②步:旧状态收尾告别。
    m_pCurrentState->Exit(m_pOwner);

    //change state to the new state
    //(原文注释:把状态换成新状态)

    // 第③步:指针改指新状态——从这一行起,角色的"大脑"就是新状态了。
    m_pCurrentState = pNewState;

    //call the entry method of the new state
    //(原文注释:调用新状态的进入方法)

    // 第④步:新状态登场准备。
    m_pCurrentState->Enter(m_pOwner);
  }

  //change state back to the previous state
  //(原文注释:切换回上一个状态)

  //【退回上一个状态】实现只有一句:调用 ChangeState 切到 m_pPreviousState
  // (ChangeState 内部又会把"退回来之前的那个状态"存档为新的上一状态,
  //  一切自动维护)。本工程的用法:妻子的"上厕所"是个 blip——上完厕所
  // 调用它,回到被打断的家务状态(见 MinersWifeOwnedStates.cpp)。
  void  RevertToPreviousState()
  {
    ChangeState(m_pPreviousState);
  }

  //returns true if the current state's type is equal to the type of the
  //class passed as a parameter. 
  //(原文注释大意:若"当前状态的类型"与"传进来的参数状态的类型"相同,
  //  返回真。——用于查询"角色现在正处于某某状态吗?")

  //【类型查询】typeid 是 C++ 的"运行时类型信息"(RTTI)运算符:返回一个
  // 对象的真实类型信息;两个 typeid 相等即"是同一种类型"。
  // 参数 st:任意一个状态对象;"&"是引用(别名)传递,const 保证只读。
  // 本示例没有调用它,作者保留它作为状态机的标准工具之一。
  bool  isInState(const State<entity_type>& st)const
  {
    return typeid(*m_pCurrentState) == typeid(st);
  }

  //【三个只读访问器】分别取出当前/全局/上一状态的指针。
  // 括号后的 const 与 ID()const 同理(详见 BaseGameEntity.h 的注释):
  // 承诺不修改对象任何成员。
  State<entity_type>*  CurrentState()  const{return m_pCurrentState;}
  State<entity_type>*  GlobalState()   const{return m_pGlobalState;}
  State<entity_type>*  PreviousState() const{return m_pPreviousState;}
};




// 配对文件上方 #ifndef STATEMACHINE_H 的收尾(作用见 Locations.h 的"包含保护"注释)。
#endif


