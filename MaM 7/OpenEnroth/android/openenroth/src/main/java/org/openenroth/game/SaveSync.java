package org.openenroth.game;

import android.content.ContentResolver;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;
import java.util.TreeSet;

/**
 * Keeps the saved games in sync with a folder the player picked, so that a sync app (for example one for Google
 * Drive) can carry them to other devices. The engine keeps saving into its own private folder, the files are
 * copied when the game starts and when it goes to the background.
 *
 * Each file is compared with its state after the last sync. A file changed on one side only is copied over.
 * A file changed on both sides keeps the newer version and the other one goes to the "zaloha" subfolder of the
 * picked folder, so no save is ever lost.
 */
public final class SaveSync {
    private static final String TAG = "SaveSync";
    private static final String PREFS = "save_sync";
    private static final String KEY_TREE = "tree";
    private static final String KEY_ASKED = "asked";
    private static final String BACKUP_DIR = "zaloha";
    private static final String STATE_FILE = "save-sync-state.properties";

    private SaveSync() {}

    private static SharedPreferences prefs(Context context) {
        return context.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
    }

    public static Uri folder(Context context) {
        String tree = prefs(context).getString(KEY_TREE, null);
        return tree == null ? null : Uri.parse(tree);
    }

    public static void setFolder(Context context, Uri tree) {
        prefs(context).edit().putString(KEY_TREE, tree.toString()).putBoolean(KEY_ASKED, true).apply();
        // A new folder starts without history, the first sync compares the contents.
        new File(context.getFilesDir(), STATE_FILE).delete();
    }

    public static boolean wasAsked(Context context) {
        return prefs(context).getBoolean(KEY_ASKED, false);
    }

    public static void setAsked(Context context) {
        prefs(context).edit().putBoolean(KEY_ASKED, true).apply();
    }

    static File localDir(Context context) {
        return new File(context.getFilesDir(), ".openenroth/saves");
    }

    /** Syncs in both directions. Returns false when the picked folder is not accessible. */
    public static synchronized boolean sync(Context context) {
        Uri tree = folder(context);
        if (tree == null)
            return true;
        try {
            new Run(context, tree).sync();
            return true;
        } catch (IOException | RuntimeException e) {
            Log.w(TAG, "Save sync failed", e);
            return false;
        }
    }

    public static void syncInBackground(Context context) {
        Context app = context.getApplicationContext();
        if (folder(app) != null)
            new Thread(() -> sync(app), "SaveSync").start();
    }

    private static final class Remote {
        String id;
        long modified;
        long size;
    }

    private static final class Run {
        private final Context _context;
        private final ContentResolver _resolver;
        private final Uri _tree;
        private final String _rootId;
        private final File _local;
        private final File _stateFile;
        private final Properties _state = new Properties();

        Run(Context context, Uri tree) {
            _context = context;
            _resolver = context.getContentResolver();
            _tree = tree;
            _rootId = DocumentsContract.getTreeDocumentId(tree);
            _local = localDir(context);
            _stateFile = new File(context.getFilesDir(), STATE_FILE);
        }

        void sync() throws IOException {
            if (!_local.isDirectory() && !_local.mkdirs())
                throw new IOException("Cannot create " + _local);
            if (_stateFile.exists())
                try (InputStream in = new FileInputStream(_stateFile)) {
                    _state.load(in);
                }

            Map<String, Remote> remote = list(_rootId, false);
            Map<String, File> local = new HashMap<>();
            File[] files = _local.listFiles();
            if (files != null)
                for (File file : files)
                    if (file.isFile())
                        local.put(file.getName(), file);

            TreeSet<String> names = new TreeSet<>(local.keySet());
            names.addAll(remote.keySet());
            for (String name : names) {
                if (name.startsWith("."))
                    continue;
                syncFile(name, local.get(name), remote.get(name));
            }

            try (OutputStream out = new FileOutputStream(_stateFile)) {
                _state.store(out, null);
            }
        }

        private void syncFile(String name, File local, Remote remote) throws IOException {
            String recorded = _state.getProperty(name);
            if (local == null) {
                local = new File(_local, name);
                pull(remote, local);
            } else if (remote == null) {
                remote = push(local, name, null);
            } else {
                boolean localChanged = recorded == null || !recorded.startsWith(localKey(local) + "|");
                boolean remoteChanged = recorded == null || !recorded.endsWith("|" + remoteKey(remote));
                if (localChanged && remoteChanged) {
                    if (!sameContents(local, remote)) {
                        // Changed on both sides: the newer file wins, the other one is kept in the backup folder.
                        if (remote.modified > local.lastModified()) {
                            backupLocal(local);
                            pull(remote, local);
                        } else {
                            backupRemote(name, remote);
                            remote = push(local, name, remote);
                        }
                    }
                } else if (localChanged) {
                    remote = push(local, name, remote);
                } else if (remoteChanged) {
                    pull(remote, local);
                }
            }
            if (remote != null)
                remote = stat(remote.id);
            if (remote != null && local.isFile())
                _state.setProperty(name, localKey(local) + "|" + remoteKey(remote));
        }

        private static String localKey(File file) {
            return file.lastModified() + ":" + file.length();
        }

        private static String remoteKey(Remote remote) {
            return remote.modified + ":" + remote.size;
        }

        private void pull(Remote remote, File local) throws IOException {
            File temp = new File(local.getPath() + ".sync");
            try (InputStream in = _resolver.openInputStream(docUri(remote.id));
                 OutputStream out = new FileOutputStream(temp)) {
                copy(in, out);
            }
            if (!temp.renameTo(local)) {
                temp.delete();
                throw new IOException("Cannot replace " + local);
            }
            if (remote.modified > 0)
                local.setLastModified(remote.modified);
        }

        private Remote push(File local, String name, Remote existing) throws IOException {
            String id = existing != null ? existing.id : create(_rootId, name, "application/octet-stream");
            try (InputStream in = new FileInputStream(local);
                 OutputStream out = _resolver.openOutputStream(docUri(id), "wt")) {
                copy(in, out);
            }
            Remote result = stat(id);
            if (result == null)
                throw new IOException("Cannot read back " + name);
            return result;
        }

        private void backupLocal(File local) throws IOException {
            String id = create(backupDirId(), backupName(local.getName(), "telefon"), "application/octet-stream");
            try (InputStream in = new FileInputStream(local);
                 OutputStream out = _resolver.openOutputStream(docUri(id), "wt")) {
                copy(in, out);
            }
        }

        private void backupRemote(String name, Remote remote) throws IOException {
            String id = create(backupDirId(), backupName(name, "slozka"), "application/octet-stream");
            try (InputStream in = _resolver.openInputStream(docUri(remote.id));
                 OutputStream out = _resolver.openOutputStream(docUri(id), "wt")) {
                copy(in, out);
            }
        }

        private static String backupName(String name, String origin) {
            String stamp = new SimpleDateFormat("yyyy-MM-dd_HH-mm-ss", Locale.ROOT).format(new Date());
            int dot = name.lastIndexOf('.');
            String base = dot > 0 ? name.substring(0, dot) : name;
            String ext = dot > 0 ? name.substring(dot) : "";
            return base + "_" + stamp + "_" + origin + ext;
        }

        private String backupDirId() throws IOException {
            Remote dir = list(_rootId, true).get(BACKUP_DIR);
            if (dir != null)
                return dir.id;
            return create(_rootId, BACKUP_DIR, DocumentsContract.Document.MIME_TYPE_DIR);
        }

        private boolean sameContents(File local, Remote remote) throws IOException {
            if (local.length() != remote.size)
                return false;
            byte[] a = new byte[1 << 16];
            byte[] b = new byte[1 << 16];
            try (InputStream l = new FileInputStream(local);
                 InputStream r = _resolver.openInputStream(docUri(remote.id))) {
                while (true) {
                    int n = readFully(l, a);
                    int m = readFully(r, b);
                    if (n != m)
                        return false;
                    if (n <= 0)
                        return true;
                    if (!Arrays.equals(Arrays.copyOf(a, n), Arrays.copyOf(b, m)))
                        return false;
                }
            }
        }

        private static int readFully(InputStream in, byte[] buffer) throws IOException {
            int total = 0;
            while (total < buffer.length) {
                int read = in.read(buffer, total, buffer.length - total);
                if (read < 0)
                    break;
                total += read;
            }
            return total;
        }

        private Uri docUri(String id) {
            return DocumentsContract.buildDocumentUriUsingTree(_tree, id);
        }

        private String create(String parentId, String name, String mime) throws IOException {
            Uri uri = DocumentsContract.createDocument(_resolver, docUri(parentId), mime, name);
            if (uri == null)
                throw new IOException("Cannot create " + name);
            return DocumentsContract.getDocumentId(uri);
        }

        private static final String[] COLUMNS = {
            DocumentsContract.Document.COLUMN_DOCUMENT_ID,
            DocumentsContract.Document.COLUMN_DISPLAY_NAME,
            DocumentsContract.Document.COLUMN_LAST_MODIFIED,
            DocumentsContract.Document.COLUMN_SIZE,
            DocumentsContract.Document.COLUMN_MIME_TYPE,
        };

        private Map<String, Remote> list(String parentId, boolean directories) throws IOException {
            Map<String, Remote> result = new HashMap<>();
            Uri children = DocumentsContract.buildChildDocumentsUriUsingTree(_tree, parentId);
            try (Cursor cursor = _resolver.query(children, COLUMNS, null, null, null)) {
                if (cursor == null)
                    throw new IOException("Cannot list the save folder");
                while (cursor.moveToNext()) {
                    boolean dir = DocumentsContract.Document.MIME_TYPE_DIR.equals(cursor.getString(4));
                    if (dir != directories)
                        continue;
                    Remote remote = new Remote();
                    remote.id = cursor.getString(0);
                    remote.modified = cursor.isNull(2) ? 0 : cursor.getLong(2);
                    remote.size = cursor.isNull(3) ? 0 : cursor.getLong(3);
                    result.put(cursor.getString(1), remote);
                }
            }
            return result;
        }

        private Remote stat(String id) throws IOException {
            try (Cursor cursor = _resolver.query(docUri(id), COLUMNS, null, null, null)) {
                if (cursor == null || !cursor.moveToFirst())
                    return null;
                Remote remote = new Remote();
                remote.id = id;
                remote.modified = cursor.isNull(2) ? 0 : cursor.getLong(2);
                remote.size = cursor.isNull(3) ? 0 : cursor.getLong(3);
                return remote;
            }
        }

        private static void copy(InputStream in, OutputStream out) throws IOException {
            if (in == null || out == null)
                throw new IOException("Cannot open a save file");
            byte[] buffer = new byte[1 << 16];
            int read;
            while ((read = in.read(buffer)) > 0)
                out.write(buffer, 0, read);
        }
    }
}
