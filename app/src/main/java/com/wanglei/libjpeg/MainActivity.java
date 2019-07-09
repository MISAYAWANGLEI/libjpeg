package com.wanglei.libjpeg;

import android.Manifest;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import kr.co.namee.permissiongen.PermissionFail;
import kr.co.namee.permissiongen.PermissionGen;
import kr.co.namee.permissiongen.PermissionSuccess;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("native-lib");
    }

    public static final int QUALITY = 50;
    private Bitmap inputBitmap=null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        File input = new File(Environment.getExternalStorageDirectory(), "test.jpeg");
        inputBitmap = BitmapFactory.decodeFile(input.getAbsolutePath());
        //
        PermissionGen.with(MainActivity.this)
                .addRequestCode(100)
                .permissions(
                        Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE)
                .request();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions,
                                           int[] grantResults) {
        PermissionGen.onRequestPermissionsResult(this, requestCode,
                permissions, grantResults);
    }

    @PermissionSuccess(requestCode = 100)
    public void doSomething(){
        Toast.makeText(this, "PermissionSuccess", Toast.LENGTH_SHORT).show();
    }

    @PermissionFail(requestCode = 100)
    public void doFailSomething(){
        Toast.makeText(this, "PermissionFail", Toast.LENGTH_SHORT).show();
    }

    public void topng(View view) {
        qualityCompress(inputBitmap, Bitmap.CompressFormat.PNG,QUALITY,
                Environment.getExternalStorageDirectory()+"/test_png.png");
        Toast.makeText(this,"success",Toast.LENGTH_SHORT).show();
    }

    public void tojpg(View view) {
        qualityCompress(inputBitmap, Bitmap.CompressFormat.JPEG,QUALITY,
                Environment.getExternalStorageDirectory()+"/test_jpg.jpeg");
        Toast.makeText(this,"success",Toast.LENGTH_SHORT).show();
    }

    public void towebp(View view) {
        qualityCompress(inputBitmap, Bitmap.CompressFormat.WEBP,QUALITY,
                Environment.getExternalStorageDirectory()+"/test_webp.webp");
        Toast.makeText(this,"success",Toast.LENGTH_SHORT).show();
    }

    public void libjpegCompress(View view) {
        nativeLibjpegCompress(inputBitmap, QUALITY,
                Environment.getExternalStorageDirectory() + "/test_libjpeg.jpg");
        Toast.makeText(this,"success",Toast.LENGTH_SHORT).show();
    }

    /**
     * 对图片进行质量压缩
     * @param bitmap 待压缩图片
     * @param format 压缩的格式
     * @param q      质量
     * @param path   文件地址
     */
    private void qualityCompress(Bitmap bitmap, Bitmap.CompressFormat format,
                          int q, String path) {
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(path);
            bitmap.compress(format, q, fos);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } finally {
            if (null != fos) {
                try {
                    fos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

    }

    public native void nativeLibjpegCompress(Bitmap bitmap, int quality, String path);

}
