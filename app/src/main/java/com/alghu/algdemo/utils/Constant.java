package com.alghu.algdemo.utils;

import android.os.Environment;

public class Constant {

    public static final int AI_OK = 0;

    public static final int RUN_ON_CPU = 0;
    public static final int RUN_ON_GPU = 1;
    public static final int RUN_ON_NPU = 2;

    public static final int IMAGE_REQUEST_CODE = 0;
    public static final int CAMEAR_REQUEST_CODE = 1;
    public static final int CAPTURE_REQUEST_CODE = 2;

    public static final int ALG_RUN_FACE = 0;
    public static final int ALG_RUN_DLPR = 1;
    public static final int ALG_RUN_HEAD = 2;

    public static final int DATA_BY_IMAGE = 0;
    public static final int DATA_BY_CAMERA = 1;
    public static final int DATA_BY_CAPTURE = 2;

    public static final int MAX_PREVIEW_WIDTH = 640;
    public static final int MAX_PREVIEW_HEIGHT = 480;

    public static final int MAX_FACE_NUM = 30;
    public static final int MAX_FACE_LEN = 1+1+4+6+10+128; //id score x y w h,score glass male smile hat age land

    public static final int MAX_VEH_NUM = 30;
    public static final int MAX_VEH_LEN = 6+6+8; //class score  x y w h,score x y w h,char*8

    public static final int MAX_HEAD_NUM = 30;
    public static final int MAX_HEAD_LEN = 1+1+4; //id score x y w h

    public static final double meanValueOfBlue = 103.939;
    public static final double meanValueOfGreen = 116.779;
    public static final double meanValueOfRed = 123.68;

}
