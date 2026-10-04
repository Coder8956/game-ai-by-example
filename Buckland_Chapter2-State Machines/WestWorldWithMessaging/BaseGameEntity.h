//==============================================================================================
//【文件说明】BaseGameEntity.h —— 所有游戏角色的"祖宗类"(基类)
//
//【这个文件是干什么的?】
//  游戏里每个角色(矿工 Miner、妻子 MinersWife)身上都有一些共同的东西:
//  一个全世界唯一的编号 ID、一个每帧都要执行的 Update()、"能收消息"的本领。
//  这些共同点不适合在每个类里各写一遍,于是抽出来放在这个"基类"里,
//  让 Miner、MinersWife 通过": public BaseGameEntity"继承它。
//
//【基类 vs 子类(继承关系)】
//  基类(本文件)  = 爸爸:规定"凡是角色,必须有 ID、必须有 Update()、必须能收消息";
//  子类(子文件)  = 儿子:从爸爸那里继承这些规定,再补充自己的个性。
//     Miner.h      —— "矿工"子类:继承本类,再加上位置、金块、口渴、疲劳等矿工数据;
//     MinersWife.h —— "妻子"子类:继承本类,再加上做饭、位置等妻子数据。
//
//【谁在使用这个文件?】
//  1. Miner.h / MinersWife.h  —— #include 本文件,并写 ": public BaseGameEntity";
//  2. EntityManager.cpp       —— 用 BaseGameEntity* 指针统一管理所有角色;
//  3. MessageDispatcher.cpp   —— 用 BaseGameEntity* 指针调用角色的 HandleMessage();
//  4. BaseGameEntity.cpp      —— 本文件里声明的 SetID() 在那里实现。
//
//【virtual 关键字的小预告(下面会用到)】
//  virtual 的意思是"虚函数":子类可以(且应当)用自己的版本覆盖父类版本。
//  这样程序运行时,虽然拿着"爸爸的指针",调用的却是"儿子的函数"——多态。
//==============================================================================================

//------------------------------------------------------------------------------------------------
// 包含保护(原理详解见 Locations.h):防止本文件被重复包含。
//------------------------------------------------------------------------------------------------
#ifndef ENTITY_H
#define ENTITY_H
//------------------------------------------------------------------------
//
//  Name:   BaseGameEntity.h
//
//  Desc:   Base class for a game object
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:BaseGameEntity.h
//   描述  :Base class for a game object —— 游戏对象的基类
//   作者  :Mat Buckland,2002 年(本书作者)
//------------------------------------------------------------------------

// #include <string>:标准库字符串类型。本文件虽未直接使用,
// 但保留它便于子类(Miner.h)间接获得 string 定义(原作者的写法)。
#include <string>

// #include "messaging/Telegram.h":消息"信封"结构。
// 双引号 "..." 表示"先到本工程及附加包含目录里找"。Telegram 是
// 角色之间传递消息时用的数据包(见 Common\Messaging\Telegram.h),
// 下面 HandleMessage 的参数 const Telegram& msg 要用到它。
#include "messaging/Telegram.h"


//--------------------------------------------------------------------------------
//【类 class】class BaseGameEntity —— 定义一个"游戏实体/角色"的基类。
//  class 是 C++ 定义类的关键字;BaseGameEntity 是这个类的名字。
//  类 = 数据(成员变量)+ 行为(成员函数)的封装。
//--------------------------------------------------------------------------------
class BaseGameEntity
{

// private: 私有区——这里的东西只有本类自己能访问,
// 外部代码(Miner、main 等)一律碰不到。这叫"封装",防止数据被乱改。
private:

  //every entity must have a unique identifying number
  //(原文注释:每个实体必须有一个唯一标识编号)

  // 【成员变量 m_ID】每个角色的唯一身份证号(整数)。
  // 命名规则:开头小写 m_ 是匈牙利前缀,表示"member"(类的成员变量),
  // 一眼就能看出它是属于某个对象的。后面的 ID 是"编号"的缩写。
  // 构造时通过 SetID() 设置(见 BaseGameEntity.cpp),之后用 ID() 读取。
  int          m_ID;

  //this is the next valid ID. Each time a BaseGameEntity is instantiated
  //this value is updated
  //(原文注释:下一个可用的编号。每实例化一个 BaseGameEntity,这个值就被更新)

  // 【静态成员变量 static】m_iNextValidID —— 下一个可以分配出去的编号。
  //   static(静态)意味着:它不属于任何一个具体对象,而是整个"类"共用一份,
  //   存在全局内存里。不管创建几个角色,大家共享这一个计数器。
  //   初始化在 BaseGameEntity.cpp 第 6 行(int BaseGameEntity::m_iNextValidID = 0;)。
  //   作用:保证每个新角色拿到的编号都不会与已有的重复。
  static int  m_iNextValidID;

  //this must be called within the constructor to make sure the ID is set
  //correctly. It verifies that the value passed to the method is greater
  //or equal to the next valid ID, before setting the ID and incrementing
  //the next valid ID
  //(原文注释:必须在构造函数内调用它,确保 ID 设置正确。它会先检查传入的值
  //  是否大于等于下一个有效 ID,然后再设置 ID 并递增下一个有效 ID)

  // 【成员函数声明】void SetID(int val) —— 给角色设置编号的"内部工具函数"。
  //   void      :返回类型,"空"的意思,即本函数不返回任何值;
  //   SetID     :函数名,Set = 设置,ID = 编号;
  //   (int val) :参数,要设置成的新编号。
  //   声明在 private 区 → 只有本类(构造函数)能调用它,外部不能乱设编号。
  //   具体实现(怎么检查、怎么递增)写在 BaseGameEntity.cpp。
  void SetID(int val);

// public: 公有区——从这里开始,下面的东西外部代码都可以访问。
public:

  //--------------------------------------------------------------------------------
  //【构造函数】BaseGameEntity(int id) —— 创建角色对象时自动调用的函数。
  //  名字必须与类名完全相同(BaseGameEntity),没有返回类型。
  //  参数 (int id):创建时由外部传入的编号(如 main.cpp 里 ent_Miner_Bob=0)。
  //  函数体 { SetID(id); }:一进来立刻调用私有工具 SetID() 完成编号检查与赋值。
  //  【什么时候被调用?】创建子类对象时,子类构造函数会先调用父类构造函数,
  //  见 Miner.h 的 ": BaseGameEntity(id)"。
  //--------------------------------------------------------------------------------
  BaseGameEntity(int id)
  {
    SetID(id);
  }

  //--------------------------------------------------------------------------------
  //【虚析构函数】virtual ~BaseGameEntity(){} —— 对象销毁时自动调用。
  //  virtual     :虚函数。父类指针指向子类对象、delete 这个指针时,能正确
  //                调用到"子类自己的析构函数"(而不是误调父类的);
  //  ~BaseGameEntity :波浪号 ~ + 类名 = 析构函数(与构造函数配对,负责"善后");
  //  {}          :函数体为空——基类本身没有需要清理的资源,子类(如 Miner)
  //                的析构函数里才需要 delete 状态机指针。
  //  为什么要写 virtual?因为 main.cpp 用 BaseGameEntity 的父类视角管理对象,
  //  若析构不虚,delete 时可能漏掉子类资源的释放,造成内存泄漏。
  //--------------------------------------------------------------------------------
  virtual ~BaseGameEntity(){}

  //all entities must implement an update function
  //(原文注释:所有实体都必须实现一个更新函数)

  //--------------------------------------------------------------------------------
  //【纯虚函数 =0】virtual void Update()=0 —— "抽象函数"。
  //  =0 写在末尾,表示这个函数"只有声明、没有实现",强制子类必须自己实现。
  //  子类(Miner、MinersWife)里必须写 void Update() 并给出实际内容,
  //  否则子类自己也会变成"抽象类",无法创建对象。
  //  Update 的意思是"更新/心跳":游戏每一帧(每次循环)调用一次,
  //  让角色做当前该做的事。
  //--------------------------------------------------------------------------------
  virtual void  Update()=0;

  //all entities can communicate using messages. They are sent
  //using the MessageDispatcher singleton class
  //(原文注释:所有实体都能用消息通信。消息通过
  //  MessageDispatcher 单例类发送)

  // 【纯虚函数】virtual bool HandleMessage(const Telegram& msg)=0:
  //   又一个必须由子类实现的接口——"处理一条消息"。
  //    bool        :返回值类型,true=消息已被本角色处理,false=处理不了;
  //    const Telegram& msg:参数是"消息信封"的常量引用。const 表示函数内
  //    只读不改;引用 & 表示"直接使用原对象,不复制一份"(省内存);
  //    真正的发送/接收流程由 MessageDispatcher.cpp 负责。
  //    =0          :纯虚,子类必须实现。
  //--------------------------------------------------------------------------------
  virtual bool  HandleMessage(const Telegram& msg)=0;

  //--------------------------------------------------------------------------------
  //【内联成员函数】int ID()const{return m_ID;}
  //   给外部提供一个"只读"查看编号的窗口:
  //    ID()      :函数名;
  //    const     :在参数表后面,表示"本函数不修改对象的任何成员变量";
  //    {return m_ID;}:直接返回私有成员 m_ID 的值。
  //    因为 m_ID 在 private 区,外部想读编号,只能通过这个公有函数 ID()。
  //--------------------------------------------------------------------------------
  int           ID()const{return m_ID;}  
};



#endif


