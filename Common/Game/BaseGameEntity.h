//==============================================================================================
//【文件说明】BaseGameEntity.h —— 所有游戏实体的"基类"(祖宗类)
//
//【这个文件是干什么的?】
//  游戏里能"动、能渲染、能收消息"的东西(矿工、妻子、球员、子弹、门……)
//  都从这个类派生。它规定了所有实体共有的东西:
//    唯一编号 ID(每个实体出生时自动分配,且全局不重复);
//    类型编号 m_iType(health/troll/ammo 等,各工程自己枚举);
//    位置/缩放/包围球半径(物理与碰撞用);
//    虚函数 Update()/Render()/HandleMessage() —— 子类各自实现具体行为。
//
//【谁在使用这个文件?】
//  MovingEntity.h(运动实体基类)、EntityManager.h(实体管理器)、
//  MessageDispatcher.cpp(按 ID 找实体)、以及全书所有具体角色类。
//
//【本文件包含了谁?】
//  <vector>/<string>/<iosfwd> —— 标准库(iosfwd 只前置声明输出流,加速编译);
//  "2D/Vector2D.h"、"2D/Geometry.h"、"misc/utils.h"。
//  struct Telegram; —— 前置声明(HandleMessage 只用到引用)。
//
//【C++ 小课堂:虚函数与纯虚函数】
//  virtual void Update(){} —— 虚函数:子类可以重写(override),
//    用父类指针调用时会自动调到子类版本。{} 表示基类给了空默认实现。
//  virtual void Render()=0 —— 纯虚函数(=0):基类不给实现,
//    子类必须实现,否则不能 new;含纯虚函数的类叫"抽象类",不能实例化。
//  static int m_iNextValidID —— 静态成员:全类共享一个变量,不属于某个对象,
//    所有实体共用这一个"下一个可用编号"计数器。
//==============================================================================================
 #ifndef BASE_GAME_ENTITY_H
#define BASE_GAME_ENTITY_H
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786) 原理详见 SoccerPitch.h;包含保护原理详见 Goal.h。
//--------------------------------------------------------------------------------
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name: BaseGameEntity.h
//
//  Desc: Base class to define a common interface for all game
//        entities
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <vector>
#include <string>
#include <iosfwd>
#include "2D/Vector2D.h"
#include "2D/Geometry.h"
#include "misc/utils.h"



struct Telegram;


//--------------------------------------------------------------------------------
// class BaseGameEntity —— 实体基类。
//--------------------------------------------------------------------------------
class BaseGameEntity
{
public:
  
  // 匿名枚举:default_entity_type = -1,表示"未指定类型"。
  enum {default_entity_type = -1};

private:
  
//--------------------------------------------------------------------------------
// 私有成员:
//   m_ID          —— 本实体的唯一编号;
//   m_iType       —— 类型编号;
//   m_bTag        —— 通用标记位(算法常用它临时标记实体);
//   m_iNextValidID —— 静态计数器:所有实体共享,记下一个可用 ID;
//   SetID(val)    —— 内部用:校验并分配 ID。
//--------------------------------------------------------------------------------
  //each entity has a unique ID
  int         m_ID;

  //every entity has a type associated with it (health, troll, ammo etc)
  int         m_iType;

  //this is a generic flag. 
  bool        m_bTag;

  //this is the next valid ID. Each time a BaseGameEntity is instantiated
  //this value is updated
  static int  m_iNextValidID;

  //this must be called within each constructor to make sure the ID is set
  //correctly. It verifies that the value passed to the method is greater
  //or equal to the next valid ID, before setting the ID and incrementing
  //the next valid ID
  void SetID(int val);


//--------------------------------------------------------------------------------
// protected 成员:自家子类能直接访问,外部不能:
//   m_vPosition        —— 位置坐标;
//   m_vScale           —— 缩放比例;
//   m_dBoundingRadius   —— 包围球半径(粗略碰撞用)。
//--------------------------------------------------------------------------------
protected:
  
  //its location in the environment
  Vector2D m_vPosition;

  Vector2D m_vScale;

  //the magnitude of this object's bounding radius
  double    m_dBoundingRadius;

  
  BaseGameEntity(int ID);

public:

//--------------------------------------------------------------------------------
// 下面 5 个虚函数是"子类必须按需要重写"的接口:
//   Update()         —— 每帧更新;
//   Render()=0        —— 绘制(纯虚,子类必须实现);
//   HandleMessage()   —— 收电报消息;
//   Write/Read()     —— 存档/读档到流。
//--------------------------------------------------------------------------------
  virtual ~BaseGameEntity(){}

  virtual void Update(){}; 

  virtual void Render()=0;
  
  virtual bool HandleMessage(const Telegram& msg){return false;}
  
  //entities should be able to read/write their data to a stream
  virtual void Write(std::ostream&  os)const{}
  virtual void Read (std::ifstream& is){}

  // 静态函数 GetNextValidID/ResetNextValidID:读/重置 ID 计数器(不依赖具体对象)。
  //use this to grab the next valid ID
  static int   GetNextValidID(){return m_iNextValidID;}
  
  //this can be used to reset the next ID
  static void  ResetNextValidID(){m_iNextValidID = 0;}
  


  // 下面一堆访问器:Pos/SetPos 位置;BRadius/SetBRadius 半径;ID 编号;
  // IsTagged/Tag/UnTag 标记位;Scale/SetScale 缩放;EntityType/SetEntityType 类型。
  Vector2D     Pos()const{return m_vPosition;}
  void         SetPos(Vector2D new_pos){m_vPosition = new_pos;}

  double       BRadius()const{return m_dBoundingRadius;}
  void         SetBRadius(double r){m_dBoundingRadius = r;}
  int          ID()const{return m_ID;}

  bool         IsTagged()const{return m_bTag;}
  void         Tag(){m_bTag = true;}
  void         UnTag(){m_bTag = false;}

  Vector2D     Scale()const{return m_vScale;}
  void         SetScale(Vector2D val){m_dBoundingRadius *= MaxOf(val.x, val.y)/MaxOf(m_vScale.x, m_vScale.y); m_vScale = val;}
  void         SetScale(double val){m_dBoundingRadius *= (val/MaxOf(m_vScale.x, m_vScale.y)); m_vScale = Vector2D(val, val);} 

  int          EntityType()const{return m_iType;}
  void         SetEntityType(int new_type){m_iType = new_type;}

};



      
#endif




