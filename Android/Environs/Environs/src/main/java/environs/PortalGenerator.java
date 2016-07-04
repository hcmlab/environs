package environs;
/**
 * PortalGenerator class using java API.
 * A platform layer portal generator (capture/encoder).
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
import android.annotation.SuppressLint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Build;

import java.io.IOException;
import java.nio.ByteBuffer;


/**
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 * PortalGenerator class using java API
 */
@SuppressWarnings("deprecation")
@SuppressLint("NewApi")
public class PortalGenerator extends PortalCamera implements Camera.PreviewCallback, Encoder.OnOutputListener
{
    private static final String className = "PortalGenerator. . . . .";

    public int destID = 0;
    public int portalID = 0;
    public String areaName = null;
    public String appName = null;
    public String key = "";
    public int sendID = -1;
    Environs env;
    int hEnvirons = 0;

    SurfaceTexture previewTexture = null;

    // Determines whether the actual implementation (or subclasses) rely on an input surface
    boolean useInputSurface = false;

    boolean encoderStarted = false;
    boolean runWorker = false;
    boolean encoderInputAvail = false;
    private boolean cameraTriggered = false;

    private long lastMillis = 0;
    Thread workerThread = null;

    // Encoder source
    MediaCodec.BufferInfo bufferInfo = null;
    Encoder encoder = null;
    static final String mimeType = "video/avc";

    static int reqBitRate = 5000000;


    public PortalGenerator()
    {
        if (Utils.isDebug) Utils.Log ( 4, className, "PortalGenerator" );
    }


    public boolean init ( int sourceType, int reqWidth, int reqHeight )
    {
        try
        {
            if (Utils.isDebug) Utils.Log ( 4, className, "Init");

            do {
                if ( !useInputSurface ) {
                    if ( Encoder.colorFormatsPreferred < 0 ) {
                        if (!Encoder.findSupportedColorFormat (reqWidth, reqHeight, reqBitRate, reqFps) )
                            break;
                    }
                }

                sendID = Environs.AcquirePortalSendIDN ( Environs.DATA_STREAM_H264_NALUS, portalID );
                if ( sendID < 0 ) {
                    Utils.LogE(className, "Init: Failed to acquire a send ID from native layer.");
                    break;
                }

                Camera.Parameters params = initCamera(sourceType, reqWidth, reqHeight);
                if (params == null)
                    break;

                Camera.Size size = params.getPreviewSize();

                if (!initEncoder(size.width, size.height))
                    break;
                encoderStarted = true;

                if (!completeCameraInit())
                    break;

                try {
                    if (!initPortal())
                        break;
                }
                catch (IOException ex) {
                    Utils.LogE(className, "Init: Failed to setPreviewDisplay [" + ex.getMessage() + "]");
                    break;
                }

                if (Utils.isDebug) Utils.Log ( 4, className, "Init: ok");
                return true;
            }
            while ( false );

            release ( );

            if (Utils.isDebug) Utils.Log ( 4, className, "Init: failed");
        }
        catch (Exception ex) {
            Utils.LogE(className, "Init: Failed [" + ex.getMessage() + "]");
            ex.printStackTrace();
        }
        return false;
    }


    boolean completeCameraInit() throws Exception
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB)
        {
            previewTexture = new SurfaceTexture(10);

            camera.setPreviewTexture(previewTexture);
        } else
            camera.setPreviewDisplay(null);

        return true;
    }


    byte [] yuvBuffer = null;

    boolean initPortal () throws Exception
    {
        int reqBuffSize = width * height;

        reqBuffSize += reqBuffSize >> 1;

        camera.addCallbackBuffer(new byte[reqBuffSize]);

        camera.setPreviewCallbackWithBuffer(this);

        yuvBuffer = new byte[reqBuffSize];

        //SendBuffer = ByteBuffer.allocateDirect(reqBuffSize);

        return true;
    }


    boolean initEncoder ( int reqWidth, int reqHeight )
    {
        if (Utils.isDebug) Utils.Log ( 4, className, "initEncoder");

        bufferInfo = new MediaCodec.BufferInfo();

        MediaFormat format = MediaFormat.createVideoFormat(mimeType, reqWidth, reqHeight);

        format.setInteger(MediaFormat.KEY_COLOR_FORMAT,
                useInputSurface ?
                        MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface :
                        Encoder.colorFormatsPreferred);

        format.setInteger(MediaFormat.KEY_BIT_RATE, reqBitRate);

        format.setInteger(MediaFormat.KEY_FRAME_RATE, reqFps);
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 5);

        Utils.Log1 ( className, "initEncoder: Encoder format " + format);

        try {
            encoder = new Encoder();
            encoder.useInputSurface = useInputSurface;
            encoder.performOutput = false;
            encoder.setFormat(format);

            if ( !encoder.init() )
                return false;

            encoder.setOutputListener(this);

        } catch (Exception e) {
            e.printStackTrace();
        }

        if (Utils.isDebug) Utils.Log ( 4, className, "initEncoder: ok");
        return true;
    }


    // SPS and PPS NALs (Config frame) for H.264.
    private ByteBuffer spsPps = null;
    private int spsPpsSize = 0;

    public void onEncoderOutput ( ByteBuffer buffer, MediaCodec.BufferInfo info )
    {
        if (Utils.isDebug) Utils.Log ( 8, className, "onEncoderOutput: Cap: " + buffer.capacity() + " size: " + info.size + " offset: " + info.offset );

        if (info.size <= 0)
            return;

        int flags = 0;

        if ((info.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
            if (Utils.isDebug) Utils.Log ( 5, className, "onEncoderOutput: Sps Pps data offset [" + info.offset + "] of size [" + info.size + "]");

            spsPps = ByteBuffer.allocateDirect(info.size);

            buffer.position(info.offset);
            buffer.limit(info.offset + info.size);
            spsPps.put(buffer);
            buffer.position(0);
            spsPpsSize = info.size;
            return;
        }

        if ((info.flags & MediaCodec.BUFFER_FLAG_KEY_FRAME) != 0)
            flags = Environs.DATA_STREAM_IFRAME;

        Environs.SendTcpPortalN ( sendID, flags, spsPps, spsPpsSize, buffer, info.offset, info.size );
    }


    public void start ()
    {
        if (Utils.isDebug) Utils.Log ( 4, className, "Start");

        synchronized (this) {
            if ( camera == null )
                return;

            runWorker = true;

            if (workerThread == null) {
                if (Utils.isDebug) Utils.Log ( 5, className, "Start: Created worker thread." );

                workerThread = new Thread(new PortalWorker());
                workerThread.start();
            }
        }

        super.start();
    }


    //
    // YV12 is a 4:2:0 YCrCb planar format
    // http://developer.android.com/reference/android/graphics/ImageFormat.html#YV12
    //
    public static void ConvertYV12toYUV420Planes(byte[] YV12, byte[] YUV, int width, int height) throws Exception
    {
        int planeSize = width * height;
        int planeSizeCx = planeSize >> 2;

        System.arraycopy(YV12, 0, YUV, 0, planeSize);
        System.arraycopy(YV12, planeSize, YUV, planeSize + planeSizeCx, planeSizeCx);
        System.arraycopy(YV12, planeSize + planeSizeCx, YUV, planeSize, planeSizeCx);
    }

    public static void ConvertYV12toYUV420PackedSemiPlanar(byte[] YV12, byte[] YUV, int width, int height) throws  Exception
    {
        int planeSize = width * height;
        int planeSizeCx = planeSize >> 2;

        System.arraycopy(YV12, 0, YUV, 0, planeSize);

        for (int i = 0; i < planeSizeCx; i++) {
            YUV[planeSize + i*2] = YV12[planeSize + i + planeSizeCx];
            YUV[planeSize + i*2 + 1] = YV12[planeSize + i];
        }
    }

    int maxCount = 0;

    @Override
    public void onPreviewFrame(byte[] data, Camera cam) {
        long curMillis = System.currentTimeMillis();
        if (Utils.isDebug) Utils.Log ( 8, className, "onPreviewFrame: millis [" + (curMillis - lastMillis) + "]");

        lastMillis = curMillis;

        if (encoder == null) {
            return;
        }

        if ( yuvBuffer.length < data.length ) {
            yuvBuffer = new byte[data.length];
        }

        if (previewColorFormat == Encoder.colorFormatsPreferred) {
            encoder.perform(data, 0, data.length);

            camera.addCallbackBuffer(data);

        }
        else if (previewColorFormat == MediaCodecInfo.CodecCapabilities.COLOR_FormatYCrYCb)
        {
            try {
                if (Encoder.colorFormatsPreferred == MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar)
                    ConvertYV12toYUV420Planes(data, yuvBuffer, width, height);
                else
                    ConvertYV12toYUV420PackedSemiPlanar(data, yuvBuffer, width, height);
            } catch (Exception e) {
                e.printStackTrace();
                return;
            }
            finally {
                camera.addCallbackBuffer(data);
            }

            encoder.perform(yuvBuffer, 0, yuvBuffer.length);
        }
        else {
            // We need a conversion from N21 to YUV or something else to YUV
            // Currently we just feed the data into the encoder
            // and let it crash ...
            encoder.perform(data, 0, data.length);

            camera.addCallbackBuffer(data);
        }

        synchronized (this) {
            cameraTriggered = true;
            this.notify();
        }
        return;
    }


    private class PortalWorker implements Runnable {
        @Override
        public void run() {
            if (Utils.isDebug) Utils.Log ( 5, className, "PortalWorker: started" );

            do {
                synchronized (PortalGenerator.this) {
                    if (!cameraTriggered) {
                        try {
                            PortalGenerator.this.wait();
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        cameraTriggered = false;
                    }
                }
                if (!runWorker)
                    break;

                encoder.getNextOutput();
            }
            while (runWorker);

            if (Utils.isDebug) Utils.Log ( 5, className, "PortalWorker: terminated." );
        }
    }


    public void stop () {
        if (Utils.isDebug) Utils.Log ( 7, className, "Stop" );

        runWorker = false;

        super.stop();

        synchronized (this) {
            this.notify();
        }

        if (workerThread != null) {
            if (Utils.isDebug) Utils.Log ( 5, className, "Stop: Waiting for worker thread ...");
            try {
                workerThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            workerThread = null;
        }

        synchronized (this) {
            if (encoder != null && encoderStarted)
            {
                try {
                    if (useInputSurface && encoder.CoDec != null) {
                        if (Utils.isDebug) Utils.Log ( 5, className, "Stop: Signal end of stream.");

                        encoder.CoDec.signalEndOfInputStream();
                    }

                    if (Utils.isDebug) Utils.Log ( 5, className, "Stop: Encoder.");
                    encoder.stop();
                } catch (Exception e) {
                    e.printStackTrace();
                }

                encoderStarted = false;
            }
        }
    }


    public void release () {
        if (Utils.isDebug) Utils.Log ( 4, className, "release");

        stop();

        super.release();

        synchronized (this) {
            if (encoder != null) {
                if (Utils.isDebug) Utils.Log ( 5, className, "release: Encoder.");
                try {
                    encoder.release();
                } catch (Exception e) {
                    e.printStackTrace();
                }
                encoder = null;
            }
        }

        try {
            if (sendID >= 0)
                Environs.ReleasePortalSendIDN(sendID);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
