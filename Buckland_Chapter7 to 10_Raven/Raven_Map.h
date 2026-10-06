//==============================================================================================
//【文件说明】Raven_Map.h —— 游戏地图
//
//【这个文件是干什么的?】
//  地图就是「环境里所有静态东西」的集合:墙、触发器(加血/给武器/按钮)、
//  门、出生点,还有一张导航图(寻路用的图)。它能从地图编辑器文件读出这些几何。
//
//【谁在使用这个文件?】
//  Raven_Game.h/.cpp —— 持有一个 Raven_Map,每帧调 UpdateTriggerSystem;
//  Raven_PathPlanner —— 从地图拿导航图做寻路。
#ifndef RAVEN_MAP_H
#define RAVEN_MAP_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Map.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   this class creates and stores all the entities that make up the
//          Raven game environment. (walls, bots, health etc)
//
//          It can read a Raven map editor file and recreate the necessary
//          geometry.
//-----------------------------------------------------------------------------
#include <vector>
#include <string>
#include <list>
#include "graph/SparseGraph.h"
#include "2d/Wall2D.h"
#include "triggers/Trigger.h"
#include "Raven_Bot.h"
#include "Graph/GraphEdgeTypes.h"
#include "Graph/GraphNodeTypes.h"
#include "misc/CellSpacePartition.h"
#include "triggers/TriggerSystem.h"

class BaseGameEntity;
class Raven_Door;


//--------------------------------------------------------------------------------
// class Raven_Map —— 地图类。
class Raven_Map
{
public:

// 下面一串 typedef:导航图节点/导航图/格子空间/触发器/触发器系统。
  typedef NavGraphNode<Trigger<Raven_Bot>*>         GraphNode;
  typedef SparseGraph<GraphNode, NavGraphEdge>      NavGraph;
  typedef CellSpacePartition<NavGraph::NodeType*>   CellSpace;

  typedef Trigger<Raven_Bot>                        TriggerType;
  typedef TriggerSystem<TriggerType>                TriggerSystem;
  
private:
 
  //the walls that comprise the current map's architecture. 
// m_Walls:所有墙;m_TriggerSystem:触发器系统(加血/给枪/按钮/声音通知)。
//(原文注释:组成地图建筑结构的墙)
  std::vector<Wall2D*>                m_Walls;

  //trigger are objects that define a region of space. When a raven bot
  //enters that area, it 'triggers' an event. That event may be anything
  //from increasing a bot's health to opening a door or requesting a lift.
  TriggerSystem                      m_TriggerSystem;    

  //this holds a number of spawn positions. When a bot is instantiated
  //it will appear at a randomly selected point chosen from this vector
// m_SpawnPoints:出生点;m_Doors:所有滑动门。
//(原文注释翻译:出生点列表;机器人实例化时随机选一个)
  std::vector<Vector2D>              m_SpawnPoints;

  //a map may contain a number of sliding doors.
  std::vector<Raven_Door*>           m_Doors;
 
  //this map's accompanying navigation graph
// m_pNavGraph:导航图;m_pSpacePartition:格子空间分区(加速查邻居)。
//(原文注释:这张地图附带的导航图)
  NavGraph*                          m_pNavGraph;  

  //the graph nodes will be partitioned enabling fast lookup
  CellSpace*                        m_pSpacePartition;

  //the size of the search radius the cellspace partition uses when looking for 
  //neighbors 
// m_dCellSpaceNeighborhoodRange:邻居搜索半径;m_iSizeX/Y:地图宽高。
//(原文注释:格子空间查邻居时用的搜索半径)
  double                             m_dCellSpaceNeighborhoodRange;

  int m_iSizeX;
  int m_iSizeY;
  
  void  PartitionNavGraph();

  //this will hold a pre-calculated lookup table of the cost to travel from
  //one node to any other.
// m_PathCosts:节点两两之间代价表(预计算加速);下面是从文件读地图的私有函数。
//(原文注释翻译:预计算的节点间代价查找表)
  std::vector<std::vector<double> >  m_PathCosts;


    //stream constructors for loading from a file
  void AddWall(std::ifstream& in);
  void AddSpawnPoint(std::ifstream& in);
  void AddHealth_Giver(std::ifstream& in);
  void AddWeapon_Giver(int type_of_weapon, std::ifstream& in);
  void AddDoor(std::ifstream& in);
  void AddDoorTrigger(std::ifstream& in);

  void Clear();
  
public:
  
// 构造/析构;Render 画地图;LoadMap 从文件读地图。
  Raven_Map();  
  ~Raven_Map();

  void Render();

  //loads an environment from a file
  bool LoadMap(const std::string& FileName); 

  //adds a wall and returns a pointer to that wall. (this method can be
  //used by objects such as doors to add walls to the environment)
// AddWall:加墙;AddSoundTrigger:加声音触发器;CalculateCostToTravelBetweenNodes:查两节点代价。
//(原文注释翻译:加一面墙并返回指针;门也用它加墙)
  Wall2D* AddWall(Vector2D from, Vector2D to);

  void    AddSoundTrigger(Raven_Bot* pSoundSource, double range);

  double   CalculateCostToTravelBetweenNodes(int nd1, int nd2)const;

  //returns the position of a graph node selected at random
  Vector2D GetRandomNodeLocation()const;
  
  
// UpdateTriggerSystem:每帧让触发器系统测试所有 bot;下面是一堆 GetXxx() 访问器。
  void  UpdateTriggerSystem(std::list<Raven_Bot*>& bots);

  const Raven_Map::TriggerSystem::TriggerList&  GetTriggers()const{return m_TriggerSystem.GetTriggers();}
  const std::vector<Wall2D*>&        GetWalls()const{return m_Walls;}
  NavGraph&                          GetNavGraph()const{return *m_pNavGraph;}
  std::vector<Raven_Door*>&          GetDoors(){return m_Doors;}
  const std::vector<Vector2D>&       GetSpawnPoints()const{return m_SpawnPoints;}
  CellSpace* const                   GetCellSpace()const{return m_pSpacePartition;}
  Vector2D                           GetRandomSpawnPoint(){return m_SpawnPoints[RandInt(0,m_SpawnPoints.size()-1)];}
  int                                GetSizeX()const{return m_iSizeX;}
  int                                GetSizeY()const{return m_iSizeY;}
  int                                GetMaxDimension()const{return Maximum(m_iSizeX, m_iSizeY);}
  double                             GetCellSpaceNeighborhoodRange()const{return m_dCellSpaceNeighborhoodRange;}

};



#endif