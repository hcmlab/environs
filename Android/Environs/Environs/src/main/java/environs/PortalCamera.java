package environs;
/**
 * Portal camera. A platform layer portal source.
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
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.media.MediaCodecInfo;
import android.os.Build;
import java.util.List;

/**
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 * Portal camera. A platform layer portal source.
 */
@SuppressWarnings("deprecation")
@SuppressLint("NewApi")
class PortalCamera extends PortalBase
{
    private static final String className = "PortalCamera. . . . .  .";

    // Portal source
    Camera camera = null;
    boolean cameraStarted = false;

    int reqFps = 30;
    int previewColorFormat = MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar; // 19
    List<Integer> previewFormats = null;

    public int width;
    public int height;


    public PortalCamera()
    {
        if (Utils.isDebug) Utils.Log ( 4, className, "PortalGenerator" );
    }


    Camera.Parameters initCamera ( int sourceType, int reqWidth, int reqHeight )
    {
        if (Utils.isDebug) Utils.Log ( 4, className, "initCamera" );

        if (camera != null) {
            Utils.LogW(className, "initCamera: Camera already initialized");
            return camera.getParameters();
        }

        int camType = Camera.CameraInfo.CAMERA_FACING_FRONT;

        if (sourceType == PortalType.BackCam)
            camType = Camera.CameraInfo.CAMERA_FACING_BACK;

        Camera.CameraInfo info = new Camera.CameraInfo();

        int numCameras = Camera.getNumberOfCameras();

        for (int i = 0; i < numCameras; i++) {
            Camera.getCameraInfo(i, info);
            if (info.facing == camType) {
                camera = Camera.open(i);
                break;
            }
        }
        if (camera == null) {
            Utils.LogW(className, "initCamera: Requested camera not available. Initializing default camera.");
            camera = Camera.open();
        }

        if (camera == null) {
            Utils.LogE(className, "initCamera: Failed to initialize camera.");
            return null;
        }

        Camera.Parameters params = camera.getParameters();
        if (params == null)
            return null;

        adjustResolution(params, reqWidth, reqHeight);

        //params.setPreviewFrameRate(reqFps);

        if ( Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH )
            params.setRecordingHint(true);

        List<String> FocusModes = params.getSupportedFocusModes();
        if (FocusModes != null && FocusModes.contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO))
        {
            params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
        }

        // ImageFormat.NV21 // default preview format
        // ImageFormat.YV12  // Android YUV

        int pFormat = 0;

        /*if (Encoder.colorFormatsPreferred == MediaCodecInfo.CodecCapabilities.COLOR_TI_FormatYUV420PackedSemiPlanar) {

            params.setPreviewFormat(ImageFormat.NV21);

            pFormat = params.getPreviewFormat();
            if (pFormat == ImageFormat.NV21) {
                previewColorFormat = MediaCodecInfo.CodecCapabilities.COLOR_TI_FormatYUV420PackedSemiPlanar;

                Utils.Log(6, className, "initCamera: Successfully set preview format to COLOR_TI_FormatYUV420PackedSemiPlanar");
            }
            else {
                Utils.LogW(className, "initCamera: Failed to set preferred color format. Current is " + pFormat);
                pFormat = 0;
            }
        }
        */

        if (pFormat == 0) {
            // Try setting YUV
            params.setPreviewFormat(ImageFormat.YV12);

            pFormat = params.getPreviewFormat();
            if (pFormat == ImageFormat.YV12) {
                previewColorFormat = MediaCodecInfo.CodecCapabilities.COLOR_FormatYCrYCb;

                if (Utils.isDebug) Utils.Log ( 6, className, "initCamera: Successfully set preview format to YV12 (Android YUV)");
            }
            else {
                params.setPreviewFormat(ImageFormat.NV21);

                pFormat = params.getPreviewFormat();
                if (pFormat == ImageFormat.NV21) {
                    previewColorFormat = MediaCodecInfo.CodecCapabilities.COLOR_TI_FormatYUV420PackedSemiPlanar;

                    if (Utils.isDebug) Utils.Log ( 6, className, "initCamera: Successfully set preview format to TI YUV (NV21)");
                }
                else {
                    Utils.LogW(className, "initCamera: Failed to set preferred color format. Current is " + pFormat);
                }
            }

        }

        camera.setParameters(params);
        return params;
    }


    void adjustResolution (Camera.Parameters params, int reqWidth, int reqHeight)
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "adjustResolution" );

        if (reqWidth == 0 || reqHeight == 0) {
            for (Camera.Size size : params.getSupportedPreviewSizes()) {
                if (size.width > reqWidth || size.height == reqHeight) {
                    reqWidth = size.width;
                    reqHeight = size.height;
                }
            }
            params.setPreviewSize ( reqWidth, reqHeight );
            if (Utils.isDebug) Utils.Log ( 5, className, "adjustResolution: width [" + reqWidth + "] height [" + reqHeight + "]");
            width = reqWidth; height = reqHeight;
            return;
        }


        for (Camera.Size size : params.getSupportedPreviewSizes()) {
            if (size.width == reqWidth && size.height == reqHeight) {
                params.setPreviewSize ( reqWidth, reqHeight );
                if (Utils.isDebug) Utils.Log ( 5, className, "adjustResolution: width [" + reqWidth + "] height [" + reqHeight + "]");
                width = reqWidth; height = reqHeight;
                return;
            }
        }

        Camera.Size preferred = params.getPreferredPreviewSizeForVideo();
        params.setPreviewSize(preferred.width, preferred.height);
        width = preferred.width; height = preferred.height;
        if (Utils.isDebug) Utils.Log ( 3, className, "adjustResolution: set to preferred width [" + width + "] height [" + height + "]");
    }


    public void start ()  {
        if (Utils.isDebug) Utils.Log ( 7, className, "Start" );

        synchronized (this) {
            if ( cameraStarted )
                return;

            if ( camera == null )
                return;

            if (Utils.isDebug) Utils.Log ( 5, className, "Start: Camera");
            try {
                camera.startPreview();
                cameraStarted = true;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }


    public void stop () {
        if (Utils.isDebug) Utils.Log ( 7, className, "Stop" );

        synchronized (this) {
            if ( !cameraStarted )
                return;

            if (camera == null)
                return;

            if (Utils.isDebug) Utils.Log ( 5, className, "Stop: Camera" );

            try {
                camera.stopPreview();
                camera.setPreviewCallback(null);

                cameraStarted = false;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }


    public void release ()
    {
        if (Utils.isDebug) Utils.Log ( 7, className, "release");

        if (camera == null)
            return;

        if (Utils.isDebug) Utils.Log ( 5, className, "release: Camera");

        try {
            camera.release();
            camera = null;
            cameraStarted = false;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    boolean initPortal () throws Exception {
        throw new NoSuchMethodException();
    }


    boolean completeCameraInit() throws Exception {
        throw new NoSuchMethodException();
    }
}
