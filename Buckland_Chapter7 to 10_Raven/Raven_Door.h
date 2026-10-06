//==============================================================================================
//【文件说明】Raven_Door.h —— 会滑动的「门」
//
//【这个文件是干什么的?】
//  地图上有一种门:平时关着挡路;机器人发 Msg_OpenSesame 消息给它,它就滑开,
//  让机器人过;过一会又自动关上。本类继承 BaseGameEntity(所有游戏实体的祖宗),
//  内部用两面墙(m_pWall1/2)表示门板,自己维护 open/opening/closed/closing 四种状态。
//
//【谁在使用这个文件?】
//  Raven_Map.h/.cpp —— 读地图文件时 new 出 Raven_Door;
//  Raven_Game.cpp  —— 每帧 Update/Render 所有门;
//  triggers\Trigger_OnButtonSendMsg.h —— 按钮被踩时给门发开门消息。
#ifndef RAVEN_Raven_Door_H
#define RAVEN_Raven_Door_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Door.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to emulate a sliding door that can be opened by sending
//          it a msg_OpenSesame telegram. The door stays open for a user
//          specified amount of time before closing. 
//(原文注释翻译:模拟滑动门;收到 msg_OpenSesame 电报就开,开一段时间后自动关)
//-----------------------------------------------------------------------------
#include <vector>
#include <iosfwd>
#include "2d/vector2d.h"
#include "game/BaseGameEntity.h"



struct Telegram;
class Raven_Map;
class Wall2D;


//--------------------------------------------------------------------------------
// class Raven_Door —— 滑动门类。冒号 public 表示继承 BaseGameEntity。
class Raven_Door : public BaseGameEntity
{
protected:
  
// 嵌套枚举:门的四种状态——开/正在开/关/正在关。
  enum door_status{open, opening, closed, closing};

protected:

  door_status                m_Status;

  //a sliding door is created from two walls, back to back.These walls must
  //be added to a map's geometry in order for an agent to detect them
// m_pWall1/m_pWall2:组成门板的两面墙指针。
//(原文注释翻译:滑动门由两面背对背的墙拼成;这两面墙必须加进地图几何里,
//           机器人才能把它们当障碍物检测到。)
  Wall2D*                    m_pWall1;
  Wall2D*                    m_pWall2;

  //a container of the id's of the triggers able to open this door
// m_Switches:能开这扇门的按钮编号列表。
//(原文注释:能开这扇门的那些触发器(按钮)的编号列表)
  std::vector<unsigned int>  m_Switches;

  //how long the door remains open before it starts to shut again
// m_iNumTicksStayOpen:门开着多少帧后开始关;m_iNumTicksCurrentlyOpen:已开了几帧。
//(原文注释:门开多久后开始关)
  int                        m_iNumTicksStayOpen;

  //how long the door has been open (0 if status is not open)
  int                        m_iNumTicksCurrentlyOpen;

  //the door's position and size when in the open position
  Vector2D  m_vP1;
  Vector2D  m_vP2;
  double     m_dSize;
  
  //a normalized vector facing along the door. This is used frequently
  //by the other methods so we might as well just calculate it once in the
//(原文注释翻译:沿门方向的归一化向量;很多方法都要用,干脆构造时算一次存着)
  //ctor
// m_vP1/P2:门全开时两端点;m_dSize:门全宽;m_dCurrentSize:当前宽度。
  Vector2D  m_vtoP2Norm;

  //the door's current size
  double     m_dCurrentSize;

// Open()/Close():开门/关门(内部用);ChangePosition:移动门板两端点。
  void  Open();
  void  Close();
  
  void ChangePosition(Vector2D newP1, Vector2D newP2);
 
public:
  
// 构造(从地图文件读)、析构;Render/Update/HandleMessage/Read 是标准接口。
  Raven_Door(Raven_Map* pMap, std::ifstream& is);
  ~Raven_Door();

  //the usual suspects
  void Render();
  void Update();
  bool HandleMessage(const Telegram& msg);
  void Read(std::ifstream&  os);


  //adds the ID of a switch
//(原文注释:加一个按钮编号)
  void AddSwitch(unsigned int id);

  std::vector<unsigned int> GetSwitchIDs()const{return m_Switches;}
};


#endif
