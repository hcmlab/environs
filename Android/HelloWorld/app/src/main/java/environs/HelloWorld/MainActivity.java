package environs.HelloWorld;
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
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import environs.*;
import environs.HelloWorld.R;

/**
 * MainActivity.
 *
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 */
public class MainActivity extends Activity
{

    Environs environs;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate ( savedInstanceState );
        setContentView ( R.layout.activity_main );

        /// Instantiate the Environs instance to take part on the application environment
        environs = Environs.CreateInstance(this, "HelloWorld", "Environs" );

        if (environs == null) {
            Log.e("HelloWorld", "Failed to create an Environs object!");
            return;
        }
        //environs.ResetCryptLayer ();

        /// Use default mediator for device management
        environs.AddObserver(new Observer());

        /// Start Environs
        environs.Start ( );
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate( R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
