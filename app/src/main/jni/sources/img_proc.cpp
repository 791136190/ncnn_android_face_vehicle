#include "huAlgImgProc.h"
#include "huAlgMemSys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus  
extern "C" {
#endif

	int alg_rgbResize(const INPUT_IMG* pSrc, const rect_i32* pSrcRoi, INPUT_IMG* pDet)
	{
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
		if (NULL == pSrc || NULL == pDet || NULL == pSrc->data || NULL == pDet->data)
		{
			fprintf(stderr, "get ptr is null\n");
			return ALG_PTR_NULL;
		}
		if ((pSrc->Ch != 3) || (pSrc->Ch != pDet->Ch) || (pSrc->eImgType != pDet->eImgType))
		{
			fprintf(stderr, "get param err\n");
			return ALG_PARAM_ERR;
		}

		if (pSrc->ImgH == pDet->ImgH && pSrc->ImgW == pDet->ImgW)
		{
			memcpy(pDet->data, pSrc->data, pSrc->ImgH * pSrc->ImgW * pSrc->Ch);
			return ALG_OK;
		}

		float scale_ratio_h = (float)pSrc->ImgH / pDet->ImgH;
		float scale_ratio_w = (float)pSrc->ImgW / pDet->ImgW;

		unsigned int TmpH = 0;
		unsigned int TmpW = 0;
		unsigned char* pSrcB = (unsigned char*)pSrc->data;
		unsigned char* pSrcG = (unsigned char*)pSrc->data + pSrc->ImgW * pSrc->ImgH;
		unsigned char* pSrcR = (unsigned char*)pSrc->data + pSrc->ImgW * pSrc->ImgH * 2;

		unsigned char* pDetB = (unsigned char*)pDet->data;
		unsigned char* pDetG = (unsigned char*)pDet->data + pDet->ImgW * pDet->ImgH;
		unsigned char* pDetR = (unsigned char*)pDet->data + pDet->ImgW * pDet->ImgH * 2;

		if (NULL != pSrcRoi)
		{
			scale_ratio_h = (float)pSrcRoi->h / pDet->ImgH;
			scale_ratio_w = (float)pSrcRoi->w / pDet->ImgW;

			pSrcB = (unsigned char*)pSrc->data + pSrc->ImgW * pSrcRoi->y + pSrcRoi->x;
			pSrcG = (unsigned char*)pSrc->data + pSrc->ImgW * pSrcRoi->y + pSrcRoi->x + pSrc->ImgW * pSrc->ImgH;
			pSrcR = (unsigned char*)pSrc->data + pSrc->ImgW * pSrcRoi->y + pSrcRoi->x + pSrc->ImgW * pSrc->ImgH * 2;
		}
		if (NULL == pSrcB || NULL == pSrcG || NULL == pSrcR)
		{
			fprintf(stderr, "get src bgr ptr err\n");
			return ALG_PTR_NULL;
		}

		unsigned int DetTmp = 0;
		unsigned int SrcTmp = 0;
		unsigned int DetXTmp = 0;
		unsigned int SrcXTmp = 0;
		unsigned int nDetTmp[4];
		unsigned int nSrcTmp[4];
		unsigned int nn = pDet->ImgW - 3;
		for (TmpH = 0; TmpH < pDet->ImgH; TmpH++)
		{
			DetXTmp = TmpH*pDet->ImgW;
			SrcXTmp = (unsigned int)(TmpH*scale_ratio_h) * pSrc->ImgW;

			uint32x4_t u32Det = vmovq_n_u32(DetXTmp);
			uint32x4_t u32Src = vmovq_n_u32(SrcXTmp);

			for (TmpW = 0; TmpW < nn; TmpW += 4)
			{
				uint32x4_t nTmpW = { TmpW , TmpW + 1, TmpW + 2, TmpW + 3 };
				vst1q_u32(nDetTmp, vaddq_u32(u32Det, nTmpW));
				vst1q_u32(nSrcTmp, vaddq_u32(u32Src, vcvtq_u32_f32(vmulq_f32(vcvtq_f32_u32(nTmpW), vmovq_n_f32(scale_ratio_w)))));

				*(pDetB + nDetTmp[0]) = *(pSrcB + nSrcTmp[0]);
				*(pDetG + nDetTmp[0]) = *(pSrcG + nSrcTmp[0]);
				*(pDetR + nDetTmp[0]) = *(pSrcR + nSrcTmp[0]);
				*(pDetB + nDetTmp[1]) = *(pSrcB + nSrcTmp[1]);
				*(pDetG + nDetTmp[1]) = *(pSrcG + nSrcTmp[1]);
				*(pDetR + nDetTmp[1]) = *(pSrcR + nSrcTmp[1]);
				*(pDetB + nDetTmp[2]) = *(pSrcB + nSrcTmp[2]);
				*(pDetG + nDetTmp[2]) = *(pSrcG + nSrcTmp[2]);
				*(pDetR + nDetTmp[2]) = *(pSrcR + nSrcTmp[2]);
				*(pDetB + nDetTmp[3]) = *(pSrcB + nSrcTmp[3]);
				*(pDetG + nDetTmp[3]) = *(pSrcG + nSrcTmp[3]);
				*(pDetR + nDetTmp[3]) = *(pSrcR + nSrcTmp[3]);
			}

			for (; TmpW < pDet->ImgW; TmpW++)
			{
				DetTmp = DetXTmp + TmpW;
				SrcTmp = SrcXTmp + TmpW*scale_ratio_w;

				*(pDetB + DetTmp) = *(pSrcB + SrcTmp);
				*(pDetG + DetTmp) = *(pSrcG + SrcTmp);
				*(pDetR + DetTmp) = *(pSrcR + SrcTmp);
			}
		}
#else
		if (NULL == pSrc || NULL == pDet || NULL == pSrc->data || NULL == pDet->data)
		{
			fprintf(stderr, "get ptr is null\n");
			return ALG_PTR_NULL;
		}
		if ((pSrc->Ch != 3) || (pSrc->Ch != pDet->Ch) || (pSrc->eImgType != pDet->eImgType))
		{
			fprintf(stderr, "get param err\n");
			return ALG_PARAM_ERR;
		}

		if (pSrc->ImgH == pDet->ImgH && pSrc->ImgW == pDet->ImgW)
		{
			memcpy(pDet->data, pSrc->data, pSrc->ImgH * pSrc->ImgW * pSrc->Ch);
			return ALG_OK;
		}

		float scale_ratio_h = (float)pSrc->ImgH / pDet->ImgH;
		float scale_ratio_w = (float)pSrc->ImgW / pDet->ImgW;

		unsigned int TmpH = 0;
		unsigned int TmpW = 0;
		unsigned char* pSrcB = (unsigned char*)pSrc->data;
		unsigned char* pSrcG = (unsigned char*)pSrc->data + pSrc->ImgW * pSrc->ImgH;
		unsigned char* pSrcR = (unsigned char*)pSrc->data + pSrc->ImgW * pSrc->ImgH * 2;

		unsigned char* pDetB = (unsigned char*)pDet->data;
		unsigned char* pDetG = (unsigned char*)pDet->data + pDet->ImgW * pDet->ImgH;
		unsigned char* pDetR = (unsigned char*)pDet->data + pDet->ImgW * pDet->ImgH * 2;

		if (NULL != pSrcRoi)
		{
			scale_ratio_h = (float)pSrcRoi->h / pDet->ImgH;
			scale_ratio_w = (float)pSrcRoi->w / pDet->ImgW;

			pSrcB = (unsigned char*)pSrc->data + pSrc->ImgW * pSrcRoi->y + pSrcRoi->x;
			pSrcG = (unsigned char*)pSrc->data + pSrc->ImgW * pSrcRoi->y + pSrcRoi->x + pSrc->ImgW * pSrc->ImgH;
			pSrcR = (unsigned char*)pSrc->data + pSrc->ImgW * pSrcRoi->y + pSrcRoi->x + pSrc->ImgW * pSrc->ImgH * 2;
		}
		if (NULL == pSrcB || NULL == pSrcG || NULL == pSrcR)
		{
			fprintf(stderr, "get src bgr ptr err\n");
			return ALG_PTR_NULL;
		}

		unsigned int DetTmp = 0;
		unsigned int SrcTmp = 0;
		unsigned int HWTmp = 0;
		unsigned int HHTmp = 0;

		for (TmpH = 0; TmpH < pDet->ImgH; TmpH++)
		{
			HWTmp = (unsigned int)(TmpH * scale_ratio_h) * pSrc->ImgW;
			HHTmp = TmpH * pDet->ImgW;

			for (TmpW = 0; TmpW < pDet->ImgW; TmpW++)
			{
				DetTmp = HHTmp + TmpW;
				SrcTmp = HWTmp + (unsigned int)(TmpW * scale_ratio_w);

				*(pDetB + DetTmp) = *(pSrcB + SrcTmp);
				*(pDetG + DetTmp) = *(pSrcG + SrcTmp);
				*(pDetR + DetTmp) = *(pSrcR + SrcTmp);
			}
		}
#endif
		return ALG_OK;
	}

	static __inline unsigned char border_color(const int color)
	{
		if (color > 255)
			return   255;
		else if (color < 0)
			return   0;
		else
			return  color;
	}
	int alg_bgrcut2bgrplane(unsigned char* __restrict pBgrCutBuf, const int nWidth, const int nHeight, unsigned char* __restrict pBgrPlaneBuf)
	{
		if (NULL == pBgrCutBuf || NULL == pBgrPlaneBuf)
		{
			fprintf(stderr, "get ptr null,pBgrCutBuf:%p, pBgrPlaneBuf:%p\n", pBgrCutBuf, pBgrPlaneBuf);
			return ALG_PTR_NULL;
		}

		const unsigned int nYLen = nWidth * nHeight;
		if (nYLen < 1)
		{
			fprintf(stderr, "get nYLen:%u\n", nYLen);
			return ALG_ERR_OTHER;
		}

		unsigned char* pBBuf = pBgrPlaneBuf;
		unsigned char* pGBuf = pBgrPlaneBuf + nYLen;
		unsigned char* pRBuf = pBgrPlaneBuf + nYLen * 2;

		for (unsigned int i = 0; i < nYLen; i++)
		{
			pBBuf[i] = pBgrCutBuf[i * 3 + 0];
			pGBuf[i] = pBgrCutBuf[i * 3 + 1];
			pRBuf[i] = pBgrCutBuf[i * 3 + 2];
		}

		return ALG_OK;
	}

	int alg_bgrplane2bgrcut(unsigned char* __restrict pBgrPlaneBuf, const int nWidth, const int nHeight, unsigned char* __restrict pBgrCutBuf)
	{
		if (NULL == pBgrCutBuf || NULL == pBgrPlaneBuf)
		{
			fprintf(stderr, "get ptr null,pYuvBuf:%p, pRgbBuf:%p\n", pBgrCutBuf, pBgrPlaneBuf);
			return ALG_PTR_NULL;
		}

		const unsigned int nYLen = nWidth * nHeight;
		if (nYLen < 1)
		{
			fprintf(stderr, "get ptr null,pYuvBuf:%u\n", nYLen);
			return ALG_ERR_OTHER;
		}

		unsigned char* pBBuf = pBgrPlaneBuf;
		unsigned char* pGBuf = pBgrPlaneBuf + nYLen;
		unsigned char* pRBuf = pBgrPlaneBuf + nYLen * 2;
		unsigned char* pBGR = pBgrCutBuf;

		for (unsigned int i = 0; i < nYLen; i++)
		{
			pBGR[0] = pBBuf[i];
			pBGR[1] = pGBuf[i];
			pBGR[2] = pRBuf[i];
			pBGR += 3;
		}

		return ALG_OK;
	}

	int alg_yuv4202bgrplane(unsigned char* __restrict pYuvBuf, const int nWidth, const int nHeight, unsigned char* __restrict pRgbBuf)
	{
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
		if (NULL == pYuvBuf || NULL == pRgbBuf)
		{
			fprintf(stderr, "get ptr null,pYuvBuf:%p, pRgbBuf:%p\n", pYuvBuf, pRgbBuf);
			return ALG_PTR_NULL;
		}

		const unsigned int nYLen = nWidth * nHeight;
		if (nYLen < 1)
		{
			fprintf(stderr, "get ptr null,pYuvBuf:%u\n", nYLen);
			return ALG_ERR_OTHER;
		}

		unsigned char* yptr = pYuvBuf;
		unsigned char* uptr = yptr + nYLen;
		unsigned char* vptr = uptr + ((nYLen) >> 2);

		unsigned char* b = pRgbBuf;
		unsigned char* g = b + (nYLen);
		unsigned char* r = b + ((nYLen) << 1);

		int8x8_t _v128 = vdup_n_s8(128);
		int8x8_t _v90 = vdup_n_s8(90);
		int8x8_t _v46 = vdup_n_s8(46);
		int8x8_t _v22 = vdup_n_s8(22);
		int8x8_t _v113 = vdup_n_s8(113);

		int y = 0;
		int nn = 0;// 一次跑8个y数据
		unsigned int yTmp = 0;

		for (y = 0; y < nHeight; y += 2)
		{
			yTmp = y * nWidth;

			yptr = pYuvBuf + yTmp;
			uptr = pYuvBuf + (nYLen)+((yTmp) >> 2);
			vptr = uptr + ((nYLen) >> 2);

			b = pRgbBuf + yTmp;
			g = b + nYLen;
			r = g + nYLen;

			for (nn = 0; nn + 7 < nWidth; nn += 8)
			{
				// R = ((Y << 6) + 90 * (V-128)) >> 6
				// G = ((Y << 6) - 46 * (V-128) - 22 * (U-128)) >> 6
				// B = ((Y << 6) + 113 * (U-128)) >> 6
				int16x8_t _yy0 = vreinterpretq_s16_u16(vshll_n_u8(vld1_u8(yptr), 6));
				int16x8_t _yy1 = vreinterpretq_s16_u16(vshll_n_u8(vld1_u8(yptr + nWidth), 6));
				int8x8_t _uu = vsub_s8(vreinterpret_s8_u8(vld1_u8(uptr)), _v128);
				int8x8_t _vv = vsub_s8(vreinterpret_s8_u8(vld1_u8(vptr)), _v128);

				int8x8x2_t _uuuu = vzip_s8(_uu, _uu);
				_uu = _uuuu.val[0];//第一维是0-3

				int8x8x2_t _vvvv = vzip_s8(_vv, _vv);
				_vv = _vvvv.val[0];

				int16x8_t _r0 = vmlal_s8(_yy0, _vv, _v90);
				int16x8_t _g0 = vmlsl_s8(_yy0, _vv, _v46);
				_g0 = vmlsl_s8(_g0, _uu, _v22);
				int16x8_t _b0 = vmlal_s8(_yy0, _uu, _v113);

				int16x8_t _r1 = vmlal_s8(_yy1, _vv, _v90);
				int16x8_t _g1 = vmlsl_s8(_yy1, _vv, _v46);
				_g1 = vmlsl_s8(_g1, _uu, _v22);
				int16x8_t _b1 = vmlal_s8(_yy1, _uu, _v113);

				vst1_u8(b, vqshrun_n_s16(_b0, 6));
				vst1_u8(b + nWidth, vqshrun_n_s16(_b1, 6));
				vst1_u8(g, vqshrun_n_s16(_g0, 6));
				vst1_u8(g + nWidth, vqshrun_n_s16(_g1, 6));
				vst1_u8(r, vqshrun_n_s16(_r0, 6));
				vst1_u8(r + nWidth, vqshrun_n_s16(_r1, 6));

				yptr += 8;
				uptr += 4;
				vptr += 4;
				b += 8;
				g += 8;
				r += 8;
			}

			for (; nn < nWidth; nn += 2)
			{
				int u = uptr[0] - 128;
				int v = vptr[0] - 128;

				int ruv = 90 * v;
				int guv = -46 * v - 22 * u;
				int buv = 113 * u;

				int y00 = *(yptr) << 6;
				*(b) = border_color((y00 + buv) >> 6);
				*(g) = border_color((y00 + guv) >> 6);
				*(r) = border_color((y00 + ruv) >> 6);

				int y01 = *(yptr + 1) << 6;
				*(b + 1) = border_color((y01 + buv) >> 6);
				*(g + 1) = border_color((y01 + guv) >> 6);
				*(r + 1) = border_color((y01 + ruv) >> 6);

				int y10 = *(yptr + nWidth) << 6;
				*(b + nWidth) = border_color((y10 + buv) >> 6);
				*(g + nWidth) = border_color((y10 + guv) >> 6);
				*(r + nWidth) = border_color((y10 + ruv) >> 6);

				int y11 = *(yptr + nWidth + 1) << 6;
				*(b + nWidth + 1) = border_color((y11 + buv) >> 6);
				*(g + nWidth + 1) = border_color((y11 + guv) >> 6);
				*(r + nWidth + 1) = border_color((y11 + ruv) >> 6);

				yptr += 2;
				uptr += 1;
				vptr += 1;
				b += 2;
				g += 2;
				r += 2;
			}
		}
#else
		unsigned char* yptr = pYuvBuf;
		unsigned char* uptr = yptr + (nWidth * nHeight);
		unsigned char* vptr = uptr + (nWidth * nHeight) / 4;

		unsigned char* b = pRgbBuf;
		unsigned char* g = b + (nWidth * nHeight);
		unsigned char* r = b + (nWidth * nHeight) * 2;

		int y = 0;
		int nn = 0;

		for (y = 0; y < nHeight; y += 2)
		{
			yptr = pYuvBuf + y * nWidth;
			uptr = pYuvBuf + (nWidth * nHeight) + ((y * nWidth) >> 2);
			vptr = uptr + ((nWidth * nHeight) >> 2);

			b = pRgbBuf + y * nWidth;
			g = b + (nWidth * nHeight);
			r = b + (nWidth * nHeight) * 2;
			for (nn = 0; nn < nWidth; nn += 2)
			{
				int u = uptr[0] - 128;
				int v = vptr[0] - 128;

				int ruv = 90 * v;
				int guv = -46 * v - 22 * u;
				int buv = 113 * u;

				int y00 = *(yptr) << 6;
				*(b) = border_color((y00 + buv) >> 6);
				*(g) = border_color((y00 + guv) >> 6);
				*(r) = border_color((y00 + ruv) >> 6);

				int y01 = *(yptr + 1) << 6;
				*(b + 1) = border_color((y01 + buv) >> 6);
				*(g + 1) = border_color((y01 + guv) >> 6);
				*(r + 1) = border_color((y01 + ruv) >> 6);

				int y10 = *(yptr + nWidth) << 6;
				*(b + nWidth) = border_color((y10 + buv) >> 6);
				*(g + nWidth) = border_color((y10 + guv) >> 6);
				*(r + nWidth) = border_color((y10 + ruv) >> 6);

				int y11 = *(yptr + nWidth + 1) << 6;
				*(b + nWidth + 1) = border_color((y11 + buv) >> 6);
				*(g + nWidth + 1) = border_color((y11 + guv) >> 6);
				*(r + nWidth + 1) = border_color((y11 + ruv) >> 6);

				yptr += 2;
				uptr += 1;
				vptr += 1;
				b += 2;
				g += 2;
				r += 2;
			}
		}

#endif
		return ALG_OK;
	}

	int alg_yuv4202bgrplanemn(unsigned char* __restrict pYuvBuf, const int nWidth, const int nHeight, float* __restrict pBgrBuf, const float* mean_vals, const float* norm_vals)
	{
		const size_t ImagLen = nWidth * nHeight;

		unsigned char* yptr = pYuvBuf;
		unsigned char* uptr = yptr + ImagLen;
		unsigned char* vptr = uptr + ImagLen / 4;

		float* b = pBgrBuf;
		float* g = b + ImagLen;
		float* r = g + ImagLen;

		const float b_mean = mean_vals[0];
		const float g_mean = mean_vals[1];
		const float r_mean = mean_vals[2];

		const float b_norm = norm_vals[0];
		const float g_norm = norm_vals[1];
		const float r_norm = norm_vals[2];

		int y = 0;
		int nn = 0;

		for (y = 0; y < nHeight; y += 2)
		{
			yptr = pYuvBuf + y * nWidth;
			uptr = pYuvBuf + ImagLen + ((y * nWidth) >> 2);
			vptr = uptr + (ImagLen >> 2);

			b = pBgrBuf + y * nWidth;
			g = b + ImagLen;
			r = g + ImagLen;
			for (nn = 0; nn < nWidth; nn += 2)
			{
				int u = uptr[0] - 128;
				int v = vptr[0] - 128;

				int ruv = 90 * v;
				int guv = -46 * v - 22 * u;
				int buv = 113 * u;

				int y00 = *(yptr) << 6;
				*(b) = (border_color((y00 + buv) >> 6) - b_mean) * b_norm;
				*(g) = (border_color((y00 + guv) >> 6) - g_mean) * g_norm;
				*(r) = (border_color((y00 + ruv) >> 6) - r_mean) * r_norm;

				int y01 = *(yptr + 1) << 6;
				*(b + 1) = (border_color((y01 + buv) >> 6) - b_mean) * b_norm;
				*(g + 1) = (border_color((y01 + guv) >> 6) - g_mean) * g_norm;
				*(r + 1) = (border_color((y01 + ruv) >> 6) - r_mean) * r_norm;

				int y10 = *(yptr + nWidth) << 6;
				*(b + nWidth) = (border_color((y10 + buv) >> 6) - b_mean) * b_norm;
				*(g + nWidth) = (border_color((y10 + guv) >> 6) - g_mean) * g_norm;
				*(r + nWidth) = (border_color((y10 + ruv) >> 6) - r_mean) * r_norm;

				int y11 = *(yptr + nWidth + 1) << 6;
				*(b + (nWidth + 1)) = (border_color((y11 + buv) >> 6) - b_mean) * b_norm;
				*(g + (nWidth + 1)) = (border_color((y11 + guv) >> 6) - g_mean) * g_norm;
				*(r + (nWidth + 1)) = (border_color((y11 + ruv) >> 6) - r_mean) * r_norm;

				yptr += 2;
				uptr += 1;
				vptr += 1;
				b += 2;
				g += 2;
				r += 2;
			}
		}
		return ALG_OK;
	}

	int alg_rgbcut2rgbplanemn(const unsigned char* __restrict pSrcBuf, const int nWidth, const int nHeight, float* __restrict pBgrBuf, const float* mean_vals, const float* norm_vals)
	{
		const size_t ImagLen = nWidth * nHeight;

		const unsigned char* pSrcRgb = pSrcBuf;//交织模式

		float* r = pBgrBuf;
		float* g = r + ImagLen;
		float* b = g + ImagLen;

		if (mean_vals && norm_vals)
		{
			const float r_mean = mean_vals[0];
			const float g_mean = mean_vals[1];
			const float b_mean = mean_vals[2];

			const float r_norm = norm_vals[0];
			const float g_norm = norm_vals[1];
			const float b_norm = norm_vals[2];


			for (int nn = 0; nn < nWidth*nHeight; nn++)
			{
				*r = (*(pSrcRgb + 0) - r_mean) * r_norm;
				*g = (*(pSrcRgb + 1) - g_mean) * g_norm;
				*b = (*(pSrcRgb + 2) - b_mean) * b_norm;
				r++;
				g++;
				b++;
				pSrcRgb += 3;
			}
		}
		else if (mean_vals)
		{
			const float r_mean = mean_vals[0];
			const float g_mean = mean_vals[1];
			const float b_mean = mean_vals[2];

			for (int nn = 0; nn < nWidth * nHeight; nn++)
			{
				*r = (*(pSrcRgb + 0) - r_mean);
				*g = (*(pSrcRgb + 1) - g_mean);
				*b = (*(pSrcRgb + 2) - b_mean);
				r++;
				g++;
				b++;
				pSrcRgb += 3;
			}
		}
		else
		{
			for (int nn = 0; nn < nWidth * nHeight; nn++)
			{
				*r++ = *pSrcRgb++;
				*g++ = *pSrcRgb++;
				*b++ = *pSrcRgb++;
			}
		}
		return ALG_OK;
	}

	int alg_rgbplanHFilp(const void* __restrict pSrcBuf, const int nWidth, const int nHeight, void* __restrict pDetBuf, const int nEleSize)
	{
		if (sizeof(float) == nEleSize)
		{
			float* pDetR = (float*)pDetBuf;
			float* pDetG = (float*)pDetBuf + nHeight * nWidth;
			float* pDetB = (float*)pDetBuf + nHeight * nWidth * 2;

			for (int i = 0; i < nHeight; i++)
			{
				const float* pSrcR = (float*)pSrcBuf + i * nWidth;
				const float* pSrcG = (float*)pSrcBuf + i * nWidth + nHeight * nWidth;
				const float* pSrcB = (float*)pSrcBuf + i * nWidth + nHeight * nWidth * 2;

				for (int j = 0; j < nWidth; j++)
				{
					*pDetR++ = pSrcR[nWidth - 1 - j];
					*pDetG++ = pSrcG[nWidth - 1 - j];
					*pDetB++ = pSrcB[nWidth - 1 - j];
				}
			}
		}
		else if (sizeof(unsigned char) == nEleSize)
		{
			unsigned char* pDetR = (unsigned char*)pDetBuf;
			unsigned char* pDetG = (unsigned char*)pDetBuf + nHeight * nWidth;
			unsigned char* pDetB = (unsigned char*)pDetBuf + nHeight * nWidth * 2;

			for (int i = 0; i < nHeight; i++)
			{
				const unsigned char* pSrcR = (unsigned char*)pSrcBuf + i * nWidth;
				const unsigned char* pSrcG = (unsigned char*)pSrcBuf + i * nWidth + nHeight * nWidth;
				const unsigned char* pSrcB = (unsigned char*)pSrcBuf + i * nWidth + nHeight * nWidth * 2;

				for (int j = 0; j < nWidth; j++)
				{
					*pDetR++ = pSrcR[nWidth - 1 - j];
					*pDetG++ = pSrcG[nWidth - 1 - j];
					*pDetB++ = pSrcB[nWidth - 1 - j];
				}
			}
		}
		return 0;
	}

	int alg_yuv4202rgbcut(unsigned char* __restrict pYuvBuf, const int nWidth, const int nHeight, unsigned char* __restrict pRgbBuf)
	{
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
		if (NULL == pYuvBuf || NULL == pRgbBuf)
		{
			fprintf(stderr, "get ptr null,pYuvBuf:%p, pRgbBuf:%p\n", pYuvBuf, pRgbBuf);
			return ALG_PTR_NULL;
		}

		const unsigned int nYLen = nWidth * nHeight;
		if (nYLen < 1)
		{
			fprintf(stderr, "get ptr null,pYuvBuf:%u\n", nYLen);
			return ALG_ERR_OTHER;
		}

		unsigned char* yptr = pYuvBuf;
		unsigned char* uptr = yptr + nYLen;
		unsigned char* vptr = uptr + ((nYLen) >> 2);

		unsigned char* pRgb = pRgbBuf;

		int8x8_t _v128 = vdup_n_s8(128);
		int8x8_t _v90 = vdup_n_s8(90);
		int8x8_t _v46 = vdup_n_s8(46);
		int8x8_t _v22 = vdup_n_s8(22);
		int8x8_t _v113 = vdup_n_s8(113);

		int y = 0;
		int nn = 0;// 一次跑8个y数据
		unsigned int yTmp = 0;

		for (y = 0; y < nHeight; y += 2)
		{
			yTmp = y * nWidth;

			yptr = pYuvBuf + yTmp;
			uptr = pYuvBuf + (nYLen)+((yTmp) >> 2);
			vptr = uptr + ((nYLen) >> 2);

			pRgb = pRgbBuf + y * nWidth * 3;
			for (nn = 0; nn + 7 < nWidth; nn += 8)
			{
				// R = ((Y << 6) + 90 * (V-128)) >> 6
				// G = ((Y << 6) - 46 * (V-128) - 22 * (U-128)) >> 6
				// B = ((Y << 6) + 113 * (U-128)) >> 6
				int16x8_t _yy0 = vreinterpretq_s16_u16(vshll_n_u8(vld1_u8(yptr), 6));
				int16x8_t _yy1 = vreinterpretq_s16_u16(vshll_n_u8(vld1_u8(yptr + nWidth), 6));
				int8x8_t _uu = vsub_s8(vreinterpret_s8_u8(vld1_u8(uptr)), _v128);
				int8x8_t _vv = vsub_s8(vreinterpret_s8_u8(vld1_u8(vptr)), _v128);

				int8x8x2_t _uuuu = vzip_s8(_uu, _uu);
				_uu = _uuuu.val[0];//第一维是0-3

				int8x8x2_t _vvvv = vzip_s8(_vv, _vv);
				_vv = _vvvv.val[0];

				int16x8_t _r0 = vmlal_s8(_yy0, _vv, _v90);
				int16x8_t _g0 = vmlsl_s8(_yy0, _vv, _v46);
				_g0 = vmlsl_s8(_g0, _uu, _v22);
				int16x8_t _b0 = vmlal_s8(_yy0, _uu, _v113);

				int16x8_t _r1 = vmlal_s8(_yy1, _vv, _v90);
				int16x8_t _g1 = vmlsl_s8(_yy1, _vv, _v46);
				_g1 = vmlsl_s8(_g1, _uu, _v22);
				int16x8_t _b1 = vmlal_s8(_yy1, _uu, _v113);

				uint8x8x3_t rgb0;
				rgb0.val[0] = vqshrun_n_s16(_r0, 6);
				rgb0.val[1] = vqshrun_n_s16(_g0, 6);
				rgb0.val[2] = vqshrun_n_s16(_b0, 6);
				vst3_u8(pRgb, rgb0);

				uint8x8x3_t rgb1;
				rgb1.val[0] = vqshrun_n_s16(_r1, 6);
				rgb1.val[1] = vqshrun_n_s16(_g1, 6);
				rgb1.val[2] = vqshrun_n_s16(_b1, 6);
				vst3_u8(pRgb + nWidth * 3, rgb1);

				yptr += 8;
				uptr += 4;
				vptr += 4;
				pRgb += 24;
			}

			for (; nn < nWidth; nn += 2)
			{
				int u = uptr[0] - 128;
				int v = vptr[0] - 128;

				int ruv = 90 * v;
				int guv = -46 * v - 22 * u;
				int buv = 113 * u;

				int y00 = *(yptr) << 6;
				*(pRgb) = border_color((y00 + ruv) >> 6);
				*(pRgb + 1) = border_color((y00 + guv) >> 6);
				*(pRgb + 2) = border_color((y00 + buv) >> 6);

				int y01 = *(yptr + 1) << 6;
				*(pRgb + 3) = border_color((y01 + ruv) >> 6);
				*(pRgb + 4) = border_color((y01 + guv) >> 6);
				*(pRgb + 5) = border_color((y01 + buv) >> 6);

				int y10 = *(yptr + nWidth) << 6;
				*(pRgb + nWidth * 3) = border_color((y10 + ruv) >> 6);
				*(pRgb + nWidth * 3 + 1) = border_color((y10 + guv) >> 6);
				*(pRgb + nWidth * 3 + 2) = border_color((y10 + buv) >> 6);

				int y11 = *(yptr + nWidth + 1) << 6;
				*(pRgb + nWidth * 3 + 3) = border_color((y11 + ruv) >> 6);
				*(pRgb + nWidth * 3 + 4) = border_color((y11 + guv) >> 6);
				*(pRgb + nWidth * 3 + 5) = border_color((y11 + buv) >> 6);

				yptr += 2;
				uptr += 1;
				vptr += 1;
				pRgb += 6;
			}
		}
#else
		unsigned char* yptr = pYuvBuf;
		unsigned char* uptr = yptr + (nWidth * nHeight);
		unsigned char* vptr = uptr + ((nWidth * nHeight) >> 2);

		unsigned char* pRgb = pRgbBuf;

		int y = 0;
		int nn = 0;

		for (y = 0; y < nHeight; y += 2)
		{
			yptr = pYuvBuf + y * nWidth;
			uptr = pYuvBuf + (nWidth * nHeight) + ((y * nWidth) >> 2);
			vptr = uptr + ((nWidth * nHeight) >> 2);

			pRgb = pRgbBuf + y * nWidth * 3;
			for (nn = 0; nn < nWidth; nn += 2)
			{
				int u = uptr[0] - 128;
				int v = vptr[0] - 128;

				int ruv = 90 * v;
				int guv = -46 * v - 22 * u;
				int buv = 113 * u;

				int y00 = *(yptr) << 6;
				*(pRgb) = border_color((y00 + ruv) >> 6);
				*(pRgb + 1) = border_color((y00 + guv) >> 6);
				*(pRgb + 2) = border_color((y00 + buv) >> 6);

				int y01 = *(yptr + 1) << 6;
				*(pRgb + 3) = border_color((y01 + ruv) >> 6);
				*(pRgb + 4) = border_color((y01 + guv) >> 6);
				*(pRgb + 5) = border_color((y01 + buv) >> 6);

				int y10 = *(yptr + nWidth) << 6;
				*(pRgb + nWidth * 3) = border_color((y10 + ruv) >> 6);
				*(pRgb + nWidth * 3 + 1) = border_color((y10 + guv) >> 6);
				*(pRgb + nWidth * 3 + 2) = border_color((y10 + buv) >> 6);

				int y11 = *(yptr + nWidth + 1) << 6;
				*(pRgb + nWidth * 3 + 3) = border_color((y11 + ruv) >> 6);
				*(pRgb + nWidth * 3 + 4) = border_color((y11 + guv) >> 6);
				*(pRgb + nWidth * 3 + 5) = border_color((y11 + buv) >> 6);

				yptr += 2;
				uptr += 1;
				vptr += 1;
				pRgb += 6;
			}
		}
#endif
		return 0;
	}
	
	int alg_yuv4202bgrcut(unsigned char* __restrict pYuvBuf, const int nWidth, const int nHeight, unsigned char* __restrict pRgbBuf)
	{
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
		if (NULL == pYuvBuf || NULL == pRgbBuf)
		{
			fprintf(stderr, "get ptr null,pYuvBuf:%p, pRgbBuf:%p\n", pYuvBuf, pRgbBuf);
			return ALG_PTR_NULL;
		}

		const unsigned int nYLen = nWidth * nHeight;
		if (nYLen < 1)
		{
			fprintf(stderr, "get ptr null,pYuvBuf:%u\n", nYLen);
			return ALG_ERR_OTHER;
		}

		unsigned char* yptr = pYuvBuf;
		unsigned char* uptr = yptr + nYLen;
		unsigned char* vptr = uptr + ((nYLen) >> 2);

		unsigned char* pRgb = pRgbBuf;

		int8x8_t _v128 = vdup_n_s8(128);
		int8x8_t _v90 = vdup_n_s8(90);
		int8x8_t _v46 = vdup_n_s8(46);
		int8x8_t _v22 = vdup_n_s8(22);
		int8x8_t _v113 = vdup_n_s8(113);

		int y = 0;
		int nn = 0;// 一次跑8个y数据
		unsigned int yTmp = 0;

		for (y = 0; y < nHeight; y += 2)
		{
			yTmp = y * nWidth;

			yptr = pYuvBuf + yTmp;
			uptr = pYuvBuf + (nYLen)+((yTmp) >> 2);
			vptr = uptr + ((nYLen) >> 2);

			pRgb = pRgbBuf + y * nWidth * 3;
			for (nn = 0; nn + 7 < nWidth; nn += 8)
			{
				// R = ((Y << 6) + 90 * (V-128)) >> 6
				// G = ((Y << 6) - 46 * (V-128) - 22 * (U-128)) >> 6
				// B = ((Y << 6) + 113 * (U-128)) >> 6
				int16x8_t _yy0 = vreinterpretq_s16_u16(vshll_n_u8(vld1_u8(yptr), 6));
				int16x8_t _yy1 = vreinterpretq_s16_u16(vshll_n_u8(vld1_u8(yptr + nWidth), 6));
				int8x8_t _uu = vsub_s8(vreinterpret_s8_u8(vld1_u8(uptr)), _v128);
				int8x8_t _vv = vsub_s8(vreinterpret_s8_u8(vld1_u8(vptr)), _v128);

				int8x8x2_t _uuuu = vzip_s8(_uu, _uu);
				_uu = _uuuu.val[0];//第一维是0-3

				int8x8x2_t _vvvv = vzip_s8(_vv, _vv);
				_vv = _vvvv.val[0];

				int16x8_t _r0 = vmlal_s8(_yy0, _vv, _v90);
				int16x8_t _g0 = vmlsl_s8(_yy0, _vv, _v46);
				_g0 = vmlsl_s8(_g0, _uu, _v22);
				int16x8_t _b0 = vmlal_s8(_yy0, _uu, _v113);

				int16x8_t _r1 = vmlal_s8(_yy1, _vv, _v90);
				int16x8_t _g1 = vmlsl_s8(_yy1, _vv, _v46);
				_g1 = vmlsl_s8(_g1, _uu, _v22);
				int16x8_t _b1 = vmlal_s8(_yy1, _uu, _v113);

				uint8x8x3_t rgb0;
				rgb0.val[0] = vqshrun_n_s16(_b0, 6);
				rgb0.val[1] = vqshrun_n_s16(_g0, 6);
				rgb0.val[2] = vqshrun_n_s16(_r0, 6);
				vst3_u8(pRgb, rgb0);

				uint8x8x3_t rgb1;
				rgb1.val[0] = vqshrun_n_s16(_b1, 6);
				rgb1.val[1] = vqshrun_n_s16(_g1, 6);
				rgb1.val[2] = vqshrun_n_s16(_r1, 6);
				vst3_u8(pRgb + nWidth * 3, rgb1);

				yptr += 8;
				uptr += 4;
				vptr += 4;
				pRgb += 24;
			}

			for (; nn < nWidth; nn += 2)
			{
				int u = uptr[0] - 128;
				int v = vptr[0] - 128;

				int ruv = 90 * v;
				int guv = -46 * v - 22 * u;
				int buv = 113 * u;

				int y00 = *(yptr) << 6;
				*(pRgb) = border_color((y00 + buv) >> 6);
				*(pRgb + 1) = border_color((y00 + guv) >> 6);
				*(pRgb + 2) = border_color((y00 + ruv) >> 6);

				int y01 = *(yptr + 1) << 6;
				*(pRgb + 3) = border_color((y01 + buv) >> 6);
				*(pRgb + 4) = border_color((y01 + guv) >> 6);
				*(pRgb + 5) = border_color((y01 + ruv) >> 6);

				int y10 = *(yptr + nWidth) << 6;
				*(pRgb + nWidth * 3) = border_color((y10 + buv) >> 6);
				*(pRgb + nWidth * 3 + 1) = border_color((y10 + guv) >> 6);
				*(pRgb + nWidth * 3 + 2) = border_color((y10 + ruv) >> 6);

				int y11 = *(yptr + nWidth + 1) << 6;
				*(pRgb + nWidth * 3 + 3) = border_color((y11 + buv) >> 6);
				*(pRgb + nWidth * 3 + 4) = border_color((y11 + guv) >> 6);
				*(pRgb + nWidth * 3 + 5) = border_color((y11 + ruv) >> 6);

				yptr += 2;
				uptr += 1;
				vptr += 1;
				pRgb += 6;
			}
		}
#else
		unsigned char* yptr = pYuvBuf;
		unsigned char* uptr = yptr + (nWidth * nHeight);
		unsigned char* vptr = uptr + ((nWidth * nHeight) >> 2);

		unsigned char* pRgb = pRgbBuf;

		int y = 0;
		int nn = 0;

		for (y = 0; y < nHeight; y += 2)
		{
			yptr = pYuvBuf + y * nWidth;
			uptr = pYuvBuf + (nWidth * nHeight) + ((y * nWidth) >> 2);
			vptr = uptr + ((nWidth * nHeight) >> 2);

			pRgb = pRgbBuf + y * nWidth * 3;
			for (nn = 0; nn < nWidth; nn += 2)
			{
				int u = uptr[0] - 128;
				int v = vptr[0] - 128;

				int ruv = 90 * v;
				int guv = -46 * v - 22 * u;
				int buv = 113 * u;

				int y00 = *(yptr) << 6;
				*(pRgb + 0) = border_color((y00 + buv) >> 6);
				*(pRgb + 1) = border_color((y00 + guv) >> 6);
				*(pRgb + 2) = border_color((y00 + ruv) >> 6);

				int y01 = *(yptr + 1) << 6;
				*(pRgb + 3) = border_color((y01 + buv) >> 6);
				*(pRgb + 4) = border_color((y01 + guv) >> 6);
				*(pRgb + 5) = border_color((y01 + ruv) >> 6);

				int y10 = *(yptr + nWidth) << 6;
				*(pRgb + nWidth * 3 + 0) = border_color((y10 + buv) >> 6);
				*(pRgb + nWidth * 3 + 1) = border_color((y10 + guv) >> 6);
				*(pRgb + nWidth * 3 + 2) = border_color((y10 + ruv) >> 6);

				int y11 = *(yptr + nWidth + 1) << 6;
				*(pRgb + nWidth * 3 + 3) = border_color((y11 + buv) >> 6);
				*(pRgb + nWidth * 3 + 4) = border_color((y11 + guv) >> 6);
				*(pRgb + nWidth * 3 + 5) = border_color((y11 + ruv) >> 6);

				yptr += 2;
				uptr += 1;
				vptr += 1;
				pRgb += 6;
			}
		}
#endif
		return 0;
	}

	int alg_CutImg(const INPUT_IMG* pSrc, unsigned char* pbCut, rect_i32* pDetRect)
	{
		if (NULL == pSrc || NULL == pbCut)
		{
			return -1;
		}

		if (pSrc->eImgType == IMG_TYPE_YUV420)
		{
			if ((pSrc->ImgW & IMG_ALIGNMENT_BIT) | (pSrc->ImgH & IMG_ALIGNMENT_BIT))
			{
				printf("get \n");
			}
			const unsigned char* pbImgY = (unsigned char*)pSrc->data;
			const unsigned char* pbImgU = (unsigned char*)pSrc->data + pSrc->ImgW * pSrc->ImgH;
			const unsigned char* pbImgV = (unsigned char*)pSrc->data + pSrc->ImgW * pSrc->ImgH * 5 / 4;
			unsigned char* pbCutY = pbCut;
			unsigned char* pbCutU = pbCut + pDetRect->w * pDetRect->h;
			unsigned char* pbCutV = pbCut + pDetRect->w * pDetRect->h * 5 / 4;
			for (int h = 0; h < pDetRect->h; h++)
			{
				memcpy(pbCutY + h * pDetRect->w, pbImgY + (h + pDetRect->y) * pSrc->ImgW + pDetRect->x, pDetRect->w);
			}

			for (int h = 0; h < pDetRect->h / 2; h++)
			{
				memcpy(pbCutU + h * pDetRect->w / 2, pbImgU + (h + pDetRect->y / 2) * (pSrc->ImgW / 2) + pDetRect->x / 2, pDetRect->w / 2);
				memcpy(pbCutV + h * pDetRect->w / 2, pbImgV + (h + pDetRect->y / 2) * (pSrc->ImgW / 2) + pDetRect->x / 2, pDetRect->w / 2);
			}
		}
		else if (pSrc->eImgType == IMG_TYPE_BGRCUT)
		{
			for (int h = 0; h < pDetRect->h; h++)
			{
				memcpy(pbCut + h * pDetRect->w * 3, (unsigned char*)pSrc->data + (h + pDetRect->y) * pSrc->ImgW * 3 + pDetRect->x * 3, pDetRect->w * 3);
			}
		}

		return 0;
	}

	int alg_rgbcut2gray(const unsigned char* pSrc, unsigned char* pDst, unsigned int imgW, unsigned int imgH)
	{
		unsigned char *pdet = pDst;
		const unsigned char* psrc = pSrc;

		for (unsigned int i = 0; i < imgH; i++)
		{
			for (unsigned int j = 0; j < imgW; j++)
			{
				//*pdet = border_color((int)(*(psrc + 0) * 0.299f + *(psrc + 1)*0.587f + *(psrc + 2)*0.114f));
				//(R * 38 + G * 75 + B * 15) >> 7
				*pdet = border_color((*(psrc + 0) * 38 + *(psrc + 1) * 75 + *(psrc + 2) * 15) >> 7 );
				
				pdet++;
				psrc += 3;
			}
		}

		return 0;
	}

	int alg_rgbplane2gray(const unsigned char* pSrc, unsigned char* pDst, unsigned int imgW, unsigned int imgH)
	{
		unsigned char *pdet = pDst;
		const unsigned char* psrcR = pSrc;
		const unsigned char* psrcG = pSrc + imgW * imgH;
		const unsigned char* psrcB = pSrc + imgW * imgH * 2;

		for (unsigned int i = 0; i < imgH; i++)
		{
			for (unsigned int j = 0; j < imgW; j++)
			{
				//*pdet = border_color((int)(*(psrc + 0) * 0.299f + *(psrc + 1)*0.587f + *(psrc + 2)*0.114f));
				//(R * 38 + G * 75 + B * 15) >> 7
				*pdet = border_color((*(psrcR) * 38 + *(psrcG) * 75 + *(psrcB) * 15) >> 7);

				pdet++;
				psrcR++;
				psrcG++;
				psrcB++;
			}
		}

		return 0;
	}
	
	int alg_gray2rgbcut(const unsigned char* pSrc, unsigned char* pDst, unsigned int imgW, unsigned int imgH)
	{
		unsigned char *pdet = pDst;
		const unsigned char* psrc = pSrc;

		for (unsigned int i = 0; i < imgH; i++)
		{
			for (unsigned int j = 0; j < imgW; j++)
			{
				*(pdet + 0) = *psrc;
				*(pdet + 1) = *psrc;
				*(pdet + 2) = *psrc;
				psrc++;
				pdet += 3;
			}
		}

		return 0;
	}

	int alg_bgrcutResize(unsigned char* pSrc, unsigned char* pDst, unsigned int imgSW, unsigned int imgSH, unsigned int imgDW, unsigned int imgDH) //bgr连续存储
	{
		float scale_ratio_h = (float)imgSH / imgDH;
		float scale_ratio_w = (float)imgSW / imgDW;

		for (unsigned int h = 0; h < imgDH; h++)
		{
			for (unsigned int w = 0; w < imgDW; w++)
			{
				unsigned int dst_index = (h * imgDW + w) * 3;
				unsigned int src_index = ((unsigned int)(h * scale_ratio_h) * imgSW + (unsigned int)(w * scale_ratio_w)) * 3;

				*(pDst + dst_index + 0) = *(pSrc + src_index + 0);
				*(pDst + dst_index + 1) = *(pSrc + src_index + 1);
				*(pDst + dst_index + 2) = *(pSrc + src_index + 2);
			}
		}

		return 0;
	}

	void alg_bgrcutResizeV2(unsigned char *pSrc, unsigned char *pDst, int imgSW, int imgSH, unsigned int imgDW, unsigned int imgDH)
	{
		float fRateW = (float)imgSW / imgDW;
		float fRateH = (float)imgSH / imgDH;


		unsigned char bUpLeft, bUpRight, bDownLeft, bDownRight;
		unsigned char gUpLeft, gUpRight, gDownLeft, gDownRight;
		unsigned char rUpLeft, rUpRight, rDownLeft, rDownRight;

		unsigned char b, g, r;
		for (unsigned int i = 0; i < imgDH/*imgSH / fRateH*/; i++)
		{
			float fY = ((float)i) * fRateH;
			int iY = MIN((int)fY, imgSW-2);

			for (unsigned int j = 0; j < imgDW/*imgSW / fRateW*/; j++)
			{
				float fX = ((float)j) * fRateW;
				int iX = (int)fX;

				bUpLeft = pSrc[(iY * imgSW + iX) * 3 + 0];
				bUpRight = pSrc[(iY * imgSW + (iX + 1)) * 3 + 0];

				bDownLeft = pSrc[((iY + 1) * imgSW + iX) * 3 + 0];
				bDownRight = pSrc[((iY + 1) * imgSW + (iX + 1)) * 3 + 0];

				gUpLeft = pSrc[(iY * imgSW + iX) * 3 + 1];
				gUpRight = pSrc[(iY * imgSW + (iX + 1)) * 3 + 1];

				gDownLeft = pSrc[((iY + 1) * imgSW + iX) * 3 + 1];
				gDownRight = pSrc[((iY + 1) * imgSW + (iX + 1)) * 3 + 1];

				rUpLeft = pSrc[(iY * imgSW + iX) * 3 + 2];
				rUpRight = pSrc[(iY * imgSW + (iX + 1)) * 3 + 2];

				rDownLeft = pSrc[((iY + 1) * imgSW + iX) * 3 + 2];
				rDownRight = pSrc[((iY + 1) * imgSW + (iX + 1)) * 3 + 2];

				b = (unsigned char)(bUpLeft * (iX + 1 - fX) * (iY + 1 - fY) + bUpRight * (fX - iX) * (iY + 1 - fY)
					+ bDownLeft * (iX + 1 - fX) * (fY - iY) + bDownRight * (fX - iX) * (fY - iY));

				g = (unsigned char)(gUpLeft * (iX + 1 - fX) * (iY + 1 - fY) + gUpRight * (fX - iX) * (iY + 1 - fY)
					+ gDownLeft * (iX + 1 - fX) * (fY - iY) + gDownRight * (fX - iX) * (fY - iY));

				r = (unsigned char)(rUpLeft * (iX + 1 - fX) * (iY + 1 - fY) + rUpRight * (fX - iX) * (iY + 1 - fY)
					+ rDownLeft * (iX + 1 - fX) * (fY - iY) + rDownRight * (fX - iX) * (fY - iY));

				//if (iY >= 0 && iY <= imgSH * 2 && iX >= 0 && iX <= imgSW * 2)
				{
					pDst[(i * imgDW + j) * 3 + 0] = b;        //B
					pDst[(i * imgDW + j) * 3 + 1] = g;        //G
					pDst[(i * imgDW + j) * 3 + 2] = r;        //R
				}
			}
		}
	}

	int alg_bgrcutCrop(unsigned char* pSrc, unsigned char* pDst, int startX, int startY, int imgSW, int imgSH, int imgDW, int imgDH) //bgr连续存储
	{

		for (int h = 0; h < imgDH; h++)
		{
			int cur_y = h + startY;

			if (cur_y >= 0 && cur_y < imgSH)//y 在数据源范围内
			{
				for (int w = 0; w < imgDW; w++)
				{
					int cur_x = w + startX;
					if (cur_x >= 0 && cur_x < imgSW)// x 在数据源范围内
					{
						int dst_index = (h * imgDW + w) * 3;
						int src_index = (cur_y * imgSW + cur_x) * 3;

						*(pDst + dst_index + 0) = *(pSrc + src_index + 0);
						*(pDst + dst_index + 1) = *(pSrc + src_index + 1);
						*(pDst + dst_index + 2) = *(pSrc + src_index + 2);
					}
				}
			}
		}

		return 0;
	}

	float FastSin(float x)
	{
		#define _a0  0 /*-4.172325e-7f*/   /*(-(float)0x7)/((float)0x1000000); */  
		#define _a1 1.000025f        /*((float)0x1922253)/((float)0x1000000)*2/Pi; */  
		#define _a2 -2.652905e-4f    /*(-(float)0x2ae6)/((float)0x1000000)*4/(Pi*Pi); */  
		#define _a3 -0.165624f       /*(-(float)0xa45511)/((float)0x1000000)*8/(Pi*Pi*Pi); */  
		#define _a4 -1.964532e-3f    /*(-(float)0x30fd3)/((float)0x1000000)*16/(Pi*Pi*Pi*Pi); */  
		#define _a5 1.02575e-2f      /*((float)0x191cac)/((float)0x1000000)*32/(Pi*Pi*Pi*Pi*Pi); */  
		#define _a6 -9.580378e-4f    /*(-(float)0x3af27)/((float)0x1000000)*64/(Pi*Pi*Pi*Pi*Pi*Pi); */  

		return ((((((_a6*(x)+_a5)*(x)+_a4)*(x)+_a3)*(x)+_a2)*(x)+_a1)*(x)+_a0);
	}

	float FastCos(float x)
	{
		#define halfPi ((float)(SF_PI*0.5))  

		return FastSin(halfPi - (x));
	}

	float FastAtan2f(float dy, float dx)
	{
		float ax = ABS(dx);
		float ay = ABS(dy);
		float a = MIN(ax, ay) / (MAX(ax, ay) + (float)DBL_EPSILON_MIN);
		float s = a*a;
		float r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;
		if (ay > ax) r = 1.57079637f - r;
		if (dx < 0) r = 3.14159274f - r;
		if (dy<0) r = -r;
		return r;
	}
	
	float FastSqrtf(float x)
	{
		union {
			int intPart;
			float floatPart;
		} convertor;
		union {
			int intPart;
			float floatPart;
		} convertor2;
		convertor.floatPart = x;
		convertor2.floatPart = x;
		convertor.intPart = 0x1FBCF800 + (convertor.intPart >> 1);
		convertor2.intPart = 0x5f3759df - (convertor2.intPart >> 1);
		return 0.5f*(convertor.floatPart + (x * convertor2.floatPart));
	}

	int FastSqrtI32(int val)
	{
		int r = 0;
		for (int shift = 0; shift<32; shift += 2)
		{
			int x = 0x40000000l >> shift;

			if (x + r <= val)
			{
				val -= x + r;
				r = (r >> 1) | x;
			}
			else
			{
				r = r >> 1;
			}
		}
		//if(r < val) ++r;

		return r;
	}

	float FastInvSqrtf(float x)
	{
		float xhalf = 0.5f * x;
		
		float* Px = &x; //消除gcc的 -Wstrict-alhusing 警告
		size_t i = *(size_t*)Px;
		//size_t i = *(size_t*)&x;
		
		i = 0x5f3759df - (i >> 1);

		size_t* Pi = &i;
		x = *(float*)Pi;
		//x = *(float*)&i;

		x = x*(1.5f - (xhalf*x*x));//牛顿迭代 第一次迭代
		//y  = y * ( threehalfs - ( x2 * y * y ) );   //第二次迭代，可以删除

		return x;
	}

	void GetGaussinRotationM2D(float a[3][3], float b[3], float x[3])
	{
		const int n = 3;
#if 0
		//判断能否用高斯消元法，如果矩阵主对角线上有0元素存在是不能用的
		for (int i = 0; i < n; i++)
		{
			if (a[i][i] == 0.0f)
			{
				fprintf(stderr, "get i=%d, a[i][i]=%f\n", i, a[i][i]);
				return;
			}
		}

		int i = 0, j = 0, k = 0;
		float c[n];//存储初等行变换的系数，用于行的相减

		//消元的整个过程如下，总共n-1次消元过程。
		for (k = 0; k < n - 1; k++)
		{
			//求出第K次初等行变换的系数
			for (i = k + 1; i < n; i++)
			{
				c[i] = a[i][k] / a[k][k];
			}

			//第K次的消元计算
			for (i = k + 1; i < n; i++)
			{
				for (j = 0; j < n; j++)
				{
					a[i][j] = a[i][j] - c[i] * a[k][j];
				}
				b[i] = b[i] - c[i] * b[k];
			}
		}

		//先计算出最后一个未知数
		x[n - 1] = b[n - 1] / a[n - 1][n - 1];
		//求出每个未知数的值
		for (i = n - 2; i >= 0; i--)
		{
			float sum = 0;
			for (j = i + 1; j < n; j++)
			{
				sum += a[i][j] * x[j];
			}
			x[i] = (b[i] - sum) / a[i][i];
		}
#else
		double l[n][n];
		double u[n][n];
		int i, r, k;

		//进行U的第一行的赋值
		for (i = 0; i<n; i++)
		{
			u[0][i] = a[0][i];
		}

		//进行L的第一列的赋值
		for (i = 1; i<n; i++)
		{
			l[i][0] = a[i][0] / u[0][0];
		}

		//计算U的剩下的行数和L的剩下的列数
		for (r = 1; r<n; r++)
		{
			for (i = r; i <n; i++)
			{
				double sum1 = 0;
				for (k = 0; k < r; k++)
				{
					sum1 += l[r][k] * u[k][i];
					//cout << "" << r << "" << sum1 << endl;
				}
				u[r][i] = a[r][i] - sum1;
			}


			if (r != n)
			{
				for (i = r + 1; i < n; i++)
				{
					double sum2 = 0;
					for (k = 0; k < r; k++)
					{
						sum2 += l[i][k] * u[k][r];
					}
					l[i][r] = (a[i][r] - sum2) / u[r][r];
				}
			}
		}

		double y[n] = { 0 };
		y[0] = b[0];
		for (i = 1; i<n; i++)
		{
			double sum3 = 0;
			for (k = 0; k < i; k++)
			{
				sum3 += l[i][k] * y[k];
			}
			y[i] = b[i] - sum3;
		}

		x[n - 1] = (float)(y[n - 1] / u[n - 1][n - 1]);
		for (i = n - 2; i >= 0; i--)
		{
			double sum4 = 0;
			for (k = i + 1; k < n; k++)
			{
				sum4 += u[i][k] * x[k];
			}
			x[i] = (float)((y[i] - sum4) / u[i][i]);
		}
#endif
	}

	void GetRotationM2D(unsigned int x, unsigned int y, float angle, float scale, float* pMatrix)
	{
		angle = angle * 0.0174533f;//pi / 180
		float alpha = FastCos(angle) * scale;
		float beta = FastSin(angle) * scale;

		pMatrix[0] = alpha;
		pMatrix[1] = beta;
		pMatrix[2] = (1 - alpha)*x - beta*y;
		pMatrix[3] = -beta;
		pMatrix[4] = alpha;
		pMatrix[5] = beta*x + (1 - alpha)*y;

		pMatrix[6] = .0f;//便于求逆
		pMatrix[7] = .0f;
		pMatrix[8] = 1.0f;
	}

	void matrix_invert3X3(float* dst, const float* src)
	{
		static const int a[3] = { 2, 2, 1 };
		static const int b[3] = { 1, 0, 0 };
		float det;
		int i, j;

		for (i = 0, det = 0.0f; i < 3; i++) {
			float p;
			int ai = a[i];
			int bi = b[i];
			p = src[i * 3 + 0] * (src[ai * 3 + 2] * src[bi * 3 + 1] -
				src[ai * 3 + 1] * src[bi * 3 + 2]);
			if (i == 1) p = -p;
			det += p;
		}

		if (det == 0.0)
			return ;

		det = 1.0f / det;

		for (j = 0; j < 3; j++) {
			for (i = 0; i < 3; i++) {
				float p;
				int ai = a[i];
				int aj = a[j];
				int bi = b[i];
				int bj = b[j];
				p = (src[ai * 3 + aj] * src[bi * 3 + bj] -
					src[ai * 3 + bj] * src[bi * 3 + aj]);
				if (((i + j) & 1) != 0)
					p = -p;
				dst[j * 3 + i] = det * p;
			}
		}
	}

	int alg_rgbcutRotion(unsigned char* pSrc, unsigned char* pDst, float* pMatrix, unsigned int imgSW, unsigned int imgSH, unsigned int imgDW, unsigned int imgDH)
	{
		if (NULL == pSrc || NULL == pDst || NULL == pMatrix)
		{
			return -1;
		}
		//(v, w) = T'(x, y)
		//T :仿射矩阵,最末一维二维平面图不使用
		//[pMatrix[0], pMatrix[1], pMatrix[2]]
		//[pMatrix[3], pMatrix[4], pMatrix[5]]
		//[pMatrix[6], pMatrix[7], pMatrix[8]]
		int sw = (int)imgSW;
		int sh = (int)imgSH;

		for (unsigned int i = 0; i < imgDH; i++)
		{
			for (unsigned int j = 0; j < imgDW; j++)
			{
				int v = (int)(pMatrix[0] * j + pMatrix[1] * i + pMatrix[2]);//x'
				int w = (int)(pMatrix[3] * j + pMatrix[4] * i + pMatrix[5]);//y'

				if (v >= 0 && v < sw && w >= 0 && w < sh)
				{
					pDst[i*imgDW * 3 + j * 3 + 0] = pSrc[w*imgSW * 3 + v * 3 + 0];
					pDst[i*imgDW * 3 + j * 3 + 1] = pSrc[w*imgSW * 3 + v * 3 + 1];
					pDst[i*imgDW * 3 + j * 3 + 2] = pSrc[w*imgSW * 3 + v * 3 + 2];
				}
				else
				{
					pDst[i*imgDW * 3 + j * 3 + 0] = 0;
					pDst[i*imgDW * 3 + j * 3 + 1] = 0;
					pDst[i*imgDW * 3 + j * 3 + 2] = 0;
				}
			}
		}
		return 0;
	}

#if 0
	static void phash_dct_matrix(const size_t N, float* pMatrix, float* pTransposeMatrix)
	{
		float sq = 1.0f / sqrtf((float)N);
		for (size_t i = 0; i < N; i++)
		{
			pMatrix[0 * N + i] = sq;
			printf("%ff, ", pMatrix[0 * N + i]);
		}
		for (size_t i = 1; i < N; i++)
		{
			for (size_t j = 0; j < N; j++)
			{
				pMatrix[i * N + j] = sqrtf(2.0f / N) * cosf(i * SF_PI * (j + 0.5f) / N);
			}
		}
		for (size_t i = 0; i < N; i++)
		{
			for (size_t j = 0; j < N; j++)
			{
				pTransposeMatrix[i * N + j] = pMatrix[j * N + i];
			}
		}
	}
#else //32*32DCT预建表
	static float Dct32Matrix[32 * 32] = {
		0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f, 0.176777f,
		0.249699f, 0.247294f, 0.242508f, 0.235386f, 0.225997f, 0.214432f, 0.200802f, 0.185238f, 0.167890f, 0.148925f, 0.128526f, 0.106889f, 0.084222f, 0.060745f, 0.036683f, 0.012267f, -0.012267f, -0.036683f, -0.060745f, -0.084222f, -0.106889f, -0.128526f, -0.148925f, -0.167890f, -0.185238f, -0.200802f, -0.214432f, -0.225997f, -0.235386f, -0.242508f, -0.247294f, -0.249699f,
		0.248796f, 0.239235f, 0.220480f, 0.193253f, 0.158598f, 0.117849f, 0.072571f, 0.024504f, -0.024504f, -0.072571f, -0.117849f, -0.158598f, -0.193253f, -0.220480f, -0.239235f, -0.248796f, -0.248796f, -0.239235f, -0.220480f, -0.193253f, -0.158598f, -0.117849f, -0.072571f, -0.024504f, 0.024504f, 0.072571f, 0.117849f, 0.158598f, 0.193253f, 0.220480f, 0.239235f, 0.248796f,
		0.247294f, 0.225997f, 0.185238f, 0.128526f, 0.060745f, -0.012267f, -0.084222f, -0.148925f, -0.200802f, -0.235386f, -0.249699f, -0.242508f, -0.214432f, -0.167890f, -0.106889f, -0.036683f, 0.036683f, 0.106889f, 0.167890f, 0.214432f, 0.242508f, 0.249699f, 0.235386f, 0.200802f, 0.148925f, 0.084223f, 0.012267f, -0.060745f, -0.128526f, -0.185238f, -0.225997f, -0.247294f,
		0.245196f, 0.207867f, 0.138893f, 0.048773f, -0.048773f, -0.138893f, -0.207867f, -0.245196f, -0.245196f, -0.207867f, -0.138893f, -0.048773f, 0.048773f, 0.138893f, 0.207867f, 0.245196f, 0.245196f, 0.207867f, 0.138893f, 0.048773f, -0.048773f, -0.138893f, -0.207867f, -0.245196f, -0.245196f, -0.207867f, -0.138893f, -0.048773f, 0.048773f, 0.138893f, 0.207867f, 0.245196f,
		0.242508f, 0.185238f, 0.084222f, -0.036683f, -0.148925f, -0.225997f, -0.249699f, -0.214432f, -0.128526f, -0.012267f, 0.106889f, 0.200802f, 0.247294f, 0.235386f, 0.167890f, 0.060745f, -0.060745f, -0.167890f, -0.235386f, -0.247294f, -0.200802f, -0.106889f, 0.012267f, 0.128526f, 0.214432f, 0.249699f, 0.225997f, 0.148925f, 0.036682f, -0.084223f, -0.185238f, -0.242508f,
		0.239235f, 0.158598f, 0.024504f, -0.117849f, -0.220480f, -0.248796f, -0.193253f, -0.072571f, 0.072571f, 0.193253f, 0.248796f, 0.220480f, 0.117849f, -0.024504f, -0.158598f, -0.239235f, -0.239235f, -0.158598f, -0.024504f, 0.117849f, 0.220480f, 0.248796f, 0.193253f, 0.072571f, -0.072571f, -0.193253f, -0.248796f, -0.220480f, -0.117849f, 0.024504f, 0.158598f, 0.239235f,
		0.235386f, 0.128526f, -0.036683f, -0.185238f, -0.249699f, -0.200802f, -0.060745f, 0.106889f, 0.225997f, 0.242508f, 0.148925f, -0.012267f, -0.167890f, -0.247294f, -0.214432f, -0.084222f, 0.084223f, 0.214432f, 0.247294f, 0.167890f, 0.012267f, -0.148925f, -0.242508f, -0.225997f, -0.106888f, 0.060745f, 0.200802f, 0.249699f, 0.185237f, 0.036683f, -0.128526f, -0.235386f,
		0.230970f, 0.095671f, -0.095671f, -0.230970f, -0.230970f, -0.095671f, 0.095671f, 0.230970f, 0.230970f, 0.095671f, -0.095671f, -0.230970f, -0.230970f, -0.095671f, 0.095671f, 0.230970f, 0.230970f, 0.095671f, -0.095671f, -0.230970f, -0.230970f, -0.095671f, 0.095671f, 0.230970f, 0.230970f, 0.095671f, -0.095671f, -0.230970f, -0.230970f, -0.095671f, 0.095671f, 0.230970f,
		0.225997f, 0.060745f, -0.148925f, -0.249699f, -0.167890f, 0.036683f, 0.214432f, 0.235386f, 0.084223f, -0.128526f, -0.247294f, -0.185238f, 0.012267f, 0.200802f, 0.242508f, 0.106889f, -0.106889f, -0.242508f, -0.200802f, -0.012267f, 0.185238f, 0.247294f, 0.128526f, -0.084222f, -0.235386f, -0.214432f, -0.036683f, 0.167890f, 0.249699f, 0.148925f, -0.060745f, -0.225997f,
		0.220480f, 0.024504f, -0.193253f, -0.239235f, -0.072571f, 0.158598f, 0.248796f, 0.117849f, -0.117849f, -0.248796f, -0.158598f, 0.072571f, 0.239235f, 0.193253f, -0.024504f, -0.220480f, -0.220480f, -0.024504f, 0.193253f, 0.239235f, 0.072571f, -0.158598f, -0.248796f, -0.117849f, 0.117850f, 0.248796f, 0.158598f, -0.072571f, -0.239235f, -0.193252f, 0.024505f, 0.220481f,
		0.214432f, -0.012267f, -0.225997f, -0.200802f, 0.036683f, 0.235386f, 0.185238f, -0.060745f, -0.242508f, -0.167890f, 0.084223f, 0.247294f, 0.148925f, -0.106889f, -0.249699f, -0.128525f, 0.128526f, 0.249699f, 0.106888f, -0.148925f, -0.247294f, -0.084222f, 0.167890f, 0.242508f, 0.060744f, -0.185238f, -0.235386f, -0.036682f, 0.200802f, 0.225997f, 0.012266f, -0.214432f,
		0.207867f, -0.048773f, -0.245196f, -0.138893f, 0.138893f, 0.245196f, 0.048773f, -0.207867f, -0.207867f, 0.048773f, 0.245196f, 0.138893f, -0.138893f, -0.245196f, -0.048772f, 0.207867f, 0.207867f, -0.048772f, -0.245196f, -0.138893f, 0.138893f, 0.245196f, 0.048772f, -0.207867f, -0.207867f, 0.048772f, 0.245196f, 0.138892f, -0.138893f, -0.245196f, -0.048773f, 0.207867f,
		0.200802f, -0.084222f, -0.249699f, -0.060745f, 0.214432f, 0.185238f, -0.106889f, -0.247294f, -0.036683f, 0.225997f, 0.167890f, -0.128526f, -0.242508f, -0.012267f, 0.235386f, 0.148925f, -0.148925f, -0.235386f, 0.012267f, 0.242508f, 0.128525f, -0.167890f, -0.225997f, 0.036683f, 0.247294f, 0.106889f, -0.185238f, -0.214432f, 0.060745f, 0.249699f, 0.084223f, -0.200802f,
		0.193253f, -0.117849f, -0.239235f, 0.024504f, 0.248796f, 0.072571f, -0.220480f, -0.158598f, 0.158598f, 0.220480f, -0.072571f, -0.248796f, -0.024504f, 0.239235f, 0.117849f, -0.193253f, -0.193252f, 0.117850f, 0.239235f, -0.024505f, -0.248796f, -0.072571f, 0.220481f, 0.158598f, -0.158599f, -0.220480f, 0.072572f, 0.248796f, 0.024503f, -0.239235f, -0.117848f, 0.193253f,
		0.185238f, -0.148925f, -0.214432f, 0.106889f, 0.235386f, -0.060745f, -0.247294f, 0.012267f, 0.249699f, 0.036683f, -0.242508f, -0.084222f, 0.225997f, 0.128526f, -0.200802f, -0.167890f, 0.167890f, 0.200802f, -0.128526f, -0.225997f, 0.084222f, 0.242508f, -0.036682f, -0.249699f, -0.012267f, 0.247294f, 0.060745f, -0.235386f, -0.106889f, 0.214432f, 0.148925f, -0.185238f,
		0.176777f, -0.176777f, -0.176777f, 0.176777f, 0.176777f, -0.176777f, -0.176777f, 0.176777f, 0.176777f, -0.176777f, -0.176777f, 0.176777f, 0.176777f, -0.176777f, -0.176777f, 0.176777f, 0.176777f, -0.176777f, -0.176777f, 0.176777f, 0.176777f, -0.176777f, -0.176777f, 0.176777f, 0.176776f, -0.176777f, -0.176777f, 0.176777f, 0.176777f, -0.176777f, -0.176776f, 0.176777f,
		0.167890f, -0.200802f, -0.128526f, 0.225997f, 0.084222f, -0.242508f, -0.036683f, 0.249699f, -0.012267f, -0.247294f, 0.060745f, 0.235386f, -0.106889f, -0.214432f, 0.148925f, 0.185238f, -0.185238f, -0.148925f, 0.214432f, 0.106889f, -0.235386f, -0.060744f, 0.247294f, 0.012267f, -0.249699f, 0.036683f, 0.242508f, -0.084223f, -0.225997f, 0.128526f, 0.200801f, -0.167891f,
		0.158598f, -0.220480f, -0.072571f, 0.248796f, -0.024504f, -0.239235f, 0.117849f, 0.193253f, -0.193253f, -0.117849f, 0.239235f, 0.024504f, -0.248796f, 0.072571f, 0.220480f, -0.158598f, -0.158598f, 0.220480f, 0.072571f, -0.248796f, 0.024504f, 0.239235f, -0.117849f, -0.193253f, 0.193253f, 0.117849f, -0.239235f, -0.024504f, 0.248796f, -0.072571f, -0.220480f, 0.158599f,
		0.148925f, -0.235386f, -0.012267f, 0.242508f, -0.128526f, -0.167890f, 0.225997f, 0.036682f, -0.247294f, 0.106889f, 0.185238f, -0.214432f, -0.060745f, 0.249699f, -0.084223f, -0.200802f, 0.200802f, 0.084222f, -0.249699f, 0.060745f, 0.214432f, -0.185238f, -0.106889f, 0.247294f, -0.036683f, -0.225997f, 0.167890f, 0.128526f, -0.242508f, 0.012267f, 0.235386f, -0.148925f,
		0.138893f, -0.245196f, 0.048773f, 0.207867f, -0.207867f, -0.048773f, 0.245196f, -0.138893f, -0.138892f, 0.245196f, -0.048773f, -0.207867f, 0.207868f, 0.048772f, -0.245196f, 0.138893f, 0.138892f, -0.245196f, 0.048773f, 0.207867f, -0.207868f, -0.048772f, 0.245196f, -0.138893f, -0.138892f, 0.245196f, -0.048773f, -0.207867f, 0.207868f, 0.048772f, -0.245196f, 0.138893f,
		0.128526f, -0.249699f, 0.106889f, 0.148925f, -0.247294f, 0.084223f, 0.167890f, -0.242508f, 0.060745f, 0.185237f, -0.235386f, 0.036683f, 0.200802f, -0.225998f, 0.012267f, 0.214432f, -0.214432f, -0.012266f, 0.225997f, -0.200802f, -0.036682f, 0.235386f, -0.185238f, -0.060744f, 0.242508f, -0.167891f, -0.084222f, 0.247294f, -0.148925f, -0.106888f, 0.249699f, -0.128526f,
		0.117849f, -0.248796f, 0.158598f, 0.072571f, -0.239235f, 0.193253f, 0.024504f, -0.220480f, 0.220480f, -0.024505f, -0.193252f, 0.239235f, -0.072572f, -0.158598f, 0.248796f, -0.117850f, -0.117849f, 0.248796f, -0.158599f, -0.072570f, 0.239235f, -0.193253f, -0.024503f, 0.220480f, -0.220481f, 0.024505f, 0.193252f, -0.239235f, 0.072572f, 0.158597f, -0.248796f, 0.117850f,
		0.106889f, -0.242508f, 0.200802f, -0.012267f, -0.185238f, 0.247294f, -0.128526f, -0.084222f, 0.235386f, -0.214432f, 0.036683f, 0.167890f, -0.249699f, 0.148925f, 0.060745f, -0.225997f, 0.225997f, -0.060744f, -0.148925f, 0.249699f, -0.167889f, -0.036683f, 0.214432f, -0.235386f, 0.084223f, 0.128526f, -0.247294f, 0.185238f, 0.012267f, -0.200802f, 0.242508f, -0.106888f,
		0.095671f, -0.230970f, 0.230970f, -0.095671f, -0.095671f, 0.230970f, -0.230970f, 0.095671f, 0.095671f, -0.230970f, 0.230970f, -0.095671f, -0.095671f, 0.230970f, -0.230970f, 0.095671f, 0.095671f, -0.230970f, 0.230970f, -0.095671f, -0.095671f, 0.230970f, -0.230970f, 0.095671f, 0.095671f, -0.230970f, 0.230970f, -0.095671f, -0.095671f, 0.230970f, -0.230970f, 0.095670f,
		0.084222f, -0.214432f, 0.247294f, -0.167890f, 0.012267f, 0.148925f, -0.242508f, 0.225997f, -0.106889f, -0.060745f, 0.200802f, -0.249699f, 0.185238f, -0.036683f, -0.128525f, 0.235386f, -0.235386f, 0.128526f, 0.036683f, -0.185237f, 0.249699f, -0.200802f, 0.060745f, 0.106889f, -0.225997f, 0.242508f, -0.148925f, -0.012267f, 0.167890f, -0.247294f, 0.214432f, -0.084222f,
		0.072571f, -0.193253f, 0.248796f, -0.220480f, 0.117849f, 0.024504f, -0.158598f, 0.239235f, -0.239235f, 0.158598f, -0.024505f, -0.117849f, 0.220480f, -0.248796f, 0.193253f, -0.072572f, -0.072571f, 0.193252f, -0.248796f, 0.220480f, -0.117850f, -0.024504f, 0.158598f, -0.239235f, 0.239235f, -0.158599f, 0.024505f, 0.117849f, -0.220480f, 0.248796f, -0.193253f, 0.072571f,
		0.060745f, -0.167890f, 0.235386f, -0.247294f, 0.200802f, -0.106889f, -0.012267f, 0.128525f, -0.214432f, 0.249699f, -0.225997f, 0.148925f, -0.036683f, -0.084222f, 0.185237f, -0.242508f, 0.242508f, -0.185238f, 0.084223f, 0.036682f, -0.148925f, 0.225997f, -0.249699f, 0.214432f, -0.128526f, 0.012267f, 0.106889f, -0.200802f, 0.247294f, -0.235386f, 0.167890f, -0.060745f,
		0.048773f, -0.138893f, 0.207867f, -0.245196f, 0.245196f, -0.207868f, 0.138893f, -0.048773f, -0.048772f, 0.138892f, -0.207867f, 0.245196f, -0.245196f, 0.207868f, -0.138893f, 0.048773f, 0.048772f, -0.138892f, 0.207867f, -0.245196f, 0.245196f, -0.207868f, 0.138893f, -0.048773f, -0.048771f, 0.138892f, -0.207866f, 0.245196f, -0.245197f, 0.207868f, -0.138894f, 0.048773f,
		0.036683f, -0.106889f, 0.167890f, -0.214432f, 0.242508f, -0.249699f, 0.235386f, -0.200802f, 0.148925f, -0.084223f, 0.012267f, 0.060745f, -0.128526f, 0.185238f, -0.225997f, 0.247294f, -0.247294f, 0.225997f, -0.185238f, 0.128526f, -0.060744f, -0.012267f, 0.084223f, -0.148925f, 0.200802f, -0.235386f, 0.249699f, -0.242508f, 0.214432f, -0.167890f, 0.106889f, -0.036683f,
		0.024504f, -0.072571f, 0.117849f, -0.158598f, 0.193253f, -0.220480f, 0.239235f, -0.248796f, 0.248796f, -0.239235f, 0.220480f, -0.193253f, 0.158599f, -0.117849f, 0.072571f, -0.024504f, -0.024504f, 0.072571f, -0.117849f, 0.158599f, -0.193253f, 0.220480f, -0.239235f, 0.248796f, -0.248796f, 0.239235f, -0.220480f, 0.193253f, -0.158599f, 0.117850f, -0.072572f, 0.024505f,
		0.012267f, -0.036683f, 0.060745f, -0.084223f, 0.106889f, -0.128526f, 0.148925f, -0.167890f, 0.185238f, -0.200802f, 0.214432f, -0.225997f, 0.235386f, -0.242508f, 0.247294f, -0.249699f, 0.249699f, -0.247294f, 0.242508f, -0.235386f, 0.225997f, -0.214432f, 0.200802f, -0.185239f, 0.167889f, -0.148925f, 0.128526f, -0.106889f, 0.084223f, -0.060746f, 0.036684f, -0.012268f };
	static float Dct32MatrixT[32 * 32] = {
		0.176777f, 0.249699f, 0.248796f, 0.247294f, 0.245196f, 0.242508f, 0.239235f, 0.235386f, 0.230970f, 0.225997f, 0.220480f, 0.214432f, 0.207867f, 0.200802f, 0.193253f, 0.185238f, 0.176777f, 0.167890f, 0.158598f, 0.148925f, 0.138893f, 0.128526f, 0.117849f, 0.106889f, 0.095671f, 0.084222f, 0.072571f, 0.060745f, 0.048773f, 0.036683f, 0.024504f, 0.012267f,
		0.176777f, 0.247294f, 0.239235f, 0.225997f, 0.207867f, 0.185238f, 0.158598f, 0.128526f, 0.095671f, 0.060745f, 0.024504f, -0.012267f, -0.048773f, -0.084222f, -0.117849f, -0.148925f, -0.176777f, -0.200802f, -0.220480f, -0.235386f, -0.245196f, -0.249699f, -0.248796f, -0.242508f, -0.230970f, -0.214432f, -0.193253f, -0.167890f, -0.138893f, -0.106889f, -0.072571f, -0.036683f,
		0.176777f, 0.242508f, 0.220480f, 0.185238f, 0.138893f, 0.084222f, 0.024504f, -0.036683f, -0.095671f, -0.148925f, -0.193253f, -0.225997f, -0.245196f, -0.249699f, -0.239235f, -0.214432f, -0.176777f, -0.128526f, -0.072571f, -0.012267f, 0.048773f, 0.106889f, 0.158598f, 0.200802f, 0.230970f, 0.247294f, 0.248796f, 0.235386f, 0.207867f, 0.167890f, 0.117849f, 0.060745f,
		0.176777f, 0.235386f, 0.193253f, 0.128526f, 0.048773f, -0.036683f, -0.117849f, -0.185238f, -0.230970f, -0.249699f, -0.239235f, -0.200802f, -0.138893f, -0.060745f, 0.024504f, 0.106889f, 0.176777f, 0.225997f, 0.248796f, 0.242508f, 0.207867f, 0.148925f, 0.072571f, -0.012267f, -0.095671f, -0.167890f, -0.220480f, -0.247294f, -0.245196f, -0.214432f, -0.158598f, -0.084223f,
		0.176777f, 0.225997f, 0.158598f, 0.060745f, -0.048773f, -0.148925f, -0.220480f, -0.249699f, -0.230970f, -0.167890f, -0.072571f, 0.036683f, 0.138893f, 0.214432f, 0.248796f, 0.235386f, 0.176777f, 0.084222f, -0.024504f, -0.128526f, -0.207867f, -0.247294f, -0.239235f, -0.185238f, -0.095671f, 0.012267f, 0.117849f, 0.200802f, 0.245196f, 0.242508f, 0.193253f, 0.106889f,
		0.176777f, 0.214432f, 0.117849f, -0.012267f, -0.138893f, -0.225997f, -0.248796f, -0.200802f, -0.095671f, 0.036683f, 0.158598f, 0.235386f, 0.245196f, 0.185238f, 0.072571f, -0.060745f, -0.176777f, -0.242508f, -0.239235f, -0.167890f, -0.048773f, 0.084223f, 0.193253f, 0.247294f, 0.230970f, 0.148925f, 0.024504f, -0.106889f, -0.207868f, -0.249699f, -0.220480f, -0.128526f,
		0.176777f, 0.200802f, 0.072571f, -0.084222f, -0.207867f, -0.249699f, -0.193253f, -0.060745f, 0.095671f, 0.214432f, 0.248796f, 0.185238f, 0.048773f, -0.106889f, -0.220480f, -0.247294f, -0.176777f, -0.036683f, 0.117849f, 0.225997f, 0.245196f, 0.167890f, 0.024504f, -0.128526f, -0.230970f, -0.242508f, -0.158598f, -0.012267f, 0.138893f, 0.235386f, 0.239235f, 0.148925f,
		0.176777f, 0.185238f, 0.024504f, -0.148925f, -0.245196f, -0.214432f, -0.072571f, 0.106889f, 0.230970f, 0.235386f, 0.117849f, -0.060745f, -0.207867f, -0.247294f, -0.158598f, 0.012267f, 0.176777f, 0.249699f, 0.193253f, 0.036682f, -0.138893f, -0.242508f, -0.220480f, -0.084222f, 0.095671f, 0.225997f, 0.239235f, 0.128525f, -0.048773f, -0.200802f, -0.248796f, -0.167890f,
		0.176777f, 0.167890f, -0.024504f, -0.200802f, -0.245196f, -0.128526f, 0.072571f, 0.225997f, 0.230970f, 0.084223f, -0.117849f, -0.242508f, -0.207867f, -0.036683f, 0.158598f, 0.249699f, 0.176777f, -0.012267f, -0.193253f, -0.247294f, -0.138892f, 0.060745f, 0.220480f, 0.235386f, 0.095671f, -0.106889f, -0.239235f, -0.214432f, -0.048772f, 0.148925f, 0.248796f, 0.185238f,
		0.176777f, 0.148925f, -0.072571f, -0.235386f, -0.207867f, -0.012267f, 0.193253f, 0.242508f, 0.095671f, -0.128526f, -0.248796f, -0.167890f, 0.048773f, 0.225997f, 0.220480f, 0.036683f, -0.176777f, -0.247294f, -0.117849f, 0.106889f, 0.245196f, 0.185237f, -0.024505f, -0.214432f, -0.230970f, -0.060745f, 0.158598f, 0.249699f, 0.138892f, -0.084223f, -0.239235f, -0.200802f,
		0.176777f, 0.128526f, -0.117849f, -0.249699f, -0.138893f, 0.106889f, 0.248796f, 0.148925f, -0.095671f, -0.247294f, -0.158598f, 0.084223f, 0.245196f, 0.167890f, -0.072571f, -0.242508f, -0.176777f, 0.060745f, 0.239235f, 0.185238f, -0.048773f, -0.235386f, -0.193252f, 0.036683f, 0.230970f, 0.200802f, -0.024505f, -0.225997f, -0.207867f, 0.012267f, 0.220480f, 0.214432f,
		0.176777f, 0.106889f, -0.158598f, -0.242508f, -0.048773f, 0.200802f, 0.220480f, -0.012267f, -0.230970f, -0.185238f, 0.072571f, 0.247294f, 0.138893f, -0.128526f, -0.248796f, -0.084222f, 0.176777f, 0.235386f, 0.024504f, -0.214432f, -0.207867f, 0.036683f, 0.239235f, 0.167890f, -0.095671f, -0.249699f, -0.117849f, 0.148925f, 0.245196f, 0.060745f, -0.193253f, -0.225997f,
		0.176777f, 0.084222f, -0.193253f, -0.214432f, 0.048773f, 0.247294f, 0.117849f, -0.167890f, -0.230970f, 0.012267f, 0.239235f, 0.148925f, -0.138893f, -0.242508f, -0.024504f, 0.225997f, 0.176777f, -0.106889f, -0.248796f, -0.060745f, 0.207868f, 0.200802f, -0.072572f, -0.249699f, -0.095671f, 0.185238f, 0.220480f, -0.036683f, -0.245196f, -0.128526f, 0.158599f, 0.235386f,
		0.176777f, 0.060745f, -0.220480f, -0.167890f, 0.138893f, 0.235386f, -0.024504f, -0.247294f, -0.095671f, 0.200802f, 0.193253f, -0.106889f, -0.245196f, -0.012267f, 0.239235f, 0.128526f, -0.176777f, -0.214432f, 0.072571f, 0.249699f, 0.048772f, -0.225998f, -0.158598f, 0.148925f, 0.230970f, -0.036683f, -0.248796f, -0.084222f, 0.207868f, 0.185238f, -0.117849f, -0.242508f,
		0.176777f, 0.036683f, -0.239235f, -0.106889f, 0.207867f, 0.167890f, -0.158598f, -0.214432f, 0.095671f, 0.242508f, -0.024504f, -0.249699f, -0.048772f, 0.235386f, 0.117849f, -0.200802f, -0.176777f, 0.148925f, 0.220480f, -0.084223f, -0.245196f, 0.012267f, 0.248796f, 0.060745f, -0.230970f, -0.128525f, 0.193253f, 0.185237f, -0.138893f, -0.225997f, 0.072571f, 0.247294f,
		0.176777f, 0.012267f, -0.248796f, -0.036683f, 0.245196f, 0.060745f, -0.239235f, -0.084222f, 0.230970f, 0.106889f, -0.220480f, -0.128525f, 0.207867f, 0.148925f, -0.193253f, -0.167890f, 0.176777f, 0.185238f, -0.158598f, -0.200802f, 0.138893f, 0.214432f, -0.117850f, -0.225997f, 0.095671f, 0.235386f, -0.072572f, -0.242508f, 0.048773f, 0.247294f, -0.024504f, -0.249699f,
		0.176777f, -0.012267f, -0.248796f, 0.036683f, 0.245196f, -0.060745f, -0.239235f, 0.084223f, 0.230970f, -0.106889f, -0.220480f, 0.128526f, 0.207867f, -0.148925f, -0.193252f, 0.167890f, 0.176777f, -0.185238f, -0.158598f, 0.200802f, 0.138892f, -0.214432f, -0.117849f, 0.225997f, 0.095671f, -0.235386f, -0.072571f, 0.242508f, 0.048772f, -0.247294f, -0.024504f, 0.249699f,
		0.176777f, -0.036683f, -0.239235f, 0.106889f, 0.207867f, -0.167890f, -0.158598f, 0.214432f, 0.095671f, -0.242508f, -0.024504f, 0.249699f, -0.048772f, -0.235386f, 0.117850f, 0.200802f, -0.176777f, -0.148925f, 0.220480f, 0.084222f, -0.245196f, -0.012266f, 0.248796f, -0.060744f, -0.230970f, 0.128526f, 0.193252f, -0.185238f, -0.138892f, 0.225997f, 0.072571f, -0.247294f,
		0.176777f, -0.060745f, -0.220480f, 0.167890f, 0.138893f, -0.235386f, -0.024504f, 0.247294f, -0.095671f, -0.200802f, 0.193253f, 0.106888f, -0.245196f, 0.012267f, 0.239235f, -0.128526f, -0.176777f, 0.214432f, 0.072571f, -0.249699f, 0.048773f, 0.225997f, -0.158599f, -0.148925f, 0.230970f, 0.036683f, -0.248796f, 0.084223f, 0.207867f, -0.185238f, -0.117849f, 0.242508f,
		0.176777f, -0.084222f, -0.193253f, 0.214432f, 0.048773f, -0.247294f, 0.117849f, 0.167890f, -0.230970f, -0.012267f, 0.239235f, -0.148925f, -0.138893f, 0.242508f, -0.024505f, -0.225997f, 0.176777f, 0.106889f, -0.248796f, 0.060745f, 0.207867f, -0.200802f, -0.072570f, 0.249699f, -0.095671f, -0.185237f, 0.220480f, 0.036682f, -0.245196f, 0.128526f, 0.158599f, -0.235386f,
		0.176777f, -0.106889f, -0.158598f, 0.242508f, -0.048773f, -0.200802f, 0.220480f, 0.012267f, -0.230970f, 0.185238f, 0.072571f, -0.247294f, 0.138893f, 0.128525f, -0.248796f, 0.084222f, 0.176777f, -0.235386f, 0.024504f, 0.214432f, -0.207868f, -0.036682f, 0.239235f, -0.167889f, -0.095671f, 0.249699f, -0.117850f, -0.148925f, 0.245196f, -0.060744f, -0.193253f, 0.225997f,
		0.176777f, -0.128526f, -0.117849f, 0.249699f, -0.138893f, -0.106889f, 0.248796f, -0.148925f, -0.095671f, 0.247294f, -0.158598f, -0.084222f, 0.245196f, -0.167890f, -0.072571f, 0.242508f, -0.176777f, -0.060744f, 0.239235f, -0.185238f, -0.048772f, 0.235386f, -0.193253f, -0.036683f, 0.230970f, -0.200802f, -0.024504f, 0.225997f, -0.207868f, -0.012267f, 0.220480f, -0.214432f,
		0.176777f, -0.148925f, -0.072571f, 0.235386f, -0.207867f, 0.012267f, 0.193253f, -0.242508f, 0.095671f, 0.128526f, -0.248796f, 0.167890f, 0.048772f, -0.225997f, 0.220481f, -0.036682f, -0.176777f, 0.247294f, -0.117849f, -0.106889f, 0.245196f, -0.185238f, -0.024503f, 0.214432f, -0.230970f, 0.060745f, 0.158598f, -0.249699f, 0.138893f, 0.084223f, -0.239235f, 0.200802f,
		0.176777f, -0.167890f, -0.024504f, 0.200802f, -0.245196f, 0.128526f, 0.072571f, -0.225997f, 0.230970f, -0.084222f, -0.117849f, 0.242508f, -0.207867f, 0.036683f, 0.158598f, -0.249699f, 0.176777f, 0.012267f, -0.193253f, 0.247294f, -0.138893f, -0.060744f, 0.220480f, -0.235386f, 0.095671f, 0.106889f, -0.239235f, 0.214432f, -0.048773f, -0.148925f, 0.248796f, -0.185239f,
		0.176777f, -0.185238f, 0.024504f, 0.148925f, -0.245196f, 0.214432f, -0.072571f, -0.106888f, 0.230970f, -0.235386f, 0.117850f, 0.060744f, -0.207867f, 0.247294f, -0.158599f, -0.012267f, 0.176776f, -0.249699f, 0.193253f, -0.036683f, -0.138892f, 0.242508f, -0.220481f, 0.084223f, 0.095671f, -0.225997f, 0.239235f, -0.128526f, -0.048771f, 0.200802f, -0.248796f, 0.167889f,
		0.176777f, -0.200802f, 0.072571f, 0.084223f, -0.207867f, 0.249699f, -0.193253f, 0.060745f, 0.095671f, -0.214432f, 0.248796f, -0.185238f, 0.048772f, 0.106889f, -0.220480f, 0.247294f, -0.176777f, 0.036683f, 0.117849f, -0.225997f, 0.245196f, -0.167891f, 0.024505f, 0.128526f, -0.230970f, 0.242508f, -0.158599f, 0.012267f, 0.138892f, -0.235386f, 0.239235f, -0.148925f,
		0.176777f, -0.214432f, 0.117849f, 0.012267f, -0.138893f, 0.225997f, -0.248796f, 0.200802f, -0.095671f, -0.036683f, 0.158598f, -0.235386f, 0.245196f, -0.185238f, 0.072572f, 0.060745f, -0.176777f, 0.242508f, -0.239235f, 0.167890f, -0.048773f, -0.084222f, 0.193252f, -0.247294f, 0.230970f, -0.148925f, 0.024505f, 0.106889f, -0.207866f, 0.249699f, -0.220480f, 0.128526f,
		0.176777f, -0.225997f, 0.158598f, -0.060745f, -0.048773f, 0.148925f, -0.220480f, 0.249699f, -0.230970f, 0.167890f, -0.072571f, -0.036682f, 0.138892f, -0.214432f, 0.248796f, -0.235386f, 0.176777f, -0.084223f, -0.024504f, 0.128526f, -0.207867f, 0.247294f, -0.239235f, 0.185238f, -0.095671f, -0.012267f, 0.117849f, -0.200802f, 0.245196f, -0.242508f, 0.193253f, -0.106889f,
		0.176777f, -0.235386f, 0.193253f, -0.128526f, 0.048773f, 0.036682f, -0.117849f, 0.185237f, -0.230970f, 0.249699f, -0.239235f, 0.200802f, -0.138893f, 0.060745f, 0.024503f, -0.106889f, 0.176777f, -0.225997f, 0.248796f, -0.242508f, 0.207868f, -0.148925f, 0.072572f, 0.012267f, -0.095671f, 0.167890f, -0.220480f, 0.247294f, -0.245197f, 0.214432f, -0.158599f, 0.084223f,
		0.176777f, -0.242508f, 0.220480f, -0.185238f, 0.138893f, -0.084223f, 0.024504f, 0.036683f, -0.095671f, 0.148925f, -0.193252f, 0.225997f, -0.245196f, 0.249699f, -0.239235f, 0.214432f, -0.176777f, 0.128526f, -0.072571f, 0.012267f, 0.048772f, -0.106888f, 0.158597f, -0.200802f, 0.230970f, -0.247294f, 0.248796f, -0.235386f, 0.207868f, -0.167890f, 0.117850f, -0.060746f,
		0.176777f, -0.247294f, 0.239235f, -0.225997f, 0.207867f, -0.185238f, 0.158598f, -0.128526f, 0.095671f, -0.060745f, 0.024505f, 0.012266f, -0.048773f, 0.084223f, -0.117848f, 0.148925f, -0.176776f, 0.200801f, -0.220480f, 0.235386f, -0.245196f, 0.249699f, -0.248796f, 0.242508f, -0.230970f, 0.214432f, -0.193253f, 0.167890f, -0.138894f, 0.106889f, -0.072572f, 0.036684f,
		0.176777f, -0.249699f, 0.248796f, -0.247294f, 0.245196f, -0.242508f, 0.239235f, -0.235386f, 0.230970f, -0.225997f, 0.220481f, -0.214432f, 0.207867f, -0.200802f, 0.193253f, -0.185238f, 0.176777f, -0.167891f, 0.158599f, -0.148925f, 0.138893f, -0.128526f, 0.117850f, -0.106888f, 0.095670f, -0.084222f, 0.072571f, -0.060745f, 0.048773f, -0.036683f, 0.024505f, -0.012268f };

#endif
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
	static __always_inline float32x2_t dotProduct(float32x4x2_t input1, float32x4x2_t input2)
	{
		float32x2_t d0, d1;
		float32x4_t q0;
		input1.val[0] = vmulq_f32(input1.val[0], input2.val[0]);
		input1.val[1] = vmulq_f32(input1.val[1], input2.val[1]);

		q0 = vaddq_f32(input1.val[0], input1.val[1]);
		d0 = vget_low_f32(q0);
		d1 = vget_high_f32(q0);
		d0 = vpadd_f32(d0, d1);
		d0 = vpadd_f32(d0, d1);
		return d0;
	}

	static void phash_matrix_multiply(const size_t N, float* pMatrixA, float* pMatrixB, float* pMatrixOut)
	{
		float32x4x4_t line01, line23, line45, line67;
		float32x4x2_t b[8], *pA, *pB, temp;
		float32x2x4_t result;
		size_t  i = N;

		// vld4 for easier transpose 
		line01 = vld4q_f32(pMatrixB++);
		line23 = vld4q_f32(pMatrixB++);
		line45 = vld4q_f32(pMatrixB++);
		line67 = vld4q_f32(pMatrixB);

		// transpose MatB 
		vuzpq_f32(line01.val[0], line45.val[0]);
		vuzpq_f32(line01.val[1], line45.val[1]);
		vuzpq_f32(line01.val[2], line45.val[2]);
		vuzpq_f32(line01.val[3], line45.val[3]);

		vuzpq_f32(line23.val[0], line67.val[0]);
		vuzpq_f32(line23.val[1], line67.val[1]);
		vuzpq_f32(line23.val[2], line67.val[2]);
		vuzpq_f32(line23.val[3], line67.val[3]);

		// store MatB to stack 
		b[0].val[0] = line01.val[0];
		b[0].val[1] = line01.val[1];
		b[1].val[0] = line01.val[2];
		b[1].val[1] = line01.val[3];
		b[2].val[0] = line23.val[0];
		b[2].val[1] = line23.val[1];
		b[3].val[0] = line23.val[2];
		b[3].val[1] = line23.val[3];

		b[4].val[0] = line45.val[0];
		b[4].val[1] = line45.val[1];
		b[5].val[0] = line45.val[2];
		b[5].val[1] = line45.val[3];
		b[6].val[0] = line67.val[0];
		b[6].val[1] = line67.val[1];
		b[7].val[0] = line67.val[2];
		b[7].val[1] = line67.val[3];

		pA = (float32x4x2_t *)pMatrixA;
		do
		{
			// just the right amount of data for aarch32 NEON register bank size 
			pB = b;
			temp = *pA++;
			result.val[0] = dotProduct(*pB++, temp);
			result.val[1] = dotProduct(*pB++, temp);
			result.val[2] = dotProduct(*pB++, temp);
			result.val[3] = dotProduct(*pB++, temp);
			vst4_lane_f32(pMatrixOut++, result, 0);

			result.val[0] = dotProduct(*pB++, temp);
			result.val[1] = dotProduct(*pB++, temp);
			result.val[2] = dotProduct(*pB++, temp);
			result.val[3] = dotProduct(*pB, temp);
			vst4_lane_f32(pMatrixOut++, result, 0);
		} while (--i);
	}
#else
	static void phash_matrix_multiply(const size_t N, float* pMatrixA, float* pMatrixB, float* pMatrixOut)
	{

		float t = 0.0f;
		for (size_t i = 0; i < N; i++)
		{
			for (size_t j = 0; j < N; j++)
			{
				t = 0.0f;
				for (size_t k = 0; k < N; k++)
				{
					t += pMatrixA[i * N + k] * pMatrixB[k * N + j];
				}
				pMatrixOut[i * N + j] = t;
			}
		}
	}
#endif

	ulong64 algGetPhash(const INPUT_IMG* pSrc, const rect_i32* pSrcRoi)
	{
#define DCTSIZE (32)
#define SUBSIZE (8)

		if (NULL == pSrc || NULL == pSrc->data)
		{
			fprintf(stderr, "get ptr is null\n");
			return 0;
		}
		const unsigned char* pbImgY = (unsigned char*)pSrc->data + pSrc->ImgW * pSrcRoi->y + pSrcRoi->x;
		float* pCutY = (float*)alg_malloc(DCTSIZE * DCTSIZE * sizeof(float));

		float scale_ratio_h = (float)pSrcRoi->h / DCTSIZE;
		float scale_ratio_w = (float)pSrcRoi->w / DCTSIZE;

		unsigned int DetTmp = 0;
		unsigned int SrcTmp = 0;
		unsigned int HWTmp = 0;
		unsigned int HHTmp = 0;

		for (unsigned int h = 0; h < DCTSIZE; h++)
		{
			HWTmp = (unsigned int)(h * scale_ratio_h) * pSrc->ImgW;
			HHTmp = h * DCTSIZE;

			for (unsigned int w = 0; w < DCTSIZE; w++)
			{
				DetTmp = HHTmp + w;
				SrcTmp = HWTmp + (unsigned int)(w * scale_ratio_w);

				*(pCutY + DetTmp) = *(pbImgY + SrcTmp);
			}
		}

		float* pMatrix = Dct32Matrix;
		float* pTransposeMatrix = Dct32MatrixT;
		float* pMatrixTmp = (float*)alg_malloc(DCTSIZE * DCTSIZE * sizeof(float));

		phash_matrix_multiply(DCTSIZE, pMatrix, pCutY, pMatrixTmp);//[F(u,v)]=[A] [f(x,y)] [A]T 
		phash_matrix_multiply(DCTSIZE, pMatrixTmp, pTransposeMatrix, pCutY);

		float avgSum = 0.0f;
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
		float32x4_t SumTmp = vdupq_n_f32(0.0f);
		for (size_t i = 0; i < SUBSIZE; i++)
		{
			float32x4_t _k0 = vld1q_f32(pCutY + i*DCTSIZE);
			float32x4_t _k1 = vld1q_f32(pCutY + i*DCTSIZE + 4);
			SumTmp = vaddq_f32(SumTmp, vaddq_f32(_k0, _k1));
		}
		float sum[4];
		vst1q_f32(sum, SumTmp);
		avgSum = sum[0] + sum[1] + sum[2] + sum[3];
#else
		for (size_t i = 0; i < SUBSIZE; i++)
		{
			for (size_t j = 0; j < SUBSIZE; j++)
			{
				avgSum += pCutY[i * DCTSIZE + j];
			}
		}
#endif
		avgSum = avgSum / 64.0f;
		ulong64 one = 0x0000000000000001ULL;
		ulong64 hash = 0x0000000000000000ULL;

		for (size_t i = 0; i < SUBSIZE; i++)
		{
			for (size_t j = 0; j < SUBSIZE; j++)
			{
				float current = pCutY[i * DCTSIZE + j];
				if (current > avgSum)
				{
					hash |= one;
				}
				one = one << 1;
			}
		}

		alg_free(pCutY);
		alg_free(pMatrixTmp);

#undef DCTSIZE
#undef SUBSIZE

		return hash;
	}

	ulong64 algGetDhash(const INPUT_IMG* pSrc, const rect_i32* pSrcRoi)
	{
#define SUBSIZE (8)

		if (NULL == pSrc || NULL == pSrc->data)
		{
			fprintf(stderr, "get ptr is null\n");
			return 0;
		}
		const unsigned char* pbImgY = (unsigned char*)pSrc->data + pSrc->ImgW * pSrcRoi->y + pSrcRoi->x;

		float scale_ratio_h = (float)pSrcRoi->h / SUBSIZE;
		float scale_ratio_w = (float)pSrcRoi->w / (SUBSIZE + 1);

		ulong64 one = 0x0000000000000001ULL;
		ulong64 hash = 0x0000000000000000ULL;

		unsigned char last = 0;
		unsigned char now = 0;
		for (unsigned int h = 0; h < SUBSIZE; h++)
		{
			const unsigned char* pY = (unsigned char*)(pbImgY + (unsigned int)(h * scale_ratio_h) * pSrc->ImgW);
			last = *pY;
			for (unsigned int w = 1; w < SUBSIZE + 1; w++)
			{
				now = *(pY + (unsigned int)(w * scale_ratio_w));
				if (now > last)
				{
					hash |= one;
				}
				one = one << 1;

				last = now;
			}
		}

#undef SUBSIZE

		return hash;
	}

	int algGetHashDistance(const ulong64 hash1, const ulong64 hash2)
	{
		ulong64 x = hash1^hash2;
		const ulong64 m1 = 0x5555555555555555ULL;
		const ulong64 m2 = 0x3333333333333333ULL;
		const ulong64 h01 = 0x0101010101010101ULL;
		const ulong64 m4 = 0x0f0f0f0f0f0f0f0fULL;
		x -= (x >> 1) & m1;
		x = (x & m2) + ((x >> 2) & m2);
		x = (x + (x >> 4)) & m4;
		return (x * h01) >> 56;
	}

	float algGetCosDistance(const float* v1, const float* v2, const unsigned int Size)
	{
		unsigned int i = 0;
		float sum = 0.0f;
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
		unsigned int n4 = Size & ~(0x3u);

		float32x4_t sum_v1v2 = vdupq_n_f32(0);
		float32x4_t left_vec, right_vec;

		for (; i < n4; i += 4)
		{
			left_vec = vld1q_f32(v1 + i);
			right_vec = vld1q_f32(v2 + i);
			sum_v1v2 = vmlaq_f32(sum_v1v2, left_vec, right_vec);
		}

		float32x2_t r = vadd_f32(vget_high_f32(sum_v1v2), vget_low_f32(sum_v1v2));
		sum += vget_lane_f32(vpadd_f32(r, r), 0);
#endif

		for (; i < Size; i++)
		{
			sum += (v1[i] * v2[i]);
		}

		sum = MAX(0.0f, sum);
		//sum = (sum / sqrt(mod1) / sqrt(mod2) + 1) / 2.0f;//cos距离范围[-1,1]
		//sum = ABS((sum * FastInvSqrtf(mod1) * FastInvSqrtf(mod2) + 1) * 0.5f);
		//sum = (sum - 0.5f) * 2.0f;
		//printf("get sum:%f\n", sum);

		return sum;
	}

	int algGetFaceScore(const INPUT_IMG* pSrc, const rect_i32* pSrcRoi, const float* pDetPoint)
	{
#define SizeDef (20)

		if (NULL == pSrc || NULL == pSrc->data)
		{
			fprintf(stderr, "get ptr is null\n");
			return 0;
		}
		const unsigned char* pbImgY = (unsigned char*)pSrc->data + pSrc->ImgW * pSrcRoi->y + pSrcRoi->x;
		short* pCutY = (short*)alg_malloc(SizeDef * SizeDef * sizeof(short));

		float scale_ratio_h = (float)pSrcRoi->h / SizeDef;
		float scale_ratio_w = (float)pSrcRoi->w / SizeDef;

		unsigned int DetTmp = 0;
		unsigned int SrcTmp = 0;
		unsigned int HWTmp = 0;
		unsigned int HHTmp = 0;

		for (unsigned int h = 0; h < SizeDef; h++)
		{
			HWTmp = (unsigned int)(h * scale_ratio_h) * pSrc->ImgW;
			HHTmp = h * SizeDef;

			for (unsigned int w = 0; w < SizeDef; w++)
			{
				DetTmp = HHTmp + w;
				SrcTmp = HWTmp + (unsigned int)(w * scale_ratio_w);

				*(pCutY + DetTmp) = *(pbImgY + SrcTmp);
			}
		}

		int dxy = 0;
		for (unsigned int h = 1; h < SizeDef - 1; h++)
		{
			for (unsigned int w = 1; w < SizeDef - 1; w++)
			{
				short* pY = pCutY + h * SizeDef + w;

				short gx = *(pY - SizeDef - 1) - *(pY - SizeDef + 1) + \
					      (*(pY           - 1) - *(pY           + 1)) * 2 + \
					       *(pY + SizeDef - 1) - *(pY + SizeDef + 1);

				short gy = (*(pY - SizeDef - 1) + *(pY - SizeDef) * 2 + *(pY - SizeDef + 1)) - \
					       (*(pY + SizeDef - 1) + *(pY + SizeDef) * 2 + *(pY + SizeDef + 1));

				//dxy += FastSqrtI32(gx*gx + gy*gy);
				dxy += (ABS(gx) + ABS(gy));

				/*short gx = (*(pY - SizeDef - 1) - *(pY - SizeDef + 1)) * 3 + \
					       (*(pY           - 1) - *(pY           + 1)) * 10 + \
					       (*(pY + SizeDef - 1) - *(pY + SizeDef + 1)) * 3;

				short gy = (*(pY - SizeDef - 1) * 3 + *(pY - SizeDef) * 10 + *(pY - SizeDef + 1) * 3) - \
					       (*(pY + SizeDef - 1) * 3 + *(pY + SizeDef) * 10 + *(pY + SizeDef + 1) * 3);

				dxy += (ABS(gx) + ABS(gy));*/
			}
		}
		alg_free(pCutY);
		float sobel = (float)dxy / ((SizeDef - 2)*(SizeDef - 2)) / 255;

#undef SizeDef

		float eyeX = pDetPoint[1 + 0] - pDetPoint[0 + 0];
		float eyeY = pDetPoint[1 + 5] - pDetPoint[0 + 5];
		float monX = pDetPoint[4 + 0] - pDetPoint[3 + 0];
		float monY = pDetPoint[4 + 5] - pDetPoint[3 + 5];

		float angleEye = (0 == eyeX ? .0f : FastAtan2f(eyeY, eyeX)) * 57.29578f;
		float angleMon = (0 == monX ? .0f : FastAtan2f(monY, monX)) * 57.29578f;
		float angle = (angleEye + angleMon) * 0.5f;

		float position = (float)(pSrcRoi->y + pSrcRoi->h / 2) / pSrc->ImgH;
		if (position < 0.25f)
		{
			position = 0.1f;
		}
		else if (position > 0.75f)
		{
			position = 0.5f;
		}

		int score = (int)(MIN(sobel * 30, 30) + MIN((90 - ABS(angle)) * .5f, 40) + MIN(position * 40, 30));
		return score;
	}

#ifdef __cplusplus
};
#endif