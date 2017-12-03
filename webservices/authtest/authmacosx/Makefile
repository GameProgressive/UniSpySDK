# AuthService Test App Makefile
# Copyright 2011 GameSpy Industries

PROJECT=authtest
DEFINES=-D_MACOSX -DGSI_NO_THREADS
SRCROOT= ../..

SRC = \
	../authtest.c\
	../../../common/gsMemory.c\
	../../../common/macosx/MacOSXCommon.c\
	../../../darray.c\
	../../../hashtable.c\
	../../../md5c.c\
	../../../common/gsSSL.c\
	../../../common/gsAvailable.c\
	../../../common/gsDebug.c\
	../../../common/gsStringUtil.c\
	../../../common/gsPlatform.c\
	../../../common/gsPlatformSocket.c\
	../../../common/gsPlatformThread.c\
	../../../common/gsPlatformUtil.c\
	../../../common/gsSoap.c\
	../../../common/gsAssert.c\
	../../../common/gsCore.c\
	../../../common/gsXML.c\
	../../../common/gsCrypt.c\
	../../../common/gsRC4.c\
	../../../common/gsLargeInt.c\
	../../../common/gsSHA1.c\
	../../../ghttp/ghttpBuffer.c\
	../../../ghttp/ghttpCallbacks.c\
	../../../ghttp/ghttpConnection.c\
	../../../ghttp/ghttpEncryption.c\
	../../../ghttp/ghttpMain.c\
	../../../ghttp/ghttpProcess.c\
	../../../ghttp/ghttpCommon.c\
	../../../ghttp/ghttpPost.c

# include directories
INCLUDES = -I$(SRCROOT) -I$(SRCROOT)/.. -I$(SRCROOT)/../common -I$(SRCROOT)/../ghttp

LDFLAGS:= $(LDFLAGS) -lAuthservice

include ../../../common/macosx/MakeCommon.mk
