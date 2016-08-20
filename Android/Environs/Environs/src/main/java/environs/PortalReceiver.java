package environs;
/**
 * PortalReceiver
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
import java.nio.ByteBuffer;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.view.SurfaceHolder;

/**
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 * 
 * PortalReceiver class,
 */
class PortalReceiver extends Thread implements Runnable, PortalObserver
{
	private static final String className = "PortalReceiver . . . . .";

	Environs env;
	int hEnvirons = 0;
	public int nativeID;
	//int portalID = -1;
	public String key = "";
	public int errorCount;
	public final int abortAfterErrorCount = 10;

	PortalInstance portal;
	
	Decoder decoder;
	
	@Override
	public void run() {
		if (Utils.isDebug) Utils.Log ( 3, className, "Thread started");

		int receiveID = -1;

        try {
			decoder = new Decoder();

			if (portal.surface != null)
				decoder.setSurface(portal.surface);

        	Environs.BridgeForNotifier ( hEnvirons, 0, Types.NOTIFY_PORTAL_STREAM_RECEIVER_STARTED, 0 );

			//int offsetStartValue = Environs.GetBufferHeaderBytesToStartValueN ( );

        	int offset = Environs.GetBufferHeaderSizeN ( );
			int offsetData = offset + 4;
        	int offsetSize = Environs.GetBufferHeaderBytesToSizeN ( );
        	int offsetType = Environs.GetBufferHeaderBytesToTypeN ( );

			int frameCounterLast = 0;

			receiveID = Environs.AcquirePortalReceiveIDN ( portal.portalID );
        	if ( receiveID < 0 ) {
            	Utils.LogE ( className, "Failed to acquire a receiver id!" );
				if (Utils.isDebug) Utils.Log ( 3, className, "Thread terminated!" );
                return;        		
        	}

			boolean iFrameRequest = false;

        	
            // Wait for packages from portal generators
            while (env.started)
            {
            	ByteBuffer bBuffer = Environs.ReceivePortalUnitN ( receiveID );
				if (bBuffer == null ) {
            		Utils.Log1 ( className, "Received null buffer; Native decoder has been stopped!");
            		break;
            	}
            	int payloadSize = bBuffer.getInt(offsetSize);

            	if (payloadSize > 4 && payloadSize < 1500000)
            	{
					int frameCounter = bBuffer.getInt(offset);

					byte[] buffer = bBuffer.array();
					int arrayOffset = offsetData + bBuffer.arrayOffset();

					int payloadType = bBuffer.getInt(offsetType);

					payloadSize -= 4;

					//Utils.Log( className, "Received packet type " + payloadType + " of size " + payloadSize );

					// Is payload of image type?

					if ( (payloadType & Types.DATA_STREAM_VIDEO) == Types.DATA_STREAM_VIDEO )
					{
						// We should change comparison to init id instead of bitflag
						//if ( payloadType & 0xFFF == Types.DATA_STREAM_VIDEO_INIT )
						if ( (payloadType & Types.DATA_STREAM_INIT) == Types.DATA_STREAM_INIT )
						{
							if ( decoder == null ) {
								Utils.LogE ( className, "No video decoder available!");
								return;
							}

							decoder.width = bBuffer.getInt(offsetData);
							decoder.height = bBuffer.getInt(offsetData + 4);
							Utils.Log1 ( className, "Received stream video INIT. width=" + decoder.width + " height=" + decoder.height);

							portal.info.width = decoder.width;
							portal.info.height = decoder.height;
							Environs.OnPlatformNotify(hEnvirons, portal.device.nativeID, Environs.NOTIFY_PORTAL_ESTABLISHED_RESOLUTION, portal.portalID);

							if ( portal.surface != null ) {
								decoder.setSurface(portal.surface);
								if ( !decoder.initialized ) {
									Utils.LogW ( className, "Failed to initialize video decoder!" );
								}
							}
						}
						else //if ( (payloadType & Types.DATA_STREAM_H264_NALUS) == Types.DATA_STREAM_H264_NALUS )
						{
							//Utils.Log ( 6, className, "Received stream packet(S) " );
							if (decoder != null)
							{
								boolean isIFrame = ((payloadType & Types.DATA_STREAM_IFRAME) == Types.DATA_STREAM_IFRAME);
								if ( isIFrame && iFrameRequest )
									iFrameRequest = false;

								//Utils.Log ( className, "h264 stat iframe=" + isIFrame + " frameCounter=" + frameCounter + " frameCounterLast=" + frameCounterLast);

								if ( isIFrame || (frameCounter <= frameCounterLast + 1) )
								{
									try {
										if ( !decoder.initialized ) {
											if (portal.surface == null) {
												continue;
											}
											decoder.setSurface(portal.surface);
											if ( !decoder.initialized ) {
												Utils.LogW ( className, "Failed to initialize h264 decoder!" );
												continue;
											}
										}
										if ( !decoder.perform ( buffer, arrayOffset, payloadSize ) ) {
											Utils.LogE ( className, "Failed to decode NAL unit!" );
											errorCount++;
											frameCounterLast = 0;

											if (errorCount > abortAfterErrorCount) {
												Utils.LogE(className, "Too many decode errors happend. Aborting now..." );
												portal.Stop();
												break;
											}
										}
										else {
											errorCount = 0;
											frameCounterLast = frameCounter;
										}
									} catch (Exception ie) {
										ie.printStackTrace();
									}
								}
								else {
									frameCounterLast = 0;

									if ( !iFrameRequest ) {
										// Frames have been skipped. Request i-frame
										Environs.RequestPortalIntraFrameN ( Environs.CALL_WAIT, portal.portalID );
										iFrameRequest = true;
									}
								}
							}
						}
					}
					else if ( (payloadType & Types.DATA_STREAM_IMAGE) == Types.DATA_STREAM_IMAGE )
					{
						if ( (payloadType & Types.DATA_STREAM_INIT) == Types.DATA_STREAM_INIT )
						{
							int width = bBuffer.getInt(offsetData);
							int height = bBuffer.getInt(offsetData + 4);

							Utils.Log1 ( className, "Received stream image INIT. width=" + width + " height=" + height );

							portal.info.width = width;
							portal.info.height = height;
							Environs.OnPlatformNotify(hEnvirons, portal.device.nativeID, Environs.NOTIFY_PORTAL_ESTABLISHED_RESOLUTION, portal.portalID);

							if (decoder != null)
								decoder = null;
						}
						else {
							try
							{
								//Utils.Log( className, "decoding bitmap of size " + payloadSize);

								Bitmap bitmap = BitmapFactory.decodeByteArray(buffer, arrayOffset, payloadSize);

								if (bitmap != null) {
									if (!DrawBitmap(bitmap)) {
										int count = portal.observers.size();
										if (count > 0) {
											for (int i=0; i<count; i++) {
												try {
													PortalObserver observer = portal.observers.get(i);
													if (observer != this)
														observer.OnImage(bitmap);

												} catch (Exception e) {
													e.printStackTrace();
													portal.observers.remove(i);
													i--; count--;
												}
											}
										}
									}
								}
							} catch (Exception ie) {
								ie.printStackTrace();
							}
						}
					}

					//Utils.Log( className, "Received packet type " + payloadType + " of size " + payloadSize );
            	}
            }	            
		} catch (Exception e) {
            e.printStackTrace();
        }

		try {
			Environs.ReleasePortalReceiveIDN(receiveID);

			if ( decoder != null ) {
				decoder.release();
				decoder = null;
			}

			Environs.portalReceivers.remove(key);
		}
		catch (Exception e) {
			e.printStackTrace();
		}

		try {
			if (portal != null)
				portal.Dispose ();
		}
		catch (Exception e) {
			e.printStackTrace();
		}
        
        Utils.Log1 ( className, "Thread terminated!" );
	}

	@Override
	public void OnPortalChanged(PortalInstance sender, int notification) {
		if (notification == Environs.PORTAL_INSTANCE_FLAG_SURFACE_CHANGED) {

			if (portal != null && portal.surface != null && decoder != null)
				decoder.setSurface(portal.surface);
		}
	}

	@Override
	public void OnImage(Bitmap bitmap) {

	}

	private boolean DrawBitmap(Bitmap bitmap) {
		boolean success = false;

		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB)
		{
			Rect r_dest = new Rect(0, 0, Utils.width, Utils.height);

			SurfaceHolder holder = portal.surfaceHolder;
			if (holder != null) {
				final Canvas canvas = holder.lockCanvas(null);
				if ( canvas != null ) {
					try {
						synchronized (holder) {
							canvas.drawBitmap(bitmap, null, r_dest, null);
						}
						success = true;
					} catch (Exception e) {
						e.printStackTrace();
					} finally {
						if (canvas != null) {
							holder.unlockCanvasAndPost(canvas);
						}
					}
				}
			}
		}
		return success;
	}
}
