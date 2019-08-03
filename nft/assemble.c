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
#include "jpegHeaders/jpeglib.h"
#include <setjmp.h>

#define          KPM_SURF_FEATURE_DENSITY_L0    70
#define          KPM_SURF_FEATURE_DENSITY_L1   100
#define          KPM_SURF_FEATURE_DENSITY_L2   150
#define          KPM_SURF_FEATURE_DENSITY_L3   200

#define          TRACKING_EXTRACTION_LEVEL_DEFAULT 2
#define          INITIALIZATION_EXTRACTION_LEVEL_DEFAULT 1
#define          KPM_MINIMUM_IMAGE_SIZE 28

//------------PROTOTYPES-------------
float MIN(int x,int y);

//  imageSet.c
static AR2ImageT *ar2GenImageLayer1 ( ARUint8 *image, int xsize, int ysize, int nc, float srcdpi, float dstdpi );
static AR2ImageT *ar2GenImageLayer2 ( AR2ImageT *src, float dstdpi );
// jpeg.c
static int jpgwrite (FILE *fp, unsigned char *image, int w, int h, int nc, float dpi, int quality);

//------------GLOBAL-------------
AR2ImageSetT *imageSet = NULL;
static float  dpiMin = -1.0f;
static float  dpiMax = -1.0f;
static int    dpi_num = 0;
static float  *dpi_list;

#ifdef __cplusplus
extern "C" {
#endif

float EMSCRIPTEN_KEEPALIVE createImageSet( ARUint8 *image, float dpi, int xsize, int ysize){

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

    char *teste = "asa";

    int works = ar2WriteImageSet( teste, imageSet );



    if(imageSet != 0){
        return imageSet->scale[0]->dpi;
    }else{
        return 0.0f;
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

int ar2WriteImageSet( char *filename, AR2ImageSetT *imageSet )
{
        FILE          *fp;
        AR2JpegImageT  jpegImage;
        int            i;
        size_t         len;
        const char     ext[] = ".iset";
        char          *buf;
        
        len = strlen(filename) + strlen(ext) + 1; // +1 for nul terminator.
        arMalloc(buf, char, len);
        sprintf(buf, "%s%s", filename, ext);
        if( (fp=fopen(buf, "wb")) == NULL ) {
            free(buf);
            return (-1);
        }
        free(buf);

        if( fwrite(&(imageSet->num), sizeof(imageSet->num), 1, fp) != 1 ) goto bailBadWrite;

        jpegImage.xsize = imageSet->scale[0]->xsize;
        jpegImage.ysize = imageSet->scale[0]->ysize;
        jpegImage.dpi   = imageSet->scale[0]->dpi;
        jpegImage.nc    = 1;
    #if AR2_CAPABLE_ADAPTIVE_TEMPLATE
        jpegImage.image = imageSet->scale[0]->imgBWBlur[0];
    #else
        jpegImage.image = imageSet->scale[0]->imgBW;
    #endif

        if( ar2WriteJpegImage2(fp, &jpegImage, AR2_DEFAULT_JPEG_IMAGE_QUALITY) < 0 ) goto bailBadWrite;

        for( i = 1; i < imageSet->num; i++ ) {
            if( fwrite(&(imageSet->scale[i]->dpi), sizeof(imageSet->scale[i]->dpi), 1, fp) != 1 ) goto bailBadWrite;
        }

        fclose(fp);
        return 0;
        
    bailBadWrite:
        fclose(fp);
        return (-1);
}
//------------
// jpeg.c
int ar2WriteJpegImage2( FILE *fp, AR2JpegImageT *jpegImage, int quality )
{
    return jpgwrite(fp, jpegImage->image, jpegImage->xsize, jpegImage->ysize, jpegImage->nc, jpegImage->dpi, quality);
}

static int jpgwrite (FILE *fp, unsigned char *image, int w, int h, int nc, float dpi, int quality)
{
    struct jpeg_compress_struct    cinfo;
    struct jpeg_error_mgr          jerr;
    unsigned char  *p;
    int i, j;
    JSAMPARRAY img;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    jpeg_stdio_dest(&cinfo, fp);
    cinfo.image_width    = w;
    cinfo.image_height   = h;
    if( nc == 1 ) {
        cinfo.input_components = 1;
        cinfo.in_color_space = JCS_GRAYSCALE;
    }
    else if( nc == 3 ) {
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;
    }
    else return -1;
    jpeg_set_defaults(&cinfo);
    cinfo.density_unit   = 1;
    cinfo.X_density      = (UINT16)dpi;
    cinfo.Y_density      = (UINT16)dpi;
    cinfo.write_JFIF_header = 1;

    if( quality <   0 ) quality = 0;
    if( quality > 100 ) quality = 100;
    jpeg_set_quality(&cinfo, quality, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    p = image;
    img = (JSAMPARRAY) malloc(sizeof(JSAMPROW) * h);
    for (i = 0; i < h; i++) {
        img[i] = (JSAMPROW) malloc(sizeof(JSAMPLE) * nc * w);
        if( nc == 1 ) {
            for (j = 0; j < w; j++) {
                img[i][j] = *(p++);
            }
        }
        else if( nc == 3 ) {
            for (j = 0; j < w; j++) {
                img[i][j*3+0] = *(p++);
                img[i][j*3+1] = *(p++);
                img[i][j*3+2] = *(p++);
            }
        }
    }
    jpeg_write_scanlines(&cinfo, img, h);

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    for (i = 0; i < h; i++) free(img[i]);
    free(img);

    return 0;
}

#ifdef __cplusplus
}
#endif
