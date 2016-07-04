/**
 * Callback handlers
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
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_CALLBACKS_H
#define INCLUDE_HCM_ENVIRONS_CALLBACKS_H

#include "Human.Input.Decl.h"
#include "Environs.Native.h"
#include "Environs.Msg.Types.h"

#ifdef _WIN32
#define CallBackConv __cdecl
//#define CallBackConv __stdcall 
#else
#define CallBackConv
#endif

typedef void (CallBackConv * InputCallbackType) (int hInst, int nativeID, environs::Input * input);
typedef void (CallBackConv * InputDataCallbackType) (int hInst, OBJIDType objID, environs::lib::SensorFrame * input, int packSize);

typedef void (CallBackConv * NotificationCallbackType) (int hInst, OBJIDType objID, int notification, int sourceIdent, void * contextPtr, int context);
typedef void (CallBackConv * NotificationExtCallbackType) (int hInst, int deviceID, const char * areaName, const char * appName, int notification, int sourceIdent, void * contextPtr);

typedef void (CallBackConv * MessageCallbackType) (int hInst, OBJIDType objID, int sourceIdent, const void * msg, int length);
typedef void (CallBackConv * MessageExtCallbackType) (int hInst, int deviceID, const char * areaName, const char * appName, int sourceIdent, const void * msg, int length);

typedef void (CallBackConv * DataCallbackType) (int hInst, OBJIDType objID, int nativeID, int type, int fileID, const char * fileDescriptor, int size);

typedef void (CallBackConv * StatusMessageCallbackType) (int hInst, const char * msg);

#define onEnvironsSensor(hEnv,objID,pack,packSize)		hEnv->callbacks.OnDataInput ( hEnv->hEnvirons, objID, pack, packSize )

#ifdef __cplusplus

namespace environs
{
	class Instance;
#endif

	typedef struct ICallbacks
	{
#ifdef __cplusplus
		Instance				*	env;
#endif

		InputCallbackType			OnHumanInput;
		InputCallbackType			OnInputDelegate;

		InputDataCallbackType       OnDataInput;

		bool                        doOnStatusMessage;
		StatusMessageCallbackType	OnStatusMessage;

		NotificationCallbackType    OnNotify;
		MessageCallbackType         OnMessage;

		NotificationExtCallbackType	OnNotifyExt;
		MessageExtCallbackType      OnMessageExt;

		DataCallbackType			OnData;

		ICallbacks ();
		~ICallbacks ();

		bool Clear ();

#ifdef __cplusplus
		bool Init ( Instance * obj );
#endif

	} ICallbacks;

	// Place an external symbol into all files that want to call to managed layer
	//extern ICallbacks Callbacks;
#ifdef __cplusplus
}
#endif


#endif	/// INCLUDE_HCM_ENVIRONS_CALLBACKS_H
