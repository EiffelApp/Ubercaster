//
//  UCViewController.h
//  Ubercaster
//
//  Created by Emanuel on 03/01/14.
//  Copyright (c) 2014 EchosDesign. All rights reserved.
//

#import <UIKit/UIKit.h>

@class UCAppDelegate;

@interface UCViewController : UIViewController {
    UCAppDelegate *appDelegate;
    bool isRunning;
}

@property (weak, nonatomic) IBOutlet UIButton *ButtonStartStop;

@property (weak, nonatomic) IBOutlet UILabel *LabelVersion;

@property (weak, nonatomic) IBOutlet UISlider *SliderBuffer;

- (IBAction)ButtonStartStopClick:(id)sender;

@end
