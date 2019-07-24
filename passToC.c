#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include "AR/ar.h"
#include "AR2/config.h"
#include "AR2/imageFormat.h"
#include "AR2/imageSet.h"
#include "AR2/featureSet.h"
#include "AR2/util.h"
#include "KPM/kpm.h"

// #define          KPM_SURF_FEATURE_DENSITY_L0    70
// #define          KPM_SURF_FEATURE_DENSITY_L1   100
// #define          KPM_SURF_FEATURE_DENSITY_L2   150
// #define          KPM_SURF_FEATURE_DENSITY_L3   200

// #define          TRACKING_EXTRACTION_LEVEL_DEFAULT 2
// #define          INITIALIZATION_EXTRACTION_LEVEL_DEFAULT 1
#define          KPM_MINIMUM_IMAGE_SIZE 28

// #ifndef MIN
// #  define MIN(x,y) (x < y ? x : y)
// #endif

float MIN(int x,int y);

AR2ImageSetT *imageSet = NULL;
static float  dpiMin = -1.0f;
static float  dpiMax = -1.0f;
static int    dpi_num = 0;
static float  *dpi_list;

#ifdef __cplusplus
extern "C" {
#endif

int EMSCRIPTEN_KEEPALIVE createImageSet( ARUint8 *image, float dpi, int xsize, int ysize){

  float dpiWork, dpiMinAllowable;
  int	i;

  dpiMinAllowable = truncf(((float)KPM_MINIMUM_IMAGE_SIZE / (float)(MIN(xsize, ysize))) * dpi * 1000.0) / 1000.0f;

    
  if (dpiMin == -1.0f) dpiMin = dpiMinAllowable;
  if (dpiMax == -1.0f) dpiMax = dpi;

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
        // ARLOGi("Image DPI (%d): %f\n", i+1, dpiWork);
        dpi_list[dpi_num - i - 1] = dpiWork; // Lowest value goes at tail of array, highest at head.
        dpiWork *= powf(2.0f, 1.0f/3.0f);
        if( dpiWork >= dpiMax*0.95f ) dpiWork = dpiMax;
    }
    

  imageSet = ar2GenImageSet( image, xsize, ysize, 1, dpi, dpi_list, 1 );

  if(imageSet != 0){
    return 2;
  }else{
    return 1;
  }
  
}

float EMSCRIPTEN_KEEPALIVE MIN(int x ,int y){
  return x < y ? x : y;
}

#ifdef __cplusplus
}
#endif

