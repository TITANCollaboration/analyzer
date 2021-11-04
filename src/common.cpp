#include <iostream>

#include "common.h"


int write_pulse_height_event(std::string daq_prefix, int daq_chan, int flags, uint64_t timestamp, int evadcdata) {
  // Used to generate histogram data for output to realtime system (REDIS)
  //int chan;
  if(daq_prefix == "mdpp16") {
    if(pause_for_change == 0) {
      mdpp16_temporal_hist_slice[daq_chan][evadcdata]++;
    }
  //  if(daq_chan == 4) {
    //  cout << "Chan4: " << evadcdata << '\n';
  //  }
    //mdpp16_tdc_last_time = timestamp;
//    chan = daq_chan + 100;
  }

  return(0);
}

int report_counts(int interval, std::string daq_prefix, int MAX_CHANNELS, int addr_count[], unsigned int event_count)
{
	std::string daq_chan = "";
  int rate_data_exists = 0;
#ifdef USE_INFLUXDB
//  std::string point_name = daq_prefix + "_rate";
//	influxdb::Point mypoint = influxdb::Point{point_name};
#endif
	for (int i = 0; i <= MAX_CHANNELS; i++) {
		if ( addr_count[i] == 0 ) { continue; }
    rate_data_exists = 1;
		daq_chan = daq_prefix + '_' + std::to_string(i);

#ifdef USE_INFLUXDB
  //		mypoint.addField(daq_chan, addr_count[i] / interval);
    //  std::cout << "Got an event : " << addr_count[i] / interval << "Daq chan : " << daq_chan<< "Point name: " << point_name << '\n';
  #endif

	}
#ifdef USE_INFLUXDB
//  try {
//    if(rate_data_exists == 1) {
//      std::cout << "Going to write to Influx!" << '\n';
//      printf("Event Count : %i\n", event_count);
//	     influxdb_conn->write(std::move(mypoint));
//     }
//  }
//  catch (...)
//   {
//     std::cout << "A MDPP write exception occurred. Exception Nr. Interval : " << interval  << '\n';
//   }
#endif
	//memset(addr_count, 0, sizeof(addr_count) );
	return (0);
}

void print_vector() {
  cout << "Slice of time..";
  for(int i; i <= sizeof(mdpp16_2d_time_hist); i++){
    for(int j; j  <= sizeof(mdpp16_2d_time_hist[i]); j++){
    cout << mdpp16_2d_time_hist[i][j]<< ",";
     }
  }
}
