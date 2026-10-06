//==============================================================================================
//【文件说明】DebugConsole.h —— 调试控制台窗口(可缩放,显示/记录调试信息)
//
//【这个文件是干什么的?】
//  一个单独弹出的小窗口,用来显示程序运行时的调试文字。用法和 std::cout 一样:
//    debug_con << "敌人血量=" << hp << "";   // 末尾空串触发刷新到屏幕
//  它是个单例(整个程序只有一个窗口),文字先攒在内存缓冲里,攒满 500 行自动写入
//  DebugLog.txt。若代码里没定义 DEBUG 宏,debug_con 就被替换成 CSink(黑洞,全部丢弃)。
//
//【谁在使用这个文件?】各 main.cpp、GameWorld 等调试输出处。
//【本文件包含了谁?】<vector>/<windows.h>/<fstream>、misc/utils.h、misc/WindowUtils.h。
//==============================================================================================
#ifndef DEBUG_CONSOLE_H
#define DEBUG_CONSOLE_H
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
// Name:   DebugConsole.h
//
// Desc:   Creates a resizable console window for recording and displaying
//         debug info.
//
//         use the debug_con macro to send text and types to the console
//         window via the << operator (just like std::cout). Flush the
//         buffer using "" or the flush macro.  eg. 
//
//        debug_con << "Hello World!" << "";
//
// Author: Mat Buckland 2001 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【DebugConsole —— 调试控制台】:创建一个可缩放的控制台窗口,用来记录和显示
//   调试信息。用 debug_con 宏像 std::cout 一样往窗口写文字;用空串或 flush 宏刷新缓冲。
//   例:debug_con << "Hello World!" << "";  作者:Mat Buckland 2001。
//------------------------------------------------------------------------
#include <vector>
#include <windows.h>
#include <iosfwd>
#include <fstream>

#include "misc/utils.h"
#include "misc/WindowUtils.h"


//need to define a custom message
  //(原文注释:需要自定义一个 Windows 消息)—— UM_SETSCROLL=WM_USER+32,"滚到底部"。
const int UM_SETSCROLL = WM_USER + 32;

//maximum number of lines shown in console before the buffer is flushed to 
//a file
  // ↓↓↓ 上面两行英文注释的翻译:缓冲满多少行后,把内容写入文件。MaxBufferSize=500。
const int MaxBufferSize = 500;

//initial dimensions of the console window
const int DEBUG_WINDOW_WIDTH  = 400;
const int DEBUG_WINDOW_HEIGHT = 400;

  //(原文注释:取消定义 DEBUG,则所有调试消息都扔进"黑洞"——见下面 CSink)
  // #ifdef DEBUG:若定义了 DEBUG,debug_con 展开成真控制台;否则展开成 CSink(全丢弃)。
//undefine DEBUG to send all debug messages to hyperspace (a sink - see below)
//#define DEBUG
#ifdef DEBUG
#define debug_con *(DebugConsole::Instance())
#else
#define debug_con *(CSink::Instance())
#endif

  //(原文注释:在代码里用这两个宏开/关控制台输出)—— debug_on / debug_off。
//use these in your code to toggle output to the console on/off
#define debug_on  DebugConsole::On();
#define debug_off DebugConsole::Off();


//this little class just acts as a sink for any input. Used in place
//of the DebugConsole class when the console is not required
// ↓↓↓ 上面两行英文注释的翻译:这个小类当"输入黑洞"用——不需要控制台时,用它代替
//   DebugConsole。class CSink:所有 << 都什么也不做,直接返回自己(丢弃)。单例。
class CSink
{
private:

  CSink(){};

  //copy ctor and assignment should be private
  CSink(const CSink&);
  CSink& operator=(const CSink&);
  
public:

  static CSink* Instance(){static CSink instance; return &instance;}
  
  template<class T>
  CSink& operator<<(const T&)
  {
	  return *this;
  }
};



// class DebugConsole —— 真正的调试控制台(单例,全是 static 成员)。
class DebugConsole
{
private:

  static HWND	         m_hwnd;
  
  //the string buffer. All input to debug stream is stored here
  //(原文注释:字符串缓冲,所有调试输出先存在这里)—— m_Buffer:每行一个字符串。
  static std::vector<std::string> m_Buffer;
  
  //if true the next input will be pushed into the buffer. If false,
  //it will be appended.
  // ↓↓↓ 上面两行英文注释的翻译:m_bFlushed=true 时,下一条输入另起新行;否则接在上行末尾。
  static bool          m_bFlushed;  
  
  //position of debug window
  //(原文注释:调试窗口位置)—— m_iPosTop/m_iPosLeft。
  static int           m_iPosTop;
  static int           m_iPosLeft;

  //set to true if the window is destroyed
  //(原文注释:窗口被销毁时设为 true)—— m_bDestroyed。
  static bool          m_bDestroyed;

  //if false the console will just disregard any input
  //(原文注释:为 false 时控制台丢弃一切输入)—— m_bActive:总开关。
  static bool          m_bActive;

  //default logging file
  //(原文注释:默认日志文件)—— m_LogOut:输出文件流。
  static std::ofstream m_LogOut;



  //the debug window message handler
  //(原文注释:调试窗口的消息处理函数)—— DebugWindowProc:处理 Windows 消息。
  static LRESULT CALLBACK DebugWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  //this registers the window class and creates the window(called by the ctor)
  //(原文注释:注册窗口类并创建窗口,由构造时调用)—— Create。
  static bool             Create();

  static void             DrawWindow(){InvalidateRect(m_hwnd, NULL, TRUE); UpdateWindow(m_hwnd);}

private:

  // 构造函数私有 + 拷贝/赋值私有,保证全局只有一个实例(单例模式)。
  DebugConsole(){}
 
  //copy ctor and assignment should be private
  DebugConsole(const DebugConsole&);
  DebugConsole& operator=(const DebugConsole&);

public:

  ~DebugConsole(){WriteAndResetBuffer(); }

  static DebugConsole* Instance();

             
  void ClearBuffer(){m_Buffer.clear(); flush();}
  // Instance:单例取口(实现在 .cpp)。ClearBuffer:清空缓冲并刷新。


  // flush:通知窗口把滚动条滚到底(窗口没销毁才发消息)。
  static void flush()
  {
    if (!m_bDestroyed)
    {
      m_bFlushed = true; SendMessage(m_hwnd, UM_SETSCROLL, NULL, NULL);
    }
  }

  //writes the contents of the buffer to the file "debug_log.txt", clears
  //the buffer and resets the appropriate scroll info
  // ↓↓↓ 上面两行英文注释的翻译:把缓冲内容写入 debug_log.txt,清空缓冲并重置滚动位置。
  void WriteAndResetBuffer();

  //(原文注释:用来开/关)—— Off/On:关闭或启用输出。
  //use to activate deactivate
  static void  Off(){m_bActive = false;}
  static void  On()  {m_bActive = true;}

  bool Destroyed()const{return m_bDestroyed;}
 

  //(原文注释:重载 << 以接受任意类型)—— operator<<:模板,任何类型都能塞进缓冲。
  //overload the << to accept any type
  template <class T>
  DebugConsole& operator<<(const T& t)
  {
    if (!m_bActive || m_bDestroyed) return *this;
   
    //reset buffer and scroll info if it overflows. Write the excess
    //to file
    if (m_Buffer.size() > MaxBufferSize)
    {
       WriteAndResetBuffer();
    }
    
    std::ostringstream ss; ss << t;

    if (ss.str() == ""){flush(); return *this;}
    
    if (!m_bFlushed)
      {m_Buffer.back() += ss.str();}
    else
      {m_Buffer.push_back(ss.str());m_bFlushed = false;}

    return *this;
  }
};

 

#endif