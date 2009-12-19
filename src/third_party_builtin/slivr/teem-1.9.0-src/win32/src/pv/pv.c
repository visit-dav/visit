/*
 * COPYRIGHT NOTICE, DISCLAIMER, and LICENSE:
 *
 * If you modify libpng you may insert additional notices immediately following
 * this sentence.
 *
 * Copyright 2002, Milan Ikits
 * Copyright 2000, Willem van Schaik
 *
 * libpng versions 1.0.7, July 1, 2000, through 1.2.5, October 3, 2002, are
 * Copyright (c) 2000-2002 Glenn Randers-Pehrson, and are
 * distributed according to the same disclaimer and license as libpng-1.0.6
 * with the following individuals added to the list of Contributing Authors
 *
 *    Simon-Pierre Cadieux
 *    Eric S. Raymond
 *    Gilles Vollant
 *
 * and with the following additions to the disclaimer:
 *
 *    There is no warranty against interference with your enjoyment of the
 *    library or against infringement.  There is no warranty that our
 *    efforts or the library will fulfill any of your particular purposes
 *    or needs.  This library is provided with all faults, and the entire
 *    risk of satisfactory quality, performance, accuracy, and effort is with
 *    the user.
 *
 * libpng versions 0.97, January 1998, through 1.0.6, March 20, 2000, are
 * Copyright (c) 1998, 1999, 2000 Glenn Randers-Pehrson
 * Distributed according to the same disclaimer and license as libpng-0.96,
 * with the following individuals added to the list of Contributing Authors:
 *
 *    Tom Lane
 *    Glenn Randers-Pehrson
 *    Willem van Schaik
 *
 * libpng versions 0.89, June 1996, through 0.96, May 1997, are
 * Copyright (c) 1996, 1997 Andreas Dilger
 * Distributed according to the same disclaimer and license as libpng-0.88,
 * with the following individuals added to the list of Contributing Authors:
 *
 *    John Bowler
 *    Kevin Bracey
 *    Sam Bushell
 *    Magnus Holmgren
 *    Greg Roelofs
 *    Tom Tanner
 *
 * libpng versions 0.5, May 1995, through 0.88, January 1996, are
 * Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * For the purposes of this copyright and license, "Contributing Authors"
 * is defined as the following set of individuals:
 *
 *    Andreas Dilger
 *    Dave Martindale
 *    Guy Eric Schalnat
 *    Paul Schmidt
 *    Tim Wegner
 *
 * The PNG Reference Library is supplied "AS IS".  The Contributing Authors
 * and Group 42, Inc. disclaim all warranties, expressed or implied,
 * including, without limitation, the warranties of merchantability and of
 * fitness for any purpose.  The Contributing Authors and Group 42, Inc.
 * assume no liability for direct, indirect, incidental, special, exemplary,
 * or consequential damages, which may result from the use of the PNG
 * Reference Library, even if advised of the possibility of such damage.
 *
 * Permission is hereby granted to use, copy, modify, and distribute this
 * source code, or portions hereof, for any purpose, without fee, subject
 * to the following restrictions:
 *
 * 1. The origin of this source code must not be misrepresented.
 *
 * 2. Altered versions must be plainly marked as such and
 * must not be misrepresented as being the original source.
 *
 * 3. This Copyright notice may not be removed or altered from
 *    any source or altered source distribution.
 *
 * The Contributing Authors and Group 42, Inc. specifically permit, without
 * fee, and encourage the use of this source code as a component to
 * supporting the PNG file format in commercial products.  If you use this
 * source code in a product, acknowledgment is not required but would be
 * appreciated.
 */

#define PROGNAME "pv"
#define LONGNAME "PNG Image Viewer"
#define VERSION "1.0"
#define MARGIN 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <commdlg.h>
#include <pngio.h>
#include <resource.h>

// app
static char *szAppName = LONGNAME;
static char *szProgName = PROGNAME;
static char *szIconName = PROGNAME;
// file
static TCHAR szImgFileName[MAX_PATH] = "";
static char szWindowTitle[MAX_PATH];
// window
static int cWinSizeX, cWinSizeY;
static int cBorderX, cBorderY;
static int cMousePosX, cMousePosY;
static int cMouseMiddle = FALSE;
// image
static BYTE* pImage = NULL;
static int cImgSizeX, cImgSizeY;
static int cImgChannels;
static float cImgScale = 1.0;
static char cBackGround[3] = {127, 127, 127};
static int cPickColor[4];
static int cImgPosX, cImgPosY;
static int cPickPosX, cPickPosY;
// bitmap
static BYTE* pBitmapStore = NULL;
static BITMAPINFOHEADER* pBitmapInfo;
static BYTE* pBitmap = NULL;
static int cBitmapMaxX, cBitmapMaxY;
static int cBitmapSize;
// file search
static TCHAR *pFileList = NULL;
static int iFileCount;
static int iFileIndex;
static OPENFILENAME ofn;
static TCHAR szFilter[] = TEXT("PNG Files (*.PNG)\0*.png\0")
     TEXT("All Files (*.*)\0*.*\0\0");

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
void SetWindowTitle (HWND hwnd);
void SetPickTitle (HWND hwnd);
BOOL CreateBitmapStore ();
void DestroyBitmapStore ();
void FillBitmap (BYTE *pBitmap, BITMAPINFOHEADER *pBitmapInfo, int cWinSizeX, int cWinSizeY,
		 BYTE *pImg, int cImgSizeX, int cImgSizeY, int cImgChannels);
BOOL BuildFileList (PTSTR pPathName, TCHAR **ppFileList,
		    int *pFileCount, int *pFileIndex);
BOOL SearchFileList (TCHAR *pFileList, int FileCount, int *pFileIndex,
		     PTSTR pstrPrevName, PTSTR pstrNextName);
BOOL CALLBACK AboutProc (HWND hwnd, UINT message,
			 WPARAM wParam, LPARAM lParam);
BOOL CenterWindow (HWND hwndChild, HWND hwndParent);
BOOL ParseArg (char* arg);

// program entry
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
  HACCEL hAccel;
  HWND hwnd;
  MSG msg;
  WNDCLASS wndclass;
  int wSizeX=640, wSizeY=480;

  // register window class
  wndclass.style = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc = WndProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = hInstance;
  wndclass.hIcon = LoadIcon (hInstance, szIconName) ;
  wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
  wndclass.hbrBackground = NULL; // (HBRUSH) GetStockObject (GRAY_BRUSH);
  wndclass.lpszMenuName = szProgName;
  wndclass.lpszClassName = szProgName;
  if (!RegisterClass(&wndclass))
  {
    MessageBox(NULL, TEXT ("Fatal Error: pv requires the win32 api"), szAppName, MB_ICONERROR);
    return 0;
  }
  // parse command line options
  if (ParseArg(szCmdLine))
  {
    MessageBox(NULL, TEXT("Usage: pv [-s <scale>] [<file>]"), szAppName, MB_ICONERROR);
    return 0;
  }
  if (CreateBitmapStore())
  {
    MessageBox(NULL, TEXT ("Fatal Error: Failed to allocate memory"), szAppName, MB_ICONERROR);
    return 0;
  }
  // calculate size of window-borders
  cBorderX = 2*(GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXDLGFRAME));
  cBorderY = 2*(GetSystemMetrics(SM_CYBORDER) + GetSystemMetrics(SM_CYDLGFRAME)) +
    GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENUSIZE) + 1; /* WvS: don't ask me why? */
  // in case we process file given on command-line
  if (szImgFileName[0] != '\0')
  {
    // load the image from file
    if (LoadImagePNG(szImgFileName, &pImage, &cImgSizeX, &cImgSizeY, &cImgChannels))
    {
      sprintf(szWindowTitle, "Failed to load image: %s", szImgFileName);
      MessageBox(NULL, TEXT(szWindowTitle), szAppName, MB_ICONEXCLAMATION | MB_OK);
      UnregisterClass(wndclass.lpszClassName, hInstance);
      DestroyBitmapStore();
      return 0;
    }
    BuildFileList(szImgFileName, &pFileList, &iFileCount, &iFileIndex);
    wSizeX = (int)(cImgSizeX*cImgScale);
    wSizeY = (int)(cImgSizeY*cImgScale);
    if (wSizeX > cBitmapMaxX - 2*MARGIN - cBorderX) wSizeX = cBitmapMaxX - 2*MARGIN - cBorderX;
    if (wSizeX > cBitmapMaxY - 2*MARGIN - cBorderY) wSizeY = cBitmapMaxY - 2*MARGIN - cBorderY;
  }
  else
  {
    pImage = NULL;
  }
  // create window
  hwnd = CreateWindow(szProgName, szAppName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		      wSizeX + 2*MARGIN + cBorderX, wSizeY + 2*MARGIN + cBorderY,
		      NULL, NULL, hInstance, NULL);
  ShowWindow(hwnd, iCmdShow);
  UpdateWindow(hwnd);
  // initialize open filename struct
  ofn.lStructSize = sizeof (OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.hInstance = NULL;
  ofn.lpstrFilter = szFilter;
  ofn.lpstrCustomFilter = NULL;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = NULL; // set in Open and Close functions
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrFileTitle = NULL; // set in Open and Close functions
  ofn.nMaxFileTitle = MAX_PATH;
  ofn.lpstrInitialDir = NULL;
  ofn.lpstrTitle = NULL;
  ofn.Flags = 0; // set in Open and Close functions
  ofn.nFileOffset = 0;
  ofn.nFileExtension = 0;
  ofn.lpstrDefExt = TEXT ("png");
  ofn.lCustData = 0;
  ofn.lpfnHook = NULL;
  ofn.lpTemplateName = NULL;
  // load shortcuts
  hAccel = LoadAccelerators(hInstance, szProgName);
  // message loop
  while (GetMessage(&msg, NULL, 0, 0))
  {
    if (!TranslateAccelerator(hwnd, hAccel, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  // destroy accelerator table
  DestroyAcceleratorTable(hAccel);
  // unregister window class
  UnregisterClass(wndclass.lpszClassName, hInstance);
  // destroy bitmap store
  DestroyBitmapStore();
  // free file-list
  if (pFileList != NULL)
    free(pFileList);
  // exit
  return msg.wParam;
}

// the window procedure
LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static HINSTANCE hInstance;
  static HDC hdc;
  static PAINTSTRUCT ps;
  static HMENU hMenu;

  switch (message)
  {
    case WM_CREATE:
    hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
    // read the other png-files in the directory for later next/previous commands
    BuildFileList(szImgFileName, &pFileList, &iFileCount, &iFileIndex);
    SetWindowTitle(hwnd);
    return 0;

    case WM_SIZE:
    cWinSizeX = LOWORD(lParam);
    cWinSizeY = HIWORD(lParam);    
    // invalidate the client area for later update
    InvalidateRect(hwnd, NULL, TRUE);
    // fill bitmap store
    FillBitmap(pBitmap, pBitmapInfo, cWinSizeX, cWinSizeY,
	       pImage, cImgSizeX, cImgSizeY, cImgChannels);
    SetWindowTitle(hwnd);
    return 0;

    case WM_PAINT:
    hdc = BeginPaint(hwnd, &ps);
    SetDIBitsToDevice(hdc, 0, 0, cWinSizeX, cWinSizeY, 0, 0,
		      0, cWinSizeY, pBitmap, (BITMAPINFO*)pBitmapInfo, DIB_RGB_COLORS);
    EndPaint(hwnd, &ps);
    return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    {
      if (pImage)
      {
	BYTE* p;
	cMouseMiddle = TRUE;
	cMousePosX = LOWORD(lParam);
	cMousePosY = HIWORD(lParam);
	cPickPosX = cImgSizeX*(cMousePosX - cImgPosX)/(cWinSizeX-2*cImgPosX);
	if (cPickPosX < 0) cPickPosX = 0;
	if (cPickPosX > cImgSizeX-1) cPickPosX = cImgSizeX-1;
	if (cPickPosY < 0) cPickPosY = 0;
	if (cPickPosY > cImgSizeY-1) cPickPosY = cImgSizeY-1;
	cPickPosY = cImgSizeY*(cMousePosY - cImgPosY)/(cWinSizeY-2*cImgPosY);
	p = pImage + (cPickPosY*cImgSizeX+cPickPosX)*cImgChannels;
	cPickColor[0] = p[0];
	cPickColor[1] = p[1];
	cPickColor[2] = p[2];
	cPickColor[3] = cImgChannels == 4 ? p[3] : -1;
	SetPickTitle(hwnd);
      }
    }
    return 0;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    {
      cMouseMiddle = FALSE;
      SetWindowTitle(hwnd);
    }
    return 0;

    case WM_MOUSEMOVE:
    {
      if (cMouseMiddle)
      {
	if (pImage)
	{
	  BYTE* p;
	  cMousePosX = LOWORD(lParam);
	  cMousePosY = HIWORD(lParam);
	  cPickPosX = cImgSizeX*(cMousePosX - cImgPosX)/(cWinSizeX-2*cImgPosX);
	  cPickPosY = cImgSizeY*(cMousePosY - cImgPosY)/(cWinSizeY-2*cImgPosY);
	  if (cPickPosX < 0) cPickPosX = 0;
	  if (cPickPosX > cImgSizeX-1) cPickPosX = cImgSizeX-1;
	  if (cPickPosY < 0) cPickPosY = 0;
	  if (cPickPosY > cImgSizeY-1) cPickPosY = cImgSizeY-1;
	  SetPickTitle(hwnd);
	  p = pImage + (cPickPosY*cImgSizeX+cPickPosX)*cImgChannels;
	  cPickColor[0] = p[0];
	  cPickColor[1] = p[1];
	  cPickColor[2] = p[2];
	  cPickColor[3] = cImgChannels == 4 ? p[3] : -1;
	  SetPickTitle(hwnd);
	}
      }
    }
    return 0;

    case WM_INITMENUPOPUP:
    return 0;

    case WM_COMMAND:
    hMenu = GetMenu(hwnd);
    switch (LOWORD(wParam))
    {
      case IDM_FILE_OPEN:
      {
	// show the File Open dialog box
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szImgFileName;
	ofn.lpstrFileTitle = NULL; //szTitleName;
	ofn.Flags = OFN_HIDEREADONLY;
	if (!GetOpenFileName(&ofn)) return 0;
	// read the other png-files in the directory for later next/previous commands
 	BuildFileList(szImgFileName, &pFileList, &iFileCount, &iFileIndex);
	// load the image from file
	ShowCursor(FALSE); SetCursor(LoadCursor (NULL, IDC_WAIT)); ShowCursor(TRUE);
	if (LoadImagePNG(szImgFileName, &pImage, &cImgSizeX, &cImgSizeY, &cImgChannels))
	{
	  ShowCursor(FALSE); SetCursor(LoadCursor (NULL, IDC_ARROW)); ShowCursor(TRUE);
	  sprintf(szWindowTitle, "Failed to load image: %s", szImgFileName);
	  MessageBox(hwnd, TEXT(szWindowTitle), szAppName, MB_ICONEXCLAMATION | MB_OK);
	  SetWindowTitle(hwnd);
	  return 0;
	}
	ShowCursor(FALSE); SetCursor(LoadCursor (NULL, IDC_ARROW)); ShowCursor(TRUE);
	SetWindowTitle(hwnd);
	// invalidate the client area for later update
	InvalidateRect(hwnd, NULL, TRUE);
	// fill bitmap
	FillBitmap(pBitmap, pBitmapInfo, cWinSizeX, cWinSizeY,
		   pImage, cImgSizeX, cImgSizeY, cImgChannels);
      }
      return 0;

      case IDM_FILE_NEXT:
      // read next entry in the directory      
      if (SearchFileList(pFileList, iFileCount, &iFileIndex, NULL, szImgFileName))
      {
	if (!strcmp(szImgFileName, "")) return 0;
	// load the image from file
	ShowCursor(FALSE); SetCursor(LoadCursor (NULL, IDC_WAIT)); ShowCursor(TRUE);
	if (LoadImagePNG(szImgFileName, &pImage, &cImgSizeX, &cImgSizeY, &cImgChannels))
	{
	  ShowCursor(FALSE); SetCursor(LoadCursor (NULL, IDC_ARROW)); ShowCursor(TRUE);
	  sprintf(szWindowTitle, "Failed to load image: %s", szImgFileName);
	  MessageBox(hwnd, TEXT(szWindowTitle), szAppName, MB_ICONEXCLAMATION | MB_OK);
	  SetWindowTitle(hwnd);
	  return 0;
	}
	ShowCursor(FALSE); SetCursor(LoadCursor (NULL, IDC_ARROW)); ShowCursor(TRUE);
	SetWindowTitle(hwnd);
	// invalidate the client area for later update
	InvalidateRect(hwnd, NULL, TRUE);
	// fill bitmap
	FillBitmap(pBitmap, pBitmapInfo, cWinSizeX, cWinSizeY,
		   pImage, cImgSizeX, cImgSizeY, cImgChannels);
      }
      return 0;

      case IDM_FILE_PREVIOUS:
      // read previous entry in the directory
      if (SearchFileList(pFileList, iFileCount, &iFileIndex, szImgFileName, NULL))
      {
	if (!strcmp(szImgFileName, "")) return 0;
	// load the image from file
	ShowCursor(FALSE); SetCursor(LoadCursor (NULL, IDC_WAIT)); ShowCursor(TRUE);
	if (LoadImagePNG(szImgFileName, &pImage, &cImgSizeX, &cImgSizeY, &cImgChannels))
	{
	  ShowCursor(FALSE); SetCursor(LoadCursor (NULL, IDC_ARROW)); ShowCursor(TRUE);
	  sprintf(szWindowTitle, "Failed to load image: %s", szImgFileName);
	  MessageBox(hwnd, TEXT(szWindowTitle), szAppName, MB_ICONEXCLAMATION | MB_OK);
	  SetWindowTitle(hwnd);
	  return 0;
	}
	ShowCursor(FALSE); SetCursor(LoadCursor (NULL, IDC_ARROW)); ShowCursor(TRUE);
	SetWindowTitle(hwnd);
	// invalidate the client area for later update
	InvalidateRect(hwnd, NULL, TRUE);
	// fill bitmap
	FillBitmap(pBitmap, pBitmapInfo, cWinSizeX, cWinSizeY,
		   pImage, cImgSizeX, cImgSizeY, cImgChannels);
      }
      return 0;

      case IDM_FILE_EXIT:
      SendMessage(hwnd, WM_CLOSE, 0, 0);
      return 0;

      case IDM_OPTIONS_NORMAL:
      {
	RECT rect;
	int sizeX, sizeY;
	GetWindowRect(hwnd, &rect);
	cImgScale = 1.0;
	sizeX = (int)(cImgSizeX*cImgScale) + rect.right-rect.left-cWinSizeX;
	sizeY = (int)(cImgSizeY*cImgScale) + rect.bottom-rect.top-cWinSizeY;
	if (sizeX > cBitmapMaxX) sizeX = cBitmapMaxX;
	if (sizeY > cBitmapMaxY) sizeY = cBitmapMaxY;
	if (rect.left + sizeX > cBitmapMaxX) rect.left = cBitmapMaxX-sizeX;
	if (rect.top + sizeY > cBitmapMaxY) rect.top = cBitmapMaxY-sizeY;
	MoveWindow(hwnd, rect.left, rect.top, sizeX, sizeY, TRUE);
      }
      return 0;

      case IDM_OPTIONS_MAXPECT:
      MoveWindow(hwnd, 0, 0, cBitmapMaxX, cBitmapMaxY, TRUE);
      return 0;

      case IDM_OPTIONS_DOUBLE:
      {
	RECT rect;
	int sizeX, sizeY;
	GetWindowRect(hwnd, &rect);
	cImgScale *= 2.0f;
	sizeX = (int)(cImgSizeX*cImgScale) + rect.right-rect.left-cWinSizeX;
	sizeY = (int)(cImgSizeY*cImgScale) + rect.bottom-rect.top-cWinSizeY;
	if (sizeX > cBitmapMaxX || sizeY > cBitmapMaxY) cImgScale *= 0.5f;
	if (sizeX > cBitmapMaxX) sizeX = cBitmapMaxX;
	if (sizeY > cBitmapMaxY) sizeY = cBitmapMaxY;
	if (rect.left + sizeX > cBitmapMaxX) rect.left = cBitmapMaxX - sizeX;
	if (rect.top + sizeY > cBitmapMaxY) rect.top = cBitmapMaxY - sizeY;
	MoveWindow(hwnd, rect.left, rect.top, sizeX, sizeY, TRUE);
      }
      return 0;

      case IDM_OPTIONS_HALF:
      {
	RECT rect;
	int sizeX, sizeY;
	GetWindowRect(hwnd, &rect);
	cImgScale *= 0.5f;
	sizeX = (int)(cImgSizeX*cImgScale) + rect.right-rect.left-cWinSizeX;
	sizeY = (int)(cImgSizeY*cImgScale) + rect.bottom-rect.top-cWinSizeY;
	if (rect.left + sizeX > cBitmapMaxX) rect.left = cBitmapMaxX - sizeX;
	if (rect.top + sizeY > cBitmapMaxY) rect.top = cBitmapMaxY - sizeY;
	MoveWindow(hwnd, rect.left, rect.top, sizeX, sizeY, TRUE);
      }
      return 0;

      case IDM_OPTIONS_LARGER:
      {
	RECT rect;
	int sizeX, sizeY;
	GetWindowRect(hwnd, &rect);
	cImgScale *= 1.1f;
	sizeX = (int)(cImgSizeX*cImgScale) + rect.right-rect.left-cWinSizeX;
	sizeY = (int)(cImgSizeY*cImgScale) + rect.bottom-rect.top-cWinSizeY;
	if (sizeX > cBitmapMaxX || sizeY > cBitmapMaxY) cImgScale /= 1.1f;
	if (sizeX > cBitmapMaxX) sizeX = cBitmapMaxX;
	if (sizeY > cBitmapMaxY) sizeY = cBitmapMaxY;
	if (rect.left + sizeX > cBitmapMaxX) rect.left = cBitmapMaxX - sizeX;
	if (rect.top + sizeY > cBitmapMaxY) rect.top = cBitmapMaxY - sizeY;
	MoveWindow(hwnd, rect.left, rect.top, sizeX, sizeY, TRUE);
      }
      return 0;

      case IDM_OPTIONS_SMALLER:
      {
	RECT rect;
	int sizeX, sizeY;
	GetWindowRect(hwnd, &rect);
	cImgScale *= 0.9f;
	sizeX = (int)(cImgSizeX*cImgScale) + rect.right-rect.left-cWinSizeX;
	sizeY = (int)(cImgSizeY*cImgScale) + rect.bottom-rect.top-cWinSizeY;
	if (rect.left + sizeX > cBitmapMaxX) rect.left = cBitmapMaxX - sizeX;
	if (rect.top + sizeY > cBitmapMaxY) rect.top = cBitmapMaxY - sizeY;
	MoveWindow(hwnd, rect.left, rect.top, sizeX, sizeY, TRUE);
      }
      return 0;

      case IDM_HELP_ABOUT:
      DialogBox(hInstance, TEXT("About"), hwnd, AboutProc);
      return 0;

    } // end switch
    break;

    case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }

  return DefWindowProc(hwnd, message, wParam, lParam);
}

void SetWindowTitle (HWND hwnd)
{
  if (pImage)
  {
    sprintf(szWindowTitle, " %s (%d x %d @ %.3f)", 
	    szImgFileName, cImgSizeX, cImgSizeY, cImgScale);
  }
  else
  {
    sprintf(szWindowTitle, " %s", szAppName);
  }
  SetWindowText(hwnd, szWindowTitle);
}

void SetPickTitle (HWND hwnd)
{
  if (pImage)
  {
    switch (cImgChannels)
    {
      case 3:
      {
	sprintf(szWindowTitle, " XY: (%d, %d) - RGB: (%d, %d, %d)", 
		cPickPosX, cPickPosY, cPickColor[0], cPickColor[1], cPickColor[2]);
	SetWindowText(hwnd, szWindowTitle);
      }
      break;
      case 4:
      {
	sprintf(szWindowTitle, " XY: (%d, %d) - RGBA: (%d, %d, %d, %d)", 
		cPickPosX, cPickPosY, cPickColor[0], cPickColor[1], cPickColor[2], cPickColor[3]);
	SetWindowText(hwnd, szWindowTitle);
      }
      default:
      break;
    }      
  }
}

BOOL CreateBitmapStore ()
{ 
  int rowsize;
  // the largest displayed image will not be bigger than the screen
  cBitmapMaxX = GetSystemMetrics(SM_CXSCREEN);
  cBitmapMaxY = GetSystemMetrics(SM_CYSCREEN);
  // adjust for word alignment
  rowsize = ((3*cBitmapMaxX + 3) >> 2) << 2;
  cBitmapSize = rowsize*cBitmapMaxY*sizeof(BYTE);
  if ((pBitmapStore = (BYTE*)malloc(sizeof(BITMAPINFOHEADER) + cBitmapSize)) == NULL)
    return TRUE;
  pBitmap = &pBitmapStore[sizeof(BITMAPINFOHEADER)];
  pBitmapInfo = (BITMAPINFOHEADER*)pBitmapStore;
  // initialize the bitmapinfo struct
  memset(pBitmapInfo, 0, sizeof(BITMAPINFOHEADER));
  pBitmapInfo->biSize = sizeof(BITMAPINFOHEADER);
  pBitmapInfo->biPlanes = 1;
  pBitmapInfo->biBitCount = 24;
  return FALSE;
}

void DestroyBitmapStore ()
{ 
  free(pBitmapStore);
}

void FillBitmap (BYTE *pBitmap, BITMAPINFOHEADER *pBitmapInfo, int cWinSizeX, int cWinSizeY,
		 BYTE *pImg, int cImgSizeX, int cImgSizeY, int cImgChannels)
{
  int cSizeX, cSizeY;
  int x, y;
  int xImg, yImg;
  BYTE *p, *r, *s;
  WORD wImgRowSize;
  WORD wRowSize;

  pBitmapInfo->biWidth = cWinSizeX;
  pBitmapInfo->biHeight = -(long)cWinSizeY;
  wRowSize = (WORD)((3*cWinSizeX + 3L) >> 2) << 2;

  if (pImg)
  {
    cSizeX = cWinSizeX - 2*MARGIN;
    cSizeY = cWinSizeY - 2*MARGIN;
    // handle aspect ratio issues
    // if ((cSizeY / cSizeX) > (cImgSizeY / cImgSizeX))
    if (cSizeY*cImgSizeX > cImgSizeY*cSizeX)
    {
      cSizeY = cSizeX * cImgSizeY / cImgSizeX;
      cImgPosX = MARGIN;
      cImgPosY = (cWinSizeY - cSizeY) / 2;
      cImgScale = (float)cSizeX / cImgSizeX;
    }
    else
    {
      cSizeX = cSizeY * cImgSizeX / cImgSizeY;
      cImgPosY = MARGIN;
      cImgPosX = (cWinSizeX - cSizeX) / 2;
      cImgScale = (float)cSizeY / cImgSizeY;
    }
    // calculate row sizes
    wImgRowSize = cImgChannels * cSizeX;
    // copy image into bitmap
    for (y=0, p=pBitmap; y<cImgPosY; y++, p+=wRowSize)
    {
      for (x=0, r=p; x<cWinSizeX; x++, r+=3)
      {
	r[0] = cBackGround[2];
	r[1] = cBackGround[1];
	r[2] = cBackGround[0];
      }
    }
    for (y=0; y<cSizeY; y++, p+=wRowSize)
    {
      yImg = y*cImgSizeY / cSizeY;
      for (x=0, r=p; x<cImgPosX; x++, r+=3)
      {
	r[0] = cBackGround[2];
	r[1] = cBackGround[1];
	r[2] = cBackGround[0];
      }
      for (x=0; x<cSizeX; x++, r+=3)
      {
	xImg = x*cImgSizeX / cSizeX;
	s = pImg + (yImg*cImgSizeX+xImg)*cImgChannels;
	r[0] = s[2];
	r[1] = s[1];
	r[2] = s[0];
	if (cImgChannels == 4)
	{
	  float o = s[3]/255.0f;
	  r[0] = (BYTE)(r[0]*o + cBackGround[0]*(1-o));
	  r[1] = (BYTE)(r[1]*o + cBackGround[1]*(1-o));
	  r[2] = (BYTE)(r[2]*o + cBackGround[2]*(1-o));
	}
      }
      for (x=0; x<cWinSizeX-cSizeX-cImgPosX; x++, r+=3)
      {
	r[0] = cBackGround[2];
	r[1] = cBackGround[1];
	r[2] = cBackGround[0];
      }
    }
    for (y=0; y<cWinSizeY-cSizeY-cImgPosY; y++, p+=wRowSize)
    {
      for (x=0, r=p; x<cWinSizeX; x++, r+=3)
      {
	r[0] = cBackGround[2];
	r[1] = cBackGround[1];
	r[2] = cBackGround[0];
      }
    }
  }
  else
  {
    for (y=0, p=pBitmap; y<cWinSizeY; y++, p+=wRowSize)
    {
      for (x=0, r=p; x<cWinSizeX; x++, r+=3)
      {
	r[0] = cBackGround[2];
	r[1] = cBackGround[1];
	r[2] = cBackGround[0];
      }
    }
  }
}

BOOL BuildFileList (PTSTR pPathName, TCHAR **ppFileList,
		    int *pFileCount, int *pFileIndex)
{
  static TCHAR szImgPathName[MAX_PATH];
  static TCHAR szImgFindName[MAX_PATH];
  WIN32_FIND_DATA finddata;
  HANDLE hFind;
  static TCHAR szTmp[MAX_PATH];
  BOOL bOk;
  int i, ii;
  int j, jj;
  char* c;
  // free previous file-list
  if (*ppFileList != NULL)
  {
    free(*ppFileList);
    *ppFileList = NULL;
  }
  // extract foldername, filename and search-name
  strcpy(szImgPathName, pPathName);
  c = strrchr(pPathName, '\\');
  if (c == NULL)
  {
    strcpy(szImgFindName, ".\\");
    strcat(szImgFindName, "*.png");
  }
  else
  {
    strcpy(szImgFindName, pPathName);
    *(strrchr(szImgFindName, '\\') + 1) = '\0';
    strcat(szImgFindName, "*.png");
  }
  // first cycle: count number of files in directory for memory allocation
  *pFileCount = 0;
  hFind = FindFirstFile(szImgFindName, &finddata);
  bOk = (hFind != INVALID_HANDLE_VALUE); //(HANDLE)-1);
  while (bOk)
  {
    *pFileCount += 1;
    bOk = FindNextFile(hFind, &finddata);
  }
  FindClose(hFind);
  // allocate memory for file-list
  *ppFileList = (TCHAR*)malloc(*pFileCount * MAX_PATH);
  // second cycle: read directory and store filenames in file-list
  hFind = FindFirstFile(szImgFindName, &finddata);
  bOk = (hFind != (HANDLE) -1);
  i = 0;
  ii = 0;
  while (bOk)
  {
    strcpy(*ppFileList + ii, szImgPathName);
    c = strrchr(*ppFileList + ii, '\\');
    if (c == NULL) { c = *ppFileList + ii; c[0] = '.'; c[1] = '\\'; c++; }
    strcpy(c + 1, finddata.cFileName);
    if (strcmp(pPathName, *ppFileList + ii) == 0)
      *pFileIndex = i;
    ii += MAX_PATH;
    i++;
    bOk = FindNextFile(hFind, &finddata);
  }
  FindClose(hFind);
  // finally we must sort the file-list
  for (i=0; i<*pFileCount-1; i++)
  {
    ii = i*MAX_PATH;
    for (j=i+1; j<*pFileCount; j++)
    {
      jj = j*MAX_PATH;
      if (strcmp(*ppFileList + ii, *ppFileList + jj) > 0)
      {
	strcpy(szTmp, *ppFileList + jj);
	strcpy(*ppFileList + jj, *ppFileList + ii);
	strcpy(*ppFileList + ii, szTmp);
	// check if this was the current image that we moved
	if (*pFileIndex == i)
	  *pFileIndex = j;
	else
	  if (*pFileIndex == j) *pFileIndex = i;
      }
    }
  }
  return TRUE;
}

BOOL SearchFileList (TCHAR *pFileList, int FileCount, int *pFileIndex,
		     PTSTR pPrevName, PTSTR pNextName)
{
  if (FileCount > 0)
  {
    // get previous entry
    if (pPrevName != NULL)
    {
      if (*pFileIndex > 0)
	*pFileIndex -= 1;
      else
	*pFileIndex = FileCount - 1;            
      strcpy(pPrevName, pFileList + (*pFileIndex*MAX_PATH));
    }
    // get next entry
    if (pNextName != NULL)
    {
      if (*pFileIndex < FileCount - 1)
	*pFileIndex += 1;
      else
	*pFileIndex = 0;
      strcpy(pNextName, pFileList + (*pFileIndex*MAX_PATH));
    }
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

BOOL CALLBACK AboutProc (HWND hwnd, UINT message,
			 WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      ShowWindow(hwnd, SW_HIDE);
      CenterWindow(hwnd, GetWindow(hwnd, GW_OWNER));
      ShowWindow(hwnd, SW_SHOW);
    }
    return TRUE;

    case WM_COMMAND:
    switch (LOWORD(wParam))
    {
      case IDOK:
      case IDCANCEL:
      {
	EndDialog(hwnd, 0) ;
      }
      return TRUE;
    }
    break;
  }
  return FALSE;
}

BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
  RECT rChild, rParent, rWorkArea;
  int wChild, hChild, wParent, hParent;
  int xNew, yNew;
  BOOL bResult;
  // get the height and width of the child window
  GetWindowRect(hwndChild, &rChild);
  wChild = rChild.right - rChild.left;
  hChild = rChild.bottom - rChild.top;
  // get the height and width of the parent window
  GetWindowRect (hwndParent, &rParent);
  wParent = rParent.right - rParent.left;
  hParent = rParent.bottom - rParent.top;
  // get the limits of the screen
  bResult = SystemParametersInfo(SPI_GETWORKAREA, sizeof(RECT), &rWorkArea, 0);
  if (!bResult)
  {
    rWorkArea.left = rWorkArea.top = 0;
    rWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
    rWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
  }
  // calculate new x position, then adjust for the screen
  xNew = rParent.left + ((wParent - wChild) /2);
  if (xNew < rWorkArea.left)
    xNew = rWorkArea.left;
  else if ((xNew+wChild) > rWorkArea.right)
    xNew = rWorkArea.right - wChild;
  // calculate new y position, then adjust for the screen
  yNew = rParent.top  + ((hParent - hChild) /2);
  if (yNew < rWorkArea.top)
    yNew = rWorkArea.top;
  else if ((yNew+hChild) > rWorkArea.bottom)
    yNew = rWorkArea.bottom - hChild;
  // set it, and return
  return SetWindowPos(hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

/* 
 * Simple command line argument parser
 */
BOOL ParseArg (char* arg)
{
  int i=0;
  BOOL hasFileName=FALSE;
  char* c;
  if (arg == NULL) return FALSE;
  c = strtok(arg, " \t");
  while (c != NULL)
  {
    if (c[0] != '-' || c[1] == '\0')
    {
      if (hasFileName) return TRUE;
      if (c[0] == '"') // quoted
	strncpy(szImgFileName, c+1, strlen(c)-2);
      else
	strcpy(szImgFileName, c);
      {
	char* cc = szImgFileName;
	while ((cc = strchr(cc, '/'))) cc[0] = '\\';	
	hasFileName = TRUE;
      }
    }
    else
    {
      switch (c[1])
      {
	case 's':
	{
	  c = strtok(NULL, " \t");
	  if (c == NULL || sscanf(c, "%f", &cImgScale) != 1) return TRUE;
	}
	break;
	default:
	return TRUE;
      }
    }
    c = strtok(NULL, " \t");
  }
  return FALSE;  
}
