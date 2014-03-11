//
//  UCAudioLib.h
//  UCAudioLib
//
//  Created by Emanuel on 03/01/14.
//  Copyright (c) 2014 EchosDesign. All rights reserved.
//
//  Version 1.01

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>

@interface UCAudioLib : NSObject

- (void) audioStartWithBufferSize: (int)aux;  // From 4 to 100
- (void) audioStop;

@end