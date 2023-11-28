// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2009 GameSpy Industries, Inc.

#import <UIKit/UIKit.h>
#import "sake/sakeRequest.h"

int main(int argc, char* argv[])
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	// code to wake up 3G
    NSURL * url = [[NSURL alloc] initWithString:[NSString stringWithCString:"http://www.g1a2m3e4s5p6y.com"]];
    [NSData dataWithContentsOfURL:url];
    [url release];
	
	int ret = UIApplicationMain(argc, argv, nil, nil);
	[pool release];
	return ret;
}