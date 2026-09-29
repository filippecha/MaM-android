package org.openenroth.game;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.GradientDrawable;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;

import org.libsdl.app.SDLActivity;

import java.util.ArrayList;
import java.util.List;

/**
 * On-screen controls placed in the black bars left and right of the 4:3 game picture.
 * Left bar holds look/fly keys, other keyboard keys, combat and turning, right bar holds the mouse button toggles, inventory, map, jump, pass and walking.
 */
final class TouchControls extends ViewGroup {
    private static final int COLOR_IDLE = 0x55FFFFFF;
    private static final int COLOR_ACTIVE = 0xCCE0B040;
    private static final int COLOR_STROKE = 0x99FFFFFF;

    private final List<View> _leftTop = new ArrayList<>();
    private final List<View> _leftMiddle = new ArrayList<>();
    private final List<View> _leftBottom = new ArrayList<>();
    private final List<View> _rightTop = new ArrayList<>();
    private final Button _forward;
    private final Button _backward;
    private final Button _leftMouse;
    private final Button _rightMouse;
    private final View _rightClickCatcher;
    private final View _gameView;
    private boolean _rightArmed = false;

    /**
     * @param gameView                  View the game draws into, a sibling of the controls in `root`.
     */
    static void attach(Context context, ViewGroup root, View gameView) {
        TouchControls controls = new TouchControls(context, gameView);
        root.addView(controls._rightClickCatcher, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
        root.addView(controls, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
    }

    /**
     * Width of each control bar. Tablets, anything narrower than 1.9:1, get wider bars than the black bars around the
     * 4:3 picture would give, and the game view is shrunk to fit between them.
     */
    static int barWidth(Context context, int width, int height) {
        float density = context.getResources().getDisplayMetrics().density;
        int natural = (width - height * 4 / 3) / 2;
        int minimum = (int) ((isTablet(width, height) ? 150 : 110) * density);
        return Math.max(natural, minimum);
    }

    static boolean isTablet(int width, int height) {
        return width < height * 1.9f;
    }

    private TouchControls(Context context, View gameView) {
        super(context);
        _gameView = gameView;
        setMotionEventSplittingEnabled(true);

        // First row looks (down, up, center), second row flies (down, land, up), as bound in the game.
        _leftTop.add(key("Del", 0, KeyEvent.KEYCODE_FORWARD_DEL));
        _leftTop.add(key("PgDn", 0, KeyEvent.KEYCODE_PAGE_DOWN));
        _leftTop.add(key("End", 0, KeyEvent.KEYCODE_MOVE_END));
        _leftTop.add(key("Ins", 0, KeyEvent.KEYCODE_INSERT));
        _leftTop.add(key("Home", 0, KeyEvent.KEYCODE_MOVE_HOME));
        _leftTop.add(key("PgUp", 0, KeyEvent.KEYCODE_PAGE_UP));

        _leftMiddle.add(key("Esc", 0, KeyEvent.KEYCODE_ESCAPE));
        _leftMiddle.add(key("Y křik", 0, KeyEvent.KEYCODE_Y));
        _leftMiddle.add(key("Mezera", 0, KeyEvent.KEYCODE_SPACE));
        _leftMiddle.add(key("Enter", 0, KeyEvent.KEYCODE_ENTER));
        _leftMiddle.add(key(null, R.drawable.ic_sword, KeyEvent.KEYCODE_A));
        _leftMiddle.add(key(null, R.drawable.ic_wand, KeyEvent.KEYCODE_S));

        _leftBottom.add(key(null, R.drawable.ic_arrow_left, KeyEvent.KEYCODE_DPAD_LEFT));
        _leftBottom.add(key(null, R.drawable.ic_arrow_right, KeyEvent.KEYCODE_DPAD_RIGHT));

        _leftMouse = new Button("Myš L", 0);
        _rightMouse = new Button("Myš P", 0);
        _leftMouse.setOnClickListener(v -> setRightArmed(false));
        _rightMouse.setOnClickListener(v -> setRightArmed(!_rightArmed));
        addView(_leftMouse);
        addView(_rightMouse);
        _rightTop.add(_leftMouse);
        _rightTop.add(_rightMouse);
        _rightTop.add(key(null, R.drawable.ic_backpack, KeyEvent.KEYCODE_I));
        _rightTop.add(key(null, R.drawable.ic_map, KeyEvent.KEYCODE_M));
        _rightTop.add(key("X skok", 0, KeyEvent.KEYCODE_X));
        _rightTop.add(key("B skip", 0, KeyEvent.KEYCODE_B));

        _forward = key(null, R.drawable.ic_arrow_up, KeyEvent.KEYCODE_DPAD_UP);
        _backward = key(null, R.drawable.ic_arrow_down, KeyEvent.KEYCODE_DPAD_DOWN);

        _rightClickCatcher = new View(context);
        _rightClickCatcher.setVisibility(GONE);
        _rightClickCatcher.setOnTouchListener(this::onRightClickTouch);

        setRightArmed(false);
    }

    private Button key(String caption, int iconRes, int keyCode) {
        Button view = new Button(caption, iconRes);
        view.setOnTouchListener((v, event) -> {
            switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                view.setActive(true);
                SDLActivity.onNativeKeyDown(keyCode);
                return true;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                view.setActive(false);
                SDLActivity.onNativeKeyUp(keyCode);
                return true;
            default:
                return true;
            }
        });
        addView(view);
        return view;
    }

    private static GradientDrawable background(int color) {
        GradientDrawable result = new GradientDrawable();
        result.setColor(color);
        result.setCornerRadius(16);
        result.setStroke(2, COLOR_STROKE);
        return result;
    }

    private void setRightArmed(boolean armed) {
        _rightArmed = armed;
        _rightMouse.setActive(armed);
        _leftMouse.setActive(!armed);
        _rightClickCatcher.setVisibility(armed ? VISIBLE : GONE);
    }

    /** While the right button is armed, the next touch on the game is a right mouse button press held for as long as the finger stays down. */
    private boolean onRightClickTouch(View view, MotionEvent event) {
        // The catcher covers the whole layout, the game view can be inset from it.
        float x = event.getX() + view.getLeft() - _gameView.getLeft();
        float y = event.getY() + view.getTop() - _gameView.getTop();
        switch (event.getActionMasked()) {
        case MotionEvent.ACTION_DOWN:
            SDLActivity.onNativeMouse(MotionEvent.BUTTON_SECONDARY, MotionEvent.ACTION_DOWN, x, y, false);
            return true;
        case MotionEvent.ACTION_MOVE:
            SDLActivity.onNativeMouse(MotionEvent.BUTTON_SECONDARY, MotionEvent.ACTION_MOVE, x, y, false);
            return true;
        case MotionEvent.ACTION_UP:
        case MotionEvent.ACTION_CANCEL:
            SDLActivity.onNativeMouse(0, MotionEvent.ACTION_UP, x, y, false);
            setRightArmed(false);
            return true;
        default:
            return true;
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        setMeasuredDimension(MeasureSpec.getSize(widthMeasureSpec), MeasureSpec.getSize(heightMeasureSpec));
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        int width = r - l;
        int height = b - t;
        int pad = dp(6);

        // The game picture is 4:3 and centered. On screens that are not wide enough the controls overlap its edges.
        int bar = barWidth(getContext(), width, height);
        int cellWidth = (bar - 3 * pad) / 2;
        int groupGap = 3 * pad;
        int cellHeight = Math.min(cellWidth, (height - 5 * pad - 2 * groupGap) / 6); // Left bar stacks 6 rows in 3 groups.

        // Left bar: keys at the top, keys with jump/pass in the middle, turning at the bottom.
        layoutGrid(_leftTop, 3, pad, pad, (bar - 4 * pad) / 3, cellHeight, pad);
        int leftBottomTop = height - cellHeight - pad;
        layoutGrid(_leftBottom, 2, pad, leftBottomTop, cellWidth, cellHeight, pad);
        if (isTablet(width, height)) {
            // Tablets have room to spare below the middle group, attack and quick spell sit right above turning.
            layoutGrid(_leftMiddle.subList(0, 4), 2, pad, pad + 2 * cellHeight + pad + groupGap, cellWidth, cellHeight, pad);
            layoutGrid(_leftMiddle.subList(4, 6), 2, pad, leftBottomTop - cellHeight - pad, cellWidth, cellHeight, pad);
        } else {
            layoutGrid(_leftMiddle, 2, pad, pad + 2 * cellHeight + pad + groupGap, cellWidth, cellHeight, pad);
        }

        // Right bar: mouse toggles, inventory/map and jump/pass at the top, walking at the bottom.
        int right = width - bar;
        layoutGrid(_rightTop, 2, right + pad, pad, cellWidth, cellHeight, pad);

        // Walking: forward directly on top of backward, alone at the bottom.
        int walkWidth = cellWidth;
        int walkHeight = Math.min(walkWidth * 5 / 4, (height - 3 * cellHeight - 6 * pad) / 2);
        int walkLeft = right + (bar - walkWidth) / 2;
        int walkTop = height - 2 * walkHeight - pad;
        place(_forward, walkLeft, walkTop, walkWidth, walkHeight);
        place(_backward, walkLeft, walkTop + walkHeight, walkWidth, walkHeight);
    }

    private void layoutGrid(List<View> views, int columns, int left, int top, int cellWidth, int cellHeight, int pad) {
        for (int i = 0; i < views.size(); i++)
            place(views.get(i), left + (i % columns) * (cellWidth + pad), top + (i / columns) * (cellHeight + pad), cellWidth, cellHeight);
    }

    private static void place(View view, int left, int top, int width, int height) {
        view.measure(MeasureSpec.makeMeasureSpec(width, MeasureSpec.EXACTLY), MeasureSpec.makeMeasureSpec(height, MeasureSpec.EXACTLY));
        view.layout(left, top, left + width, top + height);
    }

    private int dp(int value) {
        return (int) (value * getResources().getDisplayMetrics().density);
    }

    /** Rounded button with an optional icon and caption. Without an icon the caption is drawn large in the middle. */
    private final class Button extends View {
        private final String _caption;
        private final Drawable _icon;
        private final Paint _text = new Paint(Paint.ANTI_ALIAS_FLAG);

        Button(String caption, int iconRes) {
            super(TouchControls.this.getContext());
            _caption = caption;
            _icon = iconRes != 0 ? getContext().getDrawable(iconRes) : null;
            _text.setColor(Color.WHITE);
            _text.setTextAlign(Paint.Align.CENTER);
            _text.setTypeface(Typeface.DEFAULT_BOLD);
            setActive(false);
        }

        void setActive(boolean active) {
            setBackground(background(active ? COLOR_ACTIVE : COLOR_IDLE));
        }

        @Override
        protected void onDraw(Canvas canvas) {
            int w = getWidth();
            int h = getHeight();
            if (_icon == null) {
                _text.setTextSize(h * 0.38f);
                float textWidth = _text.measureText(_caption);
                if (textWidth > w * 0.84f)
                    _text.setTextSize(h * 0.38f * w * 0.84f / textWidth);
                canvas.drawText(_caption, w / 2f, h / 2f - (_text.ascent() + _text.descent()) / 2, _text);
                return;
            }

            boolean hasCaption = _caption != null;
            int size = (int) (Math.min(w, h) * (hasCaption ? 0.62f : 0.72f));
            int top = hasCaption ? (int) (h * 0.06f) : (h - size) / 2;
            _icon.setBounds((w - size) / 2, top, (w + size) / 2, top + size);
            _icon.draw(canvas);
            if (hasCaption) {
                _text.setTextSize(h * 0.22f);
                canvas.drawText(_caption, w / 2f, h * 0.93f, _text);
            }
        }
    }
}
