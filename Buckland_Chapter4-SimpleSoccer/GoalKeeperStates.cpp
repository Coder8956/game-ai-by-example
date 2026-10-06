//==============================================================================================
//【文件说明】GoalKeeperStates.cpp —— 守门员 5 个状态类的具体实现
//
//【这个文件是干什么的?】
//  实现 TendGoal(守门)/InterceptBall(拦截)/ReturnHome(回门)/PutBallBackInPlay(发球)
//  等状态的 Enter/Execute/Exit;切换条件写在各 Execute 里(如球进手范围→拿球发球)。
//
//【本文件包含了谁?】
//  自己的 .h、GoalKeeper.h、SoccerTeam.h、SoccerPitch.h、SoccerBall.h、Goal.h、
//  SteeringBehaviors.h、ParamLoader.h、MessageDispatcher、SoccerMessages 等。
//==============================================================================================
#include "GoalKeeperStates.h"
#include "Debug/DebugConsole.h"
#include "SoccerPitch.h"
#include "PlayerBase.h"
#include "GoalKeeper.h"
#include "SteeringBehaviors.h"
#include "SoccerTeam.h"
#include "Goal.h"
#include "2D/geometry.h"
#include "FieldPlayer.h"
#include "ParamLoader.h"
#include "Messaging/Telegram.h"
#include "Messaging/MessageDispatcher.h"
#include "SoccerMessages.h"


//uncomment to send state info to debug window
//#define GOALY_STATE_INFO_ON


//--------------------------- GlobalKeeperState -------------------------------
//-----------------------------------------------------------------------------

//**************** 全局状态 GlobalKeeperState ****************
// OnMessage:收到 Msg_GoHome→回门;收到 Msg_ReceiveBall→去拦截。
GlobalKeeperState* GlobalKeeperState::Instance()
{
  static GlobalKeeperState instance;

  return &instance;
}


bool GlobalKeeperState::OnMessage(GoalKeeper* keeper, const Telegram& telegram)
{
  switch(telegram.Msg)
  {
    case Msg_GoHome:
    {
      keeper->SetDefaultHomeRegion();
      
      keeper->GetFSM()->ChangeState(ReturnHome::Instance());
    }

    break;

    case Msg_ReceiveBall:
      {
        keeper->GetFSM()->ChangeState(InterceptBall::Instance());
      }

      break;

  }//end switch

  return false;
}


//--------------------------- TendGoal -----------------------------------
//
//  This is the main state for the goalkeeper. When in this state he will
//  move left to right across the goalmouth using the 'interpose' steering
//  behavior to put himself between the ball and the back of the net.
//
//  If the ball comes within the 'goalkeeper range' he moves out of the
//  goalmouth to attempt to intercept it. (see next state)
//------------------------------------------------------------------------

//--------------------------------------------------------------------------------
//**************** 守门 TendGoal(主状态)****************
// Enter:开 Interpose(把自己插在球与门线目标之间),并设门线上的站位目标;
// Execute:每帧更新站位目标;球到手范围→停球并切发球;球进拦截范围且球队没控球→拦截;
// 离门太远且本队控球→回门。Exit:关 Interpose。
//--------------------------------------------------------------------------------
TendGoal* TendGoal::Instance()
{
  static TendGoal instance;

  return &instance;
}


void TendGoal::Enter(GoalKeeper* keeper)
{
  //turn interpose on
  keeper->Steering()->InterposeOn(Prm.GoalKeeperTendingDistance);

  //interpose will position the agent between the ball position and a target
  //position situated along the goal mouth. This call sets the target
  keeper->Steering()->SetTarget(keeper->GetRearInterposeTarget());
}

void TendGoal::Execute(GoalKeeper* keeper)
{
  //the rear interpose target will change as the ball's position changes
  //so it must be updated each update-step 
  keeper->Steering()->SetTarget(keeper->GetRearInterposeTarget());

  //if the ball comes in range the keeper traps it and then changes state
  //to put the ball back in play
  if (keeper->BallWithinKeeperRange())
  {
    keeper->Ball()->Trap();

    keeper->Pitch()->SetGoalKeeperHasBall(true);

    keeper->GetFSM()->ChangeState(PutBallBackInPlay::Instance());

    return;
  }

  //if ball is within a predefined distance, the keeper moves out from
  //position to try and intercept it.
  if (keeper->BallWithinRangeForIntercept() && !keeper->Team()->InControl())
  {
    keeper->GetFSM()->ChangeState(InterceptBall::Instance());
  }

  //if the keeper has ventured too far away from the goal-line and there
  //is no threat from the opponents he should move back towards it
  if (keeper->TooFarFromGoalMouth() && keeper->Team()->InControl())
  {
    keeper->GetFSM()->ChangeState(ReturnHome::Instance());

    return;
  }
}


void TendGoal::Exit(GoalKeeper* keeper)
{
  keeper->Steering()->InterposeOff();
}


//------------------------- ReturnHome: ----------------------------------
//
//  In this state the goalkeeper simply returns back to the center of
//  the goal region before changing state back to TendGoal
//------------------------------------------------------------------------

//**************** 回门 ReturnHome ****************
// Arrive 回老家区域中心;到位或球队丢权→切回守门 TendGoal。
ReturnHome* ReturnHome::Instance()
{
  static ReturnHome instance;

  return &instance;
}


void ReturnHome::Enter(GoalKeeper* keeper)
{
  keeper->Steering()->ArriveOn();
}

void ReturnHome::Execute(GoalKeeper* keeper)
{
  keeper->Steering()->SetTarget(keeper->HomeRegion()->Center());

  //if close enough to home or the opponents get control over the ball,
  //change state to tend goal
  if (keeper->InHomeRegion() || !keeper->Team()->InControl())
  {
    keeper->GetFSM()->ChangeState(TendGoal::Instance());
  }
}

void ReturnHome::Exit(GoalKeeper* keeper)
{
  keeper->Steering()->ArriveOff();
}



//----------------- InterceptBall ----------------------------------------
//
//  In this state the GP will attempt to intercept the ball using the
//  pursuit steering behavior, but he only does so so long as he remains
//  within his home region.
//------------------------------------------------------------------------

//**************** 拦截 InterceptBall ****************
// Enter:开 Pursuit(追球);Execute:离门太远且不是全场离球最近者→回门;
// 球到手范围→停球并切发球。Exit:关 Pursuit。
InterceptBall* InterceptBall::Instance()
{
  static InterceptBall instance;

  return &instance;
}


void InterceptBall::Enter(GoalKeeper* keeper)
{
  keeper->Steering()->PursuitOn();  

    #ifdef GOALY_STATE_INFO_ON
    debug_con << "Goaly " << keeper->ID() << " enters InterceptBall" <<  "";
    #endif
}

void InterceptBall::Execute(GoalKeeper* keeper)
{ 
  //if the goalkeeper moves to far away from the goal he should return to his
  //home region UNLESS he is the closest player to the ball, in which case,
  //he should keep trying to intercept it.
  if (keeper->TooFarFromGoalMouth() && !keeper->isClosestPlayerOnPitchToBall())
  {
    keeper->GetFSM()->ChangeState(ReturnHome::Instance());

    return;
  }
  
  //if the ball becomes in range of the goalkeeper's hands he traps the 
  //ball and puts it back in play
  if (keeper->BallWithinKeeperRange())
  {
    keeper->Ball()->Trap();
    
    keeper->Pitch()->SetGoalKeeperHasBall(true);

    keeper->GetFSM()->ChangeState(PutBallBackInPlay::Instance());

    return;
  }
}

void InterceptBall::Exit(GoalKeeper* keeper)
{
  keeper->Steering()->PursuitOff();
}



//--------------------------- PutBallBackInPlay --------------------------
//
//------------------------------------------------------------------------

//**************** 发球 PutBallBackInPlay ****************
// Enter:声明守门员控球,叫双方球员都回位;
// Execute:找到能传的队友就把球踢过去、通知接球者、切回守门;找不到就原地等。
PutBallBackInPlay* PutBallBackInPlay::Instance()
{
  static PutBallBackInPlay instance;

  return &instance;
}

void PutBallBackInPlay::Enter(GoalKeeper* keeper)
{
  //let the team know that the keeper is in control
  keeper->Team()->SetControllingPlayer(keeper);

  //send all the players home
  keeper->Team()->Opponents()->ReturnAllFieldPlayersToHome();
  keeper->Team()->ReturnAllFieldPlayersToHome();
}


void PutBallBackInPlay::Execute(GoalKeeper* keeper)
{
  PlayerBase*  receiver = NULL;
  Vector2D     BallTarget;
    
  //test if there are players further forward on the field we might
  //be able to pass to. If so, make a pass.
  if (keeper->Team()->FindPass(keeper,
                              receiver,
                              BallTarget,
                              Prm.MaxPassingForce,
                              Prm.GoalkeeperMinPassDist))
  {     
    //make the pass   
    keeper->Ball()->Kick(Vec2DNormalize(BallTarget - keeper->Ball()->Pos()),
                         Prm.MaxPassingForce);

    //goalkeeper no longer has ball 
    keeper->Pitch()->SetGoalKeeperHasBall(false);

    //let the receiving player know the ball's comin' at him
    Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                          keeper->ID(),
                          receiver->ID(),
                          Msg_ReceiveBall,
                          &BallTarget);
    
    //go back to tending the goal   
    keeper->GetFSM()->ChangeState(TendGoal::Instance());

    return;
  }  

  keeper->SetVelocity(Vector2D());
}

