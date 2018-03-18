#include <Windows.h>

#ifndef INDLL_H
#define INDLL_H

#ifdef EXPORTING_DLL
extern __declspec(dllexport) void show_message_box();
#else
extern __declspec(dllimport) void show_message_box();
#endif


#endif