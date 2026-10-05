//==============================================================================================
//【文件说明】BaseGameEntity.h —— 全工程"游戏实体"的祖宗类(基类)
//
//【这个文件是干什么的?】
//  定义所有游戏对象共有的"骨架":无论机器人、障碍物还是别的什么,凡是
//  出现在游戏世界里的东西,都继承这个类,自动获得:
//    ① 唯一编号 ID(每人一个,用于区分"谁是谁");
//    ② 实体类型编号 EntityType(角色/障碍/弹药……按需用);
//    ③ 位置 Pos、缩放 Scale、包围半径 BRadius(碰撞判定的圆半径);
//    ④ 一个通用标记 Tag(被临时"圈出来"处理时打钩);
//    ⑤ 一套虚函数接口(Update 更新 / Render 绘制 / HandleMessage 收消息……),
//       子类按需覆写,保证"人人都有这些能力,但各做各的"。
//
//【与相关文件的关系】
//  谁继承了它?
//    MovingEntity.h  —— 会动的实体(机器人)继承本类,再添加速度、朝向等;
//    Obstacle.h      —— 不会动的障碍物继承本类,只补自己的画法;
//  它包含了谁?
//    2D/Vector2D.h    —— 位置/缩放用 2D 向量表示(Common 目录);
//    2D/Geometry.h    —— 几何工具(Common 目录),MaxOf(取大)来自这里;
//    misc/Utils.h     —— 工具函数(Common 目录);
//    <vector>/<string> —— C++ 标准库容器与字符串(为子类接口备用);
//  struct Telegram   —— 前置声明:消息类型"先报个名",HandleMessage 的参数
//                       只用到它的指针/引用,不需要完整定义。
//
//【C++ 小课堂:虚函数与多态】
//  virtual 函数 = 允许子类"覆写"的函数。基类里给个空的默认实现({} 什么都不做),
//  子类可以换成自己的版本。这样程序里到处写"entity->Update()",实际调用
//  哪个版本,取决于 entity 到底指向哪种实体——这就是"多态"。
//==============================================================================================
#ifndef BASE_GAME_ENTITY_H
#define BASE_GAME_ENTITY_H
//------------------------------------------------------------------------
//
//  Name: BaseGameEntity.h
//
//  Desc: Base class to define a common interface for all game
//        entities
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// 包含 C++ 标准库 vector(动态数组)。子类可能用它存顶点数据,这里先包含备用。
#include <vector>
// 包含 C++ 标准库 string(字符串)。同样为子类接口备用。
#include <string>
// 包含 2D 向量工具(Common 目录):成员 m_vPos / m_vScale 的类型 Vector2D
// 定义在这里。路径写法 2D/xxx.h = "附加包含目录 Common 下的 2D 子文件夹"。
#include "2D/Vector2D.h"
// 包含 2D 几何工具(Common 目录):SetScale 里用到的 MaxOf(取两个数中较大者)
// 定义在这里。
#include "2D/Geometry.h"
// 包含 Common 目录 misc/Utils.h 通用工具(本文件未直接用,习惯性包含)。
#include "misc/Utils.h"

//【前置声明】struct Telegram; —— 只告诉编译器"存在一个叫 Telegram 的结构体",
// 不给内容。下面 HandleMessage 的参数只用"Telegram 的引用",引用不需要
// 完整定义也能编译。Telegram 的完整定义在 Common/misc/Telegram.h,由真正
// 使用消息的文件去包含。
struct Telegram;


//【类定义开始】class BaseGameEntity —— 所有游戏实体的基类。
// 先 public 再 private 再 protected,顺序与作者习惯有关,阅读时按区域理解。
class BaseGameEntity
{
// public: —— 公开区:下面的成员任何代码都能访问。
public:
  
//【匿名枚举】enum {default_entity_type = -1};
//  定义了一个"没有类型名的枚举",只造一个枚举常量 default_entity_type(值 -1),
//  含义:"实体类型"字段的默认值。省略类型名是因为只想要这一个常量,
//  不需要用它声明变量。
  enum {default_entity_type = -1};

// private: —— 私有区:下面的成员只有 BaseGameEntity 自己的函数能访问。
// (子类也不能直接碰,要用后面提供的 public 函数间接读写。)
private:
  
// ↓↓↓ 原文注释翻译:每个实体都有一个唯一的 ID(编号)。
// int m_ID —— 整数成员。m_ 前缀 = member(成员变量),这是原作者命名习惯。
  //each entity has a unique ID
  int         m_ID;

// ↓↓↓ 原文注释翻译:每个实体都关联一个类型(健康包、巨魔、弹药等)。
// 类型是整数,具体每个数字代表什么由各工程自己约定。
  //every entity has a type associated with it (health, troll, ammo etc)
  int         m_EntityType;

// ↓↓↓ 原文注释翻译:这是一个通用标记(布尔开关)。
// m_bTag —— b = bool(布尔)。"被打上标记"表示"本轮被选中处理",
// 用完再取消(见下面的 Tag/UnTag)。
  //this is a generic flag. 
  bool        m_bTag;

// ↓↓↓ 原文注释翻译:构造函数用它给每个实体发一个唯一编号。
  //used by the constructor to give each entity a unique ID
//【函数内静态变量:编号分配器】
//   NextValidID() —— 每次调用返回一个"新编号":
//   static int NextID = 0 —— 函数内静态变量:第一次调用时建 0,之后一直
//     保留、不会被重置(和"全局变量"类似,但只在函数内可见);
//   return NextID++ —— 先返回当前值,再把 NextID 加 1(后置自增)。
//   于是第 1 个实体拿到 0、第 2 个拿到 1……保证全工程编号不重复。
  int NextValidID(){static int NextID = 0; return NextID++;}


// protected: —— 保护区:这里的成员"子类可见、外界不可见"。
// 位置、缩放、半径是子类(机器人/障碍物)要直接用的"身体数据",所以放这里;
// 构造函数也放这里:外界不能随便 new 一个"纯基类对象",只能由子类调用。
protected:
  
// ↓↓↓ 原文注释翻译:它在环境中的位置。
// Vector2D m_vPos —— 位置用 2D 向量表示(x、y 两个分量)。m_v 前缀:
// v = vector(向量)。
  //its location in the environment
  Vector2D m_vPos;

// 缩放比例 m_vScale:默认 (1.0, 1.0) 表示"原尺寸"。
// 机器人调大调小时用它(见 GameWorld.cpp 里把某辆调成 10 倍)。
  Vector2D m_vScale;

// ↓↓↓ 原文注释翻译:该物体"包围半径"的长度(碰撞判定的圆半径)。
// double m_dBoundingRadius —— m_d 前缀:d = double(双精度小数)。
// 包围半径 = 用一个圆粗略代表物体大小,判断"是否相撞/是否够近"时用它。
  //the length of this object's bounding radius
  double    m_dBoundingRadius;

  
//【构造函数①:无参版本】冒号后是成员初始化列表,逐项"出厂设置":
//   m_ID(NextValidID())   —— 编号:调用上面的分配器领一个新号;
//   m_dBoundingRadius(0.0)—— 半径先设 0;
//   m_vPos(Vector2D())    —— 位置:默认 (0,0);
//   m_vScale(Vector2D(1.0,1.0)) —— 缩放:原尺寸;
//   m_EntityType(default_entity_type) —— 类型:默认值 -1;
//   m_bTag(false)         —— 标记:默认"未打钩"。
// 最后的 {} 是空函数体:初始化全由列表完成。
// 【小知识】初始化列表比在函数体里赋值更高效,且"基类"只能用这种方式初始化。
  BaseGameEntity():m_ID(NextValidID()),
                   m_dBoundingRadius(0.0),
                   m_vPos(Vector2D()),
                   m_vScale(Vector2D(1.0,1.0)),
                   m_EntityType(default_entity_type),
                   m_bTag(false)
  {}

//【构造函数②:带实体类型版本】与①几乎相同,只是把传入的 entity_type
// 作为初始类型(例如机器人可能用某个类型号)。
  BaseGameEntity(int entity_type):m_ID(NextValidID()),
                   m_dBoundingRadius(0.0),
                   m_vPos(Vector2D()),
                   m_vScale(Vector2D(1.0,1.0)),
                   m_EntityType(entity_type),
                   m_bTag(false)
  {}
  
//【构造函数③:类型 + 位置 + 半径】最常用版本(MovingEntity/Obstacle 都调它):
//   m_vPos(pos)      —— 出生位置;
//   m_dBoundingRadius(r) —— 包围半径;
//   m_ID(NextValidID())  —— 领新编号;
//   ……其余同上。注意初始化列表顺序与成员声明顺序不完全一致,但 C++ 规定
//   按"声明顺序"执行——好在这些初始化互不依赖,没有影响。
  BaseGameEntity(int entity_type, Vector2D pos, double r):m_vPos(pos),
                                        m_dBoundingRadius(r),
                                        m_ID(NextValidID()),
                                        m_vScale(Vector2D(1.0,1.0)),
                                        m_EntityType(entity_type),
                                        m_bTag(false)
                                        
  {}

// ↓↓↓ 原文注释翻译:
//    这个构造函数可以创建"指定编号"的实体。当某个实体因故被删除后,
//    可以用它来复用旧编号。例如 Raven 地图编辑器在做撤销/重做时用它。
//    ★ 谨慎使用!(USE WITH CAUTION!)
//   (编号本应唯一,强行指定可能造成重复,所以加了警告。)
  //this can be used to create an entity with a 'forced' ID. It can be used
  //when a previously created entity has been removed and deleted from the
  //game for some reason. For example, The Raven map editor uses this ctor 
  //in its undo/redo operations. 
  //USE WITH CAUTION!
//【构造函数④:强制指定编号版本】直接把 ForcedID 装进 m_ID,不调用分配器。
// 只有在"明确知道这个编号空闲"时才允许用(如编辑器撤销重做)。
  BaseGameEntity(int entity_type, int ForcedID):m_ID(ForcedID),
                   m_dBoundingRadius(0.0),
                   m_vPos(Vector2D()),
                   m_vScale(Vector2D(1.0,1.0)),
                   m_EntityType(entity_type),
                   m_bTag(false)
  {}



// public: —— 第二段公开区:下面全是对外接口(访问器/虚函数)。
public:

//【虚析构函数】virtual ~BaseGameEntity(){}
// 析构 = 对象销毁时自动执行的清理函数。标 virtual 是基类的"好习惯":
// 保证用"基类指针"删除子类对象时,会先调用子类析构、再调基类析构,
// 否则子类资源会泄漏。这里函数体为空:本类自己没有要清理的资源。
  virtual ~BaseGameEntity(){}

//【虚函数:更新】virtual void Update(double time_elapsed){};
//  每个实体每帧被调用一次,参数 time_elapsed 是距上一帧的时间(秒);
//  基类默认什么都不做(空实现)——子类(如 Vehicle)覆写自己的行为。
//  (行尾的 ; 是作者风格,空函数体可有可无。)
  virtual void Update(double time_elapsed){}; 

//【虚函数:绘制】virtual void Render(){};——把自己画到屏幕上的接口,默认空。
  virtual void Render(){};

//【虚函数:处理消息】virtual bool HandleMessage(const Telegram& msg){return false;}
//  消息系统接口:收到消息返回 true(已处理);默认返回 false(不处理)。
//  const Telegram& —— 只读借用消息对象(别名,不拷贝);
//  本工程(第三章)未使用消息系统,保留接口是为了系列工程的统一。
  virtual bool HandleMessage(const Telegram& msg){return false;}
  
// ↓↓↓ 原文注释翻译:实体应能把自身数据写入/读回一个流(存盘/读盘)。
//  std::ostream 输出流 / std::ifstream 输入流。默认空实现:不做任何事。
  //entities should be able to read/write their data to a stream
  virtual void Write(std::ostream&  os)const{}
// 读入函数声明(与 Write 对应)。默认空实现。
  virtual void Read (std::ifstream& is){}
  


//【访问器组】下面都是"读/写"接口:对外提供受控访问私有/保护成员的方式。
//   Pos()  —— 返回位置(按值拷贝一份);
//   SetPos(new_pos) —— 设置新位置。
//  const 结尾 = 该函数不修改任何成员。
  Vector2D     Pos()const{return m_vPos;}
  void         SetPos(Vector2D new_pos){m_vPos = new_pos;}

//   BRadius()  —— 读包围半径;SetBRadius(r) —— 设置包围半径。
//   ID()       —— 读唯一编号。
  double        BRadius()const{return m_dBoundingRadius;}
  void         SetBRadius(double r){m_dBoundingRadius = r;}
  int          ID()const{return m_ID;}

//   IsTagged() —— 询问"是否被打了标记"(bool);
//   Tag()      —— 打上标记(设为 true);
//   UnTag()    —— 取消标记(设为 false)。
//   (Tag 机制配合 EntityFunctionTemplates.h 的 TagNeighbors 使用:先给
//    范围内的邻居打钩,下一轮处理时只关心打过钩的,避免重复比较。)
  bool         IsTagged()const{return m_bTag;}
  void         Tag(){m_bTag = true;}
  void         UnTag(){m_bTag = false;}

//   Scale()  —— 读缩放比例。
//   SetScale(Vector2D val) —— 按向量设置缩放。关键点:物体变大时,包围半径
//     也要按"新缩放 / 旧缩放"的比例同步放大(m_dBoundingRadius *= …),
//     否则"外形变大"但"碰撞圈"不变,视觉与碰撞不一致。
//     *= 读作"自身乘以"。MaxOf(val.x, val.y) 取新缩放的较大分量。
  Vector2D     Scale()const{return m_vScale;}
  void         SetScale(Vector2D val){m_dBoundingRadius *= MaxOf(val.x, val.y)/MaxOf(m_vScale.x, m_vScale.y); m_vScale = val;}
//   SetScale(double val) —— 按单个数字设置缩放(x、y 同比例):半径同样按
//     比例更新。Vector2D(val, val) 构造各分量相等的向量。
  void         SetScale(double val){m_dBoundingRadius *= (val/MaxOf(m_vScale.x, m_vScale.y)); m_vScale = Vector2D(val, val);} 

//   EntityType() —— 读实体类型编号;SetEntityType() —— 设置它。
  int          EntityType()const{return m_EntityType;}
  void         SetEntityType(int new_type){m_EntityType = new_type;}

// 类定义结束的花括号 + 分号(类定义必须以分号收尾)。
};



      
#endif




