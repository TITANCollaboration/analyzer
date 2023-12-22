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
#include "rapidjson/ostreamwrapper.h"

using namespace rapidjson;
using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::this_thread::sleep_for;


std::ofstream create_csv_file(int run_number, string filename) {

  std::ofstream run_csv_file(filename);
  run_csv_file << "ppg_unix_timestamp,ppg_action,ppg_parameter,ppg_value,mdpp16_timestamp,grif16_timestamp,local_unix_timestamp" << "\n";
  return run_csv_file;
}

void check_action_value(auto ppg_data, const char* json) {
  // Yes this is a terrible way of handling this as I'm decoding twice but oh well, I just didn't
  // feel like writing it better, sorry..
  ppg_data.Parse(json);  // Parse the JSON, should be ?result?
  Value& ppg_action = ppg_data["action"];
  if (ppg_action.GetString() == "starting") {
    // pause_for_change = 1;
  } else if(ppg_action.GetString() == "started") {
    // pause_for_change = 0;
  }

}

const char* add_to_json(const char* json, uint64_t current_unix_timestamp) {

  Document ppg_data;  // Setup document type for incoming JSON
  //Value mine(kStringType);
  ppg_data.Parse(json);  // Parse the JSON, should be ?result?
  ppg_data.AddMember("mdpp16_tdc_last_time", mdpp16_tdc_last_time, ppg_data.GetAllocator());
  ppg_data.AddMember("grif16_tdc_last_time", grif16_tdc_last_time, ppg_data.GetAllocator());
  ppg_data.AddMember("current_unix_timestamp", current_unix_timestamp, ppg_data.GetAllocator());

//  << mdpp16_tdc_last_time << ","
//  << grif16_tdc_last_time << ","
//  << current_unix_timestamp

  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  ppg_data.Accept(writer);

  const char* new_json_output = buffer.GetString();
  //char[] ch = new_json_output;
  static string new_json_string;
  new_json_string = new_json_output;
  //cout <<"New output: " << new_json_output << "\n";
  return new_json_string.c_str();
}

void write_csv_data(std::ofstream& run_csv_file, const char* json, uint64_t current_unix_timestamp) {
  Document ppg_data;  // Setup document type for incoming JSON
  ppg_data.Parse(json);  // Parse the JSON, should be ?result?
  // cout << "My Json: " << json <<"\n";
  Value& ppg_command = ppg_data["command"];
  Value* ppg_bias_value;
  if(ppg_data["command"] != "new_cycle") {
    return;
  }
  Value& ppg_unix_timestamp = ppg_data["timestamp"];  // Get timestamp
  Value& ppg_scan_settings = ppg_data["scan_settings"]["EPICS"]; // Dig down to the EPICS info

  assert(ppg_scan_settings.IsArray());
  for (auto& epics_array : ppg_scan_settings.GetArray()) {  // Loop through EPICS list until we get the DT5 value
    if(epics_array["demand_dev"] == "EBIT:BIAS:VOL") {
      ppg_bias_value = &epics_array["measured_val"];

      egun_voltage = ppg_bias_value->GetDouble();
    }
  }

  run_csv_file << ppg_unix_timestamp.GetUint64() << ","
               << ppg_command.GetString() << ","
               << "EBIT:BIAS:VOL" << ","
               << egun_voltage << ","
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
  std::string json_filename = "run_" + to_string(run_number) + "_all.json";
  //ofstream ofs("run_" + to_string(run_number) + "_all.json");
  //OStreamWrapper osw(ofs);
  std::ofstream json_file(json_filename);

  zmq::context_t context(1);
  cout << "My Run Number!: " << run_number << "\n";

  //  Socket to talk to server
  cout << "Opening CSV file for run " << filename << "\n";
  run_csv_file = create_csv_file(run_number, filename);

  std::cout << "Collecting updates from EBIT PPG server...\n" << std::endl;
  zmq::socket_t subscriber(context, ZMQ_SUB);

  subscriber.connect("tcp://lxebit.triumf.ca:5656");
  subscriber.set(zmq::sockopt::subscribe, "");
  subscriber.set(zmq::sockopt::rcvtimeo, 2000); // every 2 seconds timeout so we can check if we need to close the thread


  while (1) {  // Infinite loop.. when time becomes a loop, when time becomes a loop, when time becomes a loop, ...

    if(ebit_ppg_reader_thread_termination != 0 ) {
      cout << "Closing CSV file: " << filename << "\n";
      close_csv_file(run_csv_file);
      cout << "Closing JSON file: " << json_filename << "\n";
      json_file.close();
      cout << "Terminating EBIT PPG Reader Thread\n";
      pthread_exit(NULL);
    }

    zmq::message_t incoming_msg;
    zmq::recv_result_t result = subscriber.recv(incoming_msg, zmq::recv_flags::none);

    const auto p1 = std::chrono::system_clock::now();
    current_unix_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(p1.time_since_epoch()).count();
    //cout << "Recieved message:" << incoming_msg.str() << "\n";

    //const char* json = incoming_msg.str().c_str(); // !!Uncomment me after testing
    //const char* json = "{\"command\": \"new_cycle\", \"cycle_number\": 12, \"run_number\": 113 , \"scan_settings\": {\"EPICS\": [{\"demand_dev\": \"EBIT:BIAS:VOL\", \"demand_val\": 100.0, \"human_name\": \"Drift tube 5 - PL\", \"measured_dev\":\"EBIT:BIAS:RDVOL\", \"measured_val\": 99.97}],\"FC0InOut\": null,\"PPG\": []},\"timestamp\": 1624392506385}";

    string json_str = incoming_msg.to_string();
    if(json_str.length()>0){
      const char* new_json = add_to_json(json_str.c_str(), current_unix_timestamp);
      json_file << new_json << "\n";
      /*  Document ppg_data;  // Setup document type for incoming JSON
	  ppg_data.Parse(new_json);  // Parse the JSON, should be ?result?
	  Writer<OStreamWrapper> writer(osw);
	  ppg_data.Accept(writer);*/  
      write_csv_data(run_csv_file, json_str.c_str(), current_unix_timestamp);
    }

    write_csv_data(run_csv_file, json, current_unix_timestamp);
    //sleep_for(1ms);

  }
}