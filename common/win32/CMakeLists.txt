cmake_minimum_required(VERSION 3.0.0)

add_library(rsplatform STATIC gsSocketWin32.c gsThreadWin32.c gsUtilWin32.c rsPlatformwin32.h Win32Common.c)
