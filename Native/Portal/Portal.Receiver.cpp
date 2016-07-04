/**
 *	A portal receiver receives portal stream packages, decodes them 
    and dispatches them to the renderer
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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.h"
#include "Environs.Obj.h"
#include "Portal.Info.Base.h"
#include "Portal.Receiver.h"
#include "Portal/Portal.Device.h"
#include "Device/Device.Display.h"
#include "Environs.Av.Context.h"

// The TAG for prepending to log messages
#define CLASS_NAME	"Portal.Receiver. . . . ."


namespace environs 
{
	extern int pthread_timedjoin_env ( pthread_t threadID, unsigned int milis );


	PortalReceiver::PortalReceiver ()
	{
		CVerb ( "Construct..." );

		deviceID					= 0;
        allocated                   = false;
		decoder						= 0;
        device                      = 0;
		stream						= 0;
		enabled						= false;
		ignoreBufferstatus          = true;
		portalID					= -1;
		env							= 0;

		renderSurface				= 0;
		renderCallback				= 0;
        renderCallbackType          = 0;
        
#ifndef DISPLAYDEVICE
        touchSource                 = 0;
#endif

		ZeroStruct ( portalInfo, PortalInfoBase );

		portalInfo.orientation		= 90.0;

		pthread_reset ( threadID );
	}


	PortalReceiver::~PortalReceiver ()
	{
		CVerbID ( "Destruct..." );

		Dispose ();

		int pid = PortalIndex ();

		if ( IsValidPortalIndex ( pid ) )
			((DeviceBase *)device) ->portalReceiversDevice [pid] = -1;

        if ( allocated ) {
            allocated = false;
            
            LockDispose ( &resourcesMutex );
        }
        
		DisposePortalDevice ( portalID );
        
#ifndef DISPLAYDEVICE
        if ( touchSource ) {
            delete touchSource;
            touchSource = 0;
        }
#endif
        
		CVerbID ( "Destruct: destroyed." );
	}


	void PortalReceiver::Dispose ()
	{
		CVerbID ( "Dispose" );

        enabled			= false;
        
        Stop ();
        
		renderSurface	= 0;
		renderCallback	= 0;
        
		DisposeDecoder ();
		DisposeStream ( );
	}

	
	bool PortalReceiver::Init ( int deviceIDa, int portalIDa )
	{
		deviceID = deviceIDa;

		portalID = portalInfo.portalID = portalIDa;

		CVerbID ( "Init" );

        if ( !allocated )
        {
            if ( !LockInit ( &resourcesMutex ) )
                return false;
            allocated = true;
        }
        
		if ( !stream ) {
			if ( !CreateStream ( ) )
				return false;

			if ( !stream->Init ( deviceID ) ) {
				CErrID ( "Init: Failed to initialize stream resources." );
				return false;
			}
		}
		else {
			CVerbID ( "Init: Stream already existing." );
		}
        
		if ( !decoder ) {

#ifndef ENVIRONS_IOS
			/*if ( environs.mod_PortalDecoder )
				decoder = (IPortalDecoder *) environs::API::CreateInstance ( environs.mod_PortalDecoder, 0, deviceID );*/
#endif
			if ( !decoder && !CreateDecoder () )
				return false;

            if ( decoder ) {
                decoder->env = env;
                
				if ( !((IPortalDecoder *)decoder)->Init ( deviceID ) ) 
				{
					CErrID ( "Init: Failed to initialize decoder." );
					return false;
				}
                if ( !renderCallback )
                    renderCallback = decoder->GetRenderCallback ( renderCallbackType );
                else {
                    decoder->renderCallback = renderCallback;
                    decoder->renderCallbackType = renderCallbackType;
                }
			}
		}
		else {
			CVerbID ( "Init: Decoder already existing." );
		}
        
		DeviceBase * parentDevice = (DeviceBase *) device;

		int foundID = GetFreePortalSlot ( parentDevice, portalID );
        
        if ( foundID < 0 || foundID >= MAX_PORTAL_INSTANCES ) {
            return false;
        }
        
        PortalDevice * portal = portalDevices + foundID;
        
        portalID = portal->portalID;
        
        portal->receiver = this;
        
		CVerbArgID ( "Init: Receiver assigned to slot [%i] and portalID [0x%X].", foundID, portalID );

		//portalID = portalIDa;

		int pid = PortalIndex ();
		if ( IsInvalidPortalIndex ( pid ) )
            return false;
        
#ifndef DISPLAYDEVICE
        if ( !touchSource ) {
            touchSource = new TouchSource ( deviceID, 20 );
            if ( !touchSource ) {
                CErrID ( "Init: Failed to allocate a touch source." );
                return false;
            }
            if ( !touchSource->Init ( device, portalID ) ) {
                CErrID ( "Init: Failed to initialize a touch source." );
                return false;
            }
        }
#endif

		parentDevice->portalReceiversDevice [pid] = foundID;
        
		return true;
	}

	
	bool PortalReceiver::Start ()
	{
		CVerbID ( "Start" );

		enabled = true;

		if ( stream )
			stream->Start ( );
        
#ifndef DISPLAYDEVICE
        if ( touchSource ) {
            if ( touchSource->Start() )
                API::onEnvironsNotifier1 ( env, ((DeviceBase *)device)->deviceNode->info.objID, NOTIFY_TOUCHSOURCE_STARTED );
        }
#endif
		if ( decoder ) {
			decoder->Start ();

			if ( pthread_valid ( threadID ) ) {
				CVerbID ( "Init: Stream receiver thread already running." );
			}
			else
			{
				int ret = pthread_create ( &threadID, 0, &PortalReceiver::StartPortalReceiver, this );
				if ( ret ) {
					CErrID ( "Init: Failed to create receiver thread!" );
					enabled = false;
					return false;
				}
			}
		}

		return true;
	}

	
	bool PortalReceiver::IsActive ()
	{
		CVerbID ( "IsActive" );

		if ( enabled || pthread_valid ( threadID ) )
			return true;

		if ( stream && (stream->accessCount > 1) )
			return true;

		return false;
	}

	
	void PortalReceiver::StopNonBlock ()
	{
		CVerbID ( "StopNonBlock" );

		enabled = false;

#ifndef DISPLAYDEVICE
		if ( touchSource && touchSource->Stop () ) {
			API::onEnvironsNotifier1 ( env, ((DeviceBase *) device)->deviceNode->info.objID, NOTIFY_TOUCHSOURCE_STOPPED, portalID );
		}
#endif

		if ( stream )
			stream->StopNonBlock ( );

		if ( decoder )
			decoder->Stop ( );
	}


	void PortalReceiver::Stop ()
	{
		CVerbID ( "Stop" );
        
        enabled = false;
        
#ifndef DISPLAYDEVICE
		if ( touchSource && touchSource->Stop () ) {
			API::onEnvironsNotifier1 ( env, ((DeviceBase *) device)->deviceNode->info.objID, NOTIFY_TOUCHSOURCE_STOPPED, portalID );
		}
#endif
		if ( stream )
			stream->Stop ( );
        
		if ( decoder )
            decoder->Stop ( );
        
        pthread_t thrd = threadID;
        if ( pthread_valid ( thrd ) ) {
            
            if ( IsUIThread ( ) ) {
                CLogID ( "Stop: Main thread is calling this!!! Invalid state!!! Waiting 1000ms for decoder working thread to be terminated..." );
                pthread_timedjoin_env ( thrd, 1000 );
            }
            else {
                CLogID ( "Stop: Waiting for receiver working thread to be terminated..." );
				pthread_join ( thrd, 0 );

				pthread_detach_handle ( threadID );
            }
            
            pthread_reset ( threadID );
        }
        
		renderSurface	= 0;
	}


	bool PortalReceiver::CreateStream ()
	{
		CVerbID ( "CreateStream" );

		stream = new PortalStream ( );
		if ( !stream ) {
			CErrID ( "CreateStream: Failed to create stream." );
			return false;
		}
        
		stream->env = env;

		return true;
	}


	void PortalReceiver::DisposeStream ()
	{
		CVerbID ( "DisposeStream" );

		if ( stream ) {
			CVerbID ( "DisposeStream: Disposing stream." );
			delete stream;
			stream = 0;
		}
	}


	int PortalReceiver::CreateDecoder ( )
	{
		CVerbID ( "CreateDecoder" );

		return 0;
	}


	void PortalReceiver::DisposeDecoder ()
	{
		CVerbID ( "DisposeDecoder" );

		if ( decoder ) {
			CVerbID ( "DisposeDecoder: Disposing decoder." );
			environs::API::DisposeInstance ( decoder );
			decoder = 0;
		}
	}

	
	bool PortalReceiver::SetRenderSurface ( void * penv, void * newSurface, int width, int height )
	{
		CVerbID ( "SetRenderSurface" );

		if ( !LockAcquire ( &resourcesMutex, "SetRenderSurface" ) )
			return false;
        
		renderSurface = newSurface;

		if ( decoder ) {
			decoder->SetRenderResolution ( width, height );
			decoder->SetRenderSurface ( penv, newSurface, width, height );
        }

		if ( !LockRelease ( &resourcesMutex, "SetRenderSurface" ) )
			return false;
		return true;
	}


	bool PortalReceiver::hasSurface ()
	{
		return (renderSurface != 0);
	}

    
    bool PortalReceiver::SetRenderCallback( ptRenderCallback callback, int callbackType )
    {
        renderCallback = callback;
        renderCallbackType = callbackType;
        
        if ( decoder ) {
            decoder->renderCallback = callback;
            decoder->renderCallbackType = callbackType;
        }
        return true;
    }
    

	/**
	* Release the render surface.
	* */
	void PortalReceiver::ReleaseRenderSurface ( bool useLock )
	{
		CVerbID ( "ReleaseRenderSurface" );        

		if ( !LockAcquire ( &resourcesMutex, "ReleaseRenderSurface" ) )
			return;
        
		if ( decoder ) {
			decoder->ReleaseRenderSurface ( useLock );
		}

        renderSurface = 0;
        
		LockReleaseV ( &resourcesMutex, "ReleaseRenderSurface" );
    }
    
    
    /**
     * Release the render callback.
     * */
    void PortalReceiver::ReleaseRenderCallback ( bool useLock )
    {
        CVerbID ( "ReleaseRenderCallback" );

		if ( useLock && !LockAcquire ( &resourcesMutex, "ReleaseRenderCallback" ) )
			return;
        
        renderCallback = 0;
        
        if ( useLock ) {
			LockReleaseV ( &resourcesMutex, "ReleaseRenderCallback" );
        }
    }


	void * PortalReceiver::StartPortalReceiver ( void * arg )
	{
		return ((PortalReceiver *)arg)->Thread_Receiver ();
	}


	void * PortalReceiver::Thread_Receiver ()
	{
		CVerbID ( "Thread_Receiver: Working thread started ..." );

		pthread_setname_current_envthread ( "PortalReceiver::Thread_Receiver" );
        
        unsigned int    frameCounterLast    = 0;
        bool            iFrameRequest		= false;
		bool			isIFrame			= true;

		EnvironsAVContext avContext;
		Zero ( avContext );
        
        
		// Wait for portal packages
		while ( enabled )
		{
			ByteBuffer * bBuffer = (ByteBuffer *)stream->ReceiveStreamPack ( );

			if ( !bBuffer ) {
				CLogID ( "Receiver: Received null buffer; Stream has been canceled!" );
				break;
			}

			int payloadSize = bBuffer->payloadSize;
            
			if ( payloadSize > 4 && payloadSize < 5000000 ) // max. 5 MB
            {
                char *          buffer              = BYTEBUFFER_DATA_POINTER_START ( bBuffer );
                
                unsigned int    frameCounter        = *((unsigned int *)buffer);
                buffer += 4;                
                
                if ( pthread_mutex_lock ( &resourcesMutex ) ) {
                    CErr ( "Receiver: Failed to lock." );
                    continue;
                }
                
                payloadSize -= 4;
                int payloadType = bBuffer->type;

				if ( ( payloadType & DATA_STREAM_INIT ) == DATA_STREAM_INIT )
				{
					int width = *( ( int * ) buffer );
					int height = *( ( int * ) ( buffer + 4 ) );

					CInfoArgID ( "Receiver: Received stream INIT. width [ %i ] height [ %i ]", width, height );

					if ( width > 0 && height > 0 ) {
						if ( !decoder ) {
							CErr ( "Receiver: No decoder avaiable." );
						}
						else
						{
							portalInfo.width = width;
							portalInfo.height = height;

							environs::API::onEnvironsNotifierContext1 ( env, ( ( DeviceBase * ) device )->nativeID, NOTIFY_PORTAL_ESTABLISHED_RESOLUTION, portalID, buffer, 8 );

							decoder->SetWidthHeight ( width, width, height );

							if ( decoder->InitType ( payloadType ) ) {
								if ( !decoder->AllocateResources ( ) )
									break;

								avContext.width = decoder->width;
								avContext.stride = decoder->stride;
								avContext.height = decoder->height;
								avContext.pixelFormat = decoder->avContextSubType;

								if ( renderCallback )
									renderCallback ( RENDER_CALLBACK_TYPE_INIT, renderSurface, bBuffer );
								else {
									CVerbVerbID ( "Receiver: No rendercallback avaiable." );
								}
							}
							else {
								CErrArg ( "Receiver: Failed to initialize decoder for type [ %i ].", payloadType );
								break;
							}
						}
					}

					frameCounterLast = frameCounter;
				}
				else if ( ( payloadType & DATA_STREAM_VIDEO ) == DATA_STREAM_VIDEO ) {
					CVerbVerbArgID ( "Receiver: Received stream packet of size [ %u ]", payloadSize );

					if ( !ignoreBufferstatus ) {
						isIFrame = ( ( payloadType & DATA_STREAM_IFRAME ) == DATA_STREAM_IFRAME );

						if ( iFrameRequest && isIFrame )
							iFrameRequest = false;
					}

					if ( isIFrame || ( frameCounter <= frameCounterLast + 1 ) )
					{
						int success = decoder->Perform ( payloadType, buffer, payloadSize );
						if ( success == 1 )
						{
							if ( renderCallback ) {
								if ( decoder->avContext ) {
									CVerbVerbID ( "Receiver: Present a decoded frame." );

									int callbackType = renderCallbackType;
									if ( !callbackType ) callbackType = RENDER_CALLBACK_TYPE_DECODER;
									void * arg;
									if ( !callbackType || callbackType == RENDER_CALLBACK_TYPE_DECODER )
										arg = decoder;
									else {
										if ( callbackType != decoder->avContextType )
										{
											if ( callbackType == RENDER_CALLBACK_TYPE_AVCONTEXT ) {
												avContext.data = ( char * ) decoder->avContext;
												arg = ( void * ) &avContext;
											}
											else {
												arg = decoder->avContext;
												callbackType = decoder->avContextType;
											}
										}
										else
											arg = decoder->avContext;
									}
									renderCallback ( callbackType, renderSurface, arg );
								}
								else {
									CVerbVerbID ( "Receiver: No avContext avaiable." );
								}
							}
							else {
								CVerbVerbID ( "Receiver: No rendercallback avaiable." );
							}
							frameCounterLast = frameCounter;
						}
					}
					else {
						// Frames have been skipped. Request i-frame
						frameCounterLast = 0;

						if ( !iFrameRequest && device ) {
							( ( DeviceBase * ) device )->SendPortalMessage ( MSG_PORTAL_IFRAME_REQUEST, portalInfo.portalID );
							iFrameRequest = true;
						}
					}
                }
				else if ( ( payloadType & DATA_STREAM_IMAGE_DATA ) == DATA_STREAM_IMAGE_DATA )
				{
					if ( renderCallback )
					{
						if ( decoder->decodeImage )
						{
							if ( decoder->Perform ( payloadType, buffer, payloadSize ) )
							{
								if ( decoder->avContext ) {
									CVerbVerbID ( "Receiver: Present a decoded frame." );

									int callbackType = renderCallbackType;
									void * arg;
									if ( !callbackType || callbackType == RENDER_CALLBACK_TYPE_DECODER )
										arg = decoder;
									else {
										if ( callbackType != decoder->avContextType )
										{
											if ( callbackType == RENDER_CALLBACK_TYPE_AVCONTEXT ) {
												avContext.data = ( char * ) decoder->avContext;
												arg = ( void * ) &avContext;
											}
											else {
												arg = decoder->avContext;
												callbackType = decoder->avContextType;
											}
										}
										else
											arg = decoder->avContext;
									}

									renderCallback ( callbackType, renderSurface, arg );
								}
								else {
									CVerbVerbID ( "Receiver: No avContext avaiable." );
								}
							}
						}
						else
							renderCallback ( RENDER_CALLBACK_TYPE_IMAGE, renderSurface, bBuffer );
					}
					else {
						CVerbVerbID ( "Receiver: No rendercallback avaiable." );
					}
                }

				if ( pthread_mutex_unlock ( &resourcesMutex ) ) {
					CErr ( "Receiver: Failed to unlock." );
					continue;
				}
			}
		}

        /*
		pthread_t thrd = threadID;
		pthread_reset ( threadID );

		if ( pthread_valid ( thrd ) ) {
			pthread_detach_handle ( thrd );
		}
        */

		CLogID ( "Receiver: bye bye..." );
		return 0;
	}


} /// -> namespace environs

