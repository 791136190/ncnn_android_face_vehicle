/***********************************************************************
* 说明:用于提供统一算法调用结构体和枚举参数等
*      不使用可变参数，不使用平台相关长度数据申明
*
* 作者:研发部->算法组
************************************************************************/

#ifndef HU_ALG_COMMMON_H_
#define HU_ALG_COMMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)  
#ifdef HU_COMPILE_LIBRARY
#define ALG_STD_CALL __declspec(dllexport)
#else
#define ALG_STD_CALL __declspec(dllimport)
#endif
#elif defined(__linux__) || defined(__linux)  
#define ALG_STD_CALL __attribute__((visibility("default")))
#else
#define ALG_STD_CALL
#endif

/**********************通用自错误码字************************/
#define ALG_OK         (0)
#define ALG_PTR_NULL  (-1)
#define ALG_PARAM_ERR (-2)
#define ALG_ERR_OTHER (-3)
/************************************************************/

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MID 
#define MID(a, b, c) ((a)<(b) ? ((b)<(c)?(b):(a)<(c)?(c):(a)) : ((b)>(c)?(b):(a)>(c)?(c):(a)))
#endif

#ifndef ABS
#define ABS(a) ((a) < (0) ? -(a) : (a))
#endif

#ifndef ABS_OPT
#define ABS_OPT(a, b) (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#endif

#ifdef NULL
#undef NULL
#endif

#ifdef __cplusplus
#define NULL (0)
#else
#define NULL ((void *)0)
#endif

#ifdef TRUE
#undef TRUE 
#endif
#define TRUE (1)

#ifdef FALSE
#undef FALSE
#endif
#define FALSE (0)

#define SF_PI             (3.14159265359f)
#define DBL_EPSILON_MIN   (2.2204460492503131e-016)
#define FLT_EPSILON_EPS   (1.192092896e-07f)
#define FLT_DIV_EPS       (1.0e-12f)
#define KEY_POINT_NUM	  (5u)
#define IMG_ALIGNMENT_BIT (0x1u)
#define TRACK_EMB_SIZE    (64u)

typedef unsigned long long ulong64;

typedef enum _IMG_TYPE_
{
	IMG_TYPE_YUV420   = 1,//支持NV12格式u在v前
	IMG_TYPE_BGRPLANE = 2,//b0,b1...bn, g0,g1...gn, r0 r1...rn 颜色排列按照opencv的模式
	IMG_TYPE_BGRCUT   = 3,//b0 g0 r0, b1 g1 r1..., bn gn rn
	IMG_TYPE_JPEG_FILE= 4,//直接输入jpeg文件,文件名用INPUT_IMG->data传入
	IMG_TYPE_JPEG_MEM = 5,//直接输入jpeg内存数据
}IMG_TYPE;

typedef struct _INPUT_IMG_
{
	IMG_TYPE       eImgType;//数据类型
	unsigned int   Ch;		//使用的通道数1(yuv的亮度信息)或者3(彩色)
	unsigned int   Ts;		//输入帧的时间戳
	unsigned int   FrameNum;//输入帧的帧号
	unsigned int   ImgW;	//输入帧的宽-要求对齐到 IMG_ALIGNMENT_BIT+1，否则直接返回错误
	unsigned int   ImgH;	//输入帧的高-要求对齐到 IMG_ALIGNMENT_BIT+1，否则直接返回错误
	unsigned int   ImgSize; //输入数据的字节数，数据连续存储
	void*          data;	//数据首地址,yuv或者bgr需要使用连续内存传入，yuv的patch和w需要保持一致，要求16对齐
}INPUT_IMG;

typedef enum _TRIGGER_TYPE_
{
	TRIGGER_NO = 0,   //没有触发信息
	TRIGGER_IN = 1,   //车辆进入
	TRIGGER_OUT = 2,  //车辆驶出
}TRIGGER_TYPE;

typedef enum _SIZE_MODE_
{
	MODEL_SMALL  = 1, //小模式
	MODEL_NORMAL = 2, //中模式
	MODEL_BIG    = 3, //大模式
}SIZE_MODE;

typedef struct _point_i32_ //点坐标描述-32位带符号
{
	int x;
	int y;
}point_i32;

typedef struct _point_f32_ //点坐标描述-32位浮点
{
	float x;
	float y;
}point_f32;

typedef struct _rect_i32_ //左上角加宽高-32位带符号
{
	int x;
	int y;
	int w;
	int h;
}rect_i32;

typedef struct _half_rect_i32_ //中间加半宽高-32位带符号
{
	int centerX;
	int centerY;
	int halfW;
	int halfH;
}half_rect_i32;

typedef struct _rect_p_u16 //两点指定-16位无符号
{
	unsigned short x0;
	unsigned short y0;
	unsigned short x1;
	unsigned short y1;
}rect_p_u16;

typedef struct _face_rect_ //左上角加宽高-32位带符号
{
	rect_i32 rect;
	float Point[KEY_POINT_NUM * 2];//目标关键点
}face_rect;

typedef struct _rect_f32_ //左上角加宽高-32位浮点
{
	float x;
	float y;
	float w;
	float h;
}rect_f32;

typedef struct _line_i32_ //线段描述-32位带符号
{
	point_i32 start;
	point_i32 end;
}line_i32;

typedef struct _line_f32_ //线段描述-32位浮点
{
	point_f32 start;
	point_f32 end;
}line_f32;

typedef struct _size_img_ //大小描述-32位无符号
{
	unsigned int Width;
	unsigned int Height;
}size_img;

typedef struct _target_out_s_ //标准目标输出格式
{
	unsigned int Id;		//目标id
	unsigned int Ts;		//目标检测帧的时间戳
	unsigned int FrameNum;  //目标检测帧
	unsigned int Score;		//目标分数0-100
	unsigned int Update;	//本次是否比之前的目标更可靠-如果算法输出1时，不管score是否比之前的大，都可以更新
	unsigned int Visible;	//目标是否可靠，命中屏蔽区域或者为算法预估值时，该标志为0;其它时候为1
	unsigned int EndFlag;	//目标结束标志-有跟踪时才有意义
	float Point[KEY_POINT_NUM * 2];//目标关键点
	rect_f32     rect;		//目标区域，对外输出时使用归一化
	float Emb[TRACK_EMB_SIZE];//目标特征编码位
	unsigned char Res[8];	//预留位
}target_out_s;

#ifdef __cplusplus
};
#endif

#endif // ! HU_ALG_COMMMON_H_
