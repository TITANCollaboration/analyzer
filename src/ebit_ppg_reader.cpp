#include <chrono>
#include <iostream>
#include <fstream>
#include <thread>
#include <string>
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


std::ofstream create_csv_file(int run_number, string filename) {

  std::ofstream run_csv_file(filename);
  run_csv_file << "ppg_unix_timestamp, ppg_action, ppg_parameter, ppg_value, mdpp16_timestamp, grif16_timestamp, local_unix_timestamp" << "\n";
  return run_csv_file;
}

void write_csv_data(std::ofstream& run_csv_file, const char* json, uint64_t current_unix_timestamp) {
  // 2. Modify it by DOM.
  Document ppg_data;  // Setup document type for incoming JSON
  ppg_data.Parse(json);  // Parse the JSON, should be ?result?
  Value& ppg_unix_timestamp = ppg_data["timestamp"];
  Value& ppg_action = ppg_data["action"];
  Value& ppg_parameter = ppg_data["parameter"];
  Value& ppg_value = ppg_data["value"];

  cout << "My timestamp: " << ppg_unix_timestamp.GetInt() << '\n';
  cout << "My Action: " << ppg_action.GetString() << '\n';

  egun_voltage = ppg_value.GetFloat();

  run_csv_file << ppg_unix_timestamp.GetInt() << ","
               << ppg_action.GetString() << ","
               << ppg_parameter.GetString() << ","
               << ppg_value.GetFloat() << ","
               << mdpp16_tdc_last_time << ","
               << grif16_tdc_last_time << ","
               << current_unix_timestamp
               << "\n";

}

void close_csv_file(std::ofstream& run_csv_file) {
  run_csv_file.close();
}

void read_ebit_parameter(int run_number) {
  uint64_t current_unix_timestamp = 0;
  std::ofstream run_csv_file;
  std::string filename = "run_" + to_string(run_number) + "_time_voltage.csv";
  zmq::context_t context(1);
  cout << "My Run Number!: " << run_number << "\n";

  //  Socket to talk to server
  cout << "Opening CSV file for run " << filename << "\n";
  run_csv_file = create_csv_file(run_number, filename);

  std::cout << "Collecting updates from EBIT PPG server...\n" << std::endl;
  zmq::socket_t subscriber(context, ZMQ_SUB);

  subscriber.connect("tcp://titan02.triumf.ca:5555");
  subscriber.set(zmq::sockopt::subscribe, "");

  while (1) {  // Infinite loop.. when time becomes a loop, when time becomes a loop, when time becomes a loop, ...

    if(ebit_ppg_reader_thread_termination != 0 ) {
      cout << "Closing CSV file: " << filename << "\n";
      close_csv_file(run_csv_file);
      cout << "Terminating EBIT PPG Reader Thread\n";
      pthread_exit(NULL);
    }

    zmq::message_t incoming_msg;
    zmq::recv_result_t result = subscriber.recv(incoming_msg, zmq::recv_flags::none);

    const auto p1 = std::chrono::system_clock::now();
    current_unix_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(p1.time_since_epoch()).count();
    cout << "Recieved message:" << incoming_msg.str() << "\n";

    const char* json = incoming_msg.str().c_str(); // !!Uncomment me after testing
    //const char* json = "{\"timestamp\":1000000000,\"action\":\"start\",\"parameter\":\"dt5\",\"value\":5}"; // !!Comment me out after testing

    write_csv_data(run_csv_file, json, current_unix_timestamp);
    //sleep_for(1ms);

  }
}
