package environs;
/**
 * Android Java Decoder for images and h264 streams
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

@SuppressWarnings("deprecation")
@SuppressLint("NewApi")
class Decoder extends Codec
{
	private static final String className = "Decoder. . . . . . . . .";

	Decoder() {
    	Utils.Log2 ( className, "Construct...");
		API21 = (android.os.Build.VERSION.SDK_INT > 20);
	}


	boolean initCoDec() {
		return false;
	}


	MediaCodec getCoDec () throws Exception
	{
		return MediaCodec.createDecoderByType(mimeType);
	}


	MediaFormat configureFormat (MediaFormat format) throws Exception
	{
		format.setInteger( MediaFormat.KEY_FRAME_RATE, 33 );
		format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar);
		return format;
	}


	void setConfigureFormat (MediaFormat format) throws Exception
	{
		this.format = format;
		CoDec.configure(format, null, null, 0);
	}


	boolean onPreStart (MediaCodec codec)
	{
		return true;
	}


	public void release()
	{
		super.release();
	}


	void outputCallback (int outputStatus)
	{
		//Utils.Log ( className, "decode: frame decoded!");
		/*if ( API21 )
			CoDec.releaseOutputBuffer(outputStatus, lastPresentationTime);
		else {
		}
		*/
		CoDec.releaseOutputBuffer(outputStatus, true);

		if ( ( bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM ) != 0 )
			isEoS = true;
	}


}
