#pragma once
/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */
#ifndef DBJ_VT100WIN10_INCLUDED
#define DBJ_VT100WIN10_INCLUDED

#ifdef DBJ_NANO_WIN32
/*
VT100 ESC codes for coloring the WIN10 PRO console output
full explanation:
https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences?redirectedfrom=MSDN
*/
#define DBJ_ESC "\x1b"
#define DBJ_CSI "\x1b["

#define DBJ_CLS DBJ_CSI "2J"
#define DBJ_RESET DBJ_CSI "0m"
#define DBJ_BOLD_FG DBJ_CSI "1m"
#define DBJ_ULINE DBJ_CSI "4m"
#define DBJ_ULINE_OFF DBJ_CSI "24m"
#define DBJ_COLOR_SWAP DBJ_CSI "7m"

#define DBJ_FG_RED DBJ_CSI "31m"
#define DBJ_FG_GREEN DBJ_CSI "32m"
#define DBJ_FG_YELLOW DBJ_CSI "33m"
#define DBJ_FG_BLUE DBJ_CSI "34m"
#define DBJ_FG_CYAN DBJ_CSI "36m"
#define DBJ_FG_WHITE DBJ_CSI "37m"

#define DBJ_FG_RED_BOLD DBJ_BOLD_FG DBJ_FG_RED
#define DBJ_FG_GREEN_BOLD DBJ_BOLD_FG DBJ_FG_GREEN
#define DBJ_FG_YELLOW_BOLD DBJ_BOLD_FG DBJ_FG_YELLOW
#define DBJ_FG_BLUE_BOLD DBJ_BOLD_FG DBJ_FG_BLUE
#define DBJ_FG_CYAN_BOLD DBJ_BOLD_FG DBJ_FG_CYAN
#define DBJ_FG_WHITE_BOLD DBJ_BOLD_FG DBJ_FG_WHITE

#define DBJ_BG_BLACK DBJ_CSI "40m"
#define DBJ_BG_RED DBJ_CSI "41m"
#define DBJ_BG_GREEN DBJ_CSI "42m"
#define DBJ_BG_YELLOW DBJ_CSI "43m"
#define DBJ_BG_BLUE DBJ_CSI "44m"

#else
#error VT100 escape codes are defined only for WIN10 builds
#endif // DBJ_NANO_WIN32
#endif // DBJ_VT100WIN10_INCLUDED