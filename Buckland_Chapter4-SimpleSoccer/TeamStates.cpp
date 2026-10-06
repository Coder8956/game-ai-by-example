//==============================================================================================
//【文件说明】TeamStates.cpp —— 球队级 3 个战术状态类的实现
//
//【这个文件是干什么的?】
//  实现 Attacking/Defending/PrepareForKickOff 的 Enter/Execute/Exit。
//  关键在于:进攻/防守时,每个球员的"老家区域"站位图不一样(见 BlueRegions/RedRegions),
//  切入状态时就把这些站位数组写回各球员。
//
//【本文件包含了谁?】
//  自己的 .h、SoccerTeam.h、PlayerBase.h、SoccerPitch.h、MessageDispatcher、constants.h 等。
//==============================================================================================
#include "TeamStates.h"
#include "SoccerTeam.h"
#include "PlayerBase.h"
#include "Messaging/MessageDispatcher.h"
#include "SoccerMessages.h"
#include "constants.h"
#include "SoccerPitch.h"

//uncomment to send state info to debug window
//#define DEBUG_TEAM_STATES
#include "Debug/DebugConsole.h"




// 自由函数 ChangePlayerHomeRegions:遍历全队,把 NewRegions 数组里的区域编号
// 逐个设为对应球员的老家区域(进攻/防守站位不同就靠它切换)。
void ChangePlayerHomeRegions(SoccerTeam* team, const int NewRegions[TeamSize])
{
  for (int plyr=0; plyr<TeamSize; ++plyr)
  {
    team->SetPlayerHomeRegion(plyr, NewRegions[plyr]);
  }
}

//************************************************************************ ATTACKING

//**************** 进攻 Attacking ****************
// Enter:按球队颜色套用进攻站位(BlueRegions/RedRegions),并更新等待中球员的目标;
// Execute:本队一旦丢球→切防守;否则持续算最佳接应点。Exit:清掉接应者。
Attacking* Attacking::Instance()
{
  static Attacking instance;

  return &instance;
}


void Attacking::Enter(SoccerTeam* team)
{
#ifdef DEBUG_TEAM_STATES
  debug_con << team->Name() << " entering Attacking state" << "";
#endif

  //these define the home regions for this state of each of the players
  const int BlueRegions[TeamSize] = {1,12,14,6,4};
  const int RedRegions[TeamSize] = {16,3,5,9,13};

  //set up the player's home regions
  if (team->Color() == SoccerTeam::blue)
  {
    ChangePlayerHomeRegions(team, BlueRegions);
  }
  else
  {
    ChangePlayerHomeRegions(team, RedRegions);
  }

  //if a player is in either the Wait or ReturnToHomeRegion states, its
  //steering target must be updated to that of its new home region to enable
  //it to move into the correct position.
  team->UpdateTargetsOfWaitingPlayers();
}


void Attacking::Execute(SoccerTeam* team)
{
  //if this team is no longer in control change states
  if (!team->InControl())
  {
    team->GetFSM()->ChangeState(Defending::Instance()); return;
  }

  //calculate the best position for any supporting attacker to move to
  team->DetermineBestSupportingPosition();
}

void Attacking::Exit(SoccerTeam* team)
{
  //there is no supporting player for defense
  team->SetSupportingPlayer(NULL);
}



//************************************************************************ DEFENDING

//**************** 防守 Defending ****************
// Enter:套用防守站位数组;Execute:本队一旦控球→切进攻。
Defending* Defending::Instance()
{
  static Defending instance;

  return &instance;
}

void Defending::Enter(SoccerTeam* team)
{
#ifdef DEBUG_TEAM_STATES
  debug_con << team->Name() << " entering Defending state" << "";
#endif

  //these define the home regions for this state of each of the players
  const int BlueRegions[TeamSize] = {1,6,8,3,5};
  const int RedRegions[TeamSize] = {16,9,11,12,14};

  //set up the player's home regions
  if (team->Color() == SoccerTeam::blue)
  {
    ChangePlayerHomeRegions(team, BlueRegions);
  }
  else
  {
    ChangePlayerHomeRegions(team, RedRegions);
  }
  
  //if a player is in either the Wait or ReturnToHomeRegion states, its
  //steering target must be updated to that of its new home region
  team->UpdateTargetsOfWaitingPlayers();
}

void Defending::Execute(SoccerTeam* team)
{
  //if in control change states
  if (team->InControl())
  {
    team->GetFSM()->ChangeState(Attacking::Instance()); return;
  }
}


void Defending::Exit(SoccerTeam* team){}


//************************************************************************ KICKOFF
//**************** 开球准备 PrepareForKickOff ****************
// Enter:清空关键球员指针,叫所有球员回家;
// Execute:双方全员都回到位→切防守(开球);Exit:吹响哨子 SetGameOn 正式开赛。
PrepareForKickOff* PrepareForKickOff::Instance()
{
  static PrepareForKickOff instance;

  return &instance;
}

void PrepareForKickOff::Enter(SoccerTeam* team)
{
  //reset key player pointers
  team->SetControllingPlayer(NULL);
  team->SetSupportingPlayer(NULL);
  team->SetReceiver(NULL);
  team->SetPlayerClosestToBall(NULL);

  //send Msg_GoHome to each player.
  team->ReturnAllFieldPlayersToHome();
}

void PrepareForKickOff::Execute(SoccerTeam* team)
{
  //if both teams in position, start the game
  if (team->AllPlayersAtHome() && team->Opponents()->AllPlayersAtHome())
  {
    team->GetFSM()->ChangeState(Defending::Instance());
  }
}

void PrepareForKickOff::Exit(SoccerTeam* team)
{
  team->Pitch()->SetGameOn();
}


