package environs;
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
@SuppressWarnings("unused")
public class TypesSource {
/**
 * Types - This class defines integer values which identifies status values, events, message types and so on delivered by the environment.
 *
 * TypesSource.java can be removed prior to build of the library. It just serves for auto-generation of the java/c/cpp/cli headers and type files.
 *
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 */
	/**
	 * Environs native layer status
	 */
	/** Disposed. */
	public static final int STATUS_DISPOSED = -1;
	/** Uninitialized. Usually after creation of an Environs object. */
	public static final int STATUS_UNINITIALIZED = 0;
	/** Environs is about to be disposed. */
	public static final int STATUS_DISPOSING = 1;
	/** Environs is initializing. */
	public static final int STATUS_INITIALIZING = 2;
	/** Environs is initialized. Usually after a call to Environs.Init() */
	public static final int STATUS_INITIALIZED = 3;
	/** Environs is stopped. Usually after a call to Environs.Stop() */
	public static final int STATUS_STOPPED = 4;
	/** Environs is currently stopping. Threads are being shut down and allocated resources are being released. */
	public static final int STATUS_STOP_IN_PROGRESS = 5;
	/** Environs is about to Stop. Threads are being shut down and allocated resources are being released. */
	public static final int STATUS_STOPPING = 6;
	/** Environs is about to Start. Thread are being started and resources are being allocated. */
	public static final int STATUS_STARTING = 7;
	/** Environs is started. Usually after a call to Environs.Start() */
	public static final int STATUS_STARTED = 8;
	/** Environs is in connected state and connected to at least one device. */
	public static final int STATUS_CONNECTED = 9;

	/** 
	 * Environs Status enumeration. Represents the same values as for NATIVE_STATUS_* 
	 * */
	private static enum Status {
		/** Disposed. */
		Disposed		(STATUS_DISPOSED),
		/** Uninitialized. Usually after creation of an Environs object. */
		Uninitialized	(STATUS_UNINITIALIZED),
		/** Environs is about to be disposed. */
		Disposing 		(STATUS_DISPOSING),
		/** Environs is initializing. */
		Initializing 	(STATUS_INITIALIZING),
		/** Environs is initialized. Usually after a call to Environs.Init() */
		Initialized 	(STATUS_INITIALIZED),
		/** Environs is stopped. Usually after a call to Environs.Stop() */
		Stopped 		(STATUS_STOPPED),
		/** Environs is currently stopping. Threads are being shut down and allocated resources are being released. */
		StopInProgress	(STATUS_STOP_IN_PROGRESS),
		/** Environs is about to Stop. Threads are being shut down and allocated resources are being released. */
		Stopping 		(STATUS_STOPPING),
		/** Environs is about to Start. Thread are being started and resources are being allocated. */
		Starting 		(STATUS_STARTING),
		/** Environs is started. Usually after a call to Environs.Start() */
		Started 		(STATUS_STARTED),
		/** Environs is in connected state and connected to at least one device. */
		Connected 		(STATUS_CONNECTED);
		
		public int value;		  
		private Status(int value) { this.value = value; }		  
		public int value() { return value; }
		public static Status get(int v) {
			switch (v) {
				case STATUS_DISPOSED: return Disposed;
				case STATUS_DISPOSING: return Disposing;
				case STATUS_INITIALIZING: return Initializing;
				case STATUS_INITIALIZED: return Initialized;
				case STATUS_STOPPED: return Stopped;
				case STATUS_STOP_IN_PROGRESS: return StopInProgress;
				case STATUS_STOPPING: return Stopping;
				case STATUS_STARTING: return Starting;
				case STATUS_STARTED: return Started;
				case STATUS_CONNECTED: return Connected;
			}
			return Uninitialized;
		}
	}

	/**
	 * Max supported instances of Environs objects that each application can run at the same time.
	 * */
	public static final int ENVIRONS_MAX_ENVIRONS_INSTANCES_MOBILE 	= 5;
	public static final int ENVIRONS_MAX_ENVIRONS_INSTANCES_FULL 	= 10;

	/**
	 * A constant value that identifies an uninitialized display value.
	 * */
	public static final int ENVIRONS_DISPLAY_UNINITIALIZED_VALUE 	= -1;

	/**
	 * Environs thread states
	 */
	/** Uninitialized. Usually after creation of an Environs object. */
	public static final int ENVIRONS_THREAD_NO_THREAD 			= 0;
	/** Thread is either created and not yet running or terminated. */
	public static final int ENVIRONS_THREAD_DETACHEABLE 		= 1;
	/** Thread is running. */
	public static final int ENVIRONS_THREAD_RUNNING 			= 2;


	/**
	 * Environs thread Status enumeration.
	 * */
	private static enum ThreadStatus {
		/** Uninitialized. Usually after creation of an Environs object. */
		NoThread			(ENVIRONS_THREAD_NO_THREAD),
		/** Thread is either created and not yet running or terminated. */
		Detacheable			(ENVIRONS_THREAD_DETACHEABLE),
		/** Thread is running. */
		Running 			(ENVIRONS_THREAD_RUNNING);

		public int value;
		private ThreadStatus(int value) { this.value = value; }
		public int value() { return value; }
		public static ThreadStatus get(int v) {
			switch (v) {
				case ENVIRONS_THREAD_NO_THREAD: return NoThread;
				case ENVIRONS_THREAD_DETACHEABLE: return Detacheable;
				case ENVIRONS_THREAD_RUNNING: return Running;
			}
			return NoThread;
		}
	}


	/** Deleteable. Device object has been disabled.&nbsp; 
	 * "Garbage Collection" should dispose and delete the object on the next occasion. 
	 * */
	public static final int DEVICE_STATUS_DELETEABLE 			= 0;
	/** Device object has just been created. */
	public static final int DEVICE_STATUS_CREATED 				= 2;
	/** Connect in progress. Device object has been created and the connecting task is in progress. */
	public static final int DEVICE_STATUS_CONNECT_IN_PROGRESS 	= 3;
	/** Connected. Device object is connected to the destination device and active. */
	public static final int DEVICE_STATUS_CONNECTED 			= 4;

	/** 
	 * Device Connect Status enumeration. Represents the same values as for DEVICE_STATUS_* 
	 * */
	private static enum DeviceStatus {
		/** Deleteable. Device object has been disabled.&nbsp; 
		 * "Garbage Collection" should dispose and delete the object on the next occasion. 
		 * */
		Deleteable			(DEVICE_STATUS_DELETEABLE),
		/** Device object has just been created. */
		Created				(DEVICE_STATUS_CREATED),
		/** Connect in progress. Device object has been created and the connecting task is in progress. */
		ConnectInProgress 	(DEVICE_STATUS_CONNECT_IN_PROGRESS),
		/** Connected. Device object is connected to the destination device and active. */
		Connected 			(DEVICE_STATUS_CONNECTED);
		
		public int value;		  
		private DeviceStatus(int value) { this.value = value; }		  
		public int value() { return value; }
		public static DeviceStatus get(int v) {
			switch (v) {
			case DEVICE_STATUS_CREATED: return Created;
			case DEVICE_STATUS_CONNECT_IN_PROGRESS: return ConnectInProgress;
			case DEVICE_STATUS_CONNECTED: return Connected;
			}
			return Deleteable;
		}
	}
	
	/* 
	 * Environs source values which determines the source of an event, data, or message.
	 */
	/** Sent by native layer. */
	public static final int SOURCE_NATIVE			= 0;
	/** Sent by platform specific layer. */
	public static final int SOURCE_PLATFORM			= 1;
	/** Sent by another device within the environment.  */
	public static final int SOURCE_DEVICE			= 2;
	/** Sent by the app layer. */
	public static final int SOURCE_APPLICATION		= 3;

	/** 
	 * Environs source values which determines the source of an event, data, or message.&nbsp;
	 * Represents the same values as for ENVIRONS_SOURCE_* 
	 * */
	private static enum Source {
		/** Sent by native layer. */
		Native			(SOURCE_NATIVE),
		/** Sent by platform specific layer. */
		Platform		(SOURCE_PLATFORM),
		/** Sent by another device within the environment.  */
		Device 			(SOURCE_DEVICE),
		/** Sent by the app layer. */
		Application		(SOURCE_APPLICATION);
		
		public int value;		  
		private Source(int value) { this.value = value; }		  
		public int value() { return value; }
		public static Source get(int v) {
			switch (v) {
			case SOURCE_PLATFORM: return Platform;
			case SOURCE_DEVICE: return Device;
			case SOURCE_APPLICATION: return Application;
			}
			return Native;
		}
	}


	public static final int ENVIRONS_OBJECT_DISPOSED 			= -1;
	public static final int ENVIRONS_OBJECT_DISPOSED_PLATFORM 	= -2;

	/*
	 * Native payload type class is determined by the upper byte of payload
	 */
	public static final int MSG_NOTIFY_ID					= 0xFF00;
	public static final int MSG_NOTIFY_CLASS				= 0xFF0000;
	
	/*
	 * Native packet data types, first 4 bytes must confirm to one of these types
	 * Type: unsigned short 0xFFFF
	 */
	/** Class: Helo type */
	public static final int MSG_TYPE_HELO					= 0;
	public static final int MSG_HANDSHAKE					= 0x100;	// Handshake states
	
	/** Main channel */
	public static final int MSG_HANDSHAKE_MAIN				= MSG_HANDSHAKE | 0x10; // Former 'D'
	public static final int MSG_HANDSHAKE_MAIN_REQ			= MSG_HANDSHAKE | MSG_HANDSHAKE_MAIN | 1; 	// Former 'D'
	public static final int MSG_HANDSHAKE_MAIN_ACK			= MSG_HANDSHAKE | MSG_HANDSHAKE_MAIN | 2; // Former 'D'
	public static final int MSG_HANDSHAKE_MAIN_FAIL			= MSG_HANDSHAKE | MSG_HANDSHAKE_MAIN | 3; // Former 'D'
	public static final int MSG_HANDSHAKE_MAIN_CLOSED		= MSG_HANDSHAKE | MSG_HANDSHAKE_MAIN | 4; // Former 'D'

	public static final int MSG_HANDSHAKE_COMDAT			= MSG_HANDSHAKE | 0x20;
	public static final int MSG_HANDSHAKE_COMDAT_REQ		= MSG_HANDSHAKE | MSG_HANDSHAKE_COMDAT | 1;
	public static final int MSG_HANDSHAKE_COMDAT_ACK		= MSG_HANDSHAKE | MSG_HANDSHAKE_COMDAT | 2;
	public static final int MSG_HANDSHAKE_COMDAT_FAILED		= MSG_HANDSHAKE | MSG_HANDSHAKE_COMDAT | 3;
	public static final int MSG_HANDSHAKE_COMDAT_CLOSED		= MSG_HANDSHAKE | MSG_HANDSHAKE_COMDAT | 4;

	public static final int MSG_HANDSHAKE_PROC				= MSG_HANDSHAKE | 0x40;
	public static final int MSG_HANDSHAKE_PORTS				= MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 1; // Former 'P'
	public static final int MSG_HANDSHAKE_PORTS_ACK			= MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 2;
	public static final int MSG_HANDSHAKE_CONIG_REQ			= MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 3;
	public static final int MSG_HANDSHAKE_CONIG_RESP		= MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 4;
	public static final int MSG_HANDSHAKE_CONIG_RESP_ACK	= MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 5;
	public static final int MSG_HANDSHAKE_CONNECTED			= MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 0xA;
	public static final int MSG_HANDSHAKE_DISCONNECTED		= MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 0xB;
	public static final int MSG_HANDSHAKE_PING				= MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 0xC;
	

	public static final int MSG_HANDSHAKE_UDP				= MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 6;
	public static final int MSG_HANDSHAKE_UDP_ACK			= MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 7;
	
	public static final int MSG_HANDSHAKE_SUCCESS			= MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 0xE;
	public static final int MSG_HANDSHAKE_SUCCESS_ACK		= MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 0xF;
	
	public static final int MSG_HANDSHAKE_SHORT_MESSAGE		= MSG_HANDSHAKE | 0x81;
	public static final int MSG_HANDSHAKE_SHORT_MESSAGE_ACK	= MSG_HANDSHAKE | 0x82;

	public static final int NOTIFY_TYPE_CONNECTION			= (MSG_TYPE_HELO << 16);
	public static final int NOTIFY_CONNECTION_MAIN_NEW		= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_MAIN_REQ;
	public static final int NOTIFY_CONNECTION_MAIN_ACK		= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_MAIN_ACK;
	public static final int NOTIFY_CONNECTION_MAIN_FAILED	= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_MAIN_FAIL;
	public static final int NOTIFY_CONNECTION_MAIN_CLOSED	= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_MAIN_CLOSED;	
	public static final int NOTIFY_CONNECTION_COMDAT_NEW	= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_COMDAT_REQ;
	public static final int NOTIFY_CONNECTION_COMDAT_ACK	= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_COMDAT_ACK;
	public static final int NOTIFY_CONNECTION_COMDAT_FAILED	= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_COMDAT_FAILED;
	public static final int NOTIFY_CONNECTION_COMDAT_CLOSED	= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_COMDAT_CLOSED;
	public static final int NOTIFY_CONNECTION_DATA_ACK 		= NOTIFY_TYPE_CONNECTION | 0xA;
	public static final int NOTIFY_CONNECTION_DATA_CLOSED 	= NOTIFY_TYPE_CONNECTION | 0xC;

	public static final int NOTIFY_CONNECTION_PROGRESS 		= NOTIFY_TYPE_CONNECTION | 0xD;
	
	public static final int NOTIFY_CONNECTION_ESTABLISHED	= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_SUCCESS;
	public static final int NOTIFY_CONNECTION_ESTABLISHED_ACK	= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_SUCCESS_ACK;
	public static final int NOTIFY_CONNECTION_CLOSED		= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_DISCONNECTED;	
	
	public static final int NOTIFY_SHORT_MESSAGE			= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_SHORT_MESSAGE;	
	public static final int NOTIFY_SHORT_MESSAGE_ACK		= NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_SHORT_MESSAGE_ACK;

	/**
	 * Notify connection enumeration.
	 * */
	private static enum Notify_Connection { //_Connection
		type				(NOTIFY_TYPE_CONNECTION),
		MainNew				(NOTIFY_CONNECTION_MAIN_NEW),
		MainAck 			(NOTIFY_CONNECTION_MAIN_ACK),
		MainFailed 			(NOTIFY_CONNECTION_MAIN_FAILED),
		MainClosed 			(NOTIFY_CONNECTION_MAIN_CLOSED),
		ComDatNew			(NOTIFY_CONNECTION_COMDAT_NEW),
		ComDatAck 			(NOTIFY_CONNECTION_COMDAT_ACK),
		ComDatFailed 		(NOTIFY_CONNECTION_COMDAT_FAILED),
		ComDatClosed 		(NOTIFY_CONNECTION_COMDAT_CLOSED),
		DataAck 			(NOTIFY_CONNECTION_DATA_ACK),
		DataClosed 			(NOTIFY_CONNECTION_DATA_CLOSED),
		Progress 			(NOTIFY_CONNECTION_PROGRESS),
		Established 		(NOTIFY_CONNECTION_ESTABLISHED),
		EstablishedAck 		(NOTIFY_CONNECTION_ESTABLISHED_ACK),
		Closed 				(NOTIFY_CONNECTION_CLOSED),
		;

		public int value;
		private Notify_Connection(int value) { this.value = value; }
		public int value() { return value; }
		public static Notify_Connection get(int v) {
			switch (v) {
				case NOTIFY_TYPE_CONNECTION: return type;
				case NOTIFY_CONNECTION_MAIN_NEW: return MainNew;
				case NOTIFY_CONNECTION_MAIN_ACK: return MainAck;
				case NOTIFY_CONNECTION_MAIN_FAILED: return MainFailed;
				case NOTIFY_CONNECTION_MAIN_CLOSED: return MainClosed;
				case NOTIFY_CONNECTION_COMDAT_NEW: return ComDatNew;
				case NOTIFY_CONNECTION_COMDAT_ACK: return ComDatAck;
				case NOTIFY_CONNECTION_COMDAT_FAILED: return ComDatFailed;
				case NOTIFY_CONNECTION_COMDAT_CLOSED: return ComDatClosed;
				case NOTIFY_CONNECTION_DATA_ACK: return DataAck;
				case NOTIFY_CONNECTION_DATA_CLOSED: return DataClosed;
				case NOTIFY_CONNECTION_PROGRESS: return Progress;
				case NOTIFY_CONNECTION_ESTABLISHED: return Established;
				case NOTIFY_CONNECTION_ESTABLISHED_ACK: return EstablishedAck;
				case NOTIFY_CONNECTION_CLOSED: return Closed;
			}
			return type;
		}
	}

	
	public static final int DATA_STREAM 					= 0x200;	// Stream types
	public static final int DATA_STREAM_INIT				= 1;

	public static final int DATA_STREAM_IFRAME				= 0x400;

	/** Class: Image type */
	public static final int MSG_TYPE_IMAGE					= 1;
	public static final int DATA_STREAM_IMAGE				= MSG_TYPE_IMAGE << 4; // 0x10
	public static final int DATA_STREAM_IMAGE_INIT			= DATA_STREAM | DATA_STREAM_IMAGE | DATA_STREAM_INIT;
	public static final int DATA_STREAM_IMAGE_DATA			= DATA_STREAM | DATA_STREAM_IMAGE | 8;
	public static final int DATA_STREAM_IMAGE_JPEG			= DATA_STREAM_IMAGE_DATA | 2;
	public static final int DATA_STREAM_IMAGE_PNG			= DATA_STREAM_IMAGE_DATA | 4;

	/** Class: Video type */
	public static final int MSG_TYPE_STREAM					= 2;
	public static final int DATA_STREAM_VIDEO				= MSG_TYPE_STREAM << 4; // 0x20
	public static final int DATA_STREAM_VIDEO_INIT			= DATA_STREAM | DATA_STREAM_VIDEO | DATA_STREAM_INIT;	// Initialization protocol version 1 packet with width and height
	public static final int DATA_STREAM_VIDEO_HDR 			= DATA_STREAM | DATA_STREAM_VIDEO | 2;	// Header packets
	public static final int DATA_STREAM_H265_NALUS 			= DATA_STREAM | DATA_STREAM_VIDEO | 4;
	public static final int DATA_STREAM_H264_NALUS 			= DATA_STREAM | DATA_STREAM_VIDEO | 8;

	/*
	public static final int DATA_STREAM_H264_NAL 			= DATA_STREAM | DATA_STREAM_VIDEO | 4;
	 */


	/** Class: Portal constants */
	public static final int MAX_PORTAL_STREAMS_A_DEVICE		= 6;
	public static final int MAX_PORTAL_CONTEXT_WORKERS		= 2;
	public static final int MAX_PORTAL_OVERLAYS				= 6;
//	public static final int MAX_PORTAL_GENERATOR_SLOTS		= 5;
	public static final int MAX_PORTAL_REQUEST_WAIT_TIME_MS	= 30000;


	/**
	 * A portal ID is masked as follows:
	 * 0xFFFFFFFF
	 * 0xFF000000 portal map table identifier (used internal by devices to access the map)
	 * 0x0000F000 portal type
	 * 0x00000300 direction
	 * 0x000000FF portal id (the same between both devices)
	 */

	/** Class: Portal type */
	public static final int PORTAL_TYPE_ANY 				= 0;
	public static final int PORTAL_TYPE_SCREEN 				= 0x1000;
	public static final int PORTAL_TYPE_BACK_CAM 			= 0x2000;
	public static final int PORTAL_TYPE_FRONT_CAM 			= 0x4000;
	public static final int PORTAL_TYPE_MASK 				= 0xF000;
	
	/** 
	 * Portal types enumeration. Represents the same values as for PORTAL_TYPE_* 
	 * */
	private static enum PortalType {
		/** Any type. The requested portal can be of any type. Which one depends on the application logic.
		 * */
		Any					(PORTAL_TYPE_ANY),
		/** The devices screen. */
		Screen				(PORTAL_TYPE_SCREEN),
		/** The back facing camera. */
		BackCam 			(PORTAL_TYPE_BACK_CAM),
		/** The front facing camera. */
		FrontCam 			(PORTAL_TYPE_FRONT_CAM);
		
		public int value;		  
		private PortalType(int value) { this.value = value; }		  
		public int value() { return value; }
		public static PortalType get(int v) {
			switch (v) {
			case PORTAL_TYPE_SCREEN: return Screen;
			case PORTAL_TYPE_BACK_CAM: return BackCam;
			case PORTAL_TYPE_FRONT_CAM: return FrontCam;
			}
			return Any;
		}
	}

	/** Class: Portal status */
	public static final int PORTAL_STATUS_DISPOSED				= ENVIRONS_OBJECT_DISPOSED;
	public static final int PORTAL_STATUS_CREATED				= 0;
	public static final int PORTAL_STATUS_CREATED_FROM_REQUEST	= 1;
	public static final int PORTAL_STATUS_CREATED_ASK_REQUEST	= 2;
	public static final int PORTAL_STATUS_ESTABLISHED			= 4;
	public static final int PORTAL_STATUS_STARTED				= 6;

	/**
	 * Portal status enumeration.
	 * */
	private static enum PortalStatus {
		Disposed			(PORTAL_STATUS_DISPOSED),
		Created				(PORTAL_STATUS_CREATED),
		CreatedFromRequest	(PORTAL_STATUS_CREATED_FROM_REQUEST),
		CreatedAskRequest 	(PORTAL_STATUS_CREATED_ASK_REQUEST),
		Established 		(PORTAL_STATUS_ESTABLISHED),
		Started		 		(PORTAL_STATUS_STARTED);

		private PortalStatus(int value) {}
	}


	/**
	 * Portal Source Status enumeration.
	 * */
	private static enum PortalSourceStatus {
		/** Deleteable. The portal object has been disabled.&nbsp;
		 * "Garbage Collection" should dispose and delete the object on the next occasion.
		 * */
		Deleteable			(0),
		/** The portal has just been created. */
		Created				(1),
		/** Initialized. The portal is initialized, that is the resources (threads, plugins, the pipeline) has been established or are ready to use */
		Initialized 		(2),
		/** Active. The portal is initialized and actively streaming. */
		Active 				(3);

		public int value;
		private PortalSourceStatus(int value) { this.value = value; }
		public int value() { return value; }
		public static PortalSourceStatus get(int v) {
			switch (v) {
				case 1: return Created;
				case 2: return Initialized;
				case 3: return Active;
			}
			return Deleteable;
		}
	}


	/** Class: Portal stream type */
	public static final int STREAMTYPE_UNKNOWN			= 0;
	public static final int STREAMTYPE_IMAGES			= 0x10;
	public static final int STREAMTYPE_IMAGES_JPEG		= STREAMTYPE_IMAGES | 0x1;
	public static final int STREAMTYPE_IMAGES_PNG		= STREAMTYPE_IMAGES | 0x2;
	public static final int STREAMTYPE_VIDEO			= 0x20;
	public static final int STREAMTYPE_VIDEO_H264		= STREAMTYPE_VIDEO | 0x1;
	
	/** 
	 * Portal stream type enumeration. Represents the same values as for STREAMTYPE_*
	 * */
	private static enum PortalStreamType {
		Unknown				(STREAMTYPE_UNKNOWN),
		/** Sequence of jpeg images. 	*/
		Images				(STREAMTYPE_IMAGES),
		/** Sequence of jpeg images. 	*/
		ImagesJPEG			(STREAMTYPE_IMAGES_JPEG),
		/** Sequence of png images. 	*/
		ImagesPNG			(STREAMTYPE_IMAGES_PNG),
		/** Video stream. 						*/
		Video				(STREAMTYPE_VIDEO),
		/** Video stream H264. 						*/
		VideoH264			(STREAMTYPE_VIDEO_H264);
		
		public int value;		  
		private PortalStreamType(int value) { this.value = value; }		  
		public int value() { return value; }
		public static PortalStreamType get(int v) {
			switch (v) {
				case STREAMTYPE_IMAGES: return Images;
				case STREAMTYPE_IMAGES_JPEG: return ImagesJPEG;
				case STREAMTYPE_IMAGES_PNG: return ImagesPNG;
				case STREAMTYPE_VIDEO: return Video;
				case STREAMTYPE_VIDEO_H264: return VideoH264;
			}
			return Unknown;
		}
	}

	/** Class: PortalInfo flag bits */
	public static final int PORTAL_INFO_FLAG_LOCATION		= 0x1;
	public static final int PORTAL_INFO_FLAG_ANGLE			= 0x2;
	public static final int PORTAL_INFO_FLAG_SIZE			= 0x4;


	/** Class: Portal messages and notifications */
	public static final int MSG_TYPE_PORTAL 				= 5;
	public static final int MSG_PORTAL_ERROR				= 0x400;
	public static final int PORTAL_DIR_INCOMING				= 0x200;
	public static final int PORTAL_DIR_OUTGOING				= 0x100;
	public static final int PORTAL_DIR_MASK					= 0x300;
	public static final int NOTIFY_PORTAL 					= 0x800;

	public static final String MSG_PORTAL_Descriptions [] = { // DEBUG
			"Portal requested",
			"Portal provided",
			"Stream portal provided",
			"Image portal provided",
			"Portal request failed",
			"Portal Stop",
			"Portal Stop ack",
			"Portal Stop failed",
			"Portal Start",
			"Portal Start ack",
			"Portal Start failed",
			"Portal pause",
			"Portal pause ack",
			"Portal pause failed",
			"Portal buffer full",
			"Portal buffer available again",
			"Portal i-frame requested",
	};

	public static final int NOTIFY_PORTAL_INSTANCE 					= 0x100800;
	public static final int PORTAL_INSTANCE_FLAG_SURFACE_CHANGED	= NOTIFY_PORTAL_INSTANCE | 0x1;

	// Portal message subtypes
	public static final int MSG_PORTAL_REQUEST_ID			= 0;
	public static final int MSG_PORTAL_REQUEST 				= NOTIFY_PORTAL 	| MSG_PORTAL_REQUEST_ID;
	public static final int MSG_PORTAL_ASK_FOR_REQUEST_ID	= 1;
	public static final int MSG_PORTAL_ASK_FOR_REQUEST 		= NOTIFY_PORTAL 	| MSG_PORTAL_ASK_FOR_REQUEST_ID;
	public static final int MSG_PORTAL_PROVIDE_STREAM_ID	= 2;
	public static final int MSG_PORTAL_PROVIDE_STREAM		= NOTIFY_PORTAL 	| MSG_PORTAL_PROVIDE_STREAM_ID;
	public static final int MSG_PORTAL_PROVIDE_IMAGES_ID	= 3;
	public static final int MSG_PORTAL_PROVIDE_IMAGES		= NOTIFY_PORTAL 	| MSG_PORTAL_PROVIDE_IMAGES_ID;
	public static final int MSG_PORTAL_REQUEST_FAIL_ID		= 4;
	public static final int MSG_PORTAL_REQUEST_FAIL			= MSG_PORTAL_ERROR 	| MSG_PORTAL_REQUEST_FAIL_ID;
	
	public static final int MSG_PORTAL_STOP_ID 				= 5;
	public static final int MSG_PORTAL_STOP 				= NOTIFY_PORTAL 	| MSG_PORTAL_STOP_ID;
	public static final int MSG_PORTAL_STOP_ACK_ID			= 6;
	public static final int MSG_PORTAL_STOP_ACK				= NOTIFY_PORTAL 	| MSG_PORTAL_STOP_ACK_ID;
	public static final int MSG_PORTAL_STOP_FAIL_ID			= 7;
	public static final int MSG_PORTAL_STOP_FAIL			= MSG_PORTAL_ERROR 	| MSG_PORTAL_STOP_FAIL_ID;
	public static final int MSG_PORTAL_START_ID 			= 8;
	public static final int MSG_PORTAL_START 				= NOTIFY_PORTAL 	| MSG_PORTAL_START_ID;
	public static final int MSG_PORTAL_START_ACK_ID			= 9;
	public static final int MSG_PORTAL_START_ACK			= NOTIFY_PORTAL 	| MSG_PORTAL_START_ACK_ID;
	public static final int MSG_PORTAL_START_FAIL_ID		= 10;
	public static final int MSG_PORTAL_START_FAIL			= MSG_PORTAL_ERROR 	| MSG_PORTAL_START_FAIL_ID;
	public static final int MSG_PORTAL_PAUSE_ID 			= 11;
	public static final int MSG_PORTAL_PAUSE 				= NOTIFY_PORTAL 	| MSG_PORTAL_PAUSE_ID;
	public static final int MSG_PORTAL_PAUSE_ACK_ID			= 12;
	public static final int MSG_PORTAL_PAUSE_ACK			= NOTIFY_PORTAL 	| MSG_PORTAL_PAUSE_ACK_ID;
	public static final int MSG_PORTAL_PAUSE_FAIL_ID		= 13;
	public static final int MSG_PORTAL_PAUSE_FAIL			= MSG_PORTAL_ERROR 	| MSG_PORTAL_PAUSE_FAIL_ID;


	public static final int MSG_PORTAL_BUFFER_FULL_ID		= 14;
	public static final int MSG_PORTAL_BUFFER_FULL			= NOTIFY_PORTAL 	| MSG_PORTAL_BUFFER_FULL_ID;
	public static final int MSG_PORTAL_BUFFER_AVAIL_AGAIN_ID	= 15;
	public static final int MSG_PORTAL_BUFFER_AVAIL_AGAIN	= NOTIFY_PORTAL 	| MSG_PORTAL_BUFFER_AVAIL_AGAIN_ID;
	public static final int MSG_PORTAL_IFRAME_REQUEST_ID	= 16;
	public static final int MSG_PORTAL_IFRAME_REQUEST		= NOTIFY_PORTAL 	| MSG_PORTAL_IFRAME_REQUEST_ID;

	public static final int MSG_PORTAL_MAX_COUNT			= 					16 + 1;


	public static final int NOTIFY_TYPE_PORTAL					= (MSG_TYPE_PORTAL << 16);
	public static final int NOTIFY_PORTAL_ESTABLISHED 			= 0x80;
	public static final int NOTIFY_PORTAL_ESTABLISHED_RESOLUTION = NOTIFY_TYPE_PORTAL | 0x81;

	public static final int NOTIFY_PORTAL_REQUEST				= NOTIFY_TYPE_PORTAL | MSG_PORTAL_REQUEST;
	public static final int NOTIFY_PORTAL_ASK_REQUEST			= NOTIFY_TYPE_PORTAL | MSG_PORTAL_ASK_FOR_REQUEST;
	public static final int NOTIFY_PORTAL_STREAM_INCOMING		= NOTIFY_TYPE_PORTAL | MSG_PORTAL_PROVIDE_STREAM | PORTAL_DIR_INCOMING;	
	public static final int NOTIFY_PORTAL_IMAGES_INCOMING		= NOTIFY_TYPE_PORTAL | MSG_PORTAL_PROVIDE_IMAGES | PORTAL_DIR_INCOMING;
	public static final int NOTIFY_PORTAL_INCOMING_ESTABLISHED	= NOTIFY_PORTAL_STREAM_INCOMING | NOTIFY_PORTAL_IMAGES_INCOMING | NOTIFY_PORTAL_ESTABLISHED;
	public static final int NOTIFY_PORTAL_PROVIDE_STREAM_ACK	= NOTIFY_TYPE_PORTAL | MSG_PORTAL_PROVIDE_STREAM | PORTAL_DIR_OUTGOING;	
	public static final int NOTIFY_PORTAL_PROVIDE_IMAGES_ACK	= NOTIFY_TYPE_PORTAL | MSG_PORTAL_PROVIDE_IMAGES | PORTAL_DIR_OUTGOING;
	public static final int NOTIFY_PORTAL_PROVIDER_READY		= NOTIFY_PORTAL_PROVIDE_STREAM_ACK | NOTIFY_PORTAL_PROVIDE_IMAGES_ACK | NOTIFY_PORTAL_ESTABLISHED;
	public static final int NOTIFY_PORTAL_REQUEST_FAIL			= NOTIFY_TYPE_PORTAL | MSG_PORTAL_REQUEST_FAIL | PORTAL_DIR_INCOMING;
	public static final int NOTIFY_PORTAL_PROVIDE_FAIL			= NOTIFY_TYPE_PORTAL | MSG_PORTAL_REQUEST_FAIL | PORTAL_DIR_OUTGOING;
	
	public static final int NOTIFY_PORTAL_STREAM_STARTED 		= NOTIFY_TYPE_PORTAL | MSG_PORTAL_START_ACK;	
	public static final int NOTIFY_PORTAL_STREAM_PAUSED  		= NOTIFY_TYPE_PORTAL | MSG_PORTAL_PAUSE_ACK;	
	public static final int NOTIFY_PORTAL_STREAM_STOPPED 		= NOTIFY_TYPE_PORTAL | MSG_PORTAL_STOP_ACK;

	public static final int NOTIFY_PORTAL_STREAM_RECEIVER_STARTED 		= NOTIFY_PORTAL_REQUEST | PORTAL_DIR_INCOMING | 0xFF;

	/**
	 * Notify portal enumeration.
	 * */
	private static enum Notify_Portal {
		Disposed				(ENVIRONS_OBJECT_DISPOSED),
		Zero					(0),
		type					(NOTIFY_TYPE_PORTAL),
		Established				(NOTIFY_PORTAL_ESTABLISHED),
		EstablishedResolution	(NOTIFY_PORTAL_ESTABLISHED_RESOLUTION),
		Request 				(NOTIFY_PORTAL_REQUEST),
		AskRequest 				(NOTIFY_PORTAL_ASK_REQUEST),
		StreamIncoming			(NOTIFY_PORTAL_STREAM_INCOMING),
		ImagesIncoming 			(NOTIFY_PORTAL_IMAGES_INCOMING),
		IncomingEstablished 	(NOTIFY_PORTAL_INCOMING_ESTABLISHED),
		ProvideStreamAck 		(NOTIFY_PORTAL_PROVIDE_STREAM_ACK),
		ProvideImagesAck 		(NOTIFY_PORTAL_PROVIDE_IMAGES_ACK),
		ProviderReady 			(NOTIFY_PORTAL_PROVIDER_READY),
		RequestFail 			(NOTIFY_PORTAL_REQUEST_FAIL),
		ProvideFail 			(NOTIFY_PORTAL_PROVIDE_FAIL),
		StreamStarted 			(NOTIFY_PORTAL_STREAM_STARTED),
		StreamPaused 			(NOTIFY_PORTAL_STREAM_PAUSED),
		StreamStopped 			(NOTIFY_PORTAL_STREAM_STOPPED),
		StreamReceiverStarted	(NOTIFY_PORTAL_STREAM_RECEIVER_STARTED),
		LocationChanged			(NOTIFY_PORTAL_LOCATION_CHANGED),
		SizeChanged				(NOTIFY_PORTAL_SIZE_CHANGED),
		ContactChanged			(NOTIFY_CONTACT_DIRECT_CHANGED),
		;

		public int value;
		private Notify_Portal(int value) { this.value = value; }
		public int value() { return value; }
	}


	/**
	 * Environs options set/get messages
	 */
	/** Class: Options type */
	public static final int MSG_TYPE_OPTIONS					= 6;
	public static final int MSG_TYPE_OPTIONS_RESPONSE			= 7;
	public static final int MSG_OPTION_TYPE						= 0xF00;
	public static final int MSG_OPTION_SET						= 0x100;
	public static final int MSG_OPTION_GET						= 0x200;

	public static final int MSG_OPTION_TRANSPORT				= 0x10;	// Transport options
	public static final int MSG_OPT_TRANSP_TCP_PORTAL			= MSG_OPTION_TRANSPORT | 0x1;
	public static final int MSG_OPT_TRANSP_TCP_PORTAL_SET		= MSG_OPTION_SET | MSG_OPT_TRANSP_TCP_PORTAL;
	public static final int MSG_OPT_TRANSP_TCP_PORTAL_GET		= MSG_OPTION_GET | MSG_OPT_TRANSP_TCP_PORTAL;

	public static final int MSG_OPTION_PORTAL					= 0x20;	// Portal options
	public static final int MSG_OPT_PORTAL_CENTER				= MSG_OPTION_PORTAL | 0x1;
	public static final int MSG_OPT_PORTAL_CENTER_SET			= MSG_OPTION_SET | MSG_OPT_PORTAL_CENTER;
	public static final int MSG_OPT_PORTAL_CENTER_GET			= MSG_OPTION_GET | MSG_OPT_PORTAL_CENTER;	
	public static final int MSG_OPT_PORTAL_WH					= MSG_OPTION_PORTAL | 0x2;
	public static final int MSG_OPT_PORTAL_WH_SET				= MSG_OPTION_SET | MSG_OPT_PORTAL_WH;
	public static final int MSG_OPT_PORTAL_INFO					= MSG_OPTION_PORTAL | 0x4;
	public static final int MSG_OPT_PORTAL_INFO_SET				= MSG_OPTION_SET | MSG_OPT_PORTAL_INFO;
	public static final int MSG_OPT_PORTAL_INFO_GET				= MSG_OPTION_GET | MSG_OPT_PORTAL_INFO;

	public static final int MSG_OPTION_CONTACT					= 0x40;	// Physical contact options
	public static final int MSG_OPT_CONTACT_DIRECT				= MSG_OPTION_CONTACT | 0x1;
	public static final int MSG_OPT_CONTACT_DIRECT_SET			= MSG_OPTION_SET | MSG_OPT_CONTACT_DIRECT;
	public static final int MSG_OPT_CONTACT_DIRECT_GET			= MSG_OPTION_GET | MSG_OPT_CONTACT_DIRECT;
	
	public static final int NOTIFY_TYPE_OPTIONS					= (MSG_TYPE_OPTIONS << 16);
	public static final int NOTIFY_PORTAL_LOCATION_CHANGED 		= NOTIFY_TYPE_OPTIONS | MSG_OPT_PORTAL_CENTER_SET;
	public static final int NOTIFY_PORTAL_SIZE_CHANGED 			= NOTIFY_TYPE_OPTIONS | MSG_OPT_PORTAL_WH_SET;
	public static final int NOTIFY_CONTACT_DIRECT_CHANGED 		= NOTIFY_TYPE_OPTIONS | MSG_OPT_CONTACT_DIRECT_SET;

	/**
	 * Notify options enumeration.
	 * */
	private static enum Notify_Options {
		type					(NOTIFY_TYPE_OPTIONS),
		PortalLocationChanged	(NOTIFY_PORTAL_LOCATION_CHANGED),
		PortalSizeChanged		(NOTIFY_PORTAL_SIZE_CHANGED),
		DirectContactChanged 	(NOTIFY_CONTACT_DIRECT_CHANGED),
		;

		public int value;
		private Notify_Options(int value) { this.value = value; }
		public int value() { return value; }
	}

	/*
	 * Native file types to app
	 */
	public static final int MSG_TYPE_FILE					= 3;
	/** Class: File type */
	public static final int NATIVE_FILE_TYPE 				= 0x400;	// File types
	public static final int NATIVE_FILE_TYPE_APP_DEFINED 	= NATIVE_FILE_TYPE;
	public static final int NATIVE_FILE_TYPE_EXT_DEFINED 	= NATIVE_FILE_TYPE | 1;
	public static final int NATIVE_FILE_TYPE_PARTS 			= NATIVE_FILE_TYPE | 6;
	public static final int NATIVE_FILE_TYPE_ACK 			= NATIVE_FILE_TYPE | 0xF;

	public static final int MSG_TYPE_MESSAGE				= 4;
	public static final int MESSAGE_FROM_APP 				= 0x800;
	public static final int MESSAGE_APP_STRING 				= MESSAGE_FROM_APP | 1;


	public static final int NOTIFY_TYPE_FILE				= (MSG_TYPE_FILE << 16);
	public static final int NOTIFY_TYPE_FILE_PROGRESS		= NOTIFY_TYPE_FILE | 0x20;

	public static final int NOTIFY_FILE_SEND_PROGRESS		= NOTIFY_TYPE_FILE_PROGRESS | 1;
	public static final int NOTIFY_FILE_RECEIVE_PROGRESS	= NOTIFY_TYPE_FILE_PROGRESS | 2;

	/**
	 * Notify file enumeration.
	 * */
	private static enum Notify_File {
		/* TEst comment */
		type				(NOTIFY_TYPE_FILE),
		FileProgress		(NOTIFY_TYPE_FILE_PROGRESS),
		SendProgress		(NOTIFY_FILE_SEND_PROGRESS),
		ReceiveProgress 	(NOTIFY_FILE_RECEIVE_PROGRESS),
		;

		public int value;
		private Notify_File(int value) { this.value = value; }
		public int value() { return value; }
		public static Notify_File get(int v) {
			switch (v) {
				case NOTIFY_TYPE_FILE: return type;
				case NOTIFY_TYPE_FILE_PROGRESS: return FileProgress;
				case NOTIFY_FILE_SEND_PROGRESS: return SendProgress;
				case NOTIFY_FILE_RECEIVE_PROGRESS: return ReceiveProgress;
			}
			return type;
		}
	}


	/*
	 * Environs options data identifiers for onData
	 */
	
	/*
	 * Native callback for errors
	 */
	public static final int NOTIFY_TOUCHSOURCE 			= 0x40;
	public static final int NOTIFY_TOUCHSOURCE_STARTED	= NOTIFY_TOUCHSOURCE | 2;
	public static final int NOTIFY_TOUCHSOURCE_STOPPED	= NOTIFY_TOUCHSOURCE | 4;
	
	public static final int NOTIFY_TOUCHSOURCE_NOTAVAIL	= NOTIFY_TOUCHSOURCE | 8;
	public static final int NOTIFY_TOUCHSOURCE_FAILED	= NOTIFY_TOUCHSOURCE | 9;

	/**
	 * Notify touchsource enumeration.
	 * */
	private static enum Notify_TouchSource {
		type			(NOTIFY_TOUCHSOURCE),
		Started			(NOTIFY_TOUCHSOURCE_STARTED),
		Stopped			(NOTIFY_TOUCHSOURCE_STOPPED),
		NotAvailable 	(NOTIFY_TOUCHSOURCE_NOTAVAIL),
		Failed 			(NOTIFY_TOUCHSOURCE_FAILED),
		;

		public int value;
		private Notify_TouchSource(int value) { this.value = value; }
		public int value() { return value; }
		public static Notify_TouchSource get(int v) {
			switch (v) {
				case NOTIFY_TOUCHSOURCE: return type;
				case NOTIFY_TOUCHSOURCE_STARTED: return Started;
				case NOTIFY_TOUCHSOURCE_STOPPED: return Stopped;
				case NOTIFY_TOUCHSOURCE_NOTAVAIL: return NotAvailable;
				case NOTIFY_TOUCHSOURCE_FAILED: return Failed;
			}
			return type;
		}
	}
	public static interface Tests {
		int type	=		(NOTIFY_TOUCHSOURCE);
		int Started	=		(NOTIFY_TOUCHSOURCE_STARTED);
		int Stopped	=		(NOTIFY_TOUCHSOURCE_STOPPED);
		int NotAvailable = 	(NOTIFY_TOUCHSOURCE_NOTAVAIL);
		int Failed 	=		(NOTIFY_TOUCHSOURCE_FAILED);
	}


	/**
	 * Environs human input declarations
	 */
	/**
	 * Input types for human input
	 * */
	public static final int INPUT_TYPE_CURSOR 		    = 0;
	public static final int INPUT_TYPE_FINGER 			= 1;
	public static final int INPUT_TYPE_PEN				= 2;
	public static final int INPUT_TYPE_MARKER			= 4;
	public static final int INPUT_TYPE_BLOB				= 8;

	/**
	 * Input type enumeration.
	 * */
	private static enum InputType {
		Cursor			(INPUT_TYPE_CURSOR),
		Finger			(INPUT_TYPE_FINGER),
		Pen				(INPUT_TYPE_PEN),
		Marker	 		(INPUT_TYPE_MARKER),
		Blob 			(INPUT_TYPE_BLOB),
		;

		public int value;
		private InputType(int value) { this.value = value; }
		public int value() { return value; }
		public static InputType get(int v) {
			switch (v) {
				case INPUT_TYPE_CURSOR: return Cursor;
				case INPUT_TYPE_FINGER: return Finger;
				case INPUT_TYPE_PEN: return Pen;
				case INPUT_TYPE_MARKER: return Marker;
				case INPUT_TYPE_BLOB: return Blob;
			}
			return Finger;
		}
	}


	/**
	 * Native callback for errors
	 */
	public static final int NATIVE_EVENT_ERROR_MISC 			= 0x80;
	public static final int NATIVE_EVENT_DATA_CON_FAILED 		= NATIVE_EVENT_ERROR_MISC | 3;
	public static final int NATIVE_EVENT_TOUCH_SOURCE_FAILED	= NATIVE_EVENT_ERROR_MISC | 4;

	/**
	 * Input states for a particular human input entity
	 * */
	public static final int INPUT_STATE_ADD 		    = 1;
	public static final int INPUT_STATE_CHANGE 			= 2;
	public static final int INPUT_STATE_NOCHANGE		= 3;
	public static final int INPUT_STATE_DROP 			= 4;

	/**
	 * Input state enumeration.
	 * */
	private static enum InputState {
		Add			(INPUT_STATE_ADD),
		Change		(INPUT_STATE_CHANGE),
		NoChange	(INPUT_STATE_NOCHANGE),
		Drop	 	(INPUT_STATE_DROP),
		;

		public int value;
		private InputState(int value) { this.value = value; }
		public int value() { return value; }
		public static InputState get(int v) {
			switch (v) {
				case INPUT_STATE_ADD: return Add;
				case INPUT_STATE_CHANGE: return Change;
				case INPUT_STATE_NOCHANGE: return NoChange;
				case INPUT_STATE_DROP: return Drop;
			}
			return Drop;
		}
	}

	/**
	 * Input commands for a particular human input entity
	 * */
	public static final int INPUTSOURCE_COMMAND_INIT 	= 0;
	public static final int INPUTSOURCE_COMMAND_ADD 	= 1;
	public static final int INPUTSOURCE_COMMAND_CHANGE 	= 2;
	public static final int INPUTSOURCE_COMMAND_DROP 	= 4;
	public static final int INPUTSOURCE_COMMAND_CANCEL 	= 6;
	public static final int INPUTSOURCE_COMMAND_FLUSH	= 8;
	public static final int INPUTSOURCE_COMMAND_FOLLOWUP = 0x80;


	/**
	 * Input source commands enumeration.
	 * */
	private static enum InputCommand {
		Init		(INPUTSOURCE_COMMAND_INIT),
		Add			(INPUTSOURCE_COMMAND_ADD),
		Change		(INPUTSOURCE_COMMAND_CHANGE),
		Drop		(INPUTSOURCE_COMMAND_DROP),
		Cancel	 	(INPUTSOURCE_COMMAND_CANCEL),
		Flush	 	(INPUTSOURCE_COMMAND_FLUSH),
		FollowUp 	(INPUTSOURCE_COMMAND_FOLLOWUP),
		;

		public int value;
		private InputCommand(int value) { this.value = value; }
		public int value() { return value; }
		public static InputCommand get(int v) {
			switch (v) {
				case INPUTSOURCE_COMMAND_INIT: return Init;
				case INPUTSOURCE_COMMAND_ADD: return Add;
				case INPUTSOURCE_COMMAND_CHANGE: return Change;
				case INPUTSOURCE_COMMAND_DROP: return Drop;
				case INPUTSOURCE_COMMAND_CANCEL: return Cancel;
				case INPUTSOURCE_COMMAND_FLUSH: return Flush;
				case INPUTSOURCE_COMMAND_FOLLOWUP: return FollowUp;
			}
			return Drop;
		}
	}

	/**
	 * Environs Start notifications
	 */
	/** Class: Environs type */
	public static final int MSG_TYPE_ENVIRONS				= 8;

	public static final int NOTIFY_TYPE_ENVIRONS			= (MSG_TYPE_ENVIRONS << 16);
	public static final int NOTIFY_START					= NOTIFY_TYPE_ENVIRONS | 0x100;
	public static final int NOTIFY_START_IN_PROGRESS		= NOTIFY_START | 1;
	public static final int NOTIFY_START_ENABLING_WIFI		= NOTIFY_START | 2;
	public static final int NOTIFY_START_STREAM_DECODER		= NOTIFY_START | 3;
	public static final int NOTIFY_START_INIT				= NOTIFY_START | 4;

	public static final int NOTIFY_START_INIT_FAILED 		= NOTIFY_START | 7;
	public static final int NOTIFY_START_METHOD_FAILED 		= NOTIFY_START | 8;
	public static final int NOTIFY_START_DECODER_FAILED		= NOTIFY_START | 9;
	public static final int NOTIFY_START_WIFI_FAILED		= NOTIFY_START | 10;
	public static final int NOTIFY_START_FAILED 			= NOTIFY_START | 11;

	public static final int NOTIFY_START_INIT_SUCCESS		= NOTIFY_START | 12;
	public static final int NOTIFY_START_SUCCESS			= NOTIFY_START | 13;
	public static final int NOTIFY_START_LISTEN_SUCCESS		= NOTIFY_START | 14;
	public static final int NOTIFY_START_LISTENDA_SUCCESS	= NOTIFY_START | 15;

	/**
	 * Environs Stop notifications
	 */
	public static final int NOTIFY_STOP						= NOTIFY_TYPE_ENVIRONS | 0x200;
	public static final int NOTIFY_STOP_BEGIN				= NOTIFY_STOP | 1;
	public static final int NOTIFY_STOP_IN_PROGRESS			= NOTIFY_STOP | 2;
	public static final int NOTIFY_STOP_FAILED				= NOTIFY_STOP | 10;
	public static final int NOTIFY_STOP_SUCCESS				= NOTIFY_STOP | 11;
	public static final int NOTIFY_STOP_RELEASED			= NOTIFY_STOP | 12;
	
	/**
	 * Environs socket notifications
	 */
	public static final int NOTIFY_SOCKET					= NOTIFY_TYPE_ENVIRONS | 0x400;
	public static final int NOTIFY_SOCKET_BIND_FAILED		= NOTIFY_SOCKET | 7;
	public static final int NOTIFY_SOCKET_LISTEN_FAILED		= NOTIFY_SOCKET | 8;
	public static final int NOTIFY_SOCKET_FAILED			= NOTIFY_SOCKET | 9;

	/**
	 * Environs socket notifications
	 */
	public static final int NOTIFY_SETTINGS					= NOTIFY_TYPE_ENVIRONS | 0x480;
	public static final int NOTIFY_SETTINGS_CHANGED			= NOTIFY_SETTINGS | 0x1;
	
	/**
	 * Environs device paring notifications
	 */
	public static final int NOTIFY_PAIRING					= NOTIFY_TYPE_ENVIRONS | 0x800;
	public static final int NOTIFY_DEVICE_ON_SURFACE		= NOTIFY_PAIRING | 1;
	public static final int NOTIFY_DEVICE_NOT_ON_SURFACE	= NOTIFY_PAIRING | 2;
	public static final int NOTIFY_DEVICE_FLAGS_UPDATE		= NOTIFY_PAIRING | 8;


	/**
	 * Notify environs enumeration.
	 * */
	private static enum Notify_Environs {
		type					(NOTIFY_TYPE_ENVIRONS),
		Start					(NOTIFY_START),
		StartInProgress 		(NOTIFY_START_IN_PROGRESS),
		StartEnablingWifi 		(NOTIFY_START_ENABLING_WIFI),
		StartStreamDecoder 		(NOTIFY_START_STREAM_DECODER),
		StartInit 				(NOTIFY_START_INIT),

		InitFailed 				(NOTIFY_START_INIT_FAILED),
		MethodFailed 			(NOTIFY_START_METHOD_FAILED),
		DecoderFailed 			(NOTIFY_START_DECODER_FAILED),
		WifiFailed 				(NOTIFY_START_WIFI_FAILED),
		StartFailed 			(NOTIFY_START_FAILED),

		StartInitSuccess 		(NOTIFY_START_INIT_SUCCESS),
		StartSuccess 			(NOTIFY_START_SUCCESS),
		StartListenSuccess 		(NOTIFY_START_LISTEN_SUCCESS),
		StartListenDataSuccess 	(NOTIFY_START_LISTENDA_SUCCESS),

		Stop 					(NOTIFY_STOP),
		StopBegin 				(NOTIFY_STOP_BEGIN),
		StopInProgress 			(NOTIFY_STOP_IN_PROGRESS),
		StopFailed 				(NOTIFY_STOP_FAILED),
		StopSuccess 			(NOTIFY_STOP_SUCCESS),
		StopReleased 			(NOTIFY_STOP_RELEASED),

		Socket 					(NOTIFY_SOCKET),
		SocketBindFailed 		(NOTIFY_SOCKET_BIND_FAILED),
		SocketListenFailed 		(NOTIFY_SOCKET_LISTEN_FAILED),
		SocketFailed 			(NOTIFY_SOCKET_FAILED),

		Settings 				(NOTIFY_SETTINGS),
		SettingsChanged 		(NOTIFY_SETTINGS_CHANGED),

		Pairing 				(NOTIFY_PAIRING),
		DeviceOnSurface 		(NOTIFY_DEVICE_ON_SURFACE),
		DeviceNotOnSurface 		(NOTIFY_DEVICE_NOT_ON_SURFACE),
		DeviceFlagsUpdate 		(NOTIFY_DEVICE_FLAGS_UPDATE),
		;

		public int value;
		private Notify_Environs(int value) { this.value = value; }
		public int value() { return value; }
		public static Notify_Environs get(int v) {
			switch (v) {
				case NOTIFY_TYPE_ENVIRONS: return type;
				case NOTIFY_START: return Start;
				case NOTIFY_START_IN_PROGRESS: return StartInProgress;
				case NOTIFY_START_ENABLING_WIFI: return StartEnablingWifi;
				case NOTIFY_START_STREAM_DECODER: return StartStreamDecoder;
				case NOTIFY_START_INIT: return StartInit;

				case NOTIFY_START_INIT_FAILED: return InitFailed;
				case NOTIFY_START_METHOD_FAILED: return MethodFailed;
				case NOTIFY_START_DECODER_FAILED: return DecoderFailed;
				case NOTIFY_START_WIFI_FAILED: return WifiFailed;
				case NOTIFY_START_FAILED: return StartFailed;

				case NOTIFY_START_INIT_SUCCESS: return StartInitSuccess;
				case NOTIFY_START_SUCCESS: return StartSuccess;
				case NOTIFY_START_LISTEN_SUCCESS: return StartListenSuccess;
				case NOTIFY_START_LISTENDA_SUCCESS: return StartListenDataSuccess;

				case NOTIFY_STOP: return Stop;
				case NOTIFY_STOP_BEGIN: return StopBegin;
				case NOTIFY_STOP_IN_PROGRESS: return StopInProgress;
				case NOTIFY_STOP_FAILED: return StopFailed;
				case NOTIFY_STOP_SUCCESS: return StopSuccess;
				case NOTIFY_STOP_RELEASED: return StopReleased;

				case NOTIFY_SOCKET: return Socket;
				case NOTIFY_SOCKET_BIND_FAILED: return SocketBindFailed;
				case NOTIFY_SOCKET_LISTEN_FAILED: return SocketListenFailed;
				case NOTIFY_SOCKET_FAILED: return SocketFailed;

				case NOTIFY_SETTINGS: return Settings;
				case NOTIFY_SETTINGS_CHANGED: return SettingsChanged;

				case NOTIFY_PAIRING: return Pairing;
				case NOTIFY_DEVICE_ON_SURFACE: return DeviceOnSurface;
				case NOTIFY_DEVICE_NOT_ON_SURFACE: return DeviceNotOnSurface;
			}
			return type;
		}
	}

	/**
	 * Environs Start notifications
	 */
	/** Class: Environs type */
	public static final int MSG_TYPE_SENSOR					= 9;
	public static final int MSG_TYPE_MAX_COUNT				= MSG_TYPE_SENSOR + 1;

	/**
	 * Environs mediator filter constants
	 */
	public static final int MEDIATOR_FILTER_NONE				= 0;
	public static final int MEDIATOR_FILTER_AREA				= 1;
	public static final int MEDIATOR_FILTER_AREA_AND_APP		= 2;
	public static final int MEDIATOR_FILTER_ALL					= 8; /// Disable all devicelist notifications


	/**
	 * Environs mediator filter enumeration.
	 * */
	private static enum MediatorFilter {
		None					(MEDIATOR_FILTER_NONE),
		Area					(MEDIATOR_FILTER_AREA),
		AreaAndApp				(MEDIATOR_FILTER_AREA_AND_APP),
		All 					(MEDIATOR_FILTER_ALL);

		public int value;
		private MediatorFilter(int value) { this.value = value; }
		public int value() { return value; }
		public static MediatorFilter get(int v) {
			switch (v) {
				case MEDIATOR_FILTER_NONE: return None;
				case MEDIATOR_FILTER_AREA: return Area;
				case MEDIATOR_FILTER_AREA_AND_APP: return AreaAndApp;
				case MEDIATOR_FILTER_ALL: return All;
			}
			return None;
		}
	}


	/**
	 * Environs mediator broadcast found values
	 */
	public static final int DEVICEINFO_DEVICE_MEDIATOR					= 0;
	public static final int DEVICEINFO_DEVICE_BROADCAST					= 1;
	public static final int DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR 	= 2;


	/**
	 * Device source type enumeration.
	 * */
	private static enum DeviceSourceType {
		Mediator			(DEVICEINFO_DEVICE_MEDIATOR),
		Broadcast			(DEVICEINFO_DEVICE_BROADCAST),
		MediatorBroadcast	(DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR),
		;

		public int value;
		private DeviceSourceType(int value) { this.value = value; }
		public int value() { return value; }
	}


	/**
	 * Environs mediator broadcast message Start bytes
	 */
	public static final int MEDIATOR_BROADCAST_DEVICETYPE_START		= 11;
	public static final int MEDIATOR_BROADCAST_DEVICEID_START		= 12;
	public static final int MEDIATOR_BROADCAST_DEVICEID_ABS_START	= 16;
	public static final int MEDIATOR_BROADCAST_PORTS_START			= 20;
	public static final int MEDIATOR_BROADCAST_PORTS_ABS_START		= 24;
	public static final int MEDIATOR_BROADCAST_PLATFORM_START		= 24;
	public static final int MEDIATOR_BROADCAST_PLATFORM_ABS_START	= 28;
	public static final int MEDIATOR_BROADCAST_DESC_START			= 28;
	public static final int MEDIATOR_BROADCAST_DESC_ABS_START		= 32;

	public static final int MEDIATOR_BROADCAST_STATUS_SRCDEVICEID_ABS_START	= 16;
	public static final int MEDIATOR_BROADCAST_STATUS_DEVICEID_ABS_START	= 20;
	public static final int MEDIATOR_BROADCAST_STATUS_CLEAR_SET_ABS_START	= 24;
	public static final int MEDIATOR_BROADCAST_STATUS_FLAGS_ABS_START		= 28;
	public static final int MEDIATOR_BROADCAST_STATUS_DESC_ABS_START		= 40;

	/**
	 * Environs mediator broadcast message constants
	 */
	public static final int TYPES_SEPERATOR_1_ENVIRONS			= 28;
	public static final int MEDIATOR_BROADCAST_SPARE_ID_LEN		= 28;

	/**
	 * Environs DeviceInstance struct Start bytes
	 */
	public static final int MAX_NAMEPROPERTY					= 30;
	/** Ignore: for Resolver */
	public static final int MAX_LENGTH_AREA_NAME				= MAX_NAMEPROPERTY + 1;
	/** Ignore: for Resolver */
	public static final int MAX_LENGTH_APP_NAME					= MAX_NAMEPROPERTY + 1;
	/** Ignore: for Resolver */
	public static final int MAX_LENGTH_DEVICE_NAME				= MAX_NAMEPROPERTY + 1;

	public static final int DEVICEINFO_DEVICEID_START	= 0;
	public static final int DEVICEINFO_NATIVE_ID_START	= 4;
	public static final int DEVICEINFO_IP_START			= DEVICEINFO_NATIVE_ID_START + 4;
	public static final int DEVICEINFO_IPe_START		= DEVICEINFO_IP_START + 4;
	public static final int DEVICEINFO_TCP_PORT_START	= DEVICEINFO_IPe_START + 4;
	public static final int DEVICEINFO_UDP_PORT_START	= DEVICEINFO_TCP_PORT_START + 2;
	public static final int DEVICEINFO_UPDATES_START	= DEVICEINFO_UDP_PORT_START + 2;
	public static final int DEVICEINFO_PLATFORM_START	= DEVICEINFO_UPDATES_START + 4;
	public static final int DEVICEINFO_BROADCAST_START	= DEVICEINFO_PLATFORM_START + 4;
	public static final int DEVICEINFO_UNAVAILABLE_START = DEVICEINFO_BROADCAST_START + 1;
	public static final int DEVICEINFO_ISCONNECTED_START = DEVICEINFO_UNAVAILABLE_START + 1;
	public static final int DEVICEINFO_HASAPPAREA_START = DEVICEINFO_ISCONNECTED_START + 2;
	//public static final int DEVICEINFO_DEVICETYPE_START	= DEVICEINFO_ISCONNECTED_START + 2;
	public static final int DEVICEINFO_DEVICENAME_START	= DEVICEINFO_HASAPPAREA_START + 1;
	public static final int DEVICEINFO_AREANAME_START 	= DEVICEINFO_DEVICENAME_START + (MAX_NAMEPROPERTY + 1);
	public static final int DEVICEINFO_APPNAME_START 	= DEVICEINFO_AREANAME_START + (MAX_NAMEPROPERTY + 1);
	public static final int DEVICEINFO_FLAGS_START 		= DEVICEINFO_APPNAME_START + (MAX_NAMEPROPERTY + 1);
	public static final int DEVICEINFO_OBJID_START 	 	= DEVICEINFO_FLAGS_START + 2;


	/**
	 * Environs mediator notifications
	 */
	public static final int NOTIFY_MEDIATOR						= NOTIFY_TYPE_ENVIRONS | 0x1000;
	public static final int NOTIFY_MEDIATOR_SERVER				= NOTIFY_MEDIATOR | 0x100;
	
	public static final int NOTIFY_MEDIATOR_DEVICE_CHANGED		= NOTIFY_MEDIATOR | 1;
	public static final int NOTIFY_MEDIATOR_DEVICE_ADDED		= NOTIFY_MEDIATOR | 2;
	public static final int NOTIFY_MEDIATOR_DEVICE_REMOVED		= NOTIFY_MEDIATOR | 4;
	public static final int NOTIFY_MEDIATOR_SERVER_CONNECTED	= NOTIFY_MEDIATOR | 20;
	public static final int NOTIFY_MEDIATOR_SERVER_DISCONNECTED	= NOTIFY_MEDIATOR | 21;

	public static final int NOTIFY_MEDIATOR_DEVICELISTS_UPDATE_AVAILABLE	= NOTIFY_MEDIATOR | 51;
	public static final int NOTIFY_MEDIATOR_DEVICELISTS_CHANGED	= NOTIFY_MEDIATOR | 52;

	public static final int NOTIFY_MEDIATOR_MED_CHANGED			= NOTIFY_MEDIATOR | 11;
	
	public static final int NOTIFY_MEDIATOR_SRV_DEVICE_CHANGED	= NOTIFY_MEDIATOR_DEVICE_CHANGED | NOTIFY_MEDIATOR_SERVER;
	public static final int NOTIFY_MEDIATOR_SRV_DEVICE_ADDED	= NOTIFY_MEDIATOR_DEVICE_ADDED | NOTIFY_MEDIATOR_SERVER;
	public static final int NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED	= NOTIFY_MEDIATOR_DEVICE_REMOVED | NOTIFY_MEDIATOR_SERVER;
	public static final int NOTIFY_MEDIATOR_SRV_STUNT_REG_REQ	= NOTIFY_MEDIATOR | 22 | NOTIFY_MEDIATOR_SERVER;
	
	public static final int NOTIFY_MEDIATOR_SERVER_PASSWORD_FAIL = NOTIFY_MEDIATOR | 41;
	public static final int NOTIFY_MEDIATOR_SERVER_PASSWORD_MISSING = NOTIFY_MEDIATOR | 42;


	/**
	 * Notify mediator enumeration.
	 * */
	private static enum Notify_Mediator {
		type						(NOTIFY_MEDIATOR),
		Server						(NOTIFY_MEDIATOR_SERVER),
		DeviceChanged				(NOTIFY_MEDIATOR_DEVICE_CHANGED),
		DeviceAdded 				(NOTIFY_MEDIATOR_DEVICE_ADDED),
		DeviceRemoved 				(NOTIFY_MEDIATOR_DEVICE_REMOVED),
		ServerConnected 			(NOTIFY_MEDIATOR_SERVER_CONNECTED),
		ServerDisconnected 			(NOTIFY_MEDIATOR_SERVER_DISCONNECTED),

		DeviceListUpdateAvailable 	(NOTIFY_MEDIATOR_DEVICELISTS_UPDATE_AVAILABLE),
		DeviceListChanged 			(NOTIFY_MEDIATOR_DEVICELISTS_CHANGED),

		MediatorChanged 			(NOTIFY_MEDIATOR_MED_CHANGED),

		ServerDeviceChanged 		(NOTIFY_MEDIATOR_SRV_DEVICE_CHANGED),
		ServerDeviceAdded 			(NOTIFY_MEDIATOR_SRV_DEVICE_ADDED),
		ServerDeviceRemoved 		(NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED),
		ServerSTUNTRegisterRequest 	(NOTIFY_MEDIATOR_SRV_STUNT_REG_REQ),

		ServerPasswordFail 			(NOTIFY_MEDIATOR_SERVER_PASSWORD_FAIL),
		ServerPasswordMissing 		(NOTIFY_MEDIATOR_SERVER_PASSWORD_MISSING),
		;

		public int value;
		private Notify_Mediator(int value) { this.value = value; }
		public int value() { return value; }
		public static Notify_Mediator get(int v) {
			switch (v) {
				case NOTIFY_MEDIATOR: return type;
				case NOTIFY_MEDIATOR_SERVER: return Server;
				case NOTIFY_MEDIATOR_DEVICE_CHANGED: return DeviceChanged;
				case NOTIFY_MEDIATOR_DEVICE_ADDED: return DeviceAdded;
				case NOTIFY_MEDIATOR_DEVICE_REMOVED: return DeviceRemoved;
				case NOTIFY_MEDIATOR_SERVER_CONNECTED: return ServerConnected;
				case NOTIFY_MEDIATOR_SERVER_DISCONNECTED: return ServerDisconnected;

				case NOTIFY_MEDIATOR_DEVICELISTS_UPDATE_AVAILABLE: return DeviceListUpdateAvailable;
				case NOTIFY_MEDIATOR_DEVICELISTS_CHANGED: return DeviceListChanged;

				case NOTIFY_MEDIATOR_MED_CHANGED: return MediatorChanged;

				case NOTIFY_MEDIATOR_SRV_DEVICE_CHANGED: return ServerDeviceChanged;
				case NOTIFY_MEDIATOR_SRV_DEVICE_ADDED: return ServerDeviceAdded;
				case NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED: return ServerDeviceRemoved;
				case NOTIFY_MEDIATOR_SRV_STUNT_REG_REQ: return ServerSTUNTRegisterRequest;

				case NOTIFY_MEDIATOR_SERVER_PASSWORD_FAIL: return ServerPasswordFail;
				case NOTIFY_MEDIATOR_SERVER_PASSWORD_MISSING: return ServerPasswordMissing;
			}
			return type;
		}
	}

	/**
	 * Deviceflags for internalFlags of DeviceInfo objects
	 */
	public static final int DEVICEFLAGS_INTERNAL_NATIVE_READY			= 0x1;
	public static final int DEVICEFLAGS_INTERNAL_PLATFORM_READY			= 0x2;
	public static final int DEVICEFLAGS_INTERNAL_OBSERVER_READY			= 0x4;
	public static final int DEVICEFLAGS_INTERNAL_MESSAGE_READY			= 0x8;
	public static final int DEVICEFLAGS_INTERNAL_DATA_READY 			= 0x10;
	public static final int DEVICEFLAGS_INTERNAL_SENSOR_READY 			= 0x20;
	public static final int DEVICEFLAGS_INTERNAL_NOTIFY_MASK 			= 0xFF;
	public static final int DEVICEFLAGS_INTERNAL_CP_NATIVE_READY		= 0x0100;
	public static final int DEVICEFLAGS_INTERNAL_CP_PLATFORM_READY		= 0x0200;
	public static final int DEVICEFLAGS_INTERNAL_CP_OBSERVER_READY		= 0x0400;
	public static final int DEVICEFLAGS_INTERNAL_CP_MESSAGE_READY		= 0x0800;
	public static final int DEVICEFLAGS_INTERNAL_CP_DATA_READY 			= 0x1000;
	public static final int DEVICEFLAGS_INTERNAL_CP_SENSOR_READY 		= 0x2000;
	public static final int DEVICEFLAGS_INTERNAL_CP_NOTIFY_MASK 		= 0xFF00;


	/**
	 * Deviceflags for internalFlags enumeration.
	 * */
	private static enum DeviceFlagsInternal {
		NativeReady				(DEVICEFLAGS_INTERNAL_NATIVE_READY),
		PlatformReady			(DEVICEFLAGS_INTERNAL_PLATFORM_READY),
		ObserverReady			(DEVICEFLAGS_INTERNAL_OBSERVER_READY),
		MessageReady			(DEVICEFLAGS_INTERNAL_MESSAGE_READY),
		DataReady				(DEVICEFLAGS_INTERNAL_DATA_READY),
		SensorReady				(DEVICEFLAGS_INTERNAL_SENSOR_READY),
		NotifyMask				(DEVICEFLAGS_INTERNAL_NOTIFY_MASK),

		CPNativeReady			(DEVICEFLAGS_INTERNAL_CP_NATIVE_READY),
		CPPlatformReady			(DEVICEFLAGS_INTERNAL_CP_PLATFORM_READY),
		CPObserverReady			(DEVICEFLAGS_INTERNAL_CP_OBSERVER_READY),
		CPMessageReady			(DEVICEFLAGS_INTERNAL_CP_MESSAGE_READY),
		CPDataReady				(DEVICEFLAGS_INTERNAL_CP_DATA_READY),
		CPSensorReady			(DEVICEFLAGS_INTERNAL_CP_SENSOR_READY),
		CPNotifyMask			(DEVICEFLAGS_INTERNAL_CP_NOTIFY_MASK),
		;

		public int value;
		private DeviceFlagsInternal(int value) { this.value = value; }
		public int value() { return value; }
	}


	/**
	 * Environs network notifications
	 */
	public static final int NOTIFY_NETWORK						= NOTIFY_TYPE_ENVIRONS | 0x2000;
	public static final int NOTIFY_NETWORK_CHANGED				= NOTIFY_NETWORK | 0x1;

	/**
	 * Notify network enumeration.
	 * */
	private static enum Notify_Network {
		type			(NOTIFY_NETWORK),
		Changed			(NOTIFY_NETWORK_CHANGED),
		;

		public int value;
		private Notify_Network(int value) { this.value = value; }
		public int value() { return value; }
		public static Notify_Network get(int v) {
			switch (v) {
				case NOTIFY_NETWORK: return type;
				case NOTIFY_NETWORK_CHANGED: return Changed;
			}
			return type;
		}
	}

	/** Ignore: for Resolver */
	public static final String META_MSG_IDENT						= "~META~:";
	/** Ignore: for Resolver */
	public static final String META_MSG_NAME_ID						= " NAME ";
	/** Ignore: for Resolver */
	public static final String ENVIRONS_DEFAULT_AREA_NAME			= "Environs";
	/** Ignore: for Resolver */
	public static final String ENVIRONS_DEFAULT_APP_NAME			= "HCMDefaultApp";
	/** Ignore: for Resolver */
	public static final String ENVIRONS_DEFAULT_DEVICE_NAME			= "DefaultDevice";
	/** Ignore: for Resolver */
	public static final int ENVIRONS_STUNT_MAX_TRY					= 15;
	/** Ignore: for Resolver */
	public static final int ENVIRONS_STUN_MAX_TRY					= 10;


	/**
	 * Environs network notifications
	 */
	public static final int NOTIFY_TRACKER						= NOTIFY_TYPE_ENVIRONS | 0x4000;

	public static final int NOTIFY_TRACKER_FAILED_FLAG			= 0x8;

	public static final int NOTIFY_TRACKER_ENABLED				= NOTIFY_TRACKER | 0x1;
	public static final int NOTIFY_TRACKER_CHANGED				= NOTIFY_TRACKER | 0x2;
	public static final int NOTIFY_TRACKER_DISABLED				= NOTIFY_TRACKER | 0x4;
	public static final int NOTIFY_TRACKER_ENABLE_FAILED		= NOTIFY_TRACKER | NOTIFY_TRACKER_FAILED_FLAG;

	public static final int NOTIFY_TRACKER_STATE_INIT_SENSOR	= NOTIFY_TRACKER | 0x10;
	public static final int NOTIFY_TRACKER_STATE_INIT_SENSOR_FAILED	= NOTIFY_TRACKER_STATE_INIT_SENSOR | NOTIFY_TRACKER_FAILED_FLAG;
	public static final int NOTIFY_TRACKER_STATE_START			= NOTIFY_TRACKER | 0x20;
	public static final int NOTIFY_TRACKER_STATE_START_FAILED	= NOTIFY_TRACKER | NOTIFY_TRACKER_STATE_START | NOTIFY_TRACKER_FAILED_FLAG;
	public static final int NOTIFY_TRACKER_STATE_STOP			= NOTIFY_TRACKER | 0x40;

	/**
	 * Notify tracker enumeration.
	 * */
	private static enum Notify_Tracker {
		type				(NOTIFY_TRACKER),
		Failed				(NOTIFY_TRACKER_FAILED_FLAG),

		Enabled				(NOTIFY_TRACKER_ENABLED),
		Changed 			(NOTIFY_TRACKER_CHANGED),
		Disabled 			(NOTIFY_TRACKER_DISABLED),
		EnableFailed 		(NOTIFY_TRACKER_ENABLE_FAILED),

		InitSensor 			(NOTIFY_TRACKER_STATE_INIT_SENSOR),
		InitSensorFailed 	(NOTIFY_TRACKER_STATE_INIT_SENSOR_FAILED),
		Start 				(NOTIFY_TRACKER_STATE_START),
		StartFailed 		(NOTIFY_TRACKER_STATE_START_FAILED),
		Stop 				(NOTIFY_TRACKER_STATE_STOP),
		;

		public int value;
		private Notify_Tracker(int value) { this.value = value; }
		public int value() { return value; }
		public static Notify_Tracker get(int v) {
			switch (v) {
				case NOTIFY_TRACKER: return type;
				case NOTIFY_TRACKER_FAILED_FLAG: return Failed;
				case NOTIFY_TRACKER_ENABLED: return Enabled;
				case NOTIFY_TRACKER_CHANGED: return Changed;
				case NOTIFY_TRACKER_DISABLED: return Disabled;
				case NOTIFY_TRACKER_ENABLE_FAILED: return EnableFailed;

				case NOTIFY_TRACKER_STATE_INIT_SENSOR: return InitSensor;
				case NOTIFY_TRACKER_STATE_INIT_SENSOR_FAILED: return InitSensorFailed;
				case NOTIFY_TRACKER_STATE_START: return Start;
				case NOTIFY_TRACKER_STATE_START_FAILED: return StartFailed;
				case NOTIFY_TRACKER_STATE_STOP: return Stop;
			}
			return type;
		}
	}


	/**
	 * Device types. Obsolete. Should not be used anymore.
	 * Type: char
	 */
	public static final char DEVICE_TYPE_DISPLAY					= 'D';
	public static final char DEVICE_TYPE_MULTITACTION				= 'M';
	public static final char DEVICE_TYPE_SURFACE1					= 'R';
	public static final char DEVICE_TYPE_SURFACE2					= 'S';
	public static final char DEVICE_TYPE_TABLET						= 'T';
	public static final char DEVICE_TYPE_UNKNOWN					= 'U';
	public static final char DEVICE_TYPE_SMARTPHONE					= 'P';


	/**
	 * Device queue commands.
	 * Type: int
	 */
	public static final int DEVICELIST_QUEUE_COMMAND_RELOAD			= 0;
	public static final int DEVICELIST_QUEUE_COMMAND_CLEAR			= 1;
	public static final int DEVICELIST_QUEUE_COMMAND_APPEND			= 2;
	public static final int DEVICELIST_QUEUE_COMMAND_UPDATE			= 3;
	public static final int DEVICELIST_QUEUE_COMMAND_INSERT_AT		= 4;
	public static final int DEVICELIST_QUEUE_COMMAND_REMOVE_AT		= 5;
	public static final int DEVICELIST_QUEUE_COMMAND_INSERT_CALL	= 6;
	public static final int DEVICELIST_QUEUE_COMMAND_DISPOSE_LIST	= 7;
	public static final int DEVICELIST_QUEUE_COMMAND_LOCK			= 8;


	/**
	 * Device display orientation types used in Device.Display.h
	 * Type: char
	 */
	public static final int DISPLAY_ORIENTATION_LANDSCAPE			= 0;
	public static final int DISPLAY_ORIENTATION_PORTRAIT			= 1;


	/**
	 * Device activity / connectivity flags
	 * Type: int
	 */
	public static final int DEVICE_ACTIVITY_MAIN_CONNECTED			= 0x1;
	public static final int DEVICE_ACTIVITY_COMDAT_CONNECTED		= 0x2;
	public static final int DEVICE_ACTIVITY_UDP_CONNECTED			= 0x4;
	public static final int DEVICE_ACTIVITY_CONNECTED				= 0x10;
	public static final int DEVICE_ACTIVITY_REQUESTOR				= 0x100;
	public static final int DEVICE_ACTIVITY_RESPONDER				= 0x200;
	public static final int DEVICE_ACTIVITY_LISTENER_CLOSED			= 0x8000;
	public static final int DEVICE_ACTIVITY_PLATFORM_DISPOSED		= 0x1000;
	public static final int DEVICE_ACTIVITY_PLATFORM_PREDISPOSED	= 0x2000;
	public static final int DEVICE_ACTIVITY_ABORTED					= 0x4000;

	public static final int DEVICE_ACTIVITY_MAIN_INIT				= 0x10000;
	public static final int DEVICE_ACTIVITY_COMDAT_INIT				= 0x20000;
	public static final int DEVICE_ACTIVITY_UDP_INIT				= 0x40000;


	/**
	 * Environs common native declarations
	 *  
	 */
	public static final int MEDIATOR_BUFFER_SIZE_MAX				= 65535;
	public static final int MEDIATOR_REC_BUFFER_SIZE_MAX_MOBILE		= (350 * 1024);
	public static final int MEDIATOR_REC_BUFFER_SIZE_MAX_FULL		= (650 * 1024);
	public static final int ENVIRONS_SEND_SIZE_MAX					= (40 * 1024 * 1024);



	/**
	 * Extension plugin interface type
	 * Type: int
	 */
	public static final int INTERFACE_TYPE_UNKNOWN					= 0;
	/** A Capture plugin grabs images from a capture source and provides the image buffer to the pipeline. */
	public static final int INTERFACE_TYPE_CAPTURE					= 1;
	/** A Render plugin renders a capture image (compare, rotate, scale, etc.). */
	public static final int INTERFACE_TYPE_RENDER					= 2;
	/** An Encoder encodes the rendered image to a target format / stream. */
	public static final int INTERFACE_TYPE_ENCODER					= 3;
	/** A Decoder decodes stream packets to images */
	public static final int INTERFACE_TYPE_DECODER					= 4;
	/** A Tracker that analyzes raw images for objects, touches, etc. */
	public static final int INTERFACE_TYPE_TRACKER					= 5;
	/** A Sensor that implements a sensor of type SensorType_t */
	public static final int INTERFACE_TYPE_SENSOR					= 6;
	/** A InputRecognizer is called back and provided a list of the current TouchDispatch state in order to perform gesture recognition. */
	public static final int INTERFACE_TYPE_INPUT_RECOGNIZER			= 10;
	/** A TouchRecognizer is called back and provided a list of the current TouchDispatch state in order to perform gesture recognition. */
	public static final int INTERFACE_TYPE_ORIENTATION_RECOGNIZER	= 11;
	/** A TouchRecognizer is called back and provided a list of the current TouchDispatch state in order to perform gesture recognition. */
	public static final int INTERFACE_TYPE_EXTENSION_THREAD			= 12;


	/** 
	 * Extension plugin interface type enumeration.
	 * */
	private static enum InterfaceType {
		Unknown					(INTERFACE_TYPE_UNKNOWN),
		/** A Capture plugin grabs images from a capture source and provides the image buffer to the pipeline. */
		Capture					(INTERFACE_TYPE_CAPTURE),
		/** A Render plugin renders a capture image (compare, rotate, scale, etc.). */
		Render					(INTERFACE_TYPE_RENDER),
		/** An Encoder encodes the rendered image to a target format / stream. */
		Encoder 				(INTERFACE_TYPE_ENCODER),
		/** A Decoder decodes stream packets to images */
		Decoder 				(INTERFACE_TYPE_DECODER),
		/** A Tracker that analyzes raw images for objects, touches, etc. */
		Tracker 				(INTERFACE_TYPE_TRACKER),
		/** A Sensor that implements a sensor of type SensorType_t */
		Sensor 					(INTERFACE_TYPE_SENSOR),
		/** A InputRecognizer is called back and provided a list of the current TouchDispatch state in order to perform gesture recognition. */
		InputRecognizer 		(INTERFACE_TYPE_INPUT_RECOGNIZER),
		/** A TouchRecognizer is called back and provided a list of the current TouchDispatch state in order to perform gesture recognition. */
		OrientationRecognizer 	(INTERFACE_TYPE_ORIENTATION_RECOGNIZER),
		/** An external implementation of a thread function. */
		ExtThread 				(INTERFACE_TYPE_EXTENSION_THREAD);

		public int value;
		private InterfaceType(int value) { this.value = value; }
		public int value() { return value; }
		public static InterfaceType get(int v) {
			switch (v) {
			case INTERFACE_TYPE_CAPTURE: return Capture;
			case INTERFACE_TYPE_RENDER: return Render;
			case INTERFACE_TYPE_ENCODER: return Encoder;
			case INTERFACE_TYPE_DECODER: return Decoder;
			case INTERFACE_TYPE_TRACKER: return Tracker;
			case INTERFACE_TYPE_SENSOR: return Sensor;
			case INTERFACE_TYPE_INPUT_RECOGNIZER: return InputRecognizer;
			case INTERFACE_TYPE_ORIENTATION_RECOGNIZER: return OrientationRecognizer;
			case INTERFACE_TYPE_EXTENSION_THREAD: return ExtThread;
			}
			return Unknown;
		}
	}


	/**
	 * Capture subtype
	 * Type: int
	 */
	public static final int CAPTURE_TYPE_UNKNOWN	= 0;
	/** A screen such as the dekstop window,
	 *  where the device may cover only part of the display.
	 *  The screen size must not be changed as long as the grabber class is used by at least one instance. */
	public static final int CAPTURE_TYPE_SCREEN		= 1;
	/** An application window, where each device may have a different app window and may cover only part of the window. */
	public static final int CAPTURE_TYPE_APP_WINDOW	= 2;
	/** Camera */
	public static final int CAPTURE_TYPE_CAMERA		= 6;

	/** 
	 * Capture subtype enumeration.
	 * */
	private static enum CaptureType {
		Unknown					(CAPTURE_TYPE_UNKNOWN),
		/** A screen such as the dekstop window, 
		 *  where the device may cover only part of the display.
		 *  The screen size must not be changed as long as the grabber class is used by at least one instance. */
		Screen					(CAPTURE_TYPE_SCREEN),
		/** An application window, where each device may have a different app window and may cover only part of the window. */
		AppWindow				(CAPTURE_TYPE_APP_WINDOW),
		/** Camera */
		Camera 					(CAPTURE_TYPE_CAMERA);
		
		public int value;		  
		private CaptureType(int value) { this.value = value; }		  
		public int value() { return value; }
		public static CaptureType get(int v) {
			switch (v) {
			case CAPTURE_TYPE_SCREEN: return Screen;
			case CAPTURE_TYPE_APP_WINDOW: return AppWindow;
			case CAPTURE_TYPE_CAMERA: return Camera;
			}
			return Unknown;
		}
	}


	/**
	 * Portal stage buffer data type
	 * Type: int
	 */
	public static final int PORTAL_BUFFERTYPE_UNKNOWN			= 0;
	/** Windows ARGB. */
	public static final int PORTAL_BUFFERTYPE_ARGB				= 0x1;
	/** Windows ARGB and the associated HBITMAP handle. */
	public static final int PORTAL_BUFFERTYPE_ARGB_HANDLE		= 0x2;
	/** iOS ARGB. */
	public static final int PORTAL_BUFFERTYPE_BGRA				= 0x3;
	/** RGB 24bit. */
	public static final int PORTAL_BUFFERTYPE_RGB				= 0x4;
	/** BGR 24bit. */
	public static final int PORTAL_BUFFERTYPE_BGR				= 0x5;
	/** I420. */
	public static final int PORTAL_BUFFERTYPE_YUV420			= 0x10;
	/** YV12. */
	public static final int PORTAL_BUFFERTYPE_YV12				= 0x12;
	/** YUY2. */
	public static final int PORTAL_BUFFERTYPE_YUV2				= 0x14;
	/** NV12. */
	public static final int PORTAL_BUFFERTYPE_NV12				= 0x16;
	/** GDIBitmap. */
	public static final int PORTAL_BUFFERTYPE_GDI_BITMAP		= 0x100;
	/** The data follows either D3D or OpenGL texture format. */
	public static final int PORTAL_BUFFERTYPE_TEXTURE_3D		= 0x1000;
	/** The data follows either D3D or OpenGL buffer format. */
	public static final int PORTAL_BUFFERTYPE_PIXELBUFFER_3D	= 0x2000;
	/** CVPixelBufferRef of apple platforms. */
	public static final int PORTAL_BUFFERTYPE_CVPIXELBUFFER_IOSX = 0x3000;

	/** 
	 * Portal stage buffer data type enumeration.
	 * */
	private static enum PortalBufferType {
		Unknown					(PORTAL_BUFFERTYPE_UNKNOWN),
		/** Windows ARGB. */
		ARGB					(PORTAL_BUFFERTYPE_ARGB),
		/** Windows ARGB and the associated HBITMAP handle. */
		ARGBHandle				(PORTAL_BUFFERTYPE_ARGB_HANDLE),
		/** iOS ARGB. */
		BGRA					(PORTAL_BUFFERTYPE_BGRA),
		/** RGB 24bit. */
		RGB						(PORTAL_BUFFERTYPE_RGB),
		/** BGR 24bit. */
		BGR						(PORTAL_BUFFERTYPE_BGR),
		/** I420. */
		YUV420					(PORTAL_BUFFERTYPE_YUV420),
		/** YV12. */
		YV12					(PORTAL_BUFFERTYPE_YV12),
		/** YUY2. */
		YUY2					(PORTAL_BUFFERTYPE_YUV2),
		/** NV12. */
		NV12					(PORTAL_BUFFERTYPE_NV12),
		/** GDIBitmap. */
		GDIBitmap				(PORTAL_BUFFERTYPE_GDI_BITMAP),
		/** The data follows either D3D or OpenGL texture format. */
		Texture3D				(PORTAL_BUFFERTYPE_TEXTURE_3D),
		/** The data follows either D3D or OpenGL buffer format. */
		PixelBuffer3D			(PORTAL_BUFFERTYPE_PIXELBUFFER_3D),
		/** CVPixelBufferRef of apple platforms. */
		CVPixelBufferIOSX		(PORTAL_BUFFERTYPE_CVPIXELBUFFER_IOSX);
		
		public int value;		  
		private PortalBufferType(int value) { this.value = value; }		  
		public int value() { return value; }
		public static PortalBufferType get(int v) {
			switch (v) {
				case PORTAL_BUFFERTYPE_ARGB: return ARGB;
				case PORTAL_BUFFERTYPE_ARGB_HANDLE: return ARGBHandle;
				case PORTAL_BUFFERTYPE_BGRA: return BGRA;
				case PORTAL_BUFFERTYPE_RGB: return RGB;
				case PORTAL_BUFFERTYPE_BGR: return BGR;
				case PORTAL_BUFFERTYPE_YUV420: return YUV420;
				case PORTAL_BUFFERTYPE_YV12: return YV12;
				case PORTAL_BUFFERTYPE_YUV2: return YUY2;
				case PORTAL_BUFFERTYPE_NV12: return NV12;
				case PORTAL_BUFFERTYPE_GDI_BITMAP: return GDIBitmap;
				case PORTAL_BUFFERTYPE_TEXTURE_3D: return Texture3D;
				case PORTAL_BUFFERTYPE_PIXELBUFFER_3D: return PixelBuffer3D;
				case PORTAL_BUFFERTYPE_CVPIXELBUFFER_IOSX: return CVPixelBufferIOSX;
			}
			return Unknown;
		}
	}


	/**
	 * Capture plugin data buffer type
	 * Type: int
	 */
	public static final int CAPTURE_BUFFERTYPE_UNKNOWN			= 0;
	/** The data is stored as pixel data usually in rgba order. */
	public static final int CAPTURE_BUFFERTYPE_PIXELBUFFER		= 1;
	/** The data follows either D3D or OpenGL texture format. */
	public static final int CAPTURE_BUFFERTYPE_TEXTURE_3D		= 10;
	/** The data follows either D3D or OpenGL buffer format. */
	public static final int CAPTURE_BUFFERTYPE_PIXELBUFFER_3D	= 11;

	/**
	 * Capture plugin data buffer type enumeration.
	 * */
	private static enum CaptureBufferType {
		Unknown					(CAPTURE_BUFFERTYPE_UNKNOWN),
		/** The data is stored as pixel data usually in rgba order. */
		PixelBuffer				(CAPTURE_BUFFERTYPE_PIXELBUFFER),
		/** The data follows either D3D or OpenGL texture format. */
		Texture3D				(CAPTURE_BUFFERTYPE_TEXTURE_3D),
		/** The data follows either D3D or OpenGL buffer format. */
		PixelBuffer3D			(CAPTURE_BUFFERTYPE_PIXELBUFFER_3D);

		public int value;
		private CaptureBufferType(int value) { this.value = value; }
		public int value() { return value; }
		public static CaptureBufferType get(int v) {
			switch (v) {
				case CAPTURE_BUFFERTYPE_PIXELBUFFER: return PixelBuffer;
				case CAPTURE_BUFFERTYPE_TEXTURE_3D: return Texture3D;
				case CAPTURE_BUFFERTYPE_PIXELBUFFER_3D: return PixelBuffer3D;
			}
			return Unknown;
		}
	}


	/**
	 * Encoder buffer data type
	 * Type: int
	 */
	public static final int ENCODER_BUFFERTYPE_UNKNOWN			= 0;
	/** Windows ARGB. */
	public static final int ENCODER_BUFFERTYPE_ARGB				= 1;
	/** I420. */
	public static final int ENCODER_BUFFERTYPE_YUV420			= 10;

	/** 
	 * Encoder buffer data type enumeration.
	 * */
	private static enum EncoderBufferType {
		Unknown					(ENCODER_BUFFERTYPE_UNKNOWN),
		/** Windows ARGB. */
		ARGB					(ENCODER_BUFFERTYPE_ARGB),
		/** I420. */
		YUV420					(ENCODER_BUFFERTYPE_YUV420);
		
		public int value;		  
		private EncoderBufferType(int value) { this.value = value; }		  
		public int value() { return value; }
		public static EncoderBufferType get(int v) {
			switch (v) {
			case ENCODER_BUFFERTYPE_ARGB: return ARGB;
			case ENCODER_BUFFERTYPE_YUV420: return YUV420;
			}
			return Unknown;
		}
	}


	/**
	 * Input recognizer states
	 * Type: int
	 */
	public static final int RECOGNIZER_GIVE_BACK_INPUTS				= -1;
	public static final int RECOGNIZER_REJECT						= 0;
	public static final int RECOGNIZER_HANDLED						= 1;
	public static final int RECOGNIZER_TAKEN_OVER_INPUTS			= 2;


	public static final int NETWORK_CONNECTION_TRIGGER_UPDATE		= -3;
	public static final int NETWORK_CONNECTION_UNKNOWN				= -2;
	public static final int NETWORK_CONNECTION_NO_NETWORK			= -1;
	public static final int NETWORK_CONNECTION_NO_INTERNET			= 0;
	public static final int NETWORK_CONNECTION_MOBILE_DATA			= 1;
	public static final int NETWORK_CONNECTION_WIFI					= 2;
	public static final int NETWORK_CONNECTION_LAN					= 3;

	/**
	 * Network connection enumeration.
	 * */
	private static enum NetworkConnection {
		TriggerUpdate	(NETWORK_CONNECTION_TRIGGER_UPDATE),
		Unknown			(NETWORK_CONNECTION_UNKNOWN),
		NoNetwork		(NETWORK_CONNECTION_NO_NETWORK),
		NoInternet		(NETWORK_CONNECTION_NO_INTERNET),
		MobileData		(NETWORK_CONNECTION_MOBILE_DATA),
		WiFi			(NETWORK_CONNECTION_WIFI),
		LAN				(NETWORK_CONNECTION_LAN);

		public int value;
		private NetworkConnection(int value) { this.value = value; }
		public int value() { return value; }
	}


	public static final int ERR_LEVEL								= -1;
	public static final int WARN_LEVEL								= -2;


	public static final int DEVICE_INFO_ATTR_DISPOSED				= 0x1;
	public static final int DEVICE_INFO_ATTR_ISCONNECTED			= 0x2;
	public static final int DEVICE_INFO_ATTR_CONNECT_PROGRESS		= 0x4;
	public static final int DEVICE_INFO_ATTR_USER_NAME				= 0x10;

	public static final int DEVICE_INFO_ATTR_IDENTITY				= 0x20;

	public static final int DEVICE_INFO_ATTR_DEVICE_PLATFORM		= 0x40;
	public static final int DEVICE_INFO_ATTR_DEVICE_TYPE			= 0x41;
	public static final int DEVICE_INFO_ATTR_NATIVEID				= 0x80;
	public static final int DEVICE_INFO_ATTR_IP						= 0x100;
	public static final int DEVICE_INFO_ATTR_IPE					= 0x200;
	public static final int DEVICE_INFO_ATTR_TCP_PORT				= 0x400;
	public static final int DEVICE_INFO_ATTR_UDP_PORT				= 0x800;
	public static final int DEVICE_INFO_ATTR_UNAVAILABLE			= 0x1000;
	public static final int DEVICE_INFO_ATTR_BROADCAST_FOUND		= 0x2000;
	public static final int DEVICE_INFO_ATTR_DIRECT_CONTACT			= 0x4000;
	public static final int DEVICE_INFO_ATTR_APP_CONTEXT			= 0x8000;
	public static final int DEVICE_INFO_ATTR_PORTAL_CREATED			= 0x10000;
	public static final int DEVICE_INFO_ATTR_OBJID					= 0x20000;
	public static final int DEVICE_INFO_ATTR_FLAGS					= 0x40000;


	/**
	 * DeviceInfoAttr attribute enumeration.
	 * */
	private static enum DeviceInfoFlag {
		Disposed			(ENVIRONS_OBJECT_DISPOSED),
		DisposedChanged		(DEVICE_INFO_ATTR_DISPOSED),
		IsConnected			(DEVICE_INFO_ATTR_ISCONNECTED),
		ConnectProgress		(DEVICE_INFO_ATTR_CONNECT_PROGRESS),
		UserName			(DEVICE_INFO_ATTR_USER_NAME),
		Identity			(DEVICE_INFO_ATTR_IDENTITY),
		Platform			(DEVICE_INFO_ATTR_DEVICE_PLATFORM),
		DeviceType			(DEVICE_INFO_ATTR_DEVICE_TYPE),
		NativeID			(DEVICE_INFO_ATTR_NATIVEID),
		IP					(DEVICE_INFO_ATTR_IP),
		IPe					(DEVICE_INFO_ATTR_IPE),
		TcpPort				(DEVICE_INFO_ATTR_TCP_PORT),
		UdpPort				(DEVICE_INFO_ATTR_UDP_PORT),
		Unavailable			(DEVICE_INFO_ATTR_UNAVAILABLE),
		BroadcastFound		(DEVICE_INFO_ATTR_BROADCAST_FOUND),
		DirectContact		(DEVICE_INFO_ATTR_DIRECT_CONTACT),
		AppContext			(DEVICE_INFO_ATTR_APP_CONTEXT),
		PortalCreated		(DEVICE_INFO_ATTR_PORTAL_CREATED),
		ObjectID			(DEVICE_INFO_ATTR_OBJID),
		Flags				(DEVICE_INFO_ATTR_FLAGS),
		;

		public int value;
		private DeviceInfoFlag(int value) { this.value = value; }
		public int value() { return value; }
		public static DeviceInfoFlag get(int v) {
			switch (v) {
				case DEVICE_INFO_ATTR_DISPOSED: return DisposedChanged;
				case DEVICE_INFO_ATTR_ISCONNECTED: return IsConnected;
				case DEVICE_INFO_ATTR_CONNECT_PROGRESS: return ConnectProgress;
				case DEVICE_INFO_ATTR_USER_NAME: return UserName;
				case DEVICE_INFO_ATTR_IDENTITY: return Identity;
				case DEVICE_INFO_ATTR_DEVICE_PLATFORM: return Platform;
				case DEVICE_INFO_ATTR_DEVICE_TYPE: return DeviceType;
				case DEVICE_INFO_ATTR_NATIVEID: return NativeID;
				case DEVICE_INFO_ATTR_IP: return IP;
				case DEVICE_INFO_ATTR_IPE: return IPe;
				case DEVICE_INFO_ATTR_TCP_PORT: return TcpPort;
				case DEVICE_INFO_ATTR_UDP_PORT: return UdpPort;
				case DEVICE_INFO_ATTR_UNAVAILABLE: return Unavailable;
				case DEVICE_INFO_ATTR_BROADCAST_FOUND: return BroadcastFound;
				case DEVICE_INFO_ATTR_DIRECT_CONTACT: return DirectContact;
				case DEVICE_INFO_ATTR_APP_CONTEXT: return AppContext;
				case DEVICE_INFO_ATTR_PORTAL_CREATED: return PortalCreated;
				case DEVICE_INFO_ATTR_OBJID: return ObjectID;
			}
			return Disposed;
		}
	}


	public static final int FILE_INFO_ATTR_CREATED					= 0x10000;
	public static final int FILE_INFO_ATTR_AVAILABLE				= 0x20000;
	public static final int FILE_INFO_ATTR_SEND_PROGRESS			= 0x40000;
	public static final int FILE_INFO_ATTR_RECEIVE_PROGRESS			= 0x80000;


	/**
	 * FileInfo attribute enumeration.
	 * */
	private static enum FileInfoFlag {
		Disposed			(ENVIRONS_OBJECT_DISPOSED),
		Created				(FILE_INFO_ATTR_CREATED),
		Available			(FILE_INFO_ATTR_AVAILABLE),
		SendProgress		(FILE_INFO_ATTR_SEND_PROGRESS),
		ReceiveProgress		(FILE_INFO_ATTR_RECEIVE_PROGRESS);

		public int value;
		private FileInfoFlag(int value) { this.value = value; }
		public int value() { return value; }
		public static FileInfoFlag get(int v) {
			switch (v) {
				case FILE_INFO_ATTR_CREATED: return Created;
				case FILE_INFO_ATTR_AVAILABLE: return Available;
				case FILE_INFO_ATTR_SEND_PROGRESS: return SendProgress;
				case FILE_INFO_ATTR_RECEIVE_PROGRESS: return ReceiveProgress;
			}
			return Available;
		}
	}

	public static final int MESSAGE_INFO_ATTR_CREATED				= 0x200000;

	/**
	 * MessageInfo attribute enumeration.
	 * */
	private static enum MessageInfoFlag {
		Disposed			(ENVIRONS_OBJECT_DISPOSED),
		Created				(MESSAGE_INFO_ATTR_CREATED),
		;

		public int value;
		private MessageInfoFlag(int value) { this.value = value; }
		public int value() { return value; }
		public static MessageInfoFlag get(int v) {
			return Created;
		}
	}


	public static final int APP_STATUS_ACTIVE						= 0;
	public static final int APP_STATUS_SLEEPING						= 1;

    public static final int ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT		= 60;

	public static final int MAX_TCP_SEND_PACKET_SIZE				= 1200000;


	/**
	 * Environs detectable platform constants
	 * Type: int
	 */
	public static final int ENVIRONS_PLATFORMS_UNKNOWN				= 0;
	/** MS Surface Tabletops */
	public static final int ENVIRONS_PLATFORMS_MSSURFACE_FLAG		= 0x1000;
	/** Surface 1 tabletop */
	public static final int ENVIRONS_PLATFORMS_MSSUR01				= 0x1001;
	/** Samsung SUR40 PixelSense */
	public static final int ENVIRONS_PLATFORMS_SAMSUR40				= 0x1002;
	/** iPad */
	public static final int ENVIRONS_PLATFORMS_IPAD_FLAG			= 0x2000;
	public static final int ENVIRONS_PLATFORMS_IPAD1				= 0x2011;
	public static final int ENVIRONS_PLATFORMS_IPAD2				= 0x2021;
	public static final int ENVIRONS_PLATFORMS_IPAD2MINI			= 0x2022;
	public static final int ENVIRONS_PLATFORMS_IPAD3				= 0x2031;
	public static final int ENVIRONS_PLATFORMS_IPAD4				= 0x2041;
	public static final int ENVIRONS_PLATFORMS_IPAD4AIR				= 0x2042;
	public static final int ENVIRONS_PLATFORMS_IPAD4MINI			= 0x2043;
	public static final int ENVIRONS_PLATFORMS_IPAD4MINI3			= 0x2044;
	public static final int ENVIRONS_PLATFORMS_IPAD5AIR2			= 0x2051;
	/** iPhones */
	public static final int ENVIRONS_PLATFORMS_IPHONE_FLAG			= 0x4000;
	public static final int ENVIRONS_PLATFORMS_IPHONE4				= 0x4041;
	public static final int ENVIRONS_PLATFORMS_IPHONE5				= 0x4051;
	public static final int ENVIRONS_PLATFORMS_IPHONE6				= 0x4061;
	public static final int ENVIRONS_PLATFORMS_IPHONE6P				= 0x4062;
	/** MultiTaction Cells */
	public static final int ENVIRONS_PLATFORMS_MULTITACTION_FLAG	= 0x8000;
	/** MultiTaction Cell 55. */
	public static final int ENVIRONS_PLATFORMS_MULTITACTION55		= 0x8055;

	public static final int ENVIRONS_PLATFORMS_OSX_FLAG				= 0x10000;
	public static final int ENVIRONS_PLATFORMS_MACBOOK_FLAG			= 0x10010;
	public static final int ENVIRONS_PLATFORMS_MACMINI_FLAG			= 0x10020;

	public static final int ENVIRONS_PLATFORMS_LINUX_FLAG			= 0x40000;
	public static final int ENVIRONS_PLATFORMS_RASPBERRY			= 0x40100;

	public static final int ENVIRONS_PLATFORMS_WINDOWS_FLAG			= 0x20000;
	public static final int ENVIRONS_PLATFORMS_WINDOWSVISTA			= 0x20050;
	public static final int ENVIRONS_PLATFORMS_WINDOWSXP			= 0x20060;
	public static final int ENVIRONS_PLATFORMS_WINDOWS7				= 0x20070;
	public static final int ENVIRONS_PLATFORMS_WINDOWS8				= 0x20080;
	public static final int ENVIRONS_PLATFORMS_WINDOWS10			= 0x20100;

	public static final int ENVIRONS_PLATFORMS_TABLET_FLAG			= 0x100000;
	public static final int ENVIRONS_PLATFORMS_SMARTPHONE_FLAG		= 0x200000;
	public static final int ENVIRONS_PLATFORMS_TABLETOP_FLAG		= 0x400000;
	public static final int ENVIRONS_PLATFORMS_DISPLAY_FLAG			= 0x800000;

	public static final int ENVIRONS_PLATFORMS_LOCATIONNODE_FLAG	= 0x1000000;

	/**
	 * Environs detectable platforms.
	 * */
	private static enum Platforms {
		Unknown					(ENVIRONS_PLATFORMS_UNKNOWN),
		/** MS Surface Tabletops */
		MSSurface_Flag			(ENVIRONS_PLATFORMS_MSSURFACE_FLAG),
		/** Surface 1 tabletop */
		MSSUR01					(ENVIRONS_PLATFORMS_MSSUR01),
		/** Samsung SUR40 PixelSense */
		SAMSUR40				(ENVIRONS_PLATFORMS_SAMSUR40),

		/** iPad */
		iPad_Flag				(ENVIRONS_PLATFORMS_IPAD_FLAG),
		iPad1					(ENVIRONS_PLATFORMS_IPAD1),
		iPad2					(ENVIRONS_PLATFORMS_IPAD2),
		iPad2Mini				(ENVIRONS_PLATFORMS_IPAD2MINI),
		iPad3					(ENVIRONS_PLATFORMS_IPAD3),
		iPad4					(ENVIRONS_PLATFORMS_IPAD4),
		iPad4Air				(ENVIRONS_PLATFORMS_IPAD4AIR),
		iPad4Mini				(ENVIRONS_PLATFORMS_IPAD4MINI),
		iPad4Mini3				(ENVIRONS_PLATFORMS_IPAD4MINI3),
		iPad5Air2				(ENVIRONS_PLATFORMS_IPAD5AIR2),

		/** iPhones */
		iPhone_Flag				(ENVIRONS_PLATFORMS_IPHONE_FLAG),
		iPhone4					(ENVIRONS_PLATFORMS_IPHONE4),
		iPhone5					(ENVIRONS_PLATFORMS_IPHONE5),
		iPhone6					(ENVIRONS_PLATFORMS_IPHONE6),
		iPhone6p				(ENVIRONS_PLATFORMS_IPHONE6P),

		/** MultiTaction Cells */
		MultiTaction_Flag		(ENVIRONS_PLATFORMS_MULTITACTION_FLAG),
		/** MultiTaction Cell 55. */
		MultiTaction55			(ENVIRONS_PLATFORMS_MULTITACTION55),

		OSX_Flag				(ENVIRONS_PLATFORMS_OSX_FLAG),
		MacBook_Flag			(ENVIRONS_PLATFORMS_MACBOOK_FLAG),
		MacMini_Flag			(ENVIRONS_PLATFORMS_MACMINI_FLAG),

		Linux_Flag				(ENVIRONS_PLATFORMS_LINUX_FLAG),
		Raspberry_Flag			(ENVIRONS_PLATFORMS_RASPBERRY),

		Windows_Flag			(ENVIRONS_PLATFORMS_WINDOWS_FLAG),
		WindowsVista			(ENVIRONS_PLATFORMS_WINDOWSVISTA),
		WindowsXP				(ENVIRONS_PLATFORMS_WINDOWSXP),
		Windows7				(ENVIRONS_PLATFORMS_WINDOWS7),
		Windows8				(ENVIRONS_PLATFORMS_WINDOWS8),
		Windows10				(ENVIRONS_PLATFORMS_WINDOWS10),

		Tablet_Flag				(ENVIRONS_PLATFORMS_TABLET_FLAG),
		Smartphone_Flag			(ENVIRONS_PLATFORMS_SMARTPHONE_FLAG),
		Tabletop_Flag			(ENVIRONS_PLATFORMS_TABLETOP_FLAG),
		Display_Flag			(ENVIRONS_PLATFORMS_DISPLAY_FLAG),

		LocationNode_Flag		(ENVIRONS_PLATFORMS_LOCATIONNODE_FLAG),
		;

		public int value;
		private Platforms(int value) { this.value = value; }
		public int value() { return value; }
		public static Platforms get(int v) {
			switch (v) {
				case ENVIRONS_PLATFORMS_MSSURFACE_FLAG: return MSSurface_Flag;
				case ENVIRONS_PLATFORMS_MSSUR01: return MSSUR01;
				case ENVIRONS_PLATFORMS_SAMSUR40: return SAMSUR40;

				case ENVIRONS_PLATFORMS_IPAD_FLAG: return iPad_Flag;
				case ENVIRONS_PLATFORMS_IPAD1: return iPad1;
				case ENVIRONS_PLATFORMS_IPAD2: return iPad2;
				case ENVIRONS_PLATFORMS_IPAD2MINI: return iPad2Mini;
				case ENVIRONS_PLATFORMS_IPAD3: return iPad3;
				case ENVIRONS_PLATFORMS_IPAD4: return iPad4;
				case ENVIRONS_PLATFORMS_IPAD4AIR: return iPad4Air;
				case ENVIRONS_PLATFORMS_IPAD4MINI: return iPad4Mini;
				case ENVIRONS_PLATFORMS_IPAD4MINI3: return iPad4Mini3;
				case ENVIRONS_PLATFORMS_IPAD5AIR2: return iPad5Air2;

				case ENVIRONS_PLATFORMS_IPHONE_FLAG: return iPhone_Flag;
				case ENVIRONS_PLATFORMS_IPHONE4: return iPhone4;
				case ENVIRONS_PLATFORMS_IPHONE5: return iPhone5;
				case ENVIRONS_PLATFORMS_IPHONE6: return iPhone6;
				case ENVIRONS_PLATFORMS_IPHONE6P: return iPhone6p;

				case ENVIRONS_PLATFORMS_MULTITACTION_FLAG: return MultiTaction_Flag;
				case ENVIRONS_PLATFORMS_MULTITACTION55: return MultiTaction55;

				case ENVIRONS_PLATFORMS_OSX_FLAG: return OSX_Flag;
				case ENVIRONS_PLATFORMS_MACBOOK_FLAG: return MacBook_Flag;
				case ENVIRONS_PLATFORMS_MACMINI_FLAG: return MacMini_Flag;

				case ENVIRONS_PLATFORMS_WINDOWS_FLAG: return Windows_Flag;
				case ENVIRONS_PLATFORMS_WINDOWSVISTA: return WindowsVista;
				case ENVIRONS_PLATFORMS_WINDOWSXP: return WindowsXP;
				case ENVIRONS_PLATFORMS_WINDOWS7: return Windows7;
				case ENVIRONS_PLATFORMS_WINDOWS8: return Windows8;
				case ENVIRONS_PLATFORMS_WINDOWS10: return Windows10;

				case ENVIRONS_PLATFORMS_TABLET_FLAG: return Tablet_Flag;
				case ENVIRONS_PLATFORMS_SMARTPHONE_FLAG: return Smartphone_Flag;
				case ENVIRONS_PLATFORMS_TABLETOP_FLAG: return Tabletop_Flag;
				case ENVIRONS_PLATFORMS_DISPLAY_FLAG: return Display_Flag;

				case ENVIRONS_PLATFORMS_LOCATIONNODE_FLAG: return LocationNode_Flag;
			}
			return Unknown;
		}
	}

	/*
	 * Mediator device class types used for GetDevicesFrom ( type )
	 */
	public static final int MEDIATOR_DEVICE_CLASS_ALL 			= 0;
	public static final int MEDIATOR_DEVICE_CLASS_NEARBY		= 1;
	public static final int MEDIATOR_DEVICE_CLASS_MEDIATOR		= 2;

	/*
	 * Crypt declarations
	 */
	public static final int ENVIRONS_DEVICES_KEYSIZE 		= 2048;
	public static final int ENVIRONS_CRYPT_PAD_OAEP 		= (1 << 24);
	public static final int ENVIRONS_CRYPT_PAD_PKCS1 		= (2 << 24);
	public static final int ENVIRONS_CRYPT_PAD_PKCS1SHA1 	= (4 << 24);
	public static final int ENVIRONS_CRYPT_PAD_PKCS1SHA256 	= (8 << 24);

	public static final int MEDIATOR_CLIENT_MAX_BUFFER_SIZE	= 0x1FFFF;
	public static final int DEVICE_HANDSHAKE_BUFFER_MAX_SIZE = MEDIATOR_CLIENT_MAX_BUFFER_SIZE;


	/**
	 * Mediator device class types enumeration.
	 * */
	private static enum DeviceClass {
		All				(MEDIATOR_DEVICE_CLASS_ALL),
		Nearby			(MEDIATOR_DEVICE_CLASS_NEARBY),
		Mediator		(MEDIATOR_DEVICE_CLASS_MEDIATOR),
		;

		public int value;
		private DeviceClass(int value) { this.value = value; }
		public int value() { return value; }
		public static DeviceClass get(int v) {
			switch (v) {
				case MEDIATOR_DEVICE_CLASS_ALL: return All;
				case MEDIATOR_DEVICE_CLASS_NEARBY: return Nearby;
				case MEDIATOR_DEVICE_CLASS_MEDIATOR: return Mediator;
			}
			return All;
		}
	}

	/**
	 * Environs call flags
	 * Type: int
	 */
	public static final int CALL_WAIT			= 0;
	public static final int CALL_NOWAIT			= 1;


	/**
	 * Environs call enumeration.
	 * */
	private static enum Call {
		Wait			(CALL_WAIT),
		NoWait			(CALL_NOWAIT),
		;

		public int value;
		private Call(int value) { this.value = value; }
		public int value() { return value; }
		public static Call get(int v) {
			return ( v == Types.CALL_WAIT ? Wait : NoWait );
		}
	}


	/**
	 * Environs RENDER_CALLBACK_TYPES
	 * Type: int
	 */
	public static final int RENDER_CALLBACK_TYPE_ALL			= 0;
	/** Call back with received ByteBuffer */
	public static final int RENDER_CALLBACK_TYPE_INIT			= 0x10;
	/** Call back with EnvironsAVContext */
	public static final int RENDER_CALLBACK_TYPE_AVCONTEXT		= 0x20;
	/** Call back with IPortalDecoder */
	public static final int RENDER_CALLBACK_TYPE_DECODER		= 0x40;
	/** Call back with received ByteBuffer */
	public static final int RENDER_CALLBACK_TYPE_IMAGE			= 0x80;

	/**
	 * Environs RENDER_CALLBACK_TYPES enumeration.
	 * */
	private static enum RenderCallbackType {
		All				(RENDER_CALLBACK_TYPE_ALL),
		Init			(RENDER_CALLBACK_TYPE_INIT),
		AvContext		(RENDER_CALLBACK_TYPE_AVCONTEXT),
		Decoder			(RENDER_CALLBACK_TYPE_DECODER),
		Image			(RENDER_CALLBACK_TYPE_IMAGE),
		;

		public int value;
		private RenderCallbackType(int value) { this.value = value; }
		public int value() { return value; }
		public static RenderCallbackType get(int v) {
			switch (v) {
				case RENDER_CALLBACK_TYPE_ALL: return All;
				case RENDER_CALLBACK_TYPE_INIT: return Init;
				case RENDER_CALLBACK_TYPE_AVCONTEXT: return AvContext;
				case RENDER_CALLBACK_TYPE_DECODER: return Decoder;
				case RENDER_CALLBACK_TYPE_IMAGE: return Image;
			}
			return All;
		}
	}

	/**
	 * Environs AVCONTEXT_TYPES
	 * Type: int
	 */
	public static final int DECODER_AVCONTEXT_TYPE_PIXELS		= 0;
	public static final int DECODER_AVCONTEXT_TYPE_AVCONTEXT	= RENDER_CALLBACK_TYPE_AVCONTEXT;
	public static final int DECODER_AVCONTEXT_TYPE_JPG			= 2;
	public static final int DECODER_AVCONTEXT_TYPE_PNG			= 3;

	/**
	 * Environs AVCONTEXT_TYPES enumeration.
	 * */
	public static enum DecoderContextType {
		AvContext	(DECODER_AVCONTEXT_TYPE_AVCONTEXT),
		Jpg			(DECODER_AVCONTEXT_TYPE_JPG),
		Png			(DECODER_AVCONTEXT_TYPE_PNG),
		;

		public int value;
		private DecoderContextType(int value) { this.value = value; }
		public int value() { return value; }
		public static DecoderContextType get(int v) {
			switch (v) {
				case DECODER_AVCONTEXT_TYPE_AVCONTEXT: return AvContext;
				case DECODER_AVCONTEXT_TYPE_JPG: return Jpg;
				case DECODER_AVCONTEXT_TYPE_PNG: return Png;
			}
			return AvContext;
		}
	}

	/**
	 * Environs AVCONTEXT_SUBTYPES
	 * Type: int
	 */
	public static final int ENVIRONS_AVCONTEXT_SUBTYPE_RGB		= 0;
	public static final int ENVIRONS_AVCONTEXT_SUBTYPE_RGBA		= 1;
	public static final int ENVIRONS_AVCONTEXT_SUBTYPE_ARGB		= 2;

	public static final int ENVIRONS_AVCONTEXT_SUBTYPE_BGR		= 6;
	public static final int ENVIRONS_AVCONTEXT_SUBTYPE_BGRA		= 7;
	public static final int ENVIRONS_AVCONTEXT_SUBTYPE_ABGR		= 8;

	public static final int TYPES_SEPERATOR_2_ENVIRONS			= 0;

	/**
	 * Environs SENSOR_TYPES
	 * Type: int
	 */
	public static final int ENVIRONS_SENSOR_TYPE_ACCELEROMETER		= 0;
	public static final int ENVIRONS_SENSOR_TYPE_MAGNETICFIELD		= 1;
	public static final int ENVIRONS_SENSOR_TYPE_GYROSCOPE			= 2;
	public static final int ENVIRONS_SENSOR_TYPE_ORIENTATION		= 3;
	public static final int ENVIRONS_SENSOR_TYPE_LIGHT				= 4;
	public static final int ENVIRONS_SENSOR_TYPE_LOCATION			= 5;
	public static final int ENVIRONS_SENSOR_TYPE_HEADING			= 6;
	public static final int ENVIRONS_SENSOR_TYPE_ALTIMETER			= 7;
	public static final int ENVIRONS_SENSOR_TYPE_MOTION_ATTITUTDE_ROTATION		= 8;
	public static final int ENVIRONS_SENSOR_TYPE_MOTION_GRAVITY_ACCELERATION	= 9;
	public static final int ENVIRONS_SENSOR_TYPE_MOTION_MAGNETICFIELD			= 10;
	public static final int ENVIRONS_SENSOR_TYPE_HEARTRATE			= 11;
	public static final int ENVIRONS_SENSOR_TYPE_PROXIMITY			= 12;
	public static final int ENVIRONS_SENSOR_TYPE_VOC				= 13;
	public static final int ENVIRONS_SENSOR_TYPE_CO2				= 14;
	public static final int ENVIRONS_SENSOR_TYPE_HUMIDITY			= 15;
	public static final int ENVIRONS_SENSOR_TYPE_TEMPERATURE		= 16;
	public static final int ENVIRONS_SENSOR_TYPE_CUSTOM				= 17;
	public static final int ENVIRONS_SENSOR_TYPE_MAX				= 18;

	/**
	 * Sensor type enumeration.
	 * */
	private static enum SensorType {
		All					(-1),
		Accelerometer		(ENVIRONS_SENSOR_TYPE_ACCELEROMETER),
		MagneticField		(ENVIRONS_SENSOR_TYPE_MAGNETICFIELD),
		Gyroscope			(ENVIRONS_SENSOR_TYPE_GYROSCOPE),
		Orientation			(ENVIRONS_SENSOR_TYPE_ORIENTATION),
		Light				(ENVIRONS_SENSOR_TYPE_LIGHT),
		Location			(ENVIRONS_SENSOR_TYPE_LOCATION),
		Heading				(ENVIRONS_SENSOR_TYPE_HEADING),
		Altimeter			(ENVIRONS_SENSOR_TYPE_ALTIMETER),
		MotionAttitudeRotation		(ENVIRONS_SENSOR_TYPE_MOTION_ATTITUTDE_ROTATION),
		MotionGravityAcceleration	(ENVIRONS_SENSOR_TYPE_MOTION_GRAVITY_ACCELERATION),
		MotionMagneticField			(ENVIRONS_SENSOR_TYPE_MOTION_MAGNETICFIELD),
		Heartrate			(ENVIRONS_SENSOR_TYPE_HEARTRATE),
		Proximity			(ENVIRONS_SENSOR_TYPE_PROXIMITY),
		VOC					(ENVIRONS_SENSOR_TYPE_VOC),
		CO2					(ENVIRONS_SENSOR_TYPE_CO2),
		Humidity			(ENVIRONS_SENSOR_TYPE_HUMIDITY),
		Temperature			(ENVIRONS_SENSOR_TYPE_TEMPERATURE),
		Custom				(ENVIRONS_SENSOR_TYPE_CUSTOM),
		Max					(ENVIRONS_SENSOR_TYPE_MAX);

		public int value;
		private SensorType(int value) { this.value = value; }
		public int value() { return value; }
	}

	public static final String sensorFlagDescriptions [] = { // DEBUG
			"Accelerometer",
			"MagneticField",
			"Gyroscope",
			"Orientation",
			"Light",
			"Location",
			"Heading",
			"Altimeter/Pressure",
			"MotionAttitudeRotation",
			"MotionGravityAcceleration",
			"MotionMagneticField",
			"Heartrate",
			"Proximity",
			"VOC",
			"CO2",
			"Humidity",
			"Temperature",
			"Custom",
			"Invalid",
	};

	public static final int ENVIRONS_SENSOR_PACK_TYPE_EXT			= 0x1000000;

	public static final int ENVIRONS_SENSOR_FLAG_ACCELEROMETER		= 0x1;
	public static final int ENVIRONS_SENSOR_FLAG_MAGNETICFIELD		= 0x2;
	public static final int ENVIRONS_SENSOR_FLAG_GYROSCOPE			= 0x4;
	public static final int ENVIRONS_SENSOR_FLAG_ORIENTATION		= 0x8;
	public static final int ENVIRONS_SENSOR_FLAG_LIGHT				= 0x10;
	public static final int ENVIRONS_SENSOR_FLAG_LOCATION			= 0x20;
	public static final int ENVIRONS_SENSOR_FLAG_HEADING			= 0x40;
	public static final int ENVIRONS_SENSOR_FLAG_ALTIMETER			= 0x80;
	public static final int ENVIRONS_SENSOR_FLAG_MOTION_ATTITUTDE_ROTATION		= 0x100;
	public static final int ENVIRONS_SENSOR_FLAG_MOTION_GRAVITY_ACCELERATION	= 0x200;
	public static final int ENVIRONS_SENSOR_FLAG_MOTION_MAGNETICFIELD			= 0x400;
	public static final int ENVIRONS_SENSOR_FLAG_HEARTRATE			= 0x800;
	public static final int ENVIRONS_SENSOR_FLAG_PROXIMITY			= 0x1000;
	public static final int ENVIRONS_SENSOR_FLAG_VOC				= 0x2000;
	public static final int ENVIRONS_SENSOR_FLAG_CO2				= 0x4000;
	public static final int ENVIRONS_SENSOR_FLAG_HUMIDITY			= 0x8000;
	public static final int ENVIRONS_SENSOR_FLAG_TEMPERATURE		= 0x10000;
	public static final int ENVIRONS_SENSOR_FLAG_CUSTOM				= 0x800000;

	public static final int MAX_ENVIRONS_SENSOR_TYPE_VALUE			= ENVIRONS_SENSOR_FLAG_CUSTOM;

	public static final int sensorFlags [] = { // +CLI
			ENVIRONS_SENSOR_FLAG_ACCELEROMETER,
			ENVIRONS_SENSOR_FLAG_MAGNETICFIELD,
			ENVIRONS_SENSOR_FLAG_GYROSCOPE,
			ENVIRONS_SENSOR_FLAG_ORIENTATION,
			ENVIRONS_SENSOR_FLAG_LIGHT,
			ENVIRONS_SENSOR_FLAG_LOCATION,
			ENVIRONS_SENSOR_FLAG_HEADING,
			ENVIRONS_SENSOR_FLAG_ALTIMETER,
			ENVIRONS_SENSOR_FLAG_MOTION_ATTITUTDE_ROTATION,
			ENVIRONS_SENSOR_FLAG_MOTION_GRAVITY_ACCELERATION,
			ENVIRONS_SENSOR_FLAG_MOTION_MAGNETICFIELD,
			ENVIRONS_SENSOR_FLAG_HEARTRATE,
			ENVIRONS_SENSOR_FLAG_PROXIMITY,
			ENVIRONS_SENSOR_FLAG_VOC,
			ENVIRONS_SENSOR_FLAG_CO2,
			ENVIRONS_SENSOR_FLAG_HUMIDITY,
			ENVIRONS_SENSOR_FLAG_TEMPERATURE,
			ENVIRONS_SENSOR_FLAG_CUSTOM,
			MAX_ENVIRONS_SENSOR_TYPE_VALUE,
	};

	public static final int ENVIRONS_WIFI_OBSERVER_INTERVAL_MIN					= 30000;
	public static final int ENVIRONS_WIFI_OBSERVER_INTERVAL_CHECK_MIN			= 2000;
	public static final int ENVIRONS_WIFI_OBSERVER_INTERVAL_MOBILE_MIN			= 5000;
	public static final int ENVIRONS_WIFI_OBSERVER_INTERVAL_MOBILE_CHECK_MIN	= 1000;

	/** Ignore: for CLI all the remaining content*/

	/**
	 * Environs Option keys
	 * Type: String
	 */
	/** Ignore: for Resolver */
	public static final String APPENV_MAPPINGS							= "mappings";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_TOKEN_MEDIATOR_DEFAULT	= "optDefaultMedToken";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_TOKEN_MEDIATOR_CUSTOM		= "optCustomMedToken";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_TOKEN_MEDIATOR_DEFAULT_N	= "optDNefaultMedToken";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_TOKEN_MEDIATOR_CUSTOM_N	= "optCNustomMedToken";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_TOKEN_MEDIATOR_USERNAME	= "optMediatorUsername";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_INITIALS					= "optInitialSettings";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_GL_USE_NATIVE_DECODER		= "optUseNativeDecoder";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_GL_USE_HARDWARE_DECODER	= "optUseHardwareEncoder";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_GL_USE_SHOW_DEBUG_LOGS	= "useShowDebugStatus";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_GL_USE_LOG_FILE			= "useLogFile";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_GL_USE_BT_OBSERVER		= "useBtObserver";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_GL_USE_BT_INTERVAL		= "useBtInterval";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_GL_USE_WIFI_OBSERVER		= "useWifiObserver";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_GL_USE_WIFI_INTERVAL		= "useWifiInterval";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_GL_USE_PUSH_NOTIFS		= "optUsePushNotifications";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_GL_USE_SENSORS			= "optUseSensors";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_PORTAL_AUTOSTART		= "optPortalAutoStart";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_DEFAULT_MEDIATOR		= "optUseDefaultMediator";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_CUSTOM_MEDIATOR		= "optUseCustomMediator";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_PORTAL_TCP			= "optPortalTCP";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_NATIVE_RESOLUTION		= "optNativeResolution";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_STREAM				= "optUseStream";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_CLS_MEDIATOR			= "useCLSMediator";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_CLS_DEVICE			= "useCLSDevice";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_CLS_DEV_ENFORCE		= "useCLSDevEnforce";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_AUTH					= "useAuth";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_ANONYMOUS				= "useAnonymous";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_PORTAL_AUTOACCEPT		= "portalAutoAccept";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_DEVICE_ID					= "optDeviceID";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_DEVICE_UID				= "uid";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_DEVICE_NAME				= "optDeviceName";

	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_MEDIATOR_LOGIN_DLG	= "useMediatorLoginDialog";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_CUSTOMMEDIATOR_PORT	= "optMediatorPort";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_CUSTOMMEDIATOR_IP		= "optMediatorIP";
	/** Ignore: for Resolver */
	public static final String APPENV_SETTING_USE_PORTALVIEW_DIMS_AUTO	= "optUsePortalViewDimsAuto";



}
