#include <chrono>
#include <thread>
#include <sw/redis++/redis++.h>

#include "hist_output.h"
#include "common.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace sw::redis;
using namespace rapidjson;
using namespace std::literals::chrono_literals;

using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::this_thread::sleep_for;

string redis_host = "titan05.triumf.ca";
string redis_port = "6379";
string redis_connect_string = "tcp://" + redis_host + ":" + redis_port;
auto redis = Redis(redis_connect_string);
int zeros[sizeof(HIST_SIZE)];

void write_json_to_redis_queue(int time_diff) {
    StringBuffer s;
    unsigned chan_num = 0;
    unsigned hist_bin = 0;
    auto millisec_since_epoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    int hist_sum = 0;
    Writer<StringBuffer> writer(s);
    writer.StartObject();               // Between StartObject()/EndObject(),
    writer.Key("unix_ts_ms");
    writer.Uint64(millisec_since_epoch);
    writer.Key("egun_voltage");
    writer.Uint(egun_voltage);  // Need to get the voltage from somewhere...
    writer.Key("time_diff");
    writer.Uint(time_diff);
    writer.Key("last_tdc_time");
    writer.Uint64(mdpp16_tdc_last_time);
	  writer.Key("hist");
    writer.StartArray();                // Between StartArray()/EndArray(),

    for (chan_num = 0; chan_num < MDPP_CHAN_NUM; chan_num++) {
        writer.StartArray();                // Between StartArray()/EndArray(),
        //if(memcmp(mdpp16_temporal_hist[chan_num],zeros,sizeof(mdpp16_temporal_hist[chan_num])) == 0) {

      //  }
        for (hist_bin = 0; hist_bin < HIST_SIZE; hist_bin++) {
            hist_sum = hist_sum + mdpp16_temporal_hist[chan_num][hist_bin];
            writer.Uint(mdpp16_temporal_hist[chan_num][hist_bin]);                 // all values are elements of the array.
        }
        writer.EndArray();
    }
    writer.EndArray();
    writer.EndObject();
    //cout << "Hist Sum: " << hist_sum << "\n";
  //  if(mdpp16_temporal_hist[chan_num][hist_bin] != 0) {
    try {
    redis.rpush("mdpp16:queue", {s.GetString()});
  } catch (const Error &e) {
     //printf("Could not write to REDIS server.\n");
    // Error handling.
  }
}

void hist_timer(void* timing_ms) {
    /* hist_timer() : sleep for awhile, wake up, see how much time has elapsed
                      and if it is over the timing_ms threshold if it is then
                      go ahead and write the histogram to REDIS and reset the
                      histogram array to 0
    */
    long my_timing_ms = (long) timing_ms;  // How many ms to wait between each histogram write to REDIS
    milliseconds time_diff;

    cout << "Starting Histogram output thread...\n";

    while (1) {  // Infinite loop.. when time becomes a loop, when time becomes a loop, when time becomes a loop, ...
      if(mytimer_thread_termination != 0 ) {
        cout << "Terminating Timer Thread\n";
        pthread_exit(NULL);
      }
      if (pause_for_change == 0) {
        time_point<Clock> start = Clock::now();
        sleep_for(1ms);
        time_point<Clock> end = Clock::now();
        time_diff = time_diff + duration_cast<milliseconds>(end - start);

        if (time_diff.count() >= my_timing_ms) {
          write_json_to_redis_queue(time_diff.count());
          time_diff = std::chrono::milliseconds{0};  // Reset time_diff to 0
          memset(mdpp16_temporal_hist, 0, sizeof(mdpp16_temporal_hist));  // Reset entire array to 0
        }
      }
   }
}
