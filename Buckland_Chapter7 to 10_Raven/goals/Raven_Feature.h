//==============================================================================================
//【文件说明】Raven_Feature.h —— "特征提取器":把游戏世界状况折算成 0~1 的分数
//
//【这个文件是干什么的?】
//  评估器(Goal_Evaluator 子类)给目标打分时,需要一些"客观数据":机器人现在多血?
//  离血包/武器多远?弹药够不够?本文件的 Raven_Feature 类就负责回答这些问题,
//  并把各种情况统一压缩成 0~1 的小数(0=很差/很远,1=很好/很近),方便评估器加权。
//  它的函数全是 static(静态函数)——不需要造对象,直接用 Raven_Feature::Health(机器人) 调用。
//
//【谁在使用这个文件?】
//  ExploreGoal_Evaluator.cpp、GetHealthGoal_Evaluator.cpp、GetWeaponGoal_Evaluator.cpp、
//  AttackTargetGoal_Evaluator.cpp —— 这些评估器都调用 Health/DistanceToItem/WeaponStrength。
//
//【本文件包含了谁?】—— 无 #include,只有前置声明 class Raven_Bot;(详见 Goal_Evaluator.h)。
//==============================================================================================
#ifndef RAVEN_FEATURE_H
#define RAVEN_FEATURE_H
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Feature.h
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:   class that implements methods to extract feature specific
//          information from the Raven game world and present it as 
//          a value in the range 0 to 1
//
//-----------------------------------------------------------------------------
// 前置声明:Raven_Bot(机器人类)在别处定义,这里只用它的指针,先报个名字即可。
class Raven_Bot;

//--------------------------------------------------------------------------------
// class Raven_Feature:特征提取类。所有函数都是 static——属于"类"而不是"对象",
// 直接 类名::函数() 调用,不用 new。
class Raven_Feature
{
public:

  //returns a value between 0 and 1 based on the bot's health. The better
  //the health, the higher the rating
// Health:把血量折算成 0~1(满血=1,空血=0)。
//(原文注释:根据机器人血量返回 0~1 的值;血越多,分数越高)
  static double Health(Raven_Bot* pBot);
  
  //returns a value between 0 and 1 based on the bot's closeness to the 
  //given item. the further the item, the higher the rating. If there is no
  //item of the given type present in the game world at the time this method
  //is called the value returned is 1
// DistanceToItem:离某类物品(血包/武器)的距离折算分。ItemType=物品编号。
//(原文注释:根据机器人离指定物品的远近返回 0~1;离得越远分数越高。
//           若游戏世界里根本没有该类物品,则返回 1)
  static double DistanceToItem(Raven_Bot* pBot, int ItemType);
  
  //returns a value between 0 and 1 based on how much ammo the bot has for
  //the given weapon, and the maximum amount of ammo the bot can carry. The
  //closer the amount carried is to the max amount, the higher the score
// IndividualWeaponStrength:单种武器的弹药充足度(0~1)。
//(原文注释:根据机器人某武器的现有弹药与最大携带量,返回 0~1;
//           现有弹药越接近上限,分数越高)
  static double IndividualWeaponStrength(Raven_Bot* pBot, int WeaponType);

  //returns a value between 0 and 1 based on the total amount of ammo the
  //bot is carrying each of the weapons. Each of the three weapons a bot can
  //pick up can contribute a third to the score. In other words, if a bot
  //is carrying a RL and a RG and has max ammo for the RG but only half max
  //for the RL the rating will be 1/3 + 1/6 + 0 = 0.5
// TotalWeaponStrength:全部武器的总火力强度(0~1)。
//(原文注释:把机器人携带的三种武器弹药各占三分之一,合计成 0~1 总分。
//           例如拿着火箭筒和轨道炮,轨道炮满弹、火箭筒半弹,则得 1/3+1/6+0=0.5)
  static double TotalWeaponStrength(Raven_Bot* pBot);
};



#endif