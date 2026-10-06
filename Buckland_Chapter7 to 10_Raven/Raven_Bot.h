//==============================================================================================
//【文件说明】Raven_Bot.h —— 机器人本体(整个游戏的核心实体)
//
//【机器人内部模块构成图】
//
//   Raven_Bot (移动实体 MovingEntity)
//     |-- m_pBrain            Goal_Think         (高层目标决策,goals 目录,不在本分片)
//     |-- m_pSensoryMem       Raven_SensoryMemory (感官记忆:看见/听见了谁)
//     |-- m_pTargSys          Raven_TargetingSystem(从记忆里选一个最近的当目标)
//     |-- m_pSteering         Raven_Steering       (转向行为:seek/arrive/wander/避墙/分离)
//     |-- m_pPathPlanner      Raven_PathPlanner   (导航:A* 寻路)
//     |-- m_pWeaponSys        Raven_WeaponSystem  (武器:瞄准/射击/选枪)
//     |-- m_pWeaponSelectionRegulator / m_pGoalArbitrationRegulator /
//     |   m_pTargetSelectionRegulator / m_pTriggerTestRegulator /
//     |   m_pVisionUpdateRegulator              (频率限制器:不是每帧都跑,省 CPU)
//
//【一句话】Raven_Bot 本身只是个「容器 + 状态(血/分/朝向)」,真正的智能分散在上面 5 个子系统里。
//
//【谁在使用这个文件?】
//  Raven_Game.h/.cpp —— 持有所有 bot,每帧 Update/Render;
//  所有子系统(感官/目标/武器/导航/转向)—— 都持有 m_pOwner 指回本 bot。
//==============================================================================================
#ifndef RAVEN_BOT_H
#define RAVEN_BOT_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Bot.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:
//-----------------------------------------------------------------------------
#include <vector>
#include <iosfwd>
#include <map>

#include "game/MovingEntity.h"
#include "misc/utils.h"
#include "Raven_TargetingSystem.h"


class Raven_PathPlanner;
class Raven_Steering;
class Raven_Game;
class Regulator;
class Raven_Weapon;
struct Telegram;
class Raven_Bot;
class Goal_Think;
class Raven_WeaponSystem;
class Raven_SensoryMemory;




//--------------------------------------------------------------------------------
// class Raven_Bot —— 机器人类,继承 MovingEntity(会动的游戏实体)。
class Raven_Bot : public MovingEntity
{
private:

// 状态枚举:活着/死了/出生中。
  enum Status{alive, dead, spawning};

private:

  //alive, dead or spawning?
//(原文注释:活着、死了还是出生中?)
  Status                             m_Status;

  //a pointer to the world data
// m_pWorld:世界指针(地图/所有 bot/触发器都能从它拿到)。
  Raven_Game*                        m_pWorld;

  //this object handles the arbitration and processing of high level goals
  Goal_Think*                        m_pBrain;

  //this is a class that acts as the bots sensory memory. Whenever this
  //bot sees or hears an opponent, a record of the event is updated in the 
  //memory.
// 下面是 5 个子系统指针:脑(Goal_Think)/感官记忆/转向/寻路/目标系统/武器系统。
  Raven_SensoryMemory*               m_pSensoryMem;

  //the bot uses this object to steer
  Raven_Steering*                    m_pSteering;

  //the bot uses this to plan paths
  Raven_PathPlanner*                 m_pPathPlanner;

  //this is responsible for choosing the bot's current target
  Raven_TargetingSystem*             m_pTargSys;

  //this handles all the weapons. and has methods for aiming, selecting and
  //shooting them
  Raven_WeaponSystem*                m_pWeaponSys;

  //A regulator object limits the update frequency of a specific AI component
// 下面 5 个 Regulator:选武器/目标仲裁/选目标/测触发器/更新视野,
//   各自按自己的频率跑,不是每帧都跑一遍,省 CPU。
  Regulator*                         m_pWeaponSelectionRegulator;
  Regulator*                         m_pGoalArbitrationRegulator;
  Regulator*                         m_pTargetSelectionRegulator;
  Regulator*                         m_pTriggerTestRegulator;
  Regulator*                         m_pVisionUpdateRegulator;

  //the bot's health. Every time the bot is shot this value is decreased. If
  //it reaches zero then the bot dies (and respawns)
// m_iHealth:当前血;m_iMaxHealth:满血;m_iScore:杀人得分。
//(原文注释翻译:机器人的血;被打就减,到 0 就死(然后重生))
  int                                m_iHealth;
  
  //the bot's maximum health value. It starts its life with health at this value
  int                                m_iMaxHealth;

  //each time this bot kills another this value is incremented
  int                                m_iScore;
  
  //the direction the bot is facing (and therefore the direction of aim). 
  //Note that this may not be the same as the bot's heading, which always
  //points in the direction of the bot's movement
// m_vFacing:脸朝的方向;m_dFieldOfView:视野角(锥半角)。
//(原文注释翻译:机器人面朝方向(也是瞄准方向);注意它不一定等于 heading——
//           heading 永远指向移动方向,而 facing 是脸朝哪)
  Vector2D                           m_vFacing;

  //a bot only perceives other bots within this field of view
  double                             m_dFieldOfView;
  
  //to show that a player has been hit it is surrounded by a thick 
  //red circle for a fraction of a second. This variable represents
  //the number of update-steps the circle gets drawn
  int                                m_iNumUpdatesHitPersistant;

  //set to true when the bot is hit, and remains true until 
  //m_iNumUpdatesHitPersistant becomes zero. (used by the render method to
  //draw a thick red circle around a bot to indicate it's been hit)
// m_bHit:刚被打(画红圈);m_bPossessed:是否被人类玩家接管。
//(原文注释翻译:被打中时画个红圈;m_bHit=true 表示刚被打)
  bool                               m_bHit;

  //set to true when a human player takes over control of the bot
  bool                               m_bPossessed;

  //a vertex buffer containing the bot's geometry
// m_vecBotVB:机器人外形多边形顶点;m_vecBotVBTrans:变换后的顶点。
  std::vector<Vector2D>              m_vecBotVB;
  //the buffer for the transformed vertices
  std::vector<Vector2D>              m_vecBotVBTrans;


  //bots shouldn't be copied, only created or respawned
// 把拷贝构造和 operator= 声明成私有,禁止别人拷贝 bot。
//(原文注释:机器人不应被拷贝,只能创建或重生)
  Raven_Bot(const Raven_Bot&);
  Raven_Bot& operator=(const Raven_Bot&);

  //this method is called from the update method. It calculates and applies
  //the steering force for this time-step.
// UpdateMovement:本帧移动;SetUpVertexBuffer:初始化外形顶点。
  void          UpdateMovement();

  //initializes the bot's VB with its geometry
  void          SetUpVertexBuffer();


public:
  
// 构造/析构;Render/Update/HandleMessage 是标准接口。
  Raven_Bot(Raven_Game* world, Vector2D pos);
  virtual ~Raven_Bot();

  //the usual suspects
  void         Render();
  void         Update();
  bool         HandleMessage(const Telegram& msg);
  void         Write(std::ostream&  os)const{/*not implemented*/}
  void         Read (std::ifstream& is){/*not implemented*/}

  //this rotates the bot's heading until it is facing directly at the target
  //position. Returns false if not facing at the target.
// RotateFacingTowardPosition:转身朝向某点。
//(原文注释翻译:转动机器人朝向直到正对目标;没正对返回 false)
  bool          RotateFacingTowardPosition(Vector2D target);
 
  //methods for accessing attribute data
// 下面一堆 Health/Score/Facing/isAlive 等是访问器(inline 取成员)。
  int           Health()const{return m_iHealth;}
  int           MaxHealth()const{return m_iMaxHealth;}
  void          ReduceHealth(unsigned int val);
  void          IncreaseHealth(unsigned int val);
  void          RestoreHealthToMaximum();

  int           Score()const{return m_iScore;}
  void          IncrementScore(){++m_iScore;}

  Vector2D      Facing()const{return m_vFacing;}
  double        FieldOfView()const{return m_dFieldOfView;}

  bool          isPossessed()const{return m_bPossessed;}
  bool          isDead()const{return m_Status == dead;}
  bool          isAlive()const{return m_Status == alive;}
  bool          isSpawning()const{return m_Status == spawning;}
  
  void          SetSpawning(){m_Status = spawning;}
  void          SetDead(){m_Status = dead;}
  void          SetAlive(){m_Status = alive;}

  //returns a value indicating the time in seconds it will take the bot
  //to reach the given position at its current speed.
// CalculateTimeToReachPosition:走到某点要多久;isAtPosition:离某点近吗?
//(原文注释翻译:以当前速度走到某点需要多少秒)
  double        CalculateTimeToReachPosition(Vector2D pos)const; 

  //returns true if the bot is close to the given position
  bool          isAtPosition(Vector2D pos)const;


  //interface for human player
// 下面是人类玩家接管后的接口:开火/换武器/接管/离开。
  void          FireWeapon(Vector2D pos);
  void          ChangeWeapon(unsigned int type);
  void          TakePossession();
  void          Exorcise();

  //spawns the bot at the given position
  void          Spawn(Vector2D pos);
  
  //returns true if this bot is ready to test against all triggers
  bool          isReadyForTriggerUpdate()const;

  //returns true if the bot has line of sight to the given position.
  bool          hasLOSto(Vector2D pos)const;

  //returns true if this bot can move directly to the given position
  //without bumping into any walls
// canWalkTo:能直接走到;canWalkBetween:两点间能直接走通;canStepLeft/Right/Forward/Backward:四方向能否迈步。
//(原文注释翻译:能直接走到某点不撞墙吗?)
  bool          canWalkTo(Vector2D pos)const;

  //similar to above. Returns true if the bot can move between the two
  //given positions without bumping into any walls
  bool          canWalkBetween(Vector2D from, Vector2D to)const;

  //returns true if there is space enough to step in the indicated direction
  //If true PositionOfStep will be assigned the offset position
  bool          canStepLeft(Vector2D& PositionOfStep)const;
  bool          canStepRight(Vector2D& PositionOfStep)const;
  bool          canStepForward(Vector2D& PositionOfStep)const;
  bool          canStepBackward(Vector2D& PositionOfStep)const;

  
// 下面一堆 GetXxx():取出各子系统指针。
  Raven_Game* const                  GetWorld(){return m_pWorld;} 
  Raven_Steering* const              GetSteering(){return m_pSteering;}
  Raven_PathPlanner* const           GetPathPlanner(){return m_pPathPlanner;}
  Goal_Think* const                  GetBrain(){return m_pBrain;}
  const Raven_TargetingSystem* const GetTargetSys()const{return m_pTargSys;}
  Raven_TargetingSystem* const       GetTargetSys(){return m_pTargSys;}
  Raven_Bot* const                   GetTargetBot()const{return m_pTargSys->GetTarget();}
  Raven_WeaponSystem* const          GetWeaponSys()const{return m_pWeaponSys;}
  Raven_SensoryMemory* const         GetSensoryMem()const{return m_pSensoryMem;}


};




#endif