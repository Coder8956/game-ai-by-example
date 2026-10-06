//==============================================================================================
//【文件说明】navigation\PathManager.h —— 「时间分片」路径搜索调度中心
//
//【这个文件是干什么的?】
//  A* 寻路如果一次算到底,机器人多时会卡帧。本类的思路:每帧只允许
//  算一小步(若干 search cycle),把计算摊到很多帧里,画面就不会顿。
//  PathManager 是个「调度员」:它手里排着一串正在寻路的 PathPlanner,
//  每帧 UpdateSearches() 把本帧的算力平均分给它们,每家算一小步;
//  谁先算完(找到路/找不到),就发消息通知对应的 bot,并从队列撤掉。
//
//【谁在使用这个文件?】
//  Raven_Game.h/.cpp —— 游戏类持有一个 PathManager,每帧调用 UpdateSearches();
//  navigation\Raven_PathPlanner.cpp —— 每个机器人的寻路器 Register 到这里排队。
//
//【本文件包含了谁?】
//  <list>     —— 标准库「双向链表」容器(m_SearchRequests 装排队的寻路器);
//  <cassert>  —— 断言宏(本文件其实没直接用,历史遗留);
//  它还用到 std::find(在 <algorithm> 里),靠间接包含进来。
//
//【C++ 小课堂:模板(template)是什么?】
//   template <class path_planner>  class PathManager { ... };
//   类里用到「另一个类型」时,不写死具体类型,而用一个占位名 path_planner,
//   前面 template<class 占位名> 声明。用的时候写 PathManager<Raven_PathPlanner>,
//   编译器就把占位名替换成 Raven_PathPlanner 造一份专用的类。
//   好处:同一份代码能给不同类型复用(泛型编程)。
//==============================================================================================
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786):关闭老 VC6 的 4786 号警告(详见 SoccerPitch.h 注释)。
//--------------------------------------------------------------------------------
#ifndef PATH_MANAGER_H
#define PATH_MANAGER_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   PathManager.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   a template class to manage a number of graph searches, and to 
//          distribute the calculation of each search over several update-steps
//(原文注释翻译:一个模板类,用来管理多个图搜索请求,并把每个搜索的计算量
//           分摊到多个更新帧里去跑——这就是「时间分片」寻路。)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 标准库头文件:<list>=双向链表容器;<cassert>=断言宏。尖括号 <> 表示系统目录。
//--------------------------------------------------------------------------------
#include <list>
#include <cassert>



//--------------------------------------------------------------------------------
// 模板声明 + 类定义。path_planner 是占位类型(见文件头小课堂)。
//--------------------------------------------------------------------------------
template <class path_planner>
class PathManager
{
private:

  //a container of all the active search requests
// m_SearchRequests:双向链表,每个元素是一个「寻路器指针」。
//   std::list<X> = 装 X 的链表;path_planner* = 指向寻路器的指针。
//(原文注释:存放所有「正在进行中」的搜索请求的容器)
  std::list<path_planner*>  m_SearchRequests;

  //this is the total number of search cycles allocated to the manager. 
  //Each update-step these are divided equally amongst all registered path
  //requests
// m_iNumSearchCyclesPerUpdate:每帧总共允许算多少步搜索(算力预算)。
//   unsigned int = 无符号整数(不为负)。
//(原文注释翻译:这是分配给调度中心的总搜索步数;每个更新帧,这些步数被平均分给
//           所有已注册的寻路请求。)
  unsigned int              m_iNumSearchCyclesPerUpdate;

public:
    
//--------------------------------------------------------------------------------
// 构造函数:创建调度中心时,告诉它「每帧给多少搜索步」。
//--------------------------------------------------------------------------------
  PathManager(unsigned int NumCyclesPerUpdate):m_iNumSearchCyclesPerUpdate(NumCyclesPerUpdate){}

  //every time this is called the total amount of search cycles available will
  //be shared out equally between all the active path requests. If a search
  //completes successfully or fails the method will notify the relevant bot
// UpdateSearches():每帧调用一次,驱动所有排队的寻路请求各算一小步。
//(原文注释翻译:每次调用 UpdateSearches,本帧可用的总步数会被所有活跃寻路请求
//           平分。某个搜索成功或失败时,本方法会通知对应的机器人。)
  void UpdateSearches();

  //a path planner should call this method to register a search with the 
  //manager. (The method checks to ensure the path planner is only registered
  //once)
// Register(寻路器指针):把一个寻路请求加入调度队列。
//(原文注释翻译:寻路器要调用本方法来把自己注册进调度中心;
//           本方法会检查,确保同一个寻路器不会被重复注册。)
  void Register(path_planner* pPathPlanner);

// UnRegister(寻路器指针):把一个寻路请求从调度队列撤掉。
  void UnRegister(path_planner* pPathPlanner);

  //returns the amount of path requests currently active.
// GetNumActiveSearches():当前排队的寻路请求数(只读)。
//(原文注释:返回当前活跃的寻路请求数量)
  int  GetNumActiveSearches()const{return m_SearchRequests.size();}
};

///////////////////////////////////////////////////////////////////////////////
//------------------------- UpdateSearches ------------------------------------
//
//  This method iterates through all the active path planning requests 
//  updating their searches until the user specified total number of search
//  cycles has been satisfied.
//
//  If a path is found or the search is unsuccessful the relevant agent is
//  notified accordingly by Telegram
// ↓↓↓ 上面这段 UpdateSearches 英文注释的翻译:
//   本方法遍历所有活跃的寻路请求,持续推进它们的搜索,直到用完用户
//   指定的总搜索步数。若找到路径或搜索失败,就通过 Telegram(电报/消息)
//   相应地通知相关的智能体(bot)。
//-----------------------------------------------------------------------------
template <class path_planner>
//--------------------------------------------------------------------------------
// UpdateSearches 的实现(模板成员函数,必须和类声明写在同一个头文件里,
// 否则编译器实例化时找不到代码)。inline 表示内联。
//--------------------------------------------------------------------------------
inline void PathManager<path_planner>::UpdateSearches()
{
// 本帧还剩多少搜索步可用(一开始 = 每帧预算,边算边减)。
  int NumCyclesRemaining = m_iNumSearchCyclesPerUpdate;

  //iterate through the search requests until either all requests have been
  //fulfilled or there are no search cycles remaining for this update-step.
// 迭代器:可以理解成「链表当前位置的书签」。begin()=指向第一个元素。
//   std::list<...>::iterator = 这种链表专用的书签类型。
//(原文注释翻译:遍历所有搜索请求,直到全部完成,或者本帧的搜索步数用完。)
  std::list<path_planner*>::iterator curPath = m_SearchRequests.begin();
// while 循环:条件是「还剩步数」且「队列非空」。
//   NumCyclesRemaining-- :先用后减(每跑一次,剩余步数 -1);
//   !m_SearchRequests.empty():队列非空(!=逻辑非,取反)。
  while (NumCyclesRemaining-- && !m_SearchRequests.empty())
  {
    //make one search cycle of this path request
// *curPath:取出书签指向的那个寻路器指针;
// ->CycleOnce():让它算一步,返回结果(target_found 找到 / target_not_found 没找到)。
//(原文注释:让这个寻路请求往前算一个搜索步)
    int result = (*curPath)->CycleOnce();

    //if the search has terminated remove from the list
//(原文注释:如果这个搜索结束了,就把它从队列里移除)
    if ( (result == target_found) || (result == target_not_found) )
    {
      //remove this path from the path list
// erase:把当前节点从链表删掉,并返回到下一个节点的书签(赋给 curPath)。
      curPath = m_SearchRequests.erase(curPath);       
    }
    //move on to the next
//(原文注释:移到下一个)
    else
    {
// ++curPath:书签后移一位,指向下一个寻路请求。
      ++curPath;
    }

    //the iterator may now be pointing to the end of the list. If this is so,
    // it must be reset to the beginning.
// end() 不是最后一个元素,而是「末尾之后的位置」。走到这就说明一圈跑完了,
// 只要队列还没清空,就绕回开头继续轮流给大家分算力。
//(原文注释翻译:书签现在可能已经指向链表末尾;如果是这样,必须绕回开头。)
    if (curPath == m_SearchRequests.end())
    {
      curPath = m_SearchRequests.begin();
    }

  }//end while
}

//--------------------------- Register ----------------------------------------
//
//  this is called to register a search with the manager.
//(原文注释:调用本方法来把一个搜索请求注册进调度中心)
//-----------------------------------------------------------------------------
template <class path_planner>
//--------------------------------------------------------------------------------
// Register 的实现:先查这个寻路器是否已在队列里;不在才加进去(防重复)。
//--------------------------------------------------------------------------------
inline void PathManager<path_planner>::Register(path_planner* pPathPlanner)
{
  //make sure the bot does not already have a current search in the queue
// std::find(起点, 终点, 要找的东西):在链表范围内线性查找;
// 若等于 end() 说明没找到 → 该寻路器还没注册过 → 可以加进去。
//(原文注释:确保这个 bot 不会在队列里同时有两个搜索请求)
  if(std::find(m_SearchRequests.begin(),
               m_SearchRequests.end(),
               pPathPlanner) == m_SearchRequests.end())
  { 
    //add to the list
// push_back:把新寻路器追加到链表末尾。
    m_SearchRequests.push_back(pPathPlanner);
  }
}

//----------------------------- UnRegister ------------------------------------
//-----------------------------------------------------------------------------
template <class path_planner>
//--------------------------------------------------------------------------------
// UnRegister 的实现:把指定寻路器从链表中删掉。
//--------------------------------------------------------------------------------
inline void PathManager<path_planner>::UnRegister(path_planner* pPathPlanner)
{
// remove(x):从链表中删掉所有等于 x 的元素。
  m_SearchRequests.remove(pPathPlanner);

}





#endif