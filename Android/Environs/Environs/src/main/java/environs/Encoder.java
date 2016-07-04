package environs;
/**
 * Android Java Encoder for images and h264 streams
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
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;

import java.nio.ByteBuffer;
import java.util.ArrayList;

@SuppressWarnings ( { "deprecation", "unused" } )
@SuppressLint("NewApi")
public class Encoder extends Codec
{
    private static final String className = "Encoder. . . . . . . . .";

    public boolean encodeToH264 = false;
    boolean encodeToJPEG = true;
    public boolean useInputSurface = false;

    Encoder() {
        Utils.Log2 ( className, "Construct..." );
        API21 = (android.os.Build.VERSION.SDK_INT > 20);

        isEncoder = true;
    }


    boolean initCoDec()
    {
        return false;
    }


    MediaCodec getCoDec () throws Exception
    {
        return MediaCodec.createEncoderByType(mimeType);
    }


    public interface OnOutputListener {
        void onEncoderOutput(ByteBuffer buffer, MediaCodec.BufferInfo info);
    }

    OnOutputListener    outputListener;

    public void setOutputListener(OnOutputListener listener) {
        outputListener = listener;
    }


    @SuppressWarnings ( "unused" )
    void setConfigureFormat (MediaFormat format) throws Exception
    {
        this.format = format;
        CoDec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
    }


    MediaFormat configureFormat (MediaFormat format) throws Exception
    {
        //format.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 0);
        if (useInputSurface)
            format.setInteger( MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface );
        else
            format.setInteger( MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar );
        //format.setInteger( MediaFormat.KEY_I_FRAME_INTERVAL, 5 );
        format.setInteger( MediaFormat.KEY_BIT_RATE, 125000 );
        format.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
        return format;
    }


    @SuppressWarnings ( "all" )
    boolean onPreStart (MediaCodec codec)
    {
        if (Utils.isDebug) Utils.Log ( 6, className, "onPreStart" );

        if (useInputSurface) {
            try
            {
                if (Utils.isDebug) Utils.Log ( 4, className, "onPreStart: creating input surface" );

                surface = codec.createInputSurface();
                if ( surface == null ) {
                    Utils.LogE(className, "PortalInputSurface: Failed to create an input surface.");
                    return false;
                }
            }
            catch (Exception ex) {
                Utils.LogE(className, "onPreStart: " + ex.getMessage() );
                return false;
            }
        }
        return true;
    }

    public void release()
    {
        super.release();
    }


    void outputCallback ( int outputStatus )
    {
        if (Utils.isDebug) Utils.Log ( 8, className, "outputCallback");

        ByteBuffer outputBuffer;

        if ( API21 )
            outputBuffer = CoDec.getOutputBuffer(outputStatus);
        else
            outputBuffer = outputBuffers [ outputStatus ];

        /// Send data
        if (outputListener != null)
            outputListener.onEncoderOutput(outputBuffer, bufferInfo);

        //Utils.Log ( 6, className, "outputCallback: Data size=" + outputBuffer.capacity());

        CoDec.releaseOutputBuffer ( outputStatus, false );

    }



    boolean getNextOutput ()
    {
        if (Utils.isDebug) Utils.Log ( 8, className, "getNextOutput" );

        MediaCodec codec = CoDec;
        if (codec == null) {
            Utils.LogW ( className, "getNextOutput: No codec available!");
            return false;
        }

        int outputStatus = 0;

        while (outputStatus >= 0)
        {
            outputStatus = codec.dequeueOutputBuffer ( bufferInfo, timeoutUs );
            if ( outputStatus >= 0 )
            {
                outputCallback ( outputStatus );
            }
            else if ( outputStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED )
            {
                Utils.Log2 ( className, "getNextOutput: Output format changed!" );

                format = codec.getOutputFormat ();
            }
            else if ( outputStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED )
            {
                Utils.Log2 ( className, "getNextOutput: Output buffers changed!" );

                if ( !API21 )
                    outputBuffers = codec.getOutputBuffers ( );
            }
            else
                break;
        }

        if (Utils.isDebug) Utils.Log ( 8, className, "getNextOutput: ok" );
        return true;
    }

    /*
    static int []formats = {
            MediaCodecInfo.CodecCapabilities.COLOR_Format12bitRGB444,
            MediaCodecInfo.CodecCapabilities.COLOR_Format16bitARGB1555,
            MediaCodecInfo.CodecCapabilities.COLOR_Format16bitARGB4444,
            MediaCodecInfo.CodecCapabilities.COLOR_Format16bitBGR565,
            MediaCodecInfo.CodecCapabilities.COLOR_Format16bitRGB565,
            MediaCodecInfo.CodecCapabilities.COLOR_Format18BitBGR666,
            MediaCodecInfo.CodecCapabilities.COLOR_Format18bitARGB1665,
            MediaCodecInfo.CodecCapabilities.COLOR_Format18bitRGB666,
            MediaCodecInfo.CodecCapabilities.COLOR_Format19bitARGB1666,
            MediaCodecInfo.CodecCapabilities.COLOR_Format24BitABGR6666,
            MediaCodecInfo.CodecCapabilities.COLOR_Format24BitARGB6666,
            MediaCodecInfo.CodecCapabilities.COLOR_Format24bitARGB1887,
            MediaCodecInfo.CodecCapabilities.COLOR_Format24bitBGR888,
            MediaCodecInfo.CodecCapabilities.COLOR_Format24bitRGB888,
            MediaCodecInfo.CodecCapabilities.COLOR_Format25bitARGB1888,
            MediaCodecInfo.CodecCapabilities.COLOR_Format32bitARGB8888,
            MediaCodecInfo.CodecCapabilities.COLOR_Format32bitBGRA8888,
            MediaCodecInfo.CodecCapabilities.COLOR_Format8bitRGB332,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatCbYCrY,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatCrYCbY,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatL16,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatL2,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatL24,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatL32,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatL4,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatL8,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatMonochrome,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatRawBayer10bit,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatRawBayer8bit,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatRawBayer8bitcompressed,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYCbYCr,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYCrYCb,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV411PackedPlanar ,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV411Planar,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedPlanar,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedSemiPlanar,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV422PackedPlanar,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV422PackedSemiPlanar,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV422Planar,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV422SemiPlanar,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV444Interleaved,
            MediaCodecInfo.CodecCapabilities.COLOR_QCOM_FormatYUV420SemiPlanar,
            MediaCodecInfo.CodecCapabilities.COLOR_TI_FormatYUV420PackedSemiPlanar,
    };

    static int testEncoderColorFormat ()
    {
        MediaFormat format = MediaFormat.createVideoFormat(mimeType, 320, 480);

        format.setInteger(MediaFormat.KEY_BIT_RATE, 5000000);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 5);

        MediaCodec codec = null;
        for (int i=0; i<formats.length; i++) {

            try {
                codec = MediaCodec.createEncoderByType(mimeType);

                format.setInteger(MediaFormat.KEY_COLOR_FORMAT, formats[i]);
                codec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

                codec = null;
            } catch (Exception e) {
                Utils.Log(5, className, "testEncoderColorFormat: " + formats[i] +  " not supported.");
                codec = null;
                e.printStackTrace();

                continue;
            }
            codec = null;

            Utils.Log(5, className, "testEncoderColorFormat: " + formats[i] +  " SUPPORTED!");
        }

        return 0;
    }
    */

    @SuppressWarnings ( "all" )
    static ArrayList<Integer> colorFormatsSupported = new ArrayList<Integer>();
    static int colorFormatsPreferred = -1;

    static boolean findSupportedColorFormat  ( int reqWidth, int reqHeight, int reqBitRate, int reqFps )
    {
        Utils.CheckForCodecs();

        if ( colorFormatsPreferred >= 0 )
            return true;

        colorFormatsPreferred = Encoder.lookUpEncoderColorFormat ( reqWidth, reqHeight, reqBitRate, reqFps );

        if ( colorFormatsPreferred < 0 ) {
            Utils.LogE(className, "findSupportedColorFormat: Failed to find a matching color format for encoder.");
            return false;
        }
        return true;
    }


    static int lookUpEncoderColorFormat ( int reqWidth, int reqHeight, int reqBitRate, int reqFps )
    {
        //testEncoderColorFormat ();

        MediaFormat format = null;

        /// Check whether YV12 is supported
        try
        {
            //format = MediaFormat.createVideoFormat(mimeType, reqWidth, reqHeight);
            format = MediaFormat.createVideoFormat(mimeType, 320, 480);

            format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar);
            format.setInteger(MediaFormat.KEY_BIT_RATE, reqBitRate);
            format.setInteger(MediaFormat.KEY_FRAME_RATE, reqFps);
            format.setInteger ( MediaFormat.KEY_I_FRAME_INTERVAL, 5 );

            MediaCodec codec = MediaCodec.createEncoderByType(mimeType);
            codec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

            return MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar;
        }
        catch(Exception ex) {
            if (Utils.isDebug) Utils.Log ( 5, className, "lookUpEncoderColorFormat: COLOR_FormatYUV420Planar not supported." );
        }

        if ( format == null )
            return -1;

        /// Check whether NV21 is supported
        try
        {
            format.setInteger ( MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar );

            MediaCodec codec = MediaCodec.createEncoderByType(mimeType);
            codec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

            return MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar;
        }
        catch(Exception ex) {
            if (Utils.isDebug) Utils.Log ( 5, className, "lookUpEncoderColorFormat: COLOR_FormatYUV420SemiPlanar not supported." );
        }


        /// Check whether TI YUV semiplanar is supported
        try
        {
            format.setInteger ( MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_TI_FormatYUV420PackedSemiPlanar );

            MediaCodec codec = MediaCodec.createEncoderByType(mimeType);
            codec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

            return MediaCodecInfo.CodecCapabilities.COLOR_TI_FormatYUV420PackedSemiPlanar;
        }
        catch(Exception ex) {
            if (Utils.isDebug) Utils.Log ( 5, className, "lookUpEncoderColorFormat: COLOR_TI_FormatYUV420PackedSemiPlanar not supported." );
        }


        return -1;
    }

}
