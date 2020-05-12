// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2017 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include <stdio.h>
#include <time.h>
#include <vector>
#include <jni.h>

#include <android/log.h>

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "veh_det", __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO , "veh_det", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN , "veh_det", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "veh_det", __VA_ARGS__))

#include <map>
#include "platform.h"
#include "net.h"

#if NCNN_VULKAN
#include "gpu.h"
// 只在face中进行
//class GlobalGpuInstance
//{
//public:
//    GlobalGpuInstance() { ncnn::create_gpu_instance(); }
//    ~GlobalGpuInstance() { ncnn::destroy_gpu_instance(); }
//};
// initialize vulkan runtime before main()
//static GlobalGpuInstance g_global_gpu_instance;

#endif // NCNN_VULKAN

#include "veh_det.h"
//#include "veh.det.mem.h"
//#include "plate.det.mem.h"
//#include "plate.rec.mem.h"

static ncnn::Net g_veh_det_net;
static ncnn::Net g_plate_det_net;
static ncnn::Net g_plate_rec_net;

static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
static ncnn::PoolAllocator g_workspace_pool_allocator;

static __inline bool cmpScoreVeh(box_veh_ lsh, box_veh_ rsh)
{
    if (lsh.score < rsh.score)
        return true;
    else
        return false;
}

static void nms_veh(std::vector<box_veh_>& boundingBox_, const float overlap_threshold)
{
    if (boundingBox_.empty())
    {
        return;
    }
    sort(boundingBox_.begin(), boundingBox_.end(), cmpScoreVeh);
    float IOU = 0;
    int maxX = 0;
    int maxY = 0;
    int minX = 0;
    int minY = 0;
    std::vector<int> vPick;
    int nPick = 0;
    std::multimap<unsigned int, int> vScores;
    const int num_boxes = (int)boundingBox_.size();
    vPick.resize(num_boxes);
    for (int i = 0; i < num_boxes; ++i)
    {
        vScores.insert(std::pair<unsigned int, int>(boundingBox_[i].score, i));
    }
    while (vScores.size() > 0)
    {
        int last = vScores.rbegin()->second;
        vPick[nPick] = last;
        nPick += 1;
        for (std::multimap<unsigned int, int>::iterator it = vScores.begin(); it != vScores.end();)
        {
            int it_idx = it->second;
            maxX = MAX(boundingBox_.at(it_idx).x1, boundingBox_.at(last).x1);
            maxY = MAX(boundingBox_.at(it_idx).y1, boundingBox_.at(last).y1);
            minX = MIN(boundingBox_.at(it_idx).x2, boundingBox_.at(last).x2);
            minY = MIN(boundingBox_.at(it_idx).y2, boundingBox_.at(last).y2);
            //maxX1 and maxY1 reuse
            maxX = ((minX - maxX + 1) > 0) ? (minX - maxX + 1) : 0;
            maxY = ((minY - maxY + 1) > 0) ? (minY - maxY + 1) : 0;
            //IOU reuse for the area of two bbox
            IOU = (float)maxX * maxY;
            IOU = IOU / (boundingBox_.at(it_idx).area + boundingBox_.at(last).area - IOU); // 暂时只支持相加，不支持最小框做分母
            if (IOU > overlap_threshold)
            {
                it = vScores.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    vPick.resize(nPick);
    std::vector<box_veh_> tmp_;
    tmp_.resize(nPick);
    for (int i = 0; i < nPick; i++)
    {
        tmp_[i] = boundingBox_[vPick[i]];
    }
    boundingBox_ = tmp_;
}

static __inline bool cmpScorePlate(box_plate_ lsh, box_plate_ rsh)
{
    if (lsh.score < rsh.score)
        return true;
    else
        return false;
}

static void nms_plate(std::vector<box_plate_>& boundingBox_, const float overlap_threshold)
{
    if (boundingBox_.empty())
    {
        return;
    }
    sort(boundingBox_.begin(), boundingBox_.end(), cmpScorePlate);
    float IOU = 0;
    int maxX = 0;
    int maxY = 0;
    int minX = 0;
    int minY = 0;
    std::vector<int> vPick;
    int nPick = 0;
    std::multimap<unsigned int, int> vScores;
    const int num_boxes = (int)boundingBox_.size();
    vPick.resize(num_boxes);
    for (int i = 0; i < num_boxes; ++i)
    {
        vScores.insert(std::pair<unsigned int, int>(boundingBox_[i].score, i));
    }
    while (vScores.size() > 0)
    {
        int last = vScores.rbegin()->second;
        vPick[nPick] = last;
        nPick += 1;
        for (std::multimap<unsigned int, int>::iterator it = vScores.begin(); it != vScores.end();)
        {
            int it_idx = it->second;
            maxX = MAX(boundingBox_.at(it_idx).x1, boundingBox_.at(last).x1);
            maxY = MAX(boundingBox_.at(it_idx).y1, boundingBox_.at(last).y1);
            minX = MIN(boundingBox_.at(it_idx).x2, boundingBox_.at(last).x2);
            minY = MIN(boundingBox_.at(it_idx).y2, boundingBox_.at(last).y2);
            //maxX1 and maxY1 reuse
            maxX = ((minX - maxX + 1) > 0) ? (minX - maxX + 1) : 0;
            maxY = ((minY - maxY + 1) > 0) ? (minY - maxY + 1) : 0;
            //IOU reuse for the area of two bbox
            IOU = (float)maxX * maxY;
            IOU = IOU / (boundingBox_.at(it_idx).area + boundingBox_.at(last).area - IOU); // 暂时只支持相加，不支持最小框做分母
            if (IOU > overlap_threshold)
            {
                it = vScores.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    vPick.resize(nPick);
    std::vector<box_plate_> tmp_;
    tmp_.resize(nPick);
    for (int i = 0; i < nPick; i++)
    {
        tmp_[i] = boundingBox_[vPick[i]];
    }
    boundingBox_ = tmp_;
}

static inline float sigmoid(float x)
{
    return 1.f / (1.f + exp(-x));
}

static inline float get_thr(const float x)
{
    float y = -log(1.0f / x - 1);
    return y;
}

static double total_time = 0.0;
static int total_det = 0;

static struct std::vector<int> plate_rec_arg_max(const ncnn::Mat mInPut)
{
    std::vector<int> max_index;
    for (int i = 0; i < mInPut.h; ++i) {
        const float * pCurRow = mInPut.row(i);
        float TmpMax = 0;
        int TmpIndex = mInPut.w;
        for (int j = 0; j < mInPut.w; ++j) {
            if(*pCurRow > TmpMax)
            {
                TmpMax = *pCurRow;
                TmpIndex = j;
            }
            pCurRow++;
        }
        max_index.push_back(TmpIndex);
    }

    std::vector<int> max_index_final;
    for (size_t i = 0; i < max_index.size(); ++i) {
        if (max_index.at(i) == mInPut.w - 1) // 跳过空字符
        {
            continue;
        }
        if(i < max_index.size() - 1 && max_index.at(i) == max_index.at(i + 1)) // 跳过重复字符
        {
            continue;
        }
        max_index_final.push_back(max_index.at(i));
    }
    return max_index_final;
}
static struct plate_attr_ plate_rec(ncnn::Mat mInPut, const int nDetW, const int nDetH)
{
    static timeval start;
    static timeval end;

    double costtime;
    gettimeofday(&start, NULL);

    ncnn::Mat in;
    ncnn::resize_bilinear(mInPut, in, nDetW, nDetH);

    ncnn::Extractor ex = g_plate_rec_net.create_extractor();
    ex.input("data", in);


    ncnn::Mat out;
//    ex.extract(40, out); //chw, 1X20X73
    ex.extract("resnetv10_squeeze0", out); //chw, 1X20X73
    std::vector<int> max_index = plate_rec_arg_max(out);

    plate_attr_ plate_attr = {(float)out.w - 1};
    for (size_t i = 0; i < sizeof(plate_attr.Chars) / sizeof(plate_attr.Chars[0]); ++i) {
        plate_attr.Chars[i] = (float)out.w - 1;
        if(i < max_index.size())
        {
            plate_attr.Chars[i] = max_index.at(i);
        }
    }

    gettimeofday(&end, NULL);
    long dis = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    costtime = (double)(dis) / 1000;

    total_time += costtime;
    total_det += 1;

    LOGD("rec plate cost %f ms, all:%f msn", costtime, total_time/total_det);

    return plate_attr;
}

static struct std::vector<box_plate_> plate_det(ncnn::Mat mInPut, const int nDetW, const int nDetH)
{
    static timeval start;
    static timeval end;

    double costtime;
    gettimeofday(&start, NULL);

    ncnn::Mat in;
    ncnn::resize_bilinear(mInPut, in, nDetW, nDetH);

    ncnn::Extractor ex = g_plate_det_net.create_extractor();
    ex.input("data", in);

    std::vector<box_plate_> boundingBox_;

    float score_thr = get_thr(plate_confidence_threshold);

    for (int feature_index = 0; feature_index < plate_fpn_num; feature_index++)
    {
        ncnn::Mat out;
//        int blob_index[3] = {42, 69, 95};
//        ex.extract(blob_index[feature_index], out);
        char name[64];
        memset(name, 0, sizeof(name));
        sprintf(name, "plate_output%d_out%d_fwd", feature_index, feature_index);
        ex.extract(name, out);
//        LOGI("get out put %d %d %d\n", out.c, out.h, out.w);

        for (int pp = 0; pp < plate_anchor_num; pp++)
        {
            int p = pp * plate_channels_per_box;

            //printf("%f %f\n", bias_w, bias_h);
            const float* xptr = out.channel(p + 0);
            const float* yptr = out.channel(p + 1);
            const float* wptr = out.channel(p + 2);
            const float* hptr = out.channel(p + 3);

            const float* box_score_ptr = out.channel(p + 4 + plate_num_land);

            for (int h = 0; h < out.h; h++)
            {
                for (int w = 0; w < out.w; w++)
                {
                    // box score
                    //float box_score = sigmoid(box_score_ptr[0]);
                    float box_score = box_score_ptr[0];
                    if (box_score > score_thr)
                    {
                        // region box
                        const float bias_w = plate_anchor[feature_index * plate_anchor_num * 2 + pp * 2 + 0];
                        const float bias_h = plate_anchor[feature_index * plate_anchor_num * 2 + pp * 2 + 1];
                        //float bbox_cx = (w + sigmoid(xptr[0])) / out.w;
                        //float bbox_cy = (h + sigmoid(yptr[0])) / out.h;
                        float bbox_cx = (w + xptr[0]) / out.w;
                        float bbox_cy = (h + yptr[0]) / out.h;
                        float bbox_w = exp(wptr[0]) * bias_w / nDetW;
                        float bbox_h = exp(hptr[0]) * bias_h / nDetH;
                        //printf("w:%f,h:%f\n", bbox_w, bbox_h);

                        float bbox_xmin = MID(bbox_cx - bbox_w * 0.5f, 0.0f, 1.0f);
                        float bbox_ymin = MID(bbox_cy - bbox_h * 0.5f, 0.0f, 1.0f);
                        float bbox_xmax = MID(bbox_cx + bbox_w * 0.5f, 0.0f, 1.0f);
                        float bbox_ymax = MID(bbox_cy + bbox_h * 0.5f, 0.0f, 1.0f);


//                        ncnn::Mat land = out.channel_range(p + 4, plate_num_land);

                        box_plate_ bbox = { 0 };
                        bbox.id = 0;
                        bbox.score = (unsigned int)(sigmoid(box_score) * 100);

                        bbox.x1 = (int)round(bbox_xmin * nDetW);
                        bbox.y1 = (int)round(bbox_ymin * nDetH);
                        bbox.x2 = (int)round(bbox_xmax * nDetW);
                        bbox.y2 = (int)round(bbox_ymax * nDetH);
                        bbox.area = (float)(bbox.x2 - bbox.x1 + 1) * (bbox.y2 - bbox.y1 + 1);

//                        memcpy(bbox.land, land.data, sizeof(bbox.land));

                        if (bbox.x2 > bbox.x1 && bbox.y2 > bbox.y1)
                        {
                            boundingBox_.push_back(bbox);
                            //printf("w:%f,h:%f\n", (float)(bbox.x2 - bbox.x1), (float)(bbox.y2 - bbox.y1));
                        }
                    }

                    xptr++;
                    yptr++;
                    wptr++;
                    hptr++;

                    box_score_ptr++;
                }
            }
        }
    }

    // nms
    nms_plate(boundingBox_, plate_nms_threshold);

    gettimeofday(&end, NULL);
    long dis = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    costtime = (double)(dis) / 1000;

    total_time += costtime;
    total_det += 1;

    LOGD("plate det cost %f ms, all:%f ms, det:%lu\n", costtime, total_time/total_det, boundingBox_.size());

    return boundingBox_;
}

static struct std::vector<box_veh_> veh_det(ncnn::Mat mInPut, const int nDetW, const int nDetH)
{
    static timeval start;
    static timeval end;

    double costtime;
    gettimeofday(&start, NULL);

    ncnn::Mat in;
    ncnn::resize_bilinear(mInPut, in, nDetW, nDetH);

    ncnn::Extractor ex = g_veh_det_net.create_extractor();
    ex.input("data", in);

    std::vector<box_veh_> boundingBox_;

    float score_thr = get_thr(veh_confidence_threshold);

    for (int feature_index = 0; feature_index < veh_fpn_num; feature_index++)
    {
        ncnn::Mat out;
//        int blob_index[3] = {42, 69, 95};
//        ex.extract(blob_index[feature_index], out);
//        LOGI("get out put %d %d %d\n", out.c, out.h, out.w);
        char name[64];
        memset(name, 0, sizeof(name));
        sprintf(name, "veh_output%d_out%d_fwd", feature_index, feature_index);
        ex.extract(name, out);

        for (int pp = 0; pp < veh_anchor_num; pp++)
        {
            int p = pp * veh_channels_per_box;

            //printf("%f %f\n", bias_w, bias_h);
            const float* xptr = out.channel(p + 0);
            const float* yptr = out.channel(p + 1);
            const float* wptr = out.channel(p + 2);
            const float* hptr = out.channel(p + 3);

            const float* box_score_ptr = out.channel(p + 4);

            for (int h = 0; h < out.h; h++)
            {
                for (int w = 0; w < out.w; w++)
                {
                    // box score
                    //float box_score = sigmoid(box_score_ptr[0]);
                    float box_score = box_score_ptr[0];
                    if (box_score > score_thr)
                    {
                        // region box
                        const float bias_w = veh_anchor[feature_index * veh_anchor_num * 2 + pp * 2 + 0];
                        const float bias_h = veh_anchor[feature_index * veh_anchor_num * 2 + pp * 2 + 1];
                        //float bbox_cx = (w + sigmoid(xptr[0])) / out.w;
                        //float bbox_cy = (h + sigmoid(yptr[0])) / out.h;
                        float bbox_cx = (w + xptr[0]) / out.w;
                        float bbox_cy = (h + yptr[0]) / out.h;
                        float bbox_w = exp(wptr[0]) * bias_w / nDetW;
                        float bbox_h = exp(hptr[0]) * bias_h / nDetH;
                        //printf("w:%f,h:%f\n", bbox_w, bbox_h);

                        float bbox_xmin = MID(bbox_cx - bbox_w * 0.5f, 0.0f, 1.0f);
                        float bbox_ymin = MID(bbox_cy - bbox_h * 0.5f, 0.0f, 1.0f);
                        float bbox_xmax = MID(bbox_cx + bbox_w * 0.5f, 0.0f, 1.0f);
                        float bbox_ymax = MID(bbox_cy + bbox_h * 0.5f, 0.0f, 1.0f);

                        // find class index with max class score
                        // softmax class scores
                        //ncnn::Mat scores = out.channel_range(p + 5, num_class);

                        int class_index = 0;
                        float class_score = 0.f;
                        for (int q = 0; q < veh_num_class; q++)
                        {
                            ncnn::Mat scores = out.channel_range(p + 5, veh_num_class);
                            float score = sigmoid(scores.channel(q).row(h)[w]);
                            if (score > class_score)
                            {
                                class_index = q;
                                class_score = score;
                            }
                        }

                        box_veh_ bbox = { 0 };
                        bbox.id = (unsigned int)class_index;
                        bbox.score = (unsigned int)(sigmoid(box_score) * 100);

                        bbox.x1 = (int)round(bbox_xmin * nDetW);
                        bbox.y1 = (int)round(bbox_ymin * nDetH);
                        bbox.x2 = (int)round(bbox_xmax * nDetW);
                        bbox.y2 = (int)round(bbox_ymax * nDetH);
                        bbox.area = (float)(bbox.x2 - bbox.x1 + 1) * (bbox.y2 - bbox.y1 + 1);

                        if (bbox.x2 > bbox.x1 && bbox.y2 > bbox.y1)
                        {
                            boundingBox_.push_back(bbox);
                            //printf("w:%f,h:%f\n", (float)(bbox.x2 - bbox.x1), (float)(bbox.y2 - bbox.y1));
                        }
                    }

                    xptr++;
                    yptr++;
                    wptr++;
                    hptr++;

                    box_score_ptr++;
                }
            }
        }
    }

    // nms
    nms_veh(boundingBox_, veh_nms_threshold);

    gettimeofday(&end, NULL);
    long dis = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    costtime = (double)(dis) / 1000;

    total_time += costtime;
    total_det += 1;

    LOGD("detect veh cost %f ms, all:%f ms, det:%lu\n", costtime, total_time/total_det, boundingBox_.size());

    return boundingBox_;
}

static struct std::vector<BboxVeh> veh_det_run(const unsigned char* pixels, int src_w, int src_h)
{
    std::vector<BboxVeh> BoxAll;

    ncnn::Mat InPutImg = ncnn::Mat::from_pixels_resize(pixels, ncnn::Mat::PIXEL_GRAY2RGB, src_w, src_h, src_w, src_h);//这里不缩放 原图上扣车牌识别的小图

    std::vector<box_veh_> veh_box = veh_det(InPutImg, veh_det_w, veh_det_h);

    for (std::vector<struct box_veh_>::iterator it = veh_box.begin(); it != veh_box.end(); it++)
    {
        BboxVeh cur_ceh;
        cur_ceh.veh_box = *it;

        // 这里要注意，如果不是在车辆检测图上进行的，要考虑位置更新
        int veh_in_src_pos_x1 = (int)((float)it->x1 / veh_det_w * src_w);
        int veh_in_src_pos_x2 = (int)((float)it->x2 / veh_det_w * src_w);
        int veh_in_src_pos_y1 = (int)((float)it->y1 / veh_det_h * src_h);
        int veh_in_src_pos_y2 = (int)((float)it->y2 / veh_det_h * src_h);
        ncnn::Mat TmpVehIm;
        ncnn::copy_cut_border(InPutImg, TmpVehIm, veh_in_src_pos_y1, src_h - veh_in_src_pos_y2, veh_in_src_pos_x1, src_w - veh_in_src_pos_x2);
        std::vector<box_plate_> plate_box = plate_det(TmpVehIm, plate_det_w, plate_det_h);
        if (plate_box.size() > 0)
        {
            cur_ceh.plate_box = plate_box.at(0);

            int veh_in_src_w = veh_in_src_pos_x2 - veh_in_src_pos_x1;
            int veh_in_src_h = veh_in_src_pos_y2 - veh_in_src_pos_y1;
            cur_ceh.plate_box.x1 = (int)((float)cur_ceh.plate_box.x1 / plate_det_w * veh_in_src_w + veh_in_src_pos_x1);
            cur_ceh.plate_box.x2 = (int)((float)cur_ceh.plate_box.x2 / plate_det_w * veh_in_src_w + veh_in_src_pos_x1);
            cur_ceh.plate_box.y1 = (int)((float)cur_ceh.plate_box.y1 / plate_det_h * veh_in_src_h + veh_in_src_pos_y1);
            cur_ceh.plate_box.y2 = (int)((float)cur_ceh.plate_box.y2 / plate_det_h * veh_in_src_h + veh_in_src_pos_y1);
			
			int plate_in_src_w = cur_ceh.plate_box.x2 - cur_ceh.plate_box.x1;
            int plate_in_src_h = cur_ceh.plate_box.y2 - cur_ceh.plate_box.y1;

            int new_plate_x1 = MAX(0, cur_ceh.plate_box.x1 - plate_in_src_w / 6);
            int new_plate_x2 = MIN(src_w, cur_ceh.plate_box.x2 + plate_in_src_w / 6);
            int new_plate_y1 = MAX(0, cur_ceh.plate_box.y1 - plate_in_src_h / 3);
            int new_plate_y2 = MIN(src_w, cur_ceh.plate_box.y2 + plate_in_src_h / 3);
			
			cur_ceh.plate_box.x1 = new_plate_x1;
            cur_ceh.plate_box.x2 = new_plate_x2;
            cur_ceh.plate_box.y1 = new_plate_y1;
            cur_ceh.plate_box.y2 = new_plate_y2;

            ncnn::Mat TmpPlateIm;
            ncnn::copy_cut_border(InPutImg, TmpPlateIm, new_plate_y1, src_h - new_plate_y2, new_plate_x1, src_w - new_plate_x2);

            cur_ceh.plate_rec = plate_rec(TmpPlateIm, plate_rec_w, plate_rec_h);

        } else{
            memset(&cur_ceh.plate_box, 0, sizeof(cur_ceh.plate_box));
        }

        BoxAll.push_back(cur_ceh);
    }

    return BoxAll;
}

#ifdef __cplusplus  
extern "C" {  
#endif

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_alghu_algdemo_utils_AlgManager_RunVehModeByYuv(JNIEnv *env, jclass thiz, jbyteArray frame, jint src_width,
                                jint src_height, jfloatArray detect) {
    char *yuv_frame = (char*)env->GetPrimitiveArrayCritical(frame, NULL);

    int size = env->GetArrayLength(frame);

    //shift argb to rgba
    char *yuv = (char *)malloc(size);
    memcpy(yuv, yuv_frame, size);

    env->ReleasePrimitiveArrayCritical(frame, yuv_frame, JNI_ABORT);

    std::vector<BboxVeh> obj = veh_det_run((const unsigned char *)yuv, src_width, src_height);

    free(yuv);

    float *detect_out = (float *)env->GetPrimitiveArrayCritical(detect, NULL);

    int index = 0;
    for (std::vector<struct BboxVeh>::iterator it = obj.begin(); it != obj.end(); it++)
    {
        if(index >= MAX_VEH_NUM)
        {
            break;
        }
        detect_out[index * MAX_VEH_LEN + 0] = it->veh_box.id;
        detect_out[index * MAX_VEH_LEN + 1] = it->veh_box.score;
        detect_out[index * MAX_VEH_LEN + 2] = (float)it->veh_box.x1 / veh_det_w;
        detect_out[index * MAX_VEH_LEN + 3] = (float)it->veh_box.y1 / veh_det_h;
        detect_out[index * MAX_VEH_LEN + 4] = (float)it->veh_box.x2 / veh_det_w;
        detect_out[index * MAX_VEH_LEN + 5] = (float)it->veh_box.y2 / veh_det_h;
        
        detect_out[index * MAX_VEH_LEN + 6] = it->plate_box.id;
        detect_out[index * MAX_VEH_LEN + 7] = it->plate_box.score;
        detect_out[index * MAX_VEH_LEN + 8] = (float)it->plate_box.x1 / src_width;
        detect_out[index * MAX_VEH_LEN + 9] = (float)it->plate_box.y1 / src_height;
        detect_out[index * MAX_VEH_LEN + 10] = (float)it->plate_box.x2  / src_width;
        detect_out[index * MAX_VEH_LEN + 11] = (float)it->plate_box.y2  / src_height;

//        for (int i = 0; i < 5; ++i) {
//            it->attr.land[i * 2 + 0] = (it->attr.land[i * 2 + 0] * VEH_w + it->x1) / yolo_det_w;
//            it->attr.land[i * 2 + 1] = (it->attr.land[i * 2 + 1] * VEH_h + it->y1) / yolo_det_h;
//        }
//
        memcpy(detect_out + (index * MAX_VEH_LEN + 12), &it->plate_rec.Chars, sizeof(it->plate_rec.Chars));

        index++;
//        LOGI("veh det %d = %d at %f %f %f %f\n", it->veh_box.id , it->veh_box.score, (float)it->veh_box.x1,
//                (float)it->veh_box.y1 , (float)it->plate_box.x1, (float)it->plate_box.y1);

    }

    env->ReleasePrimitiveArrayCritical(detect, detect_out, JNI_ABORT);

    return detect;
}

extern "C" JNIEXPORT int JNICALL
Java_com_alghu_algdemo_utils_AlgManager_InitVehModelFromFile(JNIEnv *env, jclass /* this */,
        jstring VehDetParamPath, jstring VehDetBinPath, jstring PlateDetParamPath, jstring PlateDetBinPath, jstring PlateRecParamPath, jstring PlateRecBinPath,
        jint RunOnWhere, jint runWidth, jint runHeight)
{
    LOGI("will start init veh all\n");

    const char *veh_det_param_path = env->GetStringUTFChars(VehDetParamPath, 0);
    const char *veh_det_model_path = env->GetStringUTFChars(VehDetBinPath, 0);
    const char *plate_det_param_path = env->GetStringUTFChars(PlateDetParamPath, 0);
    const char *plate_det_model_path = env->GetStringUTFChars(PlateDetBinPath, 0);
    const char *plate_rec_param_path = env->GetStringUTFChars(PlateRecParamPath, 0);
    const char *plate_rec_model_path = env->GetStringUTFChars(PlateRecBinPath, 0);

    const int run_on_where = RunOnWhere;

    veh_det_w = (int)((float)runWidth / 32 + 0.5f) * 32;
    veh_det_h = (int)((float)runHeight / 32 + 0.5f) * 32;

#if NCNN_VULKAN
    if(1 == run_on_where) {
//        ncnn::create_gpu_instance();
        g_veh_det_net.opt.use_vulkan_compute = true;
        g_plate_det_net.opt.use_vulkan_compute = true;
        g_plate_rec_net.opt.use_vulkan_compute = true;
    }
#endif // NCNN_VULKAN

    g_blob_pool_allocator.clear();
    g_workspace_pool_allocator.clear();
    g_veh_det_net.opt.blob_allocator = &g_blob_pool_allocator;
    g_veh_det_net.opt.workspace_allocator = &g_workspace_pool_allocator;

    g_plate_det_net.opt.blob_allocator = &g_blob_pool_allocator;
    g_plate_det_net.opt.workspace_allocator = &g_workspace_pool_allocator;

    g_plate_rec_net.opt.blob_allocator = &g_blob_pool_allocator;
    g_plate_rec_net.opt.workspace_allocator = &g_workspace_pool_allocator;

    //no vulkan, size 0.0,0.0 1-105,2-78,3-69,4-67,5-98,8-175
    //no vulkan, size 1.0,0.0 4-80, size 0.5,0.0 4-85, size 0.5,0.5 4-80, size 0.0,0.5 4-85, size 0.0,1.0 4-80
    //vulkan, size 0.0,0.0 1-50,4-50

    g_veh_det_net.opt.num_threads = 4;
    g_veh_det_net.opt.use_packing_layout = true;
    g_veh_det_net.load_param(veh_det_param_path);
    g_veh_det_net.load_model(veh_det_model_path);

    g_plate_det_net.opt.num_threads = 4;
    g_plate_det_net.opt.use_packing_layout = true;
    g_plate_det_net.load_param(plate_det_param_path);
    g_plate_det_net.load_model(plate_det_model_path);

    g_plate_rec_net.opt.num_threads = 4;
    g_plate_rec_net.opt.use_packing_layout = true;
    g_plate_rec_net.load_param(plate_rec_param_path);
    g_plate_rec_net.load_model(plate_rec_model_path);

    env->ReleaseStringUTFChars(VehDetParamPath, veh_det_param_path);
    env->ReleaseStringUTFChars(VehDetBinPath, veh_det_model_path);
    env->ReleaseStringUTFChars(PlateDetParamPath, plate_det_param_path);
    env->ReleaseStringUTFChars(PlateDetBinPath, plate_det_model_path);
    env->ReleaseStringUTFChars(PlateRecParamPath, plate_rec_param_path);
    env->ReleaseStringUTFChars(PlateRecBinPath, plate_rec_model_path);

    return 0;
}

extern "C" JNIEXPORT int JNICALL
Java_com_alghu_algdemo_utils_AlgManager_UinitVehModel(JNIEnv *env, jclass /* this */, jint RunOnWhere)
{
    LOGI("will start Uinit veh all\n");

    const int run_on_where = RunOnWhere;

    g_veh_det_net.clear();

    g_plate_det_net.clear();

    g_plate_rec_net.clear();

#if NCNN_VULKAN
    if(1 == run_on_where)
    {
//        ncnn::destroy_gpu_instance();
    }
#endif // NCNN_VULKAN

    return 0;
}

#ifdef __cplusplus
}
#endif