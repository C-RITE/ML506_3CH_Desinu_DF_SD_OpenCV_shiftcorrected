/* Jungo Confidential. Copyright (c) 2008 Jungo Ltd.  http://www.jungo.com */

#define WD_MAJOR_VER 9
#define WD_MINOR_VER 2
#define WD_SUB_MINOR_VER 0

#define WD_MAJOR_VER_STR "9"
#define WD_MINOR_VER_STR "2"
#define WD_SUB_MINOR_VER_STR "0"

/* %% Replace with empty string for non-beta version %% */
#define WD_VER_BETA_STR "" 

#define WD_VERSION_STR WD_MAJOR_VER_STR "." WD_MINOR_VER_STR WD_SUB_MINOR_VER_STR WD_VER_BETA_STR
#define WD_VER (WD_MAJOR_VER * 100 + WD_MINOR_VER * 10 + WD_SUB_MINOR_VER)
#define WD_VER_ITOA WD_MAJOR_VER_STR WD_MINOR_VER_STR WD_SUB_MINOR_VER_STR
#define WD_VERSION_STR_DOTS WD_MAJOR_VER_STR "." WD_MINOR_VER_STR "." WD_SUB_MINOR_VER_STR WD_VER_BETA_STR 

