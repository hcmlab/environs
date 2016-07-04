package environs;
/**
 *	LoginDialog
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
import android.text.InputType;
import android.view.KeyEvent;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.LinearLayout;
import java.util.Timer;
import java.util.TimerTask;

/**
 *	Login Dialog
 *	---------------------------------------------------------
 *	Copyright (C) 2015 Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
public class LoginDialog
{
    private static final String className = "LoginDialog. . . . . . .";

    Environs env;
    final private static Object classLock = new Object();
    private static int count = 0;

    public String userName = "";
    public String password = "";
    public String title = "";
    public boolean result = false;
    private AlertDialog.Builder dlgBuilder = null;
    private AlertDialog loginDlg = null;
    private Activity act = null;
    private OnResultListener listener = null;
    private Timer noActivityTimer = null;
    private TimerTask noActivityTimeout = null;


    /**
     * Default constructor of LoginDialog. This class should not be instantiated using the default constructor.
     * Use SingletonInstance to get an instance which makes sure that there is only one instance at any time.
     *
     * @param message       The message shown within the dialog.
     * @param title         The title of the dialog.
     * @param userName		The username if already known. This may be null.
     */
    public LoginDialog(String message, String title, String userName)
    {
        this.userName = userName;
        this.title = title;
    }


    /**
     * Create an instance of the login dialog with the given parameters.
     * The dialog has to invoked/shown using ShowResult.
     *
     * @param message       The message shown within the dialog.
     * @param title         The title of the dialog.
     * @param userName		The username if already known. This may be null.
     * @return              An instance of the login dialog.
     */
    public static LoginDialog SingletonInstance(int hInst, String message, String title, String userName)
    {
        synchronized (classLock)
        {
            if (count != 0)
                return null;
            count = 1;
        }

        final LoginDialog instance = new LoginDialog(message, title, userName);

        instance.userName = userName;
        instance.title = title;
        instance.env = Environs.GetInstance(hInst);

        Activity act;

        try {
            if (instance.env == null)
                return null;

            act = instance.env.GetClient();
            instance.act = act;
        }
        catch ( Exception ex ) {
            ex.printStackTrace ();
            return null;
        }

        act.runOnUiThread(new Runnable() {
            @Override
            public void run() {

                Activity act = instance.act;

                AlertDialog.Builder dlgBuilder = new AlertDialog.Builder(act);
                instance.dlgBuilder = dlgBuilder;

                dlgBuilder.setTitle (instance.title );

                final EditText userName = new EditText(act);
                userName.setFocusable(true);
                userName.setClickable(true);
                userName.setFocusableInTouchMode(true);
                userName.setSelectAllOnFocus(true);
                userName.setSingleLine(true);
                userName.setImeOptions(EditorInfo.IME_ACTION_NEXT);

                final EditText password = new EditText(act);
                password.setHint("Password");
                password.setFocusable(true);
                password.setClickable(true);
                password.setFocusableInTouchMode(true);
                password.setSelectAllOnFocus(true);
                password.setSingleLine(true);
                password.setImeOptions(EditorInfo.IME_ACTION_DONE);
                password.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
                //editTextPassword.setOnFocusChangeListener(onFocusChangeListener);


                LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT,
                        LinearLayout.LayoutParams.MATCH_PARENT);
                params.setMargins(4,  4,  4,  4);

                LinearLayout linearLayout = new LinearLayout(act);
                linearLayout.setOrientation(LinearLayout.VERTICAL);
                linearLayout.addView(userName, params);
                linearLayout.addView(password, params);

                dlgBuilder.setView ( linearLayout );

                //loginDlg.setView ( loginView );

                dlgBuilder.setPositiveButton ( "Ok", new DialogInterface.OnClickListener () {
                    public void onClick ( DialogInterface dialog, int whichButton )
                    {
                        instance.userName = userName.getText().toString();
                        instance.password = password.getText().toString();
                        ResetClose ();
                        instance.DisposeTimer();

                        if (instance.listener != null) {
                            instance.result = true;
                            instance.listener.onResult(instance);
                            return;
                        }

                        instance.act.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
                        instance.act.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);

                        String name = instance.userName;
                        String pass = instance.password;

                        if (name != null && pass != null) {
                            Utils.Log2 ( className, "User: " + name + " Pass: " + pass.length ( ) + " characters" );
                            if (name.length() > 2 && pass.length() > 0) {
                                instance.env.SetMediatorUserName(name);
                                instance.env.SetMediatorPassword(pass);

                                instance.env.RegisterAtMediators();
                            } else
                                Utils.LogE("LoginDialog", "SingletonInstance: Invalid username and password for Mediator entered!");
                        }
                    }
                });

                dlgBuilder.setNegativeButton ( "Cancel", new DialogInterface.OnClickListener () {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        ResetClose ();
                        instance.DisposeTimer();

                        if (instance.listener != null) {
                            instance.result = false;
                            instance.listener.onResult(instance);
                            return;
                        }

                        instance.act.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
                        instance.act.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);

                        if (instance.env.GetMediatorLoginDialogDismissDisable()) {
                            instance.env.SetUseCustomMediator(false);
                            instance.env.SetUseDefaultMediator(false);
                        }
                    }
                });

                if (instance.userName != null && instance.userName.length() > 2) {
                    userName.setText(instance.userName);
                    password.requestFocus();
                }
                else {
                    userName.setHint("User Name");
                    userName.requestFocus();
                }

                dlgBuilder.setOnKeyListener(new DialogInterface.OnKeyListener() {
                    public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                        instance.ReScheduleTimer ();
                        return false;
                    }
                });
                //editTextName.setOnFocusChangeListener(onFocusChangeListener);
            }
        });


        return instance;
    }

    /**
     * Reset the static login dialog instance.
     *
     */
    private static void ResetClose()
    {
        synchronized (classLock)
        {
            count = 0;
        }
    }


    /**
     * Dispose the no activity timer.
     *
     */
    private void DisposeTimer ()
    {
        if (Utils.isDebug) Utils.Log ( 8, className, "DisposeTimer" );

        if ( noActivityTimer != null ) {
            noActivityTimer.cancel();
            noActivityTimer = null;
        }
    }


    /**
     * Schedules a no activity timer that fires after the seconds declared by ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT.
     * Dispose a timer if a timer has been invoked before.
     *
     */
    private void ReScheduleTimer ()
    {
        if (Utils.isDebug) Utils.Log ( 8, className, "ReScheduleTimout" );

        DisposeTimer ();

        noActivityTimeout = new TimerTask() {
            @Override
            public void run() {
                if (Utils.isDebug) Utils.Log ( 4, className, "noActivityTimeout" );

                act.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (loginDlg != null) {
                            loginDlg.getButton(DialogInterface.BUTTON_NEGATIVE).performClick();
                        }
                    }
                });
            }
        };

        noActivityTimer = new Timer();
        noActivityTimer.schedule ( noActivityTimeout, Environs.ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT * 1000 );
    }


    /**
     * Show the login dialog within a thread and unblock the calling thread.
     *
     * @return  returns true on success, false otherwise.
     */
    public boolean ShowResult()
    {
        if (act == null)
            return false;

        Thread thread = new Thread() {
            @Override
            public void run() {
                ReScheduleTimer ();

                act.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        act.getWindow().setSoftInputMode(android.view.WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);

                        loginDlg = dlgBuilder.create();
                        loginDlg.show();
                    }
                });
            }
        };

        thread.start();
        return true;
    }


    /**
     * Set the listener that is called on dialog events.
     *
     * @param listener  A listener object of type Environs.OnResultListener
     */
    @SuppressWarnings ( "unused" )
    public void setOnResultListener(OnResultListener listener) {
        this.listener = listener;
    }


    /**
     * An interface for the callee that provides a method to be called on dialog events.
     *
     */
    static interface OnResultListener {
        void onResult(LoginDialog dlg);
    }
}
