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

//#include <string>
//#include <sys/stat.h> //_mkdir函数的头文件

#include <stdio.h>
#include <time.h>
#include <vector>
#include <jni.h>

#include <android/log.h>

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "face_det", __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO , "face_det", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN , "face_det", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "face_det", __VA_ARGS__))

#include <map>
#include <huAlgImgProc.h>
#include "platform.h"
#include "net.h"

#if NCNN_VULKAN
#include "gpu.h"
class GlobalGpuInstance
{
public:
    GlobalGpuInstance() { ncnn::create_gpu_instance(); }
    ~GlobalGpuInstance() { ncnn::destroy_gpu_instance(); }
};
static GlobalGpuInstance g_global_gpu_instance;

#endif // NCNN_VULKAN

#include "huAlgImgProc.h"
#include "face_det.h"
//#include "face.det.mem.h"
//#include "face.attr.mem.h"

static ncnn::Net g_face_det_net;
static ncnn::Net g_face_attr_net;
static ncnn::Net g_face_rec_net;
static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
static ncnn::PoolAllocator g_workspace_pool_allocator;

static __inline bool cmpScore(BboxFace lsh, BboxFace rsh)
{
    if (lsh.score < rsh.score)
        return true;
    else
        return false;
}

static void face_nms(std::vector<BboxFace>& boundingBox_, const float overlap_threshold)
{
    if (boundingBox_.empty())
    {
        return;
    }
    sort(boundingBox_.begin(), boundingBox_.end(), cmpScore);
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
    std::vector<BboxFace> tmp_;
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

static struct std::vector<BboxFace> face_det_run(const unsigned char* pixels, int src_w, int src_h)
{
    static timeval start;
    static timeval end;

    double costtime;
    gettimeofday(&start, NULL);

//    LOGD("detect img %d X %d -> %d X %d, \n", w, h, face_det_w, face_det_h);

    unsigned  char * pRgbBuf = (unsigned char*)malloc(src_w * src_h * 3);

//    alg_yuv4202rgbplane((unsigned char* )pixels, src_w, src_h, pRgbBuf);
    ncnn::yuv420sp2rgb(pixels, src_w, src_h, pRgbBuf);
    ncnn::Mat in = ncnn::Mat::from_pixels_resize(pRgbBuf, ncnn::Mat::PIXEL_RGB, src_w, src_h, face_det_w, face_det_h);

    free(pRgbBuf);

//    const float mean_vals[3] = {127.5f, 127.5f, 127.5f};
//    const float norm_vals[3] = {1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5};
//    in.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Extractor g_ex = g_face_det_net.create_extractor();

    g_ex.input("data", in);
//    ncnn::Mat out;
//    g_ex.extract(38, out); // BLOB_face_output2_out2_fwd = 83; BLOB_face_output1_out1_fwd = 61; int BLOB_face_output0_out0_fwd = 38;
//    LOGI("get out put %d %d %d\n", out.c, out.h, out.w);

    std::vector<BboxFace> boundingBox_;
    //FinalFaceOut.clear();

    float score_thr = get_thr(face_confidence_threshold);

    for (int feature_index = 0; feature_index < face_fpn_num; feature_index++)
    {
        ncnn::Mat out;

//        int blob_index[3] = {38, 61, 83};
//        g_ex.extract(blob_index[feature_index], out); // BLOB_face_output2_out2_fwd = 83; BLOB_face_output1_out1_fwd = 61; int BLOB_face_output0_out0_fwd = 38;
        char name[64];
        memset(name, 0, sizeof(name));
        sprintf(name, "face_output%d_out%d_fwd", feature_index, feature_index);
        //sprintf(name, "mobilenet_output%d_out%d_fwd", feature_index, feature_index);
        g_ex.extract(name, out);
//        LOGI("get out put %d %d %d\n", out.c, out.h, out.w);

        for (int pp = 0; pp < face_anchor_num; pp++)
        {
            int p = pp * face_channels_per_box;

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
                        const float bias_w = face_anchor[feature_index * face_anchor_num * 2 + pp * 2 + 0];
                        const float bias_h = face_anchor[feature_index * face_anchor_num * 2 + pp * 2 + 1];
                        //float bbox_cx = (w + sigmoid(xptr[0])) / out.w;
                        //float bbox_cy = (h + sigmoid(yptr[0])) / out.h;
                        float bbox_cx = (w + xptr[0]) / out.w;
                        float bbox_cy = (h + yptr[0]) / out.h;
                        float bbox_w = exp(wptr[0]) * bias_w / face_det_w;
                        float bbox_h = exp(hptr[0]) * bias_h / face_det_h;
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
                        for (int q = 0; q < face_num_class; q++)
                        {
                            ncnn::Mat scores = out.channel_range(p + 5, face_num_class);
                            float score = sigmoid(scores.channel(q).row(h)[w]);
                            if (score > class_score)
                            {
                                class_index = q;
                                class_score = score;
                            }
                        }

                        BboxFace bbox = { 0 };
                        bbox.id = (unsigned int)class_index;
                        bbox.score = (unsigned int)(sigmoid(box_score) * 100);

                        bbox.x1 = (int)round(bbox_xmin * face_det_w);
                        bbox.y1 = (int)round(bbox_ymin * face_det_h);
                        bbox.x2 = (int)round(bbox_xmax * face_det_w);
                        bbox.y2 = (int)round(bbox_ymax * face_det_h);
                        bbox.area = (float)(bbox.x2 - bbox.x1 + 1) * (bbox.y2 - bbox.y1 + 1);
                        //memcpy(bbox.face_emb, box_emb_ptr, TRACK_EMB_SIZE);

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
    face_nms(boundingBox_, face_nms_threshold);

    gettimeofday(&end, NULL);
    long dis = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    costtime = (double)(dis) / 1000;

    total_time += costtime;
    total_det += 1;

    LOGD("detect cost %f ms, all:%f ms, det:%lu\n", costtime, total_time/total_det, boundingBox_.size());

    for (std::vector<struct BboxFace>::iterator it = boundingBox_.begin(); it != boundingBox_.end(); it++)
    {
        ncnn::Mat tempIm;
        /*printf("get [%d-%d-%d-%d]\n", (*it).y1, img_h - (*it).y2, (*it).x1, img_w - (*it).x2);*/
        ncnn::copy_cut_border(in, tempIm, it->y1, face_det_h-it->y2, it->x1, face_det_w-it->x2);

        gettimeofday(&start, NULL);

        ncnn::Mat attr_img;
        ncnn::resize_bilinear(tempIm, attr_img, face_attr_det_w, face_attr_det_h);
        ncnn::Extractor ex_attr = g_face_attr_net.create_extractor();
        ex_attr.input("data", attr_img);
        ncnn::Mat out_attr;

//        score = 22; land = 37; glass = 39; male = 41; smile = 43; hat = 45; age = 47;
        float* pData = NULL;

        ex_attr.extract("attr_score_fwd", out_attr);
        pData = (float*)out_attr.data;
        it->attr.score = pData[0];

        ex_attr.extract("attr_land_fwd", out_attr);
        pData = (float*)out_attr.data;
        for (int i = 0; i < out_attr.w; ++i) {
            it->attr.land[i] = pData[i];
        }

        ex_attr.extract("attr_glass_fwd", out_attr);
        pData = (float*)out_attr.data;
        it->attr.glass = pData[0];

        ex_attr.extract("attr_male_fwd", out_attr);
        pData = (float*)out_attr.data;
        it->attr.male = pData[0];

        ex_attr.extract("attr_smile_fwd", out_attr);
        pData = (float*)out_attr.data;
        it->attr.smile = pData[0];

        ex_attr.extract("attr_hat_fwd", out_attr);
        pData = (float*)out_attr.data;
        it->attr.hat = pData[0];

        ex_attr.extract("attr_age_fwd", out_attr);
        pData = (float*)out_attr.data;
//        for (int i = 0; i < out.w; ++i) {
//            it->attr.age += pData[i] * i;
//        }
        it->attr.age = pData[0];

        LOGD("score = %f; land = 37; glass = %f; male = %f; smile = %f; hat = %f; age = %f\n",
                it->attr.score, it->attr.glass, it->attr.male, it->attr.smile, it->attr.hat, it->attr.age);
        gettimeofday(&end, NULL);
        dis = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
        costtime = (double)(dis) / 1000;

        LOGD("attr cost %f ms\n", costtime);

        gettimeofday(&start, NULL);

        ncnn::Mat rec_img;
        ncnn::resize_bilinear(tempIm, rec_img, face_rec_det_w, face_rec_det_h);

        float norm_vals[3] = {1.0f/256.0f, 1.0f/256.0f, 1.0f/256.0f};
        rec_img.substract_mean_normalize(NULL, norm_vals);

        ncnn::Extractor ex_rec = g_face_rec_net.create_extractor();
        ex_rec.input("data", rec_img);

        ncnn::Mat out_rec;
        ex_rec.extract("mobileclip0_feature_flatten0_flatten0", out_rec);
        pData = (float*)out_rec.data;

        float mod_sum = 0.0f;
        for (int i = 0; i < out_rec.w; i++)
        {
            it->rec.data[i] = pData[i];
            mod_sum += (pData[i] * pData[i]);
//            LOGD("index:%d, %f\n", i, it->rec.data[i]);
        }
        mod_sum = 1.0f / sqrtf(mod_sum);

        for (int i = 0; i < out_rec.w; i++)
        {
            it->rec.data[i] *= mod_sum;
//            LOGD("after:%d, %f\n", i, it->rec.data[i]);
        }
        gettimeofday(&end, NULL);
        dis = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
        costtime = (double)(dis) / 1000;

        LOGD("rec cost %f ms\n", costtime);
    }

    return boundingBox_;
}

#ifdef __cplusplus  
extern "C" {  
#endif

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_alghu_algdemo_utils_AlgManager_RunFaceModeByYuv(JNIEnv *env, jclass /* this */, jbyteArray YuvData, jint SrcWidth,
                                jint SrcHeight, jfloatArray OutputBuffer) {

    char *yuv_frame = (char*)env->GetPrimitiveArrayCritical(YuvData, NULL);

    size_t size = (size_t)env->GetArrayLength(YuvData);

    //shift argb to rgba
    char *yuv = (char *)malloc(size);
    memcpy(yuv, yuv_frame, size);

    env->ReleasePrimitiveArrayCritical(YuvData, yuv_frame, JNI_ABORT);

    std::vector<BboxFace> obj = face_det_run((const unsigned char *)yuv, SrcWidth, SrcHeight);

    free(yuv);

    float *detect_out = (float *)env->GetPrimitiveArrayCritical(OutputBuffer, NULL);

//    for(i = 0 ; i < objectcnt ; i++)
    int index = 0;
    for (std::vector<struct BboxFace>::iterator it = obj.begin(); it != obj.end(); it++)
    {
        if(index >= MAX_FACE_NUM)
        {
            break;
        }
        detect_out[index * MAX_FACE_LEN + 0] = it->id;
        detect_out[index * MAX_FACE_LEN + 1] = it->score;
        detect_out[index * MAX_FACE_LEN + 2] = (float)it->x1 / face_det_w;
        detect_out[index * MAX_FACE_LEN + 3] = (float)it->y1 / face_det_h;
        detect_out[index * MAX_FACE_LEN + 4] = (float)it->x2 / face_det_w;
        detect_out[index * MAX_FACE_LEN + 5] = (float)it->y2 / face_det_h;

        int face_w = it->x2 - it->x1;
        int face_h = it->y2 - it->y1;

        for (int i = 0; i < 5; ++i) {
            it->attr.land[i * 2 + 0] = (it->attr.land[i * 2 + 0] * face_w + it->x1) / face_det_w;
            it->attr.land[i * 2 + 1] = (it->attr.land[i * 2 + 1] * face_h + it->y1) / face_det_h;
        }

        memcpy(detect_out + (index * MAX_FACE_LEN + 6), &it->attr, sizeof(it->attr));
        memcpy(detect_out + (index * MAX_FACE_LEN + 6 + sizeof(it->attr) / sizeof(float)), &it->rec, sizeof(it->rec));

        index++;
        LOGI("%d = %d at %f %f %f %f\n", it->id , it->score, (float)it->x1 / face_det_w, (float)it->y1 / face_det_h, (float)it->x2 / face_det_w, (float)it->y2 / face_det_h);
    }

    env->ReleasePrimitiveArrayCritical(OutputBuffer, detect_out, JNI_ABORT);

    return OutputBuffer;
}

extern "C" JNIEXPORT int JNICALL
//        String DetParamPath, String DetBinPath, String AttrParamPath, String AttrBinPath, int runOnWhere, int runWidth, int runHeight
Java_com_alghu_algdemo_utils_AlgManager_InitFaceModelFromFile(JNIEnv *env, jclass /* this */,
        jstring DetParamPath,jstring DetBinPath, jstring AttrParamPath, jstring AttrBinPath, jstring RecParamPath, jstring RecBinPath,
        jint RunOnWhere, jint runWidth, jint runHeight)
{
    LOGI("will start init face all\n");

    const char *det_param_path = env->GetStringUTFChars(DetParamPath, 0);
    const char *det_model_path = env->GetStringUTFChars(DetBinPath, 0);
    const char *attr_param_path = env->GetStringUTFChars(AttrParamPath, 0);
    const char *attr_model_path = env->GetStringUTFChars(AttrBinPath, 0);
    const char *rec_param_path = env->GetStringUTFChars(RecParamPath, 0);
    const char *rec_model_path = env->GetStringUTFChars(RecBinPath, 0);

    const int run_on_where = RunOnWhere;

    face_det_w = (int)((float)runWidth / 32 + 0.5f) * 32;
    face_det_h = (int)((float)runHeight / 32 + 0.5f) * 32;

#if NCNN_VULKAN
    if(1 == run_on_where) {
//        ncnn::create_gpu_instance();
        g_face_det_net.opt.use_vulkan_compute = true;
        g_face_attr_net.opt.use_vulkan_compute = true;
        g_face_rec_net.opt.use_vulkan_compute = true;
    }
#endif // NCNN_VULKAN

    g_blob_pool_allocator.clear();
    g_workspace_pool_allocator.clear();
    g_face_det_net.opt.blob_allocator = &g_blob_pool_allocator;
    g_face_det_net.opt.workspace_allocator = &g_workspace_pool_allocator;

    g_face_attr_net.opt.blob_allocator = &g_blob_pool_allocator;
    g_face_attr_net.opt.workspace_allocator = &g_workspace_pool_allocator;

    g_face_rec_net.opt.blob_allocator = &g_blob_pool_allocator;
    g_face_rec_net.opt.workspace_allocator = &g_workspace_pool_allocator;

    //no vulkan, size 0.0,0.0 1-105,2-78,3-69,4-67,5-98,8-175
    //no vulkan, size 1.0,0.0 4-80, size 0.5,0.0 4-85, size 0.5,0.5 4-80, size 0.0,0.5 4-85, size 0.0,1.0 4-80
    //vulkan, size 0.0,0.0 1-50,4-50

    g_face_det_net.opt.num_threads = 4;
    g_face_det_net.opt.use_packing_layout = true;
//    g_face_det_net.load_param(__face_o_param_bin);
//    g_face_det_net.load_model(__face_o_bin);
    g_face_det_net.load_param(det_param_path);
    g_face_det_net.load_model(det_model_path);

    g_face_attr_net.opt.num_threads = 4;
    g_face_attr_net.opt.use_packing_layout = true;
    g_face_attr_net.load_param(attr_param_path);
    g_face_attr_net.load_model(attr_model_path);

    g_face_rec_net.opt.num_threads = 4;
    g_face_rec_net.opt.use_packing_layout = true;
    g_face_rec_net.load_param(rec_param_path);
    g_face_rec_net.load_model(rec_model_path);

    env->ReleaseStringUTFChars(DetParamPath, det_param_path);
    env->ReleaseStringUTFChars(DetBinPath, det_model_path);
    env->ReleaseStringUTFChars(AttrParamPath, attr_param_path);
    env->ReleaseStringUTFChars(AttrBinPath, attr_model_path);
    env->ReleaseStringUTFChars(RecParamPath, rec_param_path);
    env->ReleaseStringUTFChars(RecBinPath, rec_model_path);

    return 0;
}

extern "C" JNIEXPORT int JNICALL
Java_com_alghu_algdemo_utils_AlgManager_UinitFaceModel(JNIEnv *env, jclass /* this */, jint RunOnWhere)
{
    const int run_on_where = RunOnWhere;

    g_face_det_net.clear();

    g_face_attr_net.clear();

    g_face_rec_net.clear();

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