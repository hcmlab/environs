/**
 * Common platform declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_NATIVE_PLATFORM_DECLS_H
#define INCLUDE_HCM_ENVIRONS_NATIVE_PLATFORM_DECLS_H


#ifndef EnvironsPtr
#	define EnvironsPtr				Environs OBJ_ptr
#endif

#ifndef DeviceInstancePtr
#	define DeviceInstancePtr		DeviceInstance OBJ_ptr
#endif

#ifndef DeviceInstanceEP
#	define DeviceInstanceEP         EPSPACE DeviceInstance
#endif

#ifndef DeviceInstanceEPtr
#	define DeviceInstanceEPtr			EPSPACE DeviceInstance OBJ_ptr
#endif

#ifndef DeviceInstanceSP
#	define DeviceInstanceSP				sp ( DeviceInstance )
#endif

#ifndef DeviceInstanceReferenceSP
#	define DeviceInstanceReferenceSP	c_const sp ( DeviceInstance ) c_ref
#endif

#ifndef DeviceInstanceESP
#	define DeviceInstanceESP			sp ( EPSPACE DeviceInstance )
#endif

#ifndef DeviceInstanceReferenceESP
#	define DeviceInstanceReferenceESP   c_const sp ( EPSPACE DeviceInstance ) c_ref
#endif

#ifndef DeviceInfoPtr
#	define	DeviceInfoPtr			DeviceInfo OBJ_ptr
#endif

#ifndef DeviceNotifierContextPtr
#   define DeviceNotifierContextPtr	DeviceNotifierContext OBJ_ptr
#endif

#ifndef ListCommandContextPtr
#	define ListCommandContextPtr	ListCommandContext OBJ_ptr
#endif

#ifndef DeviceNotifierContextPtr
#	define DeviceNotifierContextPtr	DeviceNotifierContext OBJ_ptr
#endif

#ifndef FileInstanceEP
#	define FileInstanceEP         EPSPACE FileInstance
#endif

#ifndef FileInstanceESP
#	define FileInstanceESP         sp ( EPSPACE FileInstance )
#endif

#ifndef MessageInstanceEP
#	define MessageInstanceEP         EPSPACE MessageInstance
#endif

#ifndef MessageInstanceESP
#	define MessageInstanceESP         sp ( EPSPACE MessageInstance )
#endif

#ifndef PortalInstancePtr
#	define PortalInstancePtr         PortalInstance OBJ_ptr
#endif

#ifndef PortalInstanceSP
#	define PortalInstanceSP         sp ( PortalInstance )
#endif

#ifndef PortalInstanceESP
#	define PortalInstanceESP         sp ( EPSPACE PortalInstance )
#endif

#ifndef PortalInstanceEP
#	define PortalInstanceEP         EPSPACE PortalInstance
#endif

#ifndef PortalInfoBaseSP
#	define PortalInfoBaseSP         sp ( PortalInfoBase )
#endif

#ifndef PortalInfoBasePtr
#	define PortalInfoBasePtr         PortalInfoBase OBJ_ptr
#endif

#ifndef ListCommandContextPtr
#	define ListCommandContextPtr	ListCommandContext OBJ_ptr
#endif

#ifndef DataObserverPtr
#	define	DataObserverPtr			DataObserver OBJ_ptr
#endif

#ifndef SensorObserverPtr
#	define	SensorObserverPtr		SensorObserver OBJ_ptr
#endif

#ifndef MessageObserverPtr
#	define	MessageObserverPtr		MessageObserver OBJ_ptr
#endif

#ifndef DeviceObserverPtr
#	define	DeviceObserverPtr		DeviceObserver OBJ_ptr
#endif

#ifndef EnvironsObserverPtr
#	define	EnvironsObserverPtr		EnvironsObserver OBJ_ptr
#endif
#ifndef StatusObserverPtr
#	define	StatusObserverPtr		StatusObserver OBJ_ptr
#endif

#ifndef ListObserverPtr
#	define	ListObserverPtr			ListObserver OBJ_ptr
#endif

#ifndef DeviceObserverPtr
#	define	DeviceObserverPtr		DeviceObserver OBJ_ptr
#endif

#ifndef MessageObserverPtr
#	define	MessageObserverPtr		MessageObserver OBJ_ptr
#endif

#ifndef DataObserverPtr
#	define	DataObserverPtr			DataObserver OBJ_ptr
#endif

#ifndef SensorObserverPtr
#	define	SensorObserverPtr		SensorObserver OBJ_ptr
#endif

#ifndef PortalObserverPtr
#	define	PortalObserverPtr		PortalObserver OBJ_ptr
#endif

#ifndef ThreadPackPortalPresenterPtr
#	define	ThreadPackPortalPresenterPtr		ThreadPackPortalPresenter OBJ_ptr
#endif

#endif  // -> INCLUDE_HCM_ENVIRONS_NATIVE_PLATFORMS_H