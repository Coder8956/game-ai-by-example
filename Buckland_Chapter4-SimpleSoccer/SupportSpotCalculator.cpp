//==============================================================================================
//【文件说明】SupportSpotCalculator.cpp —— 接应甜区评分计算器的实现
//
//【这个文件是干什么的?】
//  构造时把半场切成一张格子网;DetermineBestSupportingPosition() 逐点打分:
//  ①传球是否安全 ②能否射门 ③离控球者距离是否接近最佳距离 200 像素;
//  选出总分最高的点作为最佳接应点,并用 Regulator 限频(不必每帧重算)。
//
//【本文件包含了谁?】
//  自己的 .h、PlayerBase.h、Goal.h、SoccerBall.h、SoccerTeam.h、SoccerPitch.h、
//  ParamLoader.h、constants.h、time/regulator.h、debug/DebugConsole.h。
//==============================================================================================
#include "SupportSpotCalculator.h"
#include "PlayerBase.h"
#include "Goal.h"
#include "SoccerBall.h"
#include "constants.h"
#include "time/regulator.h"
#include "SoccerTeam.h"
#include "ParamLoader.h"
#include "SoccerPitch.h"

#include "debug/DebugConsole.h"

//------------------------------- dtor ----------------------------------------
//-----------------------------------------------------------------------------
// 析构函数:释放构造时 new 出来的限频器 m_pRegulator,防内存泄漏。
SupportSpotCalculator::~SupportSpotCalculator()
{
  delete m_pRegulator;
}


//------------------------------- ctor ----------------------------------------
//-----------------------------------------------------------------------------
// 构造函数:按 numX×numY 把比赛区域中心一块矩形切成格子,每个格子中心建一个接应点;
// 蓝队从左半区排、红队从右半区排(用 Color() 判断);最后 new 一个 Regulator 限频器。
// push_back:把元素追加进 vector 容器末尾。
SupportSpotCalculator::SupportSpotCalculator(int           numX,
                                             int           numY,
                                             SoccerTeam*   team):m_pBestSupportingSpot(NULL),
                                                                  m_pTeam(team)
{
  const Region* PlayingField = team->Pitch()->PlayingArea();

  //calculate the positions of each sweet spot, create them and 
  //store them in m_Spots
  double HeightOfSSRegion = PlayingField->Height() * 0.8;
  double WidthOfSSRegion  = PlayingField->Width() * 0.9;
  double SliceX = WidthOfSSRegion / numX ;
  double SliceY = HeightOfSSRegion / numY;

  double left  = PlayingField->Left() + (PlayingField->Width()-WidthOfSSRegion)/2.0 + SliceX/2.0;
  double right = PlayingField->Right() - (PlayingField->Width()-WidthOfSSRegion)/2.0 - SliceX/2.0;
  double top   = PlayingField->Top() + (PlayingField->Height()-HeightOfSSRegion)/2.0 + SliceY/2.0;

  for (int x=0; x<(numX/2)-1; ++x)
  {
    for (int y=0; y<numY; ++y)
    {      
      if (m_pTeam->Color() == SoccerTeam::blue)
      {
        m_Spots.push_back(SupportSpot(Vector2D(left+x*SliceX, top+y*SliceY), 0.0));
      }

      else
      {
        m_Spots.push_back(SupportSpot(Vector2D(right-x*SliceX, top+y*SliceY), 0.0));
      }
    }
  }
  
  //create the regulator
  m_pRegulator = new Regulator(Prm.SupportSpotUpdateFreq);
}


//--------------------------- DetermineBestSupportingPosition -----------------
//
//  see header or book for description
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 核心评分函数:先用限频器判断"还没到重算时间"就直接沿用上一次的最佳点;
// 否则遍历所有格子点:分数先清零(置 1.0 便于调试显示),再依次做三项测试加分,
// 全程记录最高分的点,最后返回它的位置。
//--------------------------------------------------------------------------------
Vector2D SupportSpotCalculator::DetermineBestSupportingPosition()
{
  //only update the spots every few frames                              
  if (!m_pRegulator->isReady() && m_pBestSupportingSpot)
  {
    return m_pBestSupportingSpot->m_vPos;
  }

  //reset the best supporting spot
  m_pBestSupportingSpot = NULL;
 
  double BestScoreSoFar = 0.0;

  std::vector<SupportSpot>::iterator curSpot;

  for (curSpot = m_Spots.begin(); curSpot != m_Spots.end(); ++curSpot)
  {
    //first remove any previous score. (the score is set to one so that
    //the viewer can see the positions of all the spots if he has the 
    //aids turned on)
//(原文注释:先清掉上一次的得分。分数置 1.0,这样开启辅助显示时能看到所有点的位置)
    curSpot->m_dScore = 1.0;

    //Test 1. is it possible to make a safe pass from the ball's position 
    //to this position?
//(原文注释:测试1:从控球者位置到这个点传球,是否对所有对方球员都安全?安全则加分)
    if(m_pTeam->isPassSafeFromAllOpponents(m_pTeam->ControllingPlayer()->Pos(),
                                           curSpot->m_vPos,
                                           NULL,
                                           Prm.MaxPassingForce))
    {
      curSpot->m_dScore += Prm.Spot_PassSafeScore;
    }
      
   
    //Test 2. Determine if a goal can be scored from this position.  
//(原文注释:测试2:从这个位置能否射门得分?能则加分)
    if( m_pTeam->CanShoot(curSpot->m_vPos,            
                          Prm.MaxShootingForce))
    {
      curSpot->m_dScore += Prm.Spot_CanScoreFromPositionScore;
    }   

    
    //Test 3. calculate how far this spot is away from the controlling
    //player. The further away, the higher the score. Any distances further
    //away than OptimalDistance pixels do not receive a score.
//(原文注释:测试3:算该点离控球者多远;越接近最佳距离 200 像素得分越高,
//           超过最佳距离则不得分。fabs=取绝对值。)
    if (m_pTeam->SupportingPlayer())
    {
      const double OptimalDistance = 200.0;
        
      double dist = Vec2DDistance(m_pTeam->ControllingPlayer()->Pos(),
                                 curSpot->m_vPos);
      
      double temp = fabs(OptimalDistance - dist);

      if (temp < OptimalDistance)
      {

        //normalize the distance and add it to the score
        curSpot->m_dScore += Prm.Spot_DistFromControllingPlayerScore *
                             (OptimalDistance-temp)/OptimalDistance;  
      }
    }
    
    //check to see if this spot has the highest score so far
    if (curSpot->m_dScore > BestScoreSoFar)
    {
      BestScoreSoFar = curSpot->m_dScore;

      m_pBestSupportingSpot = &(*curSpot);
    }    
    
  }

  return m_pBestSupportingSpot->m_vPos;
}





//------------------------------- GetBestSupportingSpot -----------------------
//-----------------------------------------------------------------------------
// GetBestSupportingSpot:若已有最佳点就直接返回;否则先算一次再返回(懒加载)。
Vector2D SupportSpotCalculator::GetBestSupportingSpot()
{
  if (m_pBestSupportingSpot)
  {
    return m_pBestSupportingSpot->m_vPos;
  }
    
  else
  { 
    return DetermineBestSupportingPosition();
  }
}

//----------------------------------- Render ----------------------------------
//-----------------------------------------------------------------------------
// Render:把各接应点画成空心灰圆(半径=得分),最佳点用绿色圆高亮(菜单开关控制)。
void SupportSpotCalculator::Render()const
{
    gdi->HollowBrush();
    gdi->GreyPen();

    for (unsigned int spt=0; spt<m_Spots.size(); ++spt)
    {
      gdi->Circle(m_Spots[spt].m_vPos, m_Spots[spt].m_dScore);
    }

    if (m_pBestSupportingSpot)
    {
      gdi->GreenPen();
      gdi->Circle(m_pBestSupportingSpot->m_vPos, m_pBestSupportingSpot->m_dScore);
    }
}