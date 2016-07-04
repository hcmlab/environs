//
//  InterfaceController.m
//  ChatApp.Watch WatchKit Extension
//
//  Created by chi-tai on 10.08.15.
//  Copyright (c) 2015 University of Augsburg. All rights reserved.
//

#import "InterfaceController.h"
#import "UserItem.h"


@interface InterfaceController()
{
    int userCount;
    int respSequence;
}

@property (weak, nonatomic) IBOutlet WKInterfaceTable *userList;

@end


@implementation InterfaceController

@synthesize userList;

static InterfaceController * instance = 0;


- (void) awakeWithContext:(id)context
{
    [super awakeWithContext:context];
    
    NSLog ( @"awakeWithContext" );
    
    userCount = 0;
    respSequence = 0;
    instance = self;
}


- (void) willActivate
{
    // This method is called when watch view controller is about to be visible to user
    [super willActivate];
    
    NSLog ( @"willActivate" );
    /*
    NSDictionary * request = @{ @"get":@"users", @"seq" : [NSNumber numberWithInt:respSequence] };
    
    [WKInterfaceController openParentApplication:request reply:^(NSDictionary * reply, NSError *error) {
        [self UpdateUserList:reply];
    }];
    */
    
    CFNotificationCenterAddObserver ( CFNotificationCenterGetDarwinNotifyCenter(),
                                     (__bridge const void *)(self),
                                     didReceiveWatchNotfy,
                                     CFSTR("environs.chatapp.watch.listchanged"),
                                     NULL,
                                     CFNotificationSuspensionBehaviorDrop);
}


- (void) didDeactivate
{
    [super didDeactivate];
    
    NSLog ( @"didDeactivate" );
    
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetDarwinNotifyCenter (), (__bridge const void *)(self));
    
    /*
    NSDictionary * request = @{ @"seq" : [NSNumber numberWithInt:-1] };
    
    [WKInterfaceController openParentApplication:request reply:^(NSDictionary * reply, NSError *error) {
        [self UpdateUserList:reply];
    }];
     */
}


void didReceiveWatchNotfy ( CFNotificationCenterRef center,
                           void * observer,
                           CFStringRef name,
                           const void * object,
                           CFDictionaryRef userInfo) {
    NSLog (@"Notify");
    
    if ( instance ) {
        [instance UpdateUserListNotify];
    }
}


- (void) UpdateUserListNotify
{
    /*
    NSDictionary * request = @{ @"get":@"users", @"seq" : [NSNumber numberWithInt:respSequence] };
    
    [WKInterfaceController openParentApplication:request reply:^(NSDictionary * reply, NSError *error) {
        [self UpdateUserList:reply];
    }];
     */
}


- (void) UpdateUserList:(NSDictionary *) reply
{
    if ( !reply ) {
        NSLog (@"UpdateUserList: invalid reply");
        return;
    }
    
    NSNumber * seq = reply[@"seq"];
    if ( !seq || [seq intValue] < respSequence ) {
        NSLog (@"UpdateUserList: Invalid sequence number %i -> %@", respSequence, seq );
        return;
    }
    
    NSNumber * count = reply[@"userCount"];
    if ( !count || [count intValue] <= 0 ) {
        userCount = [count intValue];
        [userList setNumberOfRows:0 withRowType:@"userItem"];
        
        NSLog (@"UpdateUserList: no users");
    }
    else {
        do
        {
            NSArray * reloads = reply[@"reloads"];
            if ( reloads ) {
                NSLog (@"UpdateUserList: reloading: %@", count);
                
                [userList setNumberOfRows:[count intValue] withRowType:@"userItem"];
                
                int rowNum = 0;
                for (int i=0; i<[count intValue]; i++ ) {
                    
                    NSString * key = [ [NSString alloc] initWithFormat:@"%inick", rowNum ];
                    NSString * userNick = reply [ key ];
                    if ( !userNick )
                        continue;
                    
                    key = [ [NSString alloc] initWithFormat:@"%itext", rowNum ];
                    NSString * userText = reply [ key ];
                    if ( !userText )
                        continue;
                    
                    UserItem * row = [userList rowControllerAtIndex:rowNum];
                    if ( !row )
                        continue;
                    
                    NSLog ( @"UpdateUserList: update userNick to [%i: %@]", rowNum, userNick );
                    [row.userNickname setText:userNick];
                    
                    NSLog ( @"UpdateUserList: update userText to [%i: %@]", rowNum, userText );
                    [row.userText setText:userText];
                    
                    
                    key = [ [NSString alloc] initWithFormat:@"%ipic", rowNum ];
                    UIImage * userPic = reply [ key ];
                    if ( userPic ) {
                        NSLog ( @"UpdateUserList: update userPicture to [%i]", rowNum );
                        [row.userPicture setImage:userPic];
                    }
                    rowNum++;
                }
                break;
            }
            
            NSArray * dels = reply[@"dels"];
            if ( dels ) {
                NSMutableIndexSet * set = [[NSMutableIndexSet alloc]init];
                
                for (NSNumber * rowNum in dels) {
                    if ( !rowNum )
                        continue;
                    
                    [set addIndex:[rowNum intValue]];
                }
                [userList removeRowsAtIndexes:set];
                break;
            }
            
            NSArray * adds = reply[@"adds"];
            if ( adds ) {
                for (NSNumber * rowNum in adds) {
                    if ( !rowNum )
                        continue;
                    
                    NSString * userNick = reply [ [ [NSString alloc] initWithFormat:@"%@nick", rowNum ] ];
                    if ( !userNick )
                        continue;
                    
                    NSString * userText = reply [ [ [NSString alloc] initWithFormat:@"%@text", rowNum ] ];
                    if ( !userText )
                        continue;
                    
                    UserItem * row = [userList rowControllerAtIndex:[rowNum intValue]];
                    if ( !row )
                        continue;
                    
                    [row.userNickname setText:userNick];
                    [row.userText setText:userText];
                }
                break;
            }
            
            NSArray * upds = reply[@"upds"];
            if ( upds ) {
                for (NSNumber * rowNum in upds) {
                    if ( !rowNum )
                        continue;
                    
                    NSString * userNick = reply [ [ [NSString alloc] initWithFormat:@"%@nick", rowNum ] ];
                    if ( !userNick )
                        continue;
                    
                    NSString * userText = reply [ [ [NSString alloc] initWithFormat:@"%@text", rowNum ] ];
                    if ( !userText )
                        continue;
                    
                    UserItem * row = [userList rowControllerAtIndex:[rowNum intValue]];
                    if ( !row )
                        continue;
                    
                    [row.userNickname setText:userNick];
                    [row.userText setText:userText];
                }
                break;
            }
        }
        while ( 0 );
        
        if ( [count intValue] != userCount ) {
            userCount = [count intValue];
            [userList setNumberOfRows:userCount withRowType:@"userItem"];
        }
    }
    
    respSequence = [seq intValue];

}

@end



