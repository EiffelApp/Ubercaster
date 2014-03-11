//
//  UCAppDelegate.h
//  Ubercaster
//
//  Created by Emanuel on 03/01/14.
//  Copyright (c) 2014 EchosDesign. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "UCAudioLib/UCAudioLib.h"

#define APP_VERSION @"Version 1.01"

@interface UCAppDelegate : UIResponder <UIApplicationDelegate> {
    UCAudioLib *audioLib;
}

@property (strong, nonatomic) UIWindow *window;

- (void) audioStartWithBufferSize: (int)aux;  // From 4 to 100
- (void) audioStop;

@end
