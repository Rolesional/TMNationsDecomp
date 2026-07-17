package com.tmnf.port;

import com.google.androidgamesdk.GameActivity;

/** Minimal Android lifecycle host. All game/platform behavior lives in C++. */
public final class TmnfActivity extends GameActivity {
    static {
        System.loadLibrary("tmnf");
    }
}

