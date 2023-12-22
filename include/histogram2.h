#define FOLDER_PATH_LENGTH  64 // folder+handle should fit in 100 char field
#define HANDLE_LENGTH       32
#define TITLE_LENGTH       128 // this goes in 155 char field
#define MAX_HISTOGRAMS   64000
#define FILE_BUFSIZ    1048576 // 64k * 4 * 4 [1M]

#define FLOAT_1D             1

typedef struct TH2IHist_struct TH2IHist;

struct TH2IHist_struct { // float has around 24bits integer precision
   int   type;    char   path[FOLDER_PATH_LENGTH];
   int   nbins;   char  title[TITLE_LENGTH]; int valid_bins;
   int   *data;   char handle[HANDLE_LENGTH];
   int   xbins; int ybins;
   // variables for underflow/overflow
   int xunderflow; int xoverflow; int yunderflow; int yoverflow; int entries;
   int   (*Reset)(TH2IHist *);
   int   (*Fill)(TH2IHist *, int, int, int);
   int   (*GetBin)(TH2IHist *, int, int);
   int   (*SetBinContent)(TH2IHist *, int, int, int);
   int   (*GetBinContent)(TH2IHist *, int, int);
   int   (*SetValidLen  )(TH2IHist *, int);
   // controls for disk flushing - operation count and time
   // every few thousand ops or every few seconds - update disk
   // better to use mmap - so done automatically
};

extern int open_folder(char* path);
extern int close_folder();
extern int Zero2_Histograms();
extern TH2IHist *hist2_querytitle(char *name);
extern TH2IHist *hist2_queryhandle(char *name);

extern TH2IHist *H2_BOOK(char *name, char *title, int xbins, int ybins);
extern int TH2IHist_Reset(TH2IHist *);
extern int TH2IHist_Fill(TH2IHist *, int binx, int biny, int count);
extern int TH2IHist_GetBin(TH2IHist *thisptr, int binx, int biny); // returns the global bin number given x and y bins
extern int TH2IHist_GetBinContent(TH2IHist *, int binx, int biny);
extern int TH2IHist_SetBinContent(TH2IHist *, int binx, int biny, int value);
extern int TH2IHist_SetValidLen(TH2IHist *, int bins);
