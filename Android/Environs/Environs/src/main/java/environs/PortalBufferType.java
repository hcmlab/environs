package environs;

/* DO NOT EDIT THIS FILE - it is machine generated by j2c.jar */

/**
 * ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
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
import android.support.annotation.IntDef;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/** 
 * Portal stage buffer data type enumeration.
 * */
public abstract class PortalBufferType {

	@IntDef(flag=true, value={Unknown, ARGB, ARGBHandle, BGRA, RGB, BGR, YUV420, 
			YV12, YUY2, NV12, GDIBitmap, Texture3D, PixelBuffer3D, CVPixelBufferIOSX, 
	})

	@Retention(RetentionPolicy.SOURCE)
	public @interface Value {}

	public static final int Unknown             	=	Types.PORTAL_BUFFERTYPE_UNKNOWN;
			/** Windows ARGB. */
	public static final int ARGB                	=	Types.PORTAL_BUFFERTYPE_ARGB;
			/** Windows ARGB and the associated HBITMAP handle. */
	public static final int ARGBHandle          	=	Types.PORTAL_BUFFERTYPE_ARGB_HANDLE;
			/** iOS ARGB. */
	public static final int BGRA                	=	Types.PORTAL_BUFFERTYPE_BGRA;
			/** RGB 24bit. */
	public static final int RGB                 	=	Types.PORTAL_BUFFERTYPE_RGB;
			/** BGR 24bit. */
	public static final int BGR                 	=	Types.PORTAL_BUFFERTYPE_BGR;
			/** I420. */
	public static final int YUV420              	=	Types.PORTAL_BUFFERTYPE_YUV420;
			/** YV12. */
	public static final int YV12                	=	Types.PORTAL_BUFFERTYPE_YV12;
			/** YUY2. */
	public static final int YUY2                	=	Types.PORTAL_BUFFERTYPE_YUV2;
			/** NV12. */
	public static final int NV12                	=	Types.PORTAL_BUFFERTYPE_NV12;
			/** GDIBitmap. */
	public static final int GDIBitmap           	=	Types.PORTAL_BUFFERTYPE_GDI_BITMAP;
			/** The data follows either D3D or OpenGL texture format. */
	public static final int Texture3D           	=	Types.PORTAL_BUFFERTYPE_TEXTURE_3D;
			/** The data follows either D3D or OpenGL buffer format. */
	public static final int PixelBuffer3D       	=	Types.PORTAL_BUFFERTYPE_PIXELBUFFER_3D;
			/** CVPixelBufferRef of apple platforms. */
	public static final int CVPixelBufferIOSX   	=	Types.PORTAL_BUFFERTYPE_CVPIXELBUFFER_IOSX;
}




