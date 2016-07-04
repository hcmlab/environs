/**
 * Dynamically accessing OpenCL
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
#ifndef INCLUDE_HCM_ENVIRONS_DYNAMIC_OPENCL_H
#define INCLUDE_HCM_ENVIRONS_DYNAMIC_OPENCL_H


#ifdef __APPLE__
#include <OpenCL/OpenCL.h>

#else
#include <CL/opencl.h>

#ifdef _WIN32 // ________________
#include <CL/OpenCL.h>
#if defined(CL_MEM_HOST_READ_ONLY) // AMD/ATI/Intel  -----------
#include <CL/cl_d3d11.h>

#else // NVIDIA ------------------------------------------------
#include <CL/cl_d3d11_ext.h>
#if !defined(CL_CONTEXT_D3D11_DEVICE_KHR)
#define CL_CONTEXT_D3D11_DEVICE_KHR	CL_CONTEXT_D3D11_DEVICE_NV
#endif
#if !defined(CL_D3D11_DEVICE_KHR)
#define CL_D3D11_DEVICE_KHR	CL_D3D11_DEVICE_NV
#endif
#if !defined(CL_D3D11_DXGI_ADAPTER_KHR)
#define CL_D3D11_DXGI_ADAPTER_KHR	CL_D3D11_DXGI_ADAPTER_NV
#endif
#if !defined(CL_PREFERRED_DEVICES_FOR_D3D11_KHR)
#define CL_PREFERRED_DEVICES_FOR_D3D11_KHR	CL_PREFERRED_DEVICES_FOR_D3D11_NV
#endif
#if !defined(CL_ALL_DEVICES_FOR_D3D11_KHR)
#define CL_ALL_DEVICES_FOR_D3D11_KHR		CL_ALL_DEVICES_FOR_D3D11_NV
#endif
#if !defined(CL_CONTEXT_INTEROP_USER_SYNC)
#define CL_CONTEXT_INTEROP_USER_SYNC 0x1085
#endif
#if !defined(cl_d3d11_device_source_khr)
#define cl_d3d11_device_source_khr cl_d3d11_device_source_nv
#endif
#if !defined(cl_d3d11_device_set_khr)
#define cl_d3d11_device_set_khr cl_d3d11_device_set_nv
#endif
#endif  //------------------------------------------------------

#endif // _WIN32 _________________

#endif



namespace environs
{

#define	CL_ERR(errmsg)					if (error != CL_SUCCESS) { CErrArgID (errmsg  " [%s]", OCLERR); }
#define	CL_ERR_break(errmsg)			if (error != CL_SUCCESS) { CErrArg (errmsg  " [%s]", OCLERR); break; }
#define	CL_ERR_break_opt(errmsg,opt)	if (error != CL_SUCCESS) { opt; CErrArgID (errmsg  " [%s] device [%d]", OCLERR, deviceID); break; }
#define	CL_ERR_break_dev(errmsg)		if (error != CL_SUCCESS) { CErrArgID (errmsg  " [%s] device [%d]", OCLERR, deviceID); break; }
#define	CL_ERR_return(errmsg)			if (error != CL_SUCCESS) { CErrArg (errmsg " [%s]", OCLERR); return false; }
#define	CL_ERR_goFinish(errmsg)			if (error != CL_SUCCESS) { CErrArg (errmsg " [%s]", OCLERR); goto Finish; }
#define	CL_ERR_goFail(errmsg)			if (error != CL_SUCCESS) { CErrArg (errmsg " [%s]", OCLERR); goto Fail; }
#define	VAL_ERR_goFinish(val,errmsg)	if (!val) { CErr (errmsg); goto Finish; }
#define	VAL_ERR_return(val,errmsg)		if (!val) { CErr (errmsg); return false; }
#define	VAL_ERR_break(val,errmsg)		if (!val) { CErr (errmsg); break; }

#define	OCLERR				oclError(error)
	extern const char* oclError ( cl_int error );

	extern const char * oclChannelDataType ( cl_uint dt );
	extern const char * oclChannelOrder ( cl_uint order );

#define oclReleaseMem(obj)	if ( obj ) { \
	dclReleaseMemObject ( obj ); \
	obj = 0; \
	}

#define oclReleaseKernel(obj)	if ( obj ) { \
	dclReleaseKernel ( obj ); \
	obj = 0; \
	}

#define oclReleaseQueue(obj)	if ( obj ) { \
	dclReleaseCommandQueue ( obj ); \
	obj = 0; \
	}

#define oclReleaseContext(obj)	if ( obj ) { \
	dclReleaseContext ( obj ); \
	obj = 0; \
	}

#define oclReleaseProgram(obj)	if ( obj ) { \
	dclReleaseProgram ( obj ); \
	obj = 0; \
	}

#define oclUnmapBuffer(mapped,buffer)	if ( mapped && buffer ) { \
	dclEnqueueUnmapMemObject ( ocl_queue, buffer, mapped, 0, 0, 0 ); \
	mapped = 0;  \
	}

#define CL_CHECK(_toCheck)    \
	do { \
		cl_int error = _toCheck; \
		if (error != CL_SUCCESS) { \
			CVerbArg ( "logOpenCLInfos: OpenCL Error: '%s' returned [%s]!", #_toCheck, OCLERR ); \
		} break; \
	} while ( 0 )



	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclGetDeviceInfo ) (
		cl_device_id    /* device */,
		cl_device_info  /* param_name */,
		size_t          /* param_value_size */,
		void *          /* param_value */,
		size_t *        /* param_value_size_ret */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_kernel ( CL_API_CALL * pclCreateKernel ) (
		cl_program      /* program */,
		const char *    /* kernel_name */,
		cl_int *        /* errcode_ret */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclGetProgramBuildInfo ) (
		cl_program            /* program */,
		cl_device_id          /* device */,
		cl_program_build_info /* param_name */,
		size_t                /* param_value_size */,
		void *                /* param_value */,
		size_t *              /* param_value_size_ret */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclBuildProgram ) (
		cl_program           /* program */,
		cl_uint              /* num_devices */,
		const cl_device_id * /* device_list */,
		const char *         /* options */,
		void ( CL_CALLBACK *  /* pfn_notify */ )( cl_program /* program */, void * /* user_data */ ),
		void *               /* user_data */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_program ( CL_API_CALL * pclCreateProgramWithSource ) (
		cl_context        /* context */,
		cl_uint           /* count */,
		const char **     /* strings */,
		const size_t *    /* lengths */,
		cl_int *          /* errcode_ret */ ) CL_API_SUFFIX__VERSION_1_0;

	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclReleaseProgram ) ( cl_program ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_context ( CL_API_CALL * pclCreateContext ) (
		const cl_context_properties * /* properties */,
		cl_uint                       /* num_devices */,
		const cl_device_id *          /* devices */,
		void ( CL_CALLBACK * /* pfn_notify */ )( const char *, const void *, size_t, void * ),
		void *                        /* user_data */,
		cl_int *                      /* errcode_ret */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclGetDeviceIDs ) (
		cl_platform_id   /* platform */,
		cl_device_type   /* device_type */,
		cl_uint          /* num_entries */,
		cl_device_id *   /* devices */,
		cl_uint *        /* num_devices */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclGetPlatformIDs ) (
		cl_uint          /* num_entries */,
		cl_platform_id * /* platforms */,
		cl_uint *        /* num_platforms */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclReleaseContext ) ( cl_context /* context */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclReleaseKernel ) ( cl_kernel /* kernel */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclSetKernelArg ) (
		cl_kernel    /* kernel */,
		cl_uint      /* arg_index */,
		size_t       /* arg_size */,
		const void * /* arg_value */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_mem ( CL_API_CALL * pclCreateBuffer ) (
		cl_context   /* context */,
		cl_mem_flags /* flags */,
		size_t       /* size */,
		void *       /* host_ptr */,
		cl_int *     /* errcode_ret */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_command_queue ( CL_API_CALL * pclCreateCommandQueue ) (
		cl_context                     /* context */,
		cl_device_id                   /* device */,
		cl_command_queue_properties    /* properties */,
		cl_int *                       /* errcode_ret */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclReleaseCommandQueue ) ( cl_command_queue /* command_queue */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclReleaseMemObject ) ( cl_mem /* memobj */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclEnqueueUnmapMemObject ) (
		cl_command_queue /* command_queue */,
		cl_mem           /* memobj */,
		void *           /* mapped_ptr */,
		cl_uint          /* num_events_in_wait_list */,
		const cl_event *  /* event_wait_list */,
		cl_event *        /* event */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclFinish ) ( cl_command_queue /* command_queue */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclEnqueueReadBuffer ) (
		cl_command_queue    /* command_queue */,
		cl_mem              /* buffer */,
		cl_bool             /* blocking_read */,
		size_t              /* offset */,
		size_t              /* cb */,
		void *              /* ptr */,
		cl_uint             /* num_events_in_wait_list */,
		const cl_event *    /* event_wait_list */,
		cl_event *          /* event */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclEnqueueNDRangeKernel ) (
		cl_command_queue /* command_queue */,
		cl_kernel        /* kernel */,
		cl_uint          /* work_dim */,
		const size_t *   /* global_work_offset */,
		const size_t *   /* global_work_size */,
		const size_t *   /* local_work_size */,
		cl_uint          /* num_events_in_wait_list */,
		const cl_event * /* event_wait_list */,
		cl_event *       /* event */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclEnqueueWriteBuffer ) (
		cl_command_queue   /* command_queue */,
		cl_mem             /* buffer */,
		cl_bool            /* blocking_write */,
		size_t             /* offset */,
		size_t             /* cb */,
		const void *       /* ptr */,
		cl_uint            /* num_events_in_wait_list */,
		const cl_event *   /* event_wait_list */,
		cl_event *         /* event */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY void * ( CL_API_CALL * pclEnqueueMapBuffer ) (
		cl_command_queue /* command_queue */,
		cl_mem           /* buffer */,
		cl_bool          /* blocking_map */,
		cl_map_flags     /* map_flags */,
		size_t           /* offset */,
		size_t           /* cb */,
		cl_uint          /* num_events_in_wait_list */,
		const cl_event * /* event_wait_list */,
		cl_event *       /* event */,
		cl_int *         /* errcode_ret */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclGetProgramInfo ) (
		cl_program         /* program */,
		cl_program_info    /* param_name */,
		size_t             /* param_value_size */,
		void *             /* param_value */,
		size_t *           /* param_value_size_ret */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_program ( CL_API_CALL * pclCreateProgramWithBinary ) (
		cl_context                     /* context */,
		cl_uint                        /* num_devices */,
		const cl_device_id *           /* device_list */,
		const size_t *                 /* lengths */,
		const unsigned char **         /* binaries */,
		cl_int *                       /* binary_status */,
		cl_int *                       /* errcode_ret */ ) CL_API_SUFFIX__VERSION_1_0;


	typedef CL_API_ENTRY cl_int ( CL_API_CALL * pclWaitForEvents ) (
		cl_uint             /* num_events */,
		const cl_event *    /* event_list */ ) CL_API_SUFFIX__VERSION_1_0;

	typedef CL_API_ENTRY void * ( CL_API_CALL * pclGetExtensionFunctionAddress ) ( const char * /* func_name */ )CL_API_SUFFIX__VERSION_1_0;

#ifdef _WIN32
	typedef CL_API_ENTRY cl_mem ( CL_API_CALL *pclCreateFromD3D11Texture2D )(
		cl_context        context,
		cl_mem_flags      flags,
		void *			  d3dTexture,
		unsigned int      subresource,
		cl_int *          errcode_ret ) CL_API_SUFFIX__VERSION_1_0;

	typedef CL_API_ENTRY cl_int ( CL_API_CALL *pclEnqueueAcquireD3D11Objects )(
		cl_command_queue command_queue,
		cl_uint          num_objects,
		const cl_mem *   mem_objects,
		cl_uint          num_events_in_wait_list,
		const cl_event * event_wait_list,
		cl_event *       event ) CL_API_SUFFIX__VERSION_1_0;

	typedef CL_API_ENTRY cl_int ( CL_API_CALL *pclEnqueueReleaseD3D11Objects )(
		cl_command_queue command_queue,
		cl_uint          num_objects,
		cl_mem *         mem_objects,
		cl_uint          num_events_in_wait_list,
		const cl_event * event_wait_list,
		cl_event *       event ) CL_API_SUFFIX__VERSION_1_0;

	typedef CL_API_ENTRY cl_int ( CL_API_CALL *pclGetDeviceIDsFromD3D11 )(
		cl_platform_id            platform,
		cl_d3d11_device_source_khr d3d_device_source,
		void *                    d3d_object,
		cl_d3d11_device_set_khr    d3d_device_set,
		cl_uint                   num_entries,
		cl_device_id *            devices,
		cl_uint *                 num_devices ) CL_API_SUFFIX__VERSION_1_0;
#endif

	typedef CL_API_ENTRY cl_int ( CL_API_CALL *pclGetImageInfo )(
		cl_mem           /* image */,
		cl_image_info    /* param_name */,
		size_t           /* param_value_size */,
		void *           /* param_value */,
		size_t *         /* param_value_size_ret */ ) CL_API_SUFFIX__VERSION_1_0;

	typedef CL_API_ENTRY cl_int ( CL_API_CALL *pclGetSupportedImageFormats )(
		cl_context           /* context */,
		cl_mem_flags         /* flags */,
		cl_mem_object_type   /* image_type */,
		cl_uint              /* num_entries */,
		cl_image_format *    /* image_formats */,
		cl_uint *            /* num_image_formats */ ) CL_API_SUFFIX__VERSION_1_0;



extern void ReleaseOpenCLLib ();
extern bool InitOpenCLLib ();

extern bool								ocl_LibInitialized;

extern pclEnqueueWriteBuffer			dclEnqueueWriteBuffer;
extern pclEnqueueNDRangeKernel			dclEnqueueNDRangeKernel;
extern pclEnqueueReadBuffer				dclEnqueueReadBuffer;
extern pclFinish						dclFinish;
extern pclEnqueueUnmapMemObject			dclEnqueueUnmapMemObject;
extern pclReleaseMemObject				dclReleaseMemObject;
extern pclReleaseCommandQueue			dclReleaseCommandQueue;
extern pclCreateCommandQueue			dclCreateCommandQueue;
extern pclCreateBuffer					dclCreateBuffer;
extern pclSetKernelArg					dclSetKernelArg;
extern pclReleaseKernel					dclReleaseKernel;
extern pclReleaseContext				dclReleaseContext;
extern pclGetPlatformIDs				dclGetPlatformIDs;
extern pclGetDeviceIDs					dclGetDeviceIDs;
extern pclCreateContext					dclCreateContext;
extern pclCreateProgramWithSource		dclCreateProgramWithSource;
extern pclReleaseProgram				dclReleaseProgram;
extern pclCreateProgramWithBinary		dclCreateProgramWithBinary;
extern pclBuildProgram					dclBuildProgram;
extern pclGetProgramBuildInfo			dclGetProgramBuildInfo;
extern pclGetProgramInfo				dclGetProgramInfo;
extern pclGetImageInfo					dclGetImageInfo;
extern pclCreateKernel					dclCreateKernel;
extern pclGetDeviceInfo					dclGetDeviceInfo;
extern pclEnqueueMapBuffer				dclEnqueueMapBuffer;
extern pclWaitForEvents					dclWaitForEvents;

extern pclGetSupportedImageFormats		dclGetSupportedImageFormats;
extern pclGetExtensionFunctionAddress	dclGetExtensionFunctionAddress;
extern unsigned int						ocl_d3d_versup;

#ifdef _WIN32
extern pclGetDeviceIDsFromD3D11			dclGetDeviceIDsFromD3D11;
extern pclCreateFromD3D11Texture2D		dclCreateFromD3D11Texture2D;
extern pclEnqueueAcquireD3D11Objects	dclEnqueueAcquireD3D11Objects;
extern pclEnqueueReleaseD3D11Objects	dclEnqueueReleaseD3D11Objects;
#endif
} // -> namespace environs


#endif	/// INCLUDE_HCM_ENVIRONS_DYNAMIC_OPENCL_H
