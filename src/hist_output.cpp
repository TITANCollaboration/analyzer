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

void write_json_to_redis_queue() {
    StringBuffer s;
    auto millisec_since_epoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    Writer<StringBuffer> writer(s);
    writer.StartObject();               // Between StartObject()/EndObject(),
    writer.Key("unix_ts_ms");
    writer.Uint64(millisec_since_epoch);
    writer.Key("egun_voltage");
    writer.Uint(1000);
	  writer.Key("hist");
    writer.StartArray();                // Between StartArray()/EndArray(),

    for (unsigned chan_num = 0; chan_num < MDPP_CHAN_NUM; chan_num++) {
        writer.StartArray();                // Between StartArray()/EndArray(),
        for (unsigned hist_bin = 0; hist_bin < HIST_SIZE; hist_bin++) {
            writer.Uint(mdpp16_temporal_hist[chan_num][hist_bin]);                 // all values are elements of the array.
        }
        writer.EndArray();
    }
    writer.EndArray();
    writer.EndObject();

  //  if(mdpp16_temporal_hist[chan_num][hist_bin] != 0) {
    redis.rpush("mdpp16:queue", {s.GetString()});
}

void hist_timer(void* timing_ms) {
    /* hist_timer() : sleep for awhile, wake up, see how much time has elapsed and if it is over the timing_ms threshold
                      if it is then go ahead and write the histogram to REDIS and reset the histogram array to 0
    */
    long my_timing_ms = (long) timing_ms;  // How many ms to wait between each histogram write to REDIS
    milliseconds time_diff;

    // Create new thread which launches the Web server
    /*try {
        // Create an Redis object, which is movable but NOT copyable.
        printf("Connecting to REDIS server.\n");
        auto redis = Redis(redis_connect_string);
        } catch (const Error &e) {
           printf("Could not connect to REDIS server.\n");
          // Error handling.
        }*/

    while (1) {  // Infinite loop.. when time becomes a loop, when time becomes a loop, when time becomes a loop, ...
      time_point<Clock> start = Clock::now();
      sleep_for(5ms);
      time_point<Clock> end = Clock::now();
      time_diff = time_diff + duration_cast<milliseconds>(end - start);

      if (time_diff.count() >= my_timing_ms) {
        write_json_to_redis_queue();
        time_diff = std::chrono::milliseconds{0};  // Reset time_diff to 0
        memset(mdpp16_temporal_hist, 0, sizeof(mdpp16_temporal_hist));  // Reset entire array to 0
      }
   }
}
