#ifndef  __HIST_OUTPUT_H__
#define  __HIST_OUTPUT_H__
using namespace std;

void hist_timer(void* timing_ms);
void write_json_to_redis_queue(int time_diff);
#endif
