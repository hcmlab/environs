/**
 * OpenCL portal implementation
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTAL_OPENCL_H
#define INCLUDE_HCM_ENVIRONS_PORTAL_OPENCL_H

#include "Interfaces/IPortal.Renderer.h"

#ifdef __APPLE__
#include <OpenCL/OpenCL.h>
#else // __APPLE__
#include <CL/opencl.h>
#endif // __APPLE__

//#define USE_OCL_DIRECT_YUV_BUFFER // using the other method is faster 1,3ms vs. 3-4ms
#define OCL_DEVICES_MAX	10


namespace environs 
{
	/**
	*	OpenCL portal implementation
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks	Header for OpenCL portal rendering
	* ****************************************************************************************
	*/
	class RenderOpenCL : implements IPortalRenderer
	{
	public:
		RenderOpenCL ( void );
		virtual ~RenderOpenCL ( );

		bool						Init ( );
		void						Dispose ( );
		
		bool						Compare ( unsigned int &equal );
		bool						Perform ( RenderDimensions * dims, RenderContext * context );
		int                         ReleaseResources ( RenderContext * context );
		int							AllocateResources ( RenderDimensions * dims );
		int							UpdateBuffers ( RenderDimensions * dims, RenderContext * context );

		//bool						ConnectOutput ( IEnvironsBase * dest );

		char					*	GetBuffer ( RenderContext * context );
		
		static bool					InitOpenCL ();
		static void					DisposeOpenCL ();

		static unsigned int			ocl_initialized;

	private:

		static cl_platform_id		ocl_platform;
		static cl_context			ocl_context;
		static cl_uint				ocl_devices_count;		
		static cl_device_id			ocl_devices [ OCL_DEVICES_MAX ];
		static unsigned int			ocl_next_cqueue;

		static cl_program			ocl_program;
		static size_t				ocl_work_item_max_0; 
		static size_t				ocl_work_group_max_0;
		size_t						ocl_square_compare_global_ws[2];
		size_t						ocl_square_rotate_global_ws[2];
		size_t						ocl_output_global_ws[2];
		size_t						ocl_local_ws_16_16[2];
		size_t						ocl_local_ws_16_32[2];
		
		cl_command_queue			ocl_queue;
		cl_kernel					ocl_kernel_compare;
		cl_kernel					ocl_kernel_compare_bgra;
		cl_kernel					ocl_kernel_compare_texture;
		cl_kernel					ocl_kernel_rotate;
		cl_kernel					ocl_kernel_copy;
		cl_kernel					ocl_kernel_resize;
		cl_kernel					ocl_kernel_2yuv;
		cl_kernel					ocl_kernel_2nv12;
		unsigned int				ocl_buffer_square_size;
		cl_mem						ocl_buffer_square;
		cl_mem						ocl_buffer_square_cache;
		cl_mem						ocl_texture2D;
		cl_mem						ocl_image_rotated;

		unsigned int				ocl_image_output_size;
		cl_mem						ocl_image_output;

		unsigned int				ocl_image_yuv_size;
		cl_mem						ocl_image_yuv;

		size_t                      ocl_image_yuv_global_ws1;
		size_t                      ocl_image_yuv_local_ws1;
		cl_mem						ocl_variable_eq;

		void					*	ocl_mapped_ptr;
		
		unsigned int				bufferedFlushCount;

		static bool					LoadEnvironsKernels ();
		static bool					LoadBinaryKernels ();
		
		bool						MainThreadedInit ();
		bool						MainThreadedDispose ();

	};

} /* namespace environs */


#endif // INCLUDE_HCM_ENVIRONS_PORTAL_OPENCL_H

