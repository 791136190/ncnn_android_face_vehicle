#ifndef VEH_DET
#define VEH_DET

#ifdef __cplusplus  
extern "C" {  
#endif

struct plate_attr_{
    float Chars[8];
};

struct box_veh_
{
    unsigned int id;
    unsigned int score;
    int x1;
    int x2;
    int y1;
    int y2;
    float area;
};

struct box_plate_
{
    unsigned int id;
    unsigned int score;
    int x1;
    int x2;
    int y1;
    int y2;
    float area;
    float land[8];
};
struct BboxVeh
{
    box_veh_ veh_box;
    box_plate_ plate_box;
    plate_attr_ plate_rec;
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

#define MAX_VEH_NUM (30)
#define MAX_VEH_LEN (6+6+8)

int veh_det_w = 320;
int veh_det_h = 320;
const int veh_fpn_num = 3;
const int veh_anchor_num = 2;
const int veh_num_class = 4;
const int veh_channels_per_box = 4 + 1 + veh_num_class;
const float veh_anchor[6 * 2] = { 192.0, 192.0, 384.0, 384.0, 48.0, 48.0, 96.0, 96.0, 12.0, 12.0, 24.0, 24.0 };//feature 从小到大,对应Anchor从大到小
float veh_confidence_threshold = 0.6f;
float veh_nms_threshold = 0.5f;

const int plate_det_w = 160;
const int plate_det_h = 160;
const int plate_fpn_num = 3;
const int plate_anchor_num = 2;
const int plate_num_land = 0;
const int plate_channels_per_box = 4 + 1 + plate_num_land;
const float plate_anchor[6 * 2] = { 192.0, 96.0, 384.0, 192.0, 48.0, 24.0, 96.0, 48.0, 12.0, 6.0, 24.0, 12.0 };//feature 从小到大,对应Anchor从大到小
float plate_confidence_threshold = 0.4f;
float plate_nms_threshold = 0.5f;


const int plate_rec_w = 208;
const int plate_rec_h = 48;

#ifdef __cplusplus  
}  
#endif
#endif