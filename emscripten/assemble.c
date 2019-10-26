//------------INCLUDES-------------
#include <emscripten/emscripten.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "AR/ar.h"
#include "AR2/config.h"
#include "AR2/imageFormat.h"
#include "AR2/imageSet.h"
#include "AR2/featureSet.h"
#include "AR2/util.h"
#include "KPM/kpm.h"
#include "jpeglib.h"
//#include <setjmp.h>

// #ifdef _WIN32
// #  define MAXPATHLEN MAX_PATH
// #else

#define          KPM_SURF_FEATURE_DENSITY_L0    70
#define          KPM_SURF_FEATURE_DENSITY_L1   100
#define          KPM_SURF_FEATURE_DENSITY_L2   150
#define          KPM_SURF_FEATURE_DENSITY_L3   200

#define          TRACKING_EXTRACTION_LEVEL_DEFAULT 4
#define          INITIALIZATION_EXTRACTION_LEVEL_DEFAULT 3
#define          KPM_MINIMUM_IMAGE_SIZE 28

//------------PROTOTYPES-------------
float MIN(int x,int y);

//------------GLOBAL-------------
enum {
    E_NO_ERROR = 0,
    E_BAD_PARAMETER = 64,
    E_INPUT_DATA_ERROR = 65,
    E_USER_INPUT_CANCELLED = 66,
    E_BACKGROUND_OPERATION_UNSUPPORTED = 69,
    E_DATA_PROCESSING_ERROR = 70,
    E_UNABLE_TO_DETACH_FROM_CONTROLLING_TERMINAL = 71,
    E_GENERIC_ERROR = 255
};

static int                  genfset = 1;
static int                  genfset3 = 1;
static AR2JpegImageT        *jpegImage;
//static ARUint8             *image;
static int                  xsize, ysize;
static int                  nc = 1;
static float                dpi = 0.0f;

static float                dpiMin = -1.0f;
static float                dpiMax = -1.0f;
static float               *dpi_list;
static int                  dpi_num = 0;

static float                sd_thresh  = -1.0f;
static float                min_thresh = -1.0f;
static float                max_thresh = -1.0f;
static int                  featureDensity = -1;
static int                  occ_size = -1;
static int                  tracking_extraction_level = -1; // Allows specification from command-line.
static int                  initialization_extraction_level = -1;
static char                 exitcode = -1;

AR2JpegImageT       *jpImage = NULL;
AR2ImageSetT        *imageSet = NULL;
AR2FeatureMapT      *featureMap = NULL;
AR2FeatureSetT      *featureSet = NULL;
KpmRefDataSet       *refDataSet = NULL;
float                scale1, scale2;
int                  procMode;
char                *sep = NULL;
int                  maxFeatureNum;
int                  err;

static int   setDPI( void );
int save_jpg_to_file(const char *filename, ARUint8 *image, int w, int h, float dpi);
static int  readImageFromFile(const char *filename, const char *ext, ARUint8 **image_p, int *xsize_p, int *ysize_p, int *nc_p, float *dpi_p);
int arUtilDivideExt( const char *filename, char *s1, char *s2 );

#ifdef __cplusplus
extern "C" {
#endif

float EMSCRIPTEN_KEEPALIVE createImageSet( ARUint8 *imageIn, float dpi, int xsizeIn, int ysizeIn, int nc){
    ARUint8  *image = NULL;
    xsize = xsizeIn;
    ysize = ysizeIn;

    char *filename = "asa";
   

    char  buf[1024];
    int  num;
    int  i, j;
    

    if (genfset) {
        switch (TRACKING_EXTRACTION_LEVEL_DEFAULT) {
            case 0:
                if( sd_thresh  == -1.0f ) sd_thresh  = AR2_DEFAULT_SD_THRESH_L0;
                if( min_thresh == -1.0f ) min_thresh = AR2_DEFAULT_MIN_SIM_THRESH_L0;
                if( max_thresh == -1.0f ) max_thresh = AR2_DEFAULT_MAX_SIM_THRESH_L0;
                if( occ_size   == -1    ) occ_size   = AR2_DEFAULT_OCCUPANCY_SIZE;
                break;
            case 1:
                if( sd_thresh  == -1.0f ) sd_thresh  = AR2_DEFAULT_SD_THRESH_L1;
                if( min_thresh == -1.0f ) min_thresh = AR2_DEFAULT_MIN_SIM_THRESH_L1;
                if( max_thresh == -1.0f ) max_thresh = AR2_DEFAULT_MAX_SIM_THRESH_L1;
                if( occ_size   == -1    ) occ_size   = AR2_DEFAULT_OCCUPANCY_SIZE;
                break;
            case 2:
                if( sd_thresh  == -1.0f ) sd_thresh  = AR2_DEFAULT_SD_THRESH_L2;
                if( min_thresh == -1.0f ) min_thresh = AR2_DEFAULT_MIN_SIM_THRESH_L2;
                if( max_thresh == -1.0f ) max_thresh = AR2_DEFAULT_MAX_SIM_THRESH_L2;
                if( occ_size   == -1    ) occ_size   = AR2_DEFAULT_OCCUPANCY_SIZE*2/3;
                break;
            case 3:
                if( sd_thresh  == -1.0f ) sd_thresh  = AR2_DEFAULT_SD_THRESH_L3;
                if( min_thresh == -1.0f ) min_thresh = AR2_DEFAULT_MIN_SIM_THRESH_L3;
                if( max_thresh == -1.0f ) max_thresh = AR2_DEFAULT_MAX_SIM_THRESH_L3;
                if( occ_size   == -1    ) occ_size   = AR2_DEFAULT_OCCUPANCY_SIZE*2/3;
                break;
            case 4: // Same as 3, but with smaller AR2_DEFAULT_OCCUPANCY_SIZE.
                if( sd_thresh  == -1.0f ) sd_thresh  = AR2_DEFAULT_SD_THRESH_L3;
                if( min_thresh == -1.0f ) min_thresh = AR2_DEFAULT_MIN_SIM_THRESH_L3;
                if( max_thresh == -1.0f ) max_thresh = AR2_DEFAULT_MAX_SIM_THRESH_L3;
                if( occ_size   == -1    ) occ_size   = AR2_DEFAULT_OCCUPANCY_SIZE*1/2;
                break;
             default: // We only get to here if the parameters are already set.
                break;
        }
        ARLOGi("MAX_THRESH  = %f\n", max_thresh);
        ARLOGi("MIN_THRESH  = %f\n", min_thresh);
        ARLOGi("SD_THRESH   = %f\n", sd_thresh);
    }
    if (genfset3) {
        switch(INITIALIZATION_EXTRACTION_LEVEL_DEFAULT) {
            case 0:
                if( featureDensity  == -1 ) featureDensity  = KPM_SURF_FEATURE_DENSITY_L0;
                break;
            default:
            case 1:
                if( featureDensity  == -1 ) featureDensity  = KPM_SURF_FEATURE_DENSITY_L1;
                break;
            case 2:
                if( featureDensity  == -1 ) featureDensity  = KPM_SURF_FEATURE_DENSITY_L2;
                break;
            case 3:
                if( featureDensity  == -1 ) featureDensity  = KPM_SURF_FEATURE_DENSITY_L3;
                break;
        }
        ARLOGi("SURF_FEATURE = %d\n", featureDensity);
    }
    

    setDPI();
   
    ARLOGi("Generating ImageSet...\n");
    ARLOGi("   (Source image xsize=%d, ysize=%d, channels=%d, dpi=%.1f).\n", xsize, ysize, nc, dpi);
    imageSet = ar2GenImageSet( image, xsize, ysize, nc, dpi, dpi_list, dpi_num );
    ARLOGi("   (dpi=%d).\n",imageSet->scale[1]->dpi);
    if( imageSet == 0 ) {
      
        return 2;
    }
    ARLOGi("  Done.\n");
    ARLOGi("Saving to %s.iset...\n", filename);
    if( ar2WriteImageSet( filename, imageSet ) < 0 ) {
        ARLOGe("Save error: %s.iset\n", filename );
        return 3;
    }
    ARLOGi("  Done.\n");

    if (genfset) {
        arMalloc( featureSet, AR2FeatureSetT, 1 );                      // A featureSet with a single image,
        arMalloc( featureSet->list, AR2FeaturePointsT, imageSet->num ); // and with 'num' scale levels of this image.
        featureSet->num = imageSet->num;
        
        ARLOGi("Generating FeatureList...\n");
        for( i = 0; i < imageSet->num; i++ ) {
            ARLOGi("Start for %f dpi image.\n", imageSet->scale[i]->dpi);
            
            featureMap = ar2GenFeatureMap( imageSet->scale[i],
                                          AR2_DEFAULT_TS1*AR2_TEMP_SCALE, AR2_DEFAULT_TS2*AR2_TEMP_SCALE,
                                          AR2_DEFAULT_GEN_FEATURE_MAP_SEARCH_SIZE1, AR2_DEFAULT_GEN_FEATURE_MAP_SEARCH_SIZE2,
                                          AR2_DEFAULT_MAX_SIM_THRESH2, AR2_DEFAULT_SD_THRESH2 );
            if( featureMap == NULL ) {
                ARLOGe("Error!!\n");
                return 4;
            }
            ARLOGi("  Done.\n");
            
            
            featureSet->list[i].coord = ar2SelectFeature2( imageSet->scale[i], featureMap,
                                                          AR2_DEFAULT_TS1*AR2_TEMP_SCALE, AR2_DEFAULT_TS2*AR2_TEMP_SCALE, AR2_DEFAULT_GEN_FEATURE_MAP_SEARCH_SIZE2,
                                                          occ_size,
                                                          max_thresh, min_thresh, sd_thresh, &num );
            if( featureSet->list[i].coord == NULL ) num = 0;
            featureSet->list[i].num   = num;
            featureSet->list[i].scale = i;
            
            scale1 = 0.0f;
            for( j = 0; j < imageSet->num; j++ ) {
                if( imageSet->scale[j]->dpi < imageSet->scale[i]->dpi ) {
                    if( imageSet->scale[j]->dpi > scale1 ) scale1 = imageSet->scale[j]->dpi;
                }
            }
            if( scale1 == 0.0f ) {
                featureSet->list[i].mindpi = imageSet->scale[i]->dpi * 0.5f;
            }
            else {
                /*
                 scale2 = imageSet->scale[i]->dpi;
                 scale = sqrtf( scale1 * scale2 );
                 featureSet->list[i].mindpi = scale2 / ((scale2/scale - 1.0f)*1.1f + 1.0f);
                 */
                featureSet->list[i].mindpi = scale1;
            }
            
            scale1 = 0.0f;
            for( j = 0; j < imageSet->num; j++ ) {
                if( imageSet->scale[j]->dpi > imageSet->scale[i]->dpi ) {
                    if( scale1 == 0.0f || imageSet->scale[j]->dpi < scale1 ) scale1 = imageSet->scale[j]->dpi;
                }
            }
            if( scale1 == 0.0f ) {
                featureSet->list[i].maxdpi = imageSet->scale[i]->dpi * 2.0f;
            }
            else {
                //scale2 = imageSet->scale[i]->dpi * 1.2f;
                scale2 = imageSet->scale[i]->dpi;
                /*
                 scale = sqrtf( scale1 * scale2 );
                 featureSet->list[i].maxdpi = scale2 * ((scale/scale2 - 1.0f)*1.1f + 1.0f);
                 */
                featureSet->list[i].maxdpi = scale2*0.8f + scale1*0.2f;
            }
            
            ar2FreeFeatureMap( featureMap );
        }
        ARLOGi("  Done.\n");
        
        ARLOGi("Saving FeatureSet...\n");
        if( ar2SaveFeatureSet( filename, "fset", featureSet ) < 0 ) {
            ARLOGe("Save error: %s.fset\n", filename );
            return 5;
        }
        ARLOGi("  Done.\n");
        ar2FreeFeatureSet( &featureSet );
    }
    
    if (genfset3) {
        ARLOGi("Generating FeatureSet3...\n");
        refDataSet  = NULL;
        procMode    = KpmProcFullSize;
        for( i = 0; i < imageSet->num; i++ ) {
            //if( imageSet->scale[i]->dpi > 100.0f ) continue;
            
            maxFeatureNum = featureDensity * imageSet->scale[i]->xsize * imageSet->scale[i]->ysize / (480*360);
            ARLOGi("(%d, %d) %f[dpi]\n", imageSet->scale[i]->xsize, imageSet->scale[i]->ysize, imageSet->scale[i]->dpi);
            if( kpmAddRefDataSet (
  #if AR2_CAPABLE_ADAPTIVE_TEMPLATE
                                  imageSet->scale[i]->imgBWBlur[1],
  #else
                                  imageSet->scale[i]->imgBW,
  #endif
                                  imageSet->scale[i]->xsize,
                                  imageSet->scale[i]->ysize,
                                  imageSet->scale[i]->dpi,
                                  procMode, KpmCompNull, maxFeatureNum, 1, i, &refDataSet) < 0 ) { // Page number set to 1 by default.
                ARLOGe("Error at kpmAddRefDataSet.\n");
                
            }
        }
        ARLOGi("  Done.\n");
        ARLOGi("Saving FeatureSet3...\n");
        if( kpmSaveRefDataSet(filename, "fset3", refDataSet) != 0 ) {
            ARLOGe("Save error: %s.fset2\n", filename );
            return 6;
        }
        ARLOGi("  Done.\n");
        kpmDeleteRefDataSet( &refDataSet );
    }
    
    ar2FreeImageSet( &imageSet );

   

    exitcode = E_NO_ERROR;
    return (exitcode);

}

float EMSCRIPTEN_KEEPALIVE MIN(int x ,int y){
    return x < y ? x : y;
}

static int EMSCRIPTEN_KEEPALIVE setDPI(void)
{
    float       dpiWork, dpiMinAllowable;
    char		buf1[256];
    int			i;

    // Determine minimum allowable DPI, truncated to 3 decimal places.
    dpiMinAllowable = truncf(((float)KPM_MINIMUM_IMAGE_SIZE / (float)(MIN(xsize, ysize))) * dpi * 1000.0) / 1000.0f;

    dpiMin = dpiMinAllowable;
    dpiMax = dpi;
    // if (background) {
    //     if (dpiMin == -1.0f) dpiMin = dpiMinAllowable;
    //     if (dpiMax == -1.0f) dpiMax = dpi;
    // }

    // if (dpiMin == -1.0f) {
    //     for (;;) {
    //         printf("Enter the minimum image resolution (DPI, in range [%.3f, %.3f]): ", dpiMinAllowable, (dpiMax == -1.0f ? dpi : dpiMax));
    //         if( fgets( buf1, 256, stdin ) == NULL ) EXIT(E_USER_INPUT_CANCELLED);
    //         if( sscanf(buf1, "%f", &dpiMin) == 0 ) continue;
    //         if (dpiMin >= dpiMinAllowable && dpiMin <= (dpiMax == -1.0f ? dpi : dpiMax)) break;
    //         else printf("Error: you entered %.3f, but value must be greater than or equal to %.3f and less than or equal to %.3f.\n", dpiMin, dpiMinAllowable, (dpiMax == -1.0f ? dpi : dpiMax));
    //     }
    // } else if (dpiMin < dpiMinAllowable) {
    //     ARLOGe("Warning: -min_dpi=%.3f smaller than minimum allowable. Value will be adjusted to %.3f.\n", dpiMin, dpiMinAllowable);
    //     dpiMin = dpiMinAllowable;
    // }
    // if (dpiMax == -1.0f) {
    //     for (;;) {
    //         printf("Enter the maximum image resolution (DPI, in range [%.3f, %.3f]): ", dpiMin, dpi);
    //         if( fgets( buf1, 256, stdin ) == NULL ) EXIT(E_USER_INPUT_CANCELLED);
    //         if( sscanf(buf1, "%f", &dpiMax) == 0 ) continue;
    //         if (dpiMax >= dpiMin && dpiMax <= dpi) break;
    //         else printf("Error: you entered %.3f, but value must be greater than or equal to minimum resolution (%.3f) and less than or equal to image resolution (%.3f).\n", dpiMax, dpiMin, dpi);
    //     }
    // } else if (dpiMax > dpi) {
    //     ARLOGe("Warning: -max_dpi=%.3f larger than maximum allowable. Value will be adjusted to %.3f.\n", dpiMax, dpi);
    //     dpiMax = dpi;
    // }
    
    // Decide how many levels we need.
    if (dpiMin == dpiMax) {
        dpi_num = 1;
    } else {
        dpiWork = dpiMin;
        for( i = 1;; i++ ) {
            dpiWork *= powf(2.0f, 1.0f/3.0f); // *= 1.25992104989487
            if( dpiWork >= dpiMax*0.95f ) {
                break;
            }
        }
        dpi_num = i + 1;
    }
    arMalloc(dpi_list, float, dpi_num);
    
    // Determine the DPI values of each level.
    dpiWork = dpiMin;
    for( i = 0; i < dpi_num; i++ ) {
        ARLOGi("Image DPI (%d): %f\n", i+1, dpiWork);
        dpi_list[dpi_num - i - 1] = dpiWork; // Lowest value goes at tail of array, highest at head.
        dpiWork *= powf(2.0f, 1.0f/3.0f);
        if( dpiWork >= dpiMax*0.95f ) dpiWork = dpiMax;
    }

    return 0;
}

int EMSCRIPTEN_KEEPALIVE save_jpg_to_file(const char *filename, ARUint8 *image,int w, int h, float dpi) {
    struct jpeg_compress_struct cinfo;

    struct jpeg_error_mgr jerr;

    FILE * outfile;
    JSAMPROW row_pointer[1];
    int row;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    if ((outfile = fopen(filename, "wb")) == NULL) {
        ARLOGi("cannot open\n");
        return 1;
    }

    jpeg_stdio_dest(&cinfo, outfile);


    cinfo.image_width = w;
    cinfo.image_height = h;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    cinfo.density_unit   = 1;
    cinfo.X_density      = (UINT16)dpi;
    cinfo.Y_density      = (UINT16)dpi;
    cinfo.write_JFIF_header = 1;

    jpeg_set_quality(&cinfo, 100, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    unsigned char bytes[w * 3];

    while (cinfo.next_scanline < cinfo.image_height) {
        for (int i = 0;  i < w; i+=3){
            bytes[i] = image[i];
            bytes[i+1] = image[i+1];
            bytes[i+2] = image[i+2];
        }
        row_pointer[0] = bytes;
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);


    return 0;
}

static int EMSCRIPTEN_KEEPALIVE readImageFromFile(const char *filename, const char *ext, ARUint8 **image_p, int *xsize_p, int *ysize_p, int *nc_p, float *dpi_p)
{
    char buf[256];
    char buf1[512], buf2[512];

    if (!filename || !image_p || !xsize_p || !ysize_p || !nc_p || !dpi_p) return (E_BAD_PARAMETER);
    // if (!filename || !ext || !image_p || !xsize_p || !ysize_p || !nc_p || !dpi_p) return (E_BAD_PARAMETER);

    // strcpy(buf1, filename);
    // strcat(buf1, "\0");

    // strcpy(buf2, ext);
    // strcat(buf2, "\0");
    
    if (!ext) {
        ARLOGe("Error: unable to determine extension of file '%s'. Exiting.\n", filename);
        return 1;
    }
    if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0 || strcmp(ext, "jpe") == 0) {
        
        ARLOGi("Reading JPEG file...\n");
        arUtilDivideExt( filename, buf1, buf2 );
        ARLOGi("Reading JPEG file name: '%s'\n", filename);
        jpegImage = ar2ReadJpegImage( buf1, buf2);
        if( jpegImage == NULL ) {
            ARLOGe("Error: unable to read JPEG image from file '%s'. Exiting.\n", filename);
            return 1;
        }
        ARLOGi("   Done.\n");
        
        *image_p = jpegImage->image;
        if (jpegImage->nc != 1 && jpegImage->nc != 3) {
            ARLOGe("Error: Input JPEG image is in neither RGB nor grayscale format. %d bytes/pixel %sformat is unsupported. Exiting.\n", jpegImage->nc, (jpegImage->nc == 4 ? "(possibly CMYK) " : ""));
            return 1;
        }
        *nc_p    = jpegImage->nc;
        ARLOGi("JPEG image '%s' is %dx%d.\n", filename, jpegImage->xsize, jpegImage->ysize);
        if (jpegImage->xsize < KPM_MINIMUM_IMAGE_SIZE || jpegImage->ysize < KPM_MINIMUM_IMAGE_SIZE) {
            ARLOGe("Error: JPEG image width and height must be at least %d pixels. Exiting.\n", KPM_MINIMUM_IMAGE_SIZE);
            return 1;
        }
        *xsize_p = jpegImage->xsize;
        *ysize_p = jpegImage->ysize;
        if (*dpi_p == -1.0) {
            if( jpegImage->dpi == 0.0f ) {
                for (;;) {
                    printf("JPEG image '%s' does not contain embedded resolution data, and no resolution specified on command-line.\nEnter resolution to use (in decimal DPI): ", filename);
                    if( fgets( buf, 256, stdin ) == NULL ) {
                        return 1;
                    }
                    if( sscanf(buf, "%f", &(jpegImage->dpi)) == 1 ) break;
                }
            }
            *dpi_p   = jpegImage->dpi;
        }

    //} else if (strcmp(ext, "png") == 0) {
        
        
        
    } else {
        ARLOGe("Error: file '%s' has extension '%s', which is not supported for reading. Exiting.\n", filename, ext);
        return 1;
    }
    
    
    return 0;
}

int EMSCRIPTEN_KEEPALIVE arUtilDivideExt( const char *filename, char *s1, char *s2 )
{
    int   j, k;

    for(j=0;;j++) {
        s1[j] = filename[j];
         
        if( s1[j] == '\0' || s1[j] == '.' ) break;
    }
    s1[j] = '\0';
    if( filename[j] == '\0' ) s2[0] = '\0';
    else {
        j++;
        for(k=0;;k++) {
            s2[k] = filename[j+k];
            if( s2[k] == '\0' ) break;
        }
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
