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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include "Environs.Native.h"
#include "Interop.h"
#include "Dyn.OpenCL.h"
using namespace environs;

#ifndef _WIN32
#   include <dlfcn.h>
#   include <errno.h>
#endif

#define CLASS_NAME  "Dyn.OpenCL . . . . . . ."

#ifdef _WIN32
#   define	LIBNAME		"OpenCL.dll"
#endif

#ifdef __APPLE__
#   define	LIBNAME		"OpenCL.dylib"
#else
#   ifdef __GNUC__
#       define	LIBNAME	"OpenCL.lib.so"
#   endif
#endif


// Disable this flag to use library as statically linked library again
#define USE_DYNAMIC_LIB


namespace environs
{
bool							ocl_LibInitialized		= false;
HMODULE							ocl_Lib						= 0;

pclEnqueueWriteBuffer			dclEnqueueWriteBuffer		= 0;
pclEnqueueNDRangeKernel			dclEnqueueNDRangeKernel		= 0;
pclEnqueueReadBuffer			dclEnqueueReadBuffer		= 0;
pclFinish						dclFinish					= 0;
pclEnqueueUnmapMemObject		dclEnqueueUnmapMemObject	= 0;
pclReleaseMemObject				dclReleaseMemObject			= 0;
pclReleaseCommandQueue			dclReleaseCommandQueue		= 0;
pclCreateCommandQueue			dclCreateCommandQueue		= 0;
pclCreateBuffer					dclCreateBuffer				= 0;
pclSetKernelArg					dclSetKernelArg				= 0;
pclReleaseKernel				dclReleaseKernel			= 0;
pclReleaseContext				dclReleaseContext			= 0;
pclGetPlatformIDs				dclGetPlatformIDs			= 0;
pclGetDeviceIDs					dclGetDeviceIDs				= 0;
pclCreateContext				dclCreateContext			= 0;
pclCreateProgramWithSource		dclCreateProgramWithSource	= 0;
pclReleaseProgram				dclReleaseProgram	= 0;
pclCreateProgramWithBinary		dclCreateProgramWithBinary	= 0;
pclBuildProgram					dclBuildProgram				= 0;
pclGetProgramBuildInfo			dclGetProgramBuildInfo		= 0;
pclGetProgramInfo				dclGetProgramInfo			= 0;
pclGetImageInfo					dclGetImageInfo				= 0;
pclCreateKernel					dclCreateKernel				= 0;
pclGetDeviceInfo				dclGetDeviceInfo			= 0;
pclEnqueueMapBuffer				dclEnqueueMapBuffer			= 0;
pclWaitForEvents				dclWaitForEvents			= 0;

pclGetSupportedImageFormats		dclGetSupportedImageFormats	= 0;
pclGetExtensionFunctionAddress	dclGetExtensionFunctionAddress = 0;
    
#ifdef _WIN32
pclGetDeviceIDsFromD3D11		dclGetDeviceIDsFromD3D11		= 0;
pclCreateFromD3D11Texture2D		dclCreateFromD3D11Texture2D		= 0;
pclEnqueueAcquireD3D11Objects	dclEnqueueAcquireD3D11Objects	= 0;
pclEnqueueReleaseD3D11Objects	dclEnqueueReleaseD3D11Objects	= 0;
unsigned int					ocl_d3d_versup					= 0;
#endif


#ifndef CL_DEVICE_PARTITION_MAX_SUB_DEVICES
#define CL_DEVICE_PARTITION_MAX_SUB_DEVICES 0x1043
#endif


void logOpenCLInfos ( cl_device_id device )
{
	char buffer[1024];
	cl_uint uint_value;
	cl_ulong ulong_value;

	CL_CHECK ( dclGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(buffer), buffer, NULL) );
	CLogArg ( "OCL: DEVICE_NAME = %s", buffer );

	CL_CHECK ( dclGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL) );
	CVerbArg ( "OCL: DEVICE_VENDOR = %s", buffer );

	CL_CHECK ( dclGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL) );
	CVerbArg ( "OCL: DEVICE_VERSION = %s", buffer );

	CL_CHECK ( dclGetDeviceInfo(device, CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL) );
	CVerbArg ( "OCL: DRIVER_VERSION = %s", buffer );

	CL_CHECK ( dclGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(uint_value), &uint_value, NULL) );
	CLogArg ( "OCL: DEVICE_MAX_COMPUTE_UNITS = %u", (unsigned int)uint_value );

	uint_value = 0;
	CL_CHECK ( dclGetDeviceInfo(device, CL_DEVICE_PARTITION_MAX_SUB_DEVICES, sizeof(uint_value), &uint_value, NULL) );
	CVerbArg ( "OCL: CL_DEVICE_PARTITION_MAX_SUB_DEVICES = %u", (unsigned int)uint_value );

	CL_CHECK ( dclGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(uint_value), &uint_value, NULL) );
	CVerbArg ( "OCL: DEVICE_MAX_CLOCK_FREQUENCY = %u", (unsigned int)uint_value );

	CL_CHECK ( dclGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(ulong_value), &ulong_value, NULL) );
	CVerbArg ( "OCL: DEVICE_GLOBAL_MEM_SIZE = %llu", (unsigned long long)ulong_value );
	
	CL_CHECK ( dclGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(uint_value), &uint_value, NULL) );
	CVerbArg ( "OCL: CL_DEVICE_MAX_WORK_GROUP_SIZE = %u", (unsigned int)uint_value );
	
	CL_CHECK ( dclGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(uint_value), &uint_value, NULL) );
	CVerbArg ( "OCL: CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS = %u", (unsigned int)uint_value );

	size_t dims [ 6 ];
	CL_CHECK ( dclGetDeviceInfo ( device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(dims), dims, NULL ) );
	for ( unsigned int i = 0; i < uint_value; i++ ) {
		CVerbArg ( "OCL: CL_DEVICE_MAX_WORK_ITEM_SIZES %i = %u", i, (unsigned int)dims [i] );
	}
	
}


#ifdef USE_DYNAMIC_LIB

bool VerifyLibAccess ()
{
	if ( !dclEnqueueWriteBuffer || !dclEnqueueNDRangeKernel || !dclEnqueueReadBuffer || !dclFinish || !dclGetImageInfo
		|| !dclEnqueueUnmapMemObject || !dclReleaseMemObject || !dclReleaseCommandQueue || !dclCreateCommandQueue || !dclGetSupportedImageFormats
		|| !dclCreateBuffer || !dclSetKernelArg || !dclReleaseKernel || !dclReleaseContext || !dclGetPlatformIDs
		|| !dclGetDeviceIDs || !dclCreateContext || !dclCreateProgramWithSource || !dclBuildProgram || !dclCreateProgramWithBinary
		|| !dclGetProgramBuildInfo || !dclCreateKernel || !dclGetDeviceInfo || !dclEnqueueMapBuffer || !dclGetProgramInfo || !dclWaitForEvents
		) {
		CErr ( "VerifyLibAccess: One of the opencl functions could not be loaded!" );
		return false;
	}
	return true;
}

    
void VerifyDirectXExtension ( )
{
#ifdef _WIN32
	while ( dclGetExtensionFunctionAddress )
	{
		// Try with nvidia extensions first, because sometimes clGetDeviceIDsFromD3D11KHR is available but crashes when a NVDIA device is present
		dclGetDeviceIDsFromD3D11 = reinterpret_cast<pclGetDeviceIDsFromD3D11>( dclGetExtensionFunctionAddress ( "clGetDeviceIDsFromD3D11NV" ) );
		dclCreateFromD3D11Texture2D = reinterpret_cast<pclCreateFromD3D11Texture2D>( dclGetExtensionFunctionAddress ( "clCreateFromD3D11Texture2DNV" ) );
		dclEnqueueAcquireD3D11Objects = reinterpret_cast<pclEnqueueAcquireD3D11Objects>( dclGetExtensionFunctionAddress ( "clEnqueueAcquireD3D11ObjectsNV" ) );
		dclEnqueueReleaseD3D11Objects = reinterpret_cast<pclEnqueueReleaseD3D11Objects>( dclGetExtensionFunctionAddress ( "clEnqueueReleaseD3D11ObjectsNV" ) );

		if ( !dclGetDeviceIDsFromD3D11 || !dclCreateFromD3D11Texture2D || !dclEnqueueAcquireD3D11Objects || !dclEnqueueReleaseD3D11Objects ) {
			CVerb ( "VerifyDirectXExtension: NVIDIA D3D11-OpenCL Interop is NOT supported!" );
		}
		else {
			CVerb ( "VerifyDirectXExtension: NVIDIA D3D11-OpenCL Interop IS supported!" );
			ocl_d3d_versup = 11;
			break;
		}

		dclGetDeviceIDsFromD3D11	= reinterpret_cast<pclGetDeviceIDsFromD3D11>(dclGetExtensionFunctionAddress ( "clGetDeviceIDsFromD3D11KHR" ));
		dclCreateFromD3D11Texture2D	= reinterpret_cast<pclCreateFromD3D11Texture2D>(dclGetExtensionFunctionAddress ( "clCreateFromD3D11Texture2DKHR" ));
		dclEnqueueAcquireD3D11Objects	= reinterpret_cast<pclEnqueueAcquireD3D11Objects>(dclGetExtensionFunctionAddress ( "clEnqueueAcquireD3D11ObjectsKHR" ));
		dclEnqueueReleaseD3D11Objects	= reinterpret_cast<pclEnqueueReleaseD3D11Objects>(dclGetExtensionFunctionAddress ( "clEnqueueReleaseD3D11ObjectsKHR" ));

		if ( !dclGetDeviceIDsFromD3D11 || !dclCreateFromD3D11Texture2D || !dclEnqueueAcquireD3D11Objects  || !dclEnqueueReleaseD3D11Objects ) {
			CVerb ( "VerifyDirectXExtension: KHR D3D11-OpenCL Interop is NOT supported!" );
		}
		else {
			CVerb ( "VerifyDirectXExtension: KHR D3D11-OpenCL Interop IS supported!" );
			ocl_d3d_versup = 11;
			break;
		}

		ocl_d3d_versup = 0;
		break;
	}
#endif
}
    

bool InitOpenCLLib ()
{
	CVerb ( "InitOpenCLLib" );

	if ( ocl_LibInitialized ) {
		CLog ( "InitOpenCLLib: already initialized." );
		return true;
	}
	
	HMODULE				hDLL	= 0;

	hDLL = dlopen ( LIBNAME, RTLD_LAZY );

	if ( !hDLL ) {
#ifdef _WIN32
		CWarnArg ( "InitOpenCLLib: Loading of " LIBNAME " FAILED with error [0x%.8x]", GetLastError () );
#else
		CWarnArg ( "InitOpenCLLib: Loading of " LIBNAME " FAILED with error [0x%.8x] [%s]", errno, dlerror () );
#endif
		return false;
	}


	dclEnqueueWriteBuffer	= ( pclEnqueueWriteBuffer ) dlsym ( hDLL, "clEnqueueWriteBuffer" );
	dclEnqueueNDRangeKernel	= ( pclEnqueueNDRangeKernel ) dlsym ( hDLL, "clEnqueueNDRangeKernel" );
	dclEnqueueReadBuffer	= ( pclEnqueueReadBuffer ) dlsym ( hDLL, "clEnqueueReadBuffer" );
	dclFinish				= ( pclFinish ) dlsym ( hDLL, "clFinish" );

	dclEnqueueUnmapMemObject= ( pclEnqueueUnmapMemObject ) dlsym ( hDLL, "clEnqueueUnmapMemObject" );
	dclReleaseMemObject		= ( pclReleaseMemObject ) dlsym ( hDLL, "clReleaseMemObject" );

	dclReleaseCommandQueue	= ( pclReleaseCommandQueue ) dlsym ( hDLL, "clReleaseCommandQueue" );
	dclCreateCommandQueue	= ( pclCreateCommandQueue ) dlsym ( hDLL, "clCreateCommandQueue" );
	dclCreateBuffer			= ( pclCreateBuffer ) dlsym ( hDLL, "clCreateBuffer" );
	dclSetKernelArg			= ( pclSetKernelArg ) dlsym ( hDLL, "clSetKernelArg" );
	dclReleaseKernel		= ( pclReleaseKernel ) dlsym ( hDLL, "clReleaseKernel" );
	dclReleaseContext		= ( pclReleaseContext ) dlsym ( hDLL, "clReleaseContext" );
	dclGetPlatformIDs		= ( pclGetPlatformIDs ) dlsym ( hDLL, "clGetPlatformIDs" );
	dclGetDeviceIDs			= ( pclGetDeviceIDs ) dlsym ( hDLL, "clGetDeviceIDs" );
	dclCreateContext		= ( pclCreateContext ) dlsym ( hDLL, "clCreateContext" );
	dclBuildProgram			= ( pclBuildProgram ) dlsym ( hDLL, "clBuildProgram" );
	dclGetProgramInfo		= ( pclGetProgramInfo ) dlsym ( hDLL, "clGetProgramInfo" );
	dclGetProgramBuildInfo	= ( pclGetProgramBuildInfo ) dlsym ( hDLL, "clGetProgramBuildInfo" );
	dclGetImageInfo			= ( pclGetImageInfo ) dlsym ( hDLL, "clGetImageInfo" );
	dclCreateKernel			= ( pclCreateKernel ) dlsym ( hDLL, "clCreateKernel" );
	dclGetDeviceInfo		= ( pclGetDeviceInfo ) dlsym ( hDLL, "clGetDeviceInfo" );
	dclEnqueueMapBuffer		= ( pclEnqueueMapBuffer ) dlsym ( hDLL, "clEnqueueMapBuffer" );
	dclWaitForEvents		= ( pclWaitForEvents ) dlsym ( hDLL, "clWaitForEvents" );

	dclGetSupportedImageFormats	= ( pclGetSupportedImageFormats ) dlsym ( hDLL, "clGetSupportedImageFormats" );
	dclCreateProgramWithSource	= ( pclCreateProgramWithSource ) dlsym ( hDLL, "clCreateProgramWithSource" );
	dclCreateProgramWithBinary	= ( pclCreateProgramWithBinary ) dlsym ( hDLL, "clCreateProgramWithBinary" );
	dclReleaseProgram			= ( pclReleaseProgram ) dlsym ( hDLL, "clReleaseProgram" );


	if ( !VerifyLibAccess () )
		goto Failed;

	dclGetExtensionFunctionAddress	= (pclGetExtensionFunctionAddress) dlsym ( hDLL, "clGetExtensionFunctionAddress" );
	VerifyDirectXExtension ();


	ocl_Lib = hDLL;
	ocl_LibInitialized = true;

	CVerb ( "InitOpenCLLib: successfully initialized access to OpenCL library." );
	return true;

Failed:
	ReleaseOpenCLLib ();
	return false;
}


void ReleaseOpenCLLib ()
{
	CLog ( "ReleaseOpenCLLib" );

	ocl_LibInitialized		= false;
	
	dclEnqueueWriteBuffer	= 0;
	dclEnqueueNDRangeKernel	= 0;
	dclEnqueueReadBuffer	= 0;
	dclFinish				= 0;
	
	dclEnqueueUnmapMemObject= 0;
	dclReleaseMemObject		= 0;
	
	dclReleaseCommandQueue	= 0;
	dclCreateCommandQueue	= 0;
	dclCreateBuffer			= 0;
	dclSetKernelArg			= 0;
	dclReleaseKernel		= 0;
	dclReleaseContext		= 0;
	dclGetPlatformIDs		= 0;
	dclGetDeviceIDs			= 0;
	dclCreateContext		= 0;
	dclBuildProgram			= 0;
	dclGetProgramInfo		= 0;
	dclGetProgramBuildInfo	= 0;
	dclCreateKernel			= 0;
	dclGetDeviceInfo		= 0;
	dclEnqueueMapBuffer		= 0;
	dclWaitForEvents		= 0;
	dclCreateProgramWithSource	= 0;
	dclCreateProgramWithBinary	= 0;

	if ( ocl_Lib ) {
		dlclose ( ocl_Lib );
		ocl_Lib = 0;
	}
	
}

#else

#ifdef _WIN32
#pragma comment ( lib, "OpenCL.lib" )
#endif

bool InitOpenCLLib ()
{
	CVerb ( "InitOpenCLLib" );

	if ( ocl_LibInitialized ) {
		CLog ( "InitOpenCLLib: already initialized." );
		return true;
	}

	dclEnqueueWriteBuffer	= ( pclEnqueueWriteBuffer ) clEnqueueWriteBuffer;
	dclEnqueueNDRangeKernel	= ( pclEnqueueNDRangeKernel ) clEnqueueNDRangeKernel;
	dclEnqueueReadBuffer	= ( pclEnqueueReadBuffer ) clEnqueueReadBuffer;
	dclFinish				= ( pclFinish ) clFinish;

	dclEnqueueUnmapMemObject= ( pclEnqueueUnmapMemObject ) clEnqueueUnmapMemObject;
	dclReleaseMemObject		= ( pclReleaseMemObject ) clReleaseMemObject;

	dclReleaseCommandQueue	= ( pclReleaseCommandQueue ) clReleaseCommandQueue;
	dclCreateCommandQueue	= ( pclCreateCommandQueue ) clCreateCommandQueue;
	dclCreateBuffer			= ( pclCreateBuffer ) clCreateBuffer;
	dclSetKernelArg			= ( pclSetKernelArg ) clSetKernelArg;
	dclReleaseKernel		= ( pclReleaseKernel ) clReleaseKernel;
	dclReleaseContext		= ( pclReleaseContext ) clReleaseContext;
	dclGetPlatformIDs		= ( pclGetPlatformIDs ) clGetPlatformIDs;
	dclGetDeviceIDs			= ( pclGetDeviceIDs ) clGetDeviceIDs;
	dclCreateContext		= ( pclCreateContext ) clCreateContext;
	dclCreateProgramWithSource	= ( pclCreateProgramWithSource ) clCreateProgramWithSource;
	dclCreateProgramWithBinary	= ( pclCreateProgramWithBinary ) clCreateProgramWithBinary;
	dclReleaseProgram		= ( pclReleaseProgram ) clReleaseProgram;
	dclBuildProgram			= ( pclBuildProgram ) clBuildProgram;
	dclGetProgramBuildInfo	= ( pclGetProgramBuildInfo ) clGetProgramBuildInfo;
	dclGetProgramInfo		= ( pclGetProgramInfo ) clGetProgramInfo;
	dclGetImageInfo			= ( pclGetImageInfo ) clGetImageInfo;
	dclCreateKernel			= ( pclCreateKernel ) clCreateKernel;
	dclGetDeviceInfo		= ( pclGetDeviceInfo ) clGetDeviceInfo;
	dclEnqueueMapBuffer		= ( pclEnqueueMapBuffer ) clEnqueueMapBuffer;
	dclWaitForEvents		= ( pclWaitForEvents ) clWaitForEvents;
	dclGetSupportedImageFormats	= ( pclGetSupportedImageFormats ) clGetSupportedImageFormats;


	if ( !VerifyLibAccess () )
		goto Failed;

	dclGetExtensionFunctionAddress	= (pclGetExtensionFunctionAddress)clGetExtensionFunctionAddress;
	VerifyDirectXExtension ();

	ocl_LibInitialized = true;

	CVerb ( "InitOpenCLLib: successfully initialized access to OpenCL library." );
	return true;

Failed:
	return false;
}

void ReleaseOpenCLLib ()
{
	CLog ( "ReleaseOpenCLLib" );

	ocl_LibInitialized = false;	
}
#endif


const char * oclError ( cl_int error )
{
	static const char* errors [] = {
		"CL_SUCCESS",
		"CL_DEVICE_NOT_FOUND",
		"CL_DEVICE_NOT_AVAILABLE",
		"CL_COMPILER_NOT_AVAILABLE",
		"CL_MEM_OBJECT_ALLOCATION_FAILURE",
		"CL_OUT_OF_RESOURCES",
		"CL_OUT_OF_HOST_MEMORY",
		"CL_PROFILING_INFO_NOT_AVAILABLE",
		"CL_MEM_COPY_OVERLAP",
		"CL_IMAGE_FORMAT_MISMATCH",
		"CL_IMAGE_FORMAT_NOT_SUPPORTED",
		"CL_BUILD_PROGRAM_FAILURE",
		"CL_MAP_FAILURE",
		"CL_MISALIGNED_SUB_BUFFER_OFFSET",
		"CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"CL_INVALID_VALUE",
		"CL_INVALID_DEVICE_TYPE",
		"CL_INVALID_PLATFORM",
		"CL_INVALID_DEVICE",
		"CL_INVALID_CONTEXT",
		"CL_INVALID_QUEUE_PROPERTIES",
		"CL_INVALID_COMMAND_QUEUE",
		"CL_INVALID_HOST_PTR",
		"CL_INVALID_MEM_OBJECT",
		"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
		"CL_INVALID_IMAGE_SIZE",
		"CL_INVALID_SAMPLER",
		"CL_INVALID_BINARY",
		"CL_INVALID_BUILD_OPTIONS",
		"CL_INVALID_PROGRAM",
		"CL_INVALID_PROGRAM_EXECUTABLE",
		"CL_INVALID_KERNEL_NAME",
		"CL_INVALID_KERNEL_DEFINITION",
		"CL_INVALID_KERNEL",
		"CL_INVALID_ARG_INDEX",
		"CL_INVALID_ARG_VALUE",
		"CL_INVALID_ARG_SIZE",
		"CL_INVALID_KERNEL_ARGS",
		"CL_INVALID_WORK_DIMENSION",
		"CL_INVALID_WORK_GROUP_SIZE",
		"CL_INVALID_WORK_ITEM_SIZE",
		"CL_INVALID_GLOBAL_OFFSET",
		"CL_INVALID_EVENT_WAIT_LIST",
		"CL_INVALID_EVENT",
		"CL_INVALID_OPERATION",
		"CL_INVALID_GL_OBJECT",
		"CL_INVALID_BUFFER_SIZE",
		"CL_INVALID_MIP_LEVEL",
		"CL_INVALID_GLOBAL_WORK_SIZE",
		"CL_INVALID_PROPERTY",
	};
	const int count = sizeof(errors) / sizeof(errors [0]);
	const int errIndex = -error;

	if ( errIndex >= 0 && errIndex < count)
		return errors [errIndex];

	if ( errIndex < 1000 )
		return "Unknown Error";

	static const char* extErrors [] = {
		"CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR",
		"CL_PLATFORM_NOT_FOUND_KHR",
		"CL_INVALID_D3D10_DEVICE_KHR",
		"CL_INVALID_D3D10_RESOURCE_KHR",
		"CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR",
		"CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR",
		"CL_INVALID_D3D11_DEVICE_KHR",
		"CL_INVALID_D3D11_RESOURCE_KHR",
		"CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR",
		"CL_INVALID_D3D9_DEVICE_KHR",
		"CL_INVALID_D3D9_RESOURCE_KHR",
		"CL_D3D9_RESOURCE_ALREADY_ACQUIRED_KHR",
		"CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR",
		"CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR",
		"CL_D3D9_RESOURCE_NOT_ACQUIRED_KHR", // 1013
	};
	const int extCount = sizeof(extErrors) / sizeof(extErrors [0]);

	const int extIndex = errIndex - 1000;
	if ( extIndex >= 0 && extIndex < extCount )
		return extErrors [extIndex];

	static const char* ext1Errors [] = {
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"CL_DEVICE_PARTITION_FAILED_EXT",
		"CL_INVALID_PARTITION_COUNT_EXT",
		"CL_INVALID_PARTITION_NAME_EXT",
		"CL_INVALID_D3D9_RESOURCE_KHR", // 1059
	};
	const int ext1Count = sizeof(ext1Errors) / sizeof(ext1Errors [0]);

	const int ext1Index = errIndex - 50;
	if ( ext1Index >= 0 && ext1Index < ext1Count )
		return ext1Errors [ext1Index];

	return "Unknown Error";
}


const char * oclChannelOrder ( cl_uint order )
{
	switch ( order )
	{
	case CL_R:
		return "CL_R";
	case CL_A:
		return "CL_A";
	case CL_INTENSITY:
		return "CL_INTENSITY";
	case CL_LUMINANCE:
		return "CL_LUMINANCE";
	case CL_RG:
		return "CL_RG";
	case CL_RA:
		return "CL_RA";
	case CL_RGB:
		return "CL_RGB";
	case CL_RGBA:
		return "CL_RGBA";
	case CL_ARGB:
		return "CL_ARGB";
	case CL_BGRA:
		return "CL_BGRA";
	}
	return "Unknown";
}


const char * oclChannelDataType ( cl_uint dt )
{
	switch ( dt )
	{
	case CL_SNORM_INT8:
		return "CL_SNORM_INT8";
	case CL_SNORM_INT16:
		return "CL_SNORM_INT16";
	case CL_UNORM_INT8:
		return "CL_UNORM_INT8";
	case CL_UNORM_INT16:
		return "CL_UNORM_INT16";
	case CL_UNORM_SHORT_565:
		return "CL_UNORM_SHORT_565";
	case CL_UNORM_SHORT_555:
		return "CL_UNORM_SHORT_555";
	case CL_UNORM_INT_101010:
		return "CL_UNORM_INT_101010";
	case CL_SIGNED_INT8:
		return "CL_SIGNED_INT8";
	case CL_SIGNED_INT16:
		return "CL_SIGNED_INT16";
	case CL_SIGNED_INT32:
		return "CL_SIGNED_INT32";
	case CL_UNSIGNED_INT8:
		return "CL_UNSIGNED_INT8";
	case CL_UNSIGNED_INT16:
		return "CL_UNSIGNED_INT16";
	case CL_UNSIGNED_INT32:
		return "CL_UNSIGNED_INT32";
	case CL_HALF_FLOAT:
		return "CL_HALF_FLOAT";
	case CL_FLOAT:
		return "CL_FLOAT";
	}
	return "Unknown";
}

} // -> namespace environs



