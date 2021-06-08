#include <chrono>
#include <iostream>
#include <thread>
#include <sw/redis++/redis++.h>
#include <zmq.hpp>
#include "ebit_ppg_reader.h"
#include "common.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::this_thread::sleep_for;

void read_ebit_parameter() {
  zmq::context_t context(1);

  //  Socket to talk to server
  std::cout << "Collecting updates from EBIT PPG server...\n" << std::endl;
  zmq::socket_t subscriber(context, ZMQ_SUB);

  subscriber.connect("tcp://titan02.triumf.ca:5555");
  subscriber.set(zmq::sockopt::subscribe, "");

  while (1) {  // Infinite loop.. when time becomes a loop, when time becomes a loop, when time becomes a loop, ...

    if(ebit_ppg_reader_thread_termination != 0 ) {
      cout << "Terminating EBIT PPG Reader Thread\n";
      pthread_exit(NULL);
    }

  // Put in a loop here of some type, maybe with a shared variable I can change when I want it to exit
//  Do something like the following in the loop...


    zmq::message_t incoming_msg;
    zmq::recv_result_t result = subscriber.recv(incoming_msg, zmq::recv_flags::none);
    cout << "Recieved message:" << incoming_msg.str() << "\n";
    // Decode JSON
    // Test JSON statement until we get a real one then comment me out!!
    const char* json = "{\"timestamp\":1000000000,\"action\":\"start\",\"parameter\":\"dt5\",\"value\":5}";
    // COmment the above out!!
    Document ppg_data;  // Setup document type for incoming JSON
    ppg_data.Parse(json);  // Parse the JSON, should be ?result?

    // 2. Modify it by DOM.
    Value& ppg_unix_timestamp = ppg_data["timestamp"];
    Value& ppg_action = ppg_data["action"];
    Value& ppg_parameter = ppg_data["parameter"];
    Value& ppg_value = ppg_data["value"];

    cout << "My timestamp: " << ppg_unix_timestamp.GetInt() << '\n';
    cout << "My Action: " << ppg_action.GetString() << '\n';

    //s.SetInt(s.GetInt() + 1);
    sleep_for(1s);

  }
}
