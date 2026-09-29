package org.openenroth.game;

import android.content.ClipboardManager;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Insets;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowInsets;
import android.view.WindowInsetsController;

import org.libsdl.app.SDLActivity;

public class OpenEnroth extends SDLActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // SDL 3.2.22 crashes in its native clipboard callback when the clipboard changes. The game doesn't use the clipboard.
        ClipboardManager.OnPrimaryClipChangedListener listener = mClipboardHandler;
        if (listener != null)
            ((ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE)).removePrimaryClipChangedListener(listener);

        if (mLayout == null || mSurface == null)
            return;

        TouchControls.attach(this, mLayout, mSurface);
        mLayout.setBackgroundColor(Color.BLACK); // Shows beside the game view when it's narrowed on tablets.

        // Tablets can keep a navigation bar or a taskbar on screen even in immersive mode, and it is drawn over the
        // game. The game and the controls are laid out in the area that stays visible. Display cutouts are left alone,
        // the black bars around the 4:3 picture cover them.
        if (Build.VERSION.SDK_INT >= 30) {
            mLayout.setOnApplyWindowInsetsListener((view, insets) -> {
                Insets bars = insets.getInsets(WindowInsets.Type.systemBars());
                view.setPadding(bars.left, bars.top, bars.right, bars.bottom);
                return insets;
            });
        }

        mLayout.addOnLayoutChangeListener((view, left, top, right, bottom, oldLeft, oldTop, oldRight, oldBottom) -> fitGameView());
    }

    @Override
    protected void onStop() {
        super.onStop();
        // Saves made in this session go to the sync folder when the game leaves the screen.
        SaveSync.syncInBackground(this);
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        // Picks up saves another device put into the sync folder meanwhile.
        SaveSync.syncInBackground(this);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus)
            hideSystemBars();
    }

    /** SDL hides the system bars with the old flags, which some tablets ignore for their taskbar. */
    private void hideSystemBars() {
        if (Build.VERSION.SDK_INT < 30)
            return;
        WindowInsetsController controller = getWindow().getInsetsController();
        if (controller == null)
            return;
        controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        controller.hide(WindowInsets.Type.systemBars());
    }

    /** Shrinks the game view on tablets so that it ends where the control bars begin. */
    private void fitGameView() {
        int width = mLayout.getWidth() - mLayout.getPaddingLeft() - mLayout.getPaddingRight();
        int height = mLayout.getHeight() - mLayout.getPaddingTop() - mLayout.getPaddingBottom();
        if (width <= 0 || height <= 0)
            return;

        int margin = 0;
        if (TouchControls.isTablet(width, height))
            margin = TouchControls.barWidth(this, width, height);

        View surface = mSurface;
        if (!(surface.getLayoutParams() instanceof ViewGroup.MarginLayoutParams))
            return;
        ViewGroup.MarginLayoutParams params = (ViewGroup.MarginLayoutParams) surface.getLayoutParams();
        if (params.leftMargin == margin && params.rightMargin == margin)
            return;
        params.leftMargin = margin;
        params.rightMargin = margin;
        surface.post(() -> surface.setLayoutParams(params));
    }
}
