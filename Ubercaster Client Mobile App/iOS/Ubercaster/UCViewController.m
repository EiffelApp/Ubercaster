//
//  UCViewController.m
//  Ubercaster
//
//  Created by Emanuel on 03/01/14.
//  Copyright (c) 2014 EchosDesign. All rights reserved.
//

#import "UCViewController.h"
#import "UCAppDelegate.h"

@interface UCViewController ()

@end

@implementation UCViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
	appDelegate = (UCAppDelegate *)[[UIApplication sharedApplication] delegate];
    
    self.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"Background.png"]];
    
    [_LabelVersion setText:APP_VERSION];
    
    isRunning=false;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)ButtonStartStopClick:(id)sender {
    if (isRunning) {
        [_ButtonStartStop setTitle:@"START" forState:UIControlStateNormal];
        [appDelegate audioStop];
        [_SliderBuffer setEnabled:true];
    } else {
        [_ButtonStartStop setTitle:@"STOP" forState:UIControlStateNormal];
        [appDelegate audioStartWithBufferSize:(int)[_SliderBuffer value]];
        [_SliderBuffer setEnabled:false];
    }
    isRunning=!isRunning;
}


@end
