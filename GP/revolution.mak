# GP makefile for Revolution SDK

CFILES = gp.c \
    gpi.c \
    gpiBuddy.c \
    gpiBuffer.c \
    gpiCallback.c \
    gpiConnect.c \
    gpiInfo.c \
    gpiKeys.c \
    gpiOperation.c \
    gpiPeer.c \
    gpiProfile.c \
    gpiPS3.c \
    gpiSearch.c \
    gpiTransfer.c \
    gpiUnique.c \
    gpiUtility.c

TARGET_NAME = rsgp

TARGET_TYPE = Static

include ../common/revolution/toolchain.rules
