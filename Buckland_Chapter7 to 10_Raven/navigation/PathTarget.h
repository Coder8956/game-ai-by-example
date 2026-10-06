//==============================================================================================
//【文件说明】navigation\PathTarget.h —— 机器人「下一步要去哪」的目标描述
//
//【这个文件是干什么的?】
//  机器人决定要去的地方,可能是两类:① 某个物品(血包/枪,按物体类型编号);
//  ② 地图上某个坐标点。本类 PathTarget 把这两种目标打包成一个对象:
//  它内部记着「我现在指的到底是物品还是坐标」,并提供一组 isXxx() 查询。
//  (注:在当前工程里,这个文件没有被其它文件 #include,属于早期设计遗留;
//   目标系统后来改放在 Raven_TargetingSystem 里实现了。)
//
//【本文件包含了谁?】
//  自己没有 #include 任何头文件。它用到了 Vector2D(坐标),靠包含它的
//  地方(若有)提前把 vector2D.h 包含进来才能编译。
//
//【C++ 小课堂:类内部嵌套 enum(枚举)】
//   enum target_type {item, position, invalid};
//   写在类内部的枚举,名字 target_type 属于这个类的命名空间,
//   外面要用得写成 PathTarget::item。三个值分别表示:
//   item=目标是个物品,position=目标是个坐标点,invalid=目标无效(还没设定)。
//==============================================================================================
#ifndef PATH_TARGET_H
#define PATH_TARGET_H


//--------------------------------------------------------------------------------
// class PathTarget —— 路径目标类。
// public 区先放嵌套枚举,再放对外操作;private 区是内部数据。
//--------------------------------------------------------------------------------
class PathTarget
{
public:
  
  enum target_type {item, position, invalid};
// 嵌套枚举:目标类型。item=物品,position=坐标点,invalid=无效(见文件头小课堂)。
  
private:

  int         m_iTargetItemType;

  Vector2D    m_vTargetPosition;

  target_type m_Type;
//--------------------------------------------------------------------------------
// 上面 3 行是私有成员(外部碰不到,只能通过 public 函数访问):
//   m_iTargetItemType = 目标物品类型编号(若目标是物品);
//   m_vTargetPosition = 目标坐标(若目标是坐标点);
//   m_Type            = 当前到底是哪种目标(上面枚举里的某个值)。
//--------------------------------------------------------------------------------

public:

//--------------------------------------------------------------------------------
// 默认构造函数:新建一个 PathTarget 时,物品编号=-1(无效),类型=invalid。
//   PathTarget():  —— 函数名与类名相同、无返回类型,这就是构造函数;
//   m_iTargetItemType(-1), m_Type(invalid) —— 初始化列表,直接给成员初值。
//--------------------------------------------------------------------------------
  PathTarget():m_iTargetItemType(-1), m_Type(invalid){}
  
//--------------------------------------------------------------------------------
// 两个「设目标」函数(声明,实现写在别处):
//   SetTargetAsItem(物品类型)  —— 把目标设成某类物品;
//   SetTargetAsPosition(坐标)   —— 把目标设成某个坐标点。
//--------------------------------------------------------------------------------
  void SetTargetAsItem(int ItemType);
  void SetTargetAsPosition(Vector2D TargetPosition);

//--------------------------------------------------------------------------------
// 下面是一组只读查询(末尾 const = 不改对象):
//   isTargetAnItem()    —— 目标是不是个物品?
//   isTargetAPosition() —— 目标是不是个坐标点?
//   isTargetValid()      —— 目标有效吗?!(m_Type==invalid) = 不是无效 = 有效。
//--------------------------------------------------------------------------------
  bool isTargetAnItem()const{return m_Type == item;}
  bool isTargetAPosition()const{return m_Type == position;}
  bool isTargetValid()const{return !(m_Type == invalid);}

// 两个取值函数(声明):拿目标坐标 / 拿目标物品类型。
  Vector2D GetTargetPosition()const;
  int      GetTargetType()const;

};

#endif