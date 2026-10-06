//==============================================================================================
//【文件说明】Trigger.h —— 触发器基类(实体走进某个区域就触发一个动作)
//
//【这个文件是干什么的?】
//  游戏里"走过回血包就加血""碰到门就开"这种事叫 Trigger。它继承 BaseGameEntity,
//  额外持有一个"影响区域" m_pRegionOfInfluence(圆形或矩形)。子类实现两个纯虚函数:
//    Try(entity) —— 每帧判断实体是否进入区域,若是就执行效果;
//    Update()    —— 触发器自身每帧状态更新(如重生倒计时)。
//  m_bActive 可临时关闭触发器;m_iGraphNodeIndex 让寻路图能定位到触发器节点。
//
//【谁在使用这个文件?】
//  TriggerSystem.h 管理所有触发器;Raven 的武器/健康/门等触发器继承它。
//
//【本文件包含了谁?】
//  "game/BaseGameEntity.h"、"TriggerRegion.h"。
//==============================================================================================
#ifndef TRIGGER_H
#define TRIGGER_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Name:   Trigger.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   base class for a trigger. A trigger is an object that is
//          activated when an entity moves within its region of influence.
//
//-----------------------------------------------------------------------------
#include "game/BaseGameEntity.h"
#include "TriggerRegion.h"

struct Telegram;
struct Vector2D;

template <class entity_type>
//--------------------------------------------------------------------------------
// class Trigger —— 触发器基类(模板,参数 entity_type 是被触发的实体类型)。
//--------------------------------------------------------------------------------
class Trigger : public BaseGameEntity
{   
private:

  // m_pRegionOfInfluence:影响区域(圆形/矩形);m_bRemoveFromGame:是否待删除;
  // m_bActive:是否激活;m_iGraphNodeIndex:对应寻路图节点编号(-1 表示没有)。
  //Every trigger owns a trigger region. If an entity comes within this 
  //region the trigger is activated
  TriggerRegion* m_pRegionOfInfluence; 

  //if this is true the trigger will be removed from the game
  bool           m_bRemoveFromGame;

  //it's convenient to be able to deactivate certain types of triggers
  //on an event. Therefore a trigger can only be triggered when this
  //value is true (respawning triggers make good use of this facility)
  bool           m_bActive;

  //some types of trigger are twinned with a graph node. This enables
  //the pathfinding component of an AI to search a navgraph for a specific
  //type of trigger.
  int            m_iGraphNodeIndex;

  // protected 接口:子类设置区域/开关/图节点;isTouchingTrigger 委托给区域判断;
protected:
  
  void SetGraphNodeIndex(int idx){m_iGraphNodeIndex = idx;}

  void SetToBeRemovedFromGame(){m_bRemoveFromGame = true;}
  void SetInactive(){m_bActive = false;}
  void SetActive(){m_bActive = true;}

  //returns true if the entity given by a position and bounding radius is
  //overlapping the trigger region
  bool isTouchingTrigger(Vector2D EntityPos, double EntityRadius)const;

  //child classes use one of these methods to initialize the trigger region
  void AddCircularTriggerRegion(Vector2D center, double radius);
  void AddRectangularTriggerRegion(Vector2D TopLeft, Vector2D BottomRight);

public:

  // 构造:默认激活、未删除、图节点 -1;虚析构 delete 区域。
  Trigger(unsigned int id):BaseGameEntity(id),
                           m_bRemoveFromGame(false),
                           m_bActive(true),
                           m_iGraphNodeIndex(-1),
                           m_pRegionOfInfluence(NULL)
                           
  {}

  virtual ~Trigger(){delete m_pRegionOfInfluence;}

  //when this is called the trigger determines if the entity is within the
  //trigger's region of influence. If it is then the trigger will be 
  //triggered and the appropriate action will be taken.
  // Try/Update:子类必须实现的两个纯虚函数(=0 表示纯虚)。
  virtual void  Try(entity_type*) = 0;

  //called each update-step of the game. This methods updates any internal
  //state the trigger may have
  virtual void  Update() = 0;

  int  GraphNodeIndex()const{return m_iGraphNodeIndex;}
  bool isToBeRemoved()const{return m_bRemoveFromGame;}
  bool isActive(){return m_bActive;}
};

 
//------------------------ AddCircularTriggerRegion ---------------------------
//-----------------------------------------------------------------------------
template <class entity_type>
  // AddCircular/AddRectangular:新建对应区域对象,先 delete 旧的防止内存泄漏。
void Trigger<entity_type>::AddCircularTriggerRegion(Vector2D center,
                                                    double    radius)
{
  //if this replaces an existing region, tidy up memory
  if (m_pRegionOfInfluence) delete m_pRegionOfInfluence;

  m_pRegionOfInfluence = new TriggerRegion_Circle(center, radius);
}

//--------------------- AddRectangularTriggerRegion ---------------------------
//-----------------------------------------------------------------------------
template <class entity_type>
void Trigger<entity_type>::AddRectangularTriggerRegion(Vector2D TopLeft,
                                                       Vector2D BottomRight)
{
  //if this replaces an existing region, tidy up memory
  if (m_pRegionOfInfluence) delete m_pRegionOfInfluence;

  m_pRegionOfInfluence = new TriggerRegion_Rectangle(TopLeft, BottomRight);
}

//--------------------- isTouchingTrigger -------------------------------------
//-----------------------------------------------------------------------------
template <class entity_type>
  // isTouchingTrigger:委托给区域的 isTouching 判断。
bool Trigger<entity_type>::isTouchingTrigger(Vector2D EntityPos,
                                             double    EntityRadius)const
{
  if (m_pRegionOfInfluence) 
  {
    return m_pRegionOfInfluence->isTouching(EntityPos, EntityRadius);
  }
    
  return false;
}


#endif