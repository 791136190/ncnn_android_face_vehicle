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

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "head_det", __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO , "head_det", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN , "head_det", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "head_det", __VA_ARGS__))

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

#include "head_det.h"

static ncnn::Net g_head_det_net;

static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
static ncnn::PoolAllocator g_workspace_pool_allocator;

static __inline bool cmpScoreHead(box_head_ lsh, box_head_ rsh)
{
    if (lsh.score < rsh.score)
        return true;
    else
        return false;
}

static void nms_head(std::vector<box_head_>& boundingBox_, const float overlap_threshold)
{
    if (boundingBox_.empty())
    {
        return;
    }
    sort(boundingBox_.begin(), boundingBox_.end(), cmpScoreHead);
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
    std::vector<box_head_> tmp_;
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

static struct std::vector<box_head_> head_det(ncnn::Mat mInPut, const int nDetW, const int nDetH)
{
    static timeval start;
    static timeval end;

    double costtime;
    gettimeofday(&start, NULL);

    ncnn::Mat in;
    ncnn::resize_bilinear(mInPut, in, nDetW, nDetH);

    ncnn::Extractor ex = g_head_det_net.create_extractor();
    ex.input("data", in);

    std::vector<box_head_> boundingBox_;

    float score_thr = get_thr(head_confidence_threshold);

    for (int feature_index = 0; feature_index < head_fpn_num; feature_index++)
    {
        ncnn::Mat out;
        char name[64];
        memset(name, 0, sizeof(name));
        sprintf(name, "head_output%d_out%d_fwd", feature_index, feature_index);
        ex.extract(name, out);

        for (int pp = 0; pp < head_anchor_num; pp++)
        {
            int p = pp * head_channels_per_box;

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
                        const float bias_w = head_anchor[feature_index * head_anchor_num * 2 + pp * 2 + 0];
                        const float bias_h = head_anchor[feature_index * head_anchor_num * 2 + pp * 2 + 1];
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
                        for (int q = 0; q < head_num_class; q++)
                        {
                            ncnn::Mat scores = out.channel_range(p + 5, head_num_class);
                            float score = sigmoid(scores.channel(q).row(h)[w]);
                            if (score > class_score)
                            {
                                class_index = q;
                                class_score = score;
                            }
                        }

                        box_head_ bbox = { 0 };
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
    nms_head(boundingBox_, head_nms_threshold);

    gettimeofday(&end, NULL);
    long dis = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    costtime = (double)(dis) / 1000;

    total_time += costtime;
    total_det += 1;

    LOGD("detect head cost %f ms, all:%f ms, det:%lu\n", costtime, total_time/total_det, boundingBox_.size());

    return boundingBox_;
}

static struct std::vector<BboxHead> head_det_run(const unsigned char* pixels, int src_w, int src_h)
{
    std::vector<BboxHead> BoxAll;

//    ncnn::Mat InPutImg = ncnn::Mat::from_pixels_resize(pixels, ncnn::Mat::PIXEL_GRAY2RGB, src_w, src_h, src_w, src_h);//这里不缩放 原图上扣车牌识别的小图
    unsigned  char * pRgbBuf = (unsigned char*)malloc(src_w * src_h * 3);

//    alg_yuv4202rgbplane((unsigned char* )pixels, src_w, src_h, pRgbBuf);
    ncnn::yuv420sp2rgb(pixels, src_w, src_h, pRgbBuf);
    ncnn::Mat InPutImg = ncnn::Mat::from_pixels(pRgbBuf, ncnn::Mat::PIXEL_RGB, src_w, src_h);//这里不缩放 原图上扣车牌识别的小图

    free(pRgbBuf);

    std::vector<box_head_> head_box = head_det(InPutImg, head_det_w, head_det_h);

    for (std::vector<struct box_head_>::iterator it = head_box.begin(); it != head_box.end(); it++)
    {
        BboxHead cur_ceh;
        cur_ceh.head_box = *it;
        BoxAll.push_back(cur_ceh);
    }

    return BoxAll;
}

#ifdef __cplusplus  
extern "C" {  
#endif

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_alghu_algdemo_utils_AlgManager_RunHeadModeByYuv(JNIEnv *env, jclass thiz, jbyteArray frame, jint src_width,
                                jint src_height, jfloatArray detect) {
    char *yuv_frame = (char*)env->GetPrimitiveArrayCritical(frame, NULL);

    int size = env->GetArrayLength(frame);

    //shift argb to rgba
    char *yuv = (char *)malloc(size);
    memcpy(yuv, yuv_frame, size);

    env->ReleasePrimitiveArrayCritical(frame, yuv_frame, JNI_ABORT);

    std::vector<BboxHead> obj = head_det_run((const unsigned char *)yuv, src_width, src_height);

    free(yuv);

    float *detect_out = (float *)env->GetPrimitiveArrayCritical(detect, NULL);

    int index = 0;
    for (std::vector<struct BboxHead>::iterator it = obj.begin(); it != obj.end(); it++)
    {
        if(index >= MAX_HEAD_NUM)
        {
            break;
        }
        detect_out[index * MAX_HEAD_LEN + 0] = it->head_box.id;
        detect_out[index * MAX_HEAD_LEN + 1] = it->head_box.score;
        detect_out[index * MAX_HEAD_LEN + 2] = (float)it->head_box.x1 / head_det_w;
        detect_out[index * MAX_HEAD_LEN + 3] = (float)it->head_box.y1 / head_det_h;
        detect_out[index * MAX_HEAD_LEN + 4] = (float)it->head_box.x2 / head_det_w;
        detect_out[index * MAX_HEAD_LEN + 5] = (float)it->head_box.y2 / head_det_h;

        index++;
//        LOGI("head det %d = %d at %f %f %f %f\n", it->head_box.id , it->head_box.score, (float)it->head_box.x1,
//                (float)it->head_box.y1 , (float)it->plate_box.x1, (float)it->plate_box.y1);

    }

    env->ReleasePrimitiveArrayCritical(detect, detect_out, JNI_ABORT);

    return detect;
}

extern "C" JNIEXPORT int JNICALL
Java_com_alghu_algdemo_utils_AlgManager_InitHeadModelFromFile(JNIEnv *env, jclass /* this */,
        jstring HeadDetParamPath, jstring HeadDetBinPath,
        jint RunOnWhere, jint runWidth, jint runHeight)
{
    LOGI("will start init head all\n");

    const char *head_det_param_path = env->GetStringUTFChars(HeadDetParamPath, 0);
    const char *head_det_model_path = env->GetStringUTFChars(HeadDetBinPath, 0);

    const int run_on_where = RunOnWhere;

    head_det_w = (int)((float)runWidth / 32 + 0.5f) * 32;
    head_det_h = (int)((float)runHeight / 32 + 0.5f) * 32;

#if NCNN_VULKAN
    if(1 == run_on_where) {
        g_head_det_net.opt.use_vulkan_compute = true;
    }
#endif // NCNN_VULKAN

    g_blob_pool_allocator.clear();
    g_workspace_pool_allocator.clear();
    g_head_det_net.opt.blob_allocator = &g_blob_pool_allocator;
    g_head_det_net.opt.workspace_allocator = &g_workspace_pool_allocator;

    //no vulkan, size 0.0,0.0 1-105,2-78,3-69,4-67,5-98,8-175
    //no vulkan, size 1.0,0.0 4-80, size 0.5,0.0 4-85, size 0.5,0.5 4-80, size 0.0,0.5 4-85, size 0.0,1.0 4-80
    //vulkan, size 0.0,0.0 1-50,4-50

    g_head_det_net.opt.num_threads = 4;
    g_head_det_net.opt.use_packing_layout = true;
    g_head_det_net.load_param(head_det_param_path);
    g_head_det_net.load_model(head_det_model_path);

    env->ReleaseStringUTFChars(HeadDetParamPath, head_det_param_path);
    env->ReleaseStringUTFChars(HeadDetBinPath, head_det_model_path);

    return 0;
}

extern "C" JNIEXPORT int JNICALL
Java_com_alghu_algdemo_utils_AlgManager_UinitHeadModel(JNIEnv *env, jclass /* this */, jint RunOnWhere)
{
    LOGI("will start Uinit head all\n");

    const int run_on_where = RunOnWhere;

    g_head_det_net.clear();

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