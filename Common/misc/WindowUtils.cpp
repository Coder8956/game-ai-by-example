//==============================================================================================
//【文件说明】WindowUtils.cpp —— 窗口小工具的实现
//  ChangeMenuState/CheckMenuItemAppropriately:菜单勾选状态;
//  ErrorBox:弹错误框;GetClientCursorPosition:鼠标坐标转 Vector2D;
//  FileOpenDlg/FileSaveDlg:标准打开/保存文件对话框;
//  ResizeWindow:按客户区大小反推窗口大小并居中;GetWindowHeight/Width:读窗口尺寸。
//==============================================================================================
#include "misc/WindowUtils.h"
#include <windows.h>
#include "2d/Vector2D.h"
#include "misc/utils.h"
#include "misc/Stream_Utility_Functions.h"



//---------------------- ChangeMenuState ---------------------------------
//
//  Changes the state of a menu item given the item identifier, the 
//  desired state and the HWND of the menu owner
//------------------------------------------------------------------------
  // 用 MENUITEMINFO 改菜单项状态并重绘菜单栏。
void ChangeMenuState(HWND hwnd, UINT MenuItem, UINT state)
{
  MENUITEMINFO mi;

  mi.cbSize = sizeof(MENUITEMINFO);
  mi.fMask  = MIIM_STATE;
  mi.fState = state;

  SetMenuItemInfo(GetMenu(hwnd), MenuItem, false, &mi);
  DrawMenuBar(hwnd);
}

//-------------------- CheckMenuItemAppropriately ----------------------------
//
//  if b is true MenuItem is checked, otherwise it is unchecked
//-----------------------------------------------------------------------------
void CheckMenuItemAppropriately(HWND hwnd, UINT MenuItem, bool b)
{
  if (b)
  {
    ChangeMenuState(hwnd, MenuItem, MFS_CHECKED);
  }
  else
  {
    ChangeMenuState(hwnd, MenuItem, MFS_UNCHECKED);
  }
}


//--------------------- CheckBufferLength --------------------------------
//
//  this is a replacement for the StringCchLength function found in the 
//  platform SDK. See MSDN for details. Only ever used for checking toolbar
//  strings
//------------------------------------------------------------------------
  // 检查字符串长度是否超 MaxLength(工具栏字符串用)。
bool CheckBufferLength(char* buff, int MaxLength, int& BufferLength)
{
  std::string s = ttos(buff);

  BufferLength = s.length();

  if (BufferLength > MaxLength)
  {
    BufferLength = 0; return false;
  }  

  return true;
}

  // 两版 ErrorBox:MessageBox 弹窗。
void ErrorBox(std::string& msg)
{
  MessageBox(NULL, msg.c_str(), "Error", MB_OK);
}

void ErrorBox(char* msg)
{
  MessageBox(NULL, msg, "Error", MB_OK);
}

//gets the coordinates of the cursor relative to an active window 
  // 取鼠标屏幕坐标并换算成客户区坐标,再转成 Vector2D。
Vector2D GetClientCursorPosition()
{
  POINT MousePos;

  GetCursorPos(&MousePos);

  ScreenToClient(GetActiveWindow(), &MousePos);

  return POINTtoVector(MousePos);
}

 
Vector2D GetClientCursorPosition(HWND hwnd)
{
  POINT MousePos;

  GetCursorPos(&MousePos);

  ScreenToClient(hwnd, &MousePos);

  return POINTtoVector(MousePos);
}


//-----------------------------------------------------------------------------
//
//  The following 3 functions are taken from Petzold's book and enable the 
//  client to use the file dialog common control
//-----------------------------------------------------------------------------
  // 初始化 OPENFILENAME 结构(过滤串、默认扩展名等);FileOpenDlg/FileSaveDlg 调它后弹标准对话框。
void FileInitialize (HWND hwnd,
                     OPENFILENAME& ofn,
                     const std::string& defaultFileTypeDescription,
                     const std::string& defaultFileExtension)
{
  std::string filter = defaultFileTypeDescription + '\0' + "*." + defaultFileExtension + '\0' +
                       "All Files (*.*)" + '\0' + "*.*" + '\0' + '\0';
  
   static TCHAR szFilter[255];

   for (unsigned int i=0; i<filter.size(); ++i)
   {
     szFilter[i] = filter.at(i);
   }
     
     ofn.lStructSize       = sizeof (OPENFILENAME) ;
     ofn.hwndOwner         = hwnd ;
     ofn.hInstance         = NULL ;
     ofn.lpstrFilter       = szFilter ;
     ofn.lpstrCustomFilter = NULL ;
     ofn.nMaxCustFilter    = 0 ;
     ofn.nFilterIndex      = 0 ;
     ofn.lpstrFile         = NULL ;          // Set in Open and Close functions
     ofn.nMaxFile          = MAX_PATH ;
     ofn.lpstrFileTitle    = NULL ;          // Set in Open and Close functions
     ofn.nMaxFileTitle     = MAX_PATH ;
     ofn.lpstrInitialDir   = NULL ;
     ofn.lpstrTitle        = NULL ;
     ofn.Flags             = 0 ;             // Set in Open and Close functions
     ofn.nFileOffset       = 0 ;
     ofn.nFileExtension    = 0 ;
     ofn.lpstrDefExt       = defaultFileExtension.c_str() ;
     ofn.lCustData         = 0L ;
     ofn.lpfnHook          = NULL ;
     ofn.lpTemplateName    = NULL ;

}



BOOL FileOpenDlg (HWND               hwnd,
                  PTSTR              pstrFileName,
                  PTSTR              pstrTitleName,
                  const std::string& defaultFileTypeDescription,
                  const std::string& defaultFileExtension)
{
     OPENFILENAME ofn;

     FileInitialize(hwnd, ofn, defaultFileTypeDescription, defaultFileExtension);
  
     ofn.hwndOwner         = hwnd ;
     ofn.lpstrFile         = pstrFileName ;
     ofn.lpstrFileTitle    = pstrTitleName ;
     ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT ;
     
     return GetOpenFileName (&ofn) ;
}

BOOL FileSaveDlg (HWND               hwnd,
                  PTSTR              pstrFileName,
                  PTSTR              pstrTitleName,
                  const std::string& defaultFileTypeDescription,
                  const std::string& defaultFileExtension)
{
     OPENFILENAME ofn; FileInitialize(hwnd, ofn, defaultFileTypeDescription, defaultFileExtension);

     ofn.hwndOwner         = hwnd ;
     ofn.lpstrFile         = pstrFileName ;
     ofn.lpstrFileTitle    = pstrTitleName ;
     ofn.Flags             = OFN_OVERWRITEPROMPT ;
     
     return GetSaveFileName (&ofn) ;
}

//-------------------------- ResizeWindow -------------------------------------
//
//  call this to resize the active window to the specified size
//-----------------------------------------------------------------------------
  // 按期望客户区大小 AdjustWindowRectEx 反推整窗尺寸,SetWindowPos 居中放置。
void ResizeWindow(HWND hwnd, int cx, int cy)
{
  //does this window have a menu. If so set a flag to true
  HMENU hwndMenu = GetMenu(hwnd);
  bool bMenu = false;
  if (hwndMenu) bMenu = true;

  //create a rect of the desired window size
  RECT DesiredSize;
  DesiredSize.left = 0;
  DesiredSize.top  = 0;
  DesiredSize.right = cx;
  DesiredSize.bottom = cy;

  //determine the size the window should be given the desired client area
  AdjustWindowRectEx(&DesiredSize,
                     WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
                     bMenu,
                     NULL);

  //resize the window to fit
  SetWindowPos(hwnd,
               NULL,
               GetSystemMetrics(SM_CXSCREEN)/2 - cx/2,
               GetSystemMetrics(SM_CYSCREEN)/2 - cy/2,
               DesiredSize.right,
               DesiredSize.bottom,
               SWP_NOZORDER);
}

//------------------------- GetWindowHeight -----------------------------------
//-----------------------------------------------------------------------------
  // 用 GetWindowRect 取矩形,bottom-top / right-left 得高/宽。
int  GetWindowHeight(HWND hwnd)
{
  if (hwnd == 0) return 0;

  RECT windowRect;

  GetWindowRect(hwnd, &windowRect);

  return windowRect.bottom - windowRect.top;
}

//------------------------- GetWindowWidth  -----------------------------------
//-----------------------------------------------------------------------------
int  GetWindowWidth(HWND hwnd)
{
  if (hwnd == 0) return 0;

  RECT windowRect;

  GetWindowRect(hwnd, &windowRect);

  return windowRect.right - windowRect.left;
}