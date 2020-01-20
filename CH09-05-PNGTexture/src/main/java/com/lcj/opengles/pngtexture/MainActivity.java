package com.lcj.opengles.pngtexture;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.documentfile.provider.DocumentFile;

import java.io.File;

public class MainActivity extends Activity {
    Button button;
    ImageView imageView;

    private Context mContext;

    Handler mHandler = new Handler() {
        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what) {
                case 1:
                    updateView();
                    break;
                case 2:
                    SomeArgs args = (SomeArgs) msg.obj;
                    final float red = (float)args.arg1;
                    final float green = (float)args.arg2;
                    final float blue = (float)args.arg3;
                    break;
                    default:
                        break;
            }
        }
    };

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mContext = getApplicationContext();

        setContentView(R.layout.activity_image_thumbnail);

        PermisionUtils.verifyStoragePermissions(this);

        button = findViewById(R.id.button);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setAction("android.intent.action.MAIN_LUO");
                startActivity(intent);
            }
        });

        imageView = findViewById(R.id.imageview);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mHandler.sendEmptyMessage(1);


    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        // real write file to sdcard
        if (requestCode == 2 && resultCode == Activity.RESULT_OK) {
            mHandler.sendEmptyMessage(1);
        }
    }

    private void updateView() {
        imageView.setImageBitmap(BitmapFactory.decodeFile("/sdcard/png/png_4_2_32bit.png"));
    }
}
