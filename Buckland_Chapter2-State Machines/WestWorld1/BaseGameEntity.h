//==============================================================================================
//【文件说明】BaseGameEntity.h —— "游戏实体基类"的声明(矿工等一切角色的公共祖宗)
//
//【这个文件是干什么的?】
//  定义 class BaseGameEntity —— 游戏中一切角色/物体(entity,实体)的公共基类。
//  现在整个游戏里只有矿工 Bob 一个角色,但本书后面章节会陆续加入他的妻子 Elsa
//  等。凡是"游戏世界里存在的东西"都叫实体,而所有实体的共同点只有两件事,
//  这两件事就由本基类统一规定:
//    1) 每个实体都有一个全游戏唯一的整数编号(ID)——由本类登记、保管;
//    2) 每个实体都会被游戏主循环周期性地"更新"(Update)——本类用"纯虚函数"
//       强迫每种实体必须自己实现 Update()。
//
//【与相关文件的关系】
//  1. BaseGameEntity.cpp —— 本类的"实现":静态成员 m_iNextValidID 的定义、
//     SetID() 函数的具体代码都在那里;
//  2. Miner.h —— 矿工类通过"class Miner : public BaseGameEntity"继承本类:
//     自动获得编号功能,并被强制实现 Update();
//  3. Miner.cpp —— 矿工的构造函数把编号交给本类基类构造函数登记
//     (Miner::Miner(int id):BaseGameEntity(id));
//  4. EntityNames.h —— 提供常用编号常量(ent_Miner_Bob=0 等),
//     main.cpp 创建矿工时把编号传进来;
//  5. MinerOwnedStates.cpp —— 用 ID() 取出编号,再配合 GetNameOfEntity()
//     把编号翻译成名字打印("Miner Bob")。
//
//【C++ 小课堂:什么是"类"?】
//  类(class)是把"数据(成员变量)"和"操作这些数据的函数(成员函数)"
//  打包在一起的一种自定义类型。基类(base class)是给一批类提取公共部分的
//  "父类",子类通过"继承"自动获得基类的成员。本工程的角色关系:
//      BaseGameEntity(基类:管编号)
//            ▲ 继承
//            │
//         Miner(矿工:管自己的数据和行为)
//==============================================================================================

//------------------------------------------------------------------------------------------------
// 包含保护(原理详解见 Locations.h):防止本文件被重复包含。
// 注意:这里的宏名叫 ENTITY_H 而不是 BASEGAMEENTITY_H——名字无所谓,全工程唯一即可。
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
//   描述  :Base class for a game object —— 一个游戏对象的基类
//   作者  :Mat Buckland,2002 年(本书作者,邮箱 fup@ai-junkie.com)
//------------------------------------------------------------------------


//--------------------------------------------------------------------------------
//【类定义】class BaseGameEntity —— 定义一个名叫 BaseGameEntity 的类。
//  花括号 { ... }; 里是"类体",注意类定义结尾的分号";"不能丢。
//  类体内部按"访问权限"分成几个区:private: 区(私有)和 public: 区(公有)。
//--------------------------------------------------------------------------------
class BaseGameEntity
{

// private:(私有区)从这里到下一个访问标签之间的成员,只有本类自己的
// 函数能访问;外界(比如 main 函数、状态类)一律碰不到。
// 这叫"封装":把数据藏进保险柜,只留几个公开的"服务窗口"(见 public 区),
// 防止数据被外部代码乱改。
private:

  //every entity must have a unique identifying number
  //(原文注释:每个实体必须有一个唯一的识别编号)

  // 【成员变量:实体的编号】
  //   int     :整数类型,占 4 字节内存;
  //   m_ID    :变量名。匈牙利命名法:前缀 m_ 表示 member(成员变量),
  //             即"这是类自己内部的数据";后面的大写开头单词是含义(ID=编号)。
  int          m_ID;

  //this is the next valid ID. Each time a BaseGameEntity is instantiated
  //this value is updated
  //(原文注释:这是"下一个可用编号"。每次实例化一个新的 BaseGameEntity,
  //  这个值都会更新)

  // 【静态成员变量:下一个可用编号】
  //   static :表示这个变量"属于整个类",而不是属于某一个对象。
  //            所有实体对象共享同一份(全局只有一份),像挂在类上的公共计数器;
  //   m_i    :前缀 m_=成员,i_=int(整数),匈牙利命名法习惯;
  //   作用   :每创建一个新实体,就从这里取号,取完 +1,保证编号永不重复。
  //            它的"真身"(定义+初始化)在 BaseGameEntity.cpp 里(类内只是声明)。
  static int  m_iNextValidID;

  //this must be called within the constructor to make sure the ID is set
  //correctly. It verifies that the value passed to the method is greater
  //or equal to the next valid ID, before setting the ID and incrementing
  //the next valid ID
  //(原文注释大意:构造函数里必须调用它来确保编号设置正确。
  //  它会检查传进来的编号是否"大于等于下一个可用编号",
  //  然后才保存编号并把"下一个可用编号"加 1)

  // 【私有工具函数:登记编号】void 返回类型(无返回值),参数 val 是要登记的编号。
  // 设为私有:外界不能直接改编号;只有本类的构造函数(下面那个)能调用它。
  // 具体代码在 BaseGameEntity.cpp。
  void SetID(int val);

// public:(公有区)从这里到类体结束的成员,外界都能访问——类对外的"服务窗口"。
public:

  // 【构造函数】:与类同名的特殊函数、没有返回类型,创建对象时自动执行。
  // 参数 int id:创建实体时必须告诉它"我要用几号"。
  // 花括号里只有一句 SetID(id):把编号交给私有的 SetID 检查并登记。
  // 注意:构造函数的函数体直接写在类里(类内定义),这种写法隐式成为"内联函数"。
  BaseGameEntity(int id)
  {
    SetID(id);
  }

  // 【虚析构函数】"~类名"= 析构函数:对象销毁时自动执行的收尾函数。
  // 花括号{}为空 = 本类没有需要收尾的资源。virtual 关键字的意义:将来若有人
  // 用"基类指针"指向并删除"子类对象"(如 delete (BaseGameEntity*)miner),
  // 只有虚析构才能保证调用到子类 Miner 自己的析构函数,避免内存泄漏。
  // "基类析构必须 virtual"是 C++ 黄金守则。
  virtual ~BaseGameEntity(){}

  //all entities must implement an update function
  //(原文注释:所有实体都必须实现一个"更新"函数)

  // 【纯虚函数:强制子类实现"每步更新"】
  //   virtual :虚函数,允许子类(Miner)提供自己的版本,运行时按对象实际类型调用;
  //   void    :无返回值;
  //   = 0     :"纯虚"标记——本类不提供函数体,子类必须实现,否则子类也变成
  //             抽象类、无法创建对象。含纯虚函数的类叫"抽象类",不能直接创建
  //             BaseGameEntity 对象,它只当"模板/合同"用。
  virtual void  Update()=0;

  // 【公有只读访问器(getter):取出编号】
  //   ID()    :函数名;
  //   ()const :括号后的 const 表示"我保证不修改对象的任何成员"(只读承诺),
  //             这样连 const 对象也能安全地调用它;
  //   {return m_ID;} :函数体直接写在类内 = 隐式内联,调用处直接展开,效率高。
  int           ID()const{return m_ID;}  
};



#endif


