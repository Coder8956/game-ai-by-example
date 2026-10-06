//==============================================================================================
//【文件说明】triggers\Trigger_OnButtonSendMsg.h —— 按钮触发器(踩了就发消息)
//
//【这个文件是干什么的?】
//  地图上的一种「按钮」:bot 一踩上去,就给指定实体(比如一扇门)发一条消息,
//  消息内容由地图文件定。本工程里它就是开门按钮。
//
//【谁在使用这个文件?】
//  Raven_Map.cpp —— 读地图时 new 出本触发器,加进触发器系统。
//==============================================================================================
#ifndef TRIGGER_SEND_MESSAGE_H
#define TRIGGER_SEND_MESSAGE_H
///-----------------------------------------------------------------------------
//
//  Name:   Trigger_OnButtonSendMsg.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   trigger class to define a button that sends a msg to a 
//          specific entity when activated.
//-----------------------------------------------------------------------------
#include "Triggers/Trigger.h"
#include "Messaging/MessageDispatcher.h"
#include "misc/cgdi.h"

//--------------------------------------------------------------------------------
// template<class entity_type> —— 模板类,entity_type 是触发它的实体类型(这里是 Raven_Bot)。
template <class entity_type>
// class Trigger_OnButtonSendMsg : public Trigger —— 继承触发器基类。
class Trigger_OnButtonSendMsg : public Trigger<entity_type>
{
private:

  //when triggered a message is sent to the entity with the following ID
// m_iReceiver:接收方 ID;m_iMessageToSend:发什么消息。
//(原文注释:触发时给下面这个 ID 的实体发消息)
  unsigned int    m_iReceiver;

  //the message that is sent
  int             m_iMessageToSend;

public:

  Trigger_OnButtonSendMsg(std::ifstream& datafile):
      
      Trigger<entity_type>(GetValueFromStream<int>(datafile))
  {
     Read(datafile);
   }

// Try:测某实体踩没踩;Update:空;Render:画个橙框;Read/Write/HandleMessage 是标准接口。
  void Try(entity_type* pEnt);

  void Update();
  
  void Render();

  void Write(std::ostream&  os)const{}
  void Read (std::ifstream& is);

  bool HandleMessage(const Telegram& msg);
};


///////////////////////////////////////////////////////////////////////////////



template <class entity_type>
//--------------------------------------------------------------------------------
// Try:如果 pEnt 碰到触发器区域,立刻发一条消息给 m_iReceiver。
void  Trigger_OnButtonSendMsg<entity_type>::Try(entity_type* pEnt)
{

  if (isTouchingTrigger(pEnt->Pos(), pEnt->BRadius()))
  {
      Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                              this->ID(),
                              m_iReceiver,
                              m_iMessageToSend,
                              NO_ADDITIONAL_INFO);

  }
}

template <class entity_type>
void Trigger_OnButtonSendMsg<entity_type>::Update()
{
}

template <class entity_type>
// Render:画个橙方框当按钮。
void Trigger_OnButtonSendMsg<entity_type>::Render()
{
 gdi->OrangePen();

  double sz = BRadius();  

  gdi->Line(Pos().x - sz, Pos().y - sz, Pos().x + sz, Pos().y - sz);
  gdi->Line(Pos().x + sz, Pos().y - sz, Pos().x + sz, Pos().y + sz);
  gdi->Line(Pos().x + sz, Pos().y + sz, Pos().x - sz, Pos().y + sz);
  gdi->Line(Pos().x - sz, Pos().y + sz, Pos().x - sz, Pos().y - sz);
}

template <class entity_type>
// Read:从文件读接收方 ID/消息类型/位置/半径。
void Trigger_OnButtonSendMsg<entity_type>::Read(std::ifstream& is)
{
  //grab the id of the entity it messages
  is >> m_iReceiver;

  //grab the message type
  is >> m_iMessageToSend;

  //grab the position and radius
  double x,y,r;
  is >> x >> y >> r;

  SetPos(Vector2D(x,y));
  SetBRadius(r);

  //create and set this trigger's region of fluence
  AddRectangularTriggerRegion(Pos()-Vector2D(BRadius(), BRadius()),   //top left corner
                              Pos()+Vector2D(BRadius(), BRadius()));  //bottom right corner
}

template <class entity_type>
// HandleMessage:本触发器不收消息,直接返回 false。
bool Trigger_OnButtonSendMsg<entity_type>::HandleMessage(const Telegram& msg)
{
  return false;
}

#endif
