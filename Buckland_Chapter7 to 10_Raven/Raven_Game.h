//==============================================================================================
//【文件说明】Raven_Game.h —— 游戏主类(整个游戏的总调度)
//
//【这个文件是干什么的?】
//  Raven_Game 是「游戏世界」的总管:持有地图、所有 bot、所有子弹、寻路管理器、墓碑。
//  每帧 Update() 推进世界,每帧 Render() 画世界。main.cpp 里就一个 Raven_Game 实例。
//
//【谁在使用这个文件?】
//  main.cpp —— 全局唯一实例,驱动主循环;
//  Raven_Bot —— 通过 m_pWorld 反查地图/其他 bot/视线遮挡等。
#ifndef RAVEN_ENV
#define RAVEN_ENV
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Game.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   this class creates and stores all the entities that make up the
//          Raven game environment. (walls, bots, health etc) and can read a
//          Raven map file and recreate the necessary geometry.
//
//          this class has methods for updating the game entities and for
//          rendering them.
//-----------------------------------------------------------------------------
#include <vector>
#include <string>
#include <list>

#include "graph/SparseGraph.h"
#include "Raven_ObjectEnumerations.h"
#include "2d/Wall2D.h"
#include "misc/utils.h"
#include "game/EntityFunctionTemplates.h"
#include "Raven_Bot.h"
#include "navigation/pathmanager.h"


class BaseGameEntity;
class Raven_Projectile;
class Raven_Map;
class GraveMarkers;



//--------------------------------------------------------------------------------
// class Raven_Game —— 游戏主类。
class Raven_Game
{
private:

  //the current game map
// m_pMap:地图;m_Bots:所有 bot 列表;m_pSelectedBot:人类选中/接管的 bot。
//(原文注释:当前游戏地图)
  Raven_Map*                       m_pMap;
 
  //a list of all the bots that are inhabiting the map
  std::list<Raven_Bot*>            m_Bots;

  //the user may select a bot to control manually. This is a pointer to that
  //bot
  Raven_Bot*                       m_pSelectedBot;
  
  //this list contains any active projectiles (slugs, rockets,
  //shotgun pellets, etc)
// m_Projectiles:子弹列表;m_pPathManager:寻路请求管理器。
//(原文注释:所有活动子弹(子弹/火箭/霰弹)列表)
  std::list<Raven_Projectile*>     m_Projectiles;

  //this class manages all the path planning requests
  PathManager<Raven_PathPlanner>*  m_pPathManager;


  //if true the game will be paused
// m_bPaused:暂停;m_bRemoveABot:要删一个 bot;m_pGraveMarkers:墓碑管理。
  bool                             m_bPaused;

  //if true a bot is removed from the game
  bool                             m_bRemoveABot;

  //when a bot is killed a "grave" is displayed for a few seconds. This
  //class manages the graves
  GraveMarkers*                    m_pGraveMarkers;

  //this iterates through each trigger, testing each one against each bot
// UpdateTriggers/Clear/AttemptToAddBot/NotifyAllBotsOfRemoval:私有辅助函数。
  void  UpdateTriggers();

  //deletes all entities, empties all containers and creates a new navgraph 
  void  Clear();

  //attempts to position a spawning bot at a free spawn point. returns false
  //if unsuccessful 
  bool AttemptToAddBot(Raven_Bot* pBot);

  //when a bot is removed from the game by a user all remaining bots
  //must be notified so that they can remove any references to that bot from
  //their memory
  void NotifyAllBotsOfRemoval(Raven_Bot* pRemovedBot)const;
  
public:
  
// 构造/析构;Render/Update 主循环;LoadMap 读地图。
  Raven_Game();
  ~Raven_Game();

  //the usual suspects
  void Render();
  void Update();

  //loads an environment from a file
  bool LoadMap(const std::string& FileName); 

  void AddBots(unsigned int NumBotsToAdd);
  void AddRocket(Raven_Bot* shooter, Vector2D target);
  void AddRailGunSlug(Raven_Bot* shooter, Vector2D target);
  void AddShotGunPellet(Raven_Bot* shooter, Vector2D target);
  void AddBolt(Raven_Bot* shooter, Vector2D target);

  //removes the last bot to be added
  void RemoveBot();

  //returns true if a bot of size BoundingRadius cannot move from A to B
  //without bumping into world geometry
// isPathObstructed:路被挡吗?GetAllBotsInFOV:某 bot 视野里有哪些 bot?
//(原文注释翻译:一个 bot 从 A 到 B 会不会撞墙?)
  bool isPathObstructed(Vector2D A, Vector2D B, double BoundingRadius = 0)const;

  //returns a vector of pointers to bots in the FOV of the given bot
  std::vector<Raven_Bot*> GetAllBotsInFOV(const Raven_Bot* pBot)const;

  //returns true if the second bot is unobstructed by walls and in the field
  //of view of the first.
// isSecondVisibleToFirst:可见吗?isLOSOkay:两点间视线通吗?
//(原文注释翻译:第二个 bot 在第一个 bot 视野内且无遮挡吗?)
  bool        isSecondVisibleToFirst(const Raven_Bot* pFirst,
                                     const Raven_Bot* pSecond)const;

  //returns true if the ray between A and B is unobstructed.
  bool        isLOSOkay(Vector2D A, Vector2D B)const;

  //starting from the given origin and moving in the direction Heading this
  //method returns the distance to the closest wall
// GetDistanceToClosestWall:到墙距离;GetPosOfClosestSwitch:最近的按钮位置。
//(原文注释翻译:从原点沿朝向走,离最近的墙多远)
  double       GetDistanceToClosestWall(Vector2D Origin, Vector2D Heading)const;

  
  //returns the position of the closest visible switch that triggers the
  //door of the specified ID
  Vector2D GetPosOfClosestSwitch(Vector2D botPos, unsigned int doorID)const;

  //given a position on the map this method returns the bot found with its
  //bounding radius of that position.If there is no bot at the position the
  //method returns NULL
// GetBotAtPosition:某位置上的 bot。
//(原文注释翻译:某位置上有 bot 吗?没有返回 NULL)
  Raven_Bot*  GetBotAtPosition(Vector2D CursorPos)const;


// TogglePause:暂停;下面是鼠标/键盘/人类接管接口;最后是一堆 GetXxx() 访问器。
  void        TogglePause(){m_bPaused = !m_bPaused;}
  
  //this method is called when the user clicks the right mouse button.
  //The method checks to see if a bot is beneath the cursor. If so, the bot
  //is recorded as selected.If the cursor is not over a bot then any selected
  // bot/s will attempt to move to that position.
  void        ClickRightMouseButton(POINTS p);

  //this method is called when the user clicks the left mouse button. If there
  //is a possessed bot, this fires the weapon, else does nothing
  void        ClickLeftMouseButton(POINTS p);

  //when called will release any possessed bot from user control
  void        ExorciseAnyPossessedBot();
 
  //if a bot is possessed the keyboard is polled for user input and any 
  //relevant bot methods are called appropriately
  void        GetPlayerInput()const;
  Raven_Bot*  PossessedBot()const{return m_pSelectedBot;}
  void        ChangeWeaponOfPossessedBot(unsigned int weapon)const;

  
  const Raven_Map* const                   GetMap()const{return m_pMap;}
  Raven_Map* const                         GetMap(){return m_pMap;}
  const std::list<Raven_Bot*>&             GetAllBots()const{return m_Bots;}
  PathManager<Raven_PathPlanner>* const    GetPathManager(){return m_pPathManager;}
  int                                      GetNumBots()const{return m_Bots.size();}

  
  void  TagRaven_BotsWithinViewRange(BaseGameEntity* pRaven_Bot, double range)
              {TagNeighbors(pRaven_Bot, m_Bots, range);}  
};





#endif