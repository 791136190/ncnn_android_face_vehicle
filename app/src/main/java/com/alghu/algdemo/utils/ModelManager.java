package com.alghu.algdemo.utils;

import java.io.Serializable;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class ModelManager implements Serializable{

    private String modelSaveDir = "";

    private String onlineModelLabel = "";

    private String offlineModelName = "";

    public int getInput_N() {
        return input_N;
    }

    public void setInput_N(int input_N) {
        this.input_N = input_N;
    }

    public int getInput_C() {
        return input_C;
    }

    public void setInput_C(int input_C) {
        this.input_C = input_C;
    }

    public int getInput_H() {
        return input_H;
    }

    public void setInput_H(int input_H) {
        this.input_H = input_H;
    }

    public int getInput_W() {
        return input_W;
    }

    public void setInput_W(int input_W) {
        this.input_W = input_W;
    }

    public int getInput_Number() {
        return input_Number;
    }

    public void setInput_Number(int input_Number) {
        this.input_Number = input_Number;
    }

    public int getOutput_N() {
        return output_N;
    }

    public void setOutput_N(int output_N) {
        this.output_N = output_N;
    }

    public int getOutput_C() {
        return output_C;
    }

    public void setOutput_C(int output_C) {
        this.output_C = output_C;
    }

    public int getOutput_H() {
        return output_H;
    }

    public void setOutput_H(int output_H) {
        this.output_H = output_H;
    }

    public int getOutput_W() {
        return output_W;
    }

    public void setOutput_W(int output_W) {
        this.output_W = output_W;
    }

    public int getOutput_Number() {
        return output_Number;
    }

    public void setOutput_Number(int output_Number) {
        this.output_Number = output_Number;
    }

    public int getRun_On_Where() {
        return run_on_where;
    }

    public void setRun_On_Where(int run_on_where) {
        this.run_on_where = run_on_where;
    }

    private int input_N;

    private int input_C;

    private int input_H;

    private int input_W;

    private int input_Number;

    private int output_N;

    private int output_C;

    private int output_H;

    private int output_W;

    private int output_Number;

    private int run_on_where;

    public boolean isMixModel() {
        return isMixModel;
    }

    public void setMixModel(boolean mixModel) {
        isMixModel = mixModel;
    }

    private  boolean isMixModel = false;

    /**
     * caffe : xxx.prototxt
     * tensorflow : xxx.pb
     * default is "" if don't have online model
     */
    private String onlineModel = "";
    /**
     * caffe: xxx.caffemodel
     * tensorflow: xxx.txt
     * default is "" if don't have online model
     */
    private String onlineModelPara = "";
    /**
     * xxx.cambricon
     * default is "" if don't have offline model
     */
    private String offlineModel = "";
    /**
     * "" or "100.100.001.010" or "100.150.010.010" ...
     *  default is "100.100.001.010" if don't know offline model version
     */
    private String offlineModelVersion = "100.100.001.010";
    /**
     * caffe or tensorflow
     */
    private String framework = "";

    public String getModelSaveDir() {
        return modelSaveDir;
    }

    public void setModelSaveDir(String modelSaveDir) {
        this.modelSaveDir = modelSaveDir;
    }

    public String getOnlineModelLabel() {
        return onlineModelLabel;
    }

    public void setOnlineModelLabel(String onlineModelLabel) {
        this.onlineModelLabel = onlineModelLabel;
    }

    public String getOfflineModelName() {
        return offlineModelName;
    }

    public void setOfflineModelName(String offlineModelName) {
        this.offlineModelName = offlineModelName;
    }

    public String getOnlineModel() {
        return onlineModel;
    }

    public void setOnlineModel(String onlineModel) {
        this.onlineModel = onlineModel;
    }

    public String getOnlineModelPara() {
        return onlineModelPara;
    }

    public void setOnlineModelPara(String onlineModelPara) {
        this.onlineModelPara = onlineModelPara;
    }

    public String getOfflineModel() {
        return offlineModel;
    }

    public void setOfflineModel(String offlineModel) {
        this.offlineModel = offlineModel;
    }

    public String getOfflineModelVersion() {
        return offlineModelVersion;
    }

    public void setOfflineModelVersion(String offlineModelVersion) {
        this.offlineModelVersion = offlineModelVersion;
    }

    public String getFramework() {
        return framework;
    }

    public void setFramework(String framework) {
        this.framework = framework;
    }

    // ####################################################
    private String FaceDetModelData = "";
    private String FaceDetModelParam = "";

    public String getFaceDetModelData() {
        return FaceDetModelData;
    }
    public void setFaceDetModelData(String FaceDetmodeldata) {
        this.FaceDetModelData = FaceDetmodeldata;
    }

    public String getFaceDetModelParam() {
        return FaceDetModelParam;
    }
    public void setFaceDetModelParam(String FaceDetmodelparam) {
        this.FaceDetModelParam = FaceDetmodelparam;
    }

    private String FaceAttrModelData = "";
    private String FaceAttrModelParam = "";

    public String getFaceAttrModelData() {
        return FaceAttrModelData;
    }
    public void setFaceAttrModelData(String FaceAttrmodeldata) {
        this.FaceAttrModelData = FaceAttrmodeldata;
    }

    public String getFaceAttrModelParam() {
        return FaceAttrModelParam;
    }
    public void setFaceAttrModelParam(String FaceAttrmodelparam) {
        this.FaceAttrModelParam = FaceAttrmodelparam;
    }

    private String FaceRecModelData = "";
    private String FaceRecModelParam = "";

    public String getFaceRecModelData() {
        return FaceRecModelData;
    }
    public void setFaceRecModelData(String FaceRecmodeldata) {
        this.FaceRecModelData = FaceRecmodeldata;
    }

    public String getFaceRecModelParam() {
        return FaceRecModelParam;
    }
    public void setFaceRecModelParam(String FaceRecmodelparam) {
        this.FaceRecModelParam = FaceRecmodelparam;
    }

    // ####################################################
    private String VehDetModelData = "";
    private String VehDetModelParam = "";

    public String getVehDetModelData() {
        return VehDetModelData;
    }
    public void setVehDetModelData(String VehDetmodeldata) {
        this.VehDetModelData = VehDetmodeldata;
    }

    public String getVehDetModelParam() {
        return VehDetModelParam;
    }
    public void setVehDetModelParam(String VehDetmodelparam) {
        this.VehDetModelParam = VehDetmodelparam;
    }

    private String PlateDetModelData = "";
    private String PlateDetModelParam = "";

    public String getPlateDetModelData() {
        return PlateDetModelData;
    }
    public void setPlateDetModelData(String PlateDetmodeldata) {
        this.PlateDetModelData = PlateDetmodeldata;
    }

    public String getPlateDetModelParam() {
        return PlateDetModelParam;
    }
    public void setPlateDetModelParam(String PlateDetmodelparam) {
        this.PlateDetModelParam = PlateDetmodelparam;
    }

    private String PlateRecModelData = "";
    private String PlateRecModelParam = "";

    public String getPlateRecModelData() {
        return PlateRecModelData;
    }
    public void setPlateRecModelData(String PlateRecmodeldata) {
        this.PlateRecModelData = PlateRecmodeldata;
    }

    public String getPlateRecModelParam() {
        return PlateRecModelParam;
    }
    public void setPlateRecModelParam(String PlateRecmodelparam) {
        this.PlateRecModelParam = PlateRecmodelparam;
    }

    // ####################################################
    private String HeadDetModelData = "";
    private String HeadDetModelParam = "";

    public String getHeadDetModelData() {
        return HeadDetModelData;
    }
    public void setHeadDetModelData(String HeadDetmodeldata) {
        this.HeadDetModelData = HeadDetmodeldata;
    }

    public String getHeadDetModelParam() {
        return HeadDetModelParam;
    }
    public void setHeadDetModelParam(String HeadDetmodelparam) {
        this.HeadDetModelParam = HeadDetmodelparam;
    }
    // ####################################################
}
