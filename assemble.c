//------------INCLUDES-------------
#include <emscripten.h>
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


#define          KPM_MINIMUM_IMAGE_SIZE 28

//------------PROTOTYPES-------------
float MIN(int x,int y);

//  imageSet.c
static AR2ImageT *ar2GenImageLayer1 ( ARUint8 *image, int xsize, int ysize, int nc, float srcdpi, float dstdpi );
static AR2ImageT *ar2GenImageLayer2 ( AR2ImageT *src, float dstdpi );

//------------GLOBAL-------------
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

//------------AUX-FUNCTIONS-------------

//  imageSet.c
static AR2ImageT *ar2GenImageLayer1 ( ARUint8 *image, int xsize, int ysize, int nc, float srcdpi, float dstdpi );
static AR2ImageT *ar2GenImageLayer2 ( AR2ImageT *src, float dstdpi );

AR2ImageSetT *ar2GenImageSet( ARUint8 *image, int xsize, int ysize, int nc, float dpi, float dpi_list[], int dpi_num )
{
    AR2ImageSetT   *imageSet;
    int             i;

    if( nc != 1 && nc != 3 )    return NULL;
    if( dpi_num <= 0 )          return NULL;
    if( dpi_list[0] > dpi )     return NULL;
    for( i = 1; i < dpi_num; i++ ) {
        if( dpi_list[i] > dpi_list[0] ) return NULL;
    }

    arMalloc( imageSet, AR2ImageSetT, 1 );
    imageSet->num = dpi_num;
    arMalloc( imageSet->scale,  AR2ImageT*,  imageSet->num );

    imageSet->scale[0] = ar2GenImageLayer1( image, xsize, ysize, nc, dpi, dpi_list[0] );
    for( i = 1; i < dpi_num; i++ ) {
        imageSet->scale[i] = ar2GenImageLayer2( imageSet->scale[0], dpi_list[i] );
    }

    return imageSet;
}

static AR2ImageT *ar2GenImageLayer1( ARUint8 *image, int xsize, int ysize, int nc, float srcdpi, float dstdpi )
{
        AR2ImageT   *dst;
        ARUint8     *p1, *p2;
        int          wx, wy;
        int          sx, sy, ex, ey;
        int          ii, jj, iii, jjj;
        int          co, value;

        wx = (int)lroundf(xsize * dstdpi / srcdpi);
        wy = (int)lroundf(ysize * dstdpi / srcdpi);

        arMalloc( dst, AR2ImageT, 1 );
        dst->xsize = wx;
        dst->ysize = wy;
        dst->dpi   = dstdpi;
    #if AR2_CAPABLE_ADAPTIVE_TEMPLATE
        for( int i = 0; i < AR2_BLUR_IMAGE_MAX; i++ ) {
            arMalloc( dst->imgWBlur[i], ARUint8, wx*wy );
        }
        p2 = dst->imgBWBlur[0];
    #else
        arMalloc( dst->imgBW, ARUint8, wx*wy );
        p2 = dst->imgBW;
    #endif

        // Scale down, nearest neighbour.
        for( jj = 0; jj < wy; jj++ ) {
            sy = (int)lroundf( jj    * srcdpi / dstdpi);
            ey = (int)lroundf((jj+1) * srcdpi / dstdpi) - 1;
            if( ey >= ysize ) ey = ysize - 1;
            for( ii = 0; ii < wx; ii++ ) {
                sx = (int)lroundf( ii    * srcdpi / dstdpi);
                ex = (int)lroundf((ii+1) * srcdpi / dstdpi) - 1;
                if( ex >= xsize ) ex = xsize - 1;

                co = value = 0;
                if( nc == 1 ) {
                    for( jjj = sy; jjj <= ey; jjj++ ) {
                        p1 = &(image[(jjj*xsize+sx)*nc]);
                        for( iii = sx; iii <= ex; iii++ ) {
                            value += *(p1++);
                            co++;
                        }
                    }
                }
                else {
                    for( jjj = sy; jjj <= ey; jjj++ ) {
                        p1 = &(image[(jjj*xsize+sx)*nc]);
                        for( iii = sx; iii <= ex; iii++ ) {
                            value += *(p1++);
                            value += *(p1++);
                            value += *(p1++);
                            co+=3;
                        }
                    }
                }
                *(p2++) = value / co;
            }
        }

    #if AR2_CAPABLE_ADAPTIVE_TEMPLATE
        for( int i = 1; i < AR2_BLUR_IMAGE_MAX; i++ ) {
            p1 = dst->imgBWBlue[0];
            p2 = dst->imgBWBlue[i];
            for( int j = 0; j < wx*wy; j++ ) *(p2++) = *(p1++);
            defocus_image( dst->imgBWBlur[i], wx, wy, 2 );
        }
    #else
        //defocus_image( dst->imgBW, wx, wy, 3 );
    #endif

        return dst;
}

static AR2ImageT *ar2GenImageLayer2( AR2ImageT *src, float dpi )
{
        AR2ImageT   *dst;
        ARUint8     *p1, *p2;
        int          wx, wy;
        int          sx, sy, ex, ey;
        int          ii, jj, iii, jjj;
        int          co, value;

        wx = (int)lroundf(src->xsize * dpi / src->dpi);
        wy = (int)lroundf(src->ysize * dpi / src->dpi);

        arMalloc( dst, AR2ImageT, 1 );
        dst->xsize = wx;
        dst->ysize = wy;
        dst->dpi   = dpi;
    #if AR2_CAPABLE_ADAPTIVE_TEMPLATE
        for( int i = 0; i < AR2_BLUR_IMAGE_MAX; i++ ) {
            arMalloc( dst->imgBWBlur[i], ARUint8, wx*wy );
        }
        p2 = dst->imgBWBlue[0];
    #else
        arMalloc( dst->imgBW, ARUint8, wx*wy );
        p2 = dst->imgBW;
    #endif

        for( jj = 0; jj < wy; jj++ ) {
            sy = (int)lroundf( jj    * src->dpi / dpi);
            ey = (int)lroundf((jj+1) * src->dpi / dpi) - 1;
            if( ey >= src->ysize ) ey = src->ysize - 1;
            for( ii = 0; ii < wx; ii++ ) {
                sx = (int)lroundf( ii    * src->dpi / dpi);
                ex = (int)lroundf((ii+1) * src->dpi / dpi) - 1;
                if( ex >= src->xsize ) ex = src->xsize - 1;

                co = value = 0;
                for( jjj = sy; jjj <= ey; jjj++ ) {
    #if AR2_CAPABLE_ADAPTIVE_TEMPLATE
                    p1 = &(src->imgBWBlur[0][jjj*src->xsize+sx]);
    #else
                    p1 = &(src->imgBW[jjj*src->xsize+sx]);
    #endif
                    for( iii = sx; iii <= ex; iii++ ) {
                        value += *(p1++);
                        co++;
                    }
                }
                *(p2++) = value / co;
            }
        }

    #if AR2_CAPABLE_ADAPTIVE_TEMPLATE
        defocus_image( dst->imageBWBlur[0], wx, wy, 3 );
        for( int i = 1; i < AR2_BLUR_IMAGE_MAX; i++ ) {
            p1 = dst->imgBWBlue[0];
            p2 = dst->imgBWBlue[i];
            for( int j = 0; j < wx*wy; j++ ) *(p2++) = *(p1++);
            defocus_image( dst->imgBWBlur[i], wx, wy, 2 );
        }
    #else
        //defocus_image( dst->imgBW, wx, wy, 3 );
    #endif

        return dst;
}

#ifdef __cplusplus
}
#endif

