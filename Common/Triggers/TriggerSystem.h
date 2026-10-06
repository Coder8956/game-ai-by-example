//==============================================================================================
//【文件说明】TriggerSystem.h —— 触发器管理器(统一更新、统一检测、统一释放)
//
//【这个文件是干什么的?】
//  用 list<trigger_type*> 管理所有触发器。每帧 Update(entities) 做两件事:
//  ① UpdateTriggers:逐个 Update 触发器,把已标记删除的 delete+erase;
//    ② TryTriggers:对每个活着且就绪的实体,调每个触发器的 Try(entity)。
//  Register 注册新触发器,Render 统一画,Clear 统一清空。
//==============================================================================================
#ifndef TRIGGERSYSTEM_H
#define TRIGGERSYSTEM_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Name:    TriggerSystem.h
//
//  Author:  Mat Buckland (ai-junkie.com)
//
//  Desc:    Class to manage a collection of triggers. Triggers may be
//           registered with an instance of this class. The instance then 
//           takes care of updating those triggers and of removing them from
//           the system if their lifetime has expired.
//
//-----------------------------------------------------------------------------
template <class trigger_type>
//--------------------------------------------------------------------------------
// class TriggerSystem —— 触发器集合管理(模板,trigger_type 是具体触发器类)。
//--------------------------------------------------------------------------------
class TriggerSystem
{
public:

  typedef std::list<trigger_type*> TriggerList;

private:

  TriggerList   m_Triggers; 


  //this method iterates through all the triggers present in the system and
  //calls their Update method in order that their internal state can be
  //updated if necessary. It also removes any triggers from the system that
  //have their m_bRemoveFromGame field set to true.
  // UpdateTriggers:遍历 m_Triggers,已死的 delete+erase,活着的 Update。
  void UpdateTriggers()
  {
    TriggerList::iterator curTrg = m_Triggers.begin();
    while (curTrg != m_Triggers.end())
    {
      //remove trigger if dead
      if ((*curTrg)->isToBeRemoved())
      {
        delete *curTrg;

        curTrg = m_Triggers.erase(curTrg);
      }
      else
      {
        //update this trigger
        (*curTrg)->Update();

        ++curTrg;
      }
    }
  }

  //this method iterates through the container of entities passed as a
  //parameter and passes each one to the Try method of each trigger *provided*
  //the entity is alive and provided the entity is ready for a trigger update.
  template <class ContainerOfEntities>
  // TryTriggers:对每个实体,若它 alive 且 ready,就逐个触发器 Try 它。
  void TryTriggers(ContainerOfEntities& entities)
  {
    //test each entity against the triggers
    ContainerOfEntities::iterator curEnt = entities.begin();
    for (curEnt; curEnt != entities.end(); ++curEnt)
    {
      //an entity must be ready for its next trigger update and it must be 
      //alive before it is tested against each trigger.
      if ((*curEnt)->isReadyForTriggerUpdate() && (*curEnt)->isAlive())
      {
        TriggerList::const_iterator curTrg;
        for (curTrg = m_Triggers.begin(); curTrg != m_Triggers.end(); ++curTrg)
        {
          (*curTrg)->Try(*curEnt);
        }
      }
    }
  }
  

public:

  ~TriggerSystem()
  {
    Clear();
  }

  //this deletes any current triggers and empties the trigger list
  // Clear:逐个 delete 所有触发器再清空链表。
  void Clear()
  {
    TriggerList::iterator curTrg;
    for (curTrg = m_Triggers.begin(); curTrg != m_Triggers.end(); ++curTrg)
    {
      delete *curTrg;
    }

    m_Triggers.clear();
  }

  //This method should be called each update-step of the game. It will first
  //update the internal state odf the triggers and then try each entity against
  //each active trigger to test if any should be triggered.
  template <class ContainerOfEntities>
  void Update(ContainerOfEntities& entities)
  {
    UpdateTriggers();
    TryTriggers(entities);
  }

  //this is used to register triggers with the TriggerSystem (the TriggerSystem
  //will take care of tidying up memory used by a trigger)
  // Register:注册触发器;Render:统一画;GetTriggers:取链表常量引用。
  void Register(trigger_type* trigger)
  {
    m_Triggers.push_back(trigger);
  }

  //some triggers are required to be rendered (like giver-triggers for example)
  void Render()
  {
    TriggerList::iterator curTrg;
    for (curTrg = m_Triggers.begin(); curTrg != m_Triggers.end(); ++curTrg)
    {
      (*curTrg)->Render();
    }
  }

  const TriggerList& GetTriggers()const{return m_Triggers;}

};


#endif