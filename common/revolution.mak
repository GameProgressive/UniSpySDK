# RSCommon makefile for Revolution SDK

CFILES = darray.c \
    gsAssert.c \
    gsAvailable.c \
    gsCore.c \
    gsCrypt.c \
    gsDebug.c \
    gsLargeInt.c \
    gsMemory.c \
    gsPlatform.c \
    gsPlatformSocket.c \
    gsPlatformThread.c \
    gsPlatformUtil.c \
    gsRC4.c \
    gsResultCodes.c \
    gsSHA1.c \
    gsSSL.c \
    gsStringUtil.c \
    gsXML.c \
    hashtable.c \
    md5c.c

TARGET_NAME = RSCommon

CFLAGS_DEBUG := $(CFLAGS_DEBUG) -DGSI_COMMON_DEBUG=1

TARGET_TYPE = Static

include revolution/toolchain.rules
