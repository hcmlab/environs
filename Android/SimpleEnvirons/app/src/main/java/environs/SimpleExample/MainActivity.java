package environs.SimpleExample;
/**
 *	MainActivity
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

import android.graphics.PixelFormat;
import android.os.Bundle;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.util.Log;
import android.view.Menu;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;

import environs.*;

@SuppressLint("NewApi")
public class MainActivity extends Activity implements TextureView.SurfaceTextureListener, SurfaceHolder.Callback {

	public static Environs environs = null;
	public static OurObserver observer = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView( R.layout.activity_main);

		SurfaceView sview = (SurfaceView)findViewById( R.id.surfaceView1);
		sview.setZOrderOnTop(true);
		SurfaceHolder holder = sview.getHolder();
		holder.setFormat(PixelFormat.TRANSPARENT);

		environs = Environs.CreateInstance ( this, "SimpleEnvirons", "Environs" );

		if (environs == null) {
			Log.e("SimpleEnvirons", "Failed to create an Environs object!");
			return;
		}
        
        // Create an observer for callbacks
		observer = new OurObserver();
		environs.AddObserver(observer);
        
        // Start Environs
		environs.Start();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		return true;
	}

	@Override
	public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) {
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
	}

    @Override
    public void onResume() {    
        super.onResume();
    }
    
    @Override
    protected void onPause() {        
        super.onPause();
    }

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
	}

	@Override
	public void onSurfaceTextureAvailable(SurfaceTexture arg0, int arg1,
			int arg2) {
		
	}

	@Override
	public boolean onSurfaceTextureDestroyed(SurfaceTexture arg0) {
		return false;
	}

	@Override
	public void onSurfaceTextureSizeChanged(SurfaceTexture arg0, int arg1,
			int arg2) {
		
	}

	@Override
	public void onSurfaceTextureUpdated(SurfaceTexture arg0) {		
	}

}
