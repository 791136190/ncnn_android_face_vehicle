#ifndef HEAD_DET
#define HEAD_DET

#ifdef __cplusplus  
extern "C" {  
#endif


struct box_head_
{
    unsigned int id;
    unsigned int score;
    int x1;
    int x2;
    int y1;
    int y2;
    float area;
};

struct BboxHead
{
    box_head_ head_box;
};

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MID
#define MID(a, b, c) ((a)<(b) ? ((b)<(c)?(b):(a)<(c)?(c):(a)) : ((b)>(c)?(b):(a)>(c)?(c):(a)))
#endif

#define MAX_HEAD_NUM (30)
#define MAX_HEAD_LEN (6)

int head_det_w = 320;
int head_det_h = 320;
const int head_fpn_num = 3;
const int head_anchor_num = 2;
const int head_num_class = 0;
const int head_channels_per_box = 4 + 1 + head_num_class;
const float head_anchor[6 * 2] = { 192.0, 240.0, 384.0, 480.0, 48.0, 60.0, 96.0, 120.0, 12.0, 15.0, 24.0, 30.0 };//feature 从小到大,对应Anchor从大到小
float head_confidence_threshold = 0.6f;
float head_nms_threshold = 0.5f;

#ifdef __cplusplus  
}  
#endif
#endif