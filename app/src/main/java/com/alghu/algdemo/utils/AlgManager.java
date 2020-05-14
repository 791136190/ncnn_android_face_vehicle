package com.alghu.algdemo.utils;

import android.content.res.AssetManager;

public class AlgManager {

    private static final String TAG = AlgManager.class.getSimpleName();

    private AlgManager() {
    }

    /* DDK model manager sync interfaces */

    /**
     *
     * @param modelname modelname is a flag that used to loaded
     * @param modelfilename modelfilename is real file which exist in assert directory
     * @param mgr  assertmanager instance
     * @param isMixModel true : when the onlinemodelpath is offline mix model file.
     *                      false : when the modelfilename is caffe or tensorflow offline model file.
     *
     * @return
     */
    public static native int loadModelSync(String modelname,String modelfilename, AssetManager mgr,boolean isMixModel);

    public static native int loadModelSyncFromBuffer(String offlineModelName, byte[] offlineModelBuffer,boolean isMixModel);

    public static native int loadModelFromFileSync(String modelName,String modelpath,boolean isMixModel);

    public static native int unloadModelSync();

    public static native float[][] runModelSync(ModelManager modelInfo, float[][] buf);

    public static native void runModelAsync(ModelManager modelInfo, float[][] buf);

    /* DDK model manager async interfaces */
//    public static native int registerListenerJNI(ModelManagerListener listener);
//
//    public static native void loadModelAsync(String modelName,String modelfilename,  AssetManager mgr,boolean isMixModel);
//
//    public static native void loadModelFromFileAsync(String modelName,String modelpath,boolean isMixModel);
//
//    public static native void loadModelAsyncFromBuffer(String offlineModelName, byte[] offlineModelBuffer,boolean isMixModel);
//
//    public static native void unloadModelAsync();
//
//    public static native String getHiAiVersion();

    /**
     *
     * @param onlinemodelpath   caffe:/xxx/xxx/xxx/xx.prototxt
     *                              tensorflow:/xxx/xxx/xxx/xx.pb
     *                              mix-model: xxx.zip when the caffe/tensorflow model exists some OPs which HiAI are not support.
     *                                         xxx.zip file can be generated by the model split tool.
     * @param onlinemodeparapath caffe:/xxx/xxx/xxx/xx.caffemodel
     *                               tensorflow:/xxx/xxx/xxx/xx.txt
     *                               mix-model: ""
     * @param framework  caffe or tensorflow
     * @param offlinemodelpath   /xxx/xxx/xxx/xx.offlinemodel
     * @param isMixModel true : when the onlinemodelpath is zip file.
     *                      false : when the onlinemodelpaht is caffe or tensorflow model file.
     * @return ture : it can run on NPU
     *          false: it should run on CPU
     */
    public static native boolean modelCompatibilityProcessFromFile(String onlinemodelpath,String onlinemodeparapath,String framework, String offlinemodelpath,boolean isMixModel);

    /**
     *
     * @param onlinemodelbuffer caffe: xx.prototxt buffer
     *                              tensorflow: xx.pb buffer
     *                              mix-model: xxx.zip buffer
     *                                         when the caffe/tensorflow model exists some OPs which HiAI are not support and the model can be split successfuly by the split tool.
     *                                         xxx.zip file can be generated by the model split tool.
     * @param modelparabuffer caffe: xx.caffemodel buffer
     *                           tensorflow: xx.txt buffer
     *                           mix-model: non-null buffer
     * @param framework  caffe or tensorflow
     * @param offlinemodelpath   /xxx/xxx/xxx/xx.offlinemodel
     * @param isMixModel true : when the onlinemodelbuffer is zip file buffer.
     *                      false : when the onlinemodelpaht is caffe or tensorflow model file buffer.
     * @return ture : it can run on NPU
     *          false: it should run on CPU
     */
    public static native boolean modelCompatibilityProcessFromBuffer(byte[] onlinemodelbuffer,byte[] modelparabuffer,String framework,String offlinemodelpath, boolean isMixModel);

    //######################################
    public static native int InitFaceModelFromFile(String DetParamPath, String DetBinPath, String AttrParamPath, String AttrBinPath, String RecParamPath, String RecBinPath, int runOnWhere, int runWidth, int runHeight);
    public static native int UinitFaceModel(int runOnWhere);
    public static native float[] RunFaceModeByYuv(byte[] YuvData, int SrcWidth, int SrcHeight, float[] OutputBuffer);

    //######################################
    public static native int InitVehModelFromFile(String VehDetParamPath, String VehDetBinPath, String PlateDetParamPath, String PlateDetBinPath, String PlateRecParamPath, String PlateRecBinPath, int runOnWhere, int runWidth, int runHeight);
    public static native int UinitVehModel(int runOnWhere);
    public static native float[] RunVehModeByYuv(byte[] YuvData, int SrcWidth, int SrcHeight, float[] OutputBuffer);

    //######################################
    public static native int InitHeadModelFromFile(String HeadDetParamPath, String HeadDetBinPath, int runOnWhere, int runWidth, int runHeight);
    public static native int UinitHeadModel(int runOnWhere);
    public static native float[] RunHeadModeByYuv(byte[] YuvData, int SrcWidth, int SrcHeight, float[] OutputBuffer);

}
