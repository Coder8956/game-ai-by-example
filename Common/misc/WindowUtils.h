//==============================================================================================
//【文件说明】WindowUtils.h —— Windows 窗口/菜单/文件对话框/鼠标光标小工具
//
//【这个文件是干什么的?】
//  一组 Win32 API 的便捷封装:键盘状态宏、刷新窗口、菜单勾选、鼠标坐标转 Vector2D、
//  打开/保存文件对话框、窗口大小查询。
//
//【谁在使用这个文件?】
//  各工程 WinMain/消息处理过程(处理按键、菜单、文件对话框)。
//==============================================================================================
#ifndef WINDOW_UTILS_H
#define WINDOW_UTILS_H
//--------------------------------------------------------------------------------
// #pragma warning(disable:4786) 原理详见 SoccerPitch.h;包含保护原理详见 Goal.h。
//--------------------------------------------------------------------------------
#pragma warning (disable:4786)

#include <windows.h>
#include <string>

struct Vector2D;

  // KEYDOWN/WAS_KEY_PRESSED/IS_KEY_PRESSED:宏,检测某虚拟键当前是否按下。
//macro to detect keypresses
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

#define WAS_KEY_PRESSED(vk_code) ((GetKeyState(vk_code) & 0x8000) != 0)
#define IS_KEY_PRESSED(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) != 0)

//Call this to refresh the client window
  // RedrawWindow:调 InvalidateRect+UpdateWindow 强制重绘客户区。
inline void RedrawWindow(HWND hwnd, bool RedrawBackGround = true)
{
  InvalidateRect(hwnd, NULL, RedrawBackGround);
  UpdateWindow(hwnd);
}

//Call this to refresh the client window
inline void RedrawWindowRect(HWND hwnd, bool RedrawBackGround, RECT& RedrawArea)
{
  InvalidateRect(hwnd, &RedrawArea, RedrawBackGround);
  UpdateWindow(hwnd);
}


//Changes the state of a menu item given the item identifier, the 
//desired state and the HWND of the menu owner
  // 菜单/错误框/鼠标/文件对话框/窗口尺寸:后面 cpp 实现,这里只声明。
void ChangeMenuState(HWND hwnd, UINT MenuItem, UINT state);

//if b is true MenuItem is checked, otherwise it is unchecked
void CheckMenuItemAppropriately(HWND hwnd, UINT MenuItem, bool b);


//this is a replacement for the StringCchLength function found in the 
//platform SDK. See MSDN for details. Only ever used for checking toolbar
//strings
bool CheckBufferLength(char* buff, int MaxLength, unsigned int& BufferLength);

void ErrorBox(std::string& msg);
void ErrorBox(char* msg);

//gets the coordinates of the cursor relative to an active window 
Vector2D GetClientCursorPosition();

//gets the coordinates of the cursor relative to an active window 
Vector2D GetClientCursorPosition(HWND hwnd);


//two handy functions from Mr Petzold. They open a common dialog box to
//grab a filename
void FileInitialize (HWND hwnd,
                     OPENFILENAME& ofn,
                     const std::string& defaultFileTypeDescription,
                     const std::string& defaultFileExtension);

BOOL FileOpenDlg (HWND               hwnd,
                  PTSTR              pstrFileName,
                  PTSTR              pstrTitleName,
                  const std::string& defaultFileTypeDescription,
                  const std::string& defaultFileExtension);

BOOL FileSaveDlg (HWND hwnd,
                  PTSTR pstrFileName,
                  PTSTR pstrTitleName,
                  const std::string& defaultFileTypeDescription,
                  const std::string& defaultFileExtension);

//call this to resize the specified window to the specified size.
void ResizeWindow(HWND hwnd, int cx, int cy);

int  GetWindowHeight(HWND hwnd);
int  GetWindowWidth(HWND hwnd);




#endif