/*
GameSpy Voice2 SDK
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 2004 GameSpy Industries, Inc

devsupport@gamespy.com
http://gamespy.net
*/

/************************
** GameSpy Voice 2 SDK **
************************/

#ifndef _GV_H_
#define _GV_H_

#include "../common/gsCommon.h"

#if defined(__cplusplus)
extern "C" {
#endif

//DEFINES
/////////
#define GV_BYTES_PER_SAMPLE          (sizeof(GVSample) / sizeof(GVByte))
#define GV_BITS_PER_SAMPLE           (GV_BYTES_PER_SAMPLE * 8)

#define GV_DEVICE_NAME_LEN   64
#define GV_CHANNEL_NAME_LEN  64

#define GV_CAPTURE   1
#define GV_PLAYBACK  2
#define GV_CAPTURE_AND_PLAYBACK  (GV_CAPTURE|GV_PLAYBACK)

#define GVRate_8KHz   8000
#define GVRate_16KHz 16000
//Used by PSP
#define GVRate_11KHz 11025

//For backwards compatability
#define GV_SAMPLES_PER_SECOND    (gvGetSampleRate())
#define GV_BYTES_PER_SECOND      (gvGetSampleRate() * GV_BYTES_PER_SAMPLE)

//TYPES
///////

//////////////////////////////////////////////////////////////
// GVCodec
// Summary
//		Identifies each of the default codecs available.
// Remarks
//		The codecs are arranged in order of descending quality and bandwidth. 
//		In other words, the codecs higher up on the list are of higher audio
//		 quality and use more bandwidth, 
//		while the codecs lower on the list are of lower audio quality and
//		 use less bandwidth.<p>
//		The GVCodecAverage codec produces good quality audio with a
//		 reasonable bandwidth cost. 
//		It is generally the best codec to use, and you should only use
//		 another codec if you are restricted to 
//		lower bandwidth or need high quality audio.
//		The particular stats for a codec can be obtained using gvGetCodecInfo.
// See Also
//		gvGetCodecInfo
typedef enum
{
	GVCodecRaw,
	GVCodecSuperHighQuality,
	GVCodecHighQuality,
	GVCodecAverage,
	GVCodecLowBandwidth,
	GVCodecSuperLowBandwidth
} GVCodec;

//////////////////////////////////////////////////////////////
// GVHardwareType
// Summary
//		Hardware type of a device.
typedef enum
{
	GVHardwareDirectSound,   // Win32
	GVHardwarePS2Spu2,       // PS2 (System output)
	GVHardwarePS2Headset,    // PS2 (USB)
	GVHardwarePS2Microphone, // PS2 (USB)
	GVHardwarePS2Speakers,   // PS2 (USB)
	GVHardwarePS2Eyetoy,     // PS2 (USB)
	GVHardwarePS3Headset,    // PS3 (USB/Bluetooth)
	GVHardwarePS3Eyetoy,     // PS3 (USB)
	GVHardwarePSPHeadset,    // PSP
	GVHardwareMacOSX,        // MacOSX
	GVHardwareCustom         // Any
} GVHardwareType;

//////////////////////////////////////////////////////////////
// GVCaptureMode
// Summary
//		enums used with gvSetCaptureMode() and gvGetCaptureMode().
// Remarks
//		The default mode for the SDK is GVCaptureModeThreshold. In
//		 GVCaptureModeThreshold , 
//		a capture device is on and captures speech based on the current
//		 threshold value.
//		When you change to GVCaptureModePushToTalk, the SDK will save the
//		 current Threshold value, 
//		set the threshold value to 0, and stop the capture device.
//		This mode also allows use of the following functions:
//		 gvSetPushToTalk(), gvGetPushToTalk().
//		When you switch the captureMode to GVCaptureModeThreshold, the saved
//		 Threshold value will be 
//		restored and the capture device will be started.
//		If the device is not a capture device, gvSetCaptureMode() will assert.<p>
// See Also
//		gvSetCaptureMode, gvGetCaptureMode
typedef enum
{
	GVCaptureModeThreshold,	// mode captures speech based on the current threshold value.
	GVCaptureModePushToTalk	// mode captures speech when gvSetPushToTalk is turned on.
} GVCaptureMode;

typedef int GVBool;
#define GVFalse 0
#define GVTrue 1

typedef gsi_u8                 GVByte;
typedef gsi_i16                GVSample;

typedef int                    GVRate;

#if defined(_PSP) || defined(_PS2) || defined(_PS3)
	typedef float              GVScalar;  // range 0-1
#else
	typedef double             GVScalar;  // range 0-1
#endif
typedef gsi_u16                GVFrameStamp;
typedef void *                 GVDecoderData;
typedef int                    GVDeviceType;
typedef struct GVIDevice *     GVDevice;

#if defined(GV_CUSTOM_SOURCE_TYPE)
typedef GV_CUSTOM_SOURCE_TYPE  GVSource;
#else
typedef int                    GVSource;
#endif

#if defined(_WIN32)
typedef GUID                   GVDeviceID;
#else
typedef int                    GVDeviceID;
#endif

//////////////////////////////////////////////////////////////
// GVDeviceInfo
// Summary
//		Information for an audio device.
// Members/Constants
//		m_id	
//		m_name	
//		m_deviceType	
//		m_defaultDevice	
//		m_hardwareType	
// See Also
//		gvListDevices, GVHardwareType
typedef struct
{
	GVDeviceID m_id;						// Used if you initialize this device with gvNewDevice.
	gsi_char m_name[GV_DEVICE_NAME_LEN];	// A user-readable name for the device.
	GVDeviceType m_deviceType;				// Indicates if this device is for capture, playback, or 
	//											both capture and playback.
	GVDeviceType m_defaultDevice;			// Indicates if this device is the default capture device, 
	//											default playback device, both, or neither. If neither, 
	//											the value will be 0 (this will always be the case on the PS2).
	GVHardwareType m_hardwareType;			// More information about the device's actual hardware. 
	//											Will differ based on platform. See GVHardwareType for settings.
} GVDeviceInfo;

//////////////////////////////////////////////////////////////
// GVCustomCodecInfo
// Summary
//		Information to define a custom codec.
// Members/Constants
//		m_samplesPerFrame	
//		m_encodedFrameSize	
//		m_newDecoderCallback	
//		m_freeDecoderCallback	
//		m_encodeCallback	
//		m_decodeCallback	
// See Also
//		gvSetCustomCodec
typedef struct
{
	int m_samplesPerFrame;  // Number of samples in an unencoded frame.
	int m_encodedFrameSize; // Number of bytes in an encoded frame.

	GVBool (* m_newDecoderCallback)(GVDecoderData * data);	// Used to allocate a new decoder instance for each 
	//															incoming source.
	void (* m_freeDecoderCallback)(GVDecoderData data);		// Used to free decoder data allocated through 
	//															m_newDecoderCallback.

	void (* m_encodeCallback)(GVByte * out, const GVSample * in);	// Used to encode data.
	void (* m_decodeAddCallback)(GVSample * out, const GVByte * in, GVDecoderData data);  // Called to decode data. 
	//																		Decode must add to, not set the output.
	void (* m_decodeSetCallback)(GVSample * out, const GVByte * in, GVDecoderData data);  // sets output (optional)
} GVCustomCodecInfo;

//GLOBALS
/////////
#if defined(_WIN32)
extern const GVDeviceID GVDefaultCaptureDeviceID;
extern const GVDeviceID GVDefaultPlaybackDeviceID;
#elif defined(_PS2)
extern const GVDeviceID GVPS2Spu2DeviceID;
#endif

//GENERAL
/////////
#if defined(_WIN32)

//////////////////////////////////////////////////////////////
// gvStartup
// Summary
//		Initializes the SDK.
// Parameters
//		hWnd	: [in] Handle to the application's main window. [Win32 only]
// Returns
//		Returns GVTrue if the SDK was able to startup successfully.
// Remarks
//		Before doing anything else with GV, you must call gvStartup.
//		The function does any necessary internal initialization.
//		It will return GVFalse in case of an error initializing.
//		The HWND passed to the Win32 version is the handle for the
//		 application's main window.
//		This can be NULL if the application does not have a main window.<p>
// See Also
//		gvCleanup
GVBool gvStartup(HWND hWnd);
#else
GVBool gvStartup(void);
#endif

//////////////////////////////////////////////////////////////
// gvCleanup
// Summary
//		Performs any necessary internal cleanup.
//		GV cannot be used again until gvStartup is called.
// See Also
//		gvStartup
void gvCleanup(void);

//////////////////////////////////////////////////////////////
// gvThink
// Summary
//		Allows playback devices to play audio scheduled for playback.
// Remarks
//		gvPlayPacket only schedules a packet to be played in the future.
//		The application must also call gvThink on a regular basis to ensure
//		 that the packets are actually played.<p>
//		gvThink will check, for each device, how much space has become
//		 available for writing in the playback buffer 
//		(which may or may not be on the actual sound hardware). It will then
//		 check to see if the device has any sources 
//		that have audio which should be played during the time period that
//		 the newly available space represents. 
//		If so, the audio will be mixed into the playback buffer, and the
//		 audio will then be played when the 
//		playback position reaches that point in the buffer. If the playback
//		 device is stopped before that happens, 
//		then the audio will not be played, even if the device is then restarted.<p>
//		gvThink should generally be called once for each run through the
//		 application's main loop, 
//		or approximately every 10-30ms. If it is not called often enough,
//		 the playback position will reach a 
//		point in the playback buffer that GV has not yet had a chance to mix
//		 to, resulting in an audible skipping effect.
// See Also
//		gvPlayPacket
void gvThink(void);

//CODEC
///////
//////////////////////////////////////////////////////////////
// gvSetCodec
// Summary
//		Sets the codec to be used by the SDK.
// Parameters
//		codec	: [in] The codec identifier.
// Returns
//		Returns GVTrue if successful; otherwise, GVFalse.
// Remarks
//		The first thing to do after initializing the SDK is to set the codec
//		 you would like to use.
//		The codec cannot be changed while any devices are initialized, so an
//		 application will typically 
//		just set the codec once, when it starts using voice.<p>
//		The codec must be one of the following values:
//		GVCodecSuperHighQuality
//		GVCodecHighQuality
//		GVCodecAverage
//		GVCodecLowBandwidth
//		GVCodecSuperLowBandwidth.

// See Also
//		gvGetCodecInfo, gvSetCustomCodec
GVBool gvSetCodec(GVCodec codec);

//////////////////////////////////////////////////////////////
// gvSetCustomCodec
// Summary
//		Tells GV to use an application-provided codec instead of a built-in codec.
// Parameters
//		info	: [in] The application fills in this structure with the
//		 information that the SDK needs to use the custom codec.
// Remarks
//		The first thing to do after initializing the SDK is to set the codec
//		 you would like to use.
//		The codec cannot be changed while any devices are initialized, so an
//		 application will typically 
//		just set the codec once, when it starts using voice.<p>
//		Before calling this function, the application must fill in the
//		 GVCustomCodecInfo structure with information 
//		about the codec to be used.
//		m_samplesPerFrame is the number of samples that this codec expects
//		 in a raw (unencoded) frame of audio. 
//		This can be whatever value is used by the codec, however it is
//		 typically about 160 samples.
//		m_encodedFrameSize is the number of bytes in an encoded frame of
//		 audio produced by this codec. 
//		The ratio of the samples per frame and encoded frame size is
//		 directly related to the codec's output 
//		stream bit rate.
//		m_newDecoderCallback is used to allocate a new decoder instance for
//		 each incoming source. 
//		Some codecs do not require this, and they should set the
//		 m_newDecoderCallback member to NULL. 
//		For codecs that do require per-source data, they should allocate a
//		 new decoder data state and set 
//		the data parameter to point to it, then return TRUE. If they cannot
//		 allocate a new decoder data, 
//		then they should return GVFalse.
//		The m_freeDecoderCallback is used to free decoder data allocated
//		 through m_newDecoderCallback. 
//		If a codec set m_newDecoderCallback to NULL, it should set
//		 m_freeDecoderCallback to NULL as well. 
//		Otherwise it should provide a function that frees the decoder data.
//		m_encodeCallback is used to encode data. The in parameter points to
//		 the input data, 
//		with is a single raw frame of samples. The number of samples passed
//		 to this function will always be
//		m_samplesPerFrame. The out parameter points to the memory into which
//		 the callback should decode the input data. 
//		The memory will always be large enough to hold one frame of
//		 compressed audio, which will always be 
//		m_encodedFrameSize bytes long. 
//		m_decodeCallback is used to decode data. The in parameter will point
//		 to an encoded frame of audio, 
//		which will be m_encodedFrameSize bytes long. The out parameter which
//		 will be large enough to hold 
//		m_samplesPerFrame samples of audio. The decoder data is provided for
//		 codecs that need it. 
//		The important thing to know with the decode callback is that it
//		 should not decode directly into the out buffer, 
//		but it should add to it. This allows GV to decode and mix at the
//		 same time, without having to decode into a 
//		temporary buffer which would then be mixed into the output stream.
// See Also
//		gvSetCodec
void gvSetCustomCodec(GVCustomCodecInfo * info);

//////////////////////////////////////////////////////////////
// gvGetCodecInfo
// Summary
//		Obtains the particular stats for the codec.
// Parameters
//		samplesPerFrame		: [out] The samples per frame.
//		encodedFrameSize	: [out] The encoded frame size in bytes
//		bitsPerSecond		: [out] The bits per second.
// Remarks
//		  Note that the bits per second doesn't take into account any
//		 overhead, such as the need to
//		transmit a frame stamp value with each packet.
//		It is based only on the encoded frame size and the number of frames
//		 per second.<p>
// See Also
//		gvSetCodec
void gvGetCodecInfo(int * samplesPerFrame, int * encodedFrameSize, int * bitsPerSecond);

//SAMPLE RATE
/////////////

void gvSetSampleRate(GVRate sampleRate);
GVRate gvGetSampleRate(void);

//DEVICES
/////////

//////////////////////////////////////////////////////////////
// gvListDevices
// Summary
//		Gets a list of devices available on the system.
// Parameters
//		devices		: [out] The list of device details to be filled in by the function.
//		maxDevices	: [in] The number of elements in the devices array.
//		types		: [in] The types of devices to survey.
// Returns
//		Returns the number of devices that it put into the list. Return
//		 value of 0 may indicate an error 
//		or that no devices were found.
// Remarks
//		 You can request capture devices with GV_CAPTURE, playback devices
//		 with GV_PLAYBACK, 
//		or capture and playback devices with GV_CAPTURE_AND_PLAYBACK.
//		For GV_CAPTURE_AND_PLAYBACK, it can list capture devices, playback devices, 
//		and devices that support both capture and playback.<p>
// See Also
//		GVDeviceInfo
int gvListDevices(GVDeviceInfo devices[], int maxDevices, GVDeviceType types);

#if defined(_WIN32)
//////////////////////////////////////////////////////////////
// gvRunSetupWizard
// Summary
//		Interacts with the user to set up the capture and playback devices.
// Parameters
//		captureDeviceID	: [in] Id of the capture device to set up
//		playbackDeviceID	: [in] Id of the playback device to set up
// Returns
//		Returns GVTrue if the user successfully completes the wizard,
//		 GVFalse otherwise.
// Remarks
//		For use with Win32 only, if the user has DirectX 8 or greater.<p>
//		The wizard will take over control of the program while it executes.
//		 It has the user speak into 
//		the capture device, and monitors the audio to automatically set
//		 system level capture and playback volumes. 
//		gvRunSetupWizard stores setup information in the registry.
// See Also
//		gvAreDevicesSetup
GVBool gvRunSetupWizard(GVDeviceID captureDeviceID, GVDeviceID playbackDeviceID);

//////////////////////////////////////////////////////////////
// gvAreDevicesSetup
// Summary
//		Determines if the registry has information on the specified device pair.
// Parameters
//		captureDeviceID	: [in] Reference to the device used to capture audio
//		playbackDeviceID	: [in] Reference to the device used to playback audio
// Returns
//		Returns GVTrue if gvRunSetupWizard has been run on the pair;
//		 otherwise, GVFalse.
// Remarks
//		If gvAreDevicesSetup returns GVTrue, and the gvRunSetupWizard does
//		 not need to be run again.
//		If it returns GVFalse, then the gvRunSetupWizard has not been run
//		 for the pair of devices.<p>
// See Also
//		gvNewDevice, gvRunSetupWizard
GVBool gvAreDevicesSetup(GVDeviceID captureDeviceID, GVDeviceID playbackDeviceID);
#endif

//////////////////////////////////////////////////////////////
// gvNewDevice
// Summary
//		Initializes a device.
// Parameters
//		deviceID	: [in] The ID for the device to be initialized
//		type		: [in] Specifies whether device handles capture or playback or both
// Returns
//		If the device was successfully initialized, a handle to the device
//		 will be returned.  
//		If there was an error setting up the device, NULL will be returned.
// Remarks
//		A device that supports both capture and playback may be initialized
//		 for just one or the other or both.<p>
// See Also
//		gvFreeDevice, gvNewCustomDevice
GVDevice gvNewDevice(GVDeviceID deviceID, GVDeviceType type);

//////////////////////////////////////////////////////////////
// gvFreeDevice
// Summary
//		Frees a device so that GV can clean it up.
// Parameters
//		device	: [in] The handle to the deviced to be freed.
// Remarks
//		Once a device has been freed it can no longer be used.
//		After calling this function you should set the variable in which you
//		 stored the device handle to NULL.<p>
// See Also
//		gvNewDevice, gvStartDevice, gvStopDevice
void gvFreeDevice(GVDevice device);

//////////////////////////////////////////////////////////////
// gvStartDevice
// Summary
//		Starts a device capturing and/or playing audio.
// Parameters
//		device	: [in] The handle to the device.
//		type	: [in] Specifies capture, playback, or both.
// Returns
//		GVTrue if the device was started successfully.
// Remarks
//		Once a device has been initialized, it is ready to start capturing
//		 or playing audio.
//		After a capture device is started, it will begin capturing audio and
//		 passing it back to the application.
//		After a playback device is started, it will play any audio that the
//		 application passes it.
//		To start a device, use gvStartDevice.<p>
//		The device parameter is the handle of the device to start. The type
//		 parameter specifies if the device 
//		should start capturing (GV_CAPTURE), playing (GV_PLAYBACK), or
//		 capturing and playing (GV_CAPTURE_AND_PLAYBACK). 
//		For devices that support both capture and playback, each can be
//		 started independently. 
//		The function will return GVTrue if the device was started
//		 successfully, and it will return GVFalse if 
//		there was an error.
// See Also
//		gvStopDevice, gvIsDeviceStarted
GVBool gvStartDevice(GVDevice device, GVDeviceType type);

//////////////////////////////////////////////////////////////
// gvStopDevice
// Summary
//		Stops a device that is capturing and/or playing audio.
// Parameters
//		device	: [in] The handle to the device.
//		type	: [in] Specifies capture, playback, or both.
// Remarks
//		When you want a device to stop capturing or playing, use gvStopDevice.
//		When a capture device is stopped, it will stop passing captured
//		 audio to the application.
//		When a playback device is stopped, it will stop playing audio.
//		For devices that support both capture and playback, each can be
//		 stopped independently.<p>
// See Also
//		gvStartDevice, gvIsDeviceStarted
void gvStopDevice(GVDevice device, GVDeviceType type);

//////////////////////////////////////////////////////////////
// gvIsDeviceStarted
// Summary
//		Checks to see if a whether or not a device has been started as the
//		 given device type.
// Parameters
//		device	: [in] The handle to the device.
//		type	: [in] Specifies capture or playback device.
// Returns
//		Returns GVTrue if the device has been started, GVFalse if not.
// See Also
//		gvStartDevice, gvStopDevice
GVBool gvIsDeviceStarted(GVDevice device, GVDeviceType type);

//////////////////////////////////////////////////////////////
// gvSetDeviceVolume
// Summary
//		Sets a device's volume.
// Parameters
//		device	: [in] The handle to the device.
//		type	: [in] Specifies setting the capture volume, playback volume, or both.
//		volume	: [in] The volume, ranging from 0.0 to 1.0.
// See Also
//		gvGetDeviceVolume
void gvSetDeviceVolume(GVDevice device, GVDeviceType type, GVScalar volume);

//////////////////////////////////////////////////////////////
// gvGetDeviceVolume
// Summary
//		Gets the volume from a capture or playback device.
// Parameters
//		device	: [in] The handle to the device.
//		type	: [in] Specifies either the playback volume or the capture volume
// Returns
//		Returns the specified volume for the device.
// Remarks
//		The volume range is 0.0 to 1.0.<p>
//		The type parameter controls if this gets set as a capture volume
//		 (GV_CAPTURE), a playback volume (GV_PLAYBACK). 
//		For a device that supports both capture and playback, this function
//		 can only be used to get either 
//		the capture volume or the playback volume, not both.
// See Also
//		gvSetDeviceVolume
GVScalar gvGetDeviceVolume(GVDevice device, GVDeviceType type);

//////////////////////////////////////////////////////////////
// gvUnpluggedCallback
// Summary
//		Called when a device has been unplugged.
// Parameters
//		device	: [in] The handle to the device.
// Remarks
//		A gvUnpluggedCallback allows an application to know if a device is
//		 unplugged or otherwise stops working.
//		The callback will be called when the SDK detects that the device has
//		 been unplugged.
//		The device will be freed by the SDK immediately after the callback
//		 returns and cannot be used again
//		by the application.<p>
// See Also
//		gvSetUnpluggedCallback
typedef void (* gvUnpluggedCallback)(GVDevice device);

//////////////////////////////////////////////////////////////
// gvSetUnpluggedCallback
// Summary
//		Sets a callback to be called when the SDK detects that the device
//		 was unplugged or is no longer functioning.
// Parameters
//		device				: [in] The handle to the device.
//		unpluggedCallback	: [in] The callback to set.  Can be NULL.
// Remarks
//		A gvUnpluggedCallback allows an application to know if a device is
//		 unplugged or otherwise stops working.
//		The callback will be called when the SDK detects that the device has
//		 been unplugged.
//		The device will be freed by the SDK immediately after the callback
//		 returns and cannot be used again by the
//		application.<p>
// See Also
//		gvUnpluggedCallback
void gvSetUnpluggedCallback(GVDevice device, gvUnpluggedCallback unpluggedCallback);

//////////////////////////////////////////////////////////////
// gvFilterCallback
// Summary
//		Used to filter audio that has either been captured or is about to be played.
//		Can also be used to monitor audio.
// Parameters
//		device		: [in] The handle to the device.
//		audio		: [ref] A frame of audio to be filtered.
//		frameStamp	: [in] The framestamp for the frame of audio.
// Remarks
//		Filtering allows you to process, or just monitor, audio that has
//		 been captured or is being played.
//		The audio will always be a single raw frame of audio.
//		Use gvGetCodecInfo to get the number of samples in a raw frame.<p>
//		The callback can modify the audio in any way that it wants. However
//		 once the function returns 
//		it can no longer access the audio. For capture devices, audio will
//		 only be passed to the filter 
//		if it crosses the threshold (if one is set). For playback devices,
//		 audio is filtered after all of 
//		the sources have been mixed.
// See Also
//		gvSetFilter
typedef void (* gvFilterCallback)(GVDevice device, GVSample * audio, GVFrameStamp frameStamp);

//////////////////////////////////////////////////////////////
// gvSetFilter
// Summary
//		Sets a device's filter callback.
// Parameters
//		device		: [in] The handle to the device.
//		type		: [in] Set the filter on capture, playback, or both.
//		callback	: [in] The filter callback to use.
// Remarks
//		Filtering allows you to process, or just monitor, audio that has
//		 been captured or is being played.
//		A filter callback, prototyped as the gvFilterCallback type, is
//		 passed the device the filtering is happening on, the audio to
//		 filter, and the audio's frame stamp.
//		The audio will always be a single raw frame of audio.
//		Use gvGetCodecInfo to get the number of samples in a raw frame.<p>
//		The callback can modify the audio in any way that it wants. However
//		 once the function returns it 
//		can no longer access the audio. For capture devices, audio will only
//		 be passed to the filter if it 
//		crosses the threshold (if one is set). For playback devices, audio
//		 is filtered after all of the sources 
//		have been mixed.
//		You can use gvSetFilter to set a filter on any device, and to set it
//		 for capture or playback. 
//		A device can have only one capture filter at a time and only one
//		 playback filter at a time. 
//		To clear a filter, call this function with the callback set to NULL.
// See Also
//		gvFilterCallback
void gvSetFilter(GVDevice device, GVDeviceType type, gvFilterCallback callback);

//////////////////////////////////////////////////////////////
// gvCapturePacket
// Summary
//		Takes captured audio data out of the internal capture buffer,
//		 storing it in the provided packet memory block.
// Parameters
//		device		: [in] A handle to the capture device
//		packet		: [in] A block of memory to receive the data
//		len			: [ref] The maximum / number of bytes moved into the
//		 memory block specified by packet parameter.
//		frameStamp	: [out] The frame stamp for the captured packet
//		volume		: [out] The peak volume for the audio in the frame
// Returns
//		GVTrue if successful in getting a packet and encoding it into the
//		 provided memory block; GVFalse  
//		if either no audio data was available to capture or an error
//		 occurred while capturing the audio.
// Remarks
//		  The packet parameter must be large enough to hold at least one
//		 encoded frame 
//		(gvGetCodecInfo can be used to get the size of an encoded frame).
//		The function will fill this memory with as many encoded frames as it can.<p>
//		The len parameter must point to an int which is set to the maximum
//		 number of bytes that can be written 
//		to the block of memory pointed to be the packet parameter. After the
//		 function returns, 
//		if it was successful, len will point to the number of bytes that
//		 were written to the block of memory. 
//		The frameStamp parameter will receive the frame stamp for the
//		 captured packet, and the volume parameter, 
//		the peak volume for the audio in the frame. The volume ranges from
//		 0.0 to 1.0, and it can be used to power 
//		a per-player graphic voice activity meter.
// See Also
//		gvGetAvailableCaptureBytes, gvGetCodecInfo
GVBool gvCapturePacket(GVDevice device, GVByte * packet, int * len, GVFrameStamp * frameStamp, GVScalar * volume);

//////////////////////////////////////////////////////////////
// gvGetAvailableCaptureBytes
// Summary
//		Discovers how many bytes are currently available for capture on the
//		 given device.
// Parameters
//		device	: [in] The handle to the device.
// Returns
//		Returns the number of bytes available.
// Remarks
//		To determine the number of encoded frames, divide the return value
//		 by the number of bytes in 
//		an encoded frame (which you can get with gvGetCodecInfo).<p>
//		Note that even if there are bytes available, gvCapturePacket may
//		 return GVFalse. 
//		This could happen if a capture threshold has been set, and the voice
//		 audio does not cross the threshold. 
//		In that case GV would skip over that captured audio, and its bytes
//		 would no longer count as available bytes.
// See Also
//		gvCapturePacket, gvGetCodecInfo
int gvGetAvailableCaptureBytes(GVDevice device);

//////////////////////////////////////////////////////////////
// gvSetCaptureThreshold
// Summary
//		Sets the threshold volume on a device. A packet will only be passed
//		 to the application 
//		if its peak volume is at least as high as the capture threshold.
// Parameters
//		device		: [in] The handle to the device.
//		threshold	: [in] The threshold volume
// Remarks
//		The range for threshold is 0.0 to 1.0.
//		A value of approximately 0.10 to 0.15 will generally work well,
//		 although ideally the 
//		user should have a way to configure the threshold.
//		GV will continue passing packets to the application for about half a
//		 second after 
//		the peak volume drops below the threshold.
//		This helps to catch speech in which the level trails off or has a small dip.
//		The default threshold is 0.0, which means that all audio will be
//		 considered over the 
//		threshold and will be captured.
//		To remove a threshold that has been set, simply call this function
//		 again with a threshold of 0.0.<p>
// See Also
//		gvGetCaptureThreshold
void gvSetCaptureThreshold(GVDevice device, GVScalar threshold);

//////////////////////////////////////////////////////////////
// gvGetCaptureThreshold
// Summary
//		Gets the current value of the capture threshold for the device.
// Parameters
//		device	: [in] The handle to the capture device.
// Returns
//		Returns the current threshold value.
// Remarks
//		Retrieves the value that was assigned by gvSetCaptureThreshold.<p>
// See Also
//		gvSetCaptureThreshold
GVScalar gvGetCaptureThreshold(GVDevice device);

//////////////////////////////////////////////////////////////
// gvSetCaptureMode
// Summary
//		Sets the capture mode for the device.
// Parameters
//		device		: [in] The handle to the capture device.
//		captureMode	: [in] The new capture mode.
// Remarks
//		The default mode for the SDK is GVCaptureModeThreshold. In
//		 GVCaptureModeThreshold , 
//		a capture device is on and captures speech based on the current
//		 threshold value.
//		When you change to GVCaptureModePushToTalk, the SDK will save the
//		 current Threshold value, 
//		set the threshold value to 0, and stop the capture device.
//		This mode also allows use of the following functions:
//		 gvSetPushToTalk(), gvGetPushToTalk().
//		When you switch the captureMode to GVCaptureModeThreshold, the saved
//		 Threshold value will be 
//		restored and the capture device will be started.
//		If the device is not a capture device, gvSetCaptureMode() will assert.<p>
// See Also
//		gvGetCaptureMode, GVCaptureMode
void gvSetCaptureMode(GVDevice device, GVCaptureMode captureMode);

//////////////////////////////////////////////////////////////
// gvGetCaptureMode
// Summary
//		Gets the capture mode for the device.
// Parameters
//		device	: [in] The handle to the capture device.
// Returns
//		The current capture mode for this device.
// See Also
//		gvSetCaptureMode, GVCaptureMode
GVCaptureMode gvGetCaptureMode(GVDevice device);

//////////////////////////////////////////////////////////////
// gvSetPushToTalk
// Summary
//		Used to turn on or off capturing for a device. Must be in
//		 GVCaptureModePushToTalk mode.
// Parameters
//		device	: [in] The handle to the capture device.
//		talkOn	: [in] GVTrue to start capture device and capture speech,
//		 GVFalse to turn off capture device.
// Remarks
//		When called with talkOn true, the device will start the capture
//		 device and capture all speech.
//		When set to false, the capture device is turned off.<p>
// See Also
//		gvGetPushToTalk, gvSetCaptureMode
void gvSetPushToTalk(GVDevice device, GVBool talkOn);

//////////////////////////////////////////////////////////////
// gvGetPushToTalk
// Summary
//		Tells you if PushToTalk is currently turned on or off.
// Parameters
//		device	: [in] The handle to the capture device.
// Returns
//		GVTrue if turned on, GVFalse if turned off.
// See Also
//		gvSetPushToTalk, gvSetCaptureMode
GVBool gvGetPushToTalk(GVDevice device);

//PLAYBACK
//////////

//////////////////////////////////////////////////////////////
// gvPlayPacket
// Summary
//		Plays a packet retrieved from the capture buffer.
// Parameters
//		device		: [in] The handle to the playback device.
//		packet		: [in] The packet with audio data.
//		len			: [in] The packet's length.
//		source		: [in] The source that originated the audio.
//		frameStamp	: [in] The packet's frame stamp.
//		mute		: [in] Mutes the packet - allows having a player muted, but keeping 
//		track of the fact that the source is really speaking
// Remarks
//		  GV will schedule the packet to be played soon.
//		A short delay is added which enables the packets to be synchronized
//		 before they are played, 
//		allowing for variations in Internet transit time and application timing.
//		The packet is synchronized based on its source, so you must ensure
//		 that each unique 
//		talker has his own unique source, and all packets are played using
//		 the correct source.
//		Note that the same packet can be played on multiple playback devices.<p>
// See Also
//		gvCapturePacket, gvThink
void gvPlayPacket(GVDevice device, const GVByte * packet, int len, GVSource source, GVFrameStamp frameStamp, GVBool mute);

//////////////////////////////////////////////////////////////
// gvIsSourceTalking
// Summary
//		Determines if a particular source is currently talking on the
//		 specified device.
// Parameters
//		device	: [in] The handle to the device.
//		source	: [in] The source identifier.
// Returns
//		Returns GVTrue if the source is talking on the specified device;
//		 otherwise, GVFalse.
// See Also
//		gvListTalkingSources
GVBool gvIsSourceTalking(GVDevice device, GVSource source);

//////////////////////////////////////////////////////////////
// gvListTalkingSources
// Summary
//		Gets a list of all of the sources that are currently talking on a
//		 particular device.
// Parameters
//		device		: [in] The handle of the device.
//		sources		: [out] An array to receive the sources, filled in by the function.
//		maxSources	: [in] The number of elements in the sources array.
// Returns
//		Returns the number of sources that are talking on the device.
// Remarks
//		The function will return the number of sources that were talking on
//		 the device, and it will 
//		store their GVSources in the sources array.
//		0 will be returned if there are no sources talking.<p>
//		GV has a hardcoded limit that does not allow more than 8 sources to
//		 talk simultaneously. 
//		This allows it to preallocate memory that it needs to store for a
//		 source while it is talking. 
//		A user will typically not understand more than 2 or 3 users talking
//		 simultaneously, 
//		so the limit should be high enough. If the application attempts to
//		 play audio from 
//		more than 8 sources at a time, audio for the 9th source will be
//		 automatically dropped.
// See Also
//		gvIsSourceTalking
int gvListTalkingSources(GVDevice device, GVSource sources[], int maxSources);

//////////////////////////////////////////////////////////////
// gvSetGlobalMute
// Summary
//		Sets the global mute value - defaults to false.
// Parameters
//		mute	: [in] Set to GVTrue to globally mute all play packets.
// Remarks
//		When gvSetGlobalMute mute is true, all data passed to gvPlayPacket
//		 will not played on the playback device.
//		You will still be able to check the gvIsSourceTalking to know that
//		 you are sending you voice packets to play.
//		When gvSetGlobalMute is false, all gvPlayPacket data will be played,
//		 if the gvPlayPacket mute is false.<p>
// See Also
//		gvGetGlobalMute, gvPlayPacket
void gvSetGlobalMute(GVBool mute);

//////////////////////////////////////////////////////////////
// gvGetGlobalMute
// Summary
//		Gets the current status of global mute.
// Returns
//		GVTrue if global mute turned on, GVFalse if off.
// See Also
//		gvSetGlobalMute
GVBool gvGetGlobalMute(void);

//CUSTOM DEVICE
///////////////

//////////////////////////////////////////////////////////////
// gvNewCustomDevice
// Summary
//		Creates a custom device, which allows an application to supply its
//		 own audio hardware interface.
// Parameters
//		type	: [in] Specifies whether device handles capture or playback or both
// Returns
//		Returns a handle to the new custom device if successful; NULL if it
//		 cannot create the device.
// See Also
//		gvGetCustomPlaybackAudio
GVDevice gvNewCustomDevice(GVDeviceType type);

// for both of these, numSamples must be a multiple of the codec's
//		 samplesPerFrame
// this ensures that no data needs to be buffered by the SDK

//////////////////////////////////////////////////////////////
// gvGetCustomPlaybackAudio
// Summary
//		Retrieves any audio data that is ready to be played through a custom
//		 playback device.
// Parameters
//		device		: [in] The custom playback device from which to retrieve audio.
//		audio		: [out] Buffer to fill with audio samples to be played
//		 by the custom device.
//		numSamples	: [in] Size of the audio buffer in samples.  Must be a
//		 multiple of the current samplesPerFrame.
// Returns
//		GVTrue if the audio buffer was filled, GVFalse otherwise.
// Remarks
//		numSamples must be a multiple of the samples per frame for the
//		 current codec (which you can check 
//		using gvGetCodecInfo).
//		This is because GV mixes audio a frame at a time.<p>
//		This function should be called at the same rate at which the custom
//		 playback device is actually playing audio. 
//		In other words, the physicial device should be pulling data with
//		 this function when it needs it - 
//		the data is not being pushed to the physical device. This is because
//		 the SDK compensates for differences 
//		in audio clock rates, and calling it at the correct rate will ensure
//		 a consistent rate of playback.
//		The GV_SAMPLES_PER_SECOND and GV_BITE_PER_SAMPLE defines can be used
//		 to determine the sample rate and bitrate 
//		of the audio.
// See Also
//		gvNewCustomDevice
GVBool gvGetCustomPlaybackAudio(GVDevice device, GVSample * audio, int numSamples);

//////////////////////////////////////////////////////////////
// gvSetCustomCaptureAudio
// Summary
//		For a custom capture device, encodes captured audio from a stream
//		 into a packet, storing it at provided memory.
// Parameters
//		device		: [in] The handle to the custom capture device.
//		audio		: [in] The incoming audio stream.
//		numSamples	: [in] The number of samples to capture.
//		packet		: [out] The memory location where the packet will be stored.
//		packetLen	: [ref] The number of bytes available in the packet,
//		frameStamp	: [out] Receives the frame stamp for the packet.
//		volume		: [out] Receives the peak volume for the packet.
// Returns
//		Returns GVTrue if the function successfully encodes the audio into
//		 the packet, otherwise GVFalse. 
//		GVFalse will be returned if a threshold is set and the audio's peak
//		 volume did not cross the threshold.
// Remarks
//		The numSamples parameter must be a multiple of the codec's
//		 samplesPerFrame; this ensures that 
//		no data needs to be buffered by the SDK.<p>
// See Also
//		gvGetCodecInfo
GVBool gvSetCustomCaptureAudio(GVDevice device, const GVSample * audio, int numSamples,
                               GVByte * packet, int * packetLen, GVFrameStamp * frameStamp, GVScalar * volume);

//CHANNELS
//////////
int gvGetNumChannels(GVDevice device, GVDeviceType type);
void gvGetChannelName(GVDevice device, GVDeviceType type, int channel, gsi_char name[GV_CHANNEL_NAME_LEN]);
void gvSetChannel(GVDevice device, GVDeviceType type, int channel);
int gvGetChannel(GVDevice device, GVDeviceType type);

#if defined(__cplusplus)
}
#endif

#endif
