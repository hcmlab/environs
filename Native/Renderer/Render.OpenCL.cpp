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
#include "stdafx.h"

#ifndef ENVIRONS_IOS

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#define GLOBAL_WORK_GROUP_SIZE_FIX

#ifdef _WIN32
#include <Windows.h>
#endif
#include "Render.OpenCL.h"
#include "Environs.Obj.h"
#include "DynLib/Dyn.OpenCL.h"
#include "Device/Device.Controller.h"
#include "Core/Performance.Count.h"
#include "Environs.Utils.h"
#include "Environs.Build.Lnk.h"

#ifdef _WIN32 // ----------------------------
#   include "Capture/Capture.Win.D3D.h"
#endif  // ---------------------------------

#include "math.h"
#define PI 3.14159265

#define CLASS_NAME	"Render.OpenCL. . . . . ."

#define KERNEL_SOURCE_FILE	"./environs.kernels.cl.c"
#define KERNEL_BINARY_FILE	"environs.kernels.cl.bin"	


#define USE_LINEAR_COMPARE
#define USE_LINEAR_YUV // improves by 2-3 ms


namespace environs
{
	//	
	// Initialization of static values
	PortalBufferType_t	RenderOpenCL_inputTypeSupport[] = { PortalBufferType::ARGB };

	PortalBufferType_t	RenderOpenCL_outputTypeSupport[] = { PortalBufferType::YUV420, PortalBufferType::NV12, PortalBufferType::ARGB };
	

	unsigned int	RenderOpenCL::ocl_initialized		= 0;
	size_t			RenderOpenCL::ocl_work_item_max_0	= 512;
	size_t			RenderOpenCL::ocl_work_group_max_0	= 512;
	cl_platform_id	RenderOpenCL::ocl_platform			= 0;
	cl_context		RenderOpenCL::ocl_context			= 0;
	cl_program		RenderOpenCL::ocl_program			= 0;
	cl_uint			RenderOpenCL::ocl_devices_count		= 0;
	cl_device_id	RenderOpenCL::ocl_devices			[OCL_DEVICES_MAX];

	unsigned int	RenderOpenCL::ocl_next_cqueue		= 0;

	int				IPortalRenderer::device_left		= 0;
	int				IPortalRenderer::device_top			= 0;


	extern void logOpenCLInfos ( cl_device_id device );

	//extern char * LoadBinary ( const char * fileName, int * size );


	// -------------------------------------------------------------------
	// Constructor
	//		Initialize member variables
	// -------------------------------------------------------------------
	RenderOpenCL::RenderOpenCL ()
	{
		CLog ( "Construct" );

		ocl_variable_eq				= 0;
		ocl_buffer_square			= 0;
		ocl_buffer_square_size		= 0;
		ocl_image_yuv_size			= 0;
		ocl_image_output_size		= 0;
		ocl_image_rotated			= 0;
		ocl_image_output			= 0;
		ocl_image_yuv				= 0;
		ocl_image_yuv_global_ws1	= 0;
		ocl_image_yuv_local_ws1		= 0;
		ocl_buffer_square_cache		= 0;
		ocl_mapped_ptr				= 0;
		ocl_queue					= 0;
		ocl_texture2D				= 0;

		Zero ( ocl_square_compare_global_ws );
		Zero ( ocl_square_rotate_global_ws );
		Zero ( ocl_output_global_ws );
		Zero ( ocl_local_ws_16_16 );
		Zero ( ocl_local_ws_16_32 );

		ocl_kernel_compare			= 0;
		ocl_kernel_compare_bgra		= 0;
		ocl_kernel_compare_texture	= 0;
		ocl_kernel_rotate			= 0;
		ocl_kernel_copy				= 0;
		ocl_kernel_resize			= 0;
		ocl_kernel_2yuv				= 0;
		ocl_kernel_2nv12			= 0;

		useRenderCache				= true;

		bufferedFlushCount			= 0;
		name						= "OpenCL Renderer";

		inputTypes					= RenderOpenCL_inputTypeSupport;
		inputTypesLength			= sizeof(RenderOpenCL_inputTypeSupport) / sizeof(RenderOpenCL_inputTypeSupport[0]);

		outputTypes					= RenderOpenCL_outputTypeSupport;
		outputTypesLength			= sizeof(RenderOpenCL_outputTypeSupport) / sizeof(RenderOpenCL_outputTypeSupport[0]);
	}


	RenderOpenCL::~RenderOpenCL ()
	{
		CLogID ( "Destructor" );

		Dispose ();
	}


	bool RenderOpenCL::InitOpenCL ()
	{
		if ( ocl_initialized )
			return true;

		if ( !InitOpenCLLib () )
			return false;

		CVerb ( "InitOpenCL" );

		cl_int	error			= 0;
		cl_uint platformCount	= 0;
		Zero ( ocl_devices );

		do {
			//error = clGetPlatformIDs ( 0, 0, &platformCount );
			// Platform
			error = dclGetPlatformIDs ( 1, &ocl_platform, &platformCount );
			//
			CL_ERR_break ( "InitOpenCL: Failed to get platform id." );


			// Devices & Context
#if defined(WINDOWS_8) && defined(ENABLE_WIND3D_CAPTURE)
			if ( ocl_d3d_versup )
			{
				while ( CaptureWinD3D::d3dDevice && dclGetDeviceIDsFromD3D11 ) {
					ocl_devices_count = OCL_DEVICES_MAX;
					/*
					// Query a device compatible with our DXGI adapter
					error = dclGetDeviceIDsFromD3D11 ( ocl_platform, CL_D3D11_DXGI_ADAPTER_KHR,
					CaptureWinD3D::dxgiAdapter, CL_PREFERRED_DEVICES_FOR_D3D11_KHR,
					ocl_devices_count, ocl_devices, &ocl_devices_count );
					CL_ERR ( "InitOpenCL: Failed to get device ids for DXGI adapter." );

					if ( error == CL_SUCCESS && ocl_devices_count > 0 )
					{
					cl_context_properties properties[] = {
					CL_CONTEXT_PLATFORM, (cl_context_properties) ocl_platform,
					CL_CONTEXT_D3D11_DEVICE_KHR, (cl_context_properties) (CaptureWinD3D::d3dDevice),
					//CL_CONTEXT_INTEROP_USER_SYNC, CL_FALSE,
					NULL, NULL };

					ocl_context = dclCreateContext ( properties, ocl_devices_count, ocl_devices, NULL, NULL, &error );
					CL_ERR ( "InitOpenCL: Failed to create context (for DXGI adapter)." )
					else
					break;
					}
					*/
					// Query a device compatible with our D3D device
					error = dclGetDeviceIDsFromD3D11 ( ocl_platform, CL_D3D11_DEVICE_KHR,
						CaptureWinD3D::d3dDevice, CL_PREFERRED_DEVICES_FOR_D3D11_KHR,
						ocl_devices_count, ocl_devices, &ocl_devices_count );
					CL_ERR_break ( "InitOpenCL: Failed to get device ids for D3D." );

					if ( ocl_devices_count == 0 ) {
						CErr ( "InitOpenCL: No GPU D3D devices available." );
						break;
					}
					else {
						CLogArg ( "InitOpenCL: We have [%d] D3D devices", ocl_devices_count );
					}

					cl_context_properties properties [ ] = {
						CL_CONTEXT_PLATFORM, ( cl_context_properties ) ocl_platform,
						CL_CONTEXT_D3D11_DEVICE_KHR, ( cl_context_properties ) ( CaptureWinD3D::d3dDevice ),
						//CL_CONTEXT_INTEROP_USER_SYNC, CL_FALSE,
						NULL, NULL };

					ocl_context = dclCreateContext ( properties, ocl_devices_count, ocl_devices, NULL, NULL, &error );
					CL_ERR_break ( "InitOpenCL: Failed to create context (for D3D device)." );

					break;
				}
			}
			else
#endif

#if (defined(WINDOWS_8) && !defined(WINDOWS_PHONE))
				opt_useWinD3D = false;
#endif
			if ( !ocl_context ) {
				ocl_devices_count = OCL_DEVICES_MAX;

				error = dclGetDeviceIDs ( ocl_platform, CL_DEVICE_TYPE_GPU, ocl_devices_count, ocl_devices, &ocl_devices_count );
				//
				CL_ERR_break ( "InitOpenCL: Failed to get device ids." );

				if ( ocl_devices_count == 0 ) {
					CErr ( "InitOpenCL: No GPU devices available." );
					break;
				}
				else {
					CLogArg ( "InitOpenCL: We have [%d] devices", ocl_devices_count );
				}

				ocl_context = dclCreateContext ( 0, ocl_devices_count, ocl_devices, NULL, NULL, &error );
				//
				CL_ERR_break ( "InitOpenCL: Failed to create context." );
			}

			// work item max sizes 
			size_t dims [ 6 ];
			error = dclGetDeviceInfo ( ocl_devices [ 0 ], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof ( dims ), dims, NULL );
			//
			CL_ERR_break ( "InitOpenCL: Failed to get work item size for dimensions." );

			ocl_work_item_max_0 = dims [ 0 ];


			error = dclGetDeviceInfo ( ocl_devices [ 0 ], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof ( ocl_work_group_max_0 ), &ocl_work_group_max_0, NULL );
			//
			CL_ERR_break ( "InitOpenCL: Failed to get work group max size." );


			logOpenCLInfos ( ocl_devices [ 0 ] );
			/*
			cl_image_format formats [80];
			cl_uint formatSize = 0;

			error = dclGetSupportedImageFormats ( ocl_context, CL_MEM_READ_ONLY, CL_MEM_OBJECT_IMAGE2D, 80, formats, &formatSize );
			if ( error != CL_SUCCESS ) {
			CErrArg ( "InitOpenCL: Failed to get supported image formats. Error [%s]", OCLERR );
			}
			else {
			if ( formatSize > 80 )
			formatSize = 80;
			for ( cl_uint i = 0; i < formatSize; i++ ) {
			CLogArg ( "InitOpenCL: image format [%u]: order[%s] - data type [%s]", i, getChannelOrder ( formats [i].image_channel_order ), getChannelDataType ( formats [i].image_channel_data_type ) );
			}
			}
			*/

			if ( !LoadEnvironsKernels () ) {
				CErrArg ( "InitOpenCL: Failed to get work group max size. Error [%s]", OCLERR );
				break;
			}

#if defined(WINDOWS_8) && defined(ENABLE_WIND3D_CAPTURE)
			if ( CaptureWinD3D::d3dDevice )
				ocl_initialized = 2;
			else
#endif
				ocl_initialized = 1;
			return true;
		}
		while ( 0 );

		return false;
	}


	void RenderOpenCL::DisposeOpenCL ()
	{
		CLog ( "DisposeOpenCL" );

		// Crashes on OSX (Bootcamp)... Dont know why...
		// Maybe Intel OpenCL implementation requires disposal and alloc to be done by the same thread???
		oclReleaseProgram ( ocl_program );

		ocl_initialized = 0;

		ocl_platform = 0;

		Zero ( ocl_devices );

		// This call will block the caller on apple devices (multiple instances of the same app)
		// Probably because of undocumented requirements .. same thread?
		oclReleaseContext ( ocl_context );
	}


	bool RenderOpenCL::LoadEnvironsKernels ()
	{
		ocl_program = 0;

		// Check whether we already have a binary precompiled
		/*if ( LoadBinaryKernels () )
		return true;*/

		cl_int		error		= 0;
		size_t	*	binSizes	= 0;
		char	**	binaries	= 0;
		FILE	*	fp			= 0;
		bool		ret			= false;

		int length = 0;
		char * sources = LoadBinary ( KERNEL_SOURCE_FILE, &length );

		if ( !sources )
			return false;
		
		bool buildFailed = true;
		size_t size = length;

		ocl_program = dclCreateProgramWithSource ( ocl_context, 1, ( const char ** ) &sources, &size, &error );
		//
		CL_ERR_goFail ( "LoadEnvironsKernels: Failed to load the kernels." );


		// Builds the program
		buildFailed = false;

		//error = dclBuildProgram ( ocl_program, ocl_devices_count, ocl_devices, NULL, NULL, NULL );
		//error = dclBuildProgram ( ocl_program, ocl_devices_count, ocl_devices, "-cl-mad-enable", NULL, NULL );
		error = dclBuildProgram ( ocl_program, ocl_devices_count, ocl_devices, "-cl-fast-relaxed-math", NULL, NULL );
		if ( error != CL_SUCCESS ) {
			CErrArg ( "LoadEnvironsKernels: Failed to build the kernels. Error [%s]", OCLERR );
			buildFailed = true;
		}

#ifdef _DEBUG
		error = dclGetProgramBuildInfo ( ocl_program, ocl_devices [ 0 ], CL_PROGRAM_BUILD_LOG, 0, NULL, &size );
		if ( error == CL_SUCCESS ) {
			char * log = ( char * ) malloc ( sizeof ( char )* ( size + 4 ) );
			if ( log ) {
				error = dclGetProgramBuildInfo ( ocl_program, ocl_devices [ 0 ], CL_PROGRAM_BUILD_LOG, size, log, NULL );
				if ( error == CL_SUCCESS ) {
					log [ size ] = '\0';
					CVerbArg ( "LoadEnvironsKernels: Build log [%s]", log );
				}
				else {
					CErrArg ( "LoadEnvironsKernels: Failed to retrieve build log. Error [%s]", OCLERR );
					log [ size ] = '\0';
					CVerbArg ( "LoadEnvironsKernels: Build log [%s]", log );
				}
				free ( log );
			}
		}
		else {
			CErrArg ( "LoadEnvironsKernels: Failed to retrieve size of build log. Error [%s]", OCLERR );
		}
#endif
	Fail:
		free ( sources );

		if ( buildFailed )
			return false;

		ret = true;

		size_t binRead = 0;
		cl_uint binDevices = 0;

		error = dclGetProgramInfo ( ocl_program, CL_PROGRAM_NUM_DEVICES, sizeof ( cl_uint ), &binDevices, &binRead );
		//
		CL_ERR_return ( "LoadEnvironsKernels: Failed to get number of devices in the program." );

		if ( binDevices <= 0 ) {
			CErr ( "LoadEnvironsKernels: No devices." ); return false;
		}

		binSizes = ( size_t * ) malloc ( sizeof ( size_t )* binDevices );
		//
		VAL_ERR_return ( binSizes, "LoadEnvironsKernels: Failed to create sizes array for binaries." );


		error = dclGetProgramInfo ( ocl_program, CL_PROGRAM_BINARY_SIZES, sizeof ( size_t ) * binDevices, binSizes, &binRead );
		//
		CL_ERR_goFinish ( "LoadEnvironsKernels: Failed to sizes of the binaries in the program." );


		binaries = ( char ** ) malloc ( sizeof ( char * )* binDevices );
		//
		VAL_ERR_goFinish ( binaries, "LoadEnvironsKernels: Failed to create array for binaries." );

		memset ( binaries, 0, sizeof ( char * ) * binDevices );

		for ( unsigned int i = 0; i < binDevices; i++ )
		{
			binaries [ i ] = ( char * ) malloc ( sizeof ( char )* ( binSizes [ i ] ) );
			if ( !binaries [ i ] ) {
				CErrArg ( "LoadEnvironsKernels: Failed to allocate memory for binary [ %i ].", i );
				goto Finish;
			}
		}

		error = dclGetProgramInfo ( ocl_program, CL_PROGRAM_BINARIES, sizeof ( unsigned char * ) * binDevices, binaries, &binRead );
		//
		CL_ERR_goFinish ( "LoadEnvironsKernels: Failed to load the binaries in the program." );

		//printf ( "%s\n", binaries [ 0 ] );   
		/*
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4996 )
#endif

		fp = fopen ( KERNEL_BINARY_FILE, "wb" );

#ifdef _WIN32
#pragma warning( pop )
#endif
		//
		VAL_ERR_goFinish ( fp, "LoadEnvironsKernels: Failed to create binary file to save the compilation." );


		fwrite ( binaries [ 0 ], sizeof ( char ), binSizes [ 0 ], fp );
		*/
	Finish:
		if ( binSizes )
			free ( binSizes );

		if ( binaries ) {
			for ( unsigned int i = 0; i < binDevices; i++ ) {
				if ( binaries [ i ] )
					free ( binaries [ i ] );
			}
			free ( binaries );
		}

		if ( fp )
			fclose ( fp );

		return ret;
	}


	bool RenderOpenCL::LoadBinaryKernels ()
	{
		// Check whether we already have a binary precompiled

		// Get filesize if it exists 
		int length = 0;
		size_t filesize = 0;
		cl_int error = 0;
		cl_int binStatus = 0;
		bool ret = false;

		char * binary = LoadBinary ( KERNEL_BINARY_FILE, &length );
		if ( !binary ) {
			return false;
		}
		filesize = length;

		ocl_program = dclCreateProgramWithBinary ( ocl_context, ocl_devices_count, ocl_devices, &filesize, ( const unsigned char ** ) &binary, &binStatus, &error );

		if ( binStatus == CL_SUCCESS && error == CL_SUCCESS && ocl_program ) {

			error = dclBuildProgram ( ocl_program, 0, NULL, NULL, NULL, NULL );
			//error = dclBuildProgram ( ocl_program, ocl_devices_count, ocl_devices, "-cl-mad-enable", NULL, NULL );
			//
			CL_ERR_goFinish ( "LoadBinaryKernels: Failed to build the kernels from binary." );

			error = dclGetProgramBuildInfo ( ocl_program, ocl_devices [ 0 ], CL_PROGRAM_BUILD_LOG, 0, NULL, &filesize );
			if ( error == CL_SUCCESS ) {
				char * log = ( char * ) malloc ( sizeof ( char )* ( filesize + 4 ) );
				if ( log ) {
					error = dclGetProgramBuildInfo ( ocl_program, ocl_devices [ 0 ], CL_PROGRAM_BUILD_LOG, filesize, log, NULL );
					if ( error == CL_SUCCESS ) {
						log [ filesize ] = '\0';
						CLogArg ( "LoadBinaryKernels: Build log [%s]", log );
					}
					else {
						CErrArg ( "LoadBinaryKernels: Failed to retrieve build log. Error [%s]", OCLERR );
					}
					free ( log );
				}
			}
			else {
				CErrArg ( "LoadBinaryKernels: Failed to retrieve size of build log. Error [%s]", OCLERR );
			}

			ret = true;
		}

	Finish:
		if ( binary )
			free ( binary );
		return ret;
	}


	bool RenderOpenCL::Init ()
	{
		CVerbID ( "Init" );

		if ( !ocl_initialized )
			return false;

		//outputType = PortalBufferType::YUV420;

		if ( !MainThreadedInit () )
			return false;

		if ( env ) {
			useRenderCache = env->useRenderCache;
		}

		cl_int error = 0;   // Used to handle error codes

		ocl_local_ws_16_16 [ 0 ] = 16;
		ocl_local_ws_16_16 [ 1 ] = 16;

		ocl_local_ws_16_32 [ 0 ] = 16;
		ocl_local_ws_16_32 [ 1 ] = ocl_work_group_max_0 / 16;

		// Create the kernels
#ifdef USE_LINEAR_COMPARE
		ocl_kernel_compare = dclCreateKernel ( ocl_program, "compare_buffers", &error );
#else
		ocl_kernel_compare = dclCreateKernel ( ocl_program, "compare_buffers_long", &error );
#endif
		CL_ERR_return ( "Init: Failed to create the compare kernel." );

		ocl_kernel_compare_bgra = dclCreateKernel ( ocl_program, "compare_buffers_bgra", &error );
		//
		CL_ERR_return ( "Init: Failed to create the compare bgra buffer kernel." );

		ocl_kernel_compare_texture = dclCreateKernel ( ocl_program, "compare_texture_buffer", &error );
		//
		CL_ERR_return ( "Init: Failed to create the compare texture/buffer kernel." );


		ocl_kernel_rotate = dclCreateKernel ( ocl_program, "imageRotateARGB", &error );
		//
		CL_ERR_return ( "Init: Failed to create the rotate kernel." );


		ocl_kernel_resize = dclCreateKernel ( ocl_program, "imageResizeARGB", &error );
		//
		CL_ERR_return ( "Init: Failed to create the resize kernel." );


		ocl_kernel_copy = dclCreateKernel ( ocl_program, "imageCopyARGB", &error );
		//
		CL_ERR_return ( "Init: Failed to create the copy kernel." );


#ifdef USE_LINEAR_YUV
		ocl_kernel_2yuv = dclCreateKernel ( ocl_program, "imageARGB2YUV1D", &error );
#else
		ocl_kernel_2yuv = dclCreateKernel ( ocl_program, "imageARGB2YUV2D", &error );
#endif
		CL_ERR_return ( "Init: Failed to create the rgb2yuv kernel." );

#ifdef USE_LINEAR_YUV
		ocl_kernel_2nv12 = dclCreateKernel ( ocl_program, "imageARGB2NV121D", &error );
#else
		ocl_kernel_2nv12 = dclCreateKernel ( ocl_program, "imageARGB2NV122D", &error );
#endif
		CL_ERR_return ( "Init: Failed to create the rgb2nv12 kernel." );

		// Command-queue
		ocl_queue = dclCreateCommandQueue ( ocl_context, ocl_devices [ ocl_next_cqueue ], 0, &error );
		//ocl_queues [ i ] = dclCreateCommandQueue ( ocl_context, ocl_devices [ i ], 0, &error );
		if ( error != CL_SUCCESS ) {
			CErrArgID ( "Init: Failed to create command queue for device [%d]. Error [%s]", ocl_next_cqueue, OCLERR );
			return false;
		}

		ocl_next_cqueue++;
		if ( ocl_next_cqueue >= ocl_devices_count )
			ocl_next_cqueue = 0;

		ocl_next_cqueue++;
		if ( ocl_next_cqueue >= ocl_devices_count )
			ocl_next_cqueue = 0;

		if ( !ocl_variable_eq ) {
			ocl_variable_eq = dclCreateBuffer ( ocl_context, CL_MEM_READ_WRITE, sizeof ( unsigned int ), 0, &error );
			//
			CL_ERR_return ( "Init: Failed to create result variable for comparison." );


			error = dclSetKernelArg ( ocl_kernel_compare, 2, sizeof ( cl_mem ), &ocl_variable_eq );
			error |= dclSetKernelArg ( ocl_kernel_compare_bgra, 2, sizeof ( cl_mem ), &ocl_variable_eq );
			error |= dclSetKernelArg ( ocl_kernel_compare_texture, 2, sizeof ( cl_mem ), &ocl_variable_eq );
			//
			CL_ERR_return ( "Init: Failed to set kernel argument for result variable for comparison." );
		}

		return true;
	}


	bool RenderOpenCL::MainThreadedInit ()
	{
		CVerbArg ( "MainThreadedInit device [%d]", deviceID );

		return true;
	}


	bool RenderOpenCL::MainThreadedDispose ()
	{
		CVerbArg ( "MainThreadedDispose device [%d]", deviceID );

		return true;
	}


	void RenderOpenCL::Dispose ()
	{
		CVerbID ( "Dispose" );

		oclUnmapBuffer ( ocl_mapped_ptr, ocl_buffer_square );

		ocl_buffer_square_size = 0;

		oclReleaseMem ( ocl_image_rotated );
		oclReleaseMem ( ocl_image_output );
		oclReleaseMem ( ocl_image_yuv );
		oclReleaseMem ( ocl_buffer_square );
		oclReleaseMem ( ocl_buffer_square_cache );

		oclReleaseMem ( ocl_variable_eq );

		oclReleaseQueue ( ocl_queue );

		oclReleaseKernel ( ocl_kernel_compare );
		oclReleaseKernel ( ocl_kernel_compare_bgra );
		oclReleaseKernel ( ocl_kernel_compare_texture );
		oclReleaseKernel ( ocl_kernel_rotate );
		oclReleaseKernel ( ocl_kernel_resize );
		oclReleaseKernel ( ocl_kernel_copy );
		oclReleaseKernel ( ocl_kernel_2yuv );
		oclReleaseKernel ( ocl_kernel_2nv12 );

		bufferedFlushCount	= 0;
	}


	char *	RenderOpenCL::GetBuffer ( RenderContext * context )
	{
#ifdef USE_OCL_DIRECT_YUV_BUFFER
		if ( !context->mapped_ptr ) {
#else
		if ( !context->renderedData ) {
#endif
			CVerbID ( "GetBuffer: Invalid buffer in portalContext" );
			return 0;
		}

#ifdef USE_OCL_DIRECT_YUV_BUFFER
		return context->mapped_ptr;
#else
		return ( char * ) context->renderedData;
#endif
	}


	void printImageInfo ( cl_mem cl_image )
	{
		size_t size = sizeof ( size_t );
		size_t retSize = 0;
		size_t width = 0;

		cl_int error = dclGetImageInfo ( cl_image,
			CL_IMAGE_WIDTH,
			size,
			&width,
			&retSize );
		if ( error != CL_SUCCESS )
			CErrArg ( "printImageInfo: Failed to get image width [%s]", OCLERR );
		else
			CLogArg ( "printImageInfo: Image width [%u]", ( unsigned int ) width );

		error = dclGetImageInfo ( cl_image,
			CL_IMAGE_HEIGHT,
			size,
			&width,
			&retSize );
		if ( error != CL_SUCCESS )
			CErrArg ( "printImageInfo: Failed to get image height [%s]", OCLERR );
		else
			CLogArg ( "printImageInfo: Image height [%u]", ( unsigned int ) width );

		error = dclGetImageInfo ( cl_image,
			CL_IMAGE_DEPTH,
			size,
			&width,
			&retSize );
		if ( error != CL_SUCCESS )
			CErrArg ( "printImageInfo: Failed to get image depth [%s]", OCLERR );
		else
			CLogArg ( "printImageInfo: Image depth [%u]", ( unsigned int ) width );

		error = dclGetImageInfo ( cl_image,
			CL_IMAGE_ELEMENT_SIZE,
			size,
			&width,
			&retSize );
		if ( error != CL_SUCCESS )
			CErrArg ( "printImageInfo: Failed to get image element size [%s]", OCLERR );
		else
			CLogArg ( "printImageInfo: Image row element size [%u]", ( unsigned int ) width );

		error = dclGetImageInfo ( cl_image,
			CL_IMAGE_ROW_PITCH,
			size,
			&width,
			&retSize );
		if ( error != CL_SUCCESS )
			CErrArg ( "printImageInfo: Failed to get image row stride [%s]", OCLERR );
		else
			CLogArg ( "printImageInfo: Image row stride [%u]", ( unsigned int ) width );

		cl_image_format format;

		error = dclGetImageInfo ( cl_image,
			CL_IMAGE_FORMAT,
			sizeof ( cl_image_format ),
			&format,
			&retSize );
		if ( error != CL_SUCCESS )
			CErrArg ( "printImageInfo: Failed to get image format [%s]", OCLERR );
		else
			CLogArg ( "printImageInfo: Image format: order[%s] - data type [%s]", oclChannelOrder ( format.image_channel_order ), oclChannelDataType ( format.image_channel_data_type ) );
	}


	int RenderOpenCL::AllocateResources ( RenderDimensions * dims )
	{
		unsigned int	ws;
		cl_int			error;
		unsigned int	portal_width, portal_heigth, mod;

		// Allocate texture2D to be handed over to opencl
		IPortalCapture * cap = ( ( WorkerStages * ) stages )->capture;

		if ( cap->bufferType == CaptureBufferType::Texture3D ) // We need texture only for Direct3D
		{
			error = CL_SUCCESS;

			while ( !ocl_texture2D ) {
#if defined(WINDOWS_8) && defined(ENABLE_WIND3D_CAPTURE)
				ocl_texture2D = dclCreateFromD3D11Texture2D ( ocl_context, CL_MEM_READ_ONLY, ( ID3D11Texture2D * ) cap->data, 0, &error );
				//
				CL_ERR_break ( "AllocateResources: Failed to create ocl memory object from D3DTexture2D" );
#endif
				printImageInfo ( ocl_texture2D );

				/* if ( error != CL_SUCCESS )
				CErrArg ( "AllocateResources: Failed to get memory buffer of texture2D [%s]", OCLERR ); */

				error = dclSetKernelArg ( ocl_kernel_compare_texture, 0, sizeof ( cl_mem ), &ocl_texture2D ); // set the square buffer as input for comparison
																											  //
				CL_ERR_break ( "AllocateResources: Failed to set D3D texture2D to kernel compare arg" );


				error = dclFinish ( ocl_queue );
				//
				CL_ERR_break ( "AllocateResources: Wait/Finish failed to enqueue 2D D3D texture to OCL" );
				break;
			}

			if ( error != CL_SUCCESS ) {
				// If texture2D is not accepted by the OCL-Implementation, then we map the memory to the texture and establish access through CPU
				oclReleaseMem ( ocl_texture2D );
				cap->ReleaseResources ();

				cap->bufferType = CaptureBufferType::PixelBuffer3D;
			}
		}

		// Allocate texture2D mapped to host memory
		if ( cap->bufferType == CaptureBufferType::PixelBuffer3D )
		{
			int captureRC = cap->AllocateResources ( dims );
			if ( captureRC <= 0 ) // Something went wrong while allocating neccessary buffers
				return captureRC;

			if ( !cap->data || !cap->dataSize || !cap->dataStride ) // If not, then we failed to map texture buffer for cpu access!
				return captureRC;

			// Bytes / 4 = Pixel
			unsigned int stride = cap->dataStride / 8;
			error = dclSetKernelArg ( ocl_kernel_compare_bgra, 4, sizeof ( unsigned int ), &stride );
			stride = dims->square / 2;
			error |= dclSetKernelArg ( ocl_kernel_compare_bgra, 5, sizeof ( unsigned int ), &stride );
			//
			CL_ERR_return ( "AllocateResources: Failed to set D3D texture2D to kernel compare arg" );
		}

		do
		{
			unsigned int size = dims->square * dims->square * 4;

			if ( !ocl_buffer_square ) {
				CLogArgID ( "AllocateResources: Recreating capture resources for size [%u]", size );

				if ( cap->bufferType == CaptureBufferType::PixelBuffer3D )
					ocl_buffer_square = dclCreateBuffer ( ocl_context, CL_MEM_READ_ONLY, cap->dataSize, 0, &error );
				else
					ocl_buffer_square = dclCreateBuffer ( ocl_context, CL_MEM_READ_ONLY, size, 0, &error );

				error |= dclSetKernelArg ( ocl_kernel_compare, 0, sizeof ( cl_mem ), &ocl_buffer_square ); // set the square buffer as input for comparison
																										   //error |= dclSetKernelArg ( ocl_kernel_rotate, 0, sizeof(cl_mem), &ocl_buffer_square ); // set the cache as input for rotation
				error |= dclSetKernelArg ( ocl_kernel_compare_bgra, 0, sizeof ( cl_mem ), &ocl_buffer_square ); // set the square buffer as input for comparison
																												//
				CL_ERR_break_opt ( "AllocateResources: Failed to initialize square buffer.", oclReleaseMem ( ocl_buffer_square ); );
			}

			if ( !ocl_buffer_square_cache ) {
				ocl_buffer_square_cache = dclCreateBuffer ( ocl_context, CL_MEM_READ_WRITE, size, 0, &error );
				error |= dclSetKernelArg ( ocl_kernel_rotate, 0, sizeof ( cl_mem ), &ocl_buffer_square_cache ); // set the cache as input for rotation

				error |= dclSetKernelArg ( ocl_kernel_compare, 1, sizeof ( cl_mem ), &ocl_buffer_square_cache ); // set the cache as object to compare with (and implicit copy to)
				error |= dclSetKernelArg ( ocl_kernel_compare_bgra, 1, sizeof ( cl_mem ), &ocl_buffer_square_cache ); // set the cache as input for rotation
																													  // TODO!! This is the reason for flickering...
																													  //error |= dclSetKernelArg ( ocl_kernel_rotate, 0, sizeof(cl_mem), &ocl_buffer_square ); // set the cache as input for rotation
				if ( ocl_texture2D ) {
					error |= dclSetKernelArg ( ocl_kernel_compare_texture, 1, sizeof ( cl_mem ), &ocl_buffer_square_cache ); // set the cache as object to compare with (and implicit copy to)
					error |= dclSetKernelArg ( ocl_kernel_rotate, 0, sizeof ( cl_mem ), &ocl_buffer_square_cache ); // set the cache as input for rotation
				}
				//
				CL_ERR_break_opt ( "AllocateResources: Failed to initialize square cache buffer.", oclReleaseMem ( ocl_buffer_square_cache ); );
			}

			if ( ocl_buffer_square_size == 0 ) {
				// Set work group sizes
				ws = dims->square / 2;
				mod = ws % 16;
				if ( mod )
					ws += 16 - mod;
				ocl_square_compare_global_ws [ 0 ] = ws;

				ws = dims->square;
				mod = ws % 16;
				if ( mod )
					ws += 16 - mod;
				ocl_square_rotate_global_ws [ 0 ] = ws;

				// ws must also be a multiple of the local work group size
#ifdef GLOBAL_WORK_GROUP_SIZE_FIX
				ws = dims->square;
				mod = ws % ocl_local_ws_16_32 [ 1 ];
				if ( mod )
					ws += ( unsigned int ) ocl_local_ws_16_32 [ 1 ] - mod;
				ocl_square_rotate_global_ws [ 1 ] = ws;
#else
				ws = dims->square;
				mod = ws % 32;
				if ( mod )
					ws += 32 - mod;
				ocl_square_rotate_global_ws [ 1 ] = ws;
				ocl_square_compare_global_ws [ 1 ] = ws;
#endif		
			}
			ocl_buffer_square_size = size;

#ifdef _WIN32
			if ( ocl_buffer_square_size < cap->dataSize ) {
				CErrArgID ( "AllocateResources: Invalid buffer size of frameSize [%i]. CL buffer too small [%i].", ocl_buffer_square_size, cap->dataSize );
				break;
			}
#endif

			//		parentSource->UpdateRenderContextDimensions ( portal_width, portal_heigth );

			portal_width = dims->streamWidth; portal_heigth = dims->streamHeight;

			if ( !ocl_image_output )
			{
				unsigned int destSize = portal_width * portal_heigth;
				ocl_image_output = dclCreateBuffer ( ocl_context, CL_MEM_READ_WRITE, destSize * 4, 0, &error );

				error |= dclSetKernelArg ( ocl_kernel_resize, 1, sizeof ( cl_mem ), &ocl_image_output ); // set the output image as destination for resizing
				error |= dclSetKernelArg ( ocl_kernel_resize, 2, sizeof ( unsigned int ), &portal_width );
				error |= dclSetKernelArg ( ocl_kernel_resize, 3, sizeof ( unsigned int ), &portal_heigth );
				error |= dclSetKernelArg ( ocl_kernel_resize, 4, sizeof ( unsigned int ), &destSize );

				error |= dclSetKernelArg ( ocl_kernel_copy, 1, sizeof ( cl_mem ), &ocl_image_output ); // set the output image as destination for copying
				error |= dclSetKernelArg ( ocl_kernel_copy, 2, sizeof ( unsigned int ), &portal_width );
				error |= dclSetKernelArg ( ocl_kernel_copy, 3, sizeof ( unsigned int ), &portal_width );
				error |= dclSetKernelArg ( ocl_kernel_copy, 4, sizeof ( unsigned int ), &portal_heigth );
				error |= dclSetKernelArg ( ocl_kernel_copy, 5, sizeof ( unsigned int ), &destSize );

				error |= dclSetKernelArg ( ocl_kernel_2yuv, 0, sizeof ( cl_mem ), &ocl_image_output ); // set the output image as destination for copying
				error |= dclSetKernelArg ( ocl_kernel_2nv12, 0, sizeof ( cl_mem ), &ocl_image_output ); // set the output image as destination for copying
																									   //
				CL_ERR_break_opt ( "AllocateResources: Failed to initialize resize output buffer.", oclReleaseMem ( ocl_image_output ); );

				ocl_image_output_size = destSize * 4;

				ws = portal_width;
				mod = ws % 16;
				if ( mod )
					ws += 16 - mod;
				ocl_output_global_ws [ 0 ] = ws;
				//else

				ws = portal_heigth;
				mod = ws % 16;
				if ( mod )
					ws += 16 - mod;
				ocl_output_global_ws [ 1 ] = ws;
			}

			if ( ( outputType == PortalBufferType::YUV420 || outputType == PortalBufferType::NV12 ) && !ocl_image_yuv )
			{
#ifdef USE_LINEAR_YUV
				ocl_image_yuv_local_ws1 = ( unsigned int ) ocl_work_item_max_0;

				unsigned int ySize = portal_width * portal_heigth;
				unsigned int destSize = ySize + ( ySize >> 1 );
#else
				unsigned int destSize = ( portal_width * portal_heigth ) + ( ( portal_width * portal_heigth ) / 2 );
#endif

#ifdef	USE_OCL_DIRECT_YUV_BUFFER
				ocl_image_yuv = dclCreateBuffer ( ocl_context, CL_MEM_ALLOC_HOST_PTR, destSize, 0, &error );
#else
				ocl_image_yuv = dclCreateBuffer ( ocl_context, CL_MEM_READ_WRITE, destSize, 0, &error );
#endif

				error |= dclSetKernelArg ( ocl_kernel_2yuv, 1, sizeof ( cl_mem ), &ocl_image_yuv ); // set the output image as destination for resizing
#ifdef USE_LINEAR_YUV
				ySize = ( portal_width >> 2 ) * portal_heigth;
				error |= dclSetKernelArg ( ocl_kernel_2yuv, 2, sizeof ( unsigned int ), &ySize );

				unsigned int uvWidth = portal_width / 8;
				error |= dclSetKernelArg ( ocl_kernel_2yuv, 3, sizeof ( unsigned int ), &uvWidth );
#else
				error |= dclSetKernelArg ( ocl_kernel_2yuv, 2, sizeof ( unsigned int ), &portal_heigth );
				error |= dclSetKernelArg ( ocl_kernel_2yuv, 3, sizeof ( unsigned int ), &portal_width );

				unsigned int destStride = portal_width / 4;
				error |= dclSetKernelArg ( ocl_kernel_2yuv, 4, sizeof ( unsigned int ), &destStride );
#endif
				CL_ERR_break_opt ( "AllocateResources: Failed to initialize YUV buffer.", oclReleaseMem ( ocl_image_yuv ); );

				error |= dclSetKernelArg ( ocl_kernel_2nv12, 1, sizeof ( cl_mem ), &ocl_image_yuv ); // set the output image as destination for resizing
#ifdef USE_LINEAR_YUV
				ySize = ( portal_width >> 2 ) * portal_heigth;
				error |= dclSetKernelArg ( ocl_kernel_2nv12, 2, sizeof ( unsigned int ), &ySize );

				uvWidth = portal_width / 4;
				error |= dclSetKernelArg ( ocl_kernel_2nv12, 3, sizeof ( unsigned int ), &uvWidth );
#else
				error |= dclSetKernelArg ( ocl_kernel_2nv12, 2, sizeof ( unsigned int ), &portal_heigth );
				error |= dclSetKernelArg ( ocl_kernel_2nv12, 3, sizeof ( unsigned int ), &portal_width );

				destStride = portal_width / 4;
				error |= dclSetKernelArg ( ocl_kernel_2nv12, 4, sizeof ( unsigned int ), &destStride );
#endif
				CL_ERR_break_opt ( "AllocateResources: Failed to initialize NV12 buffer.", oclReleaseMem ( ocl_image_yuv ); );

				ocl_image_yuv_size = destSize;

				// calculate yuv first dimension worksize
#ifdef USE_LINEAR_YUV
				if ( outputType == PortalBufferType::YUV420 )
				{
					ws = ySize + ( ySize >> 2 );
				}
				else
				{
					ws = ySize + ( ySize >> 1 );
				}
				
				mod = ws % ocl_work_item_max_0;
				if ( mod )
					ws += ( unsigned int ) ocl_work_item_max_0 - mod;
				ocl_image_yuv_global_ws1 = ws;
#else
				ws = portal_width / 4;
				ws += ws / 2;
				mod = ws % 16;
				if ( mod )
					ws += 16 - mod;
				ocl_image_yuv_global_ws1 = ws;
#endif
			}

			if ( !ocl_image_rotated ) {
				ocl_image_rotated = dclCreateBuffer ( ocl_context, CL_MEM_READ_WRITE, size, 0, &error );
				error |= dclSetKernelArg ( ocl_kernel_rotate, 1, sizeof ( cl_mem ), &ocl_image_rotated ); // set rotated image as output for rotation
				error |= dclSetKernelArg ( ocl_kernel_resize, 0, sizeof ( cl_mem ), &ocl_image_rotated ); // set rotated image as input for resizing (and cropping)
																										  //error |= dclSetKernelArg ( ocl_kernel_resize, 0, sizeof(cl_mem), &ocl_buffer_square_cache ); // set rotated image as input for resizing (and cropping)

				error |= dclSetKernelArg ( ocl_kernel_resize, 5, sizeof ( unsigned int ), &dims->xOffset );
				error |= dclSetKernelArg ( ocl_kernel_resize, 6, sizeof ( unsigned int ), &dims->yOffset );
				error |= dclSetKernelArg ( ocl_kernel_copy, 6, sizeof ( unsigned int ), &dims->xOffset );
				error |= dclSetKernelArg ( ocl_kernel_copy, 7, sizeof ( unsigned int ), &dims->yOffset );

				error |= dclSetKernelArg ( ocl_kernel_resize, 7, sizeof ( unsigned int ), &dims->square );
				error |= dclSetKernelArg ( ocl_kernel_resize, 8, sizeof ( unsigned int ), &dims->width_cap );
				error |= dclSetKernelArg ( ocl_kernel_resize, 9, sizeof ( unsigned int ), &dims->height_cap );
				error |= dclSetKernelArg ( ocl_kernel_copy, 8, sizeof ( unsigned int ), &dims->square );
				error |= dclSetKernelArg ( ocl_kernel_copy, 9, sizeof ( unsigned int ), &dims->width_cap );
				error |= dclSetKernelArg ( ocl_kernel_copy, 10, sizeof ( unsigned int ), &dims->height_cap );


				float x_ratio = ( ( float ) ( dims->width_cap - 1 ) ) / portal_width;
				float y_ratio = ( ( float ) ( dims->height_cap - 1 ) ) / portal_heigth;
				error |= dclSetKernelArg ( ocl_kernel_resize, 11, sizeof ( float ), &x_ratio );
				error |= dclSetKernelArg ( ocl_kernel_resize, 12, sizeof ( float ), &y_ratio );

				error |= dclSetKernelArg ( ocl_kernel_copy, 0, sizeof ( cl_mem ), &ocl_image_rotated ); // set rotated image as input for copy (and cropping)

				int center = dims->square / 2;
				error |= dclSetKernelArg ( ocl_kernel_rotate, 2, sizeof ( unsigned int ), &dims->square );
				error |= dclSetKernelArg ( ocl_kernel_rotate, 3, sizeof ( unsigned int ), &dims->square );
				error |= dclSetKernelArg ( ocl_kernel_rotate, 4, sizeof ( int ), &center );
				error |= dclSetKernelArg ( ocl_kernel_rotate, 5, sizeof ( int ), &center );
				//
				CL_ERR_break_opt ( "AllocateResources: Failed to initialize rotate buffer.", oclReleaseMem ( ocl_image_rotated ); );
			}

#ifdef USE_LINEAR_COMPARE
			unsigned int longs = ocl_buffer_square_size / 8;
			error = dclSetKernelArg ( ocl_kernel_compare, 3, sizeof ( unsigned int ), &longs );
			error |= dclSetKernelArg ( ocl_kernel_compare_bgra, 3, sizeof ( unsigned int ), &longs );
			error |= dclSetKernelArg ( ocl_kernel_compare_texture, 3, sizeof ( unsigned int ), &longs );

			longs = dims->square / 2;
			error |= dclSetKernelArg ( ocl_kernel_compare_texture, 4, sizeof ( unsigned int ), &longs );
#else
			unsigned int width_longs = square / 2;
			error |= dclSetKernelArg ( ocl_kernel_compare, 3, sizeof ( unsigned int ), &width_longs );
			error |= dclSetKernelArg ( ocl_kernel_compare, 4, sizeof ( unsigned int ), &square );
#endif
			unsigned int size_4 = size / 4;

			error |= dclSetKernelArg ( ocl_kernel_rotate, 8, sizeof ( unsigned int ), &size_4 );
			error |= dclSetKernelArg ( ocl_kernel_resize, 10, sizeof ( unsigned int ), &size_4 );
			error |= dclSetKernelArg ( ocl_kernel_copy, 11, sizeof ( unsigned int ), &size_4 );
			error |= dclFinish ( ocl_queue );
			//
			CL_ERR_break ( "AllocateResources: Failed to set kernel arguments with error" );

			return 1;
		}
		while ( 0 );

		return 0;
	}


	int RenderOpenCL::ReleaseResources ( RenderContext * context )
	{
		CLogID ( "ReleaseResources" );

		oclReleaseMem ( ocl_texture2D );

		ocl_buffer_square_size = 0;

		oclReleaseMem ( ocl_buffer_square );
		oclReleaseMem ( ocl_buffer_square_cache );
		oclReleaseMem ( ocl_image_rotated );

		buffersInitialized = false;
		return 1;
	}


	int RenderOpenCL::UpdateBuffers ( RenderDimensions * dims, RenderContext * context )
	{
		//CLog ( "Perform" );
		CVerbArgID ( "UpdateBuffers: Updating buffers for context [%u]", context->id );

		if ( dims->square != squareLength ) {
			oclReleaseMem ( ocl_texture2D );

			ocl_buffer_square_size = 0;

			oclReleaseMem ( ocl_buffer_square );
			oclReleaseMem ( ocl_buffer_square_cache );
			oclReleaseMem ( ocl_image_rotated );

			buffersInitialized = false;
		}

		int ret = AllocateResources ( dims );
		if ( ret <= 0 )
			return ret;

		// Now check the buffer of the render context
		unsigned int bufferSizeOutput = ( ( outputType == PortalBufferType::YUV420 || outputType == PortalBufferType::NV12 ) ? ocl_image_yuv_size : ocl_image_output_size );

		char * buffer = ( char * ) context->renderedData;
		if ( buffer ) {
			unsigned int bufferSize = context->renderedDataCapacity;

			if ( bufferSize != bufferSizeOutput ) {
				context->renderedData = 0;
				free ( buffer );
				buffer = 0;
			}
		}

		if ( !buffer ) {
			if ( !bufferSizeOutput ) {
				CErrArgID ( "UpdateBuffers: Targeted buffer size is 0! Failed while allocating memory for yuv output buffer of device [%d]. Size of yuv buffer [%u]", deviceID, bufferSizeOutput );
				return -1;
			}

			buffer = ( char * ) malloc ( bufferSizeOutput );
			if ( !buffer ) {
				CErrArgID ( "Perform: Failed to allocate memory for output buffer for yuv data. Size of yuv buffer [%u]", bufferSizeOutput );
				return 0;
			}
			context->renderedDataCapacity = bufferSizeOutput;
			context->renderedData = buffer;

			context->width = dims->streamWidth;
			context->height = dims->streamHeight;

			if ( outputType == PortalBufferType::ARGB )
			{
				context->stride = dims->streamWidth * 4;
			}
			else
			{
				context->stride = dims->streamWidth;
			}

		}

		return 1;
	}


	//
	//
	// capture 8ms
	// memory copy 16ms
	//
	bool RenderOpenCL::Perform ( RenderDimensions * dims, RenderContext * context )
	{
		//CLog ( "Perform" );
		CVerbArgID ( "Perform: Rendering portal context [%u]", context->id );

		cl_int		error;

		do
		{
#ifndef ENABLE_WORKER_STAGES_COMPARE
			//
			// Memory compare of pixel values
			//
			COMPARE_CACHE ( equal );
#endif

			//float theta = (float) (((orientation - 90) * PI) / 180);
			//float theta = 0;
			/*if (!inContact) {
			theta += (deviceAzimut - deviceAzimutLast);
			}*/
			//float theta = (orientation * PI) / 180;

			//float theta = 0 - (orientation - 90);
			// Converted to radians
			float theta = ( float ) ( 0 - ( dims->orientation - 90 ) * PI ) / 180;

			float cosTheta = cos ( theta );
            float sinTheta = sin ( theta );
            //error = 0;
			/*error |=*/ dclSetKernelArg ( ocl_kernel_rotate, 6, sizeof ( float ), &sinTheta );
			/*error |=*/ dclSetKernelArg ( ocl_kernel_rotate, 7, sizeof ( float ), &cosTheta );

			// Launching rotate kernel
			error = dclEnqueueNDRangeKernel ( ocl_queue, ocl_kernel_rotate, 2, NULL, ocl_square_rotate_global_ws, ocl_local_ws_16_32, 0, NULL, 0 );
			//
			CL_ERR_break_dev ( "Perform: Failed to enqueue the rotate kernel." );


#ifdef PERFORMANCECOUNT_CAPTURE		
			error = dclFinish ( ocl_queue );
			//
			CL_ERR_break_dev ( "Perform: Failed to execute the rotate kernel." );
			pc_doCount ( 5, true, "Rotate" );
#endif	

			// Launching resize kernel (TODO!!! check whether we can use the 5x faster copy kernel) 
			error = dclEnqueueNDRangeKernel ( ocl_queue, ocl_kernel_resize, 2, NULL, ocl_output_global_ws, ocl_local_ws_16_16, 0, 0, 0 );
			//
			CL_ERR_break_dev ( "Perform: Failed to enqueue the resize kernel." );

#ifdef PERFORMANCECOUNT_CAPTURE		
			error = dclFinish ( ocl_queue );
			//
			CL_ERR_break_dev ( "Perform: Failed to execute the resize kernel." );
			pc_doCount ( 6, true, "Resize" );
#endif	

			if ( outputType == PortalBufferType::ARGB ) {
				error = dclEnqueueReadBuffer ( ocl_queue, ocl_image_output, CL_TRUE, 0, ocl_image_output_size, ( char * ) context->renderedData, 0, 0, 0 );
				//
				CL_ERR_break_dev ( "Perform: Failed to enqueue the ARGB read buffer." );
			}
			else {
				cl_kernel kernel;

				if ( outputType == PortalBufferType::YUV420 ) {
					kernel = ocl_kernel_2yuv;
				}
				else {
					kernel = ocl_kernel_2nv12;
				}

#ifdef USE_LINEAR_YUV		
				// Launch RGB -> YUV conversion	
				error = dclEnqueueNDRangeKernel ( ocl_queue, kernel, 1, NULL, &ocl_image_yuv_global_ws1, &ocl_image_yuv_local_ws1, 0, 0, 0 );
#else
                //                const size_t global_ws_yuv [ ] = { ocl_image_yuv_global_ws1, ocl_image_output_height };
                const size_t global_ws_yuv [ ] = { ocl_image_yuv_global_ws1, (size_t) dims->streamHeight };

				// Launch RGB -> YUV conversion	
				error = dclEnqueueNDRangeKernel ( ocl_queue, kernel, 2, NULL, global_ws_yuv, ocl_local_ws_16_16, 0, NULL, NULL );
#endif
				//
				CL_ERR_break_dev ( "Perform: Failed to enqueue the YUV kernel." );

#ifdef PERFORMANCECOUNT_CAPTURE		
				error = dclFinish ( ocl_queue );
				//
				CL_ERR_break_dev ( "Perform: Failed to execute the queued commands." );

				pc_doCount ( 8, true, "YUV" );
#endif	
				// Read buffer
#ifdef USE_OCL_DIRECT_YUV_BUFFER
				// Try: using mapped buffer and map it multiple times
				if ( pack->mapped_ptr ) {
					dclEnqueueUnmapMemObject ( ocl_queue, ocl_image_yuv, pack->mapped_ptr, 0, 0, 0 );
				}

				pack->mapped_ptr = dclEnqueueMapBuffer ( ocl_queue, ocl_image_yuv, CL_TRUE, CL_MAP_READ, 0, ocl_image_yuv_size, 0, 0, 0, &error );
				error |= dclFinish ( ocl_queue );
				if ( error != CL_SUCCESS || !pack->mapped_ptr ) {
					CErrArg ( "Perform: Failed to get a pointer to yuv buffer [%s]", OCLERR );
					break;
				}
#else
				error = dclEnqueueReadBuffer ( ocl_queue, ocl_image_yuv, CL_TRUE, 0, ocl_image_yuv_size, ( char * ) context->renderedData, 0, 0, 0 );
				//
				CL_ERR_break_dev ( "Perform: Failed to enqueue the YUV420 read buffer." );
			}

			error = dclFinish ( ocl_queue );
			//
			CL_ERR_break_dev ( "Perform: Failed to complete the kernels." );
#endif

			return true;
		}
		while ( 0 );

		return false;
	}


	bool RenderOpenCL::Compare ( unsigned int &equal )
	{
		//CLog ( "Compare" );

		cl_int	error;
		bool	ret		= false;
		equal	= false;

		//
		// Memory compare of pixel values
		//
		do {
			int value = 1;
			error = dclEnqueueWriteBuffer ( ocl_queue, ocl_variable_eq, CL_TRUE, 0, sizeof ( unsigned int ), &value, 0, 0, 0 );
			//
			CL_ERR_break_dev ( "Compare: reseting equality status failed." );

#ifdef OCL_PERFORMANCE_COMPARE
			pc_doCount ( 25, true, "Start of compare" );
#endif
			IPortalCapture * cap = ( ( WorkerStages * ) stages )->capture;

			if ( cap->bufferType == CaptureBufferType::Texture3D ) {
				if ( ocl_texture2D ) {
#ifdef _WIN32
					error = dclEnqueueAcquireD3D11Objects ( ocl_queue, 1, &ocl_texture2D, 0, NULL, NULL );
					//
					CL_ERR_break_dev ( "Compare: Failed to enqueue D3D texture2D to OCL." );
#endif
				}
				else break;
			}
			else {
				if ( cap->bufferType == CaptureBufferType::PixelBuffer3D )
					error = dclEnqueueWriteBuffer ( ocl_queue, ocl_buffer_square, CL_TRUE, 0, cap->dataSize, cap->data, 0, 0, 0 );
				else
					error = dclEnqueueWriteBuffer ( ocl_queue, ocl_buffer_square, CL_TRUE, 0, ocl_buffer_square_size, cap->data, 0, 0, 0 );
				//
				CL_ERR_break_dev ( "Compare: Failed to upload pixels to device." );
			}

#ifdef OCL_PERFORMANCE_COMPARE
			error = dclFinish ( ocl_queue );
			//
			CL_ERR_break_dev ( "Compare: clFinish wait for buffer upload failed." );

			pc_doCount ( 26, true, "GetDIBits to gpu buffer" );
#endif			
			// Launching kernel
#ifdef USE_LINEAR_COMPARE
			size_t	ocl_local_ws_512 = ocl_work_item_max_0;
			size_t	ocl_global_ws = ocl_buffer_square_size / 8;

			size_t mod = ocl_global_ws % ocl_work_item_max_0;
			if ( mod )
				ocl_global_ws += ocl_work_item_max_0 - mod;

			cl_kernel compare_kernel;
			switch ( cap->bufferType )
			{
			case CaptureBufferType::PixelBuffer:
				compare_kernel = ocl_kernel_compare;
				break;
			case CaptureBufferType::Texture3D:
				compare_kernel = ocl_kernel_compare_texture;
				break;
			case CaptureBufferType::PixelBuffer3D:
				compare_kernel = ocl_kernel_compare_bgra;
				break;
			default:
				return false;
			}
			error = dclEnqueueNDRangeKernel ( ocl_queue, compare_kernel, 1, NULL, &ocl_global_ws, &ocl_local_ws_512, 0, 0, 0 );
#else
			error = dclEnqueueNDRangeKernel ( ocl_queue, ocl_kernel_compare, 2, NULL, ocl_square_compare_global_ws, ocl_local_ws_16_32, 0, 0, &0 );
#endif
			CL_ERR_break_dev ( "Compare: Failed to launch the compare kernel." );


			error |= dclEnqueueReadBuffer ( ocl_queue, ocl_variable_eq, CL_TRUE, 0, sizeof ( unsigned int ), &equal, 0, 0, 0 );
			//
			CL_ERR_break_dev ( "Compare: Failed to enqueue equality ouput buffer." );

#ifdef _WIN32
			if ( ocl_texture2D ) {
				error |= dclEnqueueReleaseD3D11Objects ( ocl_queue, 1, &ocl_texture2D, 0, NULL, NULL );
				//
				CL_ERR_break_dev ( "Compare: Failed to release 2D D3D texture from OCL." );
			}
#endif
			error |= dclFinish ( ocl_queue );
			//
			CL_ERR_break_dev ( "Compare: clFinish wait for comparison failed." );

#ifdef OCL_PERFORMANCE_COMPARE		
			pc_doCount ( 28, true, "Comparison" );
#endif		
			ret = true;
			break;
		}
		while ( 0 );

		return ret;
	}



} /* namespace environs */
    
#endif
    
    
