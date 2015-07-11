/**
 * Interface for a portal renderer (create source images)
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
#include "IPortal.Capture.h"
#include "Render.Context.h"
#include "Render.Dimensions.h"


#ifndef INCLUDE_HCM_ENVIRONS_IPORTALRENDERER_H
#define INCLUDE_HCM_ENVIRONS_IPORTALRENDERER_H


namespace environs 
{

	class IPortalRenderer;

	/**
	*	Interface for a renderer extension
	*
	*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
	*	@version	1.0
	*	@remarks
	* ****************************************************************************************
	*/
	DeclareEnvironsInterface ( IPortalRenderer )

	public:
		/** 
		* Default constructor
		*/
		IPortalRenderer ( ) :
			/** Base class initialization */
			IEnvironsBase ( InterfaceType::Render ),

			/** Default initialization */
            squareLength ( 0 ), buffersInitialized ( false ), parent ( 0 ), filledContexts ( 0 ), hAppWindow ( 0 ), stages ( 0 )
            // , capture ( 0 )
		{};

		virtual ~IPortalRenderer ( ) {};

		/** Init
		*	@param	void
		*	@return	bool
		*/
		/** Interface initializer. Do not override this method. Init () is called at the end of the Interface initializer */
		bool				Init ( unsigned int deviceID, void * parent, void * appWindow );
		
		virtual void		Dispose ( ) = 0;

		virtual bool		Perform ( RenderDimensions * dims, RenderContext * context ) = 0;
    
		virtual int         ReleaseResources ( RenderContext * context ) = 0;
		virtual int			AllocateResources ( RenderDimensions * dims ) = 0;
    
		virtual int			UpdateBuffers ( RenderDimensions * dims, RenderContext * context ) = 0;

		virtual char *		GetBuffer ( RenderContext * pack ) = 0;
		virtual	void		ReleaseBuffer ( RenderContext * pack ) {};

#ifndef ENABLE_WORKER_STAGES_COMPARE
		virtual bool		Compare ( unsigned int &equal ) = 0;
#endif

		int					squareLength;
		bool				buffersInitialized;

		bool				rendered;

		static int			device_left;
		static int			device_top;
    
        void			*	stages;
		//void			*	capture;

	private:
		virtual bool		Init ( ) = 0;

	protected:
		void			*	parent;

		unsigned int		filledContexts; // required to fill all contexts with the latest equal frame before reusing only the previous context


		void			*	hAppWindow;

	EndInterface

		

#define COMPARE_CACHE(v)	\
	unsigned int v; \
	if ( Compare ( equal ) ) { \
		if ( equal ) { \
			if ( opt_useRenderCache ) { \
				filledContexts++; \
				if ( filledContexts > MAX_PORTAL_CONTEXTS ) { \
					context->hasChanged = false; \
					return true; \
				} \
				context->hasChanged = true; \
			} \
		} \
		else {\
			filledContexts = 0;\
			context->hasChanged = true; \
		} \
	} \
	else { \
		CErrID ( "Perform: Failed to compare pixels" ); \
		break; \
	} \



} /* namespace environs */


#endif // INCLUDE_HCM_ENVIRONS_IPORTALRENDERER_H
