/**
* d2gDownloads.c
*
* GameSpy  DIRECT TO GAME SDK
* This file is part of the Direct to Game SDK designed and developed by GameSpy Tech.
* Copyright (c) 2008, GameSpy Technology
*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// includes
#include "../common/gsResultCodes.h"
#include "../common/gsAvailable.h"
#include "Direct2Game.h"
#include "d2gMain.h"
#include "d2gServices.h"
#include "d2gUtil.h"
#include "d2gDownloads.h"

typedef struct D2GIDownloadThreadData
{
	//gsi_bool mCancelThread;
	gsi_char *mUrl;
	gsi_char *mFullPath;
    gsi_char *mFullPathTemporary;   
    gsi_char *mHttpAdditionalHeaders;  
	D2GDownloadProgressCallback mProgressCallback;
	D2GDownloadCompleteCallback mCompleteCallback;
	void     *mUserData;
} D2GIDownloadThreadData;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void d2giDownloadHttpProgressCallback(  GHTTPRequest   request,
						                GHTTPState     state,
						                const char     *buffer,
						                GHTTPByteCount bufferLen,
						                GHTTPByteCount bytesReceived,
						                GHTTPByteCount totalSize,
						                void           *param)
{
	// Check progress and call any developer callbacks
	D2GIDownloadThreadData *dLoadData;
	dLoadData = (D2GIDownloadThreadData *)param;
	
	GS_ASSERT(dLoadData);

	if (state == GHTTPReceivingFile)
	{	
		// At this point the callback should be set.  We already did most of the error checking 
		// so no need to assert.
		dLoadData->mProgressCallback((gsi_u32)bytesReceived, (gsi_u32)totalSize, dLoadData->mUserData);
	}

	GSI_UNUSED(request);
	GSI_UNUSED(bufferLen);
	GSI_UNUSED(buffer);
	GSI_UNUSED(buffer);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GHTTPBool d2giDownloadHttpCompletedCallback(    GHTTPRequest   httpRequest,
											    GHTTPResult    httpResult,
											    char           *buffer,
											    GHTTPByteCount bufferLen,
												char* headers,
											    void           *param)               
{
	D2GIDownloadThreadData *dLoadData;
	GSResult result;
	dLoadData = (D2GIDownloadThreadData *)param;
	
	result = d2giResultFromHttpResult(httpResult);

	if (GS_SUCCEEDED(result))
	{   
		// The download succeeded 
        // rename the temporary download file to the actual file name
		FILE *tempFileCheck;
		tempFileCheck = fopen(dLoadData->mFullPath, "rb");
		if (tempFileCheck)
		{
			fclose(tempFileCheck);
			_tremove(dLoadData->mFullPath);
		}
        _trename( dLoadData->mFullPathTemporary, dLoadData->mFullPath);
		dLoadData->mCompleteCallback( result, dLoadData->mFullPath, dLoadData->mUserData);
	}
	else
	{
		dLoadData->mCompleteCallback( result, NULL, dLoadData->mUserData);
	}

	GSI_UNUSED(bufferLen);
	GSI_UNUSED(buffer);
	GSI_UNUSED(httpRequest);
	return GHTTPTrue;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
DWORD WINAPI d2giDownloadThreadFunc(void *arg)
{
	D2GIDownloadThreadData *dLoadData;
	GHTTPRequest           httpRequest;
	dLoadData = (D2GIDownloadThreadData *)arg;
    
	httpRequest = ghttpSaveEx(  dLoadData->mUrl,                    //URL
	                            dLoadData->mFullPathTemporary,      //Filename
	                            dLoadData->mHttpAdditionalHeaders,  //Headers    
	                            NULL,                               //Post
	                            GHTTPFalse,                         //Throttle
	                            GHTTPTrue,                          //Blocking
	                            d2giDownloadHttpProgressCallback,   //ProgressCallback
				                d2giDownloadHttpCompletedCallback,  //CompletedCallback
				                (void *)dLoadData);	
	if (IS_GHTTP_ERROR(httpRequest))
	{
		GSResult result = d2giResultFromHttpRequest(httpRequest);		
		dLoadData->mCompleteCallback(result, NULL, dLoadData->mUserData);
	}
	
	gsifree(dLoadData->mFullPath);
	gsifree(dLoadData->mUrl);
    gsifree(dLoadData->mFullPathTemporary);
    gsifree(dLoadData->mHttpAdditionalHeaders);
	gsifree(dLoadData);
	return 0;
}
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSResult d2giStartDownloadThread( const gsi_char *downloadLocation, 
                                  gsi_char       *url, 
                                  gsi_char       *httpHeaders, 
                                  D2GDownloadProgressCallback progress, 
                                  D2GDownloadCompleteCallback complete, 
                                  void           *userData )
{
#ifdef _WIN32
	GSIThreadID threadId;
	gsi_char    *fileName;
	gsi_char    *trailingJunk;
	int         returnVal;
	int downloadLocationLength;
    D2GIDownloadThreadData *dloadThreadData;
	
    dloadThreadData = (D2GIDownloadThreadData *)gsimalloc(sizeof(D2GIDownloadThreadData));
    if (dloadThreadData == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_HotError,
            "%s :: %s (%d): unable to allocate dloadThreadData\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }

    memset(dloadThreadData, 0, sizeof(D2GIDownloadThreadData));
	dloadThreadData->mUrl      = url;	
	dloadThreadData->mHttpAdditionalHeaders = httpHeaders;
	dloadThreadData->mProgressCallback = progress;
	dloadThreadData->mCompleteCallback = complete;
	dloadThreadData->mUserData = userData;


	fileName = _tcsrchr(dloadThreadData->mUrl, '/');
	fileName++;
	trailingJunk = _tcsrchr(fileName, '?');
	downloadLocationLength = _tcslen(downloadLocation);
	
	if (trailingJunk == NULL)
	{
		trailingJunk = (gsi_char *)(downloadLocation + downloadLocationLength);
	}
	dloadThreadData->mFullPath = (gsi_char *)gsimalloc(downloadLocationLength + (trailingJunk - fileName) + 1);
    if (dloadThreadData->mFullPath == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_HotError,
            "%s :: %s (%d): unable to allocate dloadThreadData->mFullPath\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
	_tsnprintf( dloadThreadData->mFullPath,
	            _tcslen(downloadLocation) + (trailingJunk - fileName), 
	            "%s%s",
	            downloadLocation, fileName);
	dloadThreadData->mFullPath[downloadLocationLength + (trailingJunk - fileName)] = '\0';        
    
    // Initialize the temporary file before starting the download
    dloadThreadData->mFullPathTemporary = (gsi_char *)gsimalloc( _tcslen(dloadThreadData->mFullPath) + 
                                                                 _tcslen( GS_D2G_DOWNLOAD_FILE_TEMP_EXT ) + 1);
    if (dloadThreadData->mFullPathTemporary == NULL)
    {
        gsDebugFormat(GSIDebugCat_Direct2Game, GSIDebugType_Memory, GSIDebugLevel_HotError,
            "%s :: %s (%d): unable to allocate dloadThreadData->mFullPathTemporary\n",
            __FILE__, __FUNCTION__, __LINE__);
        return GS_D2G_ERROR(GSResultSection_Memory, GSResultCode_OutOfMemory);
    }
    _stprintf(dloadThreadData->mFullPathTemporary, "%s%s", dloadThreadData->mFullPath, GS_D2G_DOWNLOAD_FILE_TEMP_EXT);

	gsDebugFormat(GSIDebugCat_Direct2Game, 
                  GSIDebugType_Network, 
                  GSIDebugLevel_Comment, 
                  "Starting download service...");
    
    // start the actual download thread
	returnVal = gsiStartThread( d2giDownloadThreadFunc, 
	                            0, 
	                            (void *)dloadThreadData, 
	                            &threadId);
	if (returnVal == -1) 
	{
		// There has to be some really wrong for this to happen
		// Thread creation failures would probably mean resource issues.
		gsifree(dloadThreadData->mUrl);
        gsifree(dloadThreadData->mFullPathTemporary);
		gsifree(dloadThreadData->mFullPath);
		gsifree(dloadThreadData);
		return GS_D2G_ERROR(GSResultSection_State, GSResultCode_ThreadCreateFailed);
	}	
#endif

	return GS_SUCCESS;
}
