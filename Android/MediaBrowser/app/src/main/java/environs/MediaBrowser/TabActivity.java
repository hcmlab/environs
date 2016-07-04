package environs.MediaBrowser;
/**
 *	TabActivity
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
import java.util.ArrayList;
import com.actionbarsherlock.app.SherlockFragmentActivity;

import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TabHost;
import android.widget.TabWidget;

import environs.*;

/**
* 	TabActivity class, implements the fragment activity that holds the tabs for housing the fragments
*	---------------------------------------------------------
*	Copyright (C) Chi-Tai Dang
*   All rights reserved.
*
*	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
*	@version	1.0
* ****************************************************************************************
*/
public class TabActivity extends SherlockFragmentActivity {
	private static String className = "TabActivity";
   
    /** A static instance of the "application" */
    public static TabActivity instance;
    
    public static Activity currentActivity;
    
    
    /** The environment object that we participating in. */
    public static Environs env;

    /** Definition of the theme (Sherlock) to use for this app. */
    public static int THEME = R.style.Theme_Sherlock;
        
    // Tab managing
    public TabHost g_tabHost;

    @SuppressWarnings ( "all" )
    private ViewPager  g_viewPager;

    @SuppressWarnings ( "all" )
    private TabsAdapter g_tabsAdapter;    

    public static String areaName = "Environs";
    public static String appName = "MediaBrowser";
    public static OurObserver observer = new OurObserver();
    
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Utils.Log(4, className, "onCreate");

        instance = this;
        
        setTheme(TabActivity.THEME);
       
        setContentView(R.layout.tabs);
		
        
        g_tabHost = (TabHost)findViewById(android.R.id.tabhost);
        g_tabHost.setup();

        g_viewPager = (ViewPager)findViewById(R.id.pager);

        g_tabsAdapter = new TabsAdapter(this, g_tabHost, g_viewPager);

        g_tabsAdapter.addTab(g_tabHost.newTabSpec("main").setIndicator("Home"),
        		MainTab.class, null);
        g_tabsAdapter.addTab(g_tabHost.newTabSpec("devices").setIndicator("Devices"),
        		DevicelistTab.class, null);
        g_tabsAdapter.addTab(g_tabHost.newTabSpec("settings").setIndicator("Settings"),
                SettingsTab.class, null);
        
        if (savedInstanceState != null) {
            g_tabHost.setCurrentTabByTag(savedInstanceState.getString("tab"));
        }

        //Libs.setReinitNativeLayer();

        // Create the environment
        env = Environs.CreateInstance ( this, appName, areaName );

        if ( env == null ) {
            Utils.LogE("Failed to create an Environs object!");
            return;
        }

        env.SetUseMediatorAnonymousLogon ( true );

        //env.SetMediatorFilterLevel ( MediatorFilter.None );
        env.SetMediatorFilterLevel ( MediatorFilter.AreaAndApp );

        env.AddObserver ( observer );
        env.AddObserverForMessages ( observer );
        env.AddObserverForData(observer);
    }
    
    @Override
    public void onResume() {    
    	Utils.Log(4, className, "onResume");
        super.onResume();
        currentActivity = this;
    }

    @Override
    protected void onPause() {
    	Utils.Log(4, className, "onPause");
        super.onPause();
    }  
    
    @Override
    protected void onDestroy () {
    	Utils.Log(4, className, "onDestroy");
    	instance = null;
    	super.onDestroy();     	
    }


    
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_MENU) {
        	Utils.Log ( 4, className, "onKeyUp Menu");
        }
        else if (keyCode == KeyEvent.KEYCODE_BACK) {
        	if (Fullscreen.instance != null)
        		Fullscreen.instance.finish();

            if ( g_tabHost.getCurrentTab() != 0 ) {
            	g_tabHost.setCurrentTab(0);
            	return true;
            }
            else if (env != null) {
                if (env.GetStatus() > 2) {
                    env.Stop();
                }
            }
        }
        return super.onKeyUp(keyCode, event);
    }
    

    @SuppressWarnings ( "all" )
    public static class TabsAdapter extends FragmentPagerAdapter
            implements TabHost.OnTabChangeListener, ViewPager.OnPageChangeListener {
    	
        private final Context mContext;
        private final TabHost mTabHost;
        private final ViewPager mViewPager;
        private final ArrayList<TabInfo> mTabs = new ArrayList<TabInfo>();

        static final class TabInfo {
            private final Class<?> clss;
            private final Bundle args;

            TabInfo(String _tag, Class<?> _class, Bundle _args) {
                clss = _class;
                args = _args;
            }
        }

        static class DummyTabFactory implements TabHost.TabContentFactory {
            private final Context mContext;

            public DummyTabFactory(Context context) {
                mContext = context;
            }

            @Override
            public View createTabContent(String tag) {
                View v = new View(mContext);
                v.setMinimumWidth(0);
                v.setMinimumHeight(0);
                return v;
            }
        }

        public TabsAdapter(FragmentActivity activity, TabHost tabHost, ViewPager pager) {
            super(activity.getSupportFragmentManager());
            mContext = activity;
            mTabHost = tabHost;
            mViewPager = pager;
            mTabHost.setOnTabChangedListener(this);
            mViewPager.setAdapter(this);
            mViewPager.setOnPageChangeListener(this);
        }

        public void addTab(TabHost.TabSpec tabSpec, Class<?> clss, Bundle args) {
            tabSpec.setContent(new DummyTabFactory(mContext));
            String tag = tabSpec.getTag();

            TabInfo info = new TabInfo(tag, clss, args);
            mTabs.add(info);
            mTabHost.addTab(tabSpec);
            notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            return mTabs.size();
        }

        @Override
        public Fragment getItem(int position) {
            TabInfo info = mTabs.get(position);
            return Fragment.instantiate(mContext, info.clss.getName(), info.args);
        }

        @Override
        public void onTabChanged(String tabId) {
            int position = mTabHost.getCurrentTab();
            mViewPager.setCurrentItem(position);
        }

        @Override
        public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
        }

        @Override
        public void onPageSelected(int position) {
            TabWidget widget = mTabHost.getTabWidget();
            int oldFocusability = widget.getDescendantFocusability();
            widget.setDescendantFocusability(ViewGroup.FOCUS_BLOCK_DESCENDANTS);
            mTabHost.setCurrentTab(position);
            widget.setDescendantFocusability(oldFocusability);
            
            if (!Utils.bigScreen) {
	            if (position == 2 || position == 1) {
	            	instance.getSupportActionBar().hide();
	            } else {
	            	instance.getSupportActionBar().show();            	
	            }
            }
            
            String title;
            switch(position) {
            case 0:
            	title = "MediaViewer";
            	break;
            case 1:
            	title = "Devices";
            	break;
            default:
            	title = "Settings";
            	break;
            }
            instance.setTitle(title);
        }

        @Override
        public void onPageScrollStateChanged(int state) {
        }
    }

}
