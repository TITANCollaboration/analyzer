#define WEBPORT                         9093
#define ROOTDIR "/home/ebit/daq/analyser"
// was used to send css or js files

#define MAXLEN      1024 // maximum url and other string lengths

#define MAX_QUEUE_LEN    4
#define REQUEST_TIMEOUT 10 /* 10 seconds */

void web_server_main(int *arg);
int handle_connection(int fd);
int send_test_html(int fd);
int parse_url(int fd, int *cmd, int *arg);
int get_request(int fd);
int get_line(int fd, char *buf, int maxlen);
int put_line(int fd, char *buf, int len);
int parse_line(char *buf, int first);
int remove_trailing_space(char *buf);
int send_header(int fd, int type);
int send_file(char *filename, int fd);
int send_spectrum_list(int fd);
int send_spectrum_from_handler(int num, int fd);
void load_spec_table_from_ODB();
int GetIDfromAddress(int address);
int GetIDfromName(const char* name);
void decodeurl(char *dst, const char *src);
int send_2d_hist(int num, int fd);
int send_2d_sum_hist(int num, int fd);
