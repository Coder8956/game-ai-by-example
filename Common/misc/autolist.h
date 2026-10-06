//==============================================================================================
//【文件说明】autolist.h —— 自动登记表:继承它的类,所有实例自动进/出全局链表
//
//【这个文件是干什么的?】
//  想"遍历游戏里所有玩家/敌人"时,通常要自己维护一个 vector 去 push/erase。
//  AutoList 用 C++ 构造/析构自动完成这件事:谁继承它,谁 new 出来就自动入链表,
//  delete 就自动出链表。调用者用 GetAllMembers() 拿到全局链表遍历即可。
//
//【谁在使用这个文件?】
//  Raven(第 7~10 章)中的 Raven_Bot、Raven_Object 等继承它,实现"场上所有机器人"遍历。
//
//【本文件包含了谁?】
//  <list> —— 标准库链表(静态成员 m_Members 用它存所有实例)。
//==============================================================================================
#ifndef AUTOLIST_H
#define AUTOLIST_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------

//------------------------------------------------------------------------
//
//Name:   Autolist.h
//
//Desc:   Inherit from this class to automatically create lists of
//        similar objects. Whenever an object is created it will
//        automatically be added to the list. Whenever it is destroyed
//        it will automatically be removed.
//
//Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <list>


template <class T>
//--------------------------------------------------------------------------------
// 上面 template <class T> 中,T 就是继承 AutoList 的子类自己(CRTP 惯用法)。
// class AutoList —— 自动登记表模板。
//   typedef std::list<T*> ObjectList; —— 给"T 的指针链表"起短名。
//   static ObjectList m_Members;      —— 静态成员:全类共享这一个链表。
//--------------------------------------------------------------------------------
class AutoList
{
public:

  typedef std::list<T*> ObjectList;
  
private:

  static ObjectList m_Members;

protected:

  // 构造函数:把 this(转成 T* 类型)push_back 进全局链表;析构函数 ~AutoList:
  // 自动把自己从链表 remove。static_cast<T*>(this) 是向下转型。
  AutoList()
  {
    //cast this object to type T* and add it to the list
    m_Members.push_back(static_cast<T*>(this));
  }

  ~AutoList()
  {
    m_Members.remove(static_cast<T*>(this));    
  }

public:


  // GetAllMembers:返回全局链表的引用,让调用者遍历所有活实例。
  static ObjectList& GetAllMembers(){return m_Members;}
};


template <class T>
std::list<T*> AutoList<T>::m_Members;



#endif