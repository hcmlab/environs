package environs.MediaBrowser;
/**
 *	Fullscreen activity
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
import environs.*;
import environs.MediaBrowser.R;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.view.View.OnLongClickListener;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.RelativeLayout;

/**
*	Fullscreen activity
*	---------------------------------------------------------
*	Copyright (C) Chi-Tai Dang
*   All rights reserved.
*
*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
*	@version	1.0
* ****************************************************************************************
*/
@SuppressLint("NewApi")
public class Fullscreen extends Activity implements TextureView.SurfaceTextureListener, 
					SurfaceHolder.Callback
{
	public static final String className = "Fullscreen";
	
	public static Fullscreen instance = null;
	public static Boolean initiated = false;
	public static View instanceView = null;
	
	public TextureView textureView = null;
	public static Bitmap background = null;
	private boolean hasNaviButtons = false;
	private static Button btnTop = null;
	private static Button btnBottom = null;
	private static Button btnLeft = null;
	private static Button btnRight = null;
	private static Button btnTopMenu = null;

    public static boolean syncPosition = true;

	static PortalInstance portal = null;
    
    public static int overlayAlpha = 255;
    
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        Utils.Log ( 4, className, "onCreate" );

        instance = this;
        
        synchronized (className) {
            initiated = false;        	
        }
        
        background = null;

		Environs env = TabActivity.env;
		if (env == null)
			return;

        // Request fullscreen window and disable display timeout
        Utils.RequestFullscreenAlwaysOn(this);

        setContentView( R.layout.fullscreen);
        
        instanceView = findViewById(android.R.id.content);
        if (instanceView == null) {
        	Utils.LogW ( className, "onCreate: view not found!!!");        	
        }
        Utils.RequestHiddenUIBars(instanceView);
                      
        // Register the current view as touch source for Environs
		env.SetPortalTouchView(portal, instanceView);

        /*
        // Create opengl renderer
        GLSurfaceView view = new GLSurfaceView(this);
   		view.setRenderer(new FullscreenGLRender());
   		setContentView(view);
   		*/
        
        // Section for using TextureView - should be catched later on and replaced with SurfaceView for old devices!!!
        if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.HONEYCOMB)
        	return;
        
        RelativeLayout relout = (RelativeLayout) instanceView.findViewById(R.id.relativeLayout);
        if (relout == null){
        	Utils.LogE ( className, "onCreate: Layout in view not found!!!");
        	return;
        }
        
        if (env.GetUseStream() || env.GetUseNativeDecoder()) {
        	Utils.Log ( 3, className, "onCreate - Creating SurfaceView for h264 decoding.");
        	
        	FrameLayout flout = (FrameLayout) instanceView.findViewById(R.id.frameLayout);
        	if ( flout != null ) {
        		SurfaceView sv = (SurfaceView) flout.findViewById(R.id.surfaceView1);
        		if ( sv != null ) {
        			sv.getHolder().addCallback(this);
        		}
        	}
        }
        else {
        	Utils.Log ( 3, className, "onCreate - Creating TextureView for bitmap rendering.");
	        textureView = new TextureView(this);
	        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams 
		        	(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
		    params.topMargin = 0;
		    
		    textureView.setLayoutParams(params);	        
		    relout.addView(textureView);
        }
        

    	btnTopMenu = (Button) relout.findViewById(R.id.buttonTopMenu);
    	btnTop = (Button) relout.findViewById(R.id.buttonTop);
    	btnBottom = (Button) relout.findViewById(R.id.buttonBottom);
    	btnLeft = (Button) relout.findViewById(R.id.buttonLeft);
    	btnRight = (Button) relout.findViewById(R.id.buttonRight);
    	if (btnTopMenu == null || btnTop == null || btnBottom == null || btnLeft == null || btnRight == null ) {
        	Utils.LogE ( className, "onCreate: Navigation buttons in view not found!!!");
        	return;    		
    	}

    	btnTopMenu.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
            	CharSequence colors[] = new CharSequence[] {"Clear", "Sync Position: " + (syncPosition ? "Yes" : "No"), 
            			"Personal consumption ovl", "Personal consumption ovlf", "Personal consumption ext",
            			"Personal consumption static",
            			"Personal outline 1",
            			"Personal outline 2",
            			};

            	AlertDialog.Builder builder = new AlertDialog.Builder(instance);
            	builder.setTitle("Choose an overlay");
            	builder.setItems(colors, new DialogInterface.OnClickListener() {
            	    @Override
            	    public void onClick(DialogInterface dialog, int selection) {
						if (portal != null ) {
							if (selection == 1) {
								syncPosition = !syncPosition;
								portal.device.SendMessage("SP:" + (syncPosition ? "1" : "0"));
							}
							else {
								if (selection > 1)
									selection--;
								portal.device.SendMessage("OL:" + selection + ";P:" + portal.device.portalIDIn + ";A:" + overlayAlpha);
							}
						}
            	    }
            	});
            	builder.show();
            }
        });
    	
        hasNaviButtons = env.optBool("optNavigationButtons");
        if (hasNaviButtons) {
        	btnTop.setVisibility(Button.VISIBLE);
        	btnBottom.setVisibility(Button.VISIBLE);
        	btnLeft.setVisibility(Button.VISIBLE);
        	btnRight.setVisibility(Button.VISIBLE);


        	btnTop.setOnClickListener(new Button.OnClickListener() {
				public void onClick(View v) {
					if (btnLeft.isPressed())
						ChangePortalSize(-20, -20);
					else
						MovePortalUp();
				}
			});
			btnTop.setOnTouchListener(new View.OnTouchListener() {
				@Override
				public boolean onTouch(View v, MotionEvent event) {
					return DoTwoFingerTouch(v, event);
				}
			});
        	
        	btnTop.setOnLongClickListener(new OnLongClickListener() {
				@Override
				public boolean onLongClick(View v) {
					longClick = true;
					RepeatChangePortalLocation(0, -15);
					return true;
				}
			});
        	
        	btnLeft.setOnClickListener(new Button.OnClickListener() {
				public void onClick(View v) {
					MovePortalLeft();
				}
			});
        	
        	btnLeft.setOnLongClickListener(new OnLongClickListener() {
				@Override
				public boolean onLongClick(View v) {
					longClick = true;
					RepeatChangePortalLocation(-15, 0);
					return true;
				}
			});
			btnLeft.setOnTouchListener(new View.OnTouchListener() {
				@Override
				public boolean onTouch(View v, MotionEvent event) {
					return DoTwoFingerTouch(v, event);
				}
			});
        	
        	btnBottom.setOnClickListener(new Button.OnClickListener() {
				public void onClick(View v) {
					if (btnRight.isPressed())
						ChangePortalSize(20, 20);
					else
						MovePortalDown();
				}
			});
        	
        	btnBottom.setOnLongClickListener(new OnLongClickListener() {
				@Override
				public boolean onLongClick(View v) {
					longClick = true;
					RepeatChangePortalLocation(0, 15);
					return true;
				}
			});
			btnBottom.setOnTouchListener(new View.OnTouchListener() {
				@Override
				public boolean onTouch(View v, MotionEvent event) {
					return DoTwoFingerTouch(v, event);
				}
			});
        	
        	btnRight.setOnClickListener(new Button.OnClickListener() {
				public void onClick(View v) {
					MovePortalRight();
				}
			});

        	btnRight.setOnLongClickListener(new OnLongClickListener() {
				@Override
				public boolean onLongClick(View v) {
					longClick = true;
					RepeatChangePortalLocation(15, 0);
					return true;
				}
			});
			btnRight.setOnTouchListener(new View.OnTouchListener() {
				@Override
				public boolean onTouch(View v, MotionEvent event) {
					return DoTwoFingerTouch(v, event);
				}
			});
        }
        else {
        	btnTop.setVisibility(Button.GONE);
        	btnBottom.setVisibility(Button.GONE);
        	btnLeft.setVisibility(Button.GONE);
        	btnRight.setVisibility(Button.GONE);
        }
    }
	
	private static boolean longClick = false;

	@SuppressWarnings ( "all" )
	public static boolean DoTwoFingerTouch(View v, MotionEvent event)
	{
		int actionA = event.getAction();
		if ( actionA == MotionEvent.ACTION_UP )
			longClick = false;
/*
		if (actionA == MotionEvent.ACTION_UP || actionA == MotionEvent.ACTION_CANCEL) {
			X1 = Y1 = X2 = Y2 = 0; initDist = 0;
		}
		else //if (actionA == MotionEvent.ACTION_DOWN)
		{
			int pCount = event.getPointerCount();
			if (pCount == 2) {
				if ( (X1 == 0 && Y1 == 0) || (X2 == 0 && Y2 == 0)) {
					X1 = (int)event.getX(0);
					Y1 = (int)event.getY(0);
					X2 = (int)event.getX(1);
					Y2 = (int)event.getY(1);
					Utils.Log ( 5, "Two Fingers init X1 = " + X1 + " Y1 = " + Y1 + " X2 = " + X2 + " Y2 = " + Y2);

					initDist = Math.sqrt( Math.pow((X1 - Y1), 2) + Math.pow((X2 - Y2), 2));
					Utils.Log ( 5, "Two Fingers init ... dist = " + initDist);
				}
				else {
					X1 = (int)event.getX(0);
					Y1 = (int)event.getY(0);
					X2 = (int)event.getX(1);
					Y2 = (int)event.getY(1);
					double dist = Math.sqrt( Math.pow((X1 - Y1), 2) + Math.pow((X2 - Y2), 2));

					double diff = initDist - dist;
					diff *= 2;
					if ( diff > 2 || diff < -2 ) {
						Utils.Log ( 5, "Two Fingers init ... distDiff = " + diff);
						ChangePortalSize ( (int)diff, (int)diff );
						initDist = dist;
					}
				}
			}
		}
		*/

		return false;
	}

	public static void MovePortalUp() {
		ChangePortalPosition(0, -10);
	}
	
	public static void MovePortalDown() {
		ChangePortalPosition(0, 10);
	}
	
	public static void MovePortalLeft() {
		ChangePortalPosition(-10, 0);
	}
	
	public static void MovePortalRight() {
		ChangePortalPosition(10, 0);
	}

	@SuppressWarnings ( "all" )
	private static void ChangePortalPosition(final int xDiff, final int yDiff) {
        new Thread(new Runnable() { 
            public void run(){
            	if (portal == null)
            		return;

            	PortalInfo info = portal.info;
            	if (info != null) {
					int centerX = info.centerX;
					int centerY = info.centerY;

    				if (info.orientation == 90.0f) {
    					centerX += xDiff;
    					centerY += yDiff;
    				}
    				else if (info.orientation == 0.0f) {
    					centerX += yDiff;
    					centerY -= xDiff;
    				}
    				else if (info.orientation == 180.0f) {
    					centerX -= yDiff;
    					centerY += xDiff;
    				}
    				else if (info.orientation == 270.0f) {
    					centerX -= xDiff;
    					centerY -= yDiff;
    				}
    				else {
    					/// Transform surface angle to cartesian angle
    					/// double theta = 180 - info.orientation;

    					/// Add marker angle offset (showing upwards on the surface means 0 degree on the tablet) + 90
    					/// double theta = 270 - info.orientation;
    					double theta = (double)(((270.0f - info.orientation) * Math.PI) / 180.0);

    					theta = -theta;

    					//double theta = info.orientation + 90;
    	                //if (theta < 0)
    	                //	theta = 360 + theta;
    	                double cosV = Math.cos(theta);
    	                double sinV = Math.sin(theta);

    	                double xV = (double)xDiff * cosV - (double)yDiff * sinV;
    	                double yV = (double)xDiff * sinV + (double)yDiff * cosV;

    					centerX -= (int)xV;
    					centerY -= (int)yV;
    				}

            		info.SetLocation(centerX, centerY);
            	}
            }
        }).start();	
	}
    
	private static void ChangePortalSize(final int xDiff, final int yDiff) {
        new Thread(new Runnable() { 
            public void run(){
            	if (portal == null)
            		return;

				portal.info.SetSize(portal.info.width + xDiff, portal.info.height + yDiff);
            }
        }).start();	
	}
    
	private static void RepeatChangePortalLocation(final int xDiff, final int yDiff) {
		if ( !longClick )
			return;
		
        new Thread(new Runnable() { 
            public void run(){
            	if (portal == null)
            		return;

				while ( longClick ) {
					portal.info.SetLocation(portal.info.centerX + xDiff, portal.info.centerY + yDiff);
					Utils.Log ( 2, className, "repeatChangePortalSize:" + portal.info.toString() );

					try {
						Thread.sleep(80);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
            }
        }).start();	
	}


	@SuppressWarnings ( "all" )
	public void UpdateBackground()
	{
		Bitmap bm = Fullscreen.background;
		Fullscreen.background = null;
		
    	if (bm == null)
    		return;
    	
    	if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB)
    	{
    		Rect r_dest = new Rect(0, 0, Utils.width, Utils.height);
    		
    		TextureView tv = this.textureView;
    		if (tv != null) {
                /*
                 * For TextureViews
                */
        		final Canvas canvas = tv.lockCanvas(null);
        		if (canvas == null)
        			return;
                try {
                	canvas.drawBitmap(bm, null, r_dest, null);
                } finally {
                	tv.unlockCanvasAndPost(canvas);
                }  
                return;
    		}
    		    		
        	SurfaceHolder holder = this.holder;    		
        	if (holder != null) {
        		final Canvas canvas = holder.lockCanvas(null);
                if ( canvas != null ) {
                    try {
                        synchronized (holder) {
                        	canvas.drawBitmap(bm, null, r_dest, null);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    } finally {
                        if (canvas != null) {
                            holder.unlockCanvasAndPost(canvas);
                        }
                    }   	
                }
        	}
    	}
    	else {
        	BitmapDrawable bg = new BitmapDrawable(getResources(), bm);
        	View v = Fullscreen.instanceView;
        	if (v != null)
        		v.setBackground(bg);
        	bg = null;
    	}
	}
	
	@Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK) {
        	this.finish();
        }
		return false;
    }
	
	@Override
	public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
		textureView.setAlpha(0.5f);
		textureView.setRotation(45.0f);		
	}
 
	@Override
	public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
		return true;
	}
 
	@Override
	public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
	}
 
	@Override
	public void onSurfaceTextureUpdated(SurfaceTexture surface) {		
	}

	
    @Override
    protected void onDestroy ()
    {
    	Utils.Log ( 4, className, "onDestroy" );
    	        
    	instance = null;
    	instanceView = null;
    	textureView = null;

		if (portal != null)
			portal.Stop();
    	
    	super.onDestroy(); 	
    }
    

    @Override
    public void onResume() {    
    	Utils.Log ( 4, className, "onResume" );
        super.onResume();

        TabActivity.currentActivity = this;
	    
		// Start stream
        if ( !DeviceActivity.isVideoStream && portal != null )
			portal.Start();
    }

    
    @Override
    protected void onPause() {
    	Utils.Log(4, className, "onResume");
        
        super.onPause();

		if (portal != null)
			portal.Stop();
    }
    

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    	Utils.Log ( 4, className, "surfaceChanged" );
    	this.holder = holder;		
    	
    	// Providing a render surface to Environs allows Environs to update our background with the portal automatically
		Surface surface;

		if ( holder == null || (surface = holder.getSurface()) == null ) {
			Utils.LogE ( className, "surfaceChanged: Provided surface is null.");
			return;
		}

		if (portal == null)
			return;

    	if ( !portal.SetRenderSurface(surface, width, height) )
			Utils.LogE ( className, "surfaceChanged: setRenderSurface failed." );
    	
    	if ( portal.Start() )
			Utils.Log ( 4, className, "surfaceChanged: Portal start requested." );
		else
			Utils.LogE ( className, "surfaceChanged: Portal start failed." );
	}
	
	private SurfaceHolder holder = null;

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
    	Utils.Log ( 4, className, "surfaceCreated" );		
    	this.holder = holder;
	}
	

	@SuppressWarnings ( "all" )
	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		Utils.Log(4, className, "surfaceDestroyed");

		if (portal != null)
			portal.ReleaseRenderSurface();
    	holder = null;
	}
}
