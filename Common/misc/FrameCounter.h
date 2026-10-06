//==============================================================================================
//【文件说明】FrameCounter.h —— 全局帧计数器(单例,记录"现在第几帧")
//
//【这个文件是干什么的?】
//  游戏每帧调一次 Update() 让计数器 +1。MessageDispatcher 用它判断延迟消息
//  是否到该投递的时间。宏 TickCounter = FrameCounter::Instance() 让代码写起来短。
//
//【谁在使用这个文件?】
//  MessageDispatcher.cpp(取当前帧做消息时间戳)、各工程 main.cpp(每帧 Update)。
//==============================================================================================
#ifndef FRAMECOUNTER_H
#define FRAMECOUNTER_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------


#define TickCounter FrameCounter::Instance()

//--------------------------------------------------------------------------------
// class FrameCounter —— 帧计数器(单例,私有构造)。
//   m_lCount = 从程序启动到现在的总帧数;m_iFramesElapsed = Start() 后的帧数。
//--------------------------------------------------------------------------------
class FrameCounter
{
private:

  long m_lCount;

  int  m_iFramesElapsed;

  FrameCounter():m_lCount(0), m_iFramesElapsed(0){}

  //copy ctor and assignment should be private
  FrameCounter(const FrameCounter&);
  FrameCounter& operator=(const FrameCounter&);

public:

  static FrameCounter* Instance();

  // Update:每帧 +1;GetCurrentFrame:取当前帧号;Reset 归零;Start/FramesElapsedSinceStartCalled 测速。
  void Update(){++m_lCount; ++m_iFramesElapsed;}

  long GetCurrentFrame(){return m_lCount;}

  void Reset(){m_lCount = 0;}

  void Start(){m_iFramesElapsed = 0;}
  int  FramesElapsedSinceStartCalled()const{return m_iFramesElapsed;}

};

#endif