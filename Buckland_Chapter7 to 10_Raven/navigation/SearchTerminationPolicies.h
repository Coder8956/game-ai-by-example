//==============================================================================================
//【文件说明】navigation\SearchTerminationPolicies.h —— 图搜索「什么时候算完」的判断策略
//
//【这个文件是干什么的?】
//  Dijkstra / A* 这类图搜索,每往外探一个节点都要问一句:「找到目标了吗?
//  可以停了吗?」。这个「停不停」的判断被抽成一个小策略类,写在本文件里。
//  目前给了两种策略:
//   ① FindNodeIndex    —— 当前探到的节点编号 == 目标编号 → 停(普通寻路);
//   ② FindActiveTrigger —— 当前节点上挂的触发器(如血包/枪)正好是我要的、
//                          且它现在可用 → 停(寻路去找补给品)。
//
//【谁在使用这个文件?】
//  navigation\TimeSlicedGraphAlgorithms.h —— A*/Dijkstra 算法模板里把策略类当参数;
//  navigation\Raven_PathPlanner.cpp —— 实际寻路时选用上面某一种策略。
//
//【本文件包含了谁?】
//  什么都没 #include。它用到的 graph_type、trigger_type 都是模板占位类型,
//  真正的图类/触发器类由使用方在实例化时传入。
//
//【C++ 小课堂:策略模式(Policy)与 static 成员函数】
//   把「一个会变的小判断」做成一个类,里面只放一个 static bool isSatisfied(...)。
//   static 成员函数:不属于任何对象,直接用 类名::函数名() 调用,没有 this 指针。
//   搜索算法模板写死「我会调用 策略类::isSatisfied(...)」,至于具体怎么判断,
//   由换哪个策略类决定——这就是策略模式:算法骨架不变,判断条件可插拔。
//==============================================================================================
//--------------------------------------------------------------------------------
// 原作者文件头注释(Name/Author/Desc),原样保留。
//--------------------------------------------------------------------------------
#ifndef TERMINATION_POLICIES_H
#define TERMINATION_POLICIES_H
//-----------------------------------------------------------------------------
//
//  Name:   SearchTerminationPolicies.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class templates to define termination policies for Dijkstra's
//          algorithm
//(原文注释翻译:为 Dijkstra 算法定义「终止策略」的类模板)
//-----------------------------------------------------------------------------



//--------------------------- FindNodeIndex -----------------------------------

//the search will terminate when the currently examined graph node
//is the same as the target node.
//--------------------------------------------------------------------------------
// 策略①:FindNodeIndex —— 探到指定编号节点就停。
// 它上面两行英文注释的翻译:当当前正在检查的图节点 == 目标节点时,搜索终止。
//--------------------------------------------------------------------------------
class FindNodeIndex
{
public:

// 嵌套模板:类里又套了一个模板参数 graph_type(图的类型)。
//   static bool isSatisfied(...) :静态函数,直接 FindNodeIndex::isSatisfied() 调用。
  template <class graph_type>
  static bool isSatisfied(const graph_type& G, int target, int CurrentNodeIdx)
  {
// == :等于判断。当前节点编号 == 目标编号 → 找到,返回 true。
    return CurrentNodeIdx == target;
  }
};

//--------------------------- FindActiveTrigger ------------------------------

//the search will terminate when the currently examined graph node
//is the same as the target node.
template <class trigger_type>
//--------------------------------------------------------------------------------
// 策略②:FindActiveTrigger<触发器类型> —— 探到一个「激活的、类型对的」触发器就停。
// 它上面两行英文注释与策略①相同(原作者照抄),实际含义:当当前节点上挂的
// 触发器正好是我要找的、且它现在可用时,搜索就停(比如我要找血包就一路探到血包)。
//--------------------------------------------------------------------------------
class FindActiveTrigger
{
public:

  template <class graph_type>
  static bool isSatisfied(const graph_type& G, int target, int CurrentNodeIdx)
  {
// 先假设「没满足」,下面条件凑齐再翻成 true。
    bool bSatisfied = false;

    //get a reference to the node at the given node index
// G.GetNode(编号):从图里取出那个节点;
//   const ... & :常引用(只读借用,不拷贝);NodeType 是图类里定义的节点类型。
//(原文注释:取出指定编号处的节点的引用)
    const graph_type::NodeType& node = G.GetNode(CurrentNodeIdx);

    //if the extrainfo field is pointing to a giver-trigger, test to make sure 
    //it is active and that it is of the correct type.
// 三个条件用 &&(逻辑与,都为真才算真)连成一串:
//   ① ExtraInfo() != NULL     :节点上挂了触发器(NULL=空指针,没挂);
//   ② ->isActive()            :这个触发器现在是激活的;
//   ③ ->EntityType() == target:它的类型正是我要找的目标编号。
//(原文注释翻译:如果节点的 extrainfo 字段指向一个「给予型触发器」,
//           就检查它是否处于激活状态,且类型正是我要找的那一种。)
    if ((node.ExtraInfo() != NULL) && 
         node.ExtraInfo()->isActive() && 
        (node.ExtraInfo()->EntityType() == target) )
    {    
// 三条件全中 → 标记「满足」。
      bSatisfied = true;
    }

// 返回判断结果给搜索算法:要不要停。
    return bSatisfied;
  }
};


  
#endif