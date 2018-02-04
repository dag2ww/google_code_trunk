#define IDD_FILE       1
#define IDD_OPEN       2
#define IDD_CONVERT    3
#define IDD_HELP       4
#define IDD_LAYER      5
#define IDD_PRIORITY   6
#define IDD_SETUP    7
#define IDD_OK         9
#define IDC_CHECKBOX1	101
#define IDC_ZONCHANGE	101
#define IDD_ZONCHANGE	101
#define IDC_PRECISION_UPDOWN 110
#define IDD_PRECISION_EDIT 111
#define IDD_ZOFFSET_EDIT 112
#define IDD_DEFMAXZPASS_EDIT 113
#define IDD_DEFRELEASEPLANE_EDIT 114
#define IDD_DEFCLOSEENOUGH_EDIT 115
#define IDD_CANCEL    10
#define IDD_LAYERDISP 11
#define IDD_LAYEROFF  12
#define IDD_EITHERARC 13
#define IDD_CCWARC    14
#define IDD_CWARC     15
#define IDD_ZOFFSET   16
#define IDD_ZCHAR     19
#define IDD_PRIOR     20
#define IDD_DEPTH     21
#define IDD_PRIODISP  22
#define IDD_PRECODE   23
#define IDD_POSTCODE  24
#define IDD_OPTIMIZE  25
#define IDD_RELEASE   26
#define IDD_CLOSE     27
#define IDD_IJREL     28
#define IDD_IJFIRST   29
#define IDD_LINENUM   30
#define IDD_LICENSE   31
#define IDD_USER      32
#define IDD_STATUS    33
#define ARC           0
#define LINE          1
#define POINT         2
#define CW            0
#define CCW           1
#define REV           0
#define FOR           1


struct layer_obj {char name[30];
                  int status; int priority; int arc;
                  char zchar;
                  float depth; float zoffset;
                 };

struct priority_obj {char *precode; char *postcode;
                     int optimize;
                     float release; float close;
                    };

struct convert_obj {int ijrel; int ijfirst; int line_num; int extraz;
                   };

struct entity_obj {int type; int layer; int dir;
                   double x1; double y1; double z1;
                   double x2; double y2; double z2;
                   double radius; double ang_start; double ang_end;
                  }; 
