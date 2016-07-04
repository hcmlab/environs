package environs;
/**
 * Library and license helper thread
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
import android.app.AlertDialog;
import android.content.DialogInterface;

class LibLicThread extends Thread implements Runnable
{
    private static final String className = "LibLicThread . . . . . .";

    public String libName;
    public String libUrl;
    public String licenseUrl;

    public interface OnActionListener {
        void onAction();
    }

    OnActionListener    actionListener;

    public void setActionListener(OnActionListener listener) {
        actionListener = listener;
    }

    @Override
    public void run() {
        if (Utils.isDebug) Utils.Log ( 3, className, "Thread started");

        try
        {
            final Activity act = Environs.currentActivity;
            if ( act == null ) {
                Utils.LogE ( className, "LibLicThread: We need to ask for permission, but no activity is available.");
                return;
            }

            if (Utils.isDebug) Utils.Log ( 3, className, "Thread ..." );

            if ( !Libs.load3rd ( libName, "bz2", libUrl, licenseUrl ) ) {
                Utils.LogE ( className, "LibLicThread: Download or installation of " + libName + " has failed.");
                // Error handling ...
                return;
            }

            // Show license
            final String license = Libs.getLicense(libName);
            if ( license == null ) {
                if (Utils.isDebug) Utils.Log ( 3, className, "LibLicThread: License for " + libName + " is missing.");
                // Error handling ...

                if (actionListener != null)
                    actionListener.onAction();
            }
            else {
                //Utils.Log ( className, license);
                act.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        AlertDialog.Builder alertDialog = new AlertDialog.Builder(act);

                        String modName = libName;
                        if (modName.startsWith("lib"))
                            modName = modName.substring(3);

                        alertDialog.setTitle("License " + libName);
                        alertDialog.setMessage(license);

                        alertDialog.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                if (Utils.isDebug) Utils.Log ( 3, className, "LibLicThread: License for " + libName + " accepted." );

                                if (actionListener != null)
                                    actionListener.onAction();
                            }
                        });

                        alertDialog.show();
                    }
                });
            }
        }
        catch (Exception ex) {
            Utils.LogE ( className, "LibLicThread: " + ex.getMessage() );

            ex.printStackTrace();
        }

        if (Utils.isDebug) Utils.Log ( 3, className, "Thread terminated!" );
    }
}
