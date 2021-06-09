#ifndef  __EBIT_PPG_READER_H__
#define  __EBIT_PPG_READER_H__

using namespace std;
//using namespace rapidjson;

void read_ebit_parameter(int run_number);
std::ofstream create_csv_file(int run_number, string filename);
void write_csv_data(std::ofstream& run_csv_file, const char* json, uint64_t current_unix_timestamp);
void close_csv_file(std::ofstream& run_csv_file);

#endif
