/***********************************************************************
* ˵��: �����ṩͳһͼ������ӿ�
*
* ����:�з���->�㷨��
************************************************************************/

#ifndef HU_ALG_IMG_PROC_H_
#define HU_ALG_IMG_PROC_H_

#include "huAlgCommon.h"

#ifdef __cplusplus  
extern "C" {
#endif 

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)  

#elif defined(__ARM_NEON__) || defined(__ARM_NEON)
#include <arm_neon.h>
#endif

//ͼ�����
int alg_rgbResize(const INPUT_IMG* pSrc, const rect_i32* pSrcRoi, INPUT_IMG* pDet);
int alg_bgrcut2bgrplane(unsigned char* __restrict pBgrCutBuf, const int nWidth, const int nHeight, unsigned char* __restrict pBgrPlaneBuf); // bgrͨ������
int alg_bgrplane2bgrcut(unsigned char* __restrict pBgrPlaneBuf, const int nWidth, const int nHeight, unsigned char* __restrict pBgrCutBuf); // bgrͨ����֯
int alg_yuv4202bgrplane(unsigned char* __restrict pYuvBuf, const int nWidth, const int nHeight, unsigned char* __restrict pRgbBuf);         // 420 y u v �ֱ�洢,rgb�ֱ�洢
int alg_yuv4202rgbplane(unsigned char* __restrict pYuvBuf, const int nWidth, const int nHeight, unsigned char* __restrict pRgbBuf);         // 420 y u v �ֱ�洢,rgb�ֱ�洢
int alg_yuv4202rgbcut(unsigned char* __restrict pYuvBuf, const int nWidth, const int nHeight, unsigned char* __restrict pRgbBuf);           // 420 y u v �ֱ�洢,rgb�����洢
int alg_yuv4202bgrcut(unsigned char* __restrict pYuvBuf, const int nWidth, const int nHeight, unsigned char* __restrict pRgbBuf);			// 420 y u v �ֱ�洢,bgr�����洢
int alg_yuv4202bgrplanemn(unsigned char* __restrict pYuvBuf, const int nWidth, const int nHeight, float* __restrict pBgrBuf, const float* mean_vals, const float* norm_vals);         // 420 y u v �ֱ�洢,bgr�ֱ�洢
int alg_rgbcut2rgbplanemn(const unsigned char* __restrict pSrcBuf, const int nWidth, const int nHeight, float* __restrict pBgrBuf, const float* mean_vals, const float* norm_vals); // rgb��֯ ��һ��
int alg_rgbplanHFilp(const void* __restrict pSrcBuf, const int nWidth, const int nHeight, void* __restrict pDetBuf, const int nEleSize); //rgb ƽ�棬ˮƽ��ת
int alg_CutImg(const INPUT_IMG* pSrc, unsigned char* pbCut, rect_i32* pDetRect); //����Ҫ���ȡ����
int alg_rgbcutRotion(unsigned char* pSrc, unsigned char* pDst, float* pMatrix, unsigned int imgSW, unsigned int imgSH, unsigned int imgDW, unsigned int imgDH); //rgb�����洢
int alg_rgbcut2gray(const unsigned char* pSrc, unsigned char* pDst, unsigned int imgW, unsigned int imgH);
int alg_rgbplane2gray(const unsigned char* pSrc, unsigned char* pDst, unsigned int imgW, unsigned int imgH);
int alg_gray2rgbcut(const unsigned char* pSrc, unsigned char* pDst, unsigned int imgW, unsigned int imgH);
int alg_bgrcutResize(unsigned char* pSrc, unsigned char* pDst, unsigned int imgSW, unsigned int imgSH, unsigned int imgDW, unsigned int imgDH); //bgr�����洢
void alg_bgrcutResizeV2(unsigned char *pSrc, unsigned char *pDst, int imgSW, int imgSH, unsigned int imgDW, unsigned int imgDH);
int alg_bgrcutCrop(unsigned char* pSrc, unsigned char* pDst, int startX, int startY, int imgSW, int imgSH, int imgDW, int imgDH); //bgr�����洢

float FastSin(float x);
float FastCos(float x);
float FastAtan2f(float dy, float dx);
float FastSqrtf(float x);
int FastSqrtI32(int x);
float FastInvSqrtf(float x);
void GetGaussinRotationM2D(float a[3][3], float b[3], float x[3]);
void GetRotationM2D(unsigned int x, unsigned int y, float angle, float scale, float* pMatrix);//��ȡ�������
void matrix_invert3X3(float* dst, const float* src); //��������

ulong64 algGetPhash(const INPUT_IMG* pSrc, const rect_i32* pSrcRoi);
ulong64 algGetDhash(const INPUT_IMG* pSrc, const rect_i32* pSrcRoi);
int algGetHashDistance(const ulong64 hash1, const ulong64 hash2);
float algGetCosDistance(const float* v1, const float* v2, const unsigned int Size);

int algGetFaceScore(const INPUT_IMG* pSrc, const rect_i32* pSrcRoi, const float* pDetPoint);

#ifdef __cplusplus
};
#endif
#endif // ! HU_ALG_IMG_PROC_H_
