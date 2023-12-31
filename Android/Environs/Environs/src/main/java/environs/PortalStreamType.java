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
 * Portal stream type enumeration. Represents the same values as for STREAMTYPE_*
 * */
public abstract class PortalStreamType {

	@IntDef(flag=true, value={Unknown, Images, ImagesJPEG, ImagesPNG, Video, VideoH264})

	@Retention(RetentionPolicy.SOURCE)
	public @interface Value {}

	public static final int Unknown             	=	Types.STREAMTYPE_UNKNOWN;
			/** Sequence of jpeg images. 	*/
	public static final int Images              	=	Types.STREAMTYPE_IMAGES;
			/** Sequence of jpeg images. 	*/
	public static final int ImagesJPEG          	=	Types.STREAMTYPE_IMAGES_JPEG;
			/** Sequence of png images. 	*/
	public static final int ImagesPNG           	=	Types.STREAMTYPE_IMAGES_PNG;
			/** Video stream. 						*/
	public static final int Video               	=	Types.STREAMTYPE_VIDEO;
			/** Video stream H264. 						*/
	public static final int VideoH264           	=	Types.STREAMTYPE_VIDEO_H264;
}





