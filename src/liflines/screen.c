/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* modified 2000-04-25 J.F.Chandler */
/*=============================================================
 * screen.c -- Curses user interface to LifeLines
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 29 Sep 93
 *   2.3.6 - 01 Jan 94    3.0.0 - 06 Oct 94
 *   3.0.2 - 25 Mar 95    3.0.3 - 17 Jan 96
 *===========================================================*/

#include "llstdlib.h"
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "table.h"
#include "liflines.h"
#include "arch.h"
#include "menuitem.h"
#include "lloptions.h"
#include "interp.h"
#include "llinesi.h"
#include "screen.h"

#define LINESREQ 24
#define COLSREQ  80
/*
OVERHEAD_MENU: 
1 line across top of screen
1 line above menu
1 line below menu
1 line of prompt message
1 line at very bottom of screen
This is how many lines can't be used by person (or whatever)
*/
#define OVERHEAD_MENU 5
INT LISTWIN_WIDTH=0;
INT MAINWIN_WIDTH=0;

/* center windows on real physical screen (LINES x COLS) */
#define NEWWIN(r,c)   newwin(r,c,(LINES - (r))/2,(COLS - (c))/2)
#define SUBWIN(w,r,c) subwin(w,r,c,(LINES - (r))/2,(COLS - (c))/2)

/*********************************************
 * global/exported variables
 *********************************************/

INT ll_lines = LINESREQ; /* update to be number of lines in screen */
INT ll_cols = COLSREQ;	 /* number of columns in screen used by LifeLines */
BOOLEAN stdout_vis = FALSE;
INT cur_screen = 0;
UIWINDOW main_win = NULL;
UIWINDOW stdout_win=NULL;
static UIWINDOW debug_win=NULL, debug_box_win=NULL;
static UIWINDOW ask_win=NULL, ask_msg_win=NULL;
static UIWINDOW choose_from_list_win=NULL;
static UIWINDOW add_menu_win=NULL, del_menu_win=NULL;
static UIWINDOW scan_menu_win=NULL, cset_menu_win=NULL, rpt_cset_menu_win=NULL;
static UIWINDOW utils_menu_win=NULL, tt_menu_win=NULL;
static UIWINDOW trans_menu_win=NULL;
static UIWINDOW extra_menu_win=NULL;

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN alldone, progrunning;
extern STRING qSwin2big,qSwin2small;
extern STRING empstr,empstr71,empstr120,readpath,qSronlye,qSdataerr;
extern STRING qSabverr,qSuoperr,qSbadttnum,qSnosuchtt,qSmouttt,qSmintt;
extern STRING qSmtitle,qScright,qSdbname,qSdbimmut,qSdbrdonly,qSplschs;
extern STRING qSmn_unkcmd,qSronlya,qSronlyr;
extern STRING qSaskynq,qSaskynyn,qSaskyY;
extern STRING qSmn_quit,qSmn_ret, qSmn_exit;
extern STRING qSmn_mmbrws,qSmn_mmsear,qSmn_mmadd,qSmn_mmdel;
extern STRING qSmn_mmprpt,qSmn_mmrpt,qSmn_mmcset;
extern STRING qSmn_mmtt,qSmn_mmut,qSmn_mmex;
extern STRING qSmn_csttl,qSmn_cstt,qSmn_csintcs,qSmn_csrptcs;
extern STRING qSmn_cstsort,qSmn_cspref,qSmn_cschar,qSmn_cslcas,qSmn_csucas;
extern STRING qSmn_csrpt;
extern STRING qSmn_csdsploc,qSmn_csrpttl;
extern STRING idsortttl,idloc;
extern STRING qSmn_edttttl,qSmn_svttttl;
extern STRING qSmn_utsave,qSmn_utread,qSmn_utkey,qSmn_utkpers,qSmn_utdbstat,qSmn_utmemsta;
extern STRING qSmn_utplaces,qSmn_utusropt;
extern STRING qSmn_xxbsour, qSmn_xxbeven, qSmn_xxbothr, qSmn_xxasour, qSmn_xxesour;
extern STRING qSmn_xxaeven, qSmn_xxeeven, qSmn_xxaothr, qSmn_xxeothr;
extern STRING qSchlist,qSvwlist,qSerrlist,qSdefttl,qSiddefpath;

extern STRING qSmn_uttl;
extern STRING qSmn_xttl;
extern STRING qSmn_notimpl;

extern STRING qSmn_add_ttl,qSmn_add_indi,qSmn_add_fam,qSmn_add_chil,qSmn_add_spou;
extern STRING qSmn_del_ttl,qSmn_del_chil,qSmn_del_spou,qSmn_del_indi,qSmn_del_fam;
extern STRING qSmn_sca_ttl,qSmn_sca_nmfu,qSmn_sca_nmfr,qSmn_sca_refn;
extern STRING qSmn_tt_ttl,qSmn_tt_edit,qSmn_tt_load,qSmn_tt_save,qSmn_tt_exp;
extern STRING qSmn_tt_imp,qSmn_tt_dir;
extern STRING qSmn_tt_edin,qSmn_tt_ined,qSmn_tt_gdin,qSmn_tt_ingd;
extern STRING qSmn_tt_dsin,qSmn_tt_inds,qSmn_tt_inrp;
extern STRING qSsts_sca_ful,qSsts_sca_fra,qSsts_sca_ref,qSsts_sca_non;

/*********************************************
 * local types
 *********************************************/

/*
  Data for lists display
  Both popup & full-screen lists have 3 rectangles:
    details, list items, and menu
  But they are not in the same place
*/
typedef struct listdisp_s
{
	UIWINDOW uiwin;
	struct llrect_s rectList;
	struct llrect_s rectDetails;
	struct llrect_s rectMenu;
	INT details; /* #rows of detail info */
	INT details_minhgt;
	INT details_beginhgt; /* increase from 0 goes to this */
	INT details_maxhgt;
	INT details_scroll; /* scroll offset in detail area */
	INT cur; /* current item selected, 0-based */
	INT listlen; /* #items total */
	INT top; /* current item at top of display, 0-based */
	INT mode; /* record display mode */
} listdisp;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void activate_uiwin(UIWINDOW uiwin);
static void append_to_msg_list(STRING msg);
static INT array_interact(STRING ttl, INT len, STRING *strings
	, BOOLEAN selecting, DETAILFNC detfnc, void *param);
static void begin_action(void);
static INT calculate_screen_lines(INT screen);
static void check_stdout(void);
static INT choose_one_or_list_from_indiseq(STRING ttl, INDISEQ seq, BOOLEAN multi);
static INT choose_or_view_array (STRING ttl, INT no, STRING *pstrngs
	, BOOLEAN selecting, DETAILFNC detfnc, void *param);
static INT choose_tt(STRING prompt);
static void clear_msgs(void);
static void clear_status(void);
static void clearw(void);
static void color_hseg(WINDOW *win, INT row, INT x1, INT x2, char ch);
static UIWINDOW create_uisubwindow(UIWINDOW parent, INT rows, INT cols, INT begy, INT begx);
static void create_windows(void);
static void deactivate_uiwin(void);
static void deactivate_uiwin_and_touch_all(void);
static void delete_uiwindow(UIWINDOW uiw);
static void disp_codeset(UIWINDOW uiwin, INT row, INT col, STRING menuit, INT codeset);
static void disp_locale(UIWINDOW uiwin, INT row, INT col, STRING menuit
	, STRING locVarName);
static void disp_trans_table_choice(UIWINDOW uiwin, INT row, INT col, STRING menuit, INT indx);
static void display_status(STRING text);
static void edit_tt_menu(void);
static void end_action(void);
static void export_tts(void);
static STRING get_answer(UIWINDOW uiwin, INT row, INT col);
static INT handle_list_cmds(listdisp * ld, INT code);
static BOOLEAN handle_popup_list_resize(listdisp * ld, INT code);
static void import_tts(void);
static INT interact(UIWINDOW uiwin, STRING str, INT screen);
static NODE invoke_add_menu(void);
static void invoke_cset_menu(void);
static void invoke_del_menu(void);
static INT invoke_extra_menu(void);
static RECORD invoke_scan_menu(void);
static void invoke_trans_menu(void);
static void invoke_utils_menu(void);
static void load_tt_action(void);
static void output_menu(UIWINDOW uiwin, INT screen, INT bottom, INT width);
void place_cursor(void);
static void place_std_msg(void);
static void reactivate_uiwin(UIWINDOW);
static void refresh_main(void);
static void print_list_title(char * buffer, INT len, const listdisp * ld, STRING ttl);
static void repaint_add_menu(UIWINDOW uiwin);
static void repaint_delete_menu(UIWINDOW uiwin);
static void repaint_scan_menu(UIWINDOW uiwin);
static void repaint_cset_menu(UIWINDOW uiwin);
static void repaint_rpc_menu(UIWINDOW uiwin);
/*static void repaint_tt_menu(UIWINDOW uiwin);*/
static void repaint_trans_menu(UIWINDOW uiwin);
static void repaint_utils_menu(UIWINDOW uiwin);
static void repaint_extra_menu(UIWINDOW uiwin);
static void repaint_footer_menu(INT screen);
static void repaint_main_menu(UIWINDOW uiwin);
static void rpt_cset_menu(void);
static void run_report(BOOLEAN picklist);
static void save_tt_action(void);
static void show_fam (UIWINDOW uiwin, NODE fam, INT mode, INT row, INT hgt, INT width, INT * scroll, BOOLEAN reuse);
static void show_record(UIWINDOW uiwin, STRING key, INT mode, LLRECT
	, INT * scroll, BOOLEAN reuse);
static void show_tandem_line(UIWINDOW uiwin, INT row);
static void shw_array_of_strings(STRING *strings, listdisp *ld
	, DETAILFNC detfnc, void * param);
static void shw_popup_list(INDISEQ seq, listdisp * ld);
static void shw_recordlist_details(INDISEQ seq, listdisp * ld);
static void shw_recordlist_list(INDISEQ seq, listdisp * ld);
static void switch_to_uiwin(UIWINDOW uiwin);
#ifdef HAVE_SETLOCALE
static void test_locale_name(void);
#endif
static void touch_all(BOOLEAN includeCurrent);
static INT translate_control_key(INT c);
static INT translate_hdware_key(INT c);
static void uicolor(UIWINDOW, char ch);
static void uierase(UIWINDOW uiwin);
static INT update_menu(INT screen);
static void user_options(void);

/*********************************************
 * local variables
 *********************************************/

static INT menu_enabled = 1;
static INT menu_dirty = 0;

/* what is showing now in status bar */
static char status_showing[150];
/* flag if it is not important to keep */
static BOOLEAN status_transitory = FALSE;


/* total screen lines used */
INT LINESTOTAL = LINESREQ;
/* number of lines for various menus */
static INT EMPTY_MENU = -1; /* save one horizontal line */
/* the following values are increased if ll_lines > LINESREQ */
int TANDEM_LINES = 6;     /* number of lines of tandem info */
int LIST_LINES = 6;       /* number of lines of person info in list */
int AUX_LINES = 15;       /* number of lines in aux window */
int POPUP_LINES = 17;     /* max lines in popup list */

int winx=0, winy=0; /* user specified window size */

static LIST msg_list = 0;
static BOOLEAN msg_flag = FALSE; /* need to show msg list */
static BOOLEAN viewing_msgs = FALSE; /* user is viewing msgs */
static BOOLEAN lock_std_msg = FALSE; /* to hold status message */
static UIWINDOW active_uiwin = 0;

/* we ought to use chtype, but only if it is typedef'd, but there is no
test to see if a type is typedef'd */
typedef unsigned long llchtype;
static llchtype gr_btee='+', gr_ltee='+', gr_rtee='+', gr_ttee='+';
static llchtype gr_hline='-', gr_vline= '|';
static llchtype gr_llx='*', gr_lrx='*', gr_ulx='*', gr_urx='*';

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*============================
 * init_screen -- Init screens
 * Created: c. 2000/11, Perry Rapp
 *==========================*/
int
init_screen (BOOLEAN graphical)
{
	int extralines;
	INT cols;
	if (winx) { /* user specified window size */
		ll_lines = winy;
		ll_cols = winx;
		if (ll_cols > COLS || ll_lines > LINES) {
			endwin();
			fprintf(stderr, _(qSwin2big), ll_cols, ll_lines, COLS, LINES);
			return 0; /* fail */
		}
	}
	else {
		/* by default, use full screen (as many rows & cols as they have */
		ll_lines = LINES;
		ll_cols = COLS;
	}
	/* check that terminal meet minimum requirements */
	if (ll_cols < COLSREQ || ll_lines < LINESREQ) {
		endwin();
		fprintf(stderr, _(qSwin2small), ll_cols, ll_lines, COLSREQ, LINESREQ);
		return 0; /* fail */
	}

	extralines = ll_lines - LINESREQ;
	LINESTOTAL = ll_lines;
	if(extralines > 0) {
		TANDEM_LINES += (extralines / 2);
		AUX_LINES += extralines;
		LIST_LINES += extralines;
		POPUP_LINES += extralines;
	}
	create_windows();
	cols = (ll_cols-5)/22; /* # of menu cols to start with */
	menuitem_initialize(cols);

	if (graphical) {
		gr_btee = ACS_BTEE;
		gr_hline = ACS_HLINE;
		gr_ltee = ACS_LTEE;
		gr_rtee = ACS_RTEE;
		gr_ttee = ACS_TTEE;
		gr_vline = ACS_VLINE;
		gr_llx = ACS_LLCORNER;
		gr_lrx = ACS_LRCORNER;
		gr_ulx = ACS_ULCORNER;
		gr_urx = ACS_URCORNER;
	}
	
	
	return 1; /* succeed */
}
/*============================
 * term_screen -- Terminate screens
 * Created: 2001/02/01, Perry Rapp
 *  complement of init_screen
 *==========================*/
void
term_screen (void)
{
	menuitem_terminate();
}
/*=======================================
 * draw_win_box -- wrapper for curses box
 *  handles the case that user didn't want
 *  curses graphics
 *=====================================*/
static void
draw_win_box (WINDOW * win)
{
	wborder(win, gr_vline, gr_vline, gr_hline, gr_hline, gr_ulx, gr_urx, gr_llx, gr_lrx);
}
/*=======================================
 * repaint_main_menu --
 *=====================================*/
static void
repaint_main_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row;

	uierase(uiwin);
	draw_win_box(win);
	show_horz_line(uiwin, 4, 0, ll_cols);
	show_horz_line(uiwin, ll_lines-3, 0, ll_cols);
	wmove(win, 1, 2);
	wprintw(win, _(qSmtitle), get_lifelines_version(ll_cols-4));
	mvwaddstr(win, 2, 4, _(qScright));
	mvwprintw(win, 3, 4, _(qSdbname), readpath);
	if (immutable)
		wprintw(win, _(qSdbimmut));
	else if (readonly)
		wprintw(win, _(qSdbrdonly));
	row = 5;
	/* i18n problem: the letters are not being read from the menu strings */
	mvwaddstr(win, row++, 2, _(qSplschs));
	mvwaddstr(win, row++, 4, _(qSmn_mmbrws));
	mvwaddstr(win, row++, 4, _(qSmn_mmsear));
	mvwaddstr(win, row++, 4, _(qSmn_mmadd));
	mvwaddstr(win, row++, 4, _(qSmn_mmdel));
	mvwaddstr(win, row++, 4, _(qSmn_mmprpt));
	mvwaddstr(win, row++, 4, _(qSmn_mmrpt));
	mvwaddstr(win, row++, 4, _(qSmn_mmcset));
	mvwaddstr(win, row++, 4, _(qSmn_mmtt));
	mvwaddstr(win, row++, 4, _(qSmn_mmut));
	mvwaddstr(win, row++, 4, _(qSmn_mmex));
	mvwaddstr(win, row++, 4, _(qSmn_exit));
}
/*================================================
 * repaint_footer_menu -- Paint footer menu for 
 *  whichever screen requested.
 * Created: 2001/02/01, Perry Rapp
 *==============================================*/
void
repaint_footer_menu (INT screen)
{
	UIWINDOW uiwin = main_win;
	WINDOW *win = uiw_win(uiwin);
	INT bottom = LINESTOTAL-3; /* 3 rows below menu */
	INT width = ll_cols;
	uierase(uiwin);
	draw_win_box(win);
	show_horz_line(uiwin, ll_lines-3,  0, ll_cols);
	if (!menu_enabled)
		return;
	output_menu(uiwin, screen, bottom, width);
}
/*==============================================
 * paint_list_screen -- Paint list browse screen
 *============================================*/
void
paint_list_screen (void)
{
	UIWINDOW uiwin = main_win;
	WINDOW *win = uiw_win(uiwin);
	INT row, col;
	uierase(uiwin);
	draw_win_box(win);
	show_horz_line(uiwin, LIST_LINES+1, 0, ll_cols);
	show_horz_line(uiwin, ll_lines-3, 0, ll_cols);
	show_vert_line(uiwin, LIST_LINES+1, 52, 15);
	mvwaddch(win, LIST_LINES+1, 52, gr_ttee);
	mvwaddstr(win, LIST_LINES+2, 54, "Choose an operation:");
	row = LIST_LINES+3; col = 55;
	mvwaddstr(win, row++, col, "j  Move down list");
	mvwaddstr(win, row++, col, "k  Move up list");
	mvwaddstr(win, row++, col, "e  Edit this person");
	mvwaddstr(win, row++, col, "i  Browse this person");
	mvwaddstr(win, row++, col, "m  Mark this person");
	mvwaddstr(win, row++, col, "d  Delete from list");
	mvwaddstr(win, row++, col, "t  Enter tandem mode");
	mvwaddstr(win, row++, col, "n  Name this list");
	mvwaddstr(win, row++, col, "b  Browse new persons");
	mvwaddstr(win, row++, col, "a  Add to this list");
	mvwaddstr(win, row++, col, "x  Swap mark/current");
	mvwaddstr(win, row++, col, _(qSmn_quit));
}
/*==========================================
 * create_uiwindow_impl -- Create our WINDOW wrapper
 * Created: 2001/11/24, Perry Rapp
 *========================================*/
static UIWINDOW
create_uiwindow_impl (WINDOW * win, INT rows, INT cols)
{
	UIWINDOW uiwin = (UIWINDOW)stdalloc(sizeof(*uiwin));
	memset(uiwin, 0, sizeof(*uiwin));
	uiw_win(uiwin) = win;
	uiw_rows(uiwin) = rows;
	uiw_cols(uiwin) = cols;
	return uiwin;
}
/*==========================================
 * create_boxed_newwin2 -- Create a window with
 *  an auxialiary box window outside it
 * Created: 2002/02/17, Perry Rapp
 *========================================*/
static UIWINDOW
create_boxed_newwin2 (INT rows, INT cols)
{
	INT begy = (LINES - rows)/2;
	INT begx = (COLS - cols)/2;
	WINDOW * boxwin = newwin(rows, cols, begy, begx);
	WINDOW * win=0;
	UIWINDOW uiwin=0;
	++begy;
	++begx;
	win = subwin(boxwin, rows-2, cols-2, begy, begx);
	uiwin = create_uiwindow_impl(win, rows-2, cols-2);
	uiw_boxwin(uiwin) = boxwin;
	return uiwin;
}
/*==========================================
 * delete_uiwindow -- Delete WINDOW wrapper & contents
 * Created: 2002/01/23
 *========================================*/
static void
delete_uiwindow (UIWINDOW uiw)
{
	delwin(uiw_win(uiw));
	stdfree(uiw);
}
/*==========================================
 * create_newwin -- Create our WINDOW wrapper
 * Created: 2001/11/24, Perry Rapp
 *========================================*/
static UIWINDOW
create_newwin (INT rows, INT cols, INT begy, INT begx)
{
	WINDOW * win = newwin(rows, cols, begy, begx);
	return create_uiwindow_impl(win,rows,cols);
}
/*==========================================
 * create_newwin2 -- Create our WINDOW wrapper
 * Created: 2001/11/24, Perry Rapp
 *========================================*/
static UIWINDOW
create_newwin2 (INT rows, INT cols)
{
	WINDOW * win = NEWWIN(rows, cols);
	return create_uiwindow_impl(win,rows,cols);
}
/*==========================================
 * create_uisubwindow -- Create our WINDOW wrapper
 *  for a true (& permanent) subwindow
 * Created: 2001/11/24, Perry Rapp
 *========================================*/
static UIWINDOW
create_uisubwindow (UIWINDOW parent, INT rows, INT cols, INT begy, INT begx)
{
	WINDOW * win = subwin(uiw_win(parent), rows, cols, begy, begx);
	UIWINDOW uiwin = create_uiwindow_impl(win, rows, cols);
	uiw_parent(uiwin) = parent;
	uiw_permsub(uiwin) = TRUE;
	return uiwin;
}
/*==========================================
 * create_windows -- Create and init windows
 *========================================*/
void
create_windows (void)
{
	INT col;
	stdout_win = create_boxed_newwin2(ll_lines-4, ll_cols-4);
	scrollok(uiw_win(stdout_win), TRUE);
	col = COLS/4;
	debug_box_win = create_newwin(8, ll_cols-col-2, 1, col);
	debug_win = create_uisubwindow(debug_box_win, 6, ll_cols-col-4, 2, col+1);
	scrollok(uiw_win(debug_win), TRUE);

	MAINWIN_WIDTH = ll_cols;
	LISTWIN_WIDTH = ll_cols-7;
 	main_win = create_newwin2(ll_lines, MAINWIN_WIDTH);
	tt_menu_win = create_newwin2(11,66);
	ask_win = create_newwin2(4, 73);
	ask_msg_win = create_newwin2(5, 73);
	choose_from_list_win = create_newwin2(15, 73);

	/* cset_menu_win is drawn dynamically */
	/* rpt_cset_menu_win is drawn dynamically */
	/* trans_menu_win is drawn dynamically */
	/* tt_menu_win is drawn dynamically */
	draw_win_box(uiw_win(ask_win));
	draw_win_box(uiw_win(ask_msg_win));
	draw_win_box(uiw_win(debug_box_win));
}
/*=================================
 * display_screen -- 
 * There are six screens that all use
 * the main_win. MAIN_SCREEN is the
 * intro/main menu. The other 6 are all
 * browse screens.
 * cur_screen tells which is active.
 *===============================*/
void
display_screen (INT new_screen)
{
	UIWINDOW uiwin = main_win;
	WINDOW * win = uiw_win(uiwin);
	cur_screen = new_screen;
	check_stdout();
	if (!status_showing[0] || status_transitory)
		place_std_msg();
	else
		mvwaddstr(win, ll_lines-2, 2, status_showing);
	place_cursor();
	switch_to_uiwin(uiwin);
}
/*=====================================
 * check_stdout -- Pause for stdout/err display
 *  if it is up
 * Created: 2001/12/28 (Perry Rapp)
 *===================================*/
static void
check_stdout (void)
{
	if (active_uiwin == stdout_win) {
		if (stdout_vis) {
			llwprintf("\nStrike any key to continue.\n");
			crmode();
			(void) wgetch(uiw_win(stdout_win));
			nocrmode();
			stdout_vis = FALSE;
			/* ok the status string was available until they struck a key */
			clear_status();
		}
		deactivate_uiwin_and_touch_all();
	}
}
/*=====================================
 * main_menu -- Handle main_menu screen
 *===================================*/
void
main_menu (void)
{
	INT c;
	UIWINDOW uiwin = main_win;
	WINDOW * win = uiw_win(uiwin);
	repaint_main_menu(uiwin);
	display_screen(MAIN_SCREEN);
	c = interact(uiwin, "bsadprctuxq", -1);
	place_std_msg();
	wrefresh(win);
	switch (c) {
	case 'b': browse(NULL, BROWSE_INDI); break;
	case 's':
		{
			RECORD rec = invoke_scan_menu();
			if (rec)
				browse(nztop(rec), BROWSE_UNK);
		}
		break;
	case 'a': 
		{
			NODE node;
			if (readonly) {
				msg_error(_(qSronlya));
				break;
			}
			node = invoke_add_menu();
			if (node)
				browse(node, BROWSE_UNK);
		}
		break;
	case 'd':
		{
			if (readonly) {
				msg_error(_(qSronlyr));
				break;
			}
			invoke_del_menu();
		}
		break;
	case 'p': run_report(TRUE); break;
	case 'r': run_report(FALSE); break;
	case 'c': invoke_cset_menu(); break;
	case 't': edit_tt_menu(); break;
	case 'u': invoke_utils_menu(); break;
	case 'x': 
		c = invoke_extra_menu();
		if (c != BROWSE_QUIT)
			browse(NULL, c);
		break;
	case 'q': alldone = TRUE; break;
	}
}
/*=========================================
 * run_report -- run a report program
 *  picklist:  [IN]  display list of reports to user ?
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
void
run_report (BOOLEAN picklist)
{
	/*
	Begin/End action doesn't work because the llwprintf statements
	have a lot of embedded carriage returns
	*/
/*	begin_action();*/
	interp_main(picklist);
	if (!status_showing[0] && length_list(msg_list)<8)
		clear_msgs();
/*	end_action();*/
}
/*=========================================
 * update_menu -- redraw menu if needed
 *  uses new menus
 * Created: 2001/02/01, Perry Rapp
 *=======================================*/
static INT
update_menu (INT screen)
{
	INT lines = calculate_screen_lines(screen);
	if (menu_dirty || (cur_screen != screen))
		repaint_footer_menu(screen);
	menu_dirty = FALSE;
	return lines;
}
/*=========================================
 * show_indi -- Show indi according to mode
 *  uiwin:  [IN]  where to display
 *  indi:   [IN]  whom to display
 *  mode:   [IN]  how to display (eg, traditional, gedcom, ...)
 *  rect:   [IN]  rectangular area in which to display
 *  scroll: [I/O] how far down display is scrolled
 *  reuse:  [IN]  flag to save recalculating display strings
 *=======================================*/
void
show_indi (UIWINDOW uiwin, NODE indi, INT mode, LLRECT rect
	, INT * scroll, BOOLEAN reuse)
{
	CACHEEL icel;
	icel = indi_to_cacheel_old(indi);
	lock_cache(icel);
	if (mode=='g')
		show_gedcom(uiwin, indi, GDVW_NORMAL, rect, scroll, reuse);
	else if (mode=='x')
		show_gedcom(uiwin, indi, GDVW_EXPANDED, rect, scroll, reuse);
	else if (mode=='t')
		show_gedcom(uiwin, indi, GDVW_TEXT, rect, scroll, reuse);
	else if (mode=='a')
		show_ancestors(uiwin, indi, rect, scroll, reuse);
	else if (mode=='d')
		show_descendants(uiwin, indi, rect, scroll, reuse);
	else
		show_indi_vitals(uiwin, indi, rect, scroll, reuse);
	unlock_cache(icel);
}
/*=========================================
 * show_fam -- Show family
 * [in] fam:  whom to display
 * [in] mode:  how to display
 * [in] row:   starting row to use
 * [in] hgt:   how many rows allowed
 * [in] width: how many columns allowed
 * [in] reuse: flag to save recalculating display strings
 *=======================================*/
static void
show_fam (UIWINDOW uiwin, NODE fam, INT mode, INT row, INT hgt
	, INT width, INT * scroll, BOOLEAN reuse)
{
	struct llrect_s rect;
	CACHEEL fcel;
	fcel = fam_to_cacheel(fam);
	lock_cache(fcel);
	rect.top = row;
	rect.bottom = row+hgt-1;
	rect.left = 1;
	rect.right = width-1;
	if (mode=='g')
		show_gedcom(uiwin, fam, GDVW_NORMAL, &rect, scroll, reuse);
	else if (mode=='x')
		show_gedcom(uiwin, fam, GDVW_EXPANDED, &rect, scroll, reuse);
	else
		show_fam_vitals(uiwin, fam, row, hgt, width, scroll, reuse);
	unlock_cache(fcel);
}
/*=========================================
 * display_indi -- Paint indi on-screen
 *=======================================*/
void
display_indi (NODE indi, INT mode, BOOLEAN reuse)
{
	INT screen = ONE_PER_SCREEN;
	INT lines = update_menu(screen);
	struct llrect_s rect;
	/* leave room for box all around */
	rect.top = 1;
	rect.bottom = lines;
	rect.left = 1;
	rect.right = MAINWIN_WIDTH-2;
	show_indi(main_win, indi, mode, &rect, &Scroll1, reuse);
	display_screen(screen);
}
/*=========================================
 * interact_indi -- Get menu choice for indi browse
 *=======================================*/
INT
interact_indi (void)
{
	INT screen = ONE_PER_SCREEN;
	return interact(main_win, NULL, screen);
}
/*=======================================
 * display_fam -- Paint fam on-screen
 *=====================================*/
void
display_fam (NODE fam, INT mode, BOOLEAN reuse)
{
	INT width = MAINWIN_WIDTH;
	INT screen = ONE_FAM_SCREEN;
	INT lines = update_menu(screen);
	show_fam(main_win, fam, mode, 1, lines, width, &Scroll1, reuse);
	display_screen(screen);
}
/*=========================================
 * interact_fam -- Get menu choice for indi browse
 *=======================================*/
INT
interact_fam (void)
{
	INT screen = ONE_FAM_SCREEN;
	return interact(main_win, NULL, screen);
}
/*=============================================
 * display_2indi -- Paint tandem indi screen
 *===========================================*/
void
display_2indi (NODE indi1, NODE indi2, INT mode)
{
	INT screen = TWO_PER_SCREEN;
	INT lines = update_menu(screen);
	INT lines1,lines2;
	BOOLEAN reuse = FALSE; /* can't reuse display strings in tandem */
	struct llrect_s rect;
	lines--; /* for tandem line */
	lines2 = lines/2;
	lines1 = lines - lines2;

	rect.top = 1;
	rect.bottom = lines1;
	rect.left = 1;
	rect.right = MAINWIN_WIDTH-2;

	show_indi(main_win, indi1, mode, &rect, &Scroll1, reuse);
	show_tandem_line(main_win, lines1+1);
	switch_scrolls();
	rect.top = lines1+2;
	rect.bottom = lines+1;
	show_indi(main_win, indi2, mode, &rect, &Scroll1, reuse);
	switch_scrolls();

	display_screen(screen);
}
/*=========================================
 * interact_2indi -- Get menu choice for tandem indi
 *=======================================*/
INT
interact_2indi (void)
{
	INT screen = TWO_PER_SCREEN;
	return interact(main_win, NULL, screen);
}
/*====================================
 * show_tandem_line -- Display horizontal line between top & bottom
 * PR 1999/03
 *==================================*/
static void
show_tandem_line (UIWINDOW win, INT row)
{
	show_horz_line(win, row, 0, ll_cols);
}
/*=============================================
 * display_2fam -- Paint tandem families
 *===========================================*/
void
display_2fam (NODE fam1, NODE fam2, INT mode)
{
	UIWINDOW uiwin = main_win;
	INT width=MAINWIN_WIDTH;
	INT screen = TWO_FAM_SCREEN;
	INT lines = update_menu(screen);
	INT lines1,lines2;
	BOOLEAN reuse = FALSE; /* can't reuse display strings in tandem */
	lines--; /* for tandem line */
	lines2 = lines/2;
	lines1 = lines - lines2;

	show_fam(uiwin, fam1, mode, 1, lines1, width, &Scroll1, reuse);
	show_tandem_line(main_win, lines1+1);
	switch_scrolls();
	show_fam(uiwin, fam2, mode, lines1+2, lines2, width, &Scroll1, reuse);
	switch_scrolls();

	display_screen(screen);
}
/*=========================================
 * interact_2fam -- Get menu choice for tandem fam
 *=======================================*/
INT
interact_2fam (void)
{
	INT screen = TWO_FAM_SCREEN;
	return interact(main_win, NULL, screen);
}
/*=======================================
 * aux_browse -- Handle aux_browse screen
 * This is used for browsing S, E, or X records.
 * Implemented: 2001/01/27, Perry Rapp
 *=====================================*/
INT
aux_browse (NODE node, INT mode, BOOLEAN reuse)
{
	UIWINDOW uiwin = main_win;
	INT screen = AUX_SCREEN;
	INT lines = update_menu(screen);
	struct llrect_s rect;
	rect.top = 1;
	rect.bottom = lines;
	rect.left = 1;
	rect.right = MAINWIN_WIDTH-1;
	show_aux(uiwin, node, mode, &rect,  &Scroll1, reuse);
	display_screen(screen);
	return interact(uiwin, NULL, screen);
}
/*=========================================
 * list_browse -- Handle list_browse screen
 *  cur & pindi are passed for GUI doing
 *  direct navigation in list
 *  this curses implementation does not use them
 *=======================================*/
INT
list_browse (INDISEQ seq,
             INT top,
             INT * cur,
             INT mark,
             NODE * pindi)
{
	pindi=pindi; /* unused */
	if (cur_screen != LIST_SCREEN) paint_list_screen();
	show_big_list(seq, top, *cur, mark);
	display_screen(LIST_SCREEN);
	return interact(main_win, "jkeimdtbanx$^fbq", -1);
}
/*======================================
 * ask_for_db_filename -- Ask user for lifelines database directory
 *  ttl:   [IN]  title of question (1rst line)
 *  prmpt: [IN]  prompt of question (2nd line)
 * returns static buffer
 *====================================*/
STRING
ask_for_db_filename (STRING ttl, STRING prmpt, STRING basedir)
{
	basedir=basedir; /* unused */
	/* This could have a list of existing ones like askprogram.c */
	return ask_for_string(ttl, prmpt);
}
/*======================================
 * ask_for_output_filename -- Ask user for filename to which to write
 *  returns static buffer
 *  ttl1:   [IN] title of question (1rst line)
 *  prmpt:  [IN] prompt of question (3rd line)
 *====================================*/
STRING
ask_for_output_filename (STRING ttl, STRING path, STRING prmpt)
{
	/* display current path (truncated to fit) */
	char curpath[120];
	STRING ptr = curpath;
	INT mylen = sizeof(curpath);
	if (mylen > uiw_cols(ask_msg_win)-2)
		mylen = uiw_cols(ask_msg_win)-2;
	ptr[0]=0;
	llstrcatn(&ptr, _(qSiddefpath), &mylen);
	llstrcatn(&ptr, compress_path(path, mylen-1), &mylen);

	return ask_for_string2(ttl, curpath, prmpt);
}
/*======================================
 * ask_for_input_filename -- Ask user for filename from which to read
 *  returns static buffer
 *  ttl1:   [IN] title of question (1rst line)
 *  prmpt:  [IN] prompt of question (3rd line)
 *====================================*/
STRING
ask_for_input_filename (STRING ttl, STRING path, STRING prmpt)
{
	/* display current path (truncated to fit) */
	char curpath[120];
	STRING ptr = curpath;
	INT mylen = sizeof(curpath);
	if (mylen > uiw_cols(ask_msg_win)-2)
		mylen = uiw_cols(ask_msg_win)-2;
	ptr[0]=0;
	llstrcatn(&ptr, _(qSiddefpath), &mylen);
	llstrcatn(&ptr, compress_path(path, mylen-1), &mylen);

	return ask_for_string2(ttl, curpath, prmpt);
}
/*======================================
 * refresh_main -- touch & refresh main or stdout
 *  as appropriate
 *====================================*/
static void
refresh_main (void)
{
	WINDOW *win = stdout_vis ? uiw_win(stdout_win) : uiw_win(main_win);
	wrefresh(win);
}
/*======================================
 * ask_for_string -- Ask user for string
 *  returns static buffer
 *  ttl:   [IN]  title of question (1rst line)
 *  prmpt: [IN]  prompt of question (2nd line)
 * returns static buffer (less than 100 chars)
 * ask_for_string localizes both of its arguments
 *====================================*/
STRING
ask_for_string (STRING ttl, STRING prmpt)
{
	UIWINDOW uiwin = ask_win;
	WINDOW *win = uiw_win(uiwin);
	STRING rv, p;
	uierase(uiwin);
	draw_win_box(win);
	mvwaddstr(win, 1, 1, ttl);
	mvwaddstr(win, 2, 1, prmpt);
	activate_uiwin(uiwin);
	rv = get_answer(uiwin, 2, strlen(prmpt) + 2); /* less than 100 chars */
	deactivate_uiwin_and_touch_all();
	if (!rv) return (STRING) "";
	p = rv;
	while (chartype((uchar)*p) == WHITE)
		p++;
	striptrail(p);
	return p;
}
/*======================================
 * ask_for_string2 -- Ask user for string
 * Two lines of title
 *  returns static buffer
 *  ttl1:   [IN] title of question (1rst line)
 *  ttl2:   [IN] 2nd line of title
 *  prmpt:  [IN] prompt of question (3rd line)
 * returns static buffer (less than 100 chars)
 *====================================*/
STRING
ask_for_string2 (STRING ttl1, STRING ttl2, STRING prmpt)
{
	UIWINDOW uiwin = ask_msg_win;
	WINDOW *win = uiw_win(uiwin);
	STRING rv, p;
	uierase(uiwin);
	draw_win_box(win);
	mvwaddstr(win, 1, 1, ttl1);
	mvwaddstr(win, 2, 1, ttl2);
	mvwaddstr(win, 3, 1, prmpt);
	wrefresh(win);
	rv = get_answer(uiwin, 3, strlen(prmpt) + 2); /* less than 100 chars */
	if (!rv) return (STRING) "";
	p = rv;
	while (chartype((uchar)*p) == WHITE)
		p++;
	striptrail(p);
	refresh_main();
	return p;
}
/*========================================
 * ask_yes_or_no -- Ask yes or no question
 *  ttl:  [IN]  title to display
 *======================================*/
BOOLEAN
ask_yes_or_no (STRING ttl)
{
	STRING ptr;
	INT c = ask_for_char(ttl, _(qSaskynq), _(qSaskynyn));
	for (ptr = _(qSaskyY); *ptr; ptr++) {
		if (c == *ptr) return TRUE;
	}
	return FALSE;
}
/*=========================================================
 * ask_yes_or_no_msg -- Ask yes or no question with message
 *  msg:   [IN]  top line displayed
 *  ttl:   [IN]  2nd line displayed
 *=======================================================*/
BOOLEAN
ask_yes_or_no_msg (STRING msg, STRING ttl)
{
	STRING ptr;
	INT c = ask_for_char_msg(msg, ttl, _(qSaskynq), _(qSaskynyn));
	for (ptr = _(qSaskyY); *ptr; ptr++) {
		if (c == *ptr) return TRUE;
	}
	return FALSE;
}
/*=======================================
 * ask_for_char -- Ask user for character
 *  ttl:   [IN]  1nd line displayed
 *  prmpt: [IN]  2nd line text before cursor
 *  ptrn:  [IN]  List of allowable character responses
 *=====================================*/
INT
ask_for_char (STRING ttl, STRING prmpt, STRING ptrn)
{
	UIWINDOW uiwin = ask_win;
	WINDOW *win = uiw_win(uiwin);
	uierase(uiwin);
	draw_win_box(win);
	mvwaddstr(win, 1, 2, ttl);
	mvwaddstr(win, 2, 2, prmpt);
	wrefresh(win);
	return interact(uiwin, ptrn, -1);
}
/*===========================================
 * ask_for_char_msg -- Ask user for character
 *  msg:   [IN]  top line displayed
 *  ttl:   [IN]  2nd line displayed 
 *  prmpt: [IN]  3rd line text before cursor
 *  ptrn:  [IN]  List of allowable character responses
 *=========================================*/
INT
ask_for_char_msg (STRING msg, STRING ttl, STRING prmpt, STRING ptrn)
{
	UIWINDOW uiwin = ask_msg_win;
	WINDOW *win = uiw_win(uiwin);
	INT rv;
	uierase(uiwin);
	draw_win_box(win);
	mvwaddstr(win, 1, 2, msg);
	mvwaddstr(win, 2, 2, ttl);
	mvwaddstr(win, 3, 2, prmpt);
	wrefresh(win);
	rv = interact(uiwin, ptrn, -1);
	return rv;
}
/*============================================
 * choose_from_array -- Choose from string list
 *  ttl:      [IN] title for choice display
 *  no:       [IN] number of choices
 *  pstrngs:  [IN] array of choices
 *==========================================*/
INT
choose_from_array (STRING ttl, INT no, STRING *pstrngs)
{
	BOOLEAN selecting = TRUE;
	if (!ttl) ttl=_(qSdefttl);
	return choose_or_view_array(ttl, no, pstrngs, selecting, 0, 0);
}
/*============================================
 * choose_from_array_x -- Choose from string list
 *  ttl:      [IN]  title for choice display
 *  no:       [IN]  number of choices
 *  pstrngs:  [IN]  array of choices
 *  detfnc:   [IN]  callback for details about items
 *  param:    [IN]  opaque type for callback
 *==========================================*/
INT
choose_from_array_x (STRING ttl, INT no, STRING *pstrngs, DETAILFNC detfnc
	, void *param)
{
	BOOLEAN selecting = TRUE;
	if (!ttl) ttl=_(qSdefttl);
	return choose_or_view_array(ttl, no, pstrngs, selecting, detfnc, param);
}
/*============================================
 * view_array -- Choose from string list
 *  ttl:      [IN] title for choice display
 *  no:       [IN] number of choices
 *  pstrngs:  [IN] array of choices
 *==========================================*/
void
view_array (STRING ttl, INT no, STRING *pstrngs)
{
	BOOLEAN selecting = FALSE;
	choose_or_view_array(ttl, no, pstrngs, selecting, 0, 0);
}
/*============================================
 * choose_or_view_array -- Implement choose/view from array
 *  ttl:       [IN]  title for choice display
 *  no:        [IN]  number of choices
 *  pstrngs:   [IN]  array of choices
 *  selecting: [IN]  if FALSE then view-only
 *  detfnc:    [IN]  callback for details about items
 *  param:     [IN]  opaque type for callback
 *==========================================*/
static INT
choose_or_view_array (STRING ttl, INT no, STRING *pstrngs, BOOLEAN selecting
	, DETAILFNC detfnc, void *param)
{
	INT rv;
	rv = array_interact(ttl, no, pstrngs, selecting, detfnc, param);
	refresh_main();
	return rv;
}
/*=============================================================
 * handle_list_cmds -- Process choices from list display
 *  This handles moving up & down, adjusting size of detail,
 *  and scrolling detail.
 * Returns -1 if resized window, 1 if handled, 0 if unhandled.
 * Created: 2001/12/01 (Perry Rapp)
 *===========================================================*/
static BOOLEAN
handle_list_cmds (listdisp * ld, INT code)
{
	INT rows = ld->rectList.bottom - ld->rectList.top + 1;
	switch(code) {
	case 'j': /* next item */
	case CMD_KY_DN:
		if (ld->cur < ld->listlen - 1) {
			ld->cur++;
			if (ld->cur >= ld->top + rows)
				ld->top = ld->cur + 1 - rows;
		}
		return TRUE; /* handled */
	case CMD_KY_PGDN:
		if (ld->top + rows < ld->listlen) {
			ld->cur += rows;
			if (ld->cur > ld->listlen - 1)
				ld->cur = ld->listlen - 1;
			ld->top += rows;
		}
		return TRUE; /* handled */
	case '$': /* jump to end of list */
	case CMD_KY_END:
		ld->top = ld->listlen - rows;
		if (ld->top < 0)
			ld->top = 0;
		ld->cur = ld->listlen-1;
		return TRUE; /* handled */
	case 'k': /* previous item */
	case CMD_KY_UP:
		if (ld->cur > 0) {
			ld->cur--;
			if (ld->cur < ld->top)
				ld->top = ld->cur;
		}
		return TRUE; /* handled */
	case CMD_KY_PGUP:
		if (ld->top >= rows) {
			ld->cur -= rows;
			ld->top -= rows;
		} else {
			ld->cur -= ld->top;
			ld->top -= ld->top;
		}
		return TRUE; /* handled */
	case '^': /* jump to top of list */
	case CMD_KY_HOME:
		ld->top = ld->cur = 0;
		return TRUE; /* handled */
	case '(': /* scroll detail area up */
		if (ld->details_scroll)
			ld->details_scroll--;
		return TRUE; /* handled */
	case ')': /* scroll detail area down */
		if (ld->details_scroll<2)
			ld->details_scroll++;
		return 1; /* handled */
	}
	return FALSE; /* unhandled */
}
/*=============================================================
 * handle_popup_list_resize -- Process resizes of popup list
 * In popup list, details & list compete, & menu is fixed
 * Returns TRUE if handled, FALSE if not
 * Created: 2001/12/01 (Perry Rapp)
 *===========================================================*/
static BOOLEAN
handle_popup_list_resize (listdisp * ld, INT code)
{
	switch(code) {
	case '[': /* shrink detail area */
		if (ld->details > ld->details_minhgt) {
			ld->details--;
			ld->rectDetails.bottom--;
			ld->rectList.top--;
			return -1; /* handled & needs resize */
		}
		return 1; /* handled (nothing) */
	case ']': /* enlarge detail area */
		if (!ld->details) {
			ld->details = ld->details_beginhgt;
			ld->rectDetails.bottom = ld->rectDetails.top + ld->details - 1;
			ld->rectList.top += ld->details;
			return TRUE; /* handled */
		}
		else if (ld->details < ld->details_maxhgt) {
			ld->details++;
			ld->rectDetails.bottom++;
			ld->rectList.top++;
			return TRUE; /* handled */
		}
		return TRUE; /* handled (nothing) */
	}
	return FALSE; /* unhandled */
}
/*=============================================================
 * activate_popup_list_uiwin --
 *  Choose list uiwin & activate
 *  listdisp:  [I/O]  client must fill this in
 *    This routine sets the uiwin, height, rows members
 * Created: 2001/12/01, Perry Rapp
 *===========================================================*/
static void
activate_popup_list_uiwin (listdisp * ld)
{
	INT asklen, hgt, rows, waste;
	/* 
	figure out size of window needed, get closest match from choose_win 
	we want rows for items and for details, +2 lines (above/below) if details
	we want 5 rows overhead (top line & title, bottom line then menu then line)
	*/
	asklen = ld->listlen;
	if (ld->details)
		asklen += ld->details+2;
	hgt = asklen+5;

	if (hgt>POPUP_LINES)
		hgt = POPUP_LINES;
	ld->uiwin = create_newwin2(hgt, LISTWIN_WIDTH);
	uiw_dynamic(ld->uiwin) = TRUE; /* delete when finished */
	/* list is below details to nearly bottom */
	ld->rectList.left = 1;
	ld->rectList.right = LISTWIN_WIDTH-2;
	ld->rectList.top = 2;
	if (ld->details) /* leave room for --DETAILS-- & --LIST-- lines */
		ld->rectList.top += 2+ld->details;
	ld->rectList.bottom = hgt-4;
	/* details is from top down as far as #details */
	ld->rectDetails.top = 2;
	if (ld->details) /* leave room for --DETAILS-- line */
		++ld->rectDetails.top;
	ld->rectDetails.bottom = ld->rectDetails.top + ld->details-1;
	ld->rectDetails.left = 1;
	ld->rectDetails.right = LISTWIN_WIDTH-2;
	/* menu is at bottom, single-line */
	ld->rectMenu.top = hgt-2;
	ld->rectMenu.bottom = hgt-2;
	ld->rectMenu.left = 1;
	ld->rectMenu.right = LISTWIN_WIDTH-2;
	ld->details_beginhgt = 4;
	ld->details_maxhgt = POPUP_LINES-10;
	ld->details_minhgt = 0;

	activate_uiwin(ld->uiwin);
	/* ensure cur is on-screen */
	/* (growing detail area can push current off-screen) */
	rows = ld->rectList.bottom + 1 - ld->rectList.top;
	if (ld->cur < ld->top)
		ld->top = ld->cur;
	else if (ld->cur >= ld->top + rows)
		ld->top = ld->cur + 1 - rows;
	/* don't waste space by scrolling end up */
	waste = ld->top + rows - ld->listlen;
	if (waste>0 && ld->top) {
		ld->top -= waste;
		if (ld->top < 0)
			ld->top = 0;
	}
}
/*=============================================================
 * choose_one_from_indiseq -- 
 * Choose a single person from indiseq
 * Returns index of selected item (or -1 if user quit)
 *  ttl:  [IN]  title
 *===========================================================*/
INT
choose_one_from_indiseq (STRING ttl, INDISEQ seq)
{
	return choose_one_or_list_from_indiseq(ttl, seq, FALSE);
}
/*=============================================================
 * choose_one_or_list_from_indiseq -- 
 * Implements the two choose_xxx_from_indiseq
 *  ttl:   [IN]  title/caption for choice list
 *  seq:   [IN]  list from which to choose
 *  multi: [IN]  if true, selecting a sublist
 * returns index of selected (or -1 for quit)
 * Rewritten to allow dynamic resizing (so user can
 *  resize detail area, ie, the [] functions), 2000/12, Perry Rapp
 * Localizes ttl
 *===========================================================*/
static INT
choose_one_or_list_from_indiseq (STRING ttl, INDISEQ seq, BOOLEAN multi)
{
	WINDOW *win=0;
	INT row, done;
	char fulltitle[128];
	INT elemwidth;
	listdisp ld; /* structure used in resizable list displays */
	STRING menu, choices;
	BOOLEAN first=TRUE;

	ASSERT(seq);

	calc_indiseq_names(seq); /* we certainly need the names */
	
	memset(&ld, 0, sizeof(ld));
	ld.listlen = length_indiseq(seq);
	ld.mode = 'n';

	/* TO DO: connect this to menuitem system */
	if (multi) {
		menu = "Commands:  j Move down   k Move up  d Delete   i Select   q Quit";
		choices = "jkiq123456789()[]$^";
	} else {
		menu = "Commands:   j Move down     k Move up    i Select     q Quit";
		choices = "jkdiq123456789()[]$^";
	}

resize_win: /* we come back here if we resize the window */
	activate_popup_list_uiwin(&ld);
	win = uiw_win(ld.uiwin);
	if (first) {
		elemwidth = ld.rectDetails.right - ld.rectDetails.left + 1;
		if (length_indiseq(seq)<50)
			preprint_indiseq(seq, elemwidth, &disp_shrt_rfmt);
		first=FALSE;
	}
	uierase(ld.uiwin);
	draw_win_box(win);
	row = ld.rectMenu.top-1;
	show_horz_line(ld.uiwin, row++, 0, uiw_cols(ld.uiwin));
	mvwaddstr(win, row, ld.rectMenu.left, menu);
	done = FALSE;
	while (!done) {
		INT code=0, ret=0;
		print_list_title(fulltitle, sizeof(fulltitle), &ld, ttl);
		mvwaddstr(win, 1, 1, fulltitle);
		shw_popup_list(seq, &ld);
		wmove(win, row, 11);
		wrefresh(win);
		code = interact(ld.uiwin, choices, -1);
		if (handle_list_cmds(&ld, code))
			continue;
		if (handle_popup_list_resize(&ld, code)) {
			deactivate_uiwin_and_touch_all();
			/* we're going to repick window & activate */
			goto resize_win;
		}
		if (ret == 0) { /* not handled yet */
			switch (code) {
			case 'd':
				if (!multi)
					break;
				delete_indiseq(seq, NULL, NULL, ld.cur);
				if (!(ld.listlen = length_indiseq(seq))) {
					done=TRUE;
					ld.cur = -1;
				}
				if (ld.cur == ld.listlen) ld.cur--;
				if (ld.cur < ld.top) ld.top = ld.cur;
				break;
			case 'i':
			case CMD_KY_ENTER:
				done=TRUE;
				/* ld.cur points to currently selected */
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (ld.listlen < 10 && code - '1' < ld.listlen) {
					done=TRUE;
					ld.cur = code - '1';
				}
				break;
			case 'q':
				done=TRUE;
				ld.cur = -1; /* ld.cur == -1 means cancelled */
				break;
			}
		}
	}
	deactivate_uiwin_and_touch_all();
	
	return ld.cur;
}
/*==========================================================
 * choose_list_from_indiseq -- User chooses subsequence from
 *   person sequence
 * returns input sequence, but may have deleted elements
 * called by both reports & interactive use
 *  ttl:  [IN]  title/caption for choice list
 *  seq:  [I/O] list from which to choose (user may delete items)
 * returns index of where user choose select (or -1 if quit)
 *========================================================*/
INT
choose_list_from_indiseq (STRING ttl, INDISEQ seq)
{
	return choose_one_or_list_from_indiseq(ttl, seq, TRUE);
}
#ifdef HAVE_SETLOCALE
/*======================================
 * choose_sort -- Enter new sort locale
 * Created: 2001/07/21 (Perry Rapp)
 *====================================*/
static void
test_locale_name (void)
{
	STRING str;
	STRING result = 0;
	STRING idsortttl = _("Enter locale name");
	STRING idloc = _("Locale: ");
	BOOLEAN first = TRUE;
	while (!result) {
		str = ask_for_string(idsortttl, idloc);
		if (!str || !str[0]) return;
		result = setlocale(LC_COLLATE, str);
		if (result) {
			char buffer[128];
			sprintpic1(buffer, sizeof(buffer), _("Valid locale: %1"), str);
			msg_info(buffer);
			return;
		}
		if (first) {
			idsortttl = _("Invalid locale, try again");
			first = FALSE;
		}
	}
}
#endif
/*==============================
 * draw_tt_win -- Draw menu for edit translations
 * Created: 2001/07/20 (Perry Rapp)
 *============================*/
static void
draw_tt_win (STRING prompt)
{
	UIWINDOW uiwin = tt_menu_win;
	WINDOW *win = uiw_win(uiwin);
	INT row = 0;
	uierase(uiwin);
	draw_win_box(win);
	row = 1;
	mvwaddstr(win, row++, 2, prompt);
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_tt_edin), MEDIN);
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_tt_ined), MINED);
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_tt_gdin), MGDIN);
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_tt_ingd), MINGD);
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_tt_dsin), MDSIN);
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_tt_inds), MINDS);
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_tt_inrp), MINRP);
	mvwaddstr(win, row++, 4, _(qSmn_ret));
}
/*==============================
 * disp_codeset -- Display code set line
 * including description
 * Created: 2001/08/02
 *============================*/
static void
disp_codeset (UIWINDOW uiwin, INT row, INT col, STRING menuit, INT codeset)
{
	char buff[60];
	WINDOW * win = uiw_win(uiwin);
	int menulen = strlen(menuit);
	int buflen = sizeof(buff)-menulen;
	mvwaddstr(win, row, col, menuit);
	mvwaddstr(win, row, col+menulen, get_codeset_desc(codeset, buff, buflen));
}
/*==============================
 * disp_locale -- Display locale description
 *  uiwin, row, col: [IN]  where to write description
 *  menuit: prefix of description (eg, "GUI locale")
 *  locName: option variable name (eg, "UiLocale")
 * Created: 2001/08/02
 *============================*/
static void
disp_locale (UIWINDOW uiwin, INT row, INT col, STRING menuit, STRING locVarName)
{
	char buff[60];
	WINDOW * win = uiw_win(uiwin);
	int menulen = strlen(menuit);
	int buflen = sizeof(buff)-menulen;
	mvwaddstr(win, row, col, menuit);
	mvwaddstr(win, row, col+menulen, get_sort_desc(buff, buflen, locVarName));
}
/*==============================
 * disp_trans_table_choice -- Display line in
 * translation table menu, & show current info
 * Created: 2001/07/20
 *============================*/
static void
disp_trans_table_choice (UIWINDOW uiwin, INT row, INT col, STRING menuit, INT indx)
{
	TRANTABLE tt = tran_tables[indx];
	char line[120], count[20];
	WINDOW * win = uiw_win(uiwin);
	INT mylen = sizeof(line);
	STRING ptr = line;

	ptr[0] = 0;
	llstrcatn(&ptr, menuit, &mylen);

	if (tt) {
		if (tt->name[0]) {
			llstrcatn(&ptr, "  :  ", &mylen);
			llstrcatn(&ptr, tt->name, &mylen);
		}
		else {
			llstrcatn(&ptr, "     (Unnamed table)", &mylen);
		}
		sprintf(count, " [%d]", tt->total);
		if (mylen > (INT)strlen(count))
			llstrcatn(&ptr, count, &mylen);
	}
	else {
		llstrcatn(&ptr, "     (None)", &mylen);
	}
	mvwaddstr(win, row, col, line);
}
/*==============================
 * invoke_scan_menu -- Handle scan menu
 * Created: c. 2000/12, Perry Rapp
 *============================*/
RECORD
invoke_scan_menu (void)
{
	UIWINDOW uiwin=0;
	RECORD rec=0;
	INT code=0;
	BOOLEAN done=FALSE;

	if (!scan_menu_win) {
		scan_menu_win = create_newwin2(7,66);
		/* paint it for the first & only time (it's static) */
		repaint_scan_menu(scan_menu_win);
	}
	uiwin = scan_menu_win;

	while (!done) {
		activate_uiwin(uiwin);
		wmove(uiw_win(uiwin), 1, 27);
		code = interact(uiwin, "fnrq", -1);

		switch (code) {
		case 'f':
			rec = full_name_scan(_(qSsts_sca_ful));
			if (rec)
				done=TRUE;
			break;
		case 'n':
			rec = name_fragment_scan(_(qSsts_sca_fra));
			if (rec)
				done=TRUE;
			break;
		case 'r':
			rec = refn_scan(_(qSsts_sca_ref));
			if (rec)
				done=TRUE;
			break;
		case 'q': 
			done=TRUE;
			break;
		}
		deactivate_uiwin_and_touch_all();
		if (!done)
			msg_status(_(qSsts_sca_non));
	}
	return rec;
}
/*============================
 * invoke_add_menu -- Handle add menu
 *==========================*/
static NODE
invoke_add_menu (void)
{
	UIWINDOW uiwin=0;
	WINDOW * win=0;
	NODE node=NULL;
	INT code;

	if (!add_menu_win) {
		add_menu_win = create_newwin2(8, 66);
		/* paint it for the first & only time (it's static) */
		repaint_add_menu(add_menu_win);
	}
	uiwin = add_menu_win;
	win = uiw_win(uiwin);

	activate_uiwin(uiwin);
	wmove(win, 1, 27);
	code = interact(uiwin, "pfcsq", -1);
	deactivate_uiwin_and_touch_all();

	switch (code) {
	case 'p':
		node = nztop(add_indi_by_edit());
		break;
	case 'f': add_family(NULL, NULL, NULL); break;
	case 'c': add_child(NULL, NULL); break;
	case 's': add_spouse(NULL, NULL, TRUE); break;
	case 'q': break;
	}
	return node;
}
/*===============================
 * invoke_del_menu -- Handle delete menu
 *=============================*/
void
invoke_del_menu (void)
{
	INT code;
	UIWINDOW uiwin=0;
	WINDOW * win=0;
	if (!del_menu_win) {
		del_menu_win = create_newwin2(8, 66);
		/* paint it for the first & only time (it's static) */
		repaint_delete_menu(del_menu_win);
	}
	uiwin = del_menu_win;
	win = uiw_win(uiwin);

	activate_uiwin(uiwin);
	wmove(win, 1, 30);
	code = interact(uiwin, "csifq", -1);
	deactivate_uiwin_and_touch_all();

	switch (code) {
	case 'c': choose_and_remove_child(NULL, NULL, FALSE); break;
	case 's': choose_and_remove_spouse(NULL, NULL, FALSE); break;
	case 'i': delete_indi(NULL, TRUE); break;
	case 'f': choose_and_remove_family(); break;
	case 'q': break;
	}
}
/*======================================
 * invoke_cset_menu -- Handle character set menu
 *====================================*/
static void
invoke_cset_menu (void)
{
	INT code=0;
	UIWINDOW uiwin=0;
	WINDOW *win=0;
	BOOLEAN done=FALSE;

	if (!cset_menu_win) {
		cset_menu_win = create_newwin2(12,66);
	}
	uiwin = cset_menu_win;
	win = uiw_win(uiwin);
	activate_uiwin(uiwin);

	while (!done) {
		stdout_vis=FALSE;
		touchwin(win);
		repaint_cset_menu(uiwin);
		wrefresh(win);

		wmove(win, 1, strlen(_(qSmn_csttl))+3);
#ifdef NOTYET
		code = interact(uiwin, "Lscluprtq", -1);
#endif
		code = interact(uiwin, "Lsrtq", -1);

		switch (code) {
#ifdef HAVE_SETLOCALE
		case 'L': test_locale_name(); break;
#endif
		case 's': edit_mapping(MSORT); break;
		case 'c': edit_mapping(MCHAR); break;
		case 'l': edit_mapping(MLCAS); break;
		case 'u': edit_mapping(MUCAS); break;
		case 'p': edit_mapping(MPREF); break;
		case 'r': rpt_cset_menu(); break;
		case 't': invoke_trans_menu(); break;
		case 'q': done=TRUE; break;
		}
	}
	deactivate_uiwin_and_touch_all();
}
/*======================================
 * rpt_cset_menu -- Handle report character set menu
 *====================================*/
static void
rpt_cset_menu (void)
{
	INT code;
	UIWINDOW uiwin=0;
	WINDOW *win=0;
	BOOLEAN done=FALSE;

	if (!rpt_cset_menu_win) {
		rpt_cset_menu_win = create_newwin2(7,66);
	}
	uiwin = rpt_cset_menu_win;
	win = uiw_win(uiwin);
	activate_uiwin(uiwin);

	while (!done) {
		stdout_vis=FALSE;
		touchwin(win);
		repaint_rpc_menu(uiwin);
		wrefresh(win);
		wmove(win, 1, strlen(_(qSmn_csttl))+3);
		code = interact(uiwin, "Lrq", -1);

		switch (code) {
#ifdef HAVE_SETLOCALE
		case 'L': test_locale_name(); break;
#endif
		case 'r': edit_mapping(MINRP); break;
		case 'q': done=TRUE; break;
		}
	}
	deactivate_uiwin_and_touch_all();
}
/*======================================
 * invoke_trans_menu -- menu for translation tables
 *====================================*/
static void
invoke_trans_menu (void)
{
	INT code;
	UIWINDOW uiwin=0;
	BOOLEAN done=FALSE;

	if (!trans_menu_win) {
		trans_menu_win = create_newwin2(10,66);
	}
	uiwin = trans_menu_win;

	while (!done) {
		stdout_vis=FALSE;
		repaint_trans_menu(uiwin);
		reactivate_uiwin(uiwin);
		wmove(uiw_win(uiwin), 1, strlen(_(qSmn_tt_ttl))+3);
		code = interact(uiwin, "elsxiq", -1);

		begin_action();
		switch (code) {
		case 'e': edit_tt_menu(); break;
		case 'l': load_tt_action(); break;
		case 's': save_tt_action(); break;
		case 'x': export_tts(); break;
		case 'i': import_tts(); break;
		case 'q': done=TRUE; break;
		}
		end_action(); /* displays any errors that happened */
	}
	deactivate_uiwin_and_touch_all();
}
/*======================================
 * edit_tt_menu -- menu for "Edit translation table"
 *====================================*/
static void
edit_tt_menu (void)
{
	INT ttnum;
	while ((ttnum = choose_tt(_(qSmn_edttttl))) != -1) {
		edit_mapping(ttnum);
		stdout_vis = FALSE; /* don't need to see errors after done */
	}
}
/*======================================
 * load_tt_action -- menu for "Load translation table"
 *====================================*/
static void
load_tt_action (void)
{
	FILE * fp;
	STRING fname=0;
	INT ttnum;
	STRING ttimportdir;

	if (readonly) {
		msg_error(_(qSronlye));
		return;
	}

	/* Ask which table */
	ttnum = choose_tt(_(qSmn_svttttl));
	if (ttnum == -1) return;
	if (ttnum < 0 || ttnum >= NUM_TT_MAPS) {
		msg_error(_(qSbadttnum));
		return;
	}

	/* Ask whence to load it */
	ttimportdir = getoptstr("LLTTREF", ".");
	fp = ask_for_input_file(LLREADTEXT, _(qSmintt), &fname, ttimportdir, ".tt");
	if (fp) {
		fclose(fp);
		/* Load it */
		if (!load_new_tt(fname, ttnum))
			msg_error(_(qSdataerr));
	}
	if (fname)
		stdfree(fname);
}
/*======================================
 * save_tt_action -- save a translation table
 * to a file
 *====================================*/
static void
save_tt_action (void)
{
	FILE * fp;
	STRING fname=0;
	INT ttnum;
	STRING ttexportdir;
	
	/* Ask which table */
	ttnum = choose_tt(_(qSmn_svttttl));
	if (ttnum == -1) return;
	if (ttnum < 0 || ttnum >= NUM_TT_MAPS) {
		msg_error(_(qSbadttnum));
		return;
	}
	if (!tran_tables[ttnum]) {
		msg_error(_(qSnosuchtt));
		return;
	}
	/* Ask whither to save it */
	ttexportdir = getoptstr("LLTTEXPORT", ".");
	fp = ask_for_output_file(LLWRITETEXT, _(qSmouttt), &fname, ttexportdir, ".tt");
	if (fp) {
		fclose(fp);
		/* Save it */
		if (!save_tt_to_file(ttnum, fname)) {
			msg_error(_(qSdataerr));
			return;
		}
	}
	if (fname)
		stdfree(fname);
}
/*======================================
 * import_tts -- import translation tables
 *====================================*/
static void
import_tts (void)
{
	msg_error(_(qSmn_notimpl));
}
/*======================================
 * export_tts -- export translation tables
 *====================================*/
static void
export_tts (void)
{
	msg_error(_(qSmn_notimpl));
}
/*======================================
 * choose_tt -- select a translation table (-1 for none)
 *====================================*/
static INT
choose_tt (STRING prompt)
{
	INT code;
	UIWINDOW uiwin = tt_menu_win;
	while (1) {
		draw_tt_win(prompt);
		activate_uiwin(uiwin);
		wmove(uiw_win(uiwin), 1, strlen(prompt)+3);
		code = interact(uiwin, "emixgdrq", -1);
		deactivate_uiwin_and_touch_all();
		switch (code) {
		case 'e': return MEDIN;
		case 'm': return MINED;
		case 'i': return MGDIN;
		case 'x': return MINGD;
		case 'g': return MDSIN;
		case 'd': return MINDS;
		case 'r': return MINRP;
		case 'q': return -1;
		}
	}
}
/*====================================
 * invoke_utils_menu -- Handle utilities menu
 *==================================*/
static void
invoke_utils_menu (void)
{
	INT code;
	UIWINDOW uiwin=0;
	WINDOW *win=0;

	if (!utils_menu_win) {
		utils_menu_win = create_newwin2(12, 66);
		/* paint it for the first & only time (it's static) */
		repaint_utils_menu(utils_menu_win);
	}
	uiwin = utils_menu_win;
	win = uiw_win(uiwin);
	activate_uiwin(uiwin);

	wmove(win, 1, strlen(_(qSmn_uttl))+3);
	code = interact(uiwin, "srkidmeoq", -1);
	deactivate_uiwin_and_touch_all();

	begin_action();
	switch (code) {
	case 's': archive_in_file(); break;
	case 'r': import_from_file(); break;
	case 'k': key_util(); break;
	case 'i': who_is_he_she(); break;
	case 'd': show_database_stats(); break;
	case 'm': display_cache_stats(); break;
	case 'e': edit_valtab("VPLAC", &placabbvs, ':', _(qSabverr)); break;
	case 'o': user_options(); break;
	case 'q': break;
	}
	end_action();
}
/*================================
 * invoke_extra_menu -- Handle extra menu
 *==============================*/
static INT
invoke_extra_menu (void)
{
	INT code;
	UIWINDOW uiwin=0;
	WINDOW *win=0;

	if (!extra_menu_win) {
		extra_menu_win = create_newwin2(13,66);
		/* paint it for the first & only time (it's static) */
		repaint_extra_menu(extra_menu_win);
	}
	uiwin = extra_menu_win;
	win = uiw_win(uiwin);

	while (1) {
		activate_uiwin(uiwin);
		wmove(win, 1, strlen(_(qSmn_xttl))+3);
		code = interact(uiwin, "sex123456q", -1);
		deactivate_uiwin_and_touch_all();

		switch (code) {
		case 's': return BROWSE_SOUR;
		case 'e': return BROWSE_EVEN;
		case 'x': return BROWSE_AUX;
		case '1': add_source(); return BROWSE_QUIT;
		case '2': edit_source(NULL); return BROWSE_QUIT;;
		case '3': add_event(); return BROWSE_QUIT;;
		case '4': edit_event(NULL); return BROWSE_QUIT;;
		case '5': add_other(); return BROWSE_QUIT;;
		case '6': edit_other(NULL); return BROWSE_QUIT;;
		case 'q': return BROWSE_QUIT;;
		}
	}
}
/*===============================
 * user_options -- Edit user options
 * Created: 2001/08/02 (Perry Rapp)
 *=============================*/
static void
user_options (void)
{
	edit_valtab("VUOPT", &useropts, '=', _(qSuoperr));
	update_useropts();
}
/*===============================
 * translate_hdware_key -- 
 *  translate curses keycode into menuitem.h constant
 *=============================*/
struct hdkeycvt { int key; int cmd; };
static INT
translate_hdware_key (INT c)
{
	static struct hdkeycvt hdkey[] = {
		{ KEY_UP, CMD_KY_UP }
		, { KEY_DOWN, CMD_KY_DN }
		, { KEY_NPAGE, CMD_KY_PGDN }
		, { KEY_PPAGE, CMD_KY_PGUP }
		, { KEY_HOME, CMD_KY_HOME }
		, { KEY_END, CMD_KY_END }
		, { KEY_ENTER, CMD_KY_ENTER }
	};
	int i;
	for (i=0; i<ARRSIZE(hdkey); ++i) {
		if (c == hdkey[i].key)
			return hdkey[i].cmd;
	}
	return CMD_NONE;
}
/*===============================
 * translate_control_key -- 
 *  translate control keys into menuitem.h constant
 *=============================*/
static INT
translate_control_key (INT c)
{
	static struct hdkeycvt hdkey[] = {
		{ '\r', CMD_KY_ENTER } /* Win32 */
		, { '\n', CMD_KY_ENTER } /* UNIX */
	};
	int i;
	for (i=0; i<ARRSIZE(hdkey); ++i) {
		if (c == hdkey[i].key)
			return hdkey[i].cmd;
	}
	return CMD_NONE;
}
/*===============================
 * interact -- Interact with user
 *=============================*/
static INT
interact (UIWINDOW uiwin, STRING str, INT screen)
{
	char buffer[4]; /* 3 char cmds max */
	INT offset=0;
	INT cmdnum;
	INT c, i, n = str ? strlen(str) : 0;
	while (TRUE) {
		crmode();
		keypad(uiw_win(uiwin),1);
		c = wgetch(uiw_win(uiwin));
		if (c == EOF) c = 'q';
		nocrmode();
		if (!progrunning && !lock_std_msg) {
			/* after they chose off the menu, we wipe any
			status message still lingering from before they chose */
			if (status_showing[0]) {
				status_showing[0] = 0;
				place_std_msg();
			}
		}
		if (c<0x20) {
			return translate_control_key(c);
		}
		if (has_key(c)) {
			return translate_hdware_key(c);
		}
		if (str) { /* traditional */
			for (i = 0; i < n; i++) {
				if (c == (uchar)str[i]) return c;
			}
		} else { /* new menus (in menuitem.c) */
			if (offset < (INT)sizeof(buffer)-1) {
				buffer[offset] = c;
				buffer[offset+1] = 0;
				offset++;
			} else {
				buffer[0] = c;
				buffer[1] = 0;
				offset = 1;
			}
			cmdnum = menuitem_check_cmd(screen, buffer);
			if (cmdnum != CMD_NONE && cmdnum != CMD_PARTIAL)
				return cmdnum;
			if (cmdnum != CMD_PARTIAL) {
				msg_error(_(qSmn_unkcmd));
				offset = 0;
			}
		}
		/* choice was no good, we loop & wait for another choice */
	}
}
/*============================================
 * get_answer -- Have user respond with string
 *  uiwin:   [IN] which window to use
 *  row:     [IN]  prompt location (vert)
 *  col:     [IN]  prompt location (horiz)
 *  returns static buffer <=MAXPATHLEN length
 *==========================================*/
STRING
get_answer (UIWINDOW uiwin, INT row, INT col)
{
	static char lcl[MAXPATHLEN];
	WINDOW *win = uiw_win(uiwin);
	INT len=sizeof(lcl);
	if (len > uiw_cols(uiwin)-col-1)
		len = uiw_cols(uiwin)-col-1;

	echo();
#ifdef HAVE_LIBNCURSES
	mvwgetnstr(win, row, col, lcl, len);
#else
	wmove(win, row, col);
	wgetnstr(win, lcl, len);
#endif
	noecho();

	return lcl;
}
/*=====================================================
 * shw_popup_list -- Draw list & details of popup list
 *===================================================*/
static void
shw_popup_list (INDISEQ seq, listdisp * ld)
{
	WINDOW *win = uiw_win(ld->uiwin);
	ASSERT(ld->listlen == length_indiseq(seq));
	if (ld->details) {
		INT row = ld->rectDetails.top-1;
		clear_hseg(win, row, ld->rectDetails.left, ld->rectDetails.right);
		mvwaddstr(win, row, 2, "--- CURRENT SELECTION ---");
		shw_recordlist_details(seq, ld);
		row = ld->rectDetails.bottom+1;
		mvwaddstr(win, row, ld->rectDetails.left, "--- LIST ---");
	}
	shw_recordlist_list(seq, ld);
}
/*=====================================================
 * shw_recordlist_details -- Draw record details for a list
 * For either popup list or full-screen list (list browse)
 *===================================================*/
static void
shw_recordlist_details (INDISEQ seq, listdisp * ld)
{
	WINDOW *win = uiw_win(ld->uiwin);
	INT i;
	STRING key, name;
	BOOLEAN reuse=FALSE; /* don't reuse display strings in list */
	for (i=ld->rectDetails.top; i<=ld->rectDetails.bottom; ++i) {
		clear_hseg(win, i, ld->rectDetails.left, ld->rectDetails.right-10);
	}
	element_indiseq(seq, ld->cur, &key, &name);
	show_record(ld->uiwin, key, ld->mode, &ld->rectDetails, &ld->details_scroll
		, reuse);
}
/*=====================================================
 * shw_recordlist_list -- Draw actual list items
 * For either popup list or full-screen list (list browse)
 *===================================================*/
static void
shw_recordlist_list (INDISEQ seq, listdisp * ld)
{
	WINDOW *win = uiw_win(ld->uiwin);
	INT width = (ld->rectList.right - ld->rectList.left + 1) - 4;
	INT rows = ld->rectList.bottom - ld->rectList.top + 1;
	INT i, j, row;
	INT offset=4;
	char buffer[160];
	/* for short lists, use leading numbers */
	if (ld->listlen < 10) {
		sprintf(buffer, "%d: ", ld->listlen);
		i = strlen(buffer);
		width -= i; /* for "1: " */
		offset += i;
	}
	if (width > (INT)sizeof(buffer)-1)
		width = sizeof(buffer)-1;
	for (j=0; j<rows; j++) {
		/* j is zero-based iterator */
		/* i is actual offset into indiseq */
		i = ld->top + j;
		/* row is row on screen */
		row = ld->rectList.top + j;
		clear_hseg(win, row, ld->rectList.left, ld->rectList.right);
		if (i<ld->listlen) {
			if (i == ld->cur) mvwaddch(win, row, ld->rectList.left+3, '>');
			if (ld->listlen < 10) {
				char numstr[12];
				sprintf(numstr, "%d:", i+1);
				mvwaddstr(win, row, ld->rectList.left+4, numstr);
			}
			print_indiseq_element(seq, i, buffer, width, &disp_shrt_rfmt);
			mvwaddstr(win, row, ld->rectList.left+offset, buffer);
		}
	}
}
static STRING empstr49 = (STRING) "                                                 ";
/*==========================================
 * show_big_list - Show name list in list screen
 *========================================*/
void
show_big_list (INDISEQ seq,
           INT top,
           INT cur,
           INT mark)
{
/*
TODO: To be resizable like popup list, need a listdisp structure,
	and need to repaint that RHS menu, as its height will vary
	just to be scrollable details doesn't require repainting the RHS menu
But in any case the real problem is that 
show_big_list (screen.c) is called by list_browse (screen.c)
which is called by browse_list (lbrowse.c!), and it handles menus
and listdisp is local to screen.c right now, so browse_list can't have one
A solution would be to pass in what is known from browse_list, and then
manufacture a listdisp here
- Perry, 2002/01/01
*/
	UIWINDOW uiwin = main_win;
	WINDOW *win = uiw_win(uiwin);
	INT i, j, row, len = length_indiseq(seq);
	STRING key, name;
	NODE indi;
	char scratch[200], *p;
	TRANTABLE ttd = tran_tables[MINDS];
	INT mode = 'n';
	INT viewlines = 13;
	
	for (i = LIST_LINES+2; i < LIST_LINES+2+viewlines; i++)
		mvwaddstr(win, i, 1, empstr49);
	row = LIST_LINES+2;
	for (i = top, j = 0; j < viewlines && i < len; i++, j++) {
		element_indiseq(seq, i, &key, &name);
		indi = key_to_indi(key);
		if (i == mark) mvwaddch(win, row, 2, 'x');
		if (i == cur) {
			INT drow=1;
			INT scroll=0;
			BOOLEAN reuse=FALSE;
			struct llrect_s rectList;
			rectList.top = drow;
			rectList.bottom = drow + LIST_LINES-1;
			rectList.left = 1;
			rectList.right = MAINWIN_WIDTH-2;
			mvwaddch(win, row, 3, '>');
			show_record(main_win, key, mode, &rectList, &scroll, reuse);
		}
		name = manip_name(name, ttd, TRUE, TRUE, 40);
		strcpy(scratch, name);
		p = scratch + strlen(scratch);
		*p++ = ' ';
		sprintf(p, "(%s)", key_of_record(indi, ttd));
		/*sprintf(p, "(%s)", &key[1]);*/
		mvwaddstr(win, row, 4, scratch);
		row++;
	}
}
/*================================================================
 * show_record -- Display record (any type) in requested mode
 *  uiwin:  [IN]  whither to draw
 *  key:    [IN]  key of record to display
 *  mode:   [IN]  what display mode (eg, vitals vs GEDCOM vs...)
 *  rect:   [IN]  where to draw
 *  scroll: [I/O] current scroll setting
 *  reuse:  [IN]  flag indicating if same record drawn last time
 *==============================================================*/
static void
show_record (UIWINDOW uiwin, STRING key, INT mode, LLRECT rect
	, INT * scroll, BOOLEAN reuse)
{
	INT row = rect->top;
	INT hgt = rect->bottom - rect->top + 1;
	INT width = rect->right - rect->left + 1;
	if (key[0]=='I') {
		NODE indi = key_to_indi(key);
		if (indi)
			show_indi(uiwin, indi, mode, rect, scroll, reuse);
	} else if (key[0]=='F') {
		NODE fam = key_to_fam(key);
		if (fam)
			show_fam(uiwin, fam, mode, row, hgt, width, scroll, reuse);
	} else {
		/* could be S,E,X -- show_aux handles all of these */
		NODE aux = qkey_to_type(key);
		if (aux)
			show_aux(uiwin, aux, mode, rect, scroll, reuse);
	}
}
/*================================================================
 * shw_array_of_strings -- Show string list in list interact window
 *  strings: [IN]  array (of choices) to be listed
 *  ld:      [IN]  structure of info for variable-sized list
 *  detfnc:  [IN]  callback for detail area
 *  param:   [IN]  opaque data for callback
 *==============================================================*/
static void
shw_array_of_strings (STRING *strings, listdisp * ld, DETAILFNC detfnc
	, void * param)
{
	WINDOW *win = uiw_win(ld->uiwin);
	INT i, j, row, lines;
	INT rows = ld->rectList.bottom - ld->rectList.top + 1;
	INT overflag=FALSE;
	char buffer[120];
	INT width = uiw_cols(ld->uiwin);
	if (width > (INT)sizeof(buffer)-1)
		width = sizeof(buffer)-1;
	/* clear current lines */
	lines = rows + (ld->details ? ld->details+2 : 0);
	for (i = 0; i<lines; ++i) {
		row = i+2;
		llstrncpy(buffer, empstr120, width-1);
		mvwaddstr(win, row, 1, buffer);
	}
	row = 2;
	if (ld->details) {
		row = 3+ld->details;
		mvwaddstr(win, row++, 2, "--- LIST ---");
	}
	for (j=0; j<rows;++j) {
		INT nlen=0,temp;
		i = ld->top + j;
		if (i>=ld->listlen)
			break;
		/* for short lists, we show leading numbers */
		if (ld->listlen<10) {
			char numstr[12]="";
			snprintf(numstr, sizeof(numstr), "%d: ", i+1);
			if (i == ld->cur) mvwaddch(win, row, 3, '>');
			mvwaddstr(win, row, 4, numstr);
			nlen = strlen(numstr);
		} else {
			if (i == ld->cur) mvwaddch(win, row, 3, '>');
		}
		temp = width-6-nlen;
		llstrncpy(buffer, strings[i], temp);
		if ((INT)strlen(buffer) > temp-2) {
			if (i==ld->cur)
				overflag=TRUE;
			strcpy(&buffer[temp-3], "...");
		}
		mvwaddstr(win, row, 4+nlen, buffer);
		row++;
	}
	if (ld->details) {
		STRING ptr = strings[ld->cur];
		INT count;
		row = 2;
		mvwaddstr(win, row++, 2, "-- CURRENT SELECTION --");
		for (i=0; i<ld->details; ++i) {
			/* TODO: scroll */
			if (!ptr[0]) break;
			llstrncpy(buffer, ptr, width-5);
			mvwaddstr(win, row++, 4, buffer);
			ptr += strlen(buffer);
		}
		count = ld->details-i;
		if (count && detfnc) {
			/* caller gave us a detail callback, so we set up the
			data needed & call it */
			STRING * linestr = (STRING *)stdalloc(count * sizeof(STRING));
			struct array_details_s dets;
			for (j=0; j<count; ++j) {
				linestr[j] = stdalloc(width);
				linestr[j][0] = 0;
			}
			memset(&dets, 0, sizeof(dets));
			dets.list = strings;
			dets.cur = ld->cur;
			dets.lines = linestr;
			dets.count = 4;
			if (dets.count > ld->details-i)
				dets.count = ld->details-i;
			dets.maxlen = width;
			(*detfnc)(&dets, param);
			for (j=0 ; j<count; ++j) {
				mvwaddstr(win, row++, 4, linestr[j]);
			}
			for (j=0; j<count; ++j)
				stdfree(linestr[j]);
			stdfree(linestr);
		}
	}
}
/*==================================
 * print_list_title -- Print title line of an array list
 * Adds suffix such as (5/11)
 * Truncates title if necessary (leaving room for suffix)
 *  buffer:  [OUT] output string
 *  len:     [IN]  size of buffer
 *  ld:      [IN]  list display structure
 *  ttl:     [IN]  title to print (localized)
 *================================*/
static void
print_list_title (char * buffer, INT len, const listdisp * ld, STRING ttl)
{
	STRING ptr = buffer;
	char suffix[30];
	if (len > uiw_cols(ld->uiwin)-2)
		len = uiw_cols(ld->uiwin)-2;
	sprintf(suffix, " (%d/%d)", ld->cur+1, ld->listlen);
	len -= strlen(suffix)+1; /* reserve room for suffix */
	ptr[0] = 0;
	if ((INT)strlen(ttl)>len-1) {
		len -= 4;
		llstrcatn(&ptr, ttl, &len);
		len += 4;
		llstrcatn(&ptr, "...", &len);
	} else {
		llstrcatn(&ptr, ttl, &len);
	}
	len += strlen(suffix)+1; /* we reserved this room above */
	llstrcatn(&ptr, suffix, &len);
}
/*==============================================
 * array_interact -- Interact with user over list
 *  ttl:        [IN]  title
 *  len:        [IN]  number of choices
 *  strings:    [IN]  array of choices
 *  selectable: [IN]  FALSE for view-only
 *  detfnc:     [IN]  callback for details about items
 *  param:      [IN]  opaque type for callback
 *============================================*/
static INT
array_interact (STRING ttl, INT len, STRING *strings
	, BOOLEAN selectable, DETAILFNC detfnc, void * param)
{
	WINDOW *win=0;
	INT row, done;
	char fulltitle[128];
	STRING responses = len<10 ? "jkiq123456789[]()$^" : "jkiq[]()$^";
	STRING promptline = selectable ? _(qSchlist ): _(qSvwlist);
	listdisp ld; /* structure used in resizable list displays */

	memset(&ld, 0, sizeof(ld));
	ld.listlen = len;
	ld.mode = 'n'; /* irrelevant for array list */

resize_win: /* we come back here if we resize the window */
	activate_popup_list_uiwin(&ld);
	win = uiw_win(ld.uiwin);
	uierase(ld.uiwin);
	draw_win_box(win);
	row = ld.rectMenu.top-1;
	show_horz_line(ld.uiwin, row++, 0, uiw_cols(ld.uiwin));
	mvwaddstr(win, row, ld.rectMenu.left, promptline);
	done = FALSE;
	while (!done) {
		INT code=0, ret=0;
		print_list_title(fulltitle, sizeof(fulltitle), &ld, _(ttl));
		mvwaddstr(win, 1, 1, fulltitle);
		shw_array_of_strings(strings, &ld, detfnc, param);
		wrefresh(win);
		code = interact(ld.uiwin, responses, -1);
		if (handle_list_cmds(&ld, code))
			continue;
		if (handle_popup_list_resize(&ld, code)) {
			deactivate_uiwin_and_touch_all();
			/* we're going to repick window & activate */
			goto resize_win;
		}
		if (ret == 0) { /* not handled yet */
			switch(code) {
			case 'i':
				if (selectable) {
					done=TRUE;
				}
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (len < 10 && selectable && code - '1' < len) {
					done=TRUE;
					ld.cur = code - '1';
				}
				break;
			case 'q':
				done=TRUE;
				ld.cur = -1; /* ld.cur == -1 means cancelled */
			}
		}
	}
	deactivate_uiwin_and_touch_all();
	return ld.cur;
}
/*===================================================
 * message_string -- Return background message string
 *=================================================*/
STRING
message_string (void)
{
	if (!cur_screen) return "";
	if (cur_screen == MAIN_SCREEN)
		return "LifeLines -- Main Menu";
	ASSERT(cur_screen >= 1 && cur_screen <= MAX_SCREEN);
	return g_ScreenInfo[cur_screen].Title;
}
/*=================================================
 * place_std_msg - Place standard message on screen
 *===============================================*/
void
place_std_msg (void)
{
	/* msg is placed on main window */
	UIWINDOW uiwin = main_win;
	WINDOW *win = uiw_win(uiwin);
	STRING str = message_string();
	INT row = ll_lines-2;
	clear_hseg(win, row, 2, ll_cols-2);
	mvwaddstr(win, row, 2, str);
	/* now we need to repaint main window, but if there are
	subwindows up, instead we call the touch_all routine,
	which does them all from ancestor to descendant */
	if (active_uiwin)
		touch_all(TRUE);
	else
		wrefresh(win); 
	place_cursor();
}
/*=================================================
 * llvwprintf -- Called as wprintf(fmt, argp)
 *===============================================*/
void
llvwprintf (STRING fmt, va_list args)
{
	UIWINDOW uiwin = stdout_win;
	WINDOW *win = uiw_win(uiwin);
	if (!stdout_vis) {
		clearw();
		activate_uiwin(uiwin);
	}
	vwprintw(win, fmt, args);
	wrefresh(win);
	/*
	TO DO
	It would be nice to add this to the msg list
	but we need to deal with embedded carriage returns first
	so we can't do this yet. Also, if we do put it on the msg list,
	it is going to duplicate the stdout display currently being
	used (which is nicer looking, but scrolls off-screen).
	*/
/*	msg_outputv(MSG_ERROR, fmt, args);*/ /* also send to msg list */
}
/*=================================================
 * llwprintf -- Called as wprintf(fmt, arg, arg, ...)
 *===============================================*/
void
llwprintf (STRING fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	llvwprintf(fmt, args);
	va_end(args);
}
/*==============================
 * clearw -- Clear stdout window
 *============================*/
void
clearw (void)
{
	UIWINDOW uiwin = stdout_win;
	WINDOW *win = uiw_win(uiwin);
	WINDOW *boxwin = uiw_boxwin(uiwin);
	uierase(uiwin);
	draw_win_box(boxwin);
	wmove(win, 0, 0);
	stdout_vis = TRUE;
	wrefresh(boxwin);
}
/*=======================================
 * wfield -- Write field in stdout window
 *=====================================*/
void
wfield (INT row, INT col, STRING str)
{
	UIWINDOW uiwin = stdout_win;
	WINDOW *win = uiw_win(uiwin);
	if (!stdout_vis) clearw();
	mvwaddstr(win, row, col, str);
	wrefresh(win);
}
/*===========================================
 * wpos -- Position to place in stdout window
 *=========================================*/
void
wpos (INT row, INT col)
{
	UIWINDOW uiwin = stdout_win;
	WINDOW *win = uiw_win(uiwin);
	wmove(win, row, col);
}
/*=======================================
 * show_horz_line -- Draw horizontal line
 *=====================================*/
void
show_horz_line (UIWINDOW uiwin, INT row, INT col, INT len)
{
	WINDOW *win = uiw_win(uiwin);
	INT i;
	mvwaddch(win, row, col, gr_ltee);
	for (i = 0; i < len-2; i++)
		waddch(win, gr_hline);
	waddch(win, gr_rtee);
}
/*=====================================
 * show_vert_line -- Draw vertical line
 *===================================*/
void
show_vert_line (UIWINDOW uiwin, INT row, INT col, INT len)
{
	WINDOW *win = uiw_win(uiwin);
	INT i;
	mvwaddch(win, row++, col, gr_ttee);
	for (i = 0; i < len-2; i++)
		mvwaddch(win, row++, col, gr_vline);
	mvwaddch(win, row, col, gr_btee);
}
/*=============================================
 * place_cursor -- Move to idle cursor location
 *===========================================*/
void
place_cursor (void)
{
	/* TO DO - integrate menuitem version! */
	INT row, col = 30;
	switch (cur_screen) {
	case MAIN_SCREEN:    row = 5;        break;
	case ONE_PER_SCREEN: row = ll_lines-11; break;
	case ONE_FAM_SCREEN: row = ll_lines-9;  break;
	case AUX_SCREEN:     row = AUX_LINES+2;       break;
	case TWO_PER_SCREEN: row = 2*TANDEM_LINES+3;       break;
	case TWO_FAM_SCREEN: row = 2*TANDEM_LINES+3;       break;
	case LIST_SCREEN:    row = LIST_LINES+2; col = 75; break;
	default:             row = 1; col = 1; break;
	}
	wmove(uiw_win(main_win), row, col);
}
/*=============================================
 * dbprintf -- Debug printf(fmt, arg, arg, ...)
 *===========================================*/
void
dbprintf (STRING fmt, ...)
{
	va_list args;
	touchwin(uiw_win(debug_box_win));
	va_start(args, fmt);
	vwprintw(uiw_win(debug_win), fmt, args);
	va_end(args);
	wrefresh(uiw_win(debug_box_win));
	sleep(2);
	touchwin(uiw_win(main_win));
	wrefresh(uiw_win(main_win));
}
/*==================================================
 * do_edit -- Shift to user's screen editor and back
 *================================================*/
void
do_edit (void)
{
	endwin();
#ifdef WIN32
	/* use w32system, because it will wait for the editor to finish */
	w32system(editstr);
#else
	system(editstr);
#endif
	clearok(curscr, 1);
	wrefresh(curscr);
	noecho();
}
/*================================================
 * mvwaddstr_lim -- output a string, like mvwaddstr
 *  except trim it to no more than maxlen wide
 *==============================================*/
static void
mvwaddstr_lim (WINDOW *wp, int x, int y, char *cp, INT maxlen)
{
	char buffer[60];
	if ((INT)strlen(cp)<=maxlen)
		mvwaddstr(wp, x, y, cp);
	else {
		if (maxlen > (INT)sizeof(buffer)-1)
			maxlen = sizeof(buffer)-1;
		llstrncpy(buffer, cp, maxlen-1);
		strcat(buffer, "*");
		mvwaddstr(wp, x, y, buffer);
	}
}
/*================================================
 * output_menu -- print menu array to screen
 * Caller specifies bottom row to use, & width
 * Menu structure contains all menu items & # of
 * columns to use
 *==============================================*/
static void
output_menu (UIWINDOW uiwin, INT screen, INT bottom, INT width)
{
	WINDOW *win = uiw_win(uiwin);
	INT row;
	INT icol=0;
	INT col=3;
	INT MenuRows = g_ScreenInfo[screen].MenuRows;
	INT MenuSize = g_ScreenInfo[screen].MenuSize;
	INT MenuCols = g_ScreenInfo[screen].MenuCols;
	/* reserve 2 spaces at each end, and one space in front of each Col */
	INT colwidth = (width-4)/MenuCols-1;
	INT Item = 0;
	INT page, pageitems, pages;
	/* MenuRows includes the title row but not the horiz line */
	INT top = bottom - (MenuRows+1);
	char prompt[128];
	MenuItem ** Menu = g_ScreenInfo[screen].Menu;
	INT OnePageFlag = 0;
	page = g_ScreenInfo[screen].MenuPage;
	/* reserve 2 slots for q=quit and ?=more */
	pageitems = (MenuRows-1)*MenuCols-2;
	pages = (MenuSize-1)/pageitems+1;
	if (MenuSize <= pageitems+1) /* don't need '?' if they fit */
	{
		OnePageFlag = 1;
		page = 0;
		pages = 1;
	}
	Item = page * pageitems;
	if (Item >= MenuSize)
		Item = ((MenuSize-1)/pageitems)*pageitems;
	icol = 0;
	col = 3;
	row = top;
	/* display line across */
	show_horz_line(uiwin, row++, 0, width);
	/* display title */
	sprintf(prompt, "%s            (pg %d/%d)", 
		_(qSplschs), page+1, pages);
	mvwaddstr(win, row++, 2, prompt);
	/* now display all the menu items we can fit on this page */
	while (1)
	{
		mvwaddstr_lim(win, row, col, Menu[Item++]->LocalizedDisplay, colwidth);
		if (Item == MenuSize)
			break;
		row++;
		if (icol<MenuCols-1 && row==bottom)
		{
			icol++;
			col += colwidth+1;
			row = LINESTOTAL-MenuRows-2;
			continue;
		}
		if (OnePageFlag) {
			/* one slot reserved for "q" */
			if (icol==MenuCols-1 && row==bottom-1)
				break;
		} else {
			/* two slots reserved, "q" & "?" */
			if (icol==MenuCols-1 && row==bottom-2)
				break;
		}
	}
	/* print the "q" and "?" items */
	row = bottom-2;
	col = 3+(MenuCols-1)*(colwidth+1);
	if (!OnePageFlag)
		mvwaddstr_lim(win, row, col, g_MenuItemOther.Display, colwidth);
	mvwaddstr_lim(win, ++row, col, g_MenuItemQuit.Display, colwidth);
}
/*==================================================================
 * toggle_menu() - toggle display of menu
 * Created: 1999/02, Perry Rapp
 * Joined repository: 2001/01/28, Perry Rapp
 *================================================================*/
void
toggle_menu (void)
{
	menu_enabled = !menu_enabled;
	menu_dirty = 1;
}
/*==================================================================
 * cycle_menu() - show other menu choices
 * Created: 1999/03, Perry Rapp
 * Joined repository: 2001/01/28, Perry Rapp
 *================================================================*/
void
cycle_menu (void)
{
	INT MenuSize = g_ScreenInfo[cur_screen].MenuSize;
	INT MenuRows = g_ScreenInfo[cur_screen].MenuRows;
	INT cols = g_ScreenInfo[cur_screen].MenuCols;
	INT pageitems = (MenuRows-1)*cols-2;
	if (pageitems+1 == MenuSize)
		return; /* only one page */
	g_ScreenInfo[cur_screen].MenuPage++;
	if (g_ScreenInfo[cur_screen].MenuPage > (MenuSize-1)/pageitems)
		g_ScreenInfo[cur_screen].MenuPage = 0;
        menu_dirty = 1;
}
/*==================================================================
 * adjust_menu_height() - Change height of menu on person screen
 * Created: 1999/03, Perry Rapp
 * Joined repository: 2001/01/28, Perry Rapp
 *================================================================*/
void
adjust_menu_height (INT delta)
{
	INT min=4, max=10;
	if (g_ScreenInfo[cur_screen].MenuCols == 1)
	{
		min = 5;
		max = 14;
	}
	g_ScreenInfo[cur_screen].MenuRows += delta;
	if (g_ScreenInfo[cur_screen].MenuRows<min)
		g_ScreenInfo[cur_screen].MenuRows=min;
	else if (g_ScreenInfo[cur_screen].MenuRows>max)
		g_ScreenInfo[cur_screen].MenuRows=max;
	menu_dirty = 1;
}
/*==================================================================
 * adjust_menu_cols() - Change # of columsn in current menu
 * Created: 2001/12/10, Perry Rapp
 *================================================================*/
void
adjust_menu_cols (INT delta)
{
	INT min=1, max=7;
	g_ScreenInfo[cur_screen].MenuCols += delta;
	if (g_ScreenInfo[cur_screen].MenuCols<min)
		g_ScreenInfo[cur_screen].MenuCols=min;
	else if (g_ScreenInfo[cur_screen].MenuCols>max)
		g_ScreenInfo[cur_screen].MenuCols=max;
	menu_dirty = 1;
}
/*=========================================
 * calculate_screen_lines -- How many lines above menu?
 * Created: 1999/03, Perry Rapp
 * Joined repository: 2001/01/28, Perry Rapp
 *=======================================*/
static INT
calculate_screen_lines (INT screen)
{
	INT menu = g_ScreenInfo[screen].MenuRows;
	INT lines;
	if (!menu_enabled) menu = EMPTY_MENU;
	lines = LINESTOTAL-OVERHEAD_MENU-menu;
	return lines;
}
/*=====================
 * clear_hseg -- clear a horizontal line segment
 *  (used for partial screen clears)
 *====================*/
void
clear_hseg (WINDOW *win, INT row, INT x1, INT x2)
{
	color_hseg(win, row, x1, x2, ' ');
}
/*=====================
 * color_hseg -- fill a horizontal line segment
 *  (used for clearing)
 *====================*/
static void
color_hseg (WINDOW *win, INT row, INT x1, INT x2, char ch)
{
	INT i;
	for (i=x1; i<=x2; ++i)
		mvwaddch(win, row, i, ch);
}
/*===============================================
 * display_status -- put string in status line
 * We don't touch the status_transitory flag
 * That is caller's responsibility.
 * Created: 2001/11/11, Perry Rapp
 * Does not localize
 *=============================================*/
static void
display_status (STRING text)
{
	UIWINDOW uiwin = main_win;
	WINDOW *win = uiw_win(uiwin);
	INT row;
	/* first store it */
	llstrncpy(status_showing, text, sizeof(status_showing));
	if ((INT)strlen(text)>ll_cols-6) {
		status_showing[ll_cols-8] = 0;
		strcat(status_showing, "...");
	}
	/* then display it */
	row = ll_lines-2;
	clear_hseg(win, row, 2, ll_cols-2);
	wmove(win, row, 2);
	mvwaddstr(win, row, 2, status_showing);
	place_cursor();
	wrefresh(win);
}
/*=========================================
 * msg_error -- handle error message
 * delegates to msg_outputv
 * Created: 2001/11/11, Perry Rapp
 * Does not localize
 *=======================================*/
void
msg_error (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	msg_outputv(MSG_ERROR, fmt, args); 
	va_end(args);
}
/*=========================================
 * msg_info -- handle regular messages
 * delegates to msg_outputv
 * Created: 2001/11/11, Perry Rapp
 * Does not localize
 *=======================================*/
void
msg_info (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	msg_outputv(MSG_INFO, fmt, args);
	va_end(args);
}
/*=========================================
 * msg_status -- handle transitory/status messages
 * delegates to msg_outputv
 * Created: 2001/11/11, Perry Rapp
 * Does not localize
 *=======================================*/
void
msg_status (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	msg_outputv(MSG_STATUS, fmt, args);
	va_end(args);
}
/*=========================================
 * msg_output -- handle any message
 * delegates to msg_outputv
 * Created: 2001/12/16, Perry Rapp
 * Does not localize
 *=======================================*/
void
msg_output (MSG_LEVEL level, STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	msg_outputv(level, fmt, args);
	va_end(args);
}
/*=====================================
 * msg_width -- get max width of msgs
 * Created: 2001/12/16, Perry Rapp
 *===================================*/
INT
msg_width (void)
{
	return ll_cols-6;
}
/*=========================================
 * msg_outputv -- output message varargs style arguments
 * Actually all other msg functions delegate to here.
 * fmt,args:  printf style varargs from client
 * level:     -1=error,0=info,1=status
 * Puts into message list and/or into status area
 * Created: 2001/11/11, Perry Rapp
 * Does not localize
 *=======================================*/
void
msg_outputv (MSG_LEVEL level, STRING fmt, va_list args)
{
	char buffer[250];
	STRING ptr;
	unsigned int width = MAINWIN_WIDTH-5;
	/* prefix errors & infos with * and space respectively */
	switch(level) {
		case MSG_ERROR:
			buffer[0] = '*';
			ptr = &buffer[1];
			break;
		case MSG_INFO:
			buffer[0] = ' ';
			ptr = &buffer[1];
			break;
		default:
			ptr = buffer;
			break;
	}
	/* now make string to show/put on msg list */
	vsnprintf(ptr, sizeof(buffer), fmt, args);
	/* first handle transitory/status messages */
	if (level==MSG_STATUS) {
		if (lock_std_msg)
			return; /* can't display it, status bar is locked */
		if (status_showing[0] && !status_transitory) {
			/* we are overwriting something important 
			so it is already on the msg list, we just need to make
			sure the msg list gets displayed */
			if (!viewing_msgs)
				msg_flag = TRUE;
		}
		display_status(buffer);
		return;
	}
	/* everything important goes onto msg list */
	append_to_msg_list(buffer);
	/* update flag about whether we need to show msg list to user */
	/* being careful in case we are currently *in* the msg list
	show routine */
	if (!viewing_msgs && length_list(msg_list)>1) {
		msg_flag = TRUE;
	}
	/* now put it to status area if appropriate */
	if (!lock_std_msg) {
		if (strlen(buffer)>width) {
			buffer[width-4]=0;
			strcat(buffer, "...");
			if (!viewing_msgs)
				msg_flag = TRUE;
		}
		display_status(buffer);
	}
}
/*=========================================
 * msg_impl -- put msg on the msg list
 * This is a list that we show the user
 * when the current command completes,
 * unless it only had one item, and it got
 * put on the status bar, and it wasn't too wide.
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
static void
append_to_msg_list (STRING msg)
{
		if (!msg_list)
			msg_list = create_list();
		enqueue_list(msg_list, strsave(msg));
}
/*=========================================
 * begin_action -- prepare to process users choice
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
static
void begin_action (void)
{
	clear_msgs();
}
/*=========================================
 * end_action -- finished processing users choice
 *  show msg list if appropriate
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
static
void end_action (void)
{
	/* pause for keypress for finish stdout/err if appropriate */
	check_stdout();
	/* put up list of errors if appropriate */
	if (msg_flag) {
		STRING * strngs = (STRING *)stdalloc(length_list(msg_list)*sizeof(STRING));
		INT i=0;
		FORLIST(msg_list, el)
			strngs[i++] = el;
		ENDLIST
		viewing_msgs = TRUE; /* suppress msg generation */
		view_array(_(qSerrlist), length_list(msg_list), strngs);
		viewing_msgs = FALSE;
		stdfree(strngs);
		clear_msgs();
	}
}
/*=========================================
 * clear_msgs -- delete msg list
 *  The msg list holds messages when several
 *  occurred during the last operation
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
static void
clear_msgs (void)
{
	if (msg_list) {
		free_string_list(msg_list);
		msg_list = 0;
	}
	msg_flag = FALSE;
	/* also clear status bar */
	clear_status();
}
/*=========================================
 * clear_status -- clear status string
 *  The status string is the last message displayed
 *  and is shown at the very bottom of the main screen
 * Created: 2002/02/16, Perry Rapp
 *=======================================*/
static void
clear_status (void)
{
	if (!lock_std_msg)
		status_showing[0]=0;
}
/*=========================================
 * lock_status_msg -- temporarily hold status message
 * Created: 2001/11/11, Perry Rapp
 *=======================================*/
void
lock_status_msg (BOOLEAN lock)
{
	lock_std_msg = lock;
}
/*=====================================
 * repaint_add_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_add_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	draw_win_box(win);
	mvwaddstr(win, row++, 2, _(qSmn_add_ttl));
	mvwaddstr(win, row++, 4, _(qSmn_add_indi));
	mvwaddstr(win, row++, 4, _(qSmn_add_fam));
	mvwaddstr(win, row++, 4, _(qSmn_add_chil));
	mvwaddstr(win, row++, 4, _(qSmn_add_spou));
	mvwaddstr(win, row++, 4, _(qSmn_ret));
}
/*=====================================
 * repaint_delete_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_delete_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	draw_win_box(win);
	mvwaddstr(win, row++, 2, _(qSmn_del_ttl));
	mvwaddstr(win, row++, 4, _(qSmn_del_chil));
	mvwaddstr(win, row++, 4, _(qSmn_del_spou));
	mvwaddstr(win, row++, 4, _(qSmn_del_indi));
	mvwaddstr(win, row++, 4, _(qSmn_del_fam));
	mvwaddstr(win, row++, 4, _(qSmn_ret));
}
/*=====================================
 * repaint_scan_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_scan_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	draw_win_box(win);
	mvwaddstr(win, row++, 2, _(qSmn_sca_ttl));
	mvwaddstr(win, row++, 4, _(qSmn_sca_nmfu));
	mvwaddstr(win, row++, 4, _(qSmn_sca_nmfr));
	mvwaddstr(win, row++, 4, _(qSmn_sca_refn));
	mvwaddstr(win, row++, 4, _(qSmn_ret));
}
/*=====================================
 * repaint_cset_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_cset_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	STRING csndloc = _("L  Test locale names");
	uierase(uiwin);
	draw_win_box(win);
	mvwaddstr(win, row++, 2, _(qSmn_csttl));
	disp_codeset(uiwin, row++, 4, _(qSmn_csintcs), int_codeset);
	disp_locale(uiwin, row++, 4, _(qSmn_csdsploc), "UiLocale");
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_cstsort), MSORT);
#ifdef NOTYET
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_cspref), MPREF);
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_cschar), MCHAR);
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_cslcas), MLCAS);
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_csucas), MUCAS);
#endif
#ifdef HAVE_SETLOCALE
	mvwaddstr(win, row++, 4, csndloc);
#endif
	mvwaddstr(win, row++, 4, _(qSmn_csrpt));
	mvwaddstr(win, row++, 4, _(qSmn_cstt));
	mvwaddstr(win, row++, 4, _(qSmn_ret));
}
/*=====================================
 * repaint_rpc_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_rpc_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	STRING csnrloc = _("L  Test locale names");
	INT row = 1;
	STRING csrptloc = _("Report locale: ");
	uierase(uiwin);
	draw_win_box(win);
	mvwaddstr(win, row++, 2, _(qSmn_csrpttl));
	disp_locale(uiwin, row++, 4, csrptloc, "RptLocale");
#ifdef HAVE_SETLOCALE
	mvwaddstr(win, row++, 4, csnrloc);
#endif
	disp_trans_table_choice(uiwin, row++, 4, _(qSmn_tt_inrp), MINRP);
	mvwaddstr(win, row++, 4, _(qSmn_ret));
}
/*=====================================
 * repaint_tt_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
#ifdef UNIMPLEMENTED_CODE
static void
repaint_tt_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	mvwaddstr(win, row++, 2, _(qSmn_tt_ttl));
/*TO DO*/
}
#endif
/*=====================================
 * repaint_trans_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_trans_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	char line[120];
	/*INT mylen = sizeof(line);*/
	STRING ptr = line;
	uierase(uiwin);
	draw_win_box(win);
	row = 1;
	mvwaddstr(win, row++, 2, _(qSmn_tt_ttl));
	mvwaddstr(win, row++, 4, _(qSmn_tt_edit));
	mvwaddstr(win, row++, 4, _(qSmn_tt_load));
	mvwaddstr(win, row++, 4, _(qSmn_tt_save));
	mvwaddstr(win, row++, 4, _(qSmn_tt_exp));
	mvwaddstr(win, row++, 4, _(qSmn_tt_imp));
	ptr[0] = 0;
/*	llstrcatn(&ptr, _(qSmn_tt_dir), &mylen); */
/*	llstrcatn(&ptr, lloptions.llttexport, &mylen); */
	mvwaddstr(win, row++, 4, line);
	mvwaddstr(win, row++, 4, _(qSmn_ret));
}
/*=====================================
 * repaint_utils_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_utils_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	draw_win_box(win);
	mvwaddstr(win, row++, 2, _(qSmn_uttl));
	mvwaddstr(win, row++, 4, _(qSmn_utsave));
	mvwaddstr(win, row++, 4, _(qSmn_utread));
	mvwaddstr(win, row++, 4, _(qSmn_utkey));
	mvwaddstr(win, row++, 4, _(qSmn_utkpers));
	mvwaddstr(win, row++, 4, _(qSmn_utdbstat));
	mvwaddstr(win, row++, 4, _(qSmn_utmemsta));
	mvwaddstr(win, row++, 4, _(qSmn_utplaces));
	mvwaddstr(win, row++, 4, _(qSmn_utusropt));
	mvwaddstr(win, row++, 4, _(qSmn_quit));
}
/*=====================================
 * repaint_extra_menu -- 
 * Created: 2001/11/24, Perry Rapp
 *===================================*/
static void
repaint_extra_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	INT row = 1;
	draw_win_box(win);
	mvwaddstr(win, row++, 2, _(qSmn_xttl));
	mvwaddstr(win, row++, 4, _(qSmn_xxbsour));
	mvwaddstr(win, row++, 4, _(qSmn_xxbeven));
	mvwaddstr(win, row++, 4, _(qSmn_xxbothr));
	mvwaddstr(win, row++, 4, _(qSmn_xxasour));
	mvwaddstr(win, row++, 4, _(qSmn_xxesour));
	mvwaddstr(win, row++, 4, _(qSmn_xxaeven));
	mvwaddstr(win, row++, 4, _(qSmn_xxeeven));
	mvwaddstr(win, row++, 4, _(qSmn_xxaothr));
	mvwaddstr(win, row++, 4, _(qSmn_xxeothr));
	mvwaddstr(win, row++, 4, _(qSmn_quit));
}
/*============================
 * activate_uiwin -- 
 *  push new uiwindow on top of current one
 * Created: 2001/11/24, Perry Rapp
 *==========================*/
static void
activate_uiwin (UIWINDOW uiwin)
{
	WINDOW * win = uiw_win(uiwin);
	ASSERT(uiwin && win && !uiw_parent(uiwin));
	/* link into parent/child chain */
	uiw_parent(uiwin) = active_uiwin;
	if (active_uiwin) {
		ASSERT(!uiw_child(active_uiwin));
		uiw_child(active_uiwin) = uiwin;
		/* refresh current (in case it was obscured by stdout */
		wrefresh(uiw_win(active_uiwin));
	}
	/* switch to new & refresh */
	active_uiwin = uiwin;
	touchwin(win);
	wrefresh(win);
}
/*============================
 * reactivate_uiwin --
 *  push new window on top, if not already on top
 *  and refresh it in any case
 * Created: 2001/12/22 (Perry Rapp)
 *==========================*/
static void
reactivate_uiwin (UIWINDOW uiwin)
{
	if (active_uiwin != uiwin)
		activate_uiwin(uiwin);
	else {
		WINDOW * win = uiw_win(uiwin);
		touchwin(win);
		wrefresh(win);
	}
}
/*============================
 * deactivate_uiwin -- Remove currently active
 *  and pop to its parent (if it has one)
 * Created: 2001/11/24, Perry Rapp
 *==========================*/
static void
deactivate_uiwin (void)
{
	UIWINDOW uiw = active_uiwin;
	active_uiwin = uiw_parent(active_uiwin);
	uiw_parent(uiw)=0;
	if (uiw_dynamic(uiw))
		delete_uiwindow(uiw);
	if (active_uiwin) {
		ASSERT(uiw_child(active_uiwin)==uiw);
		uiw_child(active_uiwin)=0;
	}
}
/*=============================================
 * deactivate_uiwin_and_touch_all --
 *  remove current window & repaint ones left
 *===========================================*/
static void
deactivate_uiwin_and_touch_all (void)
{
	deactivate_uiwin();
	if (active_uiwin)
		touch_all(TRUE);
}
/*============================
 * touch_all -- Repaint all ancestors of current window
 * from furthest to nearest
 * Created: 2001/12/01, Perry Rapp
 *==========================*/
static void
touch_all (BOOLEAN includeCurrent)
{
	UIWINDOW uiwin=active_uiwin;
	ASSERT(uiwin);
	/* climb to highest window ancestor */
	while (uiw_parent(uiwin)) {
		uiwin = uiw_parent(uiwin);
	}
	/* walk down touching */
	while (uiwin && (includeCurrent || uiwin!=active_uiwin)) {
		touchwin(uiw_win(uiwin));
		wrefresh(uiw_win(uiwin));
		uiwin = uiw_child(uiwin);
	}
}
/*============================
 * switch_to_uiwin -- 
 *  switch away from currently active uiwin
 *  to new uiwin
 *  currently active uiwin (if any) must be solo
 *  new uiwin must be solo
 * Created: 2001/11/24, Perry Rapp
 *==========================*/
static void
switch_to_uiwin (UIWINDOW uiwin)
{
	WINDOW * win = uiw_win(uiwin);
	if (uiwin != active_uiwin) {
		ASSERT(uiwin && win && !uiw_parent(uiwin) && !uiw_child(uiwin));
		/* link into parent/child chain */
		uiw_parent(uiwin) = active_uiwin;
		if (active_uiwin) {
			/* current active window must be solo, no parent or child */
			ASSERT(!uiw_child(active_uiwin) && !uiw_parent(active_uiwin));
		}
		/* switch to new & refresh */
		active_uiwin = uiwin;
	}
	touchwin(win);
	wrefresh(win);
}
/*============================
 * refresh_stdout -- 
 *  bring stdout to front
 * Created: 2001/11/24, Perry Rapp
 *==========================*/
void
refresh_stdout (void)
{
	wrefresh(uiw_win(stdout_win));
}
/*============================
 * uierase -- erase window 
 *  handles manual erasing if broken_curses flag set
 * Created: 2001/11/24, Perry Rapp
 *==========================*/
static void
uierase (UIWINDOW uiwin)
{
	if (getoptint("ForceScreenErase", 0) > 0) {
		/* fill virtual output with dots */
		uicolor(uiwin, '.');
		wnoutrefresh(uiw_win(uiwin));
		/* now fill it back with spaces */
		uicolor(uiwin, ' ');
		wrefresh(uiw_win(uiwin));
	} else {
		werase(uiw_win(uiwin));
	}
}
/*============================
 * uicolor -- fill window with character 
 *==========================*/
static void
uicolor (UIWINDOW uiwin, char ch)
{
	INT i;
	WINDOW *win = uiw_win(uiwin);
	for (i=0; i<uiw_rows(uiwin)+9; ++i) {
		color_hseg(win, i, 0, uiw_cols(uiwin)-1, ch);
	}
}
