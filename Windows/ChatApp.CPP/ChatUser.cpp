/**
 * ChatUser
 * ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
 * @remarks
 *
 * This file is part of the Environs framework developed at the
 * Lab for Human Centered Multimedia of the University of Augsburg.
 * http://hcm-lab.de/environs
 *
 * Environ is free software; you can redistribute it and/or modify
 * it under the terms of the Eclipse Public License v1.0.
 * A copy of the license may be obtained at:
 * http://www.eclipse.org/org/documents/epl-v10.html
 * --------------------------------------------------------------------
 */
#include <stdafx.h>

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
#   define DEBUGVERB
# define DEBUGVERBVerb
#endif

#include "ChatUser.h"
#include "ChatApp.h"
#include "Observer.h"
#include "Environs.h"
#include "Environs.Native.h"

using namespace std;
using namespace environs;

ChatUser    * chatUser;

#define CLASS_NAME  "ChatUser . . . . . . . ."

extern string loginUserName;

//
string   statusMessage = "Hi, there!";
//extern NSString            *   userImageBase64;
//extern NSString            *   defaultUserImage;

extern sp ( Observer )			obs;


ChatUser::ChatUser ()
{
	chatUser = this;

	profilePic = 0;
	userName = "Unknown";

	messages.clear ();

	lastStatusRequested = false;
	profilePicRequested = false;
	userNameRequested = false;

	LockInit ( &messagesLock );
}

ChatUser::~ChatUser ()
{
	LockDispose ( &messagesLock );
}


bool ChatUser::IsChatCommand ( const string & msg )
{
    if ( msg.length () < 5 )
        return false;
    
    return msg.find_first_of("$ca$") == 0;
}


void * InitUser ( void * chat )
{
	((ChatUser *)chat)->Init ();
	return 0;
}


bool ChatUser::InitWithDevice ( const sp ( DeviceInstance ) & userDevice )
{
    if ( !userDevice )
        return false;
    
	// Create a ChatUser object and attach it as the appContext to the DeviceInstance
    ChatUser * chat = new ChatUser ();
    if ( !chat )
        return false;
    
    chat->device = userDevice;

	pthread_t threadID;
	pthread_create ( &threadID, NULL, &InitUser, (void *)chat );
    
    return true;
}


bool ChatUser::Init ()
{
	LockAcquire ( &messagesLock, "Init" );
    
    device->appContext1 = this;
    
    // Let's observe device changes, e.g. disposal, connections, etc..
    device->AddObserver ( obs.get () );
    
    // Let's listen to messages from this device
    device->AddObserverForMessages ( obs.get () );
    
    // Get or Request Username
    // Get or Request profile image
    // All chat internal messages start with $ca$type$data
    
    sp ( MessageList ) msgs = device->GetMessagesInStorage ();
    if ( msgs && msgs->size () > 0 )
    {
        try
        {
            for ( int i=(int)msgs->size ()-1; i>=0; i-- )
            {                
                sp ( MessageInstance ) msgInst = msgs->at(i);
                if ( !msgInst )
                    continue;
                
                if ( msgInst->text() == 0 )
                    continue;
                
                if ( !IsChatCommand ( msgInst->text ()) ) {
                    if ( lastMessage.empty () && !msgInst->sent() ) {
                        lastMessage = msgInst->text ();
                    }
                    
                    messages.insert ( messages.begin(), msgInst );
                    continue;
                }
                if ( msgInst->sent() )
                    continue;
                
                HandleChatCommand ( msgInst->text (), false );
            }
        }
        catch(...)
        {            
        }
    }

	LockRelease ( &messagesLock, "Init" );

    device->NotifyAppContextChanged ( 0 );

	UpdateMessageList ( device );

	/// Ask user whether he/she is ready for commands
	device->SendMessage ( "$ca$8" );
    
	RequestProfile ();
    return true;
}


void ChatUser::RequestProfile ()
{
	LockAcquire ( &messagesLock, "RequestProfile" );
	
	if ( !userName.compare("Unknown") && !userNameRequested ) {
		// Username not found. Let's request that.
		CVerbN ( "Init: Requesting username" );
		device->SendMessage ( "$ca$0" );
	}

	if ( profilePic == 0 && !profilePicRequested ) {
		CVerbN ( "Init: Requesting profilePic" );
		device->SendMessage ( "$ca$2" );
	}

	if ( lastStatus.empty () && !lastStatusRequested ) {
		CVerbN ( "Init: Requesting status" );
		// status message not found. Let's request that.
		device->SendMessage ( "$ca$4" );
	}

	LockRelease ( &messagesLock, "RequestProfile" );
}


bool ChatUser::HandleChatCommand ( const string & msg, bool processIncoming )
{
	bool success = true;

	const char * str = msg.c_str ();

    char type = str[4];
	
	if ( type  == '1' ) // Update the username if required
	{
		CVerbArgN ( "HandleChatCommand: incoming username [%s]", msg.c_str () );
		if ( userName.empty () || processIncoming )
			userName = str + 6;
	}
	else if ( type == '3' ) // Update the profile picture if required
	{
		CVerbN ( "HandleChatCommand: incoming picture" );
		if ( profilePic == 0 || processIncoming ) {
			//         NSString * profilePicString = [msg substringFromIndex:6];
			//         
			//// Decode the base64 sting to a byte array
			//         NSData * profilePicData = [ChatUser FromBase64:profilePicString];
			//         if ( profilePicData ) {
			//	// Let's create an image from our byte array
			//             profilePic = [[NSImage alloc] initWithData:profilePicData];
			//         }
		}
	}
	else if ( type == '5' ) // Update the status message if required
	{
		CVerbArgN ( "HandleChatCommand: incoming status [%s]", msg.c_str () );
		if ( lastStatus.empty () || processIncoming )
			lastStatus = str + 6;
	}
	else if ( processIncoming ) // If this is an incoming request, then reply accordingly
	{ 
		if ( type == '0' ) // Resonse with our username
		{
			CVerbArgN ( "HandleChatCommand: sending username [%s]", msg.c_str () );

			string smsg = string ( "$ca$1$" ) + loginUserName;

			device->SendMessage ( smsg.c_str () );
		}
		else if ( type == '2' ) // Resonse with a base64 encoded string of our profile picture
		{
			CVerbN ( "HandleChatCommand: sending picture" );

			/*NSString * msgtext = [NSString stringWithFormat:@"$ca$3$%@", userImageBase64];

			device->SendMessage ( [msgtext UTF8String], (int) [msgtext length] );*/
		}
		else if ( type == '4' ) // Resonse with a base64 encoded string of our profile picture
		{
			CVerbArgN ( "HandleChatCommand: sending status [%s]", msg.c_str () );

			string smsg = string ( "$ca$5$" ) + statusMessage;

			device->SendMessage ( smsg.c_str () );
		}
		else if ( type == '8' ) // Resonse with a base64 encoded string of our profile picture
		{
			CVerbN ( "HandleChatCommand: Sending ready." );

			RequestProfile ();

			userNameRequested = true;
			profilePicRequested = true;
			lastStatusRequested = true;
		}
		else success = false;
    }
	else success = false;
    return success;
}


const string & ChatUser::GetProfileText ()
{
	profileText = userName + "(" + lastStatus + ")\n" + lastMessage;
	return profileText;
}


char * ChatUser::GetProfileImage()
{
    return profilePic;
}


/**
* OnMessage is called whenever a text message has been received from a device.
*
* @param msg			The corresponding message object of type MessageInstance
* @param changedFlags	Flags that indicate the object change.
*/
void Observer::OnMessage ( const sp ( MessageInstance ) &msg, MessageInfoFlag_t changedFlags )
{
    CVerbArgN ( "OnMessage: %s", msg->ShortText () );
    
    ChatUser * chat = chatUser;
    
    if ( !chat ) return;
    
	LockAcquire ( &chat->messagesLock, "OnMessage" );
    
    if ( ChatUser::IsChatCommand ( msg->text () ) ) {
        if ( !msg->sent() && chat->HandleChatCommand ( msg->text (), true ) ) {
            chat->device->NotifyAppContextChanged ( 0 );
        }
    }
    else {
        chat->messages.push_back(msg);
        if ( !msg->sent() ) {
            chat->lastMessage = msg->text ();
            chat->device->NotifyAppContextChanged ( 0 );
        }
        
		UpdateMessageList ( chat->device );
    }
	
	LockRelease ( &chat->messagesLock, "OnMessage" );
}


const char * ChatUser::ToBase64 ( void * rawData )
{
	return 0;
}


const char * ChatUser::FromBase64 ( const char * base64 )
{
	return 0;

}
