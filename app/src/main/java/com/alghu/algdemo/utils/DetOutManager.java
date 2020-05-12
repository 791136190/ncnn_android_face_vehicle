package com.alghu.algdemo.utils;

public class DetOutManager {

    private float[] det_info = null;
    private long FpsTime = 1;

    public float[] getDetInfo() {
        return det_info;
    }
    public void setDetInfo(float[] detInfo) {
        this.det_info = detInfo;
    }

    public long getFpsTime() {
        return FpsTime;
    }
    public void setFpsTime(long fpsTime) {
        this.FpsTime = fpsTime;
    }

}
