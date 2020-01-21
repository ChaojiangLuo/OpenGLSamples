package com.lcj.opengles.pngtexture;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;

public class StartingActivity extends Activity {

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_staring);

        if (ActivityCompat.checkSelfPermission(this,
                Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            PermisionUtils.verifyStoragePermissions(this);
        } else {
            startNativeActivity();
        }
    }

    private void startNativeActivity() {
        Intent intent = new Intent();
        intent.setAction("android.intent.action.MAIN_NATIVE");
        startActivity(intent);
        stopStartingActivity();
    }

    private void stopStartingActivity() {
        finish();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        switch (requestCode) {
            case PermisionUtils.REQUEST_EXTERNAL_STORAGE:
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    startNativeActivity();
                } else {
                    Toast.makeText(this, "You have no permission to read sdcard", Toast.LENGTH_SHORT).show();
                    stopStartingActivity();
                }
                break;
            default:
                super.onRequestPermissionsResult(requestCode, permissions, grantResults);
                break;
        }
    }
}
