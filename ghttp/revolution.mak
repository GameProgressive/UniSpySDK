# GHTTP makefile for Revolution SDK

CFILES = ghttpBuffer.c \
    ghttpCallbacks.c \
    ghttpCommon.c \
    ghttpConnection.c \
    ghttpEncryption.c \
    ghttpMain.c \
    ghttpPost.c \
    ghttpProcess.c

TARGET_NAME = rshttp

TARGET_TYPE = Static

ifeq ($(HTTP_LOG), 1)
	CFLAGS := $(CFLAGS) -DHTTP_LOG=1
endif

CFLAGS_DEBUG := $(CFLAGS_DEBUG) -DGHTTP_EXTENDEDERROR=1

include ../common/revolution/toolchain.rules
