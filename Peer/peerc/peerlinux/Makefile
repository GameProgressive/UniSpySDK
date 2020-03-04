# Peer SDK Makefile
# Copyright 2004 GameSpy Industries

TARGET=peersdk
THREADS=yes
DEBUG=no
CFLAGS=-W -Wno-missing-field-initializers

SRC = \
	../../../darray.c\
	../../../md5c.c\
	../../../hashtable.c\
	../../../common/gsAvailable.c\
	../../../common/gsPlatform.c\
	../../../common/gsPlatformSocket.c\
	../../../common/gsPlatformThread.c\
	../../../common/gsPlatformUtil.c\
	../../../common/gsStringUtil.c\
	../../../common/gsDebug.c\
	../../../common/gsMemory.c\
	../../../common/linux/LinuxCommon.c\
	../../../Chat/chatCallbacks.c\
	../../../Chat/chatChannel.c\
	../../../Chat/chatCrypt.c\
	../../../Chat/chatHandlers.c\
	../../../Chat/chatMain.c\
	../../../Chat/chatSocket.c\
	../../../pinger/pingerMain.c\
	../../../serverbrowsing/sb_crypt.c\
	../../../serverbrowsing/sb_queryengine.c\
	../../../serverbrowsing/sb_server.c\
	../../../serverbrowsing/sb_serverbrowsing.c\
	../../../serverbrowsing/sb_serverlist.c\
	../../../qr2/qr2.c\
	../../../qr2/qr2regkeys.c\
	../../../natneg/natneg.c\
	../../../natneg/NATify.c\
	../../peerAutoMatch.c\
	../../peerCallbacks.c\
	../../peerGlobalCallbacks.c\
	../../peerHost.c\
	../../peerKeys.c\
	../../peerMain.c\
	../../peerMangle.c\
	../../peerOperations.c\
	../../peerPing.c\
	../../peerPlayers.c\
	../../peerQR.c\
	../../peerRooms.c\
	../../peerSB.c\
	../peerc.c

include ../../../common/linux/Makefile.common
