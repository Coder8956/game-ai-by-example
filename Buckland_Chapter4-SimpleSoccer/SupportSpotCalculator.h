//==============================================================================================
//【文件说明】SupportSpotCalculator.h —— "接应甜区"评分计算器
//
//【这个文件是干什么的?】
//  进攻时,除了控球者,另一名队友该跑到哪接应最好?本类把半场划成一张格子网,
//  每个格子点都打一个分:传球是否安全、能否直接射门、离控球者距离是否合适……
//  分最高的点就是"最佳接应点",由 SupportSpotCalculator.cpp 负责逐帧(限频)算分。
//
//【谁在使用这个文件?】
//  SoccerTeam.h/.cpp —— 每支球队拥有一个本类对象,用它的 GetBestSupportingSpot() 拿接应点;
//  SupportSpotCalculator.cpp —— 本类实现;FieldPlayerStates.cpp 据此把球员派去支援。
//
//【本文件包含了谁?】
//  <vector>、Game/Region.h、2D/Vector2D.h、misc/Cgdi.h(绘图单例 gdi)。
//
//【C++ 小课堂:嵌套结构体 struct SupportSpot】
// 在类内部再定义一个小 struct(结构体,与 class 几乎一样,只是默认 public),
// 用来"打包"一个接应点的数据:位置 m_vPos + 得分 m_dScore,并带一个小构造函数。
//==============================================================================================
//--------------------------------------------------------------------------------
// 包含保护 + #pragma warning(disable:4786):原理见 Goal.h 与 SoccerPitch.h。
//--------------------------------------------------------------------------------
#ifndef SUPPORTSPOTCALCULATOR
#define SUPPORTSPOTCALCULATOR
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   SupportSpotCalculator.h
//
//  Desc:   Class to determine the best spots for a suppoting soccer
//          player to move to.
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//------------------------------------------------------------------------
// ↓↓↓ 原作者说明的翻译:
//   文件名:SupportSpotCalculator.h
//   描述  :计算接应球员应移动到哪些最佳位置的类。
//   作者  :Mat Buckland,2003 年(本书作者)
//
//------------------------------------------------------------------------

#include <vector>

#include "Game/Region.h"
#include "2D/Vector2D.h"
#include "misc/Cgdi.h"


class PlayerBase;
class Goal;
class SoccerBall;
class SoccerTeam;
class Regulator;



//------------------------------------------------------------------------

class SupportSpotCalculator
{
private:
  
  //a data structure to hold the values and positions of each spot
// 嵌套结构体:一个接应点 = 位置 + 得分(见文件头小课堂)。
// 构造函数 SupportSpot(pos,value):用初始化列表把位置和得分存好。
  struct SupportSpot
  {
    
    Vector2D  m_vPos;

    double    m_dScore;

    SupportSpot(Vector2D pos, double value):m_vPos(pos),
                                            m_dScore(value)
    {}
  };

private:


  SoccerTeam*               m_pTeam;

  std::vector<SupportSpot>  m_Spots;

  //a pointer to the highest valued spot from the last update
//(原文注释:指向上次更新时得分最高的那个接应点)
  SupportSpot*              m_pBestSupportingSpot;

  //this will regulate how often the spots are calculated (default is
  //one update per second)
//(原文注释:它用来限制多久才算一次接应点(默认每秒更新一次))
// Regulator(Common\time\regulator.h)是"限频器":isReady() 控制别每帧都重算,省算力。
  Regulator*                m_pRegulator;

public:
  
  SupportSpotCalculator(int numX,
                        int numY,
                        SoccerTeam* team);

  ~SupportSpotCalculator();

  //draws the spots to the screen as a hollow circles. The higher the 
  //score, the bigger the circle. The best supporting spot is drawn in
  //bright green.
//(原文注释:把各接应点画成空心圆,分越高圆越大;最佳接应点画成亮绿色)
  void       Render()const;

  //this method iterates through each possible spot and calculates its
  //score.
//(原文注释:遍历每个候选点并计算它的得分)
  Vector2D  DetermineBestSupportingPosition();

  //returns the best supporting spot if there is one. If one hasn't been
  //calculated yet, this method calls DetermineBestSupportingPosition and
  //returns the result.
//(原文注释:返回最佳接应点;若还没算过,就先调 DetermineBestSupportingPosition 再返回)
  Vector2D  GetBestSupportingSpot();
};


#endif