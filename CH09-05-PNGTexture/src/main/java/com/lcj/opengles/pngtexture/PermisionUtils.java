package com.lcj.opengles.pngtexture;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;

import androidx.core.app.ActivityCompat;

import java.io.File;
import java.io.OutputStream;
import java.util.List;

/**
 * Created by luocj on 2/6/18.
 */

public class PermisionUtils {
    // Storage Permissions
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE};

    static private PermisionUtils mPermisionUtils;

    private PermisionUtils() {

    }

    /**
     * Checks if the app has permission to write to device storage
     * If the app does not has permission then the user will be prompted to
     * grant permissions
     *
     * @param activity
     */
    public static void verifyStoragePermissions(Activity activity) {
        // Check if we have write permission
        int permission = ActivityCompat.checkSelfPermission(activity,
                Manifest.permission.READ_EXTERNAL_STORAGE);

        if (permission != PackageManager.PERMISSION_GRANTED) {
            // We don't have permission so prompt the user
            ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE,
                    REQUEST_EXTERNAL_STORAGE);
        }
    }

    public static PermisionUtils instance() {
        if (mPermisionUtils == null) {
            mPermisionUtils = new PermisionUtils();
        }

        return mPermisionUtils;
    }

    public void requestSdCardUriPermission(Activity context, int requestCode) {
        Intent intent;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N
                && android.os.Build.VERSION.SDK_INT <= android.os.Build.VERSION_CODES.P) {
            intent = createAccessIntent(context);
            if (null != intent) {
                //安卓Q以下，一般走这条路，弹出的是小型的授权框
                context.startActivityForResult(intent, requestCode);
            } else {
                intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
                context.startActivityForResult(intent, requestCode);
            }
        } else {
            // 安卓Q必须走这里，因为上面的方法会导致授权不成功
            intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
            context.startActivityForResult(intent, requestCode);
        }
    }

    private Intent createAccessIntent(Context context) {
        File file = new File(getSDCardName(context));
        StorageManager storageManager =
                (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        StorageVolume volume;
        if (null != storageManager) {
            volume = storageManager.getStorageVolume(file);
            if (null != volume) {
                return volume.createAccessIntent(null);
            }
        }
        return null;
    }

    public String getSDCardName(Context context) {
        StorageManager storageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            List<StorageVolume> storageVolumeList = storageManager.getStorageVolumes();
            for (StorageVolume storageVolume : storageVolumeList) {
                if (storageVolume.isRemovable()) {
                    return storageVolume.getUuid();
                }
            }
        }
        return null;
    }


}