package com.alghu.algdemo.utils;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.util.Log;
import android.util.Size;


import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import static android.graphics.Color.blue;
import static android.graphics.Color.green;
import static android.graphics.Color.red;
//import static com.huawei.hiaidemo.utils.Constant.meanValueOfBlue;
//import static com.huawei.hiaidemo.utils.Constant.meanValueOfGreen;
//import static com.huawei.hiaidemo.utils.Constant.meanValueOfRed;

import com.alghu.algdemo.utils.Constant;

public class Untils {

    private static final String TAG = Untils.class.getSimpleName();
    private static BufferedInputStream bis = null;
    private static InputStream fileInput = null;
    private static FileOutputStream fileOutput = null;
    private static ByteArrayOutputStream byteOut = null;

    public static boolean loadJNISo() {
        try {

            System.loadLibrary("native-lib");
            return true;
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "failed to load native library: " + e.getMessage());
            return false;
        }

    }
    public static class CompareSizesByArea implements Comparator<Size> {

        @Override
        public int compare(Size lhs, Size rhs) {
            // We cast here to ensure the multiplications won't overflow
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() -
                    (long) rhs.getWidth() * rhs.getHeight());
        }

    }

    public static Size chooseOptimalSize(Size[] choices, int textureViewWidth,
                                          int textureViewHeight, int maxWidth, int maxHeight, Size aspectRatio) {

        // Collect the supported resolutions that are at least as big as the preview Surface
        List<Size> bigEnough = new ArrayList<>();
        // Collect the supported resolutions that are smaller than the preview Surface
        List<Size> notBigEnough = new ArrayList<>();
        int w = aspectRatio.getWidth();
        int h = aspectRatio.getHeight();
        for (Size option : choices) {
            if (option.getWidth() <= maxWidth && option.getHeight() <= maxHeight &&
                    option.getHeight() == option.getWidth() * h / w) {
                if (option.getWidth() >= textureViewWidth &&
                        option.getHeight() >= textureViewHeight) {
                    bigEnough.add(option);
                } else {
                    notBigEnough.add(option);
                }
            }
        }

        // Pick the smallest of those big enough. If there is no one big enough, pick the
        // largest of those not big enough.
        if (bigEnough.size() > 0) {
            return Collections.min(bigEnough, new CompareSizesByArea());
        } else if (notBigEnough.size() > 0) {
            return Collections.max(notBigEnough, new CompareSizesByArea());
        } else {
            Log.e(TAG, "Couldn't find any suitable preview size");
            return choices[0];
        }
    }

    // argb 8888 转 i420
    private static void conver_argb_to_i420(byte[] i420, int[] argb, int width, int height) {
        final int frameSize = width * height;

        int yIndex = 0;                   // Y start index
        int uIndex = frameSize;           // U start index
//        int vIndex = frameSize + frameSize / 4;   // V start index: w*h*5/4

        int a, R, G, B, Y, U, V;
        int index = 0;
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
//                a = (argb[index] & 0xff000000) >> 24; //  is not used obviously
                R = (argb[index] & 0xff0000) >> 16;
                G = (argb[index] & 0xff00) >> 8;
                B = (argb[index] & 0xff) >> 0;

                // well known RGB to YUV algorithm
                Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
                U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
                V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

                // I420(YUV420p) -> YYYYYYYY UU VV
                i420[yIndex++] = (byte) ((Y < 0) ? 0 : ((Y > 255) ? 255 : Y));
                if (j % 2 == 0 && i % 2 == 0) {
//                    i420[uIndex++] = (byte)((U<0) ? 0 : ((U > 255) ? 255 : U));
                    i420[uIndex++] = (byte)((V<0) ? 0 : ((V > 255) ? 255 : V)); // 这里不知为啥要vu顺序才是对的
                    i420[uIndex++] = (byte)((U<0) ? 0 : ((U > 255) ? 255 : U));
                }
                index ++;
            }
        }
    }


    public static byte[] bitmap2Yuv420(Bitmap bitmap)
    {
        int[] argb = new int[bitmap.getWidth() * bitmap.getHeight()];
        // Bitmap 获取 argb
        bitmap.getPixels(argb, 0, bitmap.getWidth(), 0, 0, bitmap.getWidth(), bitmap.getHeight());
        byte[] yuv = new byte[bitmap.getWidth() * bitmap.getHeight() * 3 / 2];
        // argb 8888 转 i420
        conver_argb_to_i420(yuv, argb, bitmap.getWidth(), bitmap.getHeight());
        return yuv;
    }

    public static byte[] getModelBufferFromModelFile(String modelPath){
        try{
            bis = new BufferedInputStream(new FileInputStream(modelPath));
            byteOut = new ByteArrayOutputStream(1024);
            byte[] buffer = new byte[1024];
            int size = 0;
            while((size = bis.read(buffer,0,1024)) != -1){
                byteOut.write(buffer,0,size);
            }
            return byteOut.toByteArray();

        }catch (Exception e){
            return  new byte[0];
        }finally {
            releaseResource(byteOut);
            releaseResource(bis);
        }
    }

    private static void releaseResource(Closeable resource){

        if(resource != null){
            try {
                resource.close();
                resource = null;
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static float[] NHWCtoNCHW(float[] orinal,int N, int C,int H,int W){
        if(orinal == null || orinal.length == 0 || N*H*W*C == 0 || N < 0 || C < 0 || H < 0 || W < 0){
            return orinal;
        }
        float[] nchw = new float[orinal.length];
        for(int i = 0; i < N;i++){
            for(int j = 0; j < C;j++){
                for(int k = 0; k < H*W;k++){
                    nchw[i*C*H*W+j*H*W+k] = orinal[i*H*W*C+ k*C + j];
                }
            }
        }
        return nchw;
    }

    public static float[] NCHWtoNHWC(float[] orinal,int N, int C,int H,int W){
        if(orinal == null || orinal.length == 0 || N*H*W*C == 0 || N < 0 || C < 0 || H < 0 || W < 0){
            return orinal;
        }
        float[] nhwc = new float[orinal.length];

        for(int i = 0 ;i < N;i++){
            for(int j = 0; j < C;j++){
                for(int k = 0; k < H*W;k++){
                    nhwc[i*C*H*W+ k*C + j] = orinal[i*C*H*W+j*H*W+k];
                }
            }
        }
        return nhwc;
    }

    public static float[] getPixels(String framework,Bitmap bitmap, int resizedWidth, int resizedHeight){
        if(framework == null){
            return  null;
        }
        else if(framework.equals("caffe")){
            return getPixelForSqueezeNet(bitmap,resizedWidth, resizedHeight);
        }
        else if(framework.equals("caffe_8bit")){
            return getPixelFor8bitSqueezeNet(bitmap,resizedWidth, resizedHeight);
        }
        else if(framework.equals("tensorflow")){
            return getPixelForInceptionV3(bitmap,resizedWidth, resizedHeight);
        }
        else if(framework.equals("tensorflow_8bit")){
            return getPixelFor8bitInceptionV3(bitmap,resizedWidth, resizedHeight);
        }
        return  null;
    }

    private static float[] getPixelForSqueezeNet(Bitmap bitmap, int resizedWidth, int resizedHeight) {
        int batch = 1;
        int channel = 3;
        float[] buff = new float[channel * resizedWidth * resizedHeight];

        int k = 0;
        for (int i = 0; i < resizedHeight; i++) {
            for (int j = 0; j < resizedWidth; j++) {

                int color = bitmap.getPixel(j, i);

                //NHWC
                buff[k] = (float) (blue(color) - Constant.meanValueOfBlue);
                k++;
                buff[k] = (float) (green(color) - Constant.meanValueOfGreen);
                k++;
                buff[k] = (float) (red(color) - Constant.meanValueOfRed);
                k++;
            }
        }

        return NHWCtoNCHW(buff,batch,channel,resizedHeight,resizedWidth);
    }

    private static float[] getPixelFor8bitSqueezeNet(Bitmap bitmap, int resizedWidth, int resizedHeight) {
        int batch = 1;
        int channel = 3;
        float[] buff = new float[channel * resizedWidth * resizedHeight];

        int k = 0;
        for (int i = 0; i < resizedHeight; i++) {
            for (int j = 0; j < resizedWidth; j++) {
                int color = bitmap.getPixel(j, i);

                //NHWC
                buff[k] = (float) (blue(color));
                k++;
                buff[k] = (float) (green(color));
                k++;
                buff[k] = (float) (red(color));
                k++;

            }
        }

        return NHWCtoNCHW(buff,batch,channel,resizedHeight,resizedWidth);
    }

    private static float[] getPixelForInceptionV3(Bitmap bitmap, int resizedWidth, int resizedHeight) {
        int batch = 1;
        int channel = 3;
        float[] buff = new float[channel * resizedWidth * resizedHeight];

        int k = 0;
        for (int i = 0; i < resizedHeight; i++) {
            for (int j = 0; j < resizedWidth; j++) {

                int color = bitmap.getPixel(j, i);

                //NHWC
                buff[k] = (float) ((red(color) - Constant.meanValueOfRed))/255;
                k++;
                buff[k] = (float) ((green(color) - Constant.meanValueOfGreen))/255;
                k++;
                buff[k] = (float) ((blue(color) - Constant.meanValueOfBlue))/255;
                k++;
            }
        }

        return NHWCtoNCHW(buff,batch,channel,resizedHeight,resizedWidth);
    }

    private static float[] getPixelFor8bitInceptionV3(Bitmap bitmap, int resizedWidth, int resizedHeight) {
        int batch = 1;
        int channel = 3;
        float[] buff = new float[channel * resizedWidth * resizedHeight];

        int k = 0;
        for (int i = 0; i < resizedHeight; i++) {
            for (int j = 0; j < resizedWidth; j++) {
                int color = bitmap.getPixel(j, i);

                //NHWC
                buff[k] = (float) (red(color));
                k++;
                buff[k] = (float) (green(color));
                k++;
                buff[k] = (float) (blue(color));
                k++;

            }
        }

        return NHWCtoNCHW(buff,batch,channel,resizedHeight,resizedWidth);
    }

    public static boolean copyModelsFromAssetToAppModelsByBuffer(AssetManager am,String sourceModelName,String destDir){

        try {
            fileInput = am.open(sourceModelName);
            String filename = destDir + sourceModelName;

            fileOutput = new FileOutputStream(filename);
            BufferedOutputStream bufferedOutputStream = new BufferedOutputStream(fileOutput);
            byteOut = new ByteArrayOutputStream();
            byte[] buffer = new byte[1024];
            int len = -1;
            while ((len = fileInput.read(buffer)) != -1) {
                bufferedOutputStream.write(buffer, 0, len);
            }
            bufferedOutputStream.close();
            return true;
        } catch (Exception ex) {
            Log.e(TAG, "copyModelsFromAssetToAppModels : " + ex);
            return false;
        }finally {
            releaseResource(byteOut);
            releaseResource(fileOutput);
            releaseResource(fileInput);
        }
    }

    public static boolean copyModelsFromAssetToAppModels(AssetManager am,String sourceModelName,String destDir){

        try {
            fileInput = am.open(sourceModelName);
            String filename = destDir + sourceModelName;

            fileOutput = new FileOutputStream(filename);
            byteOut = new ByteArrayOutputStream();
            byte[] buffer = new byte[1024];
            int len = -1;
            while ((len = fileInput.read(buffer)) != -1) {
                byteOut.write(buffer, 0, len);
            }
            fileOutput.write(byteOut.toByteArray());
            return true;
        } catch (Exception ex) {
            Log.e(TAG, "copyModelsFromAssetToAppModels : " + ex);
            return false;
        }finally {
            releaseResource(byteOut);
            releaseResource(fileOutput);
            releaseResource(fileInput);
        }
    }

    public static boolean isExistModelsInAppModels(String modelname,String savedir){

        File dir = new File(savedir);
        File[] currentfiles = dir.listFiles();
        if(currentfiles == null){
            return false;
        }else{
            for(File file: currentfiles){
                if(file.getName().equals(modelname)){
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * 将asset文件写入缓存
     */
    public static boolean copyAssetAndWrite(Context mContext, String fileName) {
        try {
            File cacheDir = mContext.getCacheDir();
            if (!cacheDir.exists()) {
                cacheDir.mkdirs();
            }
            File outFile = new File(cacheDir, fileName);
            if (!outFile.exists()) {
                boolean res = outFile.createNewFile();
                if (!res) {
                    return false;
                }
            } else {
                if (outFile.length() > 10) {//表示已经写入一次
                    return true;
                }
            }
            InputStream is = mContext.getAssets().open(fileName);
            FileOutputStream fos = new FileOutputStream(outFile);
            byte[] buffer = new byte[1024];
            int byteCount;
            while ((byteCount = is.read(buffer)) != -1) {
                fos.write(buffer, 0, byteCount);
            }
            fos.flush();
            is.close();
            fos.close();
            return true;
        } catch (IOException e) {
            e.printStackTrace();
        }

        return false;
    }


}
