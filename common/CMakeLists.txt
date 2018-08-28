cmake_minimum_required(VERSION 3.0.0)

add_library(rscommon
    darray.c
    gsAvailable.c
    gsCore.c
    gsCrypt.c
    gsLargeInt.c
    gsRC4.c
    gsSHA1.c
    gsSoap.c
    gsSSL.c
    gsStringUtil.c
    gsUdpEngine.c
    gsXML.c
    hashtable.c
    md5c.c
    nonport.c
)
