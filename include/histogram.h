#define FOLDER_PATH_LENGTH  64 // folder+handle should fit in 100 char field
#define HANDLE_LENGTH       32
#define TITLE_LENGTH       128 // this goes in 155 char field
#define MAX_HISTOGRAMS   64000
#define FILE_BUFSIZ    1048576 // 64k * 4 * 4 [1M]

#define FLOAT_1D             1

typedef struct TH1IHist_struct TH1IHist;

struct TH1IHist_struct { // float has around 24bits integer precision
   int   type;    char   path[FOLDER_PATH_LENGTH];
   int   xbins;   char  title[TITLE_LENGTH]; int valid_bins;
   int   *data;   char handle[HANDLE_LENGTH];
   int underflow; int overflow; int entries;
   int   (*Reset)(TH1IHist *);
   int   (*Fill)(TH1IHist *, int, int);
   int   (*SetBinContent)(TH1IHist *, int, int);
   int   (*GetBinContent)(TH1IHist *, int);
   int   (*SetValidLen  )(TH1IHist *, int);
   // controls for disk flushing - operation count and time
   // every few thousand ops or every few seconds - update disk
   // better to use mmap - so done automatically
};

extern int open_folder(char* path);
extern int close_folder();
extern int Zero_Histograms();
extern TH1IHist *hist_querytitle(char *name);
extern TH1IHist *hist_queryhandle(char *name);

extern TH1IHist *H1_BOOK(char *name, char *title, int nbins, int, int);
extern int TH1IHist_Reset(TH1IHist *);
extern int TH1IHist_Fill(TH1IHist *, int bin, int count);
extern int TH1IHist_SetBinContent(TH1IHist *, int bin, int value);
extern int TH1IHist_SetValidLen(TH1IHist *, int bins);
