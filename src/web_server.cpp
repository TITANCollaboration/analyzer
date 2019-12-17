/* test web server: gcc -g -o server_basic web_server.c */
/* new js fn similar to odbget - will have to decode args in server.c */

#include <stdio.h>
#include <sys/select.h> /* select(), FD_SET() etc */
#include <netinet/in.h> /* sockaddr_in, IPPROTO_TCP, INADDR_ANY */
#include <signal.h>     /* signal() */
#include <stdlib.h>     /* exit() */
#include <string.h>     /* memset() */
#include <unistd.h>     // write(), read(), close()
#include <ctype.h>      // isxdigit(), isspace()
#include <signal.h>
#include "web_server.h"

#include "histogram.h"

#define MAXSPECNAMES 16

// spectrum name array cannot be longer than url-length
static char buf[MAXLEN], url[MAXLEN], filename[MAXLEN];
static char content_types[][32] = {
   "text/html", "text/css", "text/javascript", "application/json"
};
char *histo_list[MAXSPECNAMES];

#define MORE_FOLLOWS 1
#define NO_COMMAND   0 /* command 0 */
#define SPECLIST     1 /* command 1 */
#define GETSPEC      2 /* command 2 */
#define GETSPEC2D    3 /* command 3 */
#define ODBASET      4 /* command 4 */
#define ODBAGET      5 /* command 5 */
#define ODBAMGET     6 /* command 6 */
#define CALLSPECHANDLER  7 /* command 7 */

int send_spectrum_list(int fd);
int send_spectrum(int num, int fd);

///////////////////////////////////////////////////////////////////////////
/////////////////         server main loop          ///////////////////////
///////////////////////////////////////////////////////////////////////////
void web_server_main(int *arg);
int handle_connection(int fd);
int parse_url(int fd, int *cmd, int *arg);

void web_server_main(int *arg)
//int main(int *arg)
{
   struct sockaddr_in sock_addr;
   int sock_fd, client_fd;
   int sockopt = 1; // Re-use the socket

   signal(SIGPIPE, SIG_IGN);

   if ( (sock_fd = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)) == -1) {
      perror("create socket failed");  exit(-1);
   }
   setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int));
   memset(&sock_addr, 0, sizeof(sock_addr));
   sock_addr.sin_family = AF_INET;
   sock_addr.sin_port = htons(WEBPORT);
   sock_addr.sin_addr.s_addr = INADDR_ANY;

   if ( bind(sock_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1) {
      perror("bind failed"); close(sock_fd); exit(-1);
   }
   if ( listen(sock_fd, MAX_QUEUE_LEN) == -1 ) {
      perror("listen failed"); close(sock_fd); exit(-1);
   }
   fprintf(stdout, "Launch data server ...\n");
   while (1) {
      extern volatile int shutdown_webserver;
      // use select and non-blocking accept
      // on select timeouts (~100ms) check shutdown flag
      int num_fd;  fd_set read_fds;  struct timeval tv;
      tv.tv_sec = 0; tv.tv_usec = 100000; // 100ms
      num_fd = 1; FD_ZERO(&read_fds); FD_SET(sock_fd, &read_fds);
      if ( select(sock_fd + 1, &read_fds, NULL, NULL, &tv) > 0 ) {
         //  fprintf(stdout,"calling accept ...\n");
         if ( (client_fd = accept(sock_fd, NULL, NULL)) < 0 ) {
            perror("accept failed"); // close(sock_fd);  exit(-1);
         } else {
            handle_connection(client_fd); close(client_fd);
         }
      }
      if ( shutdown_webserver != 0 ) {  break; }
   }
   fprintf(stdout, "shutting down data server ...\n");
   close(sock_fd);
   return;
}

int handle_connection(int fd)
{
   int request_count, content_type, command, arg;

   if ( (request_count = get_request(fd))               < 0 ) { return (-1); }
   if ( (content_type = parse_url(fd, &command, &arg)) < 0 ) { return (-1); }
   if ( request_count > 1 ) {
      if ( send_header(fd, content_type)              < 0 ) { return (-1); }
   }
   switch (command) {
   // curl "http://titan01.triumf.ca:9093/?cmd=callspechandler&spectrum1=spec1&spectrum2=spec2"
   case SPECLIST:        // fprintf(stdout,"COMMAND: List\n"       );
      send_spectrum_list(fd); return (0);
   case CALLSPECHANDLER:  //printf("CMD: Get %d spec from Handler\n", arg);
      send_spectrum(arg, fd); return (0);
   case NO_COMMAND:      // printf("No command received.\n");
      sprintf(filename, "%s%s", ROOTDIR, url);
      printf("Getting NO COMMAND?\n");
      if ( send_file(filename, fd) < 0 ) { return (-1); }
   }
   return (0);
}

//"spectrum1d/index.html"
/*    check for javascript cmds or send file               */
/*    file should be under custom pages - check url for .. */
// Format of CALLSPECHANDLER call is:
// host:PORT/?cmd=callSpectrumHandler&spectum0=firstSpec&spectrum1=secondSpec
int parse_url(int fd, int *cmd, int *arg)
{  // strstr returns NULL if cannot find the substring
   char *ptr = url;
   int i;
   // fprintf(stdout,"URL:%s\n", url);
   while ( *ptr != '\0' ) { if (*(ptr++) == '?') {break;} }
   if ( strncmp(ptr, "cmd=getSpectrumList", 18) == 0 ) { /* list spectra */
      *cmd = SPECLIST; return (3);
   }
   if ( strncmp(ptr, "cmd=callspechandler", 18) == 0 ) {
      // loop over list to get names - insert nulls in place of &'s to terminate
      for (i = 0; i < MAXSPECNAMES; i++) {
         if ( (ptr = strstr(ptr, "&spectrum")) == NULL ) {  break; }
         *ptr = '\0'; ++ptr; // terminate previous name (if i>0 )
         ptr = strstr(ptr, "="); ++ptr; // skip over number, and '='
         histo_list[i] = ptr;
      }
      if ( i == 0 ) {
         fprintf(stderr, "can't read any spectrum requests in %s\n", url);
         return (-1);
      }
      *arg = i;  *cmd = CALLSPECHANDLER; return (3);
   }
   *cmd = NO_COMMAND;
   if ( strncmp(url + strlen(url) - 4, ".css", 4) == 0 ) { return (1); }
   if ( strncmp(url + strlen(url) - 3,  ".js", 3) == 0 ) { return (2); }
   return (0);
}

///////////////////////////////////////////////////////////////////////////
/////////////////      spectrum-specific stuff      ///////////////////////
///////////////////////////////////////////////////////////////////////////

#define LIST_HDR "getSpectrumList({'spectrumlist':['" // 34
#define LIST_SEP "', '"                               // 4
#define LIST_TRL "']})"                               // 4

int send_spectrum_list(int fd)
{
   // int i; char temp[128];
   //   put_line(fd, LIST_HDR,   strlen(LIST_HDR) );
   //   put_line(fd, first_name, strlen(first_name) );
   //   while( more ){
   //      put_line(fd, LIST_SEP, strlen(LIST_SEP));
   //      put_line(fd,  name,    strlen(name) );
   //   }
   //   put_line(fd, LIST_TRL,   strlen(LIST_TRL) );

   return (0);
}

#define HIST_HDR "{"  // 1
#define HIST_SEP ", " // 2
#define HIST_TRL "}"  // 1

int send_spectrum(int num, int fd)
{
   int i, j;  char temp[256];
   TH1I *hist;
   //printf("Send Spectrium : Num : %i, FD : %i\n", num, fd);
   put_line(fd, HIST_HDR, strlen(HIST_HDR) );
   for (j = 0; j < num; j++) {
      //printf("hist_list : %s", histo_list[j]);
      if ( j > 0 ) { put_line(fd, HIST_SEP, strlen(HIST_SEP) ); }
      if ( (hist = hist_querytitle(histo_list[j])) == NULL ) { // don't have it
         sprintf(temp, "\'%s\':NULL", histo_list[j] );
         put_line(fd, temp, strlen(temp) );
      } else {                                 // do have this - send contents
         sprintf(temp, "\'%s\':[", histo_list[j] );
         put_line(fd, temp, strlen(temp) );
         for (i = 0; i < hist->valid_bins; i++) {
            if ( i > 0) { put_line(fd, ",", 1 ); }
            sprintf(temp, "%d", (int)hist->data[i] );
            if ( put_line(fd, temp, strlen(temp) ) ) { return (-1); }
         }
         put_line(fd, "]", 1 );
      }
   }
   put_line(fd, HIST_TRL, strlen(HIST_TRL) );
   return (0);
}

///////////////////////////////////////////////////////////////////////////
////////////////////      http-specific stuff      ////////////////////////
///////////////////////////////////////////////////////////////////////////
int send_header(int fd, int type);
int send_file(char *filename, int fd);
int parse_line(char *buf, int first);
int remove_trailing_space(char *buf);
void decodeurl(char *dst, const char *src);

#define TESTA "<HTML>\n<HEAD>\n<TITLE>Served File</TITLE>\n</HEAD>\n\n<BODY>\n"
#define TESTB "<H1>Heading</H1>\nHardcoded test page.</BODY>\n</HTML>\n"
// put_line(fd,TESTA,63-6); put_line(fd,TESTB,56-3);

#define HDRA "HTTP/1.0 200 OK\r\nAccess-Control-Allow-Origin: *\r\n"
#define HDRB "Server: javaspec v0\r\nContent-Type: "
// put_line(fd, HDRA, 49); put_line(fd, HDRB, 35);

int send_header(int fd, int type)
{
   put_line(fd, HDRA, 49); put_line(fd, HDRB, 35);
   put_line(fd, content_types[type], strlen(content_types[type]) );
   put_line(fd, (char*)"\r\n\r\n", 4);
   return 0;
}

int send_file(char *filename, int fd)
{
   char temp[256];
   FILE *fp;
   fprintf(stdout, "sending file: %s\n", filename);
   if ( (fp = fopen(filename, "r")) == NULL) {
      perror("can't open file"); return (-1);
   }
   while ( fgets(temp, 256, fp) != NULL ) {
      put_line(fd, temp, strlen(temp) );
   }
   fclose(fp);
   return (0);
}

/* HTTP/1.0: GET HEAD POST - GET and HEAD must be implemented */
/* HTTP/1.0: OPTIONS PUT DELETE TRACE CONNECT */
/* GET URL[ HTTP/Version] (no spaces in URL) */
/* GET - just return data, others return header then data  */
int parse_line(char *buf, int first)
{
   char *p = url;
   //fprintf(stdout,"Rcv:%s", buf);
   remove_trailing_space(buf);
   if (strlen(buf) == 0 ) { return (0); }
   if ( first ) {
      if (        ! strncmp(buf, "GET ",  4) ) {
         buf += 4;
      } else if ( ! strncmp(buf, "HEAD ", 5) ) {
         buf += 5;
      } else {
         fprintf(stdout, "Unimplemented"); return (-1);
      }
      while ( *buf == ' ' ) { ++buf; } /* skip space */
      while ( *buf != ' ' && *buf != '\0' ) { *p++ = *buf++; } /* copy url */
      while ( *buf == ' ' ) { ++buf; } /* skip space */
      *p = '\0';
      if ( ! strncmp(buf, "HTTP/",  5) ) {
         return (MORE_FOLLOWS);
      }
      return (0);
   }
   return (MORE_FOLLOWS);
}

int remove_trailing_space(char *buf)
{
   char *p = buf + strlen(buf) - 1;
   while ( p >= buf ) { // if( !isspace(*p) ){ break; }
      if ( *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' ) { break; }
      *(p--) = '\0';
   }
   return (0);
}

// decode ascii hex strings ['0'=48 'A'=65 'a'=97]
//  - convert to uppercase, then subtract either (65-10):letter or 48:digit
void decodeurl(char *dst, const char *src)
{
   char a, b;
   while (*src) { // != '\0'
      if ( (*src == '%') && ((a = src[1]) && (b = src[2])) && // a,b != '\0'
            ( isxdigit(a) && isxdigit(b) ) ) { // a,b xdigits
         if ( a >= 'a' ) { a -=  'A' - 'a'; } //->uppercase  //     [0-9a-fA-F]
         if ( a >= 'A' ) { a -= ('A' - 10); } else { a -= '0'; }
         if ( b >= 'a' ) { b -=  'A' - 'a'; } //->uppercase
         if ( b >= 'A' ) { b -= ('A' - 10); } else { b -= '0'; }
         *dst++ = 16 * a + b;  src += 3;
      } else {
         *dst++ = *src++;
      }
   }
   *dst++ = '\0';
}

///////////////////////////////////////////////////////////////////////////
////////////////////           socket I/O          ////////////////////////
///////////////////////////////////////////////////////////////////////////
int get_request(int fd);
int get_line(int fd, char *buf, int maxlen);
int put_line(int fd, char *buf, int length);

/* get next request - should be within timeout of connecting */
/* returns number of lines contained in request              */
int get_request(int fd)
{
   int status, line_count = 0;
   struct timeval tv;
   fd_set read_fd;

   while (1) {
      tv.tv_sec = REQUEST_TIMEOUT;  tv.tv_usec = 0;
      FD_ZERO(&read_fd);  FD_SET(fd, &read_fd);

      /* select first arg is highest fd in any set + 1 */
      /* args 2-4 are 3 separate sets for read, write, exception */
      if ( (status = select (fd + 1, &read_fd, NULL, NULL, &tv )) < 0 ) {
         return (-1); /* error */
      }
      if ( status == 0 ) { return (-1); } /* timeout */
      if ( get_line(fd, buf, MAXLEN) <= 0 ) {
         fprintf(stderr, "empty get_line\n");  return (-1);
      }
      if ( (status = parse_line(buf, (line_count == 0) )) < 0 ) {
         fprintf(stderr, "parse error\n");  return (-1);
      }
      ++line_count;
      if ( status != MORE_FOLLOWS ) { break; }
   }
   return (line_count);
}

/* read up to a newline, 1 character at a time */
int get_line(int fd, char *buf, int maxlen)
{
   int i, status;
   for (i = 0; i < (maxlen - 1); i++) {
      if ( (status = read(fd, buf, 1)) < 0 ) {  /* error */
         perror("put_line failed"); return (-1); /* could continue on some ers */
      }
      if ( status == 0  ) { *(buf)  = '\0'; return (i);   } /* EOF */
      if ( *buf == '\n' ) { *(buf + 1) = '\0'; return (i + 1); } /* End of Line */
      ++buf;
   }
   return (0);
}

/* write complete line as many characters at a time as we can */
int put_line(int fd, char *buf, int length)
{
   int sent;
   while ( length ) {
      if ( (sent = write(fd, buf, length)) <= 0 ) {
         perror("put_line failed"); return (-1);
      }
      length -= sent; buf += sent;
   }
   return (0);
}
