/*
GameSpy PT SDK 
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 2000 GameSpy Industries, Inc

*/

/**************************************
** GameSpy Patching and Tracking SDK **
**************************************/

#ifndef _PT_H_
#define _PT_H_

#include "../common/gsCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********
** TYPES **
**********/

// Boolean.
///////////
typedef int PTBool;
#define PTFalse 0
#define PTTrue  1

/**************
** FUNCTIONS **
**************/
#ifndef GSI_UNICODE
#define ptCheckForPatch		ptCheckForPatchA
#define ptTrackUsage		ptTrackUsageA
#define ptCheckForPatchAndTrackUsage	ptCheckForPatchAndTrackUsageA
#define ptCreateCheckPatchTrackUsageReq       ptCreateCheckPatchTrackUsageReqA
#else
#define ptCheckForPatch		ptCheckForPatchW
#define ptTrackUsage		ptTrackUsageW
#define ptCheckForPatchAndTrackUsage	ptCheckForPatchAndTrackUsageW
#define ptCreateCheckPatchTrackUsageReq ptCreateCheckPatchTrackUsageReqW
#endif

// This callback gets called when a patch is being checked for
// with either ptCheckForPatch or ptCheckForPatchAndTrackUsage.
// If a patch is available, and the fileID is not 0, then
// ptLookupFilePlanetInfo can be used to find download sites.
///////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// ptPatchCallback
// Summary
//		This callback gets called when a patch is being checked for with
//		 either ptCheckForPatch 
//		or ptCheckForPatchAndTrackUsage.
// Parameters
//		available	: [in] PTTrue if a newer version is available. If
//		 PTFalse, ignore the other parameters.
//		mandatory	: [in] If PTTrue, this patch has been marked as mandatory.
//		versionName	: [in] A user-readable display name for the new version.
//		fileID		: [in] A FilePlanet file ID for the patch. Can be 0.
//		 Used to form a FilePlanet URL
//		downloadURL	: [in] If not an empty string, contains a URL to
//		 download the patch from.
//		param		: [in] This is optional user-data that was passed to ptCheckForPatch.
// Remarks
//		If a patch is available, and the fileID is not 0, then
//		 ptLookupFilePlanetInfo can be used 
//		to find download sites.<p>
typedef void (* ptPatchCallback)
(
	PTBool available,         // If PTTrue, a patch is available.
	PTBool mandatory,         // If PTTrue, this patch is flagged as being mandatory.
	const gsi_char * versionName, // The display name for the new version.
	int fileID,               // The FilePlanet fileID for the patch, or 0.
	const gsi_char * downloadURL, // A URL to download the patch from, or NULL.
	void * param              // User-data passed originally passed to the function.
);

//////////////////////////////////////////////////////////////
// ptCheckForPatch
// Summary
//		Determine whether a patch is available for the current version and
//		 particular distribution of a product.
// Parameters
//		productID		: [in] Numeric ID assigned by GameSpy. This is NOT the game ID.
//		versionUniqueID	: [in] Developer specified string to indentify
//		 the current version. Typically "1.0" form.
//		distributionID	: [in] Optional indentifier for distribution. This
//		 is usually 0.
//		callback		: [in] Function to be called when the operation completes.
//		blocking		: [in] When set to PTTrue, this function will not
//		 return until the operation has completed.
//		instance		: [in] Pointer to user data. This is optional and
//		 will be passed unmodified to the callback function.
// Returns
//		PTTrue is return if a query was sent. PTFalse means the operation
//		 was aborted.
// Remarks
//		The ptCheckForPatch function sends a query to determine if a patch
//		 is available for the current game 
//		version and distribution. If this function does not return PTFalse,
//		 then the callback 
//		will be called with information on a possible patch to a newer version.<p>
PTBool ptCheckForPatch
(
	int productID,                // The ID of this product.
	const gsi_char * versionUniqueID, // A string uniquely identifying this version.
	int distributionID,           // The distribution ID for this version.  Can be 0.
	ptPatchCallback callback,     // The callback to call with the patch info.
	PTBool blocking,              // If PTTrue, don't return until after the callback is called.
	void * param                  // User-data to pass to the callback.
);

//////////////////////////////////////////////////////////////
// ptTrackUsage
// Summary
//		Track usage of a product, based on version and distribution.
// Parameters
//		userID			: [in] The GP userID of the user who is using the
//		 product. Can be 0.
//		productID		: [in] The ID of this product.
//		versionUniqueID	: [in] A string uniquely identifying this version.
//		distributionID	: [in] The distribution ID for this version. Can be 0.
//		blocking		: [in] When set to PTTrue, this function will not
//		 return until the operation has completed
// Returns
//		If PTFalse is returned, there was an error tracking usage.
PTBool ptTrackUsage
(
	int userID,                   // The GP userID of the user who is using the product.  Can be 0.
	int productID,                // The ID of this product.
	const gsi_char * versionUniqueID, // A string uniquely identifying this version.
	int distributionID,           // The distribution ID for this version.  Can be 0.
	PTBool blocking
);

//////////////////////////////////////////////////////////////
// ptCheckForPatchAndTrackUsage
// Summary
//		Does the same thing as both ptCheckForPatch and ptTrackUsage, in one call.
// Parameters
//		userID			: [in] Numeric ID assigned by GameSpy. This is NOT the game ID.
//		productID		: [in] Developer specified string to indentify the
//		 current version. Typically "1.0" form.
//		versionUniqueID	: [in] Developer specified string to indentify
//		 the current version. Typically "1.0" form.
//		distributionID	: [in] Optional indentifier for distribution. This
//		 is usually 0.
//		callback		: [in] Function to be called when the operation completes.
//		blocking		: [in] When set to PTTrue, this function will not
//		 return until the operation has completed.
//		param			: [in] Pointer to user data. This is optional and
//		 will be passed unmodified to the callback function.
// Returns
//		
// See Also
//		ptCheckForPatch, ptTrackUsage
PTBool ptCheckForPatchAndTrackUsage
(
	int userID,                   // The GP userID of the user who is using the product.  Can be 0.
	int productID,                // The ID of this product.
	const gsi_char * versionUniqueID, // A string uniquely identifying this version.
	int distributionID,           // The distribution ID for this version.  Can be 0.
	ptPatchCallback callback,     // The callback to call with the patch info.
	PTBool blocking,              // If PTTrue, don't return until after the callback is called.
	void * param                  // User-data to pass to the callback.
);

// This function is similar to the function ptCheckForPatchAndTrackUsage
// except that it returns a handle that can be used to call ghttpRequestThink
// or a failure of -1
//////////////////////////////////////////////////////
PTBool ptCreateCheckPatchTrackUsageReq
(
	int userID,                   // The GP userID of the user who is using the product.  Can be 0.
	int productID,                // The ID of this product.
	const gsi_char * versionUniqueID, // A string uniquely identifying this version.
	int distributionID,           // The distribution ID for this version.  Can be 0.
	ptPatchCallback callback,     // The callback to call with the patch info.
	PTBool blocking,              // If PTTrue, don't return until after the callback is called.
	int *request,                 // The request that can be used for ghttpRequestThink
	void * param                  // User-data to pass to the callback.
 );

// This callback gets called when looking up info on a
// FilePlanet file. If the file is found, it provides a
// text description, size, and a list of download sites.
////////////////////////////////////////////////////////
typedef void (* ptFilePlanetInfoCallback)
(
	int fileID,                // The ID of the file for which info was looked up.
	PTBool found,              // PTTrue if the file was found.
	const gsi_char * description,  // A user-readable description of the file.
	const gsi_char * size,         // A user-readable size of the file.
	int numMirrors,            // The number of mirrors found for this file.
	const gsi_char ** mirrorNames, // The names of the mirrors.
	const gsi_char ** mirrorURLs,  // The URLS for downloading the files.
	void * param               // User-data passed originally passed to the function.
);

// 9/7/2004 (xgd) ptLookupFilePlanetInfo() deprecated; per case 2724.
//
// Use this function to lookup info on a fileplanet file, by ID.
// This can be used to find a list of download sites for a patch
// based on the fileID passed to ptPatchCallback.  If PTFalse is
// returned, then there was an error and the callback will not
// be called.
////////////////////////////////////////////////////////////////
PTBool ptLookupFilePlanetInfo
(
	int fileID,                        // The ID of the file to find info on.
	ptFilePlanetInfoCallback callback, // The callback to call with the patch info.
	PTBool blocking,                   // If PTTrue, don't return until after the callback is called.
	void * param                       // User-data to pass to the callback.
);

#ifdef __cplusplus
}
#endif

#endif
