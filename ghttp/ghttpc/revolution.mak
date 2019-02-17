# GHTTPC makefile for Revolution SDK

CFILES = ghttpc.c

TARGET_NAME = rshttpc

TARGET_TYPE = Application

LNKFLAGS = -L../ -L../../common 

APPLICATION_LIBS_RELEASE = -lrshttp -lrscommon
APPLICATION_LIBS_DEBUG = -lrshttpD -lrscommonD

include ../../common/revolution/toolchain.rules
