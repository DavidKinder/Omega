/* Fake curses interface to the Windows front-end */

typedef struct _WINDOW
{
  int _cury;
  int _curx;
  int _maxy;
  int _maxx;
  int _offy;
  int _offx;
  int _attr;
  int* _text;
  int* _line;
}
WINDOW;

int clear(void);
int crmode(void);
int endwin(void);
WINDOW *initscr(void);
int move(int y, int x);
WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x);
int noecho(void);
int printw(const char *fmt, ...);
int refresh(void);
int scrollok(WINDOW *win, int bf);
int touchwin(WINDOW *win);
int waddch(WINDOW *win, const char ch);
int waddstr(WINDOW *win, const char *str);
int wattrset(WINDOW *win, int attrs);
int werase(WINDOW *win);
int wgetch(WINDOW *win);
int wmove(WINDOW *win, int y, int x);
int wprintw(WINDOW *win, const char *fmt, ...);
int wrefresh(WINDOW *win);
int wstandend(WINDOW *win);
int wstandout(WINDOW *win);

void impl_getyx(WINDOW* win, int* y, int* x);
#define getyx(win,y,x) impl_getyx(win,&y,&x); 

int impl_getch(void);
#define getch() impl_getch()

extern WINDOW* stdscr;
extern WINDOW* curscr;

extern int LINES;
extern int COLS;
