//==============================================================================================
//【文件说明】Goal_Evaluator.h —— 目标"评估器"的基类(一张抽象图纸)
//
//【这个文件是干什么的?】
//  Raven 机器人(智能体)每过一段时间就要做一次决策:"现在最该干什么?"
//  候选目标有很多:去吃血包、去捡枪、去追敌人、去探索……谁来给每个候选目标打分?
//  答案就是本文件定义的 Goal_Evaluator(评估器):它是一个"抽象基类"——
//  本身不干活,只规定"任何评估器都必须会做这三件事":
//    1) CalculateDesirability —— 给某个候选目标打一个 0~1 的"渴望分";
//    2) SetGoal               —— 一旦这个目标被选中,就把它装进机器人的决策大脑;
//    3) RenderInfo            —— 在屏幕上画出该评估器的调试信息(调参数用)。
//  具体的打分规则由它的子类实现(见同目录:ExploreGoal_Evaluator、
//  GetHealthGoal_Evaluator、GetWeaponGoal_Evaluator、AttackTargetGoal_Evaluator)。
//
//【谁在使用这个文件?】
//  Goal_Think.h/.cpp          —— 决策大脑,内部持有一串 Goal_Evaluator* 评估器;
//  ExploreGoal_Evaluator.h     GetHealthGoal_Evaluator.h
//  GetWeaponGoal_Evaluator.h   AttackTargetGoal_Evaluator.h
//  (上面这 4 个头文件都以 public Goal_Evaluator 的方式继承本基类)。
//
//【本文件包含了谁?】—— 本文件一个 #include 都没有,而是用"前置声明"先打招呼:
//   class Raven_Bot;    —— 告诉编译器:"Raven_Bot 是个类,具体定义在别处";
//   struct Vector2D;    —— Vector2D 是个结构体(2D 向量)。
//  为什么只声明不包含?因为本文件只用它们的"指针",不碰它们的内部细节,
//  前置声明能加快编译速度(这是 C++ 头文件常见的优化手法)。
//
//【C++ 小课堂:抽象基类 / 纯虚函数 / 接口】
//   virtual 返回值 函数名(参数) = 0;   末尾的 =0 叫做"纯虚函数":
//   它只声明"签名",不写函数体,等于说"我这个基类不知道具体怎么干,
//   你们子类必须自己实现它"。一个类只要含纯虚函数,它就是"抽象类",
//   不能被直接 new 出来,只能当"接口/规矩"让别人继承。
//   这正是本文件的用途:定规矩——所有评估器都得实现这三个函数。
//==============================================================================================
#ifndef GOAL_EVALUATOR_H
#define GOAL_EVALUATOR_H
//--------------------------------------------------------------------------------
// #pragma:给编译器的"临时指令"(不参与程序逻辑)。
// warning(disable:4786) = 关闭编号 4786 的警告(VS 对"调试信息里名字过长"的
// 历史老警告)。这是 VC6/VS2008 时代的写法,纯粹为了让老工程编译干净。
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 包含保护 #ifndef/#define/#endif:C++ 头文件标准开场白,防止本文件被重复包含
// 导致"重复定义"错误(详解见 SimpleSoccer\Goal.h 中同名注释)。
//--------------------------------------------------------------------------------
#pragma warning (disable : 4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_Evaluator.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class template that defines an interface for objects that are
//          able to evaluate the desirability of a specific strategy level goal
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 前置声明(forward declaration):下面两行只是先报个名字。
//   Raven_Bot = 本游戏的机器人玩家类;Vector2D = 2D 向量类。
class Raven_Bot;
struct Vector2D;


//--------------------------------------------------------------------------------
// class Goal_Evaluator:抽象基类(评估器的"图纸规矩")。
//   protected:下面的成员只有"本类自己"和"它的子类"能访问(外界碰不到)。
//--------------------------------------------------------------------------------
class Goal_Evaluator
{
protected:

  //when the desirability score for a goal has been evaluated it is multiplied 
  //by this value. It can be used to create bots with preferences based upon
//(原文注释:评估出某个目标的"渴望分"后,会乘上这个系数;
//           可以用它给不同机器人设定不同的性格偏好)
  //their personality
// m_dCharacterBias:性格偏置系数。m_=成员变量,d=double 浮点型。
// 打分结果最终会乘上它:调大它 = 这个机器人"更偏爱"对应的目标。
  double       m_dCharacterBias;

// public:下面是对外公开的接口(谁都能调用)。
public:

// 构造函数:创建评估器时,把性格偏置 CharacterBias 存进 m_dCharacterBias。
// 冒号后是初始化列表(成员名(初值));花括号 { } 内为空表示不再做别的。
  Goal_Evaluator(double CharacterBias):m_dCharacterBias(CharacterBias){}
  
// 虚析构函数:~ 表示析构(对象销毁时自动调用)。
// 只要一个类会被"当基类"继承,它的析构函数就必须加 virtual——
// 否则用基类指针 delete 子类对象时,只会调用基类析构,造成子类资源泄漏。
  virtual ~Goal_Evaluator(){}
  
  //returns a score between 0 and 1 representing the desirability of the
  //strategy the concrete subclass represents
// 纯虚函数①:给目标打 0~1 的渴望分。pBot=指向要打分的机器人。=0 表示由子类实现。
//(原文注释:返回一个 0~1 的分数,表示"具体子类所代表的那个策略目标"有多诱人)
  virtual double CalculateDesirability(Raven_Bot* pBot)=0;
  
  //adds the appropriate goal to the given bot's brain
// 纯虚函数②:一旦本目标被选中,调用它把目标塞进机器人脑子里。
//(原文注释:把对应的目标"装进"指定机器人的决策大脑里)
  virtual void  SetGoal(Raven_Bot* pBot) = 0;

  //used to provide debugging/tweaking support
// 纯虚函数③:在 Position 位置画出该评估器的调试信息(调参时看"每个目标打了多少分")。
//(原文注释:用于调试/调参时在屏幕上画出该评估器的信息)
  virtual void  RenderInfo(Vector2D Position, Raven_Bot* pBot) = 0;
};




#endif

