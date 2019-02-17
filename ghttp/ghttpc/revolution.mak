# GHTTPC makefile for Revolution SDK

CFILES = ghttpc.c

TARGET_NAME = rshttpc

TARGET_TYPE = Application

LNKFLAGS = -L../ -L../../common -lrshttp -lrscommon

include ../../common/revolution/toolchain.rules
