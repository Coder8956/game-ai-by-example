//==============================================================================================
//【文件说明】BaseGameEntity.cpp —— 实体基类的实现(静态成员定义、构造、SetID)
//
//【本文件是干什么的?】
//  实现基类的三件事:
//    ① 给静态成员 m_iNextValidID 分配全局内存并初值 0;
//    ② 构造函数:初始化半径/缩放/类型/标记,并调 SetID 分配编号;
//    ③ SetID:校验传入编号 >= 当前计数器,然后写入并把计数器 +1。
//
//【为什么静态成员要在 .cpp 里定义?】
//  static 成员"属于类",不属于任何对象,必须在某个 .cpp 里出现一次实体定义,
//  否则链接器报"未定义的外部符号"。本文件第 4 行就是这个定义。
//==============================================================================================
#include "BaseGameEntity.h"


  // 静态成员的全局定义:全程序共享这一个变量,初值 0。
int BaseGameEntity::m_iNextValidID = 0;

//------------------------------ ctor -----------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 构造函数:用初始化列表把半径设 0、缩放设 (1,1)、类型设默认值、标记设 false,
// 再调 SetID(ID) 把 ID 分配好。
//--------------------------------------------------------------------------------
BaseGameEntity::BaseGameEntity(int ID):m_dBoundingRadius(0.0),
                                       m_vScale(Vector2D(1.0,1.0)),
                                       m_iType(default_entity_type),
                                       m_bTag(false)
{
  SetID(ID);
}

//----------------------------- SetID -----------------------------------------
//
//  this must be called within each constructor to make sure the ID is set
//  correctly. It verifies that the value passed to the method is greater
//  or equal to the next valid ID, before setting the ID and incrementing
//  the next valid ID
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// SetID:分配并校验 ID。assert 保证 val >= 下一个可用编号(防止重复编号);
// 然后把 m_ID 设为 val,并把静态计数器 m_iNextValidID 推进到 val+1。
//--------------------------------------------------------------------------------
void BaseGameEntity::SetID(int val)
{
  //make sure the val is equal to or greater than the next available ID
  assert ( (val >= m_iNextValidID) && "<BaseGameEntity::SetID>: invalid ID");

  m_ID = val;
    
  m_iNextValidID = m_ID + 1;
}
