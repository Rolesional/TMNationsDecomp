package com.tmnf.port;

import android.graphics.Color;
import android.os.Bundle;
import android.os.SystemClock;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import com.google.androidgamesdk.GameActivity;

import java.util.ArrayList;
import java.util.List;

/** Android lifecycle host with a small multi-touch driving overlay. */
public final class TmnfActivity extends GameActivity {
    private final List<HeldKeyTouchListener> heldKeys = new ArrayList<>();

    static {
        System.loadLibrary("tmnf");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addContentView(createDrivingControls(), new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT));
    }

    @Override
    protected void onPause() {
        for (HeldKeyTouchListener key : heldKeys) {
            key.release();
        }
        super.onPause();
    }

    private View createDrivingControls() {
        FrameLayout overlay = new FrameLayout(this);

        LinearLayout steering = new LinearLayout(this);
        steering.setOrientation(LinearLayout.HORIZONTAL);
        addControl(steering, "\u25c0", "Steer left", KeyEvent.KEYCODE_A);
        addControl(steering, "\u25b6", "Steer right", KeyEvent.KEYCODE_D);
        FrameLayout.LayoutParams steeringParams = new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.WRAP_CONTENT, FrameLayout.LayoutParams.WRAP_CONTENT,
                Gravity.START | Gravity.BOTTOM);
        steeringParams.setMargins(dp(20), 0, 0, dp(20));
        overlay.addView(steering, steeringParams);

        LinearLayout pedals = new LinearLayout(this);
        pedals.setOrientation(LinearLayout.VERTICAL);
        addControl(pedals, "\u25b2", "Drive forward", KeyEvent.KEYCODE_W);
        addControl(pedals, "\u25bc", "Drive in reverse", KeyEvent.KEYCODE_S);
        FrameLayout.LayoutParams pedalParams = new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.WRAP_CONTENT, FrameLayout.LayoutParams.WRAP_CONTENT,
                Gravity.END | Gravity.BOTTOM);
        pedalParams.setMargins(0, 0, dp(20), dp(20));
        overlay.addView(pedals, pedalParams);
        return overlay;
    }

    private void addControl(LinearLayout group, String label, String description, int keyCode) {
        Button button = new Button(this);
        button.setText(label);
        button.setTextColor(Color.WHITE);
        button.setTextSize(28.0f);
        button.setGravity(Gravity.CENTER);
        button.setIncludeFontPadding(false);
        button.setPadding(0, 0, 0, 0);
        button.setBackgroundColor(Color.argb(116, 0, 0, 0));
        button.setContentDescription(description);
        button.setFocusable(false);
        button.setFocusableInTouchMode(false);
        HeldKeyTouchListener listener = new HeldKeyTouchListener(keyCode);
        heldKeys.add(listener);
        button.setOnTouchListener(listener);

        int size = dp(72);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(size, size);
        params.setMargins(dp(4), dp(4), dp(4), dp(4));
        group.addView(button, params);
    }

    private int dp(int value) {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }

    private final class HeldKeyTouchListener implements View.OnTouchListener {
        private final int keyCode;
        private boolean held;
        private long downTime;
        private View pressedView;

        HeldKeyTouchListener(int keyCode) {
            this.keyCode = keyCode;
        }

        @Override
        public boolean onTouch(View view, MotionEvent event) {
            switch (event.getActionMasked()) {
                case MotionEvent.ACTION_DOWN:
                    press(view);
                    return true;
                case MotionEvent.ACTION_UP:
                    release();
                    view.performClick();
                    return true;
                case MotionEvent.ACTION_CANCEL:
                    release();
                    return true;
                default:
                    return true;
            }
        }

        private void press(View view) {
            if (held) {
                return;
            }
            held = true;
            pressedView = view;
            pressedView.setAlpha(0.82f);
            downTime = SystemClock.uptimeMillis();
            dispatchKeyEvent(new KeyEvent(downTime, downTime, KeyEvent.ACTION_DOWN, keyCode, 0));
        }

        void release() {
            if (!held) {
                return;
            }
            held = false;
            if (pressedView != null) {
                pressedView.setAlpha(1.0f);
                pressedView = null;
            }
            long now = SystemClock.uptimeMillis();
            dispatchKeyEvent(new KeyEvent(downTime, now, KeyEvent.ACTION_UP, keyCode, 0));
        }
    }
}
