package com.android.support;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.view.View;

public class Launcher extends Service {

    Menu menu;

    @Override
    public void onCreate() {
        super.onCreate();

        menu = new Menu(this);
        menu.SetWindowManagerWindowService();
        menu.ShowMenu();

        // Create a handler for this Service
        final Handler handler = new Handler();
        handler.post(new Runnable() {
            @Override
            public void run() {
                Thread();
                handler.postDelayed(this, 1000);
            }
        });
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void Thread() {
        // Keep the menu visible when running
        if (menu != null) {
            menu.setVisibility(View.VISIBLE);
        }
    }

    // Destroy our View properly
    @Override
    public void onDestroy() {
        super.onDestroy();
        if (menu != null) {
            menu.onDestroy();
        }
    }

    @Override
    public void onTaskRemoved(Intent intent) {
        super.onTaskRemoved(intent);
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        stopSelf();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return Service.START_NOT_STICKY;
    }
}
