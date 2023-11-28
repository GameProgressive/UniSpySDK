/**
* d2gDownloads.h
*
* GameSpy  DIRECT TO GAME SDK
* This file is part of the Direct to Game SDK designed and developed by GameSpy Tech.
* Copyright (c) 2008, GameSpy Technology
*/

#ifndef __D2GDOWNLOADS_H__
#define __D2GDOWNLOADS_H__

#define  GS_D2G_DOWNLOAD_FILE_TEMP_EXT  ".tmp"
#define  GS_MAX_FILENAME_LEN            (256)   

GSResult d2giStartDownloadThread(const gsi_char *downloadLocation, 
                                 gsi_char       *url, 
                                 gsi_char       *httpHeaders, 
                                 D2GDownloadProgressCallback progress, 
                                 D2GDownloadCompleteCallback complete, 
                                 void           *userData);
#endif
