#ifndef  __HIST_OUTPUT_H__
#define  __HIST_OUTPUT_H__
using namespace std;
#define HIST_2D_TIME 60*1
#define HIST_2D_BIN_BEGIN 10
#define HIST_2D_BIN_END 20
#define HIST_2D_BIN_COUNT HIST_2D_BIN_END-HIST_2D_BIN_BEGIN

void hist_timer(void* timing_ms);
void write_json_to_redis_queue(int time_diff);
#endif
