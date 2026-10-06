// 下面是本文件需要的头文件,详见文件头。
//==============================================================================================
//【文件说明】navigation\Raven_PathPlanner.cpp —— 寻路器的实现
//
//【这个文件是干什么的?】
//  实现 Raven_PathPlanner.h 里声明的所有函数:构造/析构、发起 A* 或 Dijkstra 搜索、
//  取出并平滑路径、被 PathManager 每帧调 CycleOnce() 推进搜索、找最近可见节点等。
//
//【本文件包含了谁?】
//  Raven_PathPlanner.h(自己的声明)、../Raven_Game.h、../Raven_Bot.h、../constants.h、
//  ../Raven_UserOptions.h、../Raven_Messages.h、../lua/Raven_Scriptor.h、
//  SearchTerminationPolicies.h、pathmanager.h,以及 Common 目录下的 utils/Cgdi/
//  GraphAlgorithms/CellSpacePartition/MessageDispatcher/DebugConsole 等。
//==============================================================================================
#include "Raven_PathPlanner.h"
#include "../Raven_Game.h"
#include "misc/utils.h"
#include "graph/GraphAlgorithms.h"
#include "misc/Cgdi.h"
#include "../Raven_Bot.h"
#include "../constants.h"
#include "../Raven_UserOptions.h"
#include "pathmanager.h"
#include "SearchTerminationPolicies.h"
#include "../lua/Raven_Scriptor.h"
#include "misc/CellSpacePartition.h"
#include "../Raven_Messages.h"
#include "Messaging/MessageDispatcher.h"
#include "graph/NodeTypeEnumerations.h"


#include "Debug/DebugConsole.h"
//#define SHOW_NAVINFO
#include <cassert>

//---------------------------- ctor -------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 构造函数:记下 owner,从地图里拿到导航图引用,当前搜索先置空(NULL)。
//--------------------------------------------------------------------------------
Raven_PathPlanner::Raven_PathPlanner(Raven_Bot* owner):m_pOwner(owner),
               m_NavGraph(m_pOwner->GetWorld()->GetMap()->GetNavGraph()),
               m_pCurrentSearch(NULL)
{
}

//-------------------------- dtor ---------------------------------------------
//-----------------------------------------------------------------------------
// 析构函数:调用 GetReadyForNewSearch 清理搜索内存。
Raven_PathPlanner::~Raven_PathPlanner()
{
  GetReadyForNewSearch();
}

//------------------------------ GetReadyForNewSearch -----------------------------------
//
//  called by the search manager when a search has been terminated to free
//  up the memory used when an instance of the search was created
//(原文注释翻译:搜索管理器在一次搜索结束时调用本方法,释放搜索实例占用的内存)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// GetReadyForNewSearch:注销旧搜索、delete 旧搜索对象、指针清零。
void Raven_PathPlanner::GetReadyForNewSearch()
{
  //unregister any existing search with the path manager
//(原文注释:把已有的搜索从路径管理器注销)
  m_pOwner->GetWorld()->GetPathManager()->UnRegister(this);

  //clean up memory used by any existing search
//(原文注释:清理已有搜索占用的内存)
  delete m_pCurrentSearch;    
  m_pCurrentSearch = 0;
}

//---------------------------- GetCostToNode ----------------------------------
//
//  returns the cost to travel from the bot's current position to a specific 
 // graph node. This method makes use of the pre-calculated lookup table
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// GetCostToNode:先找离 bot 最近的节点,加一段步行距离,再加图上两点代价。
double Raven_PathPlanner::GetCostToNode(unsigned int NodeIdx)const
{
  //find the closest visible node to the bots position
  int nd = GetClosestNodeToPosition(m_pOwner->Pos());

  //add the cost to this node
  double cost =Vec2DDistance(m_pOwner->Pos(),
                            m_NavGraph.GetNode(nd).Pos());

  //add the cost to the target node and return
  return cost + m_pOwner->GetWorld()->GetMap()->CalculateCostToTravelBetweenNodes(nd, NodeIdx);
}

//------------------------ GetCostToClosestItem ---------------------------
//
//  returns the cost to the closest instance of the giver type. This method
//  makes use of the pre-calculated lookup table. Returns -1 if no active
//  trigger found
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// GetCostToClosestItem:遍历所有触发器,找离 bot 最近的「激活的同类」。
double Raven_PathPlanner::GetCostToClosestItem(unsigned int GiverType)const
{
  //find the closest visible node to the bots position
  int nd = GetClosestNodeToPosition(m_pOwner->Pos());

  //if no closest node found return failure
//(原文注释:如果没找到最近节点,返回失败)
  if (nd == invalid_node_index) return -1;

// ClosestSoFar:目前找到的最小代价,初值给最大值(MaxDouble),任何真实代价都比它小。
  double ClosestSoFar = MaxDouble;

  //iterate through all the triggers to find the closest *active* trigger of 
  //type GiverType
//(原文注释翻译:遍历所有触发器,找类型为 GiverType 且激活的最近那个)
  const Raven_Map::TriggerSystem::TriggerList& triggers = m_pOwner->GetWorld()->GetMap()->GetTriggers();

  Raven_Map::TriggerSystem::TriggerList::const_iterator it;
// for 循环:逐个检查触发器;类型对且激活才算候选。
  for (it = triggers.begin(); it != triggers.end(); ++it)
  {
    if ( ((*it)->EntityType() == GiverType) && (*it)->isActive())
    {
      double cost = 
      m_pOwner->GetWorld()->GetMap()->CalculateCostToTravelBetweenNodes(nd,
                                                      (*it)->GraphNodeIndex());

      if (cost < ClosestSoFar)
      {
        ClosestSoFar = cost;
      }
    }
  }

  //return a negative value if no active trigger of the type found
// 全程没更新过最小值 = 没找到 → 返回 -1。
  if (isEqual(ClosestSoFar, MaxDouble))
  {
    return -1;
  }

  return ClosestSoFar;
}


//----------------------------- GetPath ------------------------------------
//
//  called by an agent after it has been notified that a search has terminated
//  successfully. The method extracts the path from m_pCurrentSearch, adds
//  additional edges appropriate to the search type and returns it as a list of
//  PathEdges.
//(原文注释翻译:bot 收到搜索成功通知后调用;从当前搜索取出路径,
//           按搜索类型补边,以 PathEdge 串返回。)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// GetPath:取出路径,补两段(起点→最近节点、最后节点→目标坐标),再按选项平滑。
Raven_PathPlanner::Path Raven_PathPlanner::GetPath()
{
  assert (m_pCurrentSearch && 
          "<Raven_PathPlanner::GetPathAsNodes>: no current search");

  Path path =  m_pCurrentSearch->GetPathAsPathEdges();

  int closest = GetClosestNodeToPosition(m_pOwner->Pos());

  path.push_front(PathEdge(m_pOwner->Pos(),
                            GetNodePosition(closest),
                            NavGraphEdge::normal));

  
  //if the bot requested a path to a location then an edge leading to the
  //destination must be added
  if (m_pCurrentSearch->GetType() == Graph_SearchTimeSliced<EdgeType>::AStar)
  {   
    path.push_back(PathEdge(path.back().Destination(),
                            m_vDestinationPos,
                            NavGraphEdge::normal));
  }

  //smooth paths if required
  if (UserOptions->m_bSmoothPathsQuick)
  {
    SmoothPathEdgesQuick(path);
  }

  if (UserOptions->m_bSmoothPathsPrecise)
  {
    SmoothPathEdgesPrecise(path);
  }

  return path;
}

//--------------------------- SmoothPathEdgesQuick ----------------------------
//
//  smooths a path by removing extraneous edges.
//-----------------------------------------------------------------------------
void Raven_PathPlanner::SmoothPathEdgesQuick(Path& path)
{
  //create a couple of iterators and point them at the front of the path
  Path::iterator e1(path.begin()), e2(path.begin());

  //increment e2 so it points to the edge following e1.
  ++e2;

  //while e2 is not the last edge in the path, step through the edges checking
  //to see if the agent can move without obstruction from the source node of
  //e1 to the destination node of e2. If the agent can move between those 
  //positions then the two edges are replaced with a single edge.
  while (e2 != path.end())
  {
    //check for obstruction, adjust and remove the edges accordingly
    if ( (e2->Behavior() == EdgeType::normal) &&
          m_pOwner->canWalkBetween(e1->Source(), e2->Destination()) )
    {
      e1->SetDestination(e2->Destination());
      e2 = path.erase(e2);
    }

    else
    {
      e1 = e2;
      ++e2;
    }
  }
}


//----------------------- SmoothPathEdgesPrecise ---------------------------------
//
//  smooths a path by removing extraneous edges.
//-----------------------------------------------------------------------------
void Raven_PathPlanner::SmoothPathEdgesPrecise(Path& path)
{
  //create a couple of iterators
  Path::iterator e1, e2;

  //point e1 to the beginning of the path
  e1 = path.begin();
    
  while (e1 != path.end())
  {
    //point e2 to the edge immediately following e1
    e2 = e1; 
    ++e2;

    //while e2 is not the last edge in the path, step through the edges
    //checking to see if the agent can move without obstruction from the 
    //source node of e1 to the destination node of e2. If the agent can move
    //between those positions then the any edges between e1 and e2 are
    //replaced with a single edge.
    while (e2 != path.end())
    {
      //check for obstruction, adjust and remove the edges accordingly
      if ( (e2->Behavior() == EdgeType::normal) &&
            m_pOwner->canWalkBetween(e1->Source(), e2->Destination()) )
      {
        e1->SetDestination(e2->Destination());
        e2 = path.erase(++e1, ++e2);
        e1 = e2;
        --e1;
      }

      else
      {
        ++e2;
      }
    }

    ++e1;
  }
}



//---------------------------- CycleOnce --------------------------------------
//
//  the path manager calls this to iterate once though the search cycle
//  of the currently assigned search algorithm.
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// CycleOnce:让当前搜索算一步;结束时给 bot 发 Msg_PathReady 或 Msg_NoPathAvailable。
int Raven_PathPlanner::CycleOnce()const
{
  assert (m_pCurrentSearch && "<Raven_PathPlanner::CycleOnce>: No search object instantiated");

  int result = m_pCurrentSearch->CycleOnce();

  //let the bot know of the failure to find a path
// 没找到 → 发 Msg_NoPathAvailable 消息。
//(原文注释:告诉 bot 找路失败了)
  if (result == target_not_found)
  {
     Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                             SENDER_ID_IRRELEVANT,
                             m_pOwner->ID(),
                             Msg_NoPathAvailable,
                             NO_ADDITIONAL_INFO);

  }

  //let the bot know a path has been found
// 找到了 → 取末端节点的触发器指针,发 Msg_PathReady 消息。
  else if (result == target_found)
  {
    //if the search was for an item type then the final node in the path will
    //represent a giver trigger. Consequently, it's worth passing the pointer
    //to the trigger in the extra info field of the message. (The pointer
    //will just be NULL if no trigger)
//(原文注释翻译:如果搜的是物品类型,路径末端节点上挂的就是给予型触发器;
//           把触发器指针通过消息的附加信息传给 bot(没触发器则为 NULL)。)
    void* pTrigger = 
    m_NavGraph.GetNode(m_pCurrentSearch->GetPathToTarget().back()).ExtraInfo();

    Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                            SENDER_ID_IRRELEVANT,
                            m_pOwner->ID(),
                            Msg_PathReady,
                            pTrigger);
  }

  return result;
}

//------------------------ GetClosestNodeToPosition ---------------------------
//
//  returns the index of the closest visible graph node to the given position
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// GetClosestNodeToPosition:在格子空间里查 pos 附近的节点,挑「可见且最近」的那个。
int Raven_PathPlanner::GetClosestNodeToPosition(Vector2D pos)const
{
  double ClosestSoFar = MaxDouble;
  int   ClosestNode  = no_closest_node_found;

  //when the cell space is queried this the the range searched for neighboring
  //graph nodes. This value is inversely proportional to the density of a 
//(原文注释翻译:这是查询格子空间时搜邻域节点的半径;值与导航图密度成反比——图越稀,半径越大)
  //navigation graph (less dense = bigger values)
  const double range = m_pOwner->GetWorld()->GetMap()->GetCellSpaceNeighborhoodRange();

  //calculate the graph nodes that are neighboring this position
  m_pOwner->GetWorld()->GetMap()->GetCellSpace()->CalculateNeighbors(pos, range);

  //iterate through the neighbors and sum up all the position vectors
  for (NodeType* pN = m_pOwner->GetWorld()->GetMap()->GetCellSpace()->begin();
                 !m_pOwner->GetWorld()->GetMap()->GetCellSpace()->end();     
                 pN = m_pOwner->GetWorld()->GetMap()->GetCellSpace()->next())
  {
    //if the path between this node and pos is unobstructed calculate the
    //distance
// canWalkBetween:两点之间直线能走通?能才算候选;记录最近的。
//(原文注释:如果这个节点和 pos 之间无遮挡,就算距离)
    if (m_pOwner->canWalkBetween(pos, pN->Pos()))
    {
      double dist = Vec2DDistanceSq(pos, pN->Pos());

      //keep a record of the closest so far
      if (dist < ClosestSoFar)
      {
        ClosestSoFar = dist;
        ClosestNode  = pN->Index();
      }
    }
  }
   
  return ClosestNode;
}

//--------------------------- RequestPathToPosition ------------------------------
//
//  Given a target, this method first determines if nodes can be reached from 
//  the  bot's current position and the target position. If either end point
//  is unreachable the method returns false. 
//
//  If nodes are reachable from both positions then an instance of the time-
//  sliced A* search is created and registered with the search manager. the
//  method then returns true.
//        
//-----------------------------------------------------------------------------
bool Raven_PathPlanner::RequestPathToPosition(Vector2D TargetPos)
{ 
  #ifdef SHOW_NAVINFO
    debug_con << "------------------------------------------------" << "";
#endif
  GetReadyForNewSearch();

  //make a note of the target position.
  m_vDestinationPos = TargetPos;

  //if the target is walkable from the bot's position a path does not need to
  //be calculated, the bot can go straight to the position by ARRIVING at
//(原文注释翻译:如果从 bot 位置直接就能走到目标,就不用算图上路径,
//           bot 直接朝目标走即可。)
  //the current waypoint
// canWalkTo:直接走直线就能到?能就不用寻路。
  if (m_pOwner->canWalkTo(TargetPos))
  { 
    return true;
  }
  
  //find the closest visible node to the bots position
  int ClosestNodeToBot = GetClosestNodeToPosition(m_pOwner->Pos());

  //remove the destination node from the list and return false if no visible
  //node found. This will occur if the navgraph is badly designed or if the bot
  //has managed to get itself *inside* the geometry (surrounded by walls),
  //or an obstacle.
//(原文注释翻译:没找到可见节点就从列表移除目标并返回 false;
//           这种情况发生在导航图设计烂了,或 bot 被墙/障碍物围住了。)
  if (ClosestNodeToBot == no_closest_node_found)
  { 
#ifdef SHOW_NAVINFO
    debug_con << "No closest node to bot found!" << "";
#endif

    return false; 
  }

  #ifdef SHOW_NAVINFO
    debug_con << "Closest node to bot is " << ClosestNodeToBot << "";
#endif

  //find the closest visible node to the target position
  int ClosestNodeToTarget = GetClosestNodeToPosition(TargetPos);
  
  //return false if there is a problem locating a visible node from the target.
  //This sort of thing occurs much more frequently than the above. For
  //example, if the user clicks inside an area bounded by walls or inside an
  //object.
  if (ClosestNodeToTarget == no_closest_node_found)
  { 
#ifdef SHOW_NAVINFO
    debug_con << "No closest node to target (" << ClosestNodeToTarget << ") found!" << "";
#endif

    return false; 
  }

  #ifdef SHOW_NAVINFO
    debug_con << "Closest node to target is " << ClosestNodeToTarget << "";
#endif

  //create an instance of a the distributed A* search class
// typedef 起别名:用欧氏距离作启发的 A* 搜索类。
  typedef Graph_SearchAStar_TS<Raven_Map::NavGraph, Heuristic_Euclid> AStar;
   
// new 出一个 A* 搜索实例,起点=bot 最近节点,终点=目标最近节点;然后注册到 PathManager。
  m_pCurrentSearch = new AStar(m_NavGraph,
                               ClosestNodeToBot,
                               ClosestNodeToTarget);

  //and register the search with the path manager
  m_pOwner->GetWorld()->GetPathManager()->Register(this);

  return true;
}


//------------------------------ RequestPathToItem -----------------------------
//
// Given an item type, this method determines the closest reachable graph node
// to the bot's position and then creates a instance of the time-sliced 
// Dijkstra's algorithm, which it registers with the search manager
//
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// RequestPathToItem:找最近节点,建一个「找激活触发器」的 Dijkstra 搜索。
bool Raven_PathPlanner::RequestPathToItem(unsigned int ItemType)
{    
  //clear the waypoint list and delete any active search
  GetReadyForNewSearch();

  //find the closest visible node to the bots position
  int ClosestNodeToBot = GetClosestNodeToPosition(m_pOwner->Pos());

  //remove the destination node from the list and return false if no visible
  //node found. This will occur if the navgraph is badly designed or if the bot
  //has managed to get itself *inside* the geometry (surrounded by walls),
  //or an obstacle
  if (ClosestNodeToBot == no_closest_node_found)
  { 
#ifdef SHOW_NAVINFO
    debug_con << "No closest node to bot found!" << "";
#endif

    return false; 
  }

  //create an instance of the search algorithm
// 终止策略=FindActiveTrigger(找激活触发器);Dijkstra 搜索用它当终止条件。
  typedef FindActiveTrigger<Trigger<Raven_Bot> > t_con; 
  typedef Graph_SearchDijkstras_TS<Raven_Map::NavGraph, t_con> DijSearch;
  
  m_pCurrentSearch = new DijSearch(m_NavGraph,
                                   ClosestNodeToBot,
                                   ItemType);  

  //register the search with the path manager
  m_pOwner->GetWorld()->GetPathManager()->Register(this);

  return true;
}

//------------------------------ GetNodePosition ------------------------------
//
//  used to retrieve the position of a graph node from its index. (takes
//  into account the enumerations 'non_graph_source_node' and 
//  'non_graph_target_node'
//(原文注释翻译:按编号取节点坐标;要处理 non_graph_source_node /
//           non_graph_target_node 这两个特殊编号。)
//----------------------------------------------------------------------------- 
// GetNodePosition:直接从导航图取节点坐标。
Vector2D Raven_PathPlanner::GetNodePosition(int idx)const
{
  return m_NavGraph.GetNode(idx).Pos();
}
  
 


