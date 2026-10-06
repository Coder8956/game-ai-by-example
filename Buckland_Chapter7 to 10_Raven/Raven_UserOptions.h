//==============================================================================================
//【文件说明】Raven_UserOptions.h —— 调试菜单选项(单例)
//
//【这个文件是干什么的?】
//  全局单例,存一堆 bool:是否显示导航图/节点编号/路径/目标/感知敌人/
//  只显示视野内 bot/目标栈/目标打分/武器打分/路径平滑/ID/血/分。
//  菜单勾选改这些 bool,Render 时按它决定画不画。
//  下面 #define UserOptions 是个宏,全工程直接 UserOptions->m_bShowGraph 用。
#ifndef USER_OPTIONS
#define USER_OPTIONS
//-----------------------------------------------------------------------------
//
//  Name:   Raven_UserOptions.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   singleton class to control a number of menu options
//(原文注释翻译:控制若干菜单选项的单例类)
//-----------------------------------------------------------------------------


#define UserOptions Raven_UserOptions::Instance()

//--------------------------------------------------------------------------------
// class Raven_UserOptions —— 单例。私有构造/拷贝,外面只能 Instance() 拿。
class Raven_UserOptions
{
private:

  Raven_UserOptions();

  //copy ctor and assignment should be private
  Raven_UserOptions(const Raven_UserOptions&);
  Raven_UserOptions& operator=(const Raven_UserOptions&);


public:

  static Raven_UserOptions* Instance();
  
// 下面一串 bool:各种调试显示开关。
  bool m_bShowGraph;

  bool m_bShowNodeIndices;

  bool m_bShowPathOfSelectedBot;

  bool m_bShowTargetOfSelectedBot;

  bool m_bShowOpponentsSensedBySelectedBot;

  bool m_bOnlyShowBotsInTargetsFOV;

  bool m_bShowGoalsOfSelectedBot;

  bool m_bShowGoalAppraisals;

  bool m_bShowWeaponAppraisals;

  bool m_bSmoothPathsQuick;
  bool m_bSmoothPathsPrecise;

  bool m_bShowBotIDs;

  bool m_bShowBotHealth;

  bool m_bShowScore;
};


#endif