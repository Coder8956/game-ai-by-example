//==============================================================================================
//【程序总览】第三章 —— 《Programming Game AI by Example》第 3 章"转向行为"
//  (Steering Behaviors)的演示程序。
//
// 这是一个 Windows 图形窗口程序:300 辆"小鱼"机器人(三角形)在屏幕上
// 成群飞行(群体行为),其中最后一只被改造成巨大的"鲨鱼",其余小鱼全部
// 开启"逃命"行为躲着它。可以用鼠标右键设目标点、键盘/菜单调节各种参数,
// 观察 Seek/Arrive/Wander/避障/避墙/沿路径等十几种转向行为的效果。
//
//【整个工程的文件地图及相互关系】(带 ▶ 的箭头表示"包含/使用"方向)
//   main.cpp(本文件:程序入口——建窗口→建世界→消息循环驱动 Update/Render)
//      │
//      ├────▶ GameWorld.h ─ GameWorld.cpp 是它的实现
//      │           └ 游戏世界容器:300 个机器人、障碍物、墙、路径、网格;
//      │             每帧 Update() 驱动所有机器人,Render() 画整个画面。
//      │             还负责键盘/菜单消息的处理。
//      ├────▶ Vehicle.h ─ Vehicle.cpp 是它的实现
//      │           └ 机器人:继承 MovingEntity;持有一个转向控制器和一个
//      │             朝向平滑器;Update() 把"转向力→加速度→速度→位置"。
//      ├────▶ SteeringBehaviors.h ─ SteeringBehaviors.cpp 是它的实现
//      │           └ 转向行为库(第三章核心):十几个行为算法 + 三种合力方式;
//      │             机器人每帧从它拿"总转向力"。
//      ├────▶ MovingEntity.h —— 会动实体的基类:速度/朝向/质量/最大推力;
//      ├────▶ BaseGameEntity.h —— 所有实体的祖宗:编号/位置/半径/标记;
//      ├────▶ Obstacle.h/.cpp —— 障碍物(圆);
//      ├────▶ Path.h/.cpp —— 路径(一串路标点,沿路径行为用);
//      ├────▶ ParamLoader.h/.cpp —— 读 params.ini 参数(宏 Prm 取用);
//      ├────▶ EntityFunctionTemplates.h —— 打标记/防穿模等实体通用模板;
//      ├────▶ constants.h —— 窗口尺寸常量;
//      │
//      │    Common\(公共目录,被各章共享,本工程只使用不修改):
//      │      misc/Cgdi.h(绘图单例 gdi)、2D/Vector2D.h(向量)、
//      │      2D/Transformations.h(坐标变换)、2D/Wall2D.h(墙)、
//      │      misc/Smoother.h(平滑器)、misc/CellSpacePartition.h(网格)、
//      │      time/PrecisionTimer.h(高精度计时器)、misc/utils.h(工具)、
//      │      misc/WindowUtils.h(菜单工具)、misc/Stream_Utility_Functions.h 等。
//      │      工程靠"附加包含目录 ..\..\Common"才能找到它们。
//
//【一次完整执行的调用流程】(建议按此顺序阅读各文件)
//   ① WinMain():注册窗口类 → 创建窗口 → 显示窗口;
//   ② 窗口创建消息 WM_CREATE 到达 → WindowProc() 里:
//      创建"后备缓冲"(内存画布,先画后翻屏,防闪烁)→ new GameWorld(宽,高);
//   ③ GameWorld 构造函数(见 GameWorld.cpp):按 params.ini 参数创建 300 辆
//      机器人(开群体飞行;最后一辆变"鲨鱼",其余开"逃命"),建网格、建路径;
//   ④ 进入主消息循环:while(!bDone){ PeekMessage 处理消息;
//      g_GameWorld->Update(计时器每帧时间) 更新世界;
//      RedrawWindow() 触发 WM_PAINT → WindowProc 里
//      g_GameWorld->Render() 画到后备缓冲,再 BitBlt 翻到屏幕;Sleep(2); }
//   ⑤ 世界 Update 逐个驱动 Vehicle::Update → SteeringBehavior::Calculate()
//      合成转向力 → 力÷质量=加速度 → 更新速度/位置/朝向;
//   ⑥ 用户交互:右键(WM_LBUTTONUP)→ SetCrosshair 设目标点;
//      键盘(WM_KEYUP)→ HandleKeyPresses / HandleMenuItems;
//   ⑦ 关窗口(WM_DESTROY)→ 清理后备缓冲 → PostQuitMessage 退出消息循环;
//      最后 delete g_GameWorld 释放整个世界。
//==============================================================================================
#pragma warning (disable:4786)

// 包含 Windows API 头文件:窗口程序的一切(HWND、消息、GDI 绘图)都靠它。
#include <windows.h>
// 包含 C 标准库 time.h:下面 srand(time(NULL)) 要用 time() 取当前时间做
// 随机数种子(让每次运行的随机结果不一样)。
#include <time.h>

// 包含窗口尺寸常量:CreateWindowEx 指定窗口大小时用。
#include "constants.h"
// 包含 Common\misc\utils.h 通用工具(习惯性包含)。
#include "misc/utils.h"
// 包含高精度计时器(Common 目录):主循环用 timer.TimeElapsed() 量帧时间。
#include "time/PrecisionTimer.h"
// 包含游戏世界类:创建世界、每帧调 Update/Render 全靠它。
#include "GameWorld.h"
// 包含绘图单例 gdi(misc/Cgdi.h):WM_PAINT 里 StartDrawing/StopDrawing。
#include "misc/Cgdi.h"
// 包含参数加载器(宏 Prm 的定义处,习惯性包含)。
#include "ParamLoader.h"
// 包含资源头(resource.h 由 VS 生成):菜单 ID(IDR_MENU1 等)。
#include "resource.h"
// 包含窗口工具(Common 目录 misc/WindowUtils.h):ChangeMenuState(勾选菜单)。
#include "misc/WindowUtils.h"

//--------------------------------------------------------------------------------
//【全局变量区】(Globals)三个全局变量全工程可见:
//--------------------------------------------------------------------------------
//--------------------------------- Globals ------------------------------
//
//------------------------------------------------------------------------

// 窗口标题文字(显示在标题栏):"Steering Behaviors - Another Big Shoal"
// (转向行为——另一个大鱼群)。char* 是 C 风格字符串指针。
char* g_szApplicationName = "Steering Behaviors - Another Big Shoal";
// 窗口类名(Windows 内部登记用,名字自己起,不重复即可)。
char*	g_szWindowClassName = "MyWindowClass";

// 全局世界指针 g_GameWorld:整个程序只有这一份世界,任何函数都能通过它
// 访问世界(创建于 WM_CREATE,销毁于程序结束)。
GameWorld* g_GameWorld;


//--------------------------------------------------------------------------------
//【WindowProc:窗口消息处理函数】
//  这是 Windows 窗口程序的"心脏":系统把各种消息(创建/画图/按键/鼠标/关闭)
// 送进来,我们按消息类型分支处理。LRESULT CALLBACK 是固定的函数签名格式。
//--------------------------------------------------------------------------------
//---------------------------- WindowProc ---------------------------------
//	
//	This is the callback function which handles all the windows messages
//-------------------------------------------------------------------------

// WindowProc 函数定义开始。四个参数:
//   HWND hwnd  —— 窗口句柄(哪个窗口的消息);
//   UINT msg   —— 消息编号(如 WM_PAINT 是画图消息);
//   WPARAM/LPARAM —— 两个消息参数(内容随消息类型而定)。
LRESULT CALLBACK WindowProc (HWND   hwnd,
                             UINT   msg,
                             WPARAM wParam,
                             LPARAM lParam)
{
   //these hold the dimensions of the client window area
// 静态变量:窗口客户区尺寸(宽、高)。static:函数多次调用间保留值。
// 注意行首是制表符,原样保留。
	 static int cxClient, cyClient; 

	 //used to create the back buffer
// 静态变量:后备缓冲三件套——
//   hdcBackBuffer —— 内存设备上下文(内存画布);
   static HDC		hdcBackBuffer;
//   hBitmap —— 位图(画布内容);
   static HBITMAP	hBitmap;
//   hOldBitmap —— 换下来时保存的旧位图(清理时还原用)。
   static HBITMAP	hOldBitmap;

// switch(消息编号) 分支处理。
    switch (msg)
    {
	
// ↓↓↓ 原文注释翻译:WM_CREATE 消息在窗口第一次创建时发送。
		//A WM_CREATE msg is sent when your application window is first
		//created
// 处理"窗口创建"消息:
    case WM_CREATE:
      {
// ↓↓↓ 原文注释翻译:要拿到客户区尺寸,先建一个 RECT(矩形),再让 Windows
//    把客户区大小填进去,然后赋给 cxClient / cyClient。
         //to get get the size of the client window first we need  to create
         //a RECT and then ask Windows to fill in our RECT structure with
         //the client window size. Then we assign to cxClient and cyClient 
         //accordingly
// 定义一个矩形结构(Windows 用它描述一块区域);
			   RECT rect;

// 让系统把客户区矩形填进 rect(& 取地址,函数往里写);
			   GetClientRect(hwnd, &rect);

// 客户区宽 = rect 的右边界值;
			   cxClient = rect.right;
// 客户区高 = rect 的下边界值。
			   cyClient = rect.bottom;

         //seed random number generator
// 播种随机数发生器:srand(种子)。种子用当前时间(time(NULL)),
// 这样每次启动的随机序列都不同(出生点、路径都不一样)。
         srand((unsigned) time(NULL));  

         
// ↓↓↓ 原文注释翻译:创建一块用于渲染的"后备缓冲"(surface = 表面)。
         //---------------create a surface to render to(backbuffer)

         //create a memory device context
// 创建与屏幕兼容的"内存设备上下文"(画布)——先在内存里画,再一次性
// 翻到屏幕,避免逐笔直画造成闪烁。
         hdcBackBuffer = CreateCompatibleDC(NULL);

         //get the DC for the front buffer
// 拿窗口的前台设备上下文(屏幕);
         HDC hdc = GetDC(hwnd);

// 创建一块与窗口客户区一样大的位图(画布纸);
         hBitmap = CreateCompatibleBitmap(hdc,
                                          cxClient,
                                          cyClient);

			  
         //select the bitmap into the memory device context
// 把位图选进内存画布(以后画图都画在这张纸上),并保存旧位图句柄。
// SelectObject 返回之前的对象,存起来等清理时还原。
			   hOldBitmap = (HBITMAP)SelectObject(hdcBackBuffer, hBitmap);

         //don't forget to release the DC
// 释放前台 DC(用完要还,否则资源泄漏)。
         ReleaseDC(hwnd, hdc); 
         
// ★ 创建游戏世界:按客户区尺寸 new GameWorld(宽, 高)。
// 这会触发 GameWorld 构造函数:造 300 辆机器人、网格、路径(见 GameWorld.cpp)。
         g_GameWorld = new GameWorld(cxClient, cyClient);

// 初始菜单勾选:默认"优先级合成法"和"显示 FPS"两项打钩。
         ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_CHECKED);
         ChangeMenuState(hwnd, ID_VIEW_FPS, MFS_CHECKED);
         
      }

      break;

// 处理"命令消息"(菜单点击):转交给世界的 HandleMenuItems。
    case WM_COMMAND:
    {
      g_GameWorld->HandleMenuItems(wParam, hwnd); 
    }

    break;


// 处理"鼠标左键抬起"(实际是右键习惯,原书如此):把鼠标位置转成 POINTS,
// 交给世界 SetCrosshair 设目标点(注意原注释写 WM_LBUTTONUP)。
    case WM_LBUTTONUP:
    {
      g_GameWorld->SetCrosshair(MAKEPOINTS(lParam));
    }
    
    break;
// 处理"键盘松开"消息:

    case WM_KEYUP:
      {
// 内层 switch:按按键码分支。
        switch(wParam)
        {
// Esc 键:发送 WM_DESTROY 给自己(触发关窗流程);
           case VK_ESCAPE:
            {             
              SendMessage(hwnd, WM_DESTROY, NULL, NULL);
            }
          
            break;

// R 键:重置世界——先删旧世界,再按同样尺寸 new 一个新世界。
// (旧世界析构会释放全部机器人;新世界重新随机生成。)
          case 'R':
            {
               delete g_GameWorld;
           
               g_GameWorld = new GameWorld(cxClient, cyClient);
            }

            break;
           

        }//end switch

        //handle any others
// 其余按键统一交给世界的 HandleKeyPresses(U/P/O/I/Y 等)。
        g_GameWorld->HandleKeyPresses(wParam);
        
      }//end WM_KEYUP

      break;

    
// 处理"重绘"消息(窗口需要刷新画面时):
    case WM_PAINT:
      {
 		       
// 声明绘图结构 PAINTSTRUCT(系统填充,含绘图 DC);
         PAINTSTRUCT ps;
          
// 开始绘图(系统发放绘图 DC);
         BeginPaint (hwnd, &ps);

        //fill our backbuffer with white
// 用 WHITENESS 模式把后备缓冲整块刷白(清空上一帧画面);
// BitBlt 是"位块传送":把一块像素搬来搬去。
         BitBlt(hdcBackBuffer,
                0,
                0,
                cxClient,
                cyClient,
                NULL,
                NULL,
                NULL,
                WHITENESS);

         
// 让绘图单例 gdi 开始往后备缓冲上画;
         gdi->StartDrawing(hdcBackBuffer);
         
// ★ 画整个世界:世界把自己的机器人都画到缓冲上;
         g_GameWorld->Render();

// 画完,通知 gdi 结束;
         gdi->StopDrawing(hdcBackBuffer);

        

         //now blit backbuffer to front
// 把后备缓冲整块复制(SRCCOPY 普通复制)到屏幕——双缓冲防闪烁;
			   BitBlt(ps.hdc, 0, 0, cxClient, cyClient, hdcBackBuffer, 0, 0, SRCCOPY); 
          
// 结束绘图(归还 DC)。
         EndPaint (hwnd, &ps);

      }

      break;


          
// 处理"窗口销毁"消息(窗口要被关掉时):
		 case WM_DESTROY:
			 {

         //clean up our backbuffer objects
// 还原旧位图(换掉我们的画布);
         SelectObject(hdcBackBuffer, hOldBitmap);

// 释放内存画布;
         DeleteDC(hdcBackBuffer);
// 释放位图对象(全部配对创建时的调用,防泄漏);
         DeleteObject(hBitmap); 

         
         
         // kill the application, this sends a WM_QUIT message  
// 投递 WM_QUIT 消息:主循环收到它就会退出(bDone = true)。
				 PostQuitMessage (0);
			 }

       break;

     }//end switch

     //this is where all the messages not specifically handled by our 
		 //winproc are sent to be processed
// 没处理的消息交给系统默认处理函数 DefWindowProc(标准收尾)。
		 return DefWindowProc (hwnd, msg, wParam, lParam);
}

//--------------------------------------------------------------------------------
//【WinMain:Windows 程序的入口函数】
//  相当于控制台程序的 main。流程:填窗口类 → 注册 → 创建窗口 → 显示 →
//  进入消息循环(每帧更新世界 + 触发重绘),直到收到退出消息。
//--------------------------------------------------------------------------------
//-------------------------------- WinMain -------------------------------
//
//	The entry point of the windows program
//------------------------------------------------------------------------
// WinMain 函数定义开始。四个参数:
//   HINSTANCE hInstance —— 本程序的实例句柄;
//   HINSTANCE hPrevInstance —— 上一个实例(已废弃,总为 NULL);
//   LPSTR szCmdLine —— 命令行参数;
//   int iCmdShow —— 窗口显示方式(最大化/最小化等)。
// int WINAPI:返回整数,固定调用约定。
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     szCmdLine, 
                    int       iCmdShow)
{
  //handle to our window
// 窗口句柄(创建成功后保存);
  HWND						hWnd;
    
  //our window class structure
// 窗口类结构 WNDCLASSEX:描述窗口的"模样和行为"(回调函数、图标、光标等);
  WNDCLASSEX     winclass;
		 
  // first fill in the window class stucture
// 结构大小(必须填,sizeof 求字节数);
  winclass.cbSize        = sizeof(WNDCLASSEX);
// 样式:CS_HREDRAW | CS_VREDRAW = 尺寸变化时重绘窗口(| 按位或,合并两个标志);
  winclass.style         = CS_HREDRAW | CS_VREDRAW;
// 窗口消息处理函数 = 上面的 WindowProc(所有消息都送到它);
  winclass.lpfnWndProc   = WindowProc;
// 类附加空间:0(不用);
  winclass.cbClsExtra    = 0;
// 窗口附加空间:0;
  winclass.cbWndExtra    = 0;
// 实例句柄;
  winclass.hInstance     = hInstance;
// 图标:系统默认应用程序图标;
  winclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
// 光标:系统默认箭头;
  winclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
// 背景刷:NULL(我们每帧自己画背景);
  winclass.hbrBackground = NULL;
// 菜单:用资源里的菜单 IDR_MENU1(MAKEINTRESOURCE 把数字 ID 转字符串指针);
  winclass.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
// 窗口类名:g_szWindowClassName;
  winclass.lpszClassName = g_szWindowClassName;
// 小图标:同大图标。
  winclass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

  //register the window class
// 注册窗口类:失败返回 0;
  if (!RegisterClassEx(&winclass))
// 注册失败:弹出错误框;
  {
    MessageBox(NULL, "Registration Failed!", "Error", 0);

    //exit the application
// 返回 0 结束程序。
    return 0;
  }

  //create the window and assign its ID to hwnd    
// 创建窗口:参数依次是扩展样式、类名、标题、样式(普通+可见+标题栏+系统菜单)、
// 初始位置(屏幕中央,用 GetSystemMetrics 取屏幕尺寸)、宽高(用 constants.h)、
// 父窗口/菜单/实例/参数(全 NULL)。成功返回窗口句柄。
  hWnd = CreateWindowEx (NULL,                 // extended style
                         g_szWindowClassName,  // window class name
                         g_szApplicationName,  // window caption
                         WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
                         GetSystemMetrics(SM_CXSCREEN)/2 - constWindowWidth/2,
                         GetSystemMetrics(SM_CYSCREEN)/2 - constWindowHeight/2,                    
                         constWindowWidth,     // initial x size
                         constWindowHeight,    // initial y size
                         NULL,                 // parent window handle
                         NULL,                 // window menu handle
                         hInstance,            // program instance handle
                         NULL);                // creation parameters

  //make sure the window creation has gone OK
// 创建失败(句柄为空):弹错误框。
  if(!hWnd)
  {
    MessageBox(NULL, "CreateWindowEx Failed!", "Error!", 0);
  }

     
  //make the window visible
// 显示窗口(按系统给的方式);
  ShowWindow (hWnd, iCmdShow);
// 立即刷新窗口一次(让画面先出现)。
  UpdateWindow (hWnd);

  // Enter the message loop
// 主循环退出标志:false = 继续跑;
  bool bDone = false;

  //create a timer
// 创建高精度计时器(Common 目录):量每帧耗时;
  PrecisionTimer timer;

// 开启"平滑更新":计时器返回平滑后的帧时间,避免帧率抖动;
  timer.SmoothUpdatesOn();

  //start the timer
// 计时器开始计时。
  timer.Start();

// 消息结构(每次循环从这里取消息)。
  MSG msg;

// while 主循环:bDone 变 true(收到退出)才停;
  while(!bDone)
  {		
// PeekMessage:有消息就取走(PM_REMOVE),没消息立即返回 0——
// 这是"游戏循环"的标准写法:消息处理与游戏更新交错进行;
    while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
    {
// 取到的是退出消息(WM_QUIT):
      if( msg.message == WM_QUIT) 
      {
        //stop loop if it's a quit message
// 置退出标志,跳出内层循环;
	      bDone = true;
      } 

      else 
// 否则:TranslateMessage 翻译消息(键盘),DispatchMessage 分发给 WindowProc;
      {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
      }
    }

// 不是退出消息:执行游戏步进——
    if (msg.message != WM_QUIT )
    {
      //update
// ★ 更新世界:把"距上一帧的时间"交给世界,世界驱动所有机器人运动;
      g_GameWorld->Update(timer.TimeElapsed());
      
      //render
// 请求重绘窗口(触发 WM_PAINT → Render);第二个参数 false 表示不擦背景;
      RedrawWindow(hWnd, false);

// 睡 2 毫秒:让出 CPU,避免空转烧满一个核心(游戏循环常见节流)。
      Sleep(2);
    }
   					
  }//end while




// 退出循环后:释放整个世界(所有机器人的析构在这里触发);
  delete g_GameWorld;

// 注销窗口类(与 RegisterClassEx 配对);
  UnregisterClass( g_szWindowClassName, winclass.hInstance );

// 返回退出消息的参数(程序退出码)。
  return msg.wParam;
}


