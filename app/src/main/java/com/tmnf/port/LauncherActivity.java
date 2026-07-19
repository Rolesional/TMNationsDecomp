package com.tmnf.port;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

/** Imports the user's original game data before starting the native game activity. */
public final class LauncherActivity extends Activity {
    private static final int REQUEST_GAME_FOLDER = 1;

    private TextView status;
    private Button selectFolder;
    private ProgressBar progress;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (hasRequiredAssets()) {
            launchGame();
            return;
        }
        showImporter();
    }

    private void showImporter() {
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setGravity(Gravity.CENTER);
        layout.setPadding(64, 48, 64, 48);
        layout.setBackgroundColor(Color.rgb(6, 19, 36));

        status = new TextView(this);
        status.setText(R.string.import_instructions);
        status.setTextColor(Color.WHITE);
        status.setTextSize(18.0f);
        status.setGravity(Gravity.CENTER);
        layout.addView(status, new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));

        selectFolder = new Button(this);
        selectFolder.setText(R.string.select_game_folder);
        selectFolder.setOnClickListener(view -> selectGameFolder());
        LinearLayout.LayoutParams buttonParams = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        buttonParams.topMargin = 32;
        layout.addView(selectFolder, buttonParams);

        progress = new ProgressBar(this);
        progress.setIndeterminate(true);
        progress.setVisibility(View.GONE);
        LinearLayout.LayoutParams progressParams = new LinearLayout.LayoutParams(64, 64);
        progressParams.topMargin = 24;
        layout.addView(progress, progressParams);

        setContentView(layout);
    }

    private void selectGameFolder() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI,
                DocumentsContract.buildDocumentUri(
                        "com.android.externalstorage.documents", "primary:Download"));
        startActivityForResult(intent, REQUEST_GAME_FOLDER);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != REQUEST_GAME_FOLDER || resultCode != RESULT_OK || data == null
                || data.getData() == null) {
            return;
        }
        Uri tree = data.getData();
        selectFolder.setEnabled(false);
        progress.setVisibility(View.VISIBLE);
        status.setText(R.string.importing_game_data);
        new Thread(() -> importGameFolder(tree), "tmnf-asset-import").start();
    }

    private void importGameFolder(Uri tree) {
        try {
            File target = getFilesDir();
            File complete = new File(target, ".tmnf-assets-ready");
            if (complete.exists() && !complete.delete()) {
                throw new IOException("Unable to replace the previous import");
            }
            ContentResolver resolver = getContentResolver();
            String rootId = DocumentsContract.getTreeDocumentId(tree);
            List<DocumentEntry> roots = children(resolver, tree, rootId);
            DocumentEntry packs = namedDirectory(roots, "Packs");
            DocumentEntry gameData = namedDirectory(roots, "GameData");
            if (packs == null || gameData == null) {
                throw new IOException("Select the TmNationsForever folder containing Packs and GameData");
            }
            copyDirectory(resolver, tree, packs.id, new File(target, "Packs"));
            copyDirectory(resolver, tree, gameData.id, new File(target, "GameData"));
            if (!hasCoreAssets()) {
                throw new IOException("The selected folder is missing the Nations campaign or Stadium pack");
            }
            try (FileOutputStream marker = new FileOutputStream(complete, false)) {
                marker.write(1);
            }
            runOnUiThread(this::launchGame);
        } catch (Exception error) {
            runOnUiThread(() -> {
                progress.setVisibility(View.GONE);
                selectFolder.setEnabled(true);
                status.setText(getString(R.string.import_failed, error.getMessage()));
            });
        }
    }

    private static DocumentEntry namedDirectory(List<DocumentEntry> entries, String name) {
        for (DocumentEntry entry : entries) {
            if (entry.directory && entry.name.equalsIgnoreCase(name)) {
                return entry;
            }
        }
        return null;
    }

    private static List<DocumentEntry> children(ContentResolver resolver, Uri tree, String parentId)
            throws IOException {
        Uri uri = DocumentsContract.buildChildDocumentsUriUsingTree(tree, parentId);
        String[] columns = {
                DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                DocumentsContract.Document.COLUMN_MIME_TYPE
        };
        ArrayList<DocumentEntry> result = new ArrayList<>();
        try (Cursor cursor = resolver.query(uri, columns, null, null, null)) {
            if (cursor == null) {
                throw new IOException("The selected folder cannot be read");
            }
            while (cursor.moveToNext()) {
                String id = cursor.getString(0);
                String name = cursor.getString(1);
                String mime = cursor.getString(2);
                if (name == null || name.isEmpty() || name.contains("/") || name.equals(".")
                        || name.equals("..")) {
                    throw new IOException("The selected folder contains an invalid file name");
                }
                result.add(new DocumentEntry(id, name,
                        DocumentsContract.Document.MIME_TYPE_DIR.equals(mime)));
            }
        } catch (SecurityException error) {
            throw new IOException("Android denied access to the selected folder", error);
        }
        return result;
    }

    private static void copyDirectory(ContentResolver resolver, Uri tree, String documentId,
                                      File target) throws IOException {
        if (!target.isDirectory() && !target.mkdirs()) {
            throw new IOException("Unable to create " + target.getName());
        }
        for (DocumentEntry entry : children(resolver, tree, documentId)) {
            File output = new File(target, entry.name);
            if (entry.directory) {
                copyDirectory(resolver, tree, entry.id, output);
                continue;
            }
            Uri source = DocumentsContract.buildDocumentUriUsingTree(tree, entry.id);
            try (InputStream input = resolver.openInputStream(source);
                 FileOutputStream file = new FileOutputStream(output, false)) {
                if (input == null) {
                    throw new IOException("Unable to read " + entry.name);
                }
                byte[] buffer = new byte[1024 * 1024];
                int count;
                while ((count = input.read(buffer)) != -1) {
                    file.write(buffer, 0, count);
                }
            }
        }
    }

    private boolean hasRequiredAssets() {
        return new File(getFilesDir(), ".tmnf-assets-ready").isFile() && hasCoreAssets();
    }

    private boolean hasCoreAssets() {
        File root = getFilesDir();
        return new File(root, "Packs/packlist.dat").isFile()
                && new File(root, "Packs/Stadium.pak").isFile()
                && new File(root,
                "GameData/Tracks/Campaigns/Nations/Nations.ConstructionCampaign.Gbx").isFile();
    }

    private void launchGame() {
        startActivity(new Intent(this, TmnfActivity.class));
        finish();
    }

    private static final class DocumentEntry {
        final String id;
        final String name;
        final boolean directory;

        DocumentEntry(String id, String name, boolean directory) {
            this.id = id;
            this.name = name;
            this.directory = directory;
        }
    }
}
