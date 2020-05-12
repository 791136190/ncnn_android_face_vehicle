/*
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.alghu.algdemo.view;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.renderscript.ScriptIntrinsicYuvToRGB;
import android.renderscript.Type;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * A {@link TextureView} that can be adjusted to a specified aspect ratio.
 */
public class AutoFitSurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = "AutoFitSurfaceView";

    private SurfaceHolder mHolder;
    private Canvas mCanvas;
    private Paint paint;
    private Bitmap mBitmap;
    private RenderScript rs;
    private ScriptIntrinsicYuvToRGB yuvToRgbIntrinsic;
    private Type.Builder yuvType, rgbaType;
    private Allocation in, out;

    private float mWidth = 0;
    private float mHeight = 0;

    public AutoFitSurfaceView(Context context) {
        this(context, null);
    }

    public AutoFitSurfaceView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public AutoFitSurfaceView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        Log.i(TAG, "AutoFitSurfaceView struct");
        mHolder = getHolder();
        mHolder.addCallback(this);
        paint = new Paint();
        paint.setColor(Color.WHITE);
        setFocusable(true);
        rs = RenderScript.create(context);
        yuvToRgbIntrinsic = ScriptIntrinsicYuvToRGB.create(rs, Element.U8_4(rs));
    }

    public void setAspectRatio(int width, int height) {
        if (width < 0 || height < 0) {
            throw new IllegalArgumentException("Size cannot be negative.");
        }
        Log.i(TAG, "setAspectRatio " + width + "X" + height);
        requestLayout();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.i(TAG, "surfaceCreated ");
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.i(TAG, "surfaceChanged " + width + "X" + height);
        mWidth = width;
        mHeight = height;
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.i(TAG, "surfaceDestroyed ");
    }

    private  void DrawRect(Rect rect, int color, String string)
    {
        paint.setColor(color) ;
        paint.setStyle(Paint.Style.STROKE);
        paint.setTextSize(40);
        paint.setStrokeWidth(4);

        mCanvas.drawRect(rect, paint);
        paint.setStyle(Paint.Style.FILL);
        mCanvas.drawText(string, rect.left, rect.top - 5, paint);
    }

    private  void DrawAttr(float[] attr, Rect rect, int width, int height)
    {
        int text_size = 48;

        paint.setColor(Color.RED) ;
        paint.setStyle(Paint.Style.STROKE);
        paint.setTextSize(text_size);
        paint.setStrokeWidth(5f);
//        score = 22; land = 37; glass = 39; male = 41; smile = 43; hat = 45; age = 47;
        mCanvas.drawCircle(attr[1] * width, attr[2] * height, 5, paint);
        mCanvas.drawCircle(attr[3] * width, attr[4] * height, 5, paint);
        mCanvas.drawCircle(attr[5] * width, attr[6] * height, 5, paint);
        mCanvas.drawCircle(attr[7] * width, attr[8] * height, 5, paint);
        mCanvas.drawCircle(attr[9] * width, attr[10] * height, 5, paint);

        paint.setStyle(Paint.Style.FILL);
        int pos = 0;
        mCanvas.drawText("score:" + String.format("%.2f", attr[0]), rect.right + 4, rect.top + 24 + text_size * pos++, paint);
        mCanvas.drawText("glass:" + String.format("%.2f", attr[11]), rect.right + 4, rect.top + 24 + text_size * pos++, paint);
        mCanvas.drawText("male:" + String.format("%.2f", attr[12]), rect.right + 4, rect.top + 24 + text_size * pos++, paint);
        mCanvas.drawText("smile:" + String.format("%.2f", attr[13]), rect.right + 4, rect.top + 24 + text_size * pos++, paint);
        mCanvas.drawText("hat:" + String.format("%.2f", attr[14]), rect.right + 4, rect.top + 24 + text_size * pos++, paint);
        mCanvas.drawText("age:" + String.format("%.2f", attr[15]), rect.right + 4, rect.top + 24 + text_size * pos++, paint);
    }

    private  void DrawName(String name, float sim, Rect rect, int width, int height)
    {
        int text_size = 48;

        paint.setColor(Color.RED) ;
        paint.setStyle(Paint.Style.STROKE);
        paint.setTextSize(text_size);
        paint.setStrokeWidth(5f);

        paint.setStyle(Paint.Style.FILL);
        int pos = 1;
        mCanvas.drawText("name:" + name, rect.left+150, rect.top - text_size, paint);
        mCanvas.drawText("sim:" + String.format("%.2f", sim), rect.left+150, rect.top - 5, paint);
    }

    private  void DrawFps(long nFpsTime)
    {
        paint.setColor(Color.RED) ;
        paint.setStyle(Paint.Style.STROKE);
        paint.setTextSize(48f);
        paint.setStrokeWidth(5f);
        paint.setStyle(Paint.Style.FILL);

        mCanvas.drawText("fps:" + String.format("%.2f",1000.0f / nFpsTime), 20, 40, paint);
    }
    static private int class_color[] = { Color.GREEN , Color.YELLOW, Color.YELLOW, Color.YELLOW, Color.YELLOW, Color.RED, Color.GREEN};
    static private String class_names[] = {"face", "car", "bus", "truck", "motorcycle", "plate", "background" };
    static private String plate_names[] = {
            "京", "沪", "津", "渝", "冀", "晋", "蒙", "辽", "吉", "黑",
            "苏", "浙", "皖", "闽", "赣", "鲁", "豫", "鄂", "湘", "粤",
            "桂", "琼", "川", "贵", "云", "藏", "陕", "甘", "青", "宁", "新",
            "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
            "A", "B", "C", "D", "E", "F", "G", "H", "J", "K",
            "L", "M", "N", "P", "Q", "R", "S", "T", "U", "V",
            "W", "X", "Y", "Z",
            "警", "港", "澳", "挂", "学", "领", "使"}; // 71

    public void DrawDetectFace(float[] data, int width, int height, int max_num, int max_len) {
        for (int i = 0 ; i < max_num ; i++)
        {
            int prob = (int)(data[i * max_len + 1]);
            if (prob > 0)
            {
                int t = (int) data[i * max_len];
                int x =  Math.max(0, (int) (data[i * max_len + 2] * width));
                int y = Math.max(0, (int) (data[i * max_len + 3] * height));
                int xe = Math.min(width, (int) (data[i * max_len + 4] * width));
                int ye = Math.min(height, (int) (data[i * max_len + 5] * height));

                Log.d(TAG, "obj" + i +" " + t +" " + prob+ " pos" + x + " " + y + " " + xe + " " + ye);
                DrawRect(new Rect(x,y,xe,ye),class_color[t], class_names[t] + " " + prob);

                if(data[i * max_len + 6] > 0)
                {
                    float[] attr = new float[max_len - 6];
                    System.arraycopy(data,i * max_len + 6, attr, 0, max_len - 6);
                    DrawAttr(attr, new Rect(x,y,xe,ye), width, height);
                }
            }
        }
    }

    public void DrawFaceInfo(byte[] data, int width, int height, float[] result, long nFpsTime, int max_num, int max_len) {
        Log.d(TAG, "draw "+data.length +" " + width + "X" + height + nFpsTime);
        if (data != null) {

            mBitmap = yuvToBitmap(data,width,height);
            if (mBitmap == null) {
                Log.d(TAG, "data data is to bitmap error");
                return;
            }

            try {
                mCanvas = mHolder.lockCanvas();
                mWidth = mCanvas.getWidth();
                mHeight = mCanvas.getHeight();
                Log.d(TAG, "canvas size is " +  mWidth+ " " + mHeight);
                float scaleWidth = mWidth/width;
                float scaleHeight = mHeight/height;
                Log.d(TAG, "scale size is " +  scaleWidth+ " " +  scaleHeight);

                Matrix matrix = new Matrix();
                matrix.setScale(scaleWidth, scaleHeight);
                mCanvas.drawBitmap(mBitmap, matrix, paint);

                DrawDetectFace(result, (int)mWidth, (int)mHeight, max_num, max_len);
                DrawFps(nFpsTime);

            }catch (Exception e){
                Log.d(TAG, "e="+e);
                mHolder.unlockCanvasAndPost(mCanvas);
                return;
            }
            mHolder.unlockCanvasAndPost(mCanvas);
        } else {
            Log.d(TAG, "data data is null");
            return;
        }
    }

    public void DrawRecFace(float[] data, int width, int height, int max_num, int max_len, Map<String, float[]> FaceRecMap) {
        for (int i = 0 ; i < max_num ; i++)
        {
            int prob = (int)(data[i * max_len + 1]);
            if (prob > 0)
            {
                int t = (int) data[i * max_len];
                int x =  Math.max(0, (int) (data[i * max_len + 2] * width));
                int y = Math.max(0, (int) (data[i * max_len + 3] * height));
                int xe = Math.min(width, (int) (data[i * max_len + 4] * width));
                int ye = Math.min(height, (int) (data[i * max_len + 5] * height));

                Log.d(TAG, "obj" + i +" " + t +" " + prob+ " pos" + x + " " + y + " " + xe + " " + ye);
                DrawRect(new Rect(x,y,xe,ye),class_color[t], class_names[t] + " " + prob);

                if(data[i * max_len + 6] > 0)
                {
                    float[] attr = new float[max_len - 6];
                    System.arraycopy(data,i * max_len + 6, attr, 0, max_len - 6);
                    DrawAttr(attr, new Rect(x,y,xe,ye), width, height);

                    float[] rec = new float[max_len - 6 - 16];
                    System.arraycopy(data,i * max_len + 6 + 16, rec, 0, max_len - 6 - 16);

                    float max_sim = -1.0f;
                    String name = null;
                    for (Iterator it = FaceRecMap.keySet().iterator(); it.hasNext();)
                    {
                        String key = (String) it.next();
                        float[] value = FaceRecMap.get(key);
                        float sim = 0.0f;
                        for (int index = 0; index < rec.length; index++) {
                            sim += (value[index] * rec[index]);
//                            Log.d(TAG, "index:" + index + "," + value[index] + "," + rec[index]);
                            if (sim > max_sim)
                            {
                                max_sim = sim;
                                name = key;
                            }
                        }
//                        Log.d(TAG, "end one time");
                    }
                    if(name != null)
                    {
                        DrawName(name, max_sim, new Rect(x,y,xe,ye), width, height);
                    }
                }

            }
        }
    }

    public void DrawFaceRec(byte[] data, int width, int height, float[] result, long nFpsTime, int max_num, int max_len, Map<String, float[]> FaceRecMap) {
        Log.d(TAG, "draw "+data.length +" " + width + "X" + height + nFpsTime);
        if (data != null) {

            mBitmap = yuvToBitmap(data,width,height);
            if (mBitmap == null) {
                Log.d(TAG, "data data is to bitmap error");
                return;
            }

            try {
                mCanvas = mHolder.lockCanvas();
                mWidth = mCanvas.getWidth();
                mHeight = mCanvas.getHeight();
                Log.d(TAG, "canvas size is " +  mWidth+ " " + mHeight);
                float scaleWidth = mWidth/width;
                float scaleHeight = mHeight/height;
                Log.d(TAG, "scale size is " +  scaleWidth+ " " +  scaleHeight);

                Matrix matrix = new Matrix();
                matrix.setScale(scaleWidth, scaleHeight);
                mCanvas.drawBitmap(mBitmap, matrix, paint);

                DrawRecFace(result, (int)mWidth, (int)mHeight, max_num, max_len, FaceRecMap);

                DrawFps(nFpsTime);

            }catch (Exception e){
                Log.d(TAG, "e="+e);
                mHolder.unlockCanvasAndPost(mCanvas);
                return;
            }
            mHolder.unlockCanvasAndPost(mCanvas);
        } else {
            Log.d(TAG, "data data is null");
            return;
        }
    }

    public void DrawDetectVeh(float[] data, int width, int height, int max_num, int max_len) {
        for (int i = 0 ; i < max_num ; i++)
        {
            int prob = (int)(data[i * max_len + 1]);
            if (prob > 0)
            {
                int t = (int) data[i * max_len] + 1;
                int x =  Math.max(0, (int) (data[i * max_len + 2] * width));
                int y = Math.max(0, (int) (data[i * max_len + 3] * height));
                int xe = Math.min(width, (int) (data[i * max_len + 4] * width));
                int ye = Math.min(height, (int) (data[i * max_len + 5] * height));

                Log.d(TAG, "obj" + i +" " + t +" " + prob+ " pos" + x + " " + y + " " + xe + " " + ye);
                DrawRect(new Rect(x,y,xe,ye),class_color[t], class_names[t] + " " + prob);

                prob = (int)(data[i * max_len + 7]);
                if (prob > 0)
                {
                    t = (int) data[i * max_len + 6] + 5;
                    x =  Math.max(0, (int) (data[i * max_len + 8] * width));
                    y = Math.max(0, (int) (data[i * max_len + 9] * height));
                    xe = Math.min(width, (int) (data[i * max_len + 10] * width));
                    ye = Math.min(height, (int) (data[i * max_len + 11] * height));

                    Log.d(TAG, "obj" + i +" " + t +" " + prob+ " pos" + x + " " + y + " " + xe + " " + ye);
                    DrawRect(new Rect(x,y,xe,ye),class_color[t], class_names[t] + " " + prob);

                    for (int index = 0; index < 8; index++)
                    {
                        int idx = (int) (data[i * max_len + 12 + index]);
                        paint.setColor(Color.GREEN) ;
                        paint.setStyle(Paint.Style.STROKE);
                        paint.setTextSize(48f);
                        paint.setStrokeWidth(5f);
                        paint.setStyle(Paint.Style.FILL);
                        if(idx < plate_names.length)
                        {
                            mCanvas.drawText(plate_names[idx], x + 150 + index * 45, y - 10, paint);
                        }
                    }
                }

            }
        }
    }

    public void DrawVehicleInfo(byte[] data, int width, int height, float[] result, long nFpsTime, int max_num, int max_len) {
        Log.d(TAG, "draw "+data.length +" " + width + "X" + height + nFpsTime);
        if (data != null) {

            mBitmap = yuvToBitmap(data,width,height);
            if (mBitmap == null) {
                Log.d(TAG, "data data is to bitmap error");
                return;
            }

            try {
                mCanvas = mHolder.lockCanvas();
                mWidth = mCanvas.getWidth();
                mHeight = mCanvas.getHeight();
                Log.d(TAG, "canvas size is " +  mWidth+ " " + mHeight);
                float scaleWidth = mWidth/width;
                float scaleHeight = mHeight/height;
                Log.d(TAG, "scale size is " +  scaleWidth+ " " +  scaleHeight);

                Matrix matrix = new Matrix();
                matrix.setScale(scaleWidth, scaleHeight);
                mCanvas.drawBitmap(mBitmap, matrix, paint);

                DrawDetectVeh(result, (int)mWidth, (int)mHeight, max_num, max_len);
                DrawFps(nFpsTime);

            }catch (Exception e){
                Log.d(TAG, "e="+e);
                mHolder.unlockCanvasAndPost(mCanvas);
                return;
            }
            mHolder.unlockCanvasAndPost(mCanvas);
        } else {
            Log.d(TAG, "data data is null");
            return;
        }
    }

    public void DrawBitmap(Bitmap data, int width, int height) {
        if (data != null) {
            mBitmap = data;
            try {
                mCanvas = mHolder.lockCanvas();
                mWidth = mCanvas.getWidth();
                mHeight = mCanvas.getHeight();
                Log.d(TAG, "canvas size is " +  mWidth+ " " + mHeight);
                float scaleWidth = mWidth/width;
                float scaleHeight = mHeight/height;
                Log.d(TAG, "scale size is " +  scaleWidth+ " " +  scaleHeight);

                Matrix matrix = new Matrix();
                matrix.setScale(scaleWidth, scaleHeight);
                mCanvas.drawBitmap(mBitmap, matrix, paint);
            }catch (Exception e){
                Log.d(TAG, "e="+e);
                mHolder.unlockCanvasAndPost(mCanvas);
                return;
            }
            mHolder.unlockCanvasAndPost(mCanvas);
        } else {
            Log.d(TAG, "data data is null");
            return;
        }
    }

    public void DrawYuv(byte[] data, int width, int height) {
        Log.d(TAG, "draw "+data.length +" " + width + "X" + height);
        if (data != null) {

            mBitmap = yuvToBitmap(data, width, height);
            if (mBitmap == null) {
                Log.d(TAG, "data data is to bitmap error");
                return;
            }

            try {
                mCanvas = mHolder.lockCanvas();
                mWidth = mCanvas.getWidth();
                mHeight = mCanvas.getHeight();
                Log.d(TAG, "canvas size is " +  mWidth+ " " + mHeight);
                float scaleWidth = mWidth/width;
                float scaleHeight = mHeight/height;
                Log.d(TAG, "scale size is " +  scaleWidth+ " " +  scaleHeight);

                Matrix matrix = new Matrix();
                matrix.setScale(scaleWidth, scaleHeight);
                mCanvas.drawBitmap(mBitmap, matrix, paint);
            }catch (Exception e){
                Log.d(TAG, "e="+e);
                mHolder.unlockCanvasAndPost(mCanvas);
                return;
            }
            mHolder.unlockCanvasAndPost(mCanvas);
        } else {
            Log.d(TAG, "data data is null");
            return;
        }
    }

    public Bitmap yuvToBitmap(byte[] yuv, int width, int height){
        if (yuvType == null){
            yuvType = new Type.Builder(rs, Element.U8(rs)).setX(yuv.length);
            in = Allocation.createTyped(rs, yuvType.create(), Allocation.USAGE_SCRIPT);
            rgbaType = new Type.Builder(rs, Element.RGBA_8888(rs)).setX(width).setY(height);
            out = Allocation.createTyped(rs, rgbaType.create(), Allocation.USAGE_SCRIPT);
        }
        in.copyFrom(yuv);
        yuvToRgbIntrinsic.setInput(in);
        yuvToRgbIntrinsic.forEach(out);
        Bitmap rgbout = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        out.copyTo(rgbout);
        return rgbout;
    }
}
