package com.tomsksoft.cameratest;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.os.Bundle;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.media.Image;
import android.util.Size;
import android.widget.ImageView;

import androidx.annotation.NonNull;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.ImageProxy;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.tomsksoft.libvideo.DecoderParamsJ;
import com.tomsksoft.libvideo.EncoderParamsJ;
import com.tomsksoft.libvideo.SWDecoderJ;
import com.tomsksoft.libvideo.SWEncoderJ;
import com.google.common.util.concurrent.ListenableFuture;

import java.nio.ByteBuffer;
import java.util.concurrent.ExecutionException;

public class LibvideoSWTest extends AppCompatActivity {

    private static final int PERMISSION_REQUEST_CAMERA = 123;

    private ImageView initial_preview;
    private ImageView decoded_preview;

    ListenableFuture<ProcessCameraProvider> cameraProviderFuture;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initial_preview = findViewById(R.id.preview);
        decoded_preview = findViewById(R.id.preview2);

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[] {Manifest.permission.CAMERA},
                    PERMISSION_REQUEST_CAMERA);
        } else {
            initializeCamera();
        }
        System.loadLibrary("libvideo_android");
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSION_REQUEST_CAMERA && grantResults.length > 0
                && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            initializeCamera();
        }
    }

    private Bitmap initial_bitmap;
    private Bitmap decoded_bitmap;

    private EncoderParamsJ encParamsJ;
    private DecoderParamsJ decParamsJ;
    private SWEncoderJ encoderJ;
    private SWDecoderJ decoderJ;

    ByteBuffer img_buf;

    int size;
    int[] pixels;

    byte[] rgb_data;
    int[] decoded_pixels;

    byte[] encoded_data;
    byte[] decoded_data;

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

                    imageAnalysis.setAnalyzer(ContextCompat.getMainExecutor(LibvideoSWTest.this),
                            new ImageAnalysis.Analyzer() {
                                @Override
                                public void analyze(@NonNull ImageProxy image) {
                                    @SuppressLint("UnsafeOptInUsageError") Image img = image.getImage();

                                    if (img != null)
                                    {
                                        // Initial frame to screen
                                        img_buf = img.getPlanes()[0].getBuffer();
                                        initial_bitmap = Bitmap.createBitmap(img.getWidth(), img.getHeight(), Bitmap.Config.ARGB_8888);
                                        initial_bitmap.copyPixelsFromBuffer(img_buf);
                                        initial_preview.setRotation(image.getImageInfo().getRotationDegrees());
                                        initial_preview.setImageBitmap(initial_bitmap);

                                        // Encoded/decoded frame to screen
                                        if (encParamsJ == null)
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
                                            decoderJ = new SWDecoderJ(decParamsJ);

                                            rgb_data = new byte[img.getWidth() * img.getHeight() * 4];
                                            decoded_pixels = new int[size];
                                        }

                                        initial_bitmap.getPixels(pixels, 0, img.getWidth(), 0, 0,
                                                img.getWidth(), img.getHeight());

                                        for (int i = 0; i < size; i++) {
                                            int color = pixels[i];
                                            rgb_data[i * 4]     = (byte)(color >> 16 & 0xff); // r
                                            rgb_data[i * 4 + 1] = (byte)(color >> 8 & 0xff); // g
                                            rgb_data[i * 4 + 2] = (byte)(color & 0xff); // b
                                            rgb_data[i * 4 + 3] = (byte)(color >> 24 & 0xff); // a
                                        }

                                        encoded_data = encoderJ.encodeFrame(rgb_data);
                                        decoded_data = decoderJ.decode(encoded_data);

                                        for (int i = 0; i < size; i++) {
                                            decoded_pixels[i] =
                                                    (decoded_data[i * 4] & 0xff) << 16     |
                                                    (decoded_data[i * 4 + 1] & 0xff) << 8  |
                                                    (decoded_data[i * 4 + 2] & 0xff)       |
                                                    (decoded_data[i * 4 + 3] & 0xff) << 24;
                                        }

                                        decoded_bitmap = Bitmap.createBitmap(img.getWidth(), img.getHeight(), Bitmap.Config.ARGB_8888);
                                        decoded_bitmap.setPixels(decoded_pixels, 0, img.getWidth(), 0, 0,
                                                img.getWidth(), img.getHeight());

                                        decoded_preview.setRotation(image.getImageInfo().getRotationDegrees());
                                        decoded_preview.setImageBitmap(decoded_bitmap);
                                    }
                                    image.close();
                                }
                            });

                    cameraProvider.bindToLifecycle(LibvideoSWTest.this, cameraSelector, imageAnalysis);

                } catch (ExecutionException | InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }, ContextCompat.getMainExecutor(this));

    }
}