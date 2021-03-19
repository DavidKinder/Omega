#include <assert.h>
#include <malloc.h>
#include <stdlib.h>

#include <deque>
#include <map>
#include <set>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <objbase.h>
#include <shlobj.h>

extern "C"
{
// Curses variables
#include "curses.h"

WINDOW* stdscr = NULL;
WINDOW* curscr = NULL;
int COLS = 80;
int LINES = 25;

// Omega variables
#ifndef __STDC__
#define __STDC__
#endif
#include "glob.h"

extern WINDOW *Levelw,*Dataw,*Flagw,*Timew,*Menuw,*Locw,*Morew,*Phasew;
extern WINDOW *Comwin,*Msg1w,*Msg2w,*Msg3w,*Msgw;
extern int target_on;

// Directory for Omega library files
char OMEGALIB[_MAX_PATH];
}

// Windows interface

#include "resource.h"

// Path to saved file to load at start
char OmegaSave[_MAX_PATH] = "";

// Handles for window components
HWND wnd = 0;
HFONT font = 0;
HICON icon = 0;

// Handles for the display bitmap
HDC drawDC = 0;
HBITMAP drawBmp = 0;

// Handle for registry key for settings
HKEY settings = 0;

// The size of the display
RECT workArea = { 0,0,0,0 };
SIZE winSize = { 0,0 };

// Details of the font
LOGFONT fontSetup;
int fontSize = 11;
TEXTMETRIC fontMetrics;

// Set of fixed width TrueType fonts
std::set<std::string> fontNames;

// Queue of input key presses
std::deque<int> inputKeys;

// Palette for text
COLORREF palette[] =
{
  RGB(0x00,0x00,0x00), // black
  RGB(0x00,0x00,0xFF), // blue
  RGB(0x00,0x66,0x11), // green
  RGB(0x00,0xCC,0xCC), // cyan
  RGB(0x88,0x00,0x00), // red 
  RGB(0xAA,0x00,0xCC), // purple
  RGB(0x99,0x55,0x00), // brown
  RGB(0xFF,0xFF,0xDD), // off white
  RGB(0xAA,0xAA,0xAA), // grey
  RGB(0x00,0xBB,0xFF), // light blue
  RGB(0x66,0xFF,0x44), // light green
  RGB(0x55,0xFF,0xFF), // light cyan
  RGB(0xFF,0x00,0x00), // light red
  RGB(0xDD,0x55,0xFF), // light purple
  RGB(0xFF,0xCC,0x00), // yellow
  RGB(0xFF,0xFF,0xFF), // white
};

// Details of the input cursor
bool cursorOn = false;
bool cursorSolid = false;
POINT cursorPos = { 0,0 };

// If true, use graphics
bool graphics = true;

// Handles for the display bitmap
HDC gfxDC = 0;
HBITMAP gfxBmp = 0;

// The size of a graphics tile
SIZE gfxSize = { 32,32 };

// Mapping for graphics tiles
std::map<int,POINT> gfxMap;

// Data to initialize the graphics tile mapping
int gfxMapData[][3] =
{
  // Objects, locations and terrain
  SPACE,0,0,
  WALL,0,26,
  PORTCULLIS,33,22,
  OPEN_DOOR,32,26,
  CLOSED_DOOR,31,26,
  WHIRLWIND,42,18,
  ABYSS,7,22,
  LAVA,79,23,
  HEDGE,60,23,
  WATER,18,23, // Also RIVER
  FIRE,78,23,
  TRAP,33,10,
  LIFT,31,27,
  STAIRS_UP,42,22,
  STAIRS_DOWN,43,22,
  FLOOR,14,23, // Also ROAD
  PLAYER,0,4,
  CORPSE,45,7,
  STATUE,0,24,
  RUBBLE,55,7,
  ALTAR,34,26,
  CASH,9,7,
  PILE,12,7,
  FOOD,4,2,
  WEAPON,13,10,
  MISSILEWEAPON,60,10,
  SCROLL,75,7,
  POTION,40,8,
  ARMOR,48,0,
  SHIELD,94,0,
  CLOAK,5,0,
  BOOTS,20,0,
  STICK,28,10,
  RING,10,5,
  THING,9,6,
  ARTIFACT,42,5,
  // Terrain types
  PLAINS,9,23,
  TUNDRA,99,23,
  MOUNTAINS,117,23,
  PASS,96,21,
  CITY,84,25,
  VILLAGE,96,24,
  FOREST,54,23,
  JUNGLE,55,23,
  SWAMP,11,23,
  VOLCANO,117,22,
  CASTLE,53,26,
  TEMPLE,34,26,
  CAVES,82,22,
  DESERT,99,22,
  CHAOS_SEA,20,23,
  STARPEAK,87,23,
  DRAGONLAIR,31,12,
  MAGIC_ISLE,51,7,
  CHAIR,24,26,
  SAFE,5,7,
  FURNITURE,48,24,
  BED,25,26,
  // Monsters in effects code
  'n'|COL_RED,66,21, // nymph (and necromancer)
  's'|COL_RED,67,21, // satyr
  'I'|COL_RED,66,21, // incubus
  'S'|COL_RED,67,21, // succubus
  // Weapons in effects code
  '!'|COL_BROWN,65,1, // bolt
  '@'|COL_LIGHT_PURPLE,64,1, // ball
  '*'|COL_LIGHT_RED,68,1, // flame bolt and ball
  '^'|COL_LIGHT_BLUE,95,1, // electricity bolt and ball
  // Monsters
  'h'|COL_YELLOW,48,15, // hornet
  '@'|COL_RED,0,3, // men, priests, etc.
  'G'|COL_RED,5,4, // guardsman
  's'|COL_WHITE,75,15, // sheep
  'g'|COL_BROWN,32,19, // grunt
  't'|COL_PURPLE,56,17, // tse-tse fly
  'f'|COL_PURPLE,31,15, // fnord
  'r'|COL_BROWN,27,13, // sewer rat and rabbit
  'f'|COL_CYAN,91,14, // aggravator fungus
  'b'|COL_BRIGHT_WHITE|COL_FG_BLINK,78,14, // blipper
  'g'|COL_GREEN,88,3, // goblin
  'p'|COL_RED,54,15, // phantasticon
  'R'|COL_GREY,2,18, // household robot
  'g'|COL_WHITE,8,11, // pencil-necked geek
  'b'|COL_GREEN,8,19, // borogrove
  'q'|COL_BROWN,12,17, // quail
  'b'|COL_BROWN,25,13, // badger and bear
  'H'|COL_BROWN,37,19, // hawk
  'h'|COL_BROWN,27,19, // hyena and horse
  'd'|COL_BROWN,27,19, // deer and doberman death-hound
  'c'|COL_BROWN,27,19, // camel and radeligian cateagle
  'a'|COL_BROWN,42,16, // anteater
  't'|COL_CYAN,33,17, // trout
  'b'|COL_CYAN,34,17, // striped bass
  'p'|COL_PURPLE,15,17, // parrot
  'n'|COL_GREY,23,4, // ninjas, night gaunt and transparent nasty
  'e'|COL_GREEN,10,16, // floating eye and etheric grue
  't'|COL_GREEN,16,19, // tove
  'g'|COL_GREY,20,20, // ghost
  'e'|COL_RED,18,3, // enchanter
  'f'|COL_GREY,90,14, // murk fungus
  'G'|COL_GREEN,84,3, // goblin chieftain and goblin shaman
  'w'|COL_BROWN,76,15, // wolf
  'a'|COL_RED,21,19, // giant soldier ant
  'e'|COL_GREY,61,17, // elephant
  's'|COL_LIGHT_RED,13,19, // salamander
  'C'|COL_GREEN,30,17, // catoblepas
  'f'|COL_LIGHT_BLUE,52,13, // lesser frost demon
  'a'|COL_YELLOW,23,15, // acid cloud
  'p'|COL_GREY,21,20, // phantom
  'K'|COL_LIGHT_GREEN,82,3, // The Goblin King
  'P'|COL_PURPLE,59,14, // pterodactyl
  'l'|COL_YELLOW,77,15, // lion
  'b'|COL_RED,67,21, // brigand
  'm'|COL_RED,27,16, // mamba
  'M'|COL_BLUE,6,15, // man o' war
  'T'|COL_GREY,21,15, // thought-form
  'M'|COL_YELLOW,64,16, // manticore
  'T'|COL_BROWN,45,11, // tasmanian devil
  'a'|COL_GREY,4,18, // automatum minor
  's'|COL_GREEN,0,15, // denebian slime devil
  'j'|COL_PURPLE,36,19, // jub-jub bird
  'h'|COL_GREY,22,20, // haunt
  'c'|COL_GREEN,16,19, // giant crocodile
  't'|COL_BROWN,37,15, // torpor beast
  'F'|COL_GREY,25,15, // astral fuzzy
  'l'|COL_BLUE,30,4, // servant of law
  'c'|COL_RED,34,4, // servant of chaos
  's'|COL_YELLOW,48,15, // swarm
  'b'|COL_GREY,39,15, // ban sidhe
  'd'|COL_LIGHT_RED,13,12, // dragonette and outer circle demon
  't'|COL_LIGHT_BLUE,8,12, // tesla monster
  'W'|COL_LIGHT_RED,13,12, // wyvern and lava worm
  'f'|COL_WHITE,30,14, // frost demon
  'S'|COL_GREY,21,20, // spectre and shadow spirit
  'B'|COL_GREEN,40,21, // bog thing
  'V'|COL_GREY,70,21, // astral vampire
  'm'|COL_PURPLE,65,16, // manaburst
  'm'|COL_GREY,55,17, // mirrorshade
  'F'|COL_WHITE|COL_BG_RED,20,14, // fire elemental
  'A'|COL_WHITE|COL_BG_BLUE,21,14, // air elemental
  'W'|COL_WHITE|COL_BG_BLUE,24,14, // water elemental
  'E'|COL_WHITE|COL_BG_BROWN,23,14, // earth elemental
  'b'|COL_YELLOW|COL_BG_BROWN,71,18, // bandersnatch
  'L'|COL_BLACK|COL_BG_WHITE,35,16, // lich
  'T'|COL_LIGHT_GREEN|COL_BG_BLUE,8,19, // triton
  'T'|COL_GREEN|COL_BG_BROWN,47,16, // triceratops
  'R'|COL_YELLOW|COL_BG_WHITE,9,21, // rakshasa
  'S'|COL_GREEN|COL_BG_RED,27,16, // demon serpent
  'a'|COL_YELLOW|COL_BG_WHITE,65,21, // angel
  'n'|COL_BLACK|COL_BG_WHITE,23,4, // ninja (chunin) and ninja (jonin)
  'B'|COL_BLACK|COL_BG_BROWN,62,17, // behemoth
  'N'|COL_BLACK|COL_BG_WHITE,6,20, // nazgul
  'U'|COL_BLACK|COL_BG_WHITE,68,16, // unicorn
  'r'|COL_BLACK|COL_BG_BROWN,49,16, // rodent of unusual size
  'F'|COL_BLACK|COL_BG_WHITE,26,19, // illusory fiend
  'W'|COL_GREY|COL_BG_RED,47,17, // Great Wyrm
  'F'|COL_GREY|COL_BG_RED,36,21, // flame devil
  'S'|COL_YELLOW|COL_BG_BROWN,28,19, // sandman
  'm'|COL_RED|COL_BG_WHITE,54,17, // mirror master
  'G'|COL_GREY|COL_BG_GREEN,14,16, // elder etheric grue
  'S'|COL_GREY|COL_BG_GREEN,26,16, // loathly serpent
  'Z'|COL_BLACK|COL_BG_WHITE,26,20, // zombie overlord
  'r'|COL_GREY|COL_BG_BROWN,44,18, // ricochet
  'D'|COL_GREY|COL_BG_RED,21,12, // inner circle demon and dragon
  'f'|COL_YELLOW|COL_BG_WHITE,19,14, // fairy
  'A'|COL_GREY|COL_BG_WHITE,8,18, // automatum major
  'J'|COL_BROWN|COL_BG_RED,41,16, // jabberwock
  'F'|COL_LIGHT_BLUE|COL_BG_WHITE,37,21, // frost demon lord
  'T'|COL_YELLOW|COL_BG_BLUE,73,18, // tigershark
  'S'|COL_BLACK|COL_BG_WHITE,34,15, // shadow slayer
  'C'|COL_GREY|COL_BG_BROWN,9,19, // coma beast
  'A'|COL_YELLOW|COL_BG_WHITE,62,21, // high angel and archangel
  'J'|COL_GREY|COL_BG_BROWN,23,13, // jotun
  'i'|COL_BLACK|COL_BG_WHITE,72,21, // invisible slayer
  'W'|COL_GREEN|COL_BG_RED,29,12, // king wyvern
  'd'|COL_BLACK|COL_BG_WHITE,85,1, // deathstar
  'T'|COL_YELLOW|COL_BG_WHITE,6,4, // high thaumaturgist
  'V'|COL_BLACK|COL_BG_RED,18,20, // vampire lord
  'D'|COL_BLACK|COL_BG_RED,31,13, // demon prince
  'D'|COL_BLACK|COL_BG_WHITE,58,20, // Death
  'E'|COL_BLACK|COL_BG_WHITE,65,14, // The Eater of Magic
  'L'|COL_BRIGHT_WHITE|COL_BG_BLUE,18,3, // The LawBringer
  'D'|COL_BRIGHT_WHITE|COL_BG_RED,47,12, // The Dragon Lord
  'D'|COL_WHITE|COL_BG_RED,7,21, // The Demon Emperor
  'E'|COL_BROWN|COL_BG_WHITE,4,3, // The Elemental Lord of Earth
  'A'|COL_LIGHT_BLUE|COL_BG_WHITE,20,3, // The Elemental Lord of Air
  'W'|COL_BLUE|COL_BG_WHITE,9,3, // The Elemental Lord of Water
  'F'|COL_LIGHT_RED|COL_BG_WHITE,15,4, // The Elemental Lord of Fire
  'M'|COL_PURPLE|COL_BG_WHITE,89,21, // The Elemental Master
};

BYTE* offsetDWord(BYTE* ptr)
{
  int offset = (int)(DWORD_PTR)ptr;
  if (offset & 3)
    return ptr + (4 - (offset & 3));
  return ptr;
}

// Show a dialog with the resource changed to an appropriate font name and size.
// Note that different possibilities in variations on the dialog format are not checked!
int showDialog(HINSTANCE instance, int resId, HWND parent, DLGPROC proc)
{
  int code = -1;

  NONCLIENTMETRICS ncm;
  ::ZeroMemory(&ncm,sizeof ncm);
  ncm.cbSize = sizeof ncm;
  ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof ncm,&ncm,0);
  WCHAR fontName[256];
  MultiByteToWideChar(CP_ACP,0,
    ncm.lfMessageFont.lfFaceName,strlen(ncm.lfMessageFont.lfFaceName)+1,fontName,256);

  HRSRC resInfo = FindResource(instance,MAKEINTRESOURCE(resId),RT_DIALOG);
  if (resInfo != 0)
  {
    DWORD resSize = SizeofResource(instance,resInfo);
    HGLOBAL resGlobal = LoadResource(instance,resInfo);
    if (resGlobal != 0)
    {
      BYTE* resMem = (BYTE*)LockResource(resGlobal);
      if (resMem != NULL)
      {
        HGLOBAL copyGlobal = GlobalAlloc(GMEM_ZEROINIT,resSize+64);
        if (copyGlobal != 0)
        {
          BYTE* copyMem = (BYTE*)GlobalLock(copyGlobal);
          if (copyMem != 0)
          {
            int titleLen = (wcslen(((WCHAR*)resMem)+15)+1)*sizeof(WCHAR);
            int copy1Size = (18*sizeof(WORD))+titleLen;
            int font1Len = (wcslen((WCHAR*)(resMem+copy1Size))+1)*sizeof(WCHAR);
            int copy2Size = resSize-copy1Size-font1Len;
            int font2Len = (wcslen(fontName)+1)*sizeof(WCHAR);

            memcpy(copyMem,resMem,copy1Size);
            wcscpy((WCHAR*)(copyMem+copy1Size),fontName);
            memcpy(offsetDWord(copyMem+copy1Size+font2Len),offsetDWord(resMem+copy1Size+font1Len),copy2Size);
            *((WORD*)(copyMem+copy1Size-(3*sizeof(WORD)))) = 9;
            GlobalUnlock(copyGlobal);
          }
          code = DialogBoxIndirect(instance,(LPDLGTEMPLATE)copyGlobal,parent,proc);
          GlobalFree(copyGlobal);
        }
      }
    }
  }

  return code;
}

void fatal(char* msg)
{
  MessageBox(0,msg,"Omega",MB_ICONERROR|MB_OK);
  exit(1);
}

void drawCursor(void)
{
  SetTextColor(drawDC,RGB(0,0,0));
  SetBkColor(drawDC,palette[cursorSolid ? 7 : 0]);
  TextOut(drawDC,cursorPos.x,cursorPos.y," ",1);
  InvalidateRect(wnd,NULL,FALSE);
}

void drawRect(int x, int y, int w, int h, COLORREF colour)
{
  SetBkColor(drawDC,colour);
  RECT r = { x,y,x+w,y+h };
  ExtTextOut(drawDC,0,0,ETO_OPAQUE,&r,"",0,NULL);
}

void copyLibFile(const char* name)
{
  // Copy the file if it does not exist
  char fromPath[_MAX_PATH];
  sprintf(fromPath,"%s\\%s",OMEGALIB,name);
  CopyFile(fromPath,name,TRUE);

  // Remove any read-only etc. flags
  SetFileAttributes(name,FILE_ATTRIBUTE_NORMAL);
}

// Called by Windows with any messages for the window
LRESULT CALLBACK wndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_DESTROY:
    // If the window is closed, shut down the program
    PostQuitMessage(0);
    break;

  case WM_ERASEBKGND:
    return 1;
  case WM_PAINT:
    {
      // Copy the drawing bitmap to the painting device context
      PAINTSTRUCT ps;
      BeginPaint(wnd,&ps);
      BitBlt(ps.hdc,0,0,winSize.cx,winSize.cy,drawDC,0,0,SRCCOPY);
      EndPaint(wnd,&ps);
    }
    return 0;

  case WM_CHAR:
    if (wParam == 13)
      wParam = 10;
    inputKeys.push_back(wParam);
    return 0;
  case WM_KEYDOWN:
    switch (wParam)
    {
    case VK_LEFT:
      inputKeys.push_back('4');
      return 0;
    case VK_RIGHT:
      inputKeys.push_back('6');
      return 0;
    case VK_UP:
      inputKeys.push_back('8');
      return 0;
    case VK_DOWN:
      inputKeys.push_back('2');
      return 0;
    }
    break;

  case WM_TIMER:
    // Toggle and redraw the input cursor
    cursorSolid = !cursorSolid;
    if (cursorOn)
      drawCursor();
    break;
  }

  // Use the default Windows processing
  return DefWindowProc(wnd,msg,wParam,lParam);
}

// Called by Windows with any messages for the saved game list control
LONG_PTR oldSavedListProc = 0;
LRESULT CALLBACK savedListProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  LRESULT result = CallWindowProc((WNDPROC)oldSavedListProc,wnd,msg,wParam,lParam);

  // If the user releases the mouse over a blank area, clear the selection
  if (msg == WM_LBUTTONUP)
  {
    bool found = false;
    int count = SendMessage(wnd,LB_GETCOUNT,0,0);
    for (int i = 0; i < count; i++)
    {
      RECT r;
      if (SendMessage(wnd,LB_GETITEMRECT,i,(LPARAM)&r) != LB_ERR)
      {
        POINT p = { LOWORD(lParam), HIWORD(lParam) };
        if (PtInRect(&r,p))
          found = true;
      }
    }
    if (found == false)
      PostMessage(wnd,LB_SETCURSEL,-1,0);
  }

  return result;
}

// Called by Windows with any messages for the setup dialog
INT_PTR CALLBACK dlgProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_INITDIALOG:
    {
      // Initialize the graphics control
      SendMessage(GetDlgItem(wnd,IDC_GRAPHICS),BM_SETCHECK,graphics ? BST_CHECKED : BST_UNCHECKED,0);

      // Initialize the font face control
      HWND fontFaceCtrl = GetDlgItem(wnd,IDC_FONTFACE);
      for (std::set<std::string>::iterator it = fontNames.begin(); it != fontNames.end(); ++it)
        SendMessage(fontFaceCtrl,CB_ADDSTRING,0,(LPARAM)it->c_str());
      SendMessage(fontFaceCtrl,CB_SELECTSTRING,-1,(LPARAM)fontSetup.lfFaceName);

      // Initialize the font size control
      HWND fontSizeCtrl = GetDlgItem(wnd,IDC_FONTSIZE);
      char fontSizeStr[16];
      for (int i = 8; i <= 18; i++)
      {
        itoa(i,fontSizeStr,10);
        SendMessage(fontSizeCtrl,CB_ADDSTRING,0,(LPARAM)fontSizeStr);
      }
      itoa(fontSize,fontSizeStr,10);
      SendMessage(fontSizeCtrl,CB_SELECTSTRING,-1,(LPARAM)fontSizeStr);

      // Initialize the saved games control
      HWND savedCtrl = GetDlgItem(wnd,IDC_SAVED);
      oldSavedListProc = SetWindowLongPtr(savedCtrl,GWLP_WNDPROC,(LONG_PTR)savedListProc);
      WIN32_FIND_DATA fd;
      HANDLE find = FindFirstFile("Save\\*.*",&fd);
      if (find != INVALID_HANDLE_VALUE)
      {
        do
        {
          if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            SendMessage(savedCtrl,LB_ADDSTRING,0,(LPARAM)fd.cFileName);
        }
        while (FindNextFile(find,&fd) != 0);
      }

      // Set the dialog's icon
      SendMessage(wnd,WM_SETICON,ICON_BIG,(LPARAM)icon);
      SendMessage(wnd,WM_SETICON,ICON_SMALL,(LPARAM)icon);

      // Centre the dialog
      RECT dlgRect;
      GetWindowRect(wnd,&dlgRect);
      int dlgWidth = dlgRect.right-dlgRect.left;
      int dlgHeight = dlgRect.bottom-dlgRect.top;
      SetWindowPos(wnd,HWND_TOP,
        (workArea.right+workArea.left-dlgWidth)/2,(workArea.bottom+workArea.top-dlgHeight)/2,
        0,0,SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER);
    }
    return TRUE;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
    case IDCANCEL:
      {
        // Read in the user's display settings
        graphics = (SendMessage(GetDlgItem(wnd,IDC_GRAPHICS),BM_GETCHECK,0,0) == BST_CHECKED);
        GetWindowText(GetDlgItem(wnd,IDC_FONTFACE),fontSetup.lfFaceName,LF_FACESIZE);
        char fontSizeStr[16];
        GetWindowText(GetDlgItem(wnd,IDC_FONTSIZE),fontSizeStr,16);
        fontSize = atoi(fontSizeStr);
        if (fontSize < 6)
          fontSize = 6;

        // Save the user's display settings
        DWORD regGraphics = graphics ? 1 : 0;
        RegSetValueEx(settings,"Graphics",0,REG_DWORD,(BYTE*)&regGraphics,sizeof regGraphics);
        RegSetValueEx(settings,"Font Name",0,REG_SZ,(BYTE*)fontSetup.lfFaceName,strlen(fontSetup.lfFaceName)+1);
        DWORD regFontSize = fontSize;
        RegSetValueEx(settings,"Font Size",0,REG_DWORD,(BYTE*)&regFontSize,sizeof regFontSize);

        // Read in the user's saved game selection
        int sel = SendMessage(GetDlgItem(wnd,IDC_SAVED),LB_GETCURSEL,0,0);
        if (sel != LB_ERR)
        {
          char saved[_MAX_PATH];
          SendMessage(GetDlgItem(wnd,IDC_SAVED),LB_GETTEXT,sel,(LPARAM)saved);
          sprintf(OmegaSave,"Save\\%s",saved);
        }
      }

      EndDialog(wnd,wParam);
      return TRUE;
    }
    break;
  }
  return FALSE;
}

// Called by Windows when enumerating fronts
int CALLBACK fontProc(ENUMLOGFONTEX* font, NEWTEXTMETRICEX* metric, DWORD fontType, LPARAM)
{
  bool allow = false;
  if (fontType & TRUETYPE_FONTTYPE)
    allow = true;
  else if (!(fontType & RASTER_FONTTYPE))
    allow = ((metric->ntmTm.ntmFlags & NTM_PS_OPENTYPE|NTM_TT_OPENTYPE|NTM_TYPE1) != 0);

  if (allow && (font->elfLogFont.lfPitchAndFamily & FIXED_PITCH))
  {
    if (font->elfLogFont.lfFaceName[0] != '@')
      fontNames.insert(font->elfLogFont.lfFaceName);
  }
  return 1;
}

// Entry point into the program
int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show)
{
  // Don't display horrible old error dialogs
  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

  // Initialize COM and controls
  CoInitialize(NULL);
  InitCommonControls();

  // Get the path to Omega's library files
  GetModuleFileName(0,OMEGALIB,_MAX_PATH);
  char* pathLib = strrchr(OMEGALIB,'\\');
  if (pathLib == NULL)
    fatal("Failed to set up OMEGALIB path");
  strcpy(pathLib+1,"OmegaLib\\");

  // Create the directory for Omega to store files in
  char omegaDir[_MAX_PATH];
  if (FAILED(SHGetFolderPath(0,CSIDL_APPDATA,0,SHGFP_TYPE_CURRENT,omegaDir)))
    fatal("Failed to get CSIDL_APPDATA path");
  strcat(omegaDir,"\\Omega");
  CreateDirectory(omegaDir,NULL);
  if (SetCurrentDirectory(omegaDir) == 0)
    fatal("Failed to set current directory");
  copyLibFile("omega.hi");
  copyLibFile("omega.log");
  strcat(omegaDir,"\\Save");
  CreateDirectory(omegaDir,NULL);

  // Get the desktop window
  HWND desktopWnd = GetDesktopWindow();
  HDC desktopDC = GetDC(desktopWnd);

  // Get the display rectange of the main monitor
  HMONITOR mon = MonitorFromWindow(desktopWnd,MONITOR_DEFAULTTOPRIMARY);
  MONITORINFO mi;
  ZeroMemory(&mi,sizeof mi);
  mi.cbSize = sizeof mi;
  GetMonitorInfo(mon,&mi);
  workArea = mi.rcWork;
  int scrWidth = workArea.right-workArea.left;
  int scrHeight = workArea.bottom-workArea.top;

  // Get all fixed width fonts
  ZeroMemory(&fontSetup,sizeof fontSetup);
  fontSetup.lfCharSet = ANSI_CHARSET;
  EnumFontFamiliesEx(desktopDC,&fontSetup,(FONTENUMPROC)fontProc,0,0);

  // Get the Omega icon
  icon = LoadIcon(instance,MAKEINTRESOURCE(IDI_OMEGA));

  // Choose the initial font name
  if (fontNames.count("Consolas") == 1)
    strcpy(fontSetup.lfFaceName,"Consolas");
  else if (fontNames.count("Lucida Console") == 1)
    strcpy(fontSetup.lfFaceName,"Lucida Console");
  else if ((fontNames.size() == 0) || (fontNames.count("Courier New") == 1))
    strcpy(fontSetup.lfFaceName,"Courier New");
  else
    strcpy(fontSetup.lfFaceName,fontNames.begin()->c_str());

  // Open the registry key that settings are stored under
  if (RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\David Kinder\\Omega",0,NULL,
    REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&settings,NULL) != ERROR_SUCCESS)
    fatal("Failed to open settings registry key");

  // Read the user's settings, if present
  BYTE setData[256];
  DWORD setLength = 256;
  DWORD setType = 0;
  if (RegQueryValueEx(settings,"Graphics",NULL,&setType,setData,&setLength) == ERROR_SUCCESS)
  {
    if (setType == REG_DWORD)
      graphics = (*((DWORD*)setData) != 0);
  }
  setLength = 256;
  if (RegQueryValueEx(settings,"Font Name",NULL,&setType,setData,&setLength) == ERROR_SUCCESS)
  {
    if (setType == REG_SZ)
      strcpy(fontSetup.lfFaceName,(char*)setData);
  }
  setLength = 256;
  if (RegQueryValueEx(settings,"Font Size",NULL,&setType,setData,&setLength) == ERROR_SUCCESS)
  {
    if (setType == REG_DWORD)
      fontSize = *((DWORD*)setData);
  }

  // Show the setup dialog
  if (showDialog(instance,IDD_SETUP,0,dlgProc) != IDOK)
    return 0;

  // Create a font
  int logY = GetDeviceCaps(desktopDC,LOGPIXELSY);
  fontSetup.lfHeight = -MulDiv(fontSize,logY,72);
  font = CreateFontIndirect(&fontSetup);
  HGDIOBJ oldFont = SelectObject(desktopDC,font);
  GetTextMetrics(desktopDC,&fontMetrics);
  SelectObject(desktopDC,oldFont);

  // Make the display higher, if possible, and set the window size
  int maxLines = (scrHeight*85)/(fontMetrics.tmHeight*100);
  if (maxLines > 32)
    maxLines = 32;
  if (maxLines > LINES)
    LINES = maxLines;
  winSize.cx = fontMetrics.tmAveCharWidth*COLS;
  winSize.cy = fontMetrics.tmHeight*LINES;

  // Create the display bitmap
  drawDC = CreateCompatibleDC(desktopDC);
  BITMAPINFO bi;
  ZeroMemory(&bi,sizeof bi);
  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth = winSize.cx;
  bi.bmiHeader.biHeight = -1*winSize.cy;
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 24;
  bi.bmiHeader.biCompression = BI_RGB;
  void* drawBits;
  drawBmp = CreateDIBSection(drawDC,&bi,DIB_RGB_COLORS,&drawBits,0,0);
  if (drawBmp == 0)
    fatal("Failed to create bitmap");

  // Initialize the display bitmap
  SelectObject(drawDC,drawBmp);
  SelectObject(drawDC,font);
  SetBkMode(drawDC,OPAQUE);
  drawRect(0,0,winSize.cx,winSize.cy,RGB(0,0,0));

  // Free the desktop window
  ReleaseDC(desktopWnd,desktopDC);
  desktopDC = 0;

  if (graphics)
  {
    // Load the graphics bitmap
    char bitmapPath[_MAX_PATH];
    GetModuleFileName(0,bitmapPath,_MAX_PATH);
    char* bitmapFile = strrchr(bitmapPath,'\\');
    if (bitmapFile == NULL)
      fatal("Failed to set up bitmap path");
    strcpy(bitmapFile+1,"32x32.bmp");
    gfxBmp = (HBITMAP)LoadImage(NULL,bitmapPath,IMAGE_BITMAP,0,0,
      LR_CREATEDIBSECTION|LR_LOADFROMFILE);
    if (gfxBmp == 0)
    {
      MessageBox(0,"Graphics file not loaded.","Omega",MB_ICONERROR|MB_OK);
      graphics = false;
    }

    // Create a device context for the graphics
    gfxDC = CreateCompatibleDC(drawDC);
    SelectObject(gfxDC,gfxBmp);

    // Initialize the graphics tile mapping
    for (int i = 0; i < sizeof gfxMapData / sizeof gfxMapData[0]; i++)
    {
      assert(gfxMap.count(gfxMapData[i][0]) == 0);
      POINT p = { gfxMapData[i][1],gfxMapData[i][2] };
      gfxMap[gfxMapData[i][0]] = p;
    }
  }

  // Register window class
  WNDCLASSEX wc;
  ZeroMemory(&wc,sizeof wc);
  wc.cbSize = sizeof wc;
  wc.style = CS_HREDRAW|CS_VREDRAW;
  wc.lpfnWndProc = wndProc;
  wc.hInstance = instance;
  wc.hIcon = icon;
  wc.hCursor = LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszClassName = "Omega";
  RegisterClassEx(&wc);

  // Create the window
  wnd = CreateWindowEx(WS_EX_CLIENTEDGE,"Omega","Omega",
    WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_CLIPCHILDREN|WS_MINIMIZEBOX,0,0,100,100,0,0,instance,NULL);
  if (wnd == 0)
    fatal("Failed to create window");

  // Get the size of the window frame
  RECT rectW,rectC;
  GetWindowRect(wnd,&rectW);
  GetClientRect(wnd,&rectC);

  // Centre and show the window
  int w = winSize.cx+(rectW.right-rectW.left-rectC.right);
  int h = winSize.cy+(rectW.bottom-rectW.top-rectC.bottom);
  int x = workArea.left+((scrWidth-w)/2);
  int y = workArea.top+((scrHeight-h)/2);
  MoveWindow(wnd,x,y,w,h,FALSE);
  ShowWindow(wnd,show);

  // Start a timer to flash the cursor
  SetTimer(wnd,1,GetCaretBlinkTime(),NULL);

  // Run Omega
  char* argv[2] = { "omega",OmegaSave };
  int argc = (OmegaSave[0] == '\0') ? 1 : 2;
  return main(argc,argv);
}

// Curses functions

int impl_wprintw(WINDOW *win, const char *fmt, va_list list)
{
  char buffer[256];
  vsprintf(buffer,fmt,list);

  int len = (int)strlen(buffer);
  for (int i = 0; i < len; i++)
    waddch(win,*(buffer+i));
  return len;
}

extern "C" int clear(void)
{
  return werase(stdscr);
}

extern "C" int crmode(void)
{
  if (graphics)
  {
    // Adjust what Omega thinks is the screen height so that
    // the larger size of graphics tiles is taken into account
    if (gfxSize.cy > fontMetrics.tmHeight)
    {
      int height = fontMetrics.tmHeight*(Levelw->_maxy+1);
      ScreenLength = height/gfxSize.cy;
    }
  }
  return 0;
}

extern "C" int endwin(void)
{
  // Do nothing
  return 0;
}

extern "C" WINDOW *initscr(void)
{
  // Set up default windows
  stdscr = newwin(LINES,COLS,0,0);
  curscr = stdscr;
  return stdscr;
}

extern "C" int move(int y, int x)
{
  return wmove(stdscr,y,x);
}

extern "C" WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x)
{
  WINDOW *win = (WINDOW *)calloc(1,sizeof(WINDOW));
  win->_cury = 0;
  win->_curx = 0;
  win->_maxy = nlines-1;
  win->_maxx = ncols-1;
  win->_offy = begin_y;
  win->_offx = begin_x;
  win->_text = (int*)malloc(ncols*nlines*sizeof(int));
  win->_line = (int*)malloc(nlines*sizeof(int));
  wattrset(win,COL_WHITE>>8);
  werase(win);
  return win;
}

extern "C" int noecho(void)
{
  // Do nothing
  return 0;
}

extern "C" int printw(const char *fmt, ...)
{
  va_list arg;
  va_start(arg,fmt);
  int n = impl_wprintw(stdscr,fmt,arg);
  va_end(arg);
  return n;
}

extern "C" int refresh(void)
{
  return wrefresh(stdscr);
}

extern "C" int scrollok(WINDOW *win, int bf)
{
  // Do nothing
  return 0;
}

extern "C" int touchwin(WINDOW *win)
{
  for (int i = 0; i <= win->_maxy; i++)
    win->_line[i] = 1;
  return 0;
}

extern "C" int waddch(WINDOW *win, const char ch)
{
  switch (ch)
  {
  case '\n':
    win->_curx = 0;
    if (win->_cury < win->_maxy)
      win->_cury++;
    break;
  default:
    if (isprint(ch))
    {
      win->_text[(win->_cury*(win->_maxx+1))+win->_curx] = ch+(win->_attr<<8);
      win->_line[win->_cury] = 1;
      if (win->_curx < win->_maxx)
        win->_curx++;
    }
    break;
  }
  return 0;
}

extern "C" int waddstr(WINDOW *win, const char *str)
{
  return wprintw(win,str);
}

extern "C" int wattrset(WINDOW *win, int attrs)
{
  win->_attr = attrs;
  return 0;
}

extern "C" int werase(WINDOW *win)
{
  for (int i = 0; i < (win->_maxx+1)*(win->_maxy+1); i++)
    win->_text[i] = SPACE;
  for (int i = 0; i <= win->_maxy; i++)
    win->_line[i] = 1;
  win->_curx = 0;
  win->_cury = 0;
  return 0;
}

extern "C" int wgetch(WINDOW *win)
{
  // Turn the cursor on
  if (win != Levelw)
  {
    cursorOn = true;
    cursorSolid = false;
    cursorPos.x = fontMetrics.tmAveCharWidth*(win->_offx+win->_curx);
    cursorPos.y = fontMetrics.tmHeight*(win->_offy+win->_cury);
  }

  // Redraw the display
  InvalidateRect(wnd,NULL,FALSE);

  // Pump messages
  while (inputKeys.size() == 0)
  {
    // While there are messages, process them
    MSG msg;
    while (PeekMessage(&msg,0,0,0,PM_REMOVE))
    {
      // If a quit message has been received, exit the program
      if (msg.message == WM_QUIT)
        exit(0);
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if (inputKeys.size() == 0)
      WaitMessage();
  }

  // Turn the cursor off
  cursorSolid = false;
  if (cursorOn)
    drawCursor();
  cursorOn = false;

  int c = inputKeys.front();
  inputKeys.pop_front();
  return c;
}

extern "C" int wmove(WINDOW *win, int y, int x)
{
  if (y > win->_maxy)
    y = win->_maxy;
  if (x > win->_maxx)
    x = win->_maxx;
  win->_cury = y;
  win->_curx = x;
  return 0;
}

extern "C" int wprintw(WINDOW *win, const char *fmt, ...)
{
  va_list arg;
  va_start(arg,fmt);
  int n = impl_wprintw(win,fmt,arg);
  va_end(arg);
  return n;
}

extern "C" int wrefresh(WINDOW *win)
{
  if ((win == stdscr) || (win == Menuw))
  {
    touchwin(Msgw);
    touchwin(Msg1w);
    touchwin(Msg2w);
    touchwin(Msg3w);
    touchwin(Levelw);
    touchwin(Timew);
    touchwin(Flagw);
    touchwin(Dataw);
    touchwin(Locw);
    touchwin(Morew);
    touchwin(Phasew);
    touchwin(Comwin);
    touchwin(Menuw); 
    touchwin(stdscr);
  }

  if (graphics && (win == Levelw))
  {
    // Get the size of the graphics window in pixels
    SIZE gfxWinPixels;
    gfxWinPixels.cx = fontMetrics.tmAveCharWidth*(win->_maxx+1);
    gfxWinPixels.cy = fontMetrics.tmHeight*(win->_maxy+1);

    // Get the size of the graphics window in graphics tiles
    SIZE gfxWinTiles;
    gfxWinTiles.cx = (gfxSize.cx > fontMetrics.tmAveCharWidth) ? gfxWinPixels.cx/gfxSize.cx : win->_maxx+1;
    gfxWinTiles.cy = (gfxSize.cy > fontMetrics.tmHeight) ? gfxWinPixels.cy/gfxSize.cy : win->_maxy+1;

    // Get the resulting margin (if any) due to the difference in text and tile size
    SIZE gfxMargin;
    gfxMargin.cx = gfxWinPixels.cx - (gfxWinTiles.cx*gfxSize.cx);
    gfxMargin.cy = gfxWinPixels.cy - (gfxWinTiles.cy*gfxSize.cy);

    // Get the top-left corner to start drawing graphics at
    POINT gfxTopLeft;
    gfxTopLeft.x = (fontMetrics.tmAveCharWidth*win->_offx) + (gfxMargin.cx/2);
    gfxTopLeft.y = (fontMetrics.tmHeight*win->_offy) + (gfxMargin.cy/2);

    // Get the X offset to draw from
    int offsetX = win->_curx-(gfxWinTiles.cx/2);
    if (offsetX < 0)
      offsetX = 0;
    else if (offsetX > win->_maxx+1-gfxWinTiles.cx)
      offsetX = win->_maxx+1-gfxWinTiles.cx;

    // Draw the tiles
    drawRect(fontMetrics.tmAveCharWidth*win->_offx,fontMetrics.tmHeight*win->_offy,
      gfxWinPixels.cx,gfxWinPixels.cy,RGB(0,0,0));
    for (int y = 0; y < gfxWinTiles.cy; y++)
    {
      for (int x = 0; x < gfxWinTiles.cx; x++)
      {
        POINT tile = { -1,-1 };
        int c = win->_text[(y*(win->_maxx+1))+x+offsetX];

        // Map character to a tile for special cases
        if (Current_Environment != E_COUNTRYSIDE)
        {
          switch (c)
          {
          case 'o'|COL_WHITE: // snowball weapon
            tile.x = 69;
            tile.y = 1;
            break;
          case '!'|COL_RED: // incubus/succubus and satyr/nymph
            tile.x = 67;
            tile.y = 21;
            break;
          }
        }

        // If not a special case, look the tile up in the map
        if (tile.x == -1)
        {
          std::map<int,POINT>::const_iterator it = gfxMap.find(c);
          if (it != gfxMap.end())
            tile = it->second;
        }

        // Draw the tile
        if (tile.x != -1)
        {
          BitBlt(drawDC,
            gfxTopLeft.x+(x*gfxSize.cx),gfxTopLeft.y+(y*gfxSize.cy),gfxSize.cx,gfxSize.cy,
            gfxDC,tile.x*gfxSize.cx,tile.y*gfxSize.cy,SRCCOPY);
        }
        else
        {
          drawRect(gfxTopLeft.x+(x*gfxSize.cx),gfxTopLeft.y+(y*gfxSize.cy),
            gfxSize.cx,gfxSize.cy,RGB(255,0,0));
        }
      }
    }

    // Draw the target cursor
    if (target_on != 0)
    {
      int x = gfxTopLeft.x+((win->_curx-offsetX)*gfxSize.cx);
      int y = gfxTopLeft.y+(win->_cury*gfxSize.cy);
      RECT r = { x,y,x+gfxSize.cx,y+gfxSize.cy };
      FrameRect(drawDC,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));
    }
  }
  else
  {
    for (int y = 0; y <= win->_maxy; y++)
    {
      // Check if the line needs updating
      if (win->_line[y] != 0)
      {
        int len = win->_maxx+1;
        if (len > 0)
        {
          // Get the text string for the line
          char* text = (char*)alloca(len);
          for (int x = 0; x < len; x++)
            text[x] = (char)(win->_text[(y*len)+x] & 0xFF);
          win->_line[y] = 0;

          // Step over the text string, drawing it whenever the character attribute changes
          int x1 = 0, x2 = 0;
          int attr1 = (win->_text[y*len] & 0xFF00)>>8;
          int attr2 = 0;
          while ((x1 < len) && (x2 <= len))
          {
            if (x2 < len)
              attr2 = (win->_text[(y*len)+x2] & 0xFF00)>>8;
            if ((x2 == len) || (attr1 != attr2))
            {
              SetTextColor(drawDC,palette[attr1&0x0F]);
              SetBkColor(drawDC,palette[(attr1&0xF0)>>4]);
              TextOut(drawDC,
                fontMetrics.tmAveCharWidth*(win->_offx+x1),
                fontMetrics.tmHeight*(win->_offy+y),
                text+x1,x2-x1);

              attr1 = attr2;
              x1 = x2;
            }
            else
              x2++;
          }
        }
      }
    }

    // Draw the target cursor
    if (((win == Levelw)) && (target_on != 0))
    {
      int x = fontMetrics.tmAveCharWidth*(win->_offx+win->_curx);
      int y = fontMetrics.tmHeight*(win->_offy+win->_cury);
      RECT r = { x,y,x+fontMetrics.tmAveCharWidth,y+fontMetrics.tmHeight };
      FrameRect(drawDC,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));

      // Make sure the target cursor does not get left on screen
      win->_line[win->_cury] = 1;
    }
  }
  return 0;
}

extern "C" int wstandend(WINDOW *win)
{
  if (win != Levelw)
    win->_attr = (COL_BG_BLACK|COL_WHITE)>>8;
  return 0;
}

extern "C" int wstandout(WINDOW *win)
{
  if (win != Levelw)
    win->_attr = (COL_BG_WHITE|COL_BLACK)>>8;
  return 0;
}

extern "C" void impl_getyx(WINDOW* win, int* y, int* x)
{
  *y = win->_cury;
  *x = win->_curx;
}

extern "C" int impl_getch(void)
{
  return wgetch(stdscr);
}

// Unix utility functions

void impl_sleep(unsigned int milli)
{
  // Redraw the display
  InvalidateRect(wnd,NULL,FALSE);

  // Pump messages until past end time
  DWORD endTime = GetTickCount()+milli;
  while (GetTickCount() < endTime)
  {
    // While there are messages, process them
    MSG msg;
    while (PeekMessage(&msg,0,0,0,PM_REMOVE))
    {
      // If a quit message has been received, exit the program
      if (msg.message == WM_QUIT)
        exit(0);
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    // Wait for a message or until past end time
    DWORD timeOut = endTime-GetTickCount();
    if (timeOut > 0)
      MsgWaitForMultipleObjects(0,NULL,FALSE,timeOut,QS_ALLINPUT);
  }
}

extern "C" unsigned int sleep(unsigned int seconds)
{
  impl_sleep(seconds*1000);
  return 0;
}

extern "C" unsigned int usleep(unsigned int useconds)
{
  impl_sleep(useconds/1000);
  return 0;
}

extern "C" char *getlogin(void)
{
  char *login = getenv("OMEGA_LOGIN");
  if (login == NULL)
    login = "pcuser";
  return login;
}

extern "C" int chown(const char*, short, short)
{
  // Do nothing
  return 0;
}

// Omega functions

extern "C" char* save_dir(char* name)
{
  static char path[_MAX_PATH];
  sprintf(path,"Save\\%s",name);
  return path;
}
