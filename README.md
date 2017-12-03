# RetroSpySDK
RetroSpy is a project that aim to create a successfully working upgraded SDK and Server for GameSpy Services

### Platform Notes

Windows : Should work with Visual Studio 2008 (Visual Studio 2017 update later), Macros: _WIN32 (_WIN64 for 64bit) Compiler: Visual Studio (_MSC_VER)
Xbox : Not tested (missing XDK/XBox unit) Macros: _XBOX (If also define _WIN32) Compiler: Visual Studio (_MSC_VER)
Xbox360 : Not tested (missing X360DK/Xbox360 unit, even if i know that Xbox360 could load SDK-compiler binaries in retail unit with mods) Macros: _X360 (It also define _WIN32 and _XBOX) Compiler: Visual Studio (_MSC_VER)
MacOSX : Not tested (there's the XCode project but i don't own a Mac) Macros: _MACOSX (Also define _UNIX) Compiler: Clang (__clang__)
iOS (iPhone,iPad,iPod) : Not tested (there's the XCode project but i don't own a Mac) Macros: _IPHONE (Also define _UNIX) Compiler: Clang (__clang__)
Linux : Not tested (It will be tested later) Macros: _LINUX (Also define _UNIX) Compiler: Clang (__clang__) and GCC (__GCC__)
Nintendo DS : Missing NitroWifi SDK for compile Macros: _NITRO Compiler: CodeWarrior (__MWERKS__)
Nintendo Wii : NDEV Binaries (SDK-compiler binaries) dosen't work with retail unit (elf crashes after loaded from HBC) Macros: _REVOLUTION Compiler: CodeWarrior (__MWERKS__)
PSP: Not tested (Missing PSP) Macros: _PSP Compiler: GCC (__GCC__) (There is also a SNSystem plugin for VS)
PS3: Not tested (I don't have a CFW so i can't install a DEX FW) Macros: _PS3 Compiler: GCC (__GCC__) (There is also a SNSystem plugin for VS)
PS2: Not tested (I don't own a Network Adater) Macros: _PS2 (Also defines EENET,INSOCK and SN_SYSTEMS) Compiler: CodeWarrior (__MWERKS__) (There is also a SNSystem plugin for VS)
Android: Missing project files (not included in OpenSDK or 2007SDK)

### Compiler Notes

MSVC:
	Tested with VS2008, Upgrade to VS2007 will code later
	The _CRT_SECURE_NO_WARNINGS macro is defined
	
### How to build
	Visual Studio:
		Open SDK-VSXXX.sln
