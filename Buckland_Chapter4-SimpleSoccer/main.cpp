//==============================================================================================
//【程序总览】SimpleSoccer —— 《Programming Game AI by Example》第 4 章的足球模拟示例
//
// 这是一个 Windows 窗口程序(不是控制台):红蓝两支 5 人足球队在一块 600×400 的
// 场地上自动比赛。它演示了游戏 AI 的两大经典技术:
//   ★ 层级状态机(Hierarchical FSM)★  —— 球队有进攻/防守/开球状态,
//     球员又各有追球/带球/传球/接球/支援/回位/等待等状态,守门员另有守门/拦截/发球状态;
//   ★ 转向行为(Steering Behaviors)★ —— 每个球员每帧把直奔、减速、避让队友、
//     追球、拦截等几种"力"叠加,算出该往哪走。
//
//【整个工程的文件地图及相互关系】(▶ 表示"包含/创建/驱动"方向)
//   main.cpp(本文件:Windows 程序入口 WinMain + 消息循环 + 球场渲染)◀── 总调度
//      │
//      ├────▶ constants.h        —— 窗口宽高、每队人数等常量。
//      ├────▶ ParamLoader.h/.cpp  —— 单例:从 Params.ini 读全部比赛参数(宏 Prm.XXX)。
//      ├────▶ SoccerPitch.h/.cpp  —— 球场:持有两队、两门、足球、边界墙、区域,
//      │                              Update() 每帧驱动全场,Render() 画全场。
//      │           │
//      │           ├────▶ SoccerTeam.h/.cpp  —— 红队/蓝队(球队级状态机 + 战术决策):
//      │           │           │              射门 CanShoot、传球 FindPass、找接应点。
//      │           │           ├────▶ TeamStates.h/.cpp        —— 球队状态:进攻/防守/开球。
//      │           │           ├────▶ FieldPlayer.h/.cpp       —— 4 名场上球员(球员级状态机)。
//      │           │           │           ├────▶ FieldPlayerStates.h/.cpp —— 追球/带球/
//      │           │           │           │                                  踢球/接球/支援/回位/等待。
//      │           │           │           └────▶ SteeringBehaviors.h/.cpp  —— 转向行为(直奔/
//      │           │           │                                             减速/避让/追球/拦截)。
//      │           │           ├────▶ GoalKeeper.h/.cpp       —— 1 名守门员(守门员级状态机)。
//      │           │           │           └────▶ GoalKeeperStates.h/.cpp —— 守门/拦截/回门/发球。
//      │           │           └────▶ SupportSpotCalculator.h/.cpp —— 接应甜区评分。
//      │           ├────▶ Goal.h(+空 Goal.cpp)—— 球门:门柱、朝向、进球检测 Scored()。
//      │           ├────▶ SoccerBall.h/.cpp   —— 足球:踢球 Update、撞墙反弹、预测未来位置。
//      │           └────▶ PlayerBase.h/.cpp  —— 球员基类(位置/朝向/距离查询)。
//      │
//      ├────▶ SoccerMessages.h/.cpp  —— 球员间消息种类与消息名翻译。
//      ├────▶ resource.h / Script1.rc —— 菜单与图标(资源编号),非游戏逻辑。
//      │
//      │      (以下在公共目录 Common\,靠"附加包含目录"找到,本章只使用不展开)
//      │      FSM/State.h、FSM/StateMachine.h  —— 状态接口与状态机模板;
//      │      Game/MovingEntity.h、Game/Region.h —— 运动物体基类、场地矩形区域;
//      │      Messaging/*  —— 球员间消息分发;misc/Cgdi.h  —— 绘图单例 gdi;
//      │      misc/iniFileLoaderBase.h —— INI 读取基类;2D/* —— 向量与几何工具。
//
//【一次完整运行的调用流程】
//   ① WinMain 注册窗口类、创建窗口,启动高精度定时器 timer;
//   ② 进入消息循环:PeekMessage 取消息,TranslateMessage+DispatchMessage 派发;
//   ③ 窗口首次创建(WM_CREATE)时:new SoccerPitch —— 球场构造函数里依次 new 出
//      比赛区域、切分区域、两门、足球、红蓝两队(队里再 new 出 1 门将 + 4 场上球员);
//   ④ 每当 timer 到下一帧(ReadyForNextFrame):先 g_SoccerPitch->Update() ——
//      球场先更新球,再让红队、蓝队各 Update(球队状态机→每个球员状态机→转向力→移动);
//      再 RedrawWindow 触发 WM_PAINT:g_SoccerPitch->Render() 把全场画到后台缓冲、
//      BitBlt 贴到屏幕;
//   ⑤ 检测到进球就把球放回中场、两队切到开球准备状态;按 Esc 退出、P 暂停、R 重开。
//==============================================================================================
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786):关闭老 VC6 的 4786 号警告(详见 SoccerPitch.h 注释)。
//--------------------------------------------------------------------------------
#pragma warning (disable:4786)
//--------------------------------------------------------------------------------
// #include 列表(预处理阶段把这些文件内容复制进来):
//   <windows.h>/<time.h>       —— Windows 系统 API 与时间;
//   "constants.h"             —— 窗口宽高/人数常量;
//   "misc/utils.h"            —— 随机数、ttos 等工具;
//   "Time/PrecisionTimer.h"   —— 高精度帧定时器;
//   "SoccerPitch.h"           —— 球场类;
//   "misc/Cgdi.h"            —— 绘图单例 gdi;
//   "ParamLoader.h"           —— 参数单例宏 Prm;
//   "Resource.h"             —— 菜单/图标编号;
//   "misc/WindowUtils.h"     —— 菜单勾选等窗口小工具;
//   "debug/DebugConsole.h"    —— 调试控制台。
//--------------------------------------------------------------------------------
#include <windows.h>
#include <time.h>

#include "constants.h"
#include "misc/utils.h"
#include "Time/PrecisionTimer.h"
#include "SoccerPitch.h"
#include "misc/Cgdi.h"
#include "ParamLoader.h"
#include "Resource.h"
#include "misc/WindowUtils.h"
#include "debug/DebugConsole.h"


//--------------------------------- Globals ------------------------------
//
//------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// 全局变量(g_ 前缀):程序名/窗口类名、球场指针 g_SoccerPitch、帧定时器 timer。
// WIN32 程序里这些是全程序唯一一份,各回调函数都靠它们访问球场。
//--------------------------------------------------------------------------------
char* g_szApplicationName = "Simple Soccer";
char*	g_szWindowClassName = "MyWindowClass";

SoccerPitch* g_SoccerPitch;

//create a timer
PrecisionTimer timer(Prm.FrameRate);


//used when a user clicks on a menu item to ensure the option is 'checked'
//correctly
// CheckAllMenuItemsAppropriately:根据当前各 b 开关的取值,把对应菜单项打勾/取消。
void CheckAllMenuItemsAppropriately(HWND hwnd)
{
   CheckMenuItemAppropriately(hwnd, IDM_SHOW_REGIONS, Prm.bRegions);
   CheckMenuItemAppropriately(hwnd, IDM_SHOW_STATES, Prm.bStates);
   CheckMenuItemAppropriately(hwnd, IDM_SHOW_IDS, Prm.bIDs);
   CheckMenuItemAppropriately(hwnd, IDM_AIDS_SUPPORTSPOTS, Prm.bSupportSpots);
   CheckMenuItemAppropriately(hwnd, ID_AIDS_SHOWTARGETS, Prm.bViewTargets);
   CheckMenuItemAppropriately(hwnd, IDM_AIDS_HIGHLITE, Prm.bHighlightIfThreatened);
}


//---------------------------- WindowProc ---------------------------------
//	
//	This is the callback function which handles all the windows messages
//-------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// WindowProc:Windows"窗口过程回调函数"。操作系统把窗口消息(创建/菜单/按键/
// 重绘/缩放/销毁)一个个塞进这个函数,我们用 switch(msg) 分类处理:
//   WM_CREATE  窗口首次创建:建后台缓冲(backbuffer)、new 出球场;
//   WM_COMMAND 点菜单:翻转对应的 b 调试开关;
//   WM_KEYUP   按键:Esc 退出、R 重开球场、P 暂停;
//   WM_PAINT   重绘:让球场 Render 并把后台缓冲贴到屏幕;
//   WM_SIZE    改窗口大小:重建后台位图;
//   WM_DESTROY 销毁:释放资源、PostQuitMessage 退出。
//--------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc (HWND   hwnd,
                             UINT   msg,
                             WPARAM wParam,
                             LPARAM lParam)
{
 
   //these hold the dimensions of the client window area
	 static int cxClient, cyClient; 

	 //used to create the back buffer
   static HDC		hdcBackBuffer;
   static HBITMAP	hBitmap;
   static HBITMAP	hOldBitmap;

    switch (msg)
    {
	
		//A WM_CREATE msg is sent when your application window is first
		//created
    case WM_CREATE:
      {
         //to get get the size of the client window first we need  to create
         //a RECT and then ask Windows to fill in our RECT structure with
         //the client window size. Then we assign to cxClient and cyClient 
         //accordingly
			   RECT rect;

			   GetClientRect(hwnd, &rect);

			   cxClient = rect.right;
			   cyClient = rect.bottom;

         //seed random number generator
         srand((unsigned) time(NULL));

         
         //---------------create a surface to render to(backbuffer)

         //create a memory device context
         hdcBackBuffer = CreateCompatibleDC(NULL);

         //get the DC for the front buffer
         HDC hdc = GetDC(hwnd);

         hBitmap = CreateCompatibleBitmap(hdc,
                                          cxClient,
                                          cyClient);

			  
         //select the bitmap into the memory device context
			   hOldBitmap = (HBITMAP)SelectObject(hdcBackBuffer, hBitmap);

         //don't forget to release the DC
         ReleaseDC(hwnd, hdc); 
         
         g_SoccerPitch = new SoccerPitch(cxClient, cyClient); 
         
         CheckAllMenuItemsAppropriately(hwnd);

      }

      break;

    case WM_COMMAND:
      {
        switch(wParam)
        {
          case ID_AIDS_NOAIDS:

            Prm.bStates        = 0;
            Prm.bRegions       = 0;
            Prm.bIDs           = 0;
            Prm.bSupportSpots  = 0;
            Prm.bViewTargets   = 0;

            CheckAllMenuItemsAppropriately(hwnd);

            break;
            
          case IDM_SHOW_REGIONS:

            Prm.bRegions = !Prm.bRegions;

            CheckAllMenuItemsAppropriately(hwnd);

            break;

          case IDM_SHOW_STATES:

            Prm.bStates = !Prm.bStates;

            CheckAllMenuItemsAppropriately(hwnd);

            break;

          case IDM_SHOW_IDS:

            Prm.bIDs = !Prm.bIDs;

            CheckAllMenuItemsAppropriately(hwnd);

            break;


          case IDM_AIDS_SUPPORTSPOTS:

            Prm.bSupportSpots = !Prm.bSupportSpots;

            CheckAllMenuItemsAppropriately(hwnd);

             break;

           case ID_AIDS_SHOWTARGETS:

            Prm.bViewTargets = !Prm.bViewTargets;

            CheckAllMenuItemsAppropriately(hwnd);

             break;
              
           case IDM_AIDS_HIGHLITE:

            Prm.bHighlightIfThreatened = !Prm.bHighlightIfThreatened; 

            CheckAllMenuItemsAppropriately(hwnd);

            break;
            
        }//end switch
      }

      break;


    case WM_KEYUP:
      {
        switch(wParam)
        {
           case VK_ESCAPE:
            {             
              SendMessage(hwnd, WM_DESTROY, NULL, NULL);
            }
          
            break;

          case 'R':
            {
               delete g_SoccerPitch;
           
               g_SoccerPitch = new SoccerPitch(cxClient, cyClient);
            }

            break;

          case 'P':
            {
              g_SoccerPitch->TogglePause();
            }

            break;

        }//end switch
        
      }//end WM_KEYUP

      break;

    
    case WM_PAINT:
      {
 		       
         PAINTSTRUCT ps;
          
         BeginPaint (hwnd, &ps);
         
         gdi->StartDrawing(hdcBackBuffer);
         
         g_SoccerPitch->Render();

         gdi->StopDrawing(hdcBackBuffer);

        

         //now blit backbuffer to front
			   BitBlt(ps.hdc, 0, 0, cxClient, cyClient, hdcBackBuffer, 0, 0, SRCCOPY); 
          
         EndPaint (hwnd, &ps);

      }

      break;

    //has the user resized the client area?
		case WM_SIZE:
		  {
        //if so we need to update our variables so that any drawing
        //we do using cxClient and cyClient is scaled accordingly
			  cxClient = LOWORD(lParam);
			  cyClient = HIWORD(lParam);

      //now to resize the backbuffer accordingly. First select
      //the old bitmap back into the DC
			SelectObject(hdcBackBuffer, hOldBitmap);

      //don't forget to do this or you will get resource leaks
      DeleteObject(hBitmap); 

			//get the DC for the application
      HDC hdc = GetDC(hwnd);

			//create another bitmap of the same size and mode
      //as the application
      hBitmap = CreateCompatibleBitmap(hdc,
											cxClient,
											cyClient);

			ReleaseDC(hwnd, hdc);
			
			//select the new bitmap into the DC
      SelectObject(hdcBackBuffer, hBitmap);

      }

      break;
          
		 case WM_DESTROY:
			 {

         //clean up our backbuffer objects
         SelectObject(hdcBackBuffer, hOldBitmap);

         DeleteDC(hdcBackBuffer);
         DeleteObject(hBitmap); 
         
         // kill the application, this sends a WM_QUIT message  
				 PostQuitMessage (0);
			 }

       break;

     }//end switch

     //this is where all the messages not specifically handled by our 
		 //winproc are sent to be processed
		 return DefWindowProc (hwnd, msg, wParam, lParam);
}

//-------------------------------- WinMain -------------------------------
//
//	The entry point of the windows program
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// WinMain:Windows 程序的真正入口(替代控制台的 main)。
// 流程:填窗口类 WNDCLASSEX 并注册 → CreateWindowEx 建窗口 → 启动定时器 →
// 进入消息循环(PeekMessage/DispatchMessage);每帧到时就 Update 球场 + RedrawWindow。
// 退出后 delete 球场、注销窗口类。
//--------------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     szCmdLine, 
                    int       iCmdShow)
{

  //handle to our window
  HWND						hWnd;
    
  //our window class structure
  WNDCLASSEX     winclass;
		 
  // first fill in the window class stucture
  winclass.cbSize        = sizeof(WNDCLASSEX);
  winclass.style         = CS_HREDRAW | CS_VREDRAW;
  winclass.lpfnWndProc   = WindowProc;
  winclass.cbClsExtra    = 0;
  winclass.cbWndExtra    = 0;
  winclass.hInstance     = hInstance;
  winclass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
  winclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  winclass.hbrBackground = NULL;
  winclass.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
  winclass.lpszClassName = g_szWindowClassName;
  winclass.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

  //register the window class
  if (!RegisterClassEx(&winclass))
  {
    MessageBox(NULL, "Registration Failed!", "Error", 0);

    //exit the application
    return 0;
  }

  //create the window and assign its ID to hwnd    
  hWnd = CreateWindowEx (NULL,                 // extended style
                         g_szWindowClassName,  // window class name
                         g_szApplicationName,  // window caption
                         WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
                         GetSystemMetrics(SM_CXSCREEN)/2 - WindowWidth/2,
                         GetSystemMetrics(SM_CYSCREEN)/2 - WindowHeight/2,                    
                         WindowWidth,     // initial x size
                         WindowHeight,    // initial y size
                         NULL,                 // parent window handle
                         NULL,                 // window menu handle
                         hInstance,            // program instance handle
                         NULL);                // creation parameters

  //make sure the window creation has gone OK
  if(!hWnd)
  {
    MessageBox(NULL, "CreateWindowEx Failed!", "Error!", 0);
  }
  
  //start the timer
  timer.Start();

  MSG msg;

  //enter the message loop
  bool bDone = false;

  while(!bDone)
  {
					
    while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
    {
      if( msg.message == WM_QUIT ) 
      {
        // Stop loop if it's a quit message
	      bDone = true;
      } 

      else 
      {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
      }
    }

    if (timer.ReadyForNextFrame() && msg.message != WM_QUIT)
    {
      //update game states
      g_SoccerPitch->Update(); 
      
      //render 
      RedrawWindow(hWnd, true);

      Sleep(2);
    }
   					
  }//end while

  delete g_SoccerPitch;

  UnregisterClass( g_szWindowClassName, winclass.hInstance );

  return msg.wParam;
}


