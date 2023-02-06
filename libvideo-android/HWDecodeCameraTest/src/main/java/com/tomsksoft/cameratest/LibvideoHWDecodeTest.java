package com.tomsksoft.cameratest;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.ImageProxy;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.media.Image;
import android.os.Bundle;
import android.util.Log;
import android.util.Size;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;

import com.tomsksoft.libvideo.DecoderParamsJ;
import com.tomsksoft.libvideo.EncoderParamsJ;
import com.tomsksoft.libvideo.HWDecoderJ;
import com.tomsksoft.libvideo.SWEncoderJ;
import com.tomsksoft.libvideo.Utilities;
import com.google.common.util.concurrent.ListenableFuture;

import java.nio.ByteBuffer;
import java.util.concurrent.ExecutionException;

public class LibvideoHWDecodeTest extends AppCompatActivity {

    private SurfaceHolder currentHolder;

    class DecodingSMPTE extends Thread {
        public void run() {
            for (int i = 0; i < 30; i++)
            {
                if (currentHolder != null)
                {
                    if (decoderJ == null)
                        decoderJ = new HWDecoderJ(decParamsJ, currentHolder.getSurface());

                    byte[] encoded_data = encoderJ.encodeFrame(smpteBGRA);
                    decoderJ.decode(encoded_data);
                }
            }
        }
    }

    class DecodingCamera extends Thread {
        public void run() {
                if (currentHolder != null)
                {
                    initializeCamera();
                }
        }
    }

    SurfaceHolder.Callback surfaceCallback = new SurfaceHolder.Callback() {

        @Override
        public void surfaceCreated(@NonNull SurfaceHolder surfaceHolder) {
            Log.d("Surface", "Created");
            currentHolder = surfaceHolder;

            if (decoding_smpte != null)
                decoding_smpte.start();

            if (decoding_camera != null)
            {
                if (ContextCompat.checkSelfPermission(LibvideoHWDecodeTest.this, Manifest.permission.CAMERA)
                        != PackageManager.PERMISSION_GRANTED) {
                    ActivityCompat.requestPermissions(LibvideoHWDecodeTest.this, new String[] {Manifest.permission.CAMERA},
                            PERMISSION_REQUEST_CAMERA);
                } else {
                    decoding_camera.start();
                }
            }
        }

        @Override
        public void surfaceChanged(@NonNull SurfaceHolder surfaceHolder, int i, int i1, int i2) {
            Log.d("Surface", "Changed");
            currentHolder = surfaceHolder;

            if (decoding_smpte != null && !decoding_smpte.isAlive())
                decoding_smpte.start();

            if (decoding_camera != null && !decoding_camera.isAlive())
            {
                if (decoding_camera != null)
                {
                    if (ContextCompat.checkSelfPermission(LibvideoHWDecodeTest.this, Manifest.permission.CAMERA)
                            != PackageManager.PERMISSION_GRANTED) {
                        ActivityCompat.requestPermissions(LibvideoHWDecodeTest.this, new String[] {Manifest.permission.CAMERA},
                                PERMISSION_REQUEST_CAMERA);
                    } else {
                        decoding_camera.start();
                    }
                }
            }
        }

        @Override
        public void surfaceDestroyed(@NonNull SurfaceHolder surfaceHolder) {
            Log.d("Surface", "Destroyed");
            currentHolder = surfaceHolder;
        }
    };

    EncoderParamsJ encParamsJ;
    DecoderParamsJ decParamsJ;


    SWEncoderJ encoderJ;
    HWDecoderJ decoderJ;
    byte[] smpteRGBA;
    byte[] smpteBGRA;

    DecodingSMPTE decoding_smpte;
    DecodingCamera decoding_camera;

    private static final int PERMISSION_REQUEST_CAMERA = 123;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

            System.loadLibrary("libvideo_android");

            SurfaceView surface = findViewById(R.id.surface);

            encParamsJ = new EncoderParamsJ();
            encParamsJ.width = 640;
            encParamsJ.height = 480;

            decParamsJ = new DecoderParamsJ();
            decParamsJ.width = encParamsJ.width;
            decParamsJ.height = encParamsJ.height;

            smpteRGBA = Utilities.makeRawSMPTEImage(encParamsJ.width, encParamsJ.height);
            smpteBGRA = new byte[encParamsJ.width * encParamsJ.height * 4];
            for (int i = 0; i < smpteBGRA.length / 4; i++) {
                smpteBGRA[i * 4]     = smpteRGBA[i * 4 + 2];
                smpteBGRA[i * 4 + 1] = smpteRGBA[i * 4 + 1];
                smpteBGRA[i * 4 + 2] = smpteRGBA[i * 4];
                smpteBGRA[i * 4 + 3] = smpteRGBA[i * 4 + 3];
            }

            encoderJ = new SWEncoderJ(encParamsJ);

            // For choosing decoding type comment one of the lines
//            decoding_smpte = new DecodingSMPTE();
            decoding_camera = new DecodingCamera();

            surface.getHolder().addCallback(surfaceCallback);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSION_REQUEST_CAMERA && grantResults.length > 0
                && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            initializeCamera();
        }
    }

    private ByteBuffer img_buf;
    private int size;
    private int[] pixels;
    private byte[] rgb_data;
    private Bitmap initial_bitmap;
    ListenableFuture<ProcessCameraProvider> cameraProviderFuture;

    private void initializeCamera() {
        cameraProviderFuture = ProcessCameraProvider.getInstance(this);
        cameraProviderFuture.addListener(new Runnable() {
            @Override
            public void run() {
                try {
                    ProcessCameraProvider cameraProvider = cameraProviderFuture.get();

                    ImageAnalysis imageAnalysis = new ImageAnalysis.Builder()
                            .setTargetResolution(new Size(640, 480))
                            .setOutputImageFormat(ImageAnalysis.OUTPUT_IMAGE_FORMAT_RGBA_8888)
                            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
                            .build();

                    CameraSelector cameraSelector = new CameraSelector.Builder()
                            .requireLensFacing(CameraSelector.LENS_FACING_BACK)
                            .build();


                    imageAnalysis.setAnalyzer(ContextCompat.getMainExecutor(LibvideoHWDecodeTest.this),
                            new ImageAnalysis.Analyzer() {
                                @Override
                                public void analyze(@NonNull ImageProxy image) {
                                    @SuppressLint("UnsafeOptInUsageError") Image img = image.getImage();

                                    if (img != null)
                                    {
                                        // Encoded/decoded frame to screen
                                        if (decoderJ == null)
                                        {
                                            size = img.getWidth() * img.getHeight();
                                            pixels = new int[size];

                                            encParamsJ = new EncoderParamsJ();
                                            encParamsJ.width = img.getWidth();
                                            encParamsJ.height = img.getHeight();

                                            decParamsJ = new DecoderParamsJ();
                                            decParamsJ.width = encParamsJ.width;
                                            decParamsJ.height = encParamsJ.height;

                                            encoderJ = new SWEncoderJ(encParamsJ);
                                            decoderJ = new HWDecoderJ(decParamsJ, currentHolder.getSurface());

                                            rgb_data = new byte[img.getWidth() * img.getHeight() * 4];
                                        }

                                        img_buf = img.getPlanes()[0].getBuffer();
                                        initial_bitmap = Bitmap.createBitmap(img.getWidth(), img.getHeight(), Bitmap.Config.ARGB_8888);
                                        initial_bitmap.copyPixelsFromBuffer(img_buf);

                                        initial_bitmap.getPixels(pixels, 0, img.getWidth(), 0, 0,
                                                img.getWidth(), img.getHeight());

                                        for (int i = 0; i < size; i++) {
                                            int color = pixels[i];
                                            rgb_data[i * 4]     = (byte)(color & 0xff); // b
                                            rgb_data[i * 4 + 1] = (byte)(color >> 8 & 0xff); // g
                                            rgb_data[i * 4 + 2] = (byte)(color >> 16 & 0xff); // r
                                            rgb_data[i * 4 + 3] = (byte)(color >> 24 & 0xff); // a
                                        }

                                        byte[] encoded_data = encoderJ.encodeFrame(rgb_data);
                                        decoderJ.decode(encoded_data);
                                    }
                                    image.close();
                                }
                            });

                    cameraProvider.bindToLifecycle(LibvideoHWDecodeTest.this, cameraSelector, imageAnalysis);

                } catch (ExecutionException | InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }, ContextCompat.getMainExecutor(this));
    }
}