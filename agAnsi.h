/*
 * A substantial list of macros for working
 * with ANSI escape sequences in the terminal.
 *
 * The macro functions require
 * a string literal argument. Most common
 * application is connection adjacent
 * string literals together with C's
 * concatentation method.
 *
 * The definition macros are meant
 * to be passed as arguments to 
 * the ANSI() method macro, or
 * the ANSI2() method macro. The
 * ANSI() puts the full escape
 * string into a string literal where
 * placed. ANSI2() is used for SGV
 * definitions (colors) that require
 * an ending ANSI_RESET escape sequence.
 * For example, if the ANSI code contains _FG_ or _BG_,
 * foreground and background respectively,
 * you need to use a trailing ANSI_RESET IF
 * you want the effect to stop. Most styling
 * codes work this way.
 *
 * Examples:
 *
 * 	const char *exp = ANSI(ANSI_FG_RED) "Some Red Text!" ANSI(ANSI_RESET)
 *
 * 	// Which is equivalent to
 *
 * 	const char *exp = ANSI2(ANSI_FG_RED, "Some Red Text!");
 *
 * 	Avoiding using the nice method macros:
 *
 * 	const char *exp = ANSI_ESC ANSI_FG_RED ANSI_ESC ANSI_RESET
 *
 *	Or in just a plain string...
 *
 *	"\x1b[31mSome Red Text!\x1b[0m"
 *
 *
 * 	It all works in any string literal, including those
 * 	passed to functions like printf()
 *
 * author: Anthony Segedi
 *
 */
// util
#define ANSI_RESET "0m"
#define ANSI_CURSOR_SAVE "s"
#define ANSI_CURSOR_RESTORE "u"
// line
// NOTE that cursor pos does not change with erase cmds
#define ANSI_ERASE_LN_FW "0K"
#define ANSI_ERASE_LN_BK "1K"
#define ANSI_ERASE_LN_ALL "2K"

// display
#define ANSI_ERASE_DISP_END "0J"
#define ANSI_ERASE_DISP_BEG "1J"
#define ANSI_ERASE_DISP_ALL "2J"
// deletes screen and scroll back
#define ANSI_ERASE_DISP_DEL "3J"

// style
#define ANSI_BOLD "1m"
#define ANSI_DIM "2m"
#define ANSI_BLINK_SLOW "5m"
#define ANSI_BLINK_FAST "6m"
#define ANSI_BLINK_OFF "25m"
#define ANSI_STRIKE "9m"
#define ANSI_DEFAULT "10m"

// foreground
#define ANSI_FG_BLACK "30m"
#define ANSI_FG_RED "31m"
#define ANSI_FG_GREEN "32m"
#define ANSI_FG_YELLOW "33m"
#define ANSI_FG_BLUE "34m"
#define ANSI_FG_MAGENTA "35m"
#define ANSI_FG_CYAN "36m"
#define ANSI_FG_WHITE "37m"
#define ANSI_FG_DEFAULT "39m"

// background
#define ANSI_BG_BLACK "40m"
#define ANSI_BG_RED "41m"
#define ANSI_BG_GREEN "42m"
#define ANSI_BG_YELLOW "43m"
#define ANSI_BG_BLUE "44m"
#define ANSI_BG_MAGENTA "45m"
#define ANSI_BG_CYAN "46m"
#define ANSI_BG_WHITE "47m"
#define ANSI_BG_DEFAULT "49m"

#define ANSI_ESC "\x1b["
#define ANSI(ansicode) ANSI_ESC ansicode
#define ANSI2(ansicode, str) ANSI_ESC ANSI(ansicode) str ANSI_ESC ANSI_RESET

// cursor movement
#define ANSI_CURSOR_UP(n) ANSI_ESC n "A"
#define ANSI_CURSOR_DOWN(n) ANSI_ESC n "B"
#define ANSI_CURSOR_FORWARD(n) ANSI_ESC n "C"
#define ANSI_CURSOR_BACK(n) ANSI_ESC n "D"
#define ANSI_CURSOR_LINE_NEXT(n) ANSI_ESC n "E"
#define ANSI_CURSOR_LINE_PREV(n) ANSI_ESC  n "F"
#define ANSI_CURSOR_COL_ABS(x) ANSI_ESC x "G"
#define ANSI_CURSOR_POS_ABS(x,y) ANSI_ESC x ";" y "H"


