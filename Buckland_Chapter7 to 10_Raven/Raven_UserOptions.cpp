//==============================================================================================
//【文件说明】Raven_UserOptions.cpp —— 用户选项单例的实现
//==============================================================================================
#include "Raven_UserOptions.h"

 
// Instance: Meyers 单例——函数内 static,第一次调用时构造,之后都返回它。
Raven_UserOptions* Raven_UserOptions::Instance()
{
  static Raven_UserOptions instance; 
  return &instance;
}

// 构造函数:给每个 bool 设默认值。
Raven_UserOptions::Raven_UserOptions():m_bShowGraph(false),
                      m_bShowPathOfSelectedBot(true),
                      m_bSmoothPathsQuick(false),
                      m_bSmoothPathsPrecise(false),
                      m_bShowBotIDs(false),
                      m_bShowBotHealth(true),
                      m_bShowTargetOfSelectedBot(false),
                      m_bOnlyShowBotsInTargetsFOV(false),
                      m_bShowScore(false),
                      m_bShowGoalsOfSelectedBot(true),
                      m_bShowGoalAppraisals(true),
                      m_bShowNodeIndices(false),
                      m_bShowOpponentsSensedBySelectedBot(true),
                      m_bShowWeaponAppraisals(false)
{}
