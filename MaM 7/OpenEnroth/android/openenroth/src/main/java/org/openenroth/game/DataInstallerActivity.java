package org.openenroth.game;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.graphics.Color;
import android.graphics.drawable.Icon;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Launcher activity. Unpacks the game data bundled in the APK into internal storage on first run,
 * then starts the game. The engine looks for game data in internal storage on its own.
 */
public class DataInstallerActivity extends Activity {
    private static final String ASSET_ROOT = "mm7";
    private static final String MARKER_PREFIX = ".mm7data_installed_";
    private static final String ACTION_SAVE_FOLDER = "org.openenroth.game.SAVE_FOLDER";
    private static final int PICK_FOLDER = 1;

    private final Handler _handler = new Handler(Looper.getMainLooper());
    private TextView _status;
    private ProgressBar _progress;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        buildUi();
        addShortcut();

        if (!hasBundledData() || markerFile().exists()) {
            afterInstall();
            return;
        }

        new Thread(this::install, "DataInstaller").start();
    }

    /** Asks for the save folder once, then brings the saved games up to date and starts the game. */
    private void afterInstall() {
        boolean choose = ACTION_SAVE_FOLDER.equals(getIntent().getAction());
        if (choose) {
            pickFolder();
        } else if (SaveSync.folder(this) == null && !SaveSync.wasAsked(this)) {
            new AlertDialog.Builder(this, android.R.style.Theme_DeviceDefault_Dialog_Alert)
                    .setTitle(R.string.sync_title)
                    .setMessage(R.string.sync_message)
                    .setCancelable(false)
                    .setPositiveButton(R.string.sync_pick, (dialog, which) -> pickFolder())
                    .setNegativeButton(R.string.sync_later, (dialog, which) -> {
                        SaveSync.setAsked(this);
                        syncAndStart();
                    })
                    .show();
        } else {
            syncAndStart();
        }
    }

    private void pickFolder() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
                | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
        try {
            startActivityForResult(intent, PICK_FOLDER);
        } catch (RuntimeException e) {
            Toast.makeText(this, R.string.sync_unavailable, Toast.LENGTH_LONG).show();
            SaveSync.setAsked(this);
            syncAndStart();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != PICK_FOLDER)
            return;
        Uri tree = resultCode == RESULT_OK && data != null ? data.getData() : null;
        if (tree != null) {
            try {
                getContentResolver().takePersistableUriPermission(tree,
                        Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                SaveSync.setFolder(this, tree);
            } catch (RuntimeException e) {
                Toast.makeText(this, R.string.sync_unavailable, Toast.LENGTH_LONG).show();
            }
        } else {
            SaveSync.setAsked(this);
        }
        syncAndStart();
    }

    private void syncAndStart() {
        if (SaveSync.folder(this) == null) {
            startGame();
            return;
        }
        _progress.setIndeterminate(true);
        _status.setText(R.string.sync_running);
        new Thread(() -> {
            boolean ok = SaveSync.sync(this);
            _handler.post(() -> {
                if (!ok)
                    Toast.makeText(this, R.string.sync_failed, Toast.LENGTH_LONG).show();
                startGame();
            });
        }, "SaveSync").start();
    }

    /** Long press on the launcher icon offers changing the save folder. */
    private void addShortcut() {
        if (Build.VERSION.SDK_INT < 25)
            return;
        ShortcutManager manager = getSystemService(ShortcutManager.class);
        if (manager == null)
            return;
        Intent intent = new Intent(this, DataInstallerActivity.class).setAction(ACTION_SAVE_FOLDER);
        ShortcutInfo shortcut = new ShortcutInfo.Builder(this, "save_folder")
                .setShortLabel(getString(R.string.sync_shortcut))
                .setIcon(Icon.createWithResource(this, R.mipmap.ic_launcher))
                .setIntent(intent)
                .build();
        try {
            manager.setDynamicShortcuts(Collections.singletonList(shortcut));
        } catch (RuntimeException e) {
            // Launchers may refuse shortcuts, the dialog on first start still offers the choice.
        }
    }

    private void buildUi() {
        int pad = (int) (32 * getResources().getDisplayMetrics().density);

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setGravity(Gravity.CENTER);
        layout.setPadding(pad, pad, pad, pad);
        layout.setBackgroundColor(Color.BLACK);

        _status = new TextView(this);
        _status.setTextColor(Color.WHITE);
        _status.setTextSize(18);
        _status.setGravity(Gravity.CENTER);
        _status.setText(R.string.installer_preparing);
        layout.addView(_status);

        _progress = new ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal);
        _progress.setMax(1000);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        params.topMargin = pad / 2;
        layout.addView(_progress, params);

        setContentView(layout);
    }

    private boolean hasBundledData() {
        try {
            String[] entries = getAssets().list(ASSET_ROOT);
            return entries != null && entries.length > 0;
        } catch (IOException e) {
            return false;
        }
    }

    private File markerFile() {
        long version;
        try {
            version = getPackageManager().getPackageInfo(getPackageName(), 0).lastUpdateTime;
        } catch (Exception e) {
            version = 0;
        }
        return new File(getFilesDir(), MARKER_PREFIX + version);
    }

    private void install() {
        try {
            AssetManager assets = getAssets();
            List<String> files = new ArrayList<>();
            collect(assets, ASSET_ROOT, files);

            long total = 0;
            for (String file : files)
                total += assetLength(assets, file);

            long free = getFilesDir().getUsableSpace();
            if (total > 0 && free < total + (64L << 20)) {
                fail(getString(R.string.installer_no_space, (total >> 20) + 64, free >> 20));
                return;
            }

            File[] stale = getFilesDir().listFiles((dir, name) -> name.startsWith(MARKER_PREFIX));
            if (stale != null)
                for (File file : stale)
                    file.delete();

            byte[] buffer = new byte[1 << 20];
            long done = 0;
            for (String file : files) {
                File target = new File(getFilesDir(), file.substring(ASSET_ROOT.length() + 1));
                File parent = target.getParentFile();
                if (parent != null && !parent.isDirectory() && !parent.mkdirs())
                    throw new IOException("Cannot create " + parent);

                try (InputStream in = assets.open(file, AssetManager.ACCESS_STREAMING);
                     OutputStream out = new FileOutputStream(target)) {
                    int read;
                    while ((read = in.read(buffer)) > 0) {
                        out.write(buffer, 0, read);
                        done += read;
                        report(done, total);
                    }
                }
            }

            if (!markerFile().createNewFile())
                throw new IOException("Cannot create marker file");

            _handler.post(this::afterInstall);
        } catch (IOException e) {
            fail(getString(R.string.installer_failed, String.valueOf(e.getMessage())));
        }
    }

    private static void collect(AssetManager assets, String path, List<String> out) throws IOException {
        String[] children = assets.list(path);
        if (children == null || children.length == 0) {
            out.add(path); // AssetManager lists nothing for a file.
            return;
        }
        for (String child : children)
            collect(assets, path + "/" + child, out);
    }

    private static long assetLength(AssetManager assets, String path) {
        try (AssetFileDescriptor fd = assets.openFd(path)) { // Only works for assets stored uncompressed.
            return fd.getLength();
        } catch (IOException e) {
            return 0;
        }
    }

    private long _lastReport = 0;

    private void report(long done, long total) {
        long now = System.currentTimeMillis();
        if (now - _lastReport < 100 && done < total)
            return;
        _lastReport = now;

        _handler.post(() -> {
            if (total > 0) {
                _progress.setProgress((int) (done * 1000 / total));
                _status.setText(getString(R.string.installer_progress, done >> 20, total >> 20));
            } else {
                _progress.setIndeterminate(true);
                _status.setText(getString(R.string.installer_progress_unknown, done >> 20));
            }
        });
    }

    private void fail(String message) {
        _handler.post(() -> {
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            _status.setText(message);
        });
    }

    private void startGame() {
        if (isFinishing())
            return;
        startActivity(new Intent(this, OpenEnroth.class));
        finish();
    }
}
