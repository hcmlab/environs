//
//  UserItem.h
//  ChatApp.Watch
//
//  Created by chi-tai on 10.08.15.
//  Copyright (c) 2015 University of Augsburg. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <WatchKit/WatchKit.h>

@interface UserItem : NSObject

@property (weak, nonatomic) IBOutlet WKInterfaceLabel *userNickname;
@property (weak, nonatomic) IBOutlet WKInterfaceImage *userPicture;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel *userText;

@end
