#define restrict
#define abs(x) (x<0 ? (-(x)) : (x) )

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.1     Thu Jul 19 22:35:44 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_boundary -- Returns coordinates of IMG_boundary pixels.         */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      10-Jul-2001                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C callable, and is called as follows:               */
/*                                                                          */
/*          void IMG_boundary                                               */
/*          (                                                               */
/*              const unsigned char *restrict i_data,                       */
/*              int rows, int cols,                                         */
/*              int *restrict o_coord,                                      */
/*              int *restrict o_grey                                        */
/*          );                                                              */
/*                                                                          */
/*      The arguments are defined as follows:                               */
/*                                                                          */
/*          i_data   Input images that is cols-by-rows in size.             */
/*          rows     Height of the input image                              */
/*          cols     Width of the input image                               */
/*          o_coord  Array to write output coordinates to                   */
/*          o_grey   Array to write output grey levels to                   */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      This routine scans an image looking for non-zero pixels.            */
/*      The locations of those pixels are stored out to the o_coord         */
/*      as packed Y/X pairs, with Y in the upper half, and X in             */
/*      the lower half.  The grey levels encountered are stored             */
/*      in the o_grey array in parallel.                                    */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_boundary
(
    const unsigned char *restrict i_data,
    int rows, int cols,
    int *restrict o_coord,
    int *restrict o_grey
)
{
    int x, y, p;

    for (y = 0; y < rows; y++)
        for (x = 0; x < cols; x++)
            if ((p = *i_data++) != 0)
            {
                *o_coord++ = ((y & 0xFFFF) << 16) | (x & 0xFFFF);
                *o_grey++  = p;
            }
}

/* ======================================================================== */
/*  End of file:  img_boundary.c                                            */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.1     Mon Aug 13 22:45:31 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  NAME                                                                    */
/*      IMG_conv_3x3    -- 3x3 convolution                                  */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      31-JUl-2001                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine has the following C prototype:                         */
/*                                                                          */
/*      void IMG_conv_3x3   (    const unsigned char *restrict inptr,       */
/*                                 unsigned char *restrict outptr,          */
/*                                          int            x_dim,           */
/*                           const          char *restrict mask,            */
/*                                          int            shift)           */
/*                                                                          */
/*     The convolution routine accepts three rows of 'x_dim' input points   */
/*     and performs some operation on each.  A total of 'x_dim' outputs     */
/*     are written to the output array. The 'mask' array has the 3 by 3     */
/*     array of coefficients.                                               */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*                                                                          */
/*     The convolution kernel accepts three rows of 'x_dim' input points    */
/*     and produces one output row of 'x_dim' points using the input mask   */
/*     of 3 by 3. The user defined shift value is used to shift the convo-  */
/*     lution value, down to the byte range. The convolution sum is also    */
/*     range limited to 0..255. The shift amount is non-zero for low pass   */
/*     filters, and zero for high pass and sharpening filters.              */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

#include "IMG_conv_3x3.h"

/* ======================================================================== */
/*  IMG_conv_3x3   -- Natural C version of IMG_conv_3x3().                  */
/* ======================================================================== */

void IMG_conv_3x3(const unsigned char *restrict inptr,
                 unsigned char       *restrict outptr,
                 int                 x_dim,
                 const char          *restrict mask,
                 int                 shift)
{
     const unsigned char     *IN1,*IN2,*IN3;
     unsigned char           *OUT;

     short    pix10,  pix20,  pix30;
     short    mask10, mask20, mask30;

     int      sum,    sum00,  sum11;
     int      i;
     int      sum22,  j;

     /*-------------------------------------------------------------------*/
     /* Set imgcols to the width of the image and set three pointers for  */
     /* reading data from the three input rows. Alos set the output poin- */
     /* ter.                                                              */
     /*-------------------------------------------------------------------*/

     IN1      =   inptr;
     IN2      =   IN1 + x_dim;
     IN3      =   IN2 + x_dim;
     OUT      =   outptr;

     /*-------------------------------------------------------------------*/
     /* The j: loop iterates to produce one output pixel per iteration.   */
     /* The mask values and the input values are read using the i loop.   */
     /* The convolution sum is then computed. The convolution sum is      */
     /* then shifted and range limited to 0..255                          */
     /*-------------------------------------------------------------------*/

     for (j = 0; j < x_dim ; j++)
     {
         /*---------------------------------------------------------------*/
         /* Initialize convolution sum to zero, for every iteration of    */
         /* outer loop. The inner loop computes convolution sum.          */
         /*---------------------------------------------------------------*/

         sum = 0;

         for (i = 0; i < 3; i++)
         {
             pix10  =   IN1[i];
             pix20  =   IN2[i];
             pix30  =   IN3[i];

             mask10 =   mask[i];
             mask20 =   mask[i + 3];
             mask30 =   mask[i + 6];

             sum00  =   pix10 * mask10;
             sum11  =   pix20 * mask20;
             sum22  =   pix30 * mask30;

             sum   +=   sum00 + sum11+ sum22;
         }

         /*---------------------------------------------------------------*/
         /*  Increment input pointers and shift sum and range limit to    */
         /*  0..255.                                                      */
         /*---------------------------------------------------------------*/

         IN1++;
         IN2++;
         IN3++;

         sum = (sum >> shift);

         if ( sum <  0  )       sum = 0;
         if ( sum > 255 )       sum = 255;

         /*--------------------------------------------------------------*/
         /* Store output sum into the output pointer OUT                 */
         /*--------------------------------------------------------------*/

         *OUT++   =       sum;
     }
}

/* ======================================================================== */
/*  End of file:  img_conv_3x3.c                                            */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.2     Sun Mar 17 07:44:10 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      corr_3x3: 3x3 correlation with rounding for 8 bit data              */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      14-Mar-2002                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*          void IMG_corr_3x3                                               */
/*          (                                                               */
/*              const unsigned char *i_data,       // input image       //  */
/*              int        *restrict o_data,       // output image      //  */
/*              const unsigned char  mask[3][3],   // convolution mask  //  */
/*              int                  x_dim,        // width of image    //  */
/*              int                  n_out         // number of outputs //  */
/*          );                                                              */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The correlation performs a point by point multiplication of the     */
/*      3 by 3 mask with the input image.  The result of the nine           */
/*      multiplications are then summed up together to produce a            */
/*      convolution sum.  This sum is then stored to the output array.      */
/*                                                                          */
/*      The image mask to be correlated is typically part of the input      */
/*      image and indicates the area of the best match between the          */
/*      input image and mask.  The mask is moved one column at a time,      */
/*      advancing the mask over the portion of the row specified by         */
/*      'n_out'.  When 'n_out' is larger than 'x_dim', multiple rows        */
/*      will be processed.                                                  */
/*                                                                          */
/*      An application may call this kernel once per row to calculate       */
/*      the correlation for an entire image:                                */
/*                                                                          */
/*          for (i = 0; i < rows; i++)                                      */
/*          {                                                               */
/*              IMG_corr_3x3(&i_data[i * x_dim], &o_data[i * n_out],        */
/*                          mask, x_dim, n_out);                            */
/*          }                                                               */
/*                                                                          */
/*      Alternately, the kernel may be invoked for multiple rows at         */
/*      a time, although the two outputs at the end of each row will        */
/*      have meaningless values.  For example:                              */
/*                                                                          */
/*          IMG_corr_3x3(i_data, o_data, mask, x_dim, 2 * x_dim);           */
/*                                                                          */
/*      This will produce two rows of outputs into 'o_data'.  The           */
/*      outputs at locations o_data[x_dim - 2], o_data[x_dim - 1],          */
/*      o_data[2*x_dim - 2] and o_data[2*x_dim - 1] will have               */
/*      meaningless values.  This is harmless, although the application     */
/*      will have to account for this when interpreting the results.        */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      The array pointed to by o_data does not alias with the array        */
/*      pointed to by i_data or mask.                                       */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_corr_3x3
(
    const unsigned char *i_data,       /* input image       */
    int        *restrict o_data,       /* output image      */
    const unsigned char  mask[3][3],   /* convolution mask  */
    int                  x_dim,        /* width of image    */
    int                  n_out         /* number of outputs */
)
{
    int i, j, k;

    for (i = 0; i < n_out; i++)
    {
        int sum = 0;

        for (j = 0; j < 3; j++)
            for (k = 0; k < 3; k++)
                sum += i_data[j * x_dim + i + k] * mask[j][k];

        o_data[i] = sum;
    }
}
/* ======================================================================== */
/*  End of file:  img_corr_3x3.c                                            */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.7     Sat Mar 16 22:30:58 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================= */
/*  TEXAS INSTRUMENTS, INC.                                                */
/*                                                                         */
/*  NAME                                                                   */
/*      IMG_corr_gen                                                       */
/*                                                                         */
/*  REVISION DATE                                                          */
/*      15-Oct-2000                                                        */
/*                                                                         */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*      void IMG_corr_gen                                                   */
/*      (                                                                   */
/*          short   *x,                                                     */
/*          short   *h,                                                     */
/*          short   *y,                                                     */
/*          int     m,                                                      */
/*          int     x_dim                                                   */
/*      );                                                                  */
/*                                                                          */
/*      x[]   : Input pixel array.                                          */
/*      h[M]  : Input 1xM mask array                                        */
/*      y[]   : Output array of correlation sum                             */
/*      M     : Length of filter.                                           */
/*      x_dim : Width of input image                                        */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The routine performs a generalized correlation with a 1 by M tap    */
/*      filter. It can be called repetitively to form an arbitrary MxN 2D   */
/*      generalized correlation kernel. The correlation sum is stored as    */
/*      half words. The input pixel, and mask data is assumed to come in    */
/*      as shorts.  No restrictions apply to x_dim and M.                   */
/*                                                                          */
/*      If the width of the input image is x_dim and the mask is M then     */
/*      the output array must have at-least a dimension of (x_dim - m + 8). */
/*                                                                          */
/*  C CODE                                                                  */
/*      void IMG_corr_gen(short *x, short *h, short *y, int M, int x_dim)   */
/*      {                                                                   */
/*          iters = x_dim - M;                                              */
/*          for (j = 0; j < iters; j++)                                     */
/*          {                                                               */
/*              sum =  y[j];                                                */
/*              for (i = 0; i < M; i++)                                     */
/*              {                                                           */
/*                  sum += xptr[i + j] * hptr[i];                           */
/*              }                                                           */
/*              y[j] = sum;                                                 */
/*          }                                                               */
/*      }                                                                   */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_corr_gen
(
    const short *restrict x,
    const short *restrict h,
    short       *restrict y,
    int                   m,
    int                   x_dim
)
{
    int i, j;

    for (j = 0; j < x_dim - m; j++)
         for (i = 0; i < m; i++)
              y[j] += x[i + j] * h[i];
}

/* ======================================================================== */
/*  End of file:  img_corr_gen.c                                            */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.1     Thu Jul 19 22:35:39 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_dilate_bin--This code performs 3x3 binary dilation              */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      20-Oct-2000                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C callable, and has the following C prototype:      */
/*                                                                          */
/*      void IMG_dilate_bin                                                 */
/*      (                                                                   */
/*          const unsigned char *restrict in_data,   // Incoming image  //  */
/*          unsigned char       *restrict out_data,  // Filtered output //  */
/*          const char          *restrict mask,      // Filter mask     //  */
/*          int cols  // Number of columns to process, in bytes.        //  */
/*      );                                                                  */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The function IMG_dilate_bin() implements binary dilation using an   */
/*      arbitrary 3x3 mask.  The dilation operator generates output pixels  */
/*      by ORing the pixels under the input mask together to generate       */
/*      the output pixel.  The input mask specifies whether one or more     */
/*      pixels from the input are to be ignored.                            */
/*                                                                          */
/*      In pseudo-code, the filtering operation for a pixel at (x, y)       */
/*      works like so:                                                      */
/*                                                                          */
/*          result = 0;                                                     */
/*          if (mask[0][0] != DONT_CARE) result |= input[y + 0][x + 0];     */
/*          if (mask[0][1] != DONT_CARE) result |= input[y + 1][x + 1];     */
/*          if (mask[0][2] != DONT_CARE) result |= input[y + 2][x + 2];     */
/*          if (mask[1][0] != DONT_CARE) result |= input[y + 0][x + 0];     */
/*          if (mask[1][1] != DONT_CARE) result |= input[y + 1][x + 1];     */
/*          if (mask[1][2] != DONT_CARE) result |= input[y + 2][x + 2];     */
/*          if (mask[2][0] != DONT_CARE) result |= input[y + 0][x + 0];     */
/*          if (mask[2][1] != DONT_CARE) result |= input[y + 1][x + 1];     */
/*          if (mask[2][2] != DONT_CARE) result |= input[y + 2][x + 2];     */
/*          output[y][x] = result;                                          */
/*                                                                          */
/*      For this code, "DONT_CARE" is specified by a negative value         */
/*      in the input mask.  Non-negative values in the mask cause the       */
/*      corresponding pixel to be included in the dilation operation.       */
/*                                                                          */
/*      Note that this code operates on a bitmap where each pixel is        */
/*      represented as a single bit within a byte or word.  Although        */
/*      the pseudo-code above operates only on one pixel at a time,         */
/*      with a single pixel in each array element, this implementation      */
/*      operates on a bitmap which contains 8 pixels in each byte.          */
/*                                                                          */
/*      Pixels are organized within each byte such that the pixel with      */
/*      the smallest index is in the LSB position, and the pixel with       */
/*      the largest index is in the MSB position.  (That is, the code       */
/*      assumes a LITTLE ENDIAN bit ordering.)                              */
/*                                                                          */
/*      Note that the "cols" argument actually specifies the number of      */
/*      BYTES in the output, not the number of columns.  The number of      */
/*      columns is 8 times this argument.                                   */
/*                                                                          */
/*  TECHNIQUES                                                              */
/*      The 3x3 dilation mask is applied to 32 output pixels                */
/*      simultaneously.  This is done with 32-bit-wide bitwise              */
/*      operators in the register file.  In order to do this, the code      */
/*      reads in a 34-bit-wide input window, and 40-bit operations          */
/*      are used to manipulate the pixels initially.                        */
/*                                                                          */
/*      Because the code reads a 34-bit context for each 32-bits of         */
/*      output, the input needs to be one byte longer than the output       */
/*      in order to make the rightmost two pixels well-defined.             */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      1.  Negative values in the mask specify "DONT_CARE", and non-       */
/*          negative values specify that pixels are included in the         */
/*          dilation operation.                                             */
/*                                                                          */
/*      2.  The input image needs to have a multiple of 64 pixels(bits)     */
/*          per row.  Therefore, "cols" must be a multiple of 8.            */
/*                                                                          */
/*  NOTES                                                                   */
/*      Little Endian                                                       */
/*                                                                          */
/*      "Digital Image Processing: Principles and Applications"             */
/*      by Gregory A. Baxes, Chapter 5                                      */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_dilate_bin
(
    const unsigned char *restrict in_data,
    unsigned char       *restrict out_data,
    const char          *restrict mask,
    int cols
)
{
    int i;
    unsigned long p0l, p3l, p6l;
    unsigned p0, p1, p2, p3, p4, p5, p6, p7, p8, r;

    /* -------------------------------------------------------------------- */
    /*  Iterate over the input, processing 32 pixels per iteration.         */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < cols; i += 4)
    {
        /* ---------------------------------------------------------------- */
        /*  Load in our 34-bit by 3-bit context for applying the 3x3 mask.  */
        /* ---------------------------------------------------------------- */
        p0l = ((unsigned)     in_data[i +          0]      ) |
              ((unsigned)     in_data[i +          1] << 8 ) |
              ((unsigned)     in_data[i +          2] << 16) |
              ((unsigned)     in_data[i +          3] << 24) |
              ((unsigned long)in_data[i +          4] << 32);

        p3l = ((unsigned)     in_data[i + cols   + 0]      ) |
              ((unsigned)     in_data[i + cols   + 1] << 8 ) |
              ((unsigned)     in_data[i + cols   + 2] << 16) |
              ((unsigned)     in_data[i + cols   + 3] << 24) |
              ((unsigned long)in_data[i + cols   + 4] << 32);

        p6l = ((unsigned)     in_data[i + cols*2 + 0]      ) |
              ((unsigned)     in_data[i + cols*2 + 1] << 8 ) |
              ((unsigned)     in_data[i + cols*2 + 2] << 16) |
              ((unsigned)     in_data[i + cols*2 + 3] << 24) |
              ((unsigned long)in_data[i + cols*2 + 4] << 32);

        /* ---------------------------------------------------------------- */
        /*  Generate 3 offset copies of each row so that we can perform     */
        /*  ANDs between pixels that are neighbors.                         */
        /* ---------------------------------------------------------------- */
        p0 = p0l;   p1 = p0l >> 1;   p2 = p0l >> 2;
        p3 = p3l;   p4 = p3l >> 1;   p5 = p3l >> 2;
        p6 = p6l;   p7 = p6l >> 1;   p8 = p6l >> 2;

        /* ---------------------------------------------------------------- */
        /*  Now sum the filtered pixels together by ORing.                  */
        /* ---------------------------------------------------------------- */
        r = 0;
        if (mask[0] >= 0) r |= p0;
        if (mask[1] >= 0) r |= p1;
        if (mask[2] >= 0) r |= p2;
        if (mask[3] >= 0) r |= p3;
        if (mask[4] >= 0) r |= p4;
        if (mask[5] >= 0) r |= p5;
        if (mask[6] >= 0) r |= p6;
        if (mask[7] >= 0) r |= p7;
        if (mask[8] >= 0) r |= p8;

        /* ---------------------------------------------------------------- */
        /*  Write the result as four bytes.                                 */
        /* ---------------------------------------------------------------- */
        out_data[i + 0] = (r >>  0) & 0xFF;
        out_data[i + 1] = (r >>  8) & 0xFF;
        out_data[i + 2] = (r >> 16) & 0xFF;
        out_data[i + 3] = (r >> 24) & 0xFF;
    }
}

/* ======================================================================== */
/*  End of file:  img_dilate_bin.c                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.4     Thu Jul 19 22:35:38 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_erode_bin-- This code performs 3x3 binary dilation              */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      20-Oct-2000                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C callable, and has the following C prototype:      */
/*                                                                          */
/*      void IMG_erode_bin                                                  */
/*      (                                                                   */
/*          const unsigned char *restrict in_data,   // Incoming image  //  */
/*          unsigned char       *restrict out_data,  // Filtered output //  */
/*          const char          *restrict mask,      // Filter mask     //  */
/*          int cols  // Number of columns to process, in bytes.        //  */
/*      );                                                                  */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The function IMG_erode_bin() implements binary erosion using an     */
/*      arbitrary 3x3 mask.  The erosion operator generates output pixels   */
/*      by ANDing the pixels under the input mask together to generate      */
/*      the output pixel.  The input mask specifies whether one or more     */
/*      pixels from the input are to be ignored.                            */
/*                                                                          */
/*      In pseudo-code, the filtering operation for a pixel at (x, y)       */
/*      works like so:                                                      */
/*                                                                          */
/*          result = 1;                                                     */
/*          if (mask[0][0] != DONT_CARE) result &= input[y + 0][x + 0];     */
/*          if (mask[0][1] != DONT_CARE) result &= input[y + 1][x + 1];     */
/*          if (mask[0][2] != DONT_CARE) result &= input[y + 2][x + 2];     */
/*          if (mask[1][0] != DONT_CARE) result &= input[y + 0][x + 0];     */
/*          if (mask[1][1] != DONT_CARE) result &= input[y + 1][x + 1];     */
/*          if (mask[1][2] != DONT_CARE) result &= input[y + 2][x + 2];     */
/*          if (mask[2][0] != DONT_CARE) result &= input[y + 0][x + 0];     */
/*          if (mask[2][1] != DONT_CARE) result &= input[y + 1][x + 1];     */
/*          if (mask[2][2] != DONT_CARE) result &= input[y + 2][x + 2];     */
/*          output[y][x] = result;                                          */
/*                                                                          */
/*      For this code, "DONT_CARE" is specified by a negative value         */
/*      in the input mask.  Non-negative values in the mask cause the       */
/*      corresponding pixel to be included in the erosion operation.        */
/*                                                                          */
/*      Note that this code operates on a bitmap where each pixel is        */
/*      represented as a single bit within a byte or word.  Although        */
/*      the pseudo-code above operates only on one pixel at a time,         */
/*      with a single pixel in each array element, this implementation      */
/*      operates on a bitmap which contains 8 pixels in each byte.          */
/*                                                                          */
/*      Pixels are organized within each byte such that the pixel with      */
/*      the smallest index is in the LSB position, and the pixel with       */
/*      the largest index is in the MSB position.  (That is, the code       */
/*      assumes a LITTLE ENDIAN bit ordering.)                              */
/*                                                                          */
/*      Note that the "cols" argument actually specifies the number of      */
/*      BYTES in the output, not the number of columns.  The number of      */
/*      columns is 8 times this argument.                                   */
/*                                                                          */
/*  TECHNIQUES                                                              */
/*      The 3x3 erosion mask is applied to 32 output pixels                 */
/*      simultaneously.  This is done with 32-bit-wide bitwise              */
/*      operators in the register file.  In order to do this, the code      */
/*      reads in a 34-bit-wide input window, and 40-bit operations          */
/*      are used to manipulate the pixels initially.                        */
/*                                                                          */
/*      Because the code reads a 34-bit context for each 32-bits of         */
/*      output, the input needs to be one byte longer than the output       */
/*      in order to make the rightmost two pixels well-defined.             */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      1.  Negative values in the mask specify "DONT_CARE", and non-       */
/*          negative values specify that pixels are included in the         */
/*          erosion operation.                                              */
/*                                                                          */
/*      2.  The input image needs to have a multiple of 64 pixels(bits)     */
/*          per row.  Therefore, "cols" must be a multiple of 8.            */
/*                                                                          */
/*  NOTES                                                                   */
/*      Little Endian                                                       */
/*                                                                          */
/*      "Digital Image Processing: Principles and Applications"             */
/*      by Gregory A. Baxes, Chapter 5                                      */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_erode_bin
(
    const unsigned char *restrict in_data,
    unsigned char       *restrict out_data,
    const char          *restrict mask,
    int cols
)
{
    int i;
    unsigned long p0l, p3l, p6l;
    unsigned p0, p1, p2, p3, p4, p5, p6, p7, p8, r;

    /* -------------------------------------------------------------------- */
    /*  Iterate over the input, processing 32 pixels per iteration.         */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < cols; i += 4)
    {
        /* ---------------------------------------------------------------- */
        /*  Load in our 34-bit by 3-bit context for applying the 3x3 mask.  */
        /* ---------------------------------------------------------------- */
        p0l = ((unsigned)     in_data[i +          0]      ) |
              ((unsigned)     in_data[i +          1] << 8 ) |
              ((unsigned)     in_data[i +          2] << 16) |
              ((unsigned)     in_data[i +          3] << 24) |
              ((unsigned long)in_data[i +          4] << 32);

        p3l = ((unsigned)     in_data[i + cols   + 0]      ) |
              ((unsigned)     in_data[i + cols   + 1] << 8 ) |
              ((unsigned)     in_data[i + cols   + 2] << 16) |
              ((unsigned)     in_data[i + cols   + 3] << 24) |
              ((unsigned long)in_data[i + cols   + 4] << 32);

        p6l = ((unsigned)     in_data[i + cols*2 + 0]      ) |
              ((unsigned)     in_data[i + cols*2 + 1] << 8 ) |
              ((unsigned)     in_data[i + cols*2 + 2] << 16) |
              ((unsigned)     in_data[i + cols*2 + 3] << 24) |
              ((unsigned long)in_data[i + cols*2 + 4] << 32);

        /* ---------------------------------------------------------------- */
        /*  Generate 3 offset copies of each row so that we can perform     */
        /*  ANDs between pixels that are neighbors.                         */
        /* ---------------------------------------------------------------- */
        p0 = p0l;   p1 = p0l >> 1;   p2 = p0l >> 2;
        p3 = p3l;   p4 = p3l >> 1;   p5 = p3l >> 2;
        p6 = p6l;   p7 = p6l >> 1;   p8 = p6l >> 2;

        /* ---------------------------------------------------------------- */
        /*  Now sum the filtered pixels together by ANDing.                 */
        /* ---------------------------------------------------------------- */
        r = ~0U;
        if (mask[0] >= 0) r &= p0;
        if (mask[1] >= 0) r &= p1;
        if (mask[2] >= 0) r &= p2;
        if (mask[3] >= 0) r &= p3;
        if (mask[4] >= 0) r &= p4;
        if (mask[5] >= 0) r &= p5;
        if (mask[6] >= 0) r &= p6;
        if (mask[7] >= 0) r &= p7;
        if (mask[8] >= 0) r &= p8;

        /* ---------------------------------------------------------------- */
        /*  Write the result as four bytes.                                 */
        /* ---------------------------------------------------------------- */
        out_data[i + 0] = (r >>  0) & 0xFF;
        out_data[i + 1] = (r >>  8) & 0xFF;
        out_data[i + 2] = (r >> 16) & 0xFF;
        out_data[i + 3] = (r >> 24) & 0xFF;
    }
}

/* ======================================================================== */
/*  End of file:  img_erode_bin.c                                           */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.2     Sun Mar 10 01:00:56 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_errdif_bin -- Binary Floyd-Steinberg Error Diffusion. Endian Neutral*/
/*                                                                          */
/*  REVISION DATE                                                           */
/*      10-Jul-2001                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C callable, and has the following C prototype:      */
/*                                                                          */
/*              void IMG_errdif_bin                                         */
/*              (                                                           */
/*                  unsigned char *restrict errdif_data,                    */
/*                  int           cols,                                     */
/*                  int           rows,                                     */
/*                  short         *restrict err_buf,                        */
/*                  unsigned char thresh                                    */
/*              )                                                           */
/*                                                                          */
/*      errdif_data:     Input/Output image ptr                             */
/*      cols:            Number of columns (Width)                          */
/*      rows:            Number of rows    (Height)                         */
/*      err_buf[cols+1]: Buffer where one row of errors is to be saved      */
/*      thresh:          Threshold in the range [0x00, 0xFF]                */
/*                                                                          */
/*      errdif_data[] is used for both input and output.                    */
/*                                                                          */
/*      err_buf[], additional buffer, should be provided with               */
/*      initialized to all-zero's for the first call with an image.         */
/*      The subsequent call with the same image should provide this         */
/*      kernel the returned err_buf The size of err_buf should be           */
/*      (cols+1)*Half-Word.                                                 */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The code implements the Binary Floyd-Steinberg error diffusion      */
/*      filter.  The filter kernel used is this one:                        */
/*                                                                          */
/*                                  +---+                                   */
/*                                P | 7 |                                   */
/*                          +---+---+---+                                   */
/*                          | 3 | 5 | 1 |                                   */
/*                          +---+---+---+                                   */
/*                                                                          */
/*                                                                          */
/*      Pixels are processed from left-to-right, top-to-bottom.  Each       */
/*      pixel is compared against a user-defined threshold.  Pixels         */
/*      that are larger than the threshold are set to 255, and pixels       */
/*      that are smaller or equal to the threshold are set to 0.  The       */
/*      error value for the pixel (eg. the difference between the           */
/*      thresholded pixel and its original grey level) is propagated to     */
/*      the neighboring pixels according to the filter above.  This         */
/*      error propagation diffuses the error over a larger area, hence      */
/*      the term "error diffusion."                                         */
/*                                                                          */
/*  TECHNIQUES                                                              */
/*      The processing of the filter itself is inverted so that the         */
/*      errors from previous pixels "propagate into" a given pixel at       */
/*      the time the pixel is processed, rather than "accumulate into"      */
/*      a pixel as its neighbors are processed.  This allows us to          */
/*      keep our image as an 8-bit image, and reduces the number of         */
/*      accesses to the image array.  The inverted filter kernel            */
/*      performs identically to the kernel's original form.  In this        */
/*      form, the weights specify the weighting assigned to the errors      */
/*      coming from the neighboring pixels.                                 */
/*                                                                          */
/*                          +---+---+---+                                   */
/*                          | 1 | 5 | 3 |                                   */
/*                          +---+---+---+                                   */
/*                          | 7 | P                                         */
/*                          +---+                                           */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      err_buf[] must be initialized to zeros for the first call and       */
/*      the returned err_buf should be provided for the next call.          */
/*                                                                          */
/*  SOURCE                                                                  */
/*      Floyd-Steinberg Error Diffusion.                                    */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

#include "IMG_errdif_bin.h"

void IMG_errdif_bin
(
    unsigned char *restrict errdif_data, /* Input/Output image ptr       */
    int           cols,                  /* Number of columns (Width)    */
    int           rows,                  /* Number of rows    (Height)   */
    short         *restrict err_buf,     /* row-to-row error buffer.     */
    unsigned char thresh                 /* Threshold from [0x00, 0xFF]  */
)
{
    int   x, i, y;                /* Loop counters                       */
    int   F;                      /* Current pixel value at [x,y]        */
    int   errA;                   /* Error value at [x-1, y-1]           */
    int   errB;                   /* Error value at [  x, y-1]           */
    int   errC;                   /* Error value at [x+1, y-1]           */
    int   errE;                   /* Error value at [x-1,   y]           */
    int   errF;                   /* Error value at [  x,   y]           */

    /* --------------------------------------------------------- */
    /*  Step through rows of pixels.                             */
    /* --------------------------------------------------------- */
    for (y = 0, i = 0; y < rows; y++)
    {
        /* ----------------------------------------------------- */
        /*  Start off with our initial errors set to zero at the */
        /*  start of the line since we do not have any pixels to */
        /*  the left of the row.  These error terms are          */
        /*  maintained within the inner loop.                    */
        /* ----------------------------------------------------- */
        errA = 0; errE = 0;
        errB = err_buf[0];


        /* ----------------------------------------------------- */
        /*  Step through pixels in each row.                     */
        /* ----------------------------------------------------- */
        for (x = 0; x < cols; x++, i++)
        {
            /* ------------------------------------------------- */
            /*  Load the error being propagated from pixel 'C'   */
            /*  from our error buffer.  This was calculated      */
            /*  during the previous line.                        */
            /* ------------------------------------------------- */
            errC = err_buf[x+1];

            /* ------------------------------------------------- */
            /*  Load our pixel value to quantize.                */
            /* ------------------------------------------------- */
            F = errdif_data[i];

            /* ------------------------------------------------- */
            /*  Calculate our resulting pixel.  If we assume     */
            /*  that this pixel will be set to zero, this also   */
            /*  doubles as our error term.                       */
            /* ------------------------------------------------- */
            errF = F + ((errE*7 + errA + errB*5 + errC*3) >> 4);

            /* ------------------------------------------------- */
            /*  Set pixels that are larger than the threshold to */
            /*  255, and pixels that are smaller than the        */
            /*  threshold to 0.                                  */
            /* ------------------------------------------------- */
            if (errF > thresh)  errdif_data[i] = 0xFF;
            else                errdif_data[i] = 0;

            /* ------------------------------------------------- */
            /*  If the pixel was larger than the threshold, then */
            /*  we need subtract 255 from our error.  In any     */
            /*  case, store the error to the error buffer.       */
            /* ------------------------------------------------- */
            if (errF > thresh)  err_buf[x] = errF = errF - 0xFF;
            else                err_buf[x] = errF;

            /* ------------------------------------------------- */
            /*  Propagate error terms for the next pixel.        */
            /* ------------------------------------------------- */
            errE = errF;
            errA = errB;
            errB = errC;
        }
    }
}

/* ========================================================================= */
/*  End of file:  img_errdif_bin.c                                           */
/* ------------------------------------------------------------------------- */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ========================================================================= */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.4     Thu Jul 19 22:35:27 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  NAME                                                                    */
/*      IMG_fdct_8x8 -- 8x8 Block FDCT With Rounding, Endian Neutral.       */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      19-Oct-2000                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine has the following C prototype:                         */
/*                                                                          */
/*          void IMG_fdct_8x8(short fdct_data[], unsigned num_fdcts)        */
/*                                                                          */
/*      The fdct routine accepts a list of 8x8 pixel blocks and performs    */
/*      FDCTs on each.  The array should be laid out identically to         */
/*      "fdct_data[num_fdcts][8][8]".  All operations in this array are     */
/*      performed entirely in-place.                                        */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The IMG_fdct_8x8 function implements a Chen FDCT. Output values are */
/*      rounded, providing improved accuracy.  Input terms are expected     */
/*      to be signed 12Q0 values, producing signed 16Q0 results.  (A        */
/*      smaller dynamic range may be used on the input, producing a         */
/*      correspondingly smaller output range.  Typical applications         */
/*      include processing signed 9Q0 and unsigned 8Q0 pixel data,          */
/*      producing signed 13Q0 or 12Q0 outputs, respectively.)               */
/*                                                                          */
/*      Note:  This code guarantees correct operation, even in the case     */
/*      that 'num_fdcts == 0'.  In this case, the function performs an      */
/*      early exit without storing any results or intermediate values.      */
/*                                                                          */
/*  TECHNIQUES                                                              */
/*      I've applied a sqrt(2) scaling to each pass, which results in a     */
/*      fractional Q point at various points of the algorithm.  A chief     */
/*      benefit of this technique is that the F0 and F4 terms see no        */
/*      multiplies.  To perform this scaling, the C2 and C6 cosine terms    */
/*      are scaled up by sqrt(2), and the intermediate values in stage 2    */
/*      (s0, q0, s1, and q1) are also multiplied by sqrt(2).  The           */
/*      effect in stage 2 is to cancel the multiply by C4 on s0 and q0,     */
/*      and to introduce a multiply by 2*C4 on s1 and q1.                   */
/*                                                                          */
/*      The horizontal pass does not explicitly jump from block-to-block,   */
/*      since all horizontal rows are adjacent in memory.  (eg. Moving      */
/*      between rows within a block is identical to moving between the      */
/*      last row of one block and the first row of the next block.)         */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      n/a                                                                 */
/*                                                                          */
/*  NOTES                                                                   */
/*      n/a                                                                 */
/*                                                                          */
/*  SOURCE                                                                  */
/*      Chen FDCT.                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_fdct_8x8(short *dct_data, unsigned num_fdcts)
{
    /* -------------------------------------------------------------------- */
    /*  Set up the cosine coefficients.                                     */
    /* -------------------------------------------------------------------- */
    const unsigned short c1 = 0x1F62, c3 = 0x1A9B;      /* Q13   coeffs     */
    const unsigned short c5 = 0x11C7, c7 = 0x063E;      /* Q13   coeffs     */
    const unsigned short c2 = 0x29CF, c6 = 0x1151;      /* Q13.5 coeffs     */
    const unsigned short C1 = 0xFB15, C3 = 0xD4DB;      /* Q16   coeffs     */
    const unsigned short C5 = 0x8E3A, C7 = 0x31F1;      /* Q16   coeffs     */
    const unsigned short C2 = 0xA73D, C6 = 0x4546;      /* Q15.5 coeffs     */
    const unsigned short C4 = 0xB505;                   /* Q16   coeff      */

    /* -------------------------------------------------------------------- */
    /*  Intermediate calculations.                                          */
    /* -------------------------------------------------------------------- */
    short f0, f1, f2, f3, f4, f5, f6, f7;   /* Spatial domain samples.      */
    short g0, g1, h0, h1, p0, p1;           /* Even-half intermediate.      */
    short r0, r1, r0_,r1_;                  /* Even-half intermediate.      */
    short P0, P1, R0, R1;                   /* Even-half intermediate.      */
    short g2, g3, h2, h3;                   /* Odd-half intermediate.       */
    short q1a,s1a,q0, q1, s0, s1;           /* Odd-half intermediate.       */
    short Q0, Q1, S0, S1;                   /* Odd-half intermediate.       */
    short F0, F1, F2, F3, F4, F5, F6, F7;   /* Freq. domain results.        */

    /* -------------------------------------------------------------------- */
    /*  Input and output pointers, loop control.                            */
    /* -------------------------------------------------------------------- */
    unsigned i, j;
    short (*dct)[8][8] = (short (*)[8][8])dct_data;

    if (!num_fdcts) return;

    /* -------------------------------------------------------------------- */
    /*  Outer vertical loop -- Process each 8x8 block.                      */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < num_fdcts; i++)
    {
        /* ---------------------------------------------------------------- */
        /*  Perform Vertical 1-D FDCT on columns within each block.         */
        /* ---------------------------------------------------------------- */
        for (j = 0; j < 8; j++)
        {
            /* ------------------------------------------------------------ */
            /*  Load the spatial-domain samples.                            */
            /*  The incoming terms start at Q0 precision.                   */
            /* ------------------------------------------------------------ */
            f0 = dct[i][0][j];
            f1 = dct[i][1][j];
            f2 = dct[i][2][j];
            f3 = dct[i][3][j];
            f4 = dct[i][4][j];
            f5 = dct[i][5][j];
            f6 = dct[i][6][j];
            f7 = dct[i][7][j];

            /* ------------------------------------------------------------ */
            /*  Stage 1:  Separate into even and odd halves.                */
            /*                                                              */
            /*  The results of this stage are implicitly in Q1, since we    */
            /*  do not explicitly multiply by 0.5.                          */
            /* ------------------------------------------------------------ */
            g0 = f0 + f7;               g1 = f1 + f6;   /* Results in Q1    */
            h1 = f2 + f5;               h0 = f3 + f4;   /* Results in Q1    */
            g3 = f2 - f5;               g2 = f3 - f4;   /* Results in Q1    */
            h2 = f0 - f7;               h3 = f1 - f6;   /* Results in Q1    */

            /* ------------------------------------------------------------ */
            /*  Stage 2                                                     */
            /*                                                              */
            /*  Note, on the odd-half, the results are in Q1.5 since those  */
            /*  values are scaled upwards by sqrt(2) at this point.         */
            /* ------------------------------------------------------------ */
            p0 = g0 + h0;               r0 = g0 - h0;   /* Results in Q1    */
            p1 = g1 + h1;               r1 = g1 - h1;   /* Results in Q1    */

            q1a = g2 + g2;                              /* q1a is now Q2    */
            s1a = h2 + h2;                              /* s1a is now Q2    */
            q1  = (q1a * C4 + 0x8000) >> 16;            /* Results in Q1.5  */
            s1  = (s1a * C4 + 0x8000) >> 16;            /* Results in Q1.5  */

            s0 = h3 + g3;                               /* Results in Q1.5  */
            q0 = h3 - g3;                               /* Results in Q1.5  */

            /* ------------------------------------------------------------ */
            /*  Stage 3                                                     */
            /*                                                              */
            /*  Now, the even-half ends up in Q1.5.  On P0 and P1, this     */
            /*  happens because the multiply-by-C4 was canceled with an     */
            /*  upward scaling by sqrt(2).  On R0 and R1, this happens      */
            /*  because C2 and C6 are at Q15.5, and we scale r0 and r1 to   */
            /*  Q2 before we multiply.                                      */
            /* ------------------------------------------------------------ */
            P0 = p0 + p1;                               /* Results in Q1.5  */
            P1 = p0 - p1;                               /* Results in Q1.5  */

            r0_= r0 + r0;                               /* r0_ is now Q2    */
            r1_= r1 + r1;                               /* r1_ is now Q2    */
            R1 = (C6 * r1_+ C2 * r0_+ 0x8000) >> 16;    /* Results in Q1.5  */
            R0 = (C6 * r0_- C2 * r1_+ 0x8000) >> 16;    /* Results in Q1.5  */

            Q1 = q1 + q0;               Q0 = q1 - q0;
            S1 = s1 + s0;               S0 = s1 - s0;

            /* ------------------------------------------------------------ */
            /*  Stage 4                                                     */
            /*  No further changes in Q-point happen here.                  */
            /* ------------------------------------------------------------ */
            F0 = P0;                    F4 = P1;
            F2 = R1;                    F6 = R0;

            F1 = (C7 * Q1 + C1 * S1 + 0x8000) >> 16;    /* Results in Q1.5  */
            F7 = (C7 * S1 - C1 * Q1 + 0x8000) >> 16;    /* Results in Q1.5  */
            F5 = (C3 * Q0 + C5 * S0 + 0x8000) >> 16;    /* Results in Q1.5  */
            F3 = (C3 * S0 - C5 * Q0 + 0x8000) >> 16;    /* Results in Q1.5  */

            /* ------------------------------------------------------------ */
            /*  Store the frequency domain results.                         */
            /*  These values are all at Q1.5 precision.                     */
            /* ------------------------------------------------------------ */
            dct[i][0][j] = F0;
            dct[i][1][j] = F1;
            dct[i][2][j] = F2;
            dct[i][3][j] = F3;
            dct[i][4][j] = F4;
            dct[i][5][j] = F5;
            dct[i][6][j] = F6;
            dct[i][7][j] = F7;
        }
    }

    /* -------------------------------------------------------------------- */
    /*  Perform Horizontal 1-D FDCT on each 8x8 block.                      */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < num_fdcts; i++)
    {
        /* ---------------------------------------------------------------- */
        /*  Perform Vertical 1-D FDCT on columns within each block.         */
        /* ---------------------------------------------------------------- */
        for (j = 0; j < 8; j++)
        {
            /* ------------------------------------------------------------ */
            /*  Load the spatial-domain samples.                            */
            /*  The incoming terms are at Q1.5 precision from first pass.   */
            /* ------------------------------------------------------------ */
            f0 = dct[i][j][0];
            f1 = dct[i][j][1];
            f2 = dct[i][j][2];
            f3 = dct[i][j][3];
            f4 = dct[i][j][4];
            f5 = dct[i][j][5];
            f6 = dct[i][j][6];
            f7 = dct[i][j][7];

            /* ------------------------------------------------------------ */
            /*  Stage 1:  Separate into even and odd halves.                */
            /*                                                              */
            /*  The results of this stage are implicitly in Q2.5, since we  */
            /*  do not explicitly multiply by 0.5.                          */
            /* ------------------------------------------------------------ */
            g0 = f0 + f7;               g1 = f1 + f6;   /* Results in Q2.5  */
            h1 = f2 + f5;               h0 = f3 + f4;   /* Results in Q2.5  */
            h2 = f0 - f7;               h3 = f1 - f6;   /* Results in Q2.5  */
            g3 = f2 - f5;               g2 = f3 - f4;   /* Results in Q2.5  */

            /* ------------------------------------------------------------ */
            /*  Stage 2                                                     */
            /*                                                              */
            /*  Note, on the odd-half, the results are in Q3 since those    */
            /*  values are scaled upwards by sqrt(2) at this point.  The    */
            /*  order of operations differs in this pass as compared to     */
            /*  the first due to overflow concerns.                         */
            /*                                                              */
            /*  We also inject a rounding term into the DC term which will  */
            /*  also round the Nyquist term, F4.  This trick works despite  */
            /*  the fact that we are technically still at Q2.5 here, since  */
            /*  the step from Q2.5 to Q3 later is done implicitly, rather   */
            /*  than with a multiply.  (This is due to the sqrt(2) terms    */
            /*  cancelling on the P0/P1 butterfly.)                         */
            /* ------------------------------------------------------------ */
            p0 = g0 + h0 + 4;           p1 = g1 + h1;   /* Results in Q2.5  */
            r0 = g0 - h0;               r1 = g1 - h1;   /* Results in Q2.5  */

            q1a= (g2 * C4 + 0x8000) >> 16;              /* q1a now in Q2    */
            s1a= (h2 * C4 + 0x8000) >> 16;              /* s1a now in Q2    */
            q1 = q1a + q1a;                             /* Results in Q3    */
            s1 = s1a + s1a;                             /* Results in Q3    */

            s0 = h3 + g3;                               /* Results in Q3    */
            q0 = h3 - g3;                               /* Results in Q3    */

            /* ------------------------------------------------------------ */
            /*  Stage 3                                                     */
            /*                                                              */
            /*  Now, the even-half ends up in Q0.  This happens on P0 and   */
            /*  P1 because the multiply-by-c4 was canceled with an upward   */
            /*  scaling by sqrt(2), yielding Q3 intermediate value.  The    */
            /*  final >> 3 leaves these at Q0.  On R0 and R1, this happens  */
            /*  because c2 and c6 are at Q13.5, giving a Q16 intermediate   */
            /*  value.  The final >> 16 then leaves those values at Q0.     */
            /* ------------------------------------------------------------ */
            P0 = ((short)(p0 + p1)) >> 3;               /* Results in Q0    */
            P1 = ((short)(p0 - p1)) >> 3;               /* Results in Q0    */
            R1 = (c6 * r1 + c2 * r0 + 0x8000) >> 16;    /* Results in Q0    */
            R0 = (c6 * r0 - c2 * r1 + 0x8000) >> 16;    /* Results in Q0    */

            Q1 = q1 + q0;               Q0 = q1 - q0;   /* Results in Q3    */
            S1 = s1 + s0;               S0 = s1 - s0;   /* Results in Q3    */

            /* ------------------------------------------------------------ */
            /*  Stage 4                                                     */
            /*                                                              */
            /*  Next, the odd-half ends up in Q0.  This happens because     */
            /*  our values are in Q3 and our cosine terms are in Q13,       */
            /*  giving us Q16 intermediate values. The final >> 16 leaves   */
            /*  us a Q0 result.                                             */
            /* ------------------------------------------------------------ */
            F0 = P0;                    F4 = P1;
            F2 = R1;                    F6 = R0;

            F1 = (c7 * Q1 + c1 * S1 + 0x8000) >> 16;    /* Results in Q0    */
            F7 = (c7 * S1 - c1 * Q1 + 0x8000) >> 16;    /* Results in Q0    */
            F5 = (c3 * Q0 + c5 * S0 + 0x8000) >> 16;    /* Results in Q0    */
            F3 = (c3 * S0 - c5 * Q0 + 0x8000) >> 16;    /* Results in Q0    */

            /* ------------------------------------------------------------ */
            /*  Store the results                                           */
            /* ------------------------------------------------------------ */
            dct[i][j][0] = F0;
            dct[i][j][1] = F1;
            dct[i][j][2] = F2;
            dct[i][j][3] = F3;
            dct[i][j][4] = F4;
            dct[i][j][5] = F5;
            dct[i][j][6] = F6;
            dct[i][j][7] = F7;
        }
    }

    return;
}

/* ======================================================================== */
/*  End of file:  img_fdct_8x8.c                                            */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.4     Thu Jul 19 22:35:23 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_histogram                                                       */
/*                                                                          */
/*   REVISION DATE                                                          */
/*       02-Oct-2000                                                        */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*       void IMG_histogram(unsigned char * image, int  n, int  accumulate, */
/*                         unsigned short * t_hist, unsigned short * hist)  */
/*                                                                          */
/*           image      =  input image data                                 */
/*           n          =  number of points                                 */
/*           accumulate =  defines addition/subtract from existing          */
/*                         IMG_histogram: 1, -1                             */
/*           t_hist     =  temporary IMG_histogram bins (1024)              */
/*           hist       =  running IMG_histogram bins (256)                 */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      This code takes a IMG_histogram of an array of n, 8 bit inputs. It  */
/*      returns the IMG_histogram of 256 bins at 16 bit precision.  It can  */
/*      either add or subtract to an existing IMG_histogram, using the      */
/*      'accumulate' control.  It requires some temporary storage for 4     */
/*      temporary histograms, which are later summed together.              */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      It is assumed that the array of data, t_hist is initialized         */
/*      to zero.  The input array of image data must be aligned to an 8     */
/*      byte boundary and n must be a multiple of 8.  The maximum number    */
/*      of pixels that can be profiled in each bin is 65535 in the main     */
/*      IMG_histogram and the maximum n is 262143.                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_histogram
(
    const unsigned char *restrict image,
    int   n,
    short accumulate,
    short *restrict t_hist,
    short *restrict hist
)
#if 1
{
    /* -------------------------------------------------------------------- */
    /*  This is pretty much bit-exact with our optimized implementation.    */
    /* -------------------------------------------------------------------- */
    int p0, p1, p2, p3, i;

    /* -------------------------------------------------------------------- */
    /*  This loop is unrolled four times, producing four interleaved        */
    /*  histograms into a temporary buffer.                                 */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < n; i += 4)
    {
        p0 = image[i + 0] * 4 + 0;
        p1 = image[i + 1] * 4 + 1;
        p2 = image[i + 2] * 4 + 2;
        p3 = image[i + 3] * 4 + 3;

        t_hist[p0]++;
        t_hist[p1]++;
        t_hist[p2]++;
        t_hist[p3]++;
    }

    /* -------------------------------------------------------------------- */
    /*  Accumulate the interleaved histograms back into the final           */
    /*  IMG_histogram buffer.                                               */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < 256; i++)
    {
        hist[i] += (t_hist[i*4 + 0] +
                    t_hist[i*4 + 1] +
                    t_hist[i*4 + 2] +
                    t_hist[i*4 + 3]) * accumulate;
    }
}
#else
{
    /* -------------------------------------------------------------------- */
    /*  This is the cannonical form for Histogram.  We cannot use this      */
    /*  here, as it does not use the temporary array "t_hist", and that's   */
    /*  an "output" of our code.                                            */
    /* -------------------------------------------------------------------- */
    int pixel, j;
    for (j = 0; j < n; j++)
    {
        pixel = (int) image[j];
        hist[pixel] += accumulate;
    }
}
#endif
/* ======================================================================== */
/*  End of file:  img_histogram.c                                           */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.5     Tue Aug 28 14:51:03 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  NAME                                                                    */
/*      IMG_idct_8x8_12q4 -- IEEE-1180/1990 Compliant IDCT, Little Endian.  */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      19-Oct-2000                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C callable, and has the following C prototype:      */
/*                                                                          */
/*          void IMG_idct_8x8_12q4(short idct_data[], unsigned num_idcts)   */
/*                                                                          */
/*      The IMG_idct_8x8_12q4 routine accepts a list of 8x8 DCT coeffient blocks */
/*      and performs IDCTs on each.  The array should be laid out           */
/*      equivalently to the C array idct_data[num_idcts][8][8].  The        */
/*      input data should be in 12Q4 format.                                */
/*                                                                          */
/*      The routine operates entirely in-place, requiring no additional     */
/*      storage for intermediate results.                                   */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The IMG_idct_8x8_12q4 algorithm performs an IEEE-1180 compliant IDCT, */
/*      complete with rounding and saturation to signed 9-bit quantities.   */
/*      The input coefficients are assumed to be signed 16-bit DCT          */
/*      coefficients in 12Q4 format.                                        */
/*                                                                          */
/*      Note:  This code guarantees correct operation, even in the case     */
/*      that 'num_idcts == 0'.                                              */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      This is a LITTLE ENDIAN implementation.                             */
/*                                                                          */
/*  NOTES                                                                   */
/*      The cosine terms have all been scaled by sqrt(2), so that the       */
/*      "c4" term is basically an even power of 2.                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  IMG_idct_8x8_12q4   -- Natural C version of IMG_idct_8x8_12q4().        */
/* ======================================================================== */
void IMG_idct_8x8_12q4(short *idct_data, unsigned num_idcts)
{
    /* -------------------------------------------------------------------- */
    /*  Cosine Constants (Q16, scaled down by sqrt(2)).                     */
    /* -------------------------------------------------------------------- */
    const unsigned short C0 = 0xB505;
    const unsigned short C1 = 0xB18B, C2 = 0xA73D;
    const unsigned short C3 = 0x9683, C5 = 0x6492;
    const unsigned short C6 = 0x4546, C7 = 0x2351;

    /* -------------------------------------------------------------------- */
    /*  Intermediate values (used in both loops).                           */
    /* -------------------------------------------------------------------- */
    short F0, F1, F2, F3, F4, F5, F6, F7;  /* stage 0        */
    short P0, P1, R0, R1, Q0, Q1, S0, S1;  /* stage 1        */
    short p0, p1, r0, r1, q0, q1, s0, s1;  /* stage 2        */
    short g0, g1, g2, g3, h0, h1, h2, h3;  /* stage 3        */
    short f0, f1, f2, f3, f4, f5, f6, f7;  /* stage 4        */
    short f0r,f1r,f2r,f3r,f4r,f5r,f6r,f7r; /* rounded        */
    int   f0s,f1s,f2s,f3s,f4s,f5s,f6s,f7s; /* saturated      */
    int   f0t,f1t,f2t,f3t,f4t,f5t,f6t,f7t; /* truncated      */
    int   i, j;                            /* loop counts    */
    short (*idct)[8][8] = (short (*)[8][8])idct_data;

    if (!num_idcts) return;

    /* -------------------------------------------------------------------- */
    /*  Vertical Pass                                                       */
    /*                                                                      */
    /*  This pass performs a single 8-pt IDCT per iteration.  Inputs        */
    /*  are in 12Q4 format, and results of this pass are in 11Q5            */
    /*  format. (Actually, the results are halfway between 11Q5 and         */
    /*  12Q4 due to the scaling by sqrt(2).)                                */
    /*                                                                      */
    /*  The outer loop steps between IDCT blocks, whereas the inner         */
    /*  loop focuses on columns within each IDCT block.                     */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < num_idcts; i++)
    {
        for (j = 0; j < 8; j++)
        {
            /* ------------------------------------------------------------ */
            /*  Stage 0:  Load in frequency-domain coefficients.            */
            /* ------------------------------------------------------------ */
            F0 = idct[i][0][j];
            F1 = idct[i][1][j];
            F2 = idct[i][2][j];
            F3 = idct[i][3][j];
            F4 = idct[i][4][j];
            F5 = idct[i][5][j];
            F6 = idct[i][6][j];
            F7 = idct[i][7][j];

            /* ------------------------------------------------------------ */
            /*  Stage 1 of signal flow graph.                               */
            /* ------------------------------------------------------------ */
            P0 = F0;                P1 = F4;
            R1 = F2;                R0 = F6;

            Q1 = (F1*C7 - F7*C1 + 0x8000) >> 16;
            Q0 = (F5*C3 - F3*C5 + 0x8000) >> 16;
            S0 = (F5*C5 + F3*C3 + 0x8000) >> 16;
            S1 = (F1*C1 + F7*C7 + 0x8000) >> 16;

            /* ------------------------------------------------------------ */
            /*  Stage 2 of signal flow graph.                               */
            /* ------------------------------------------------------------ */
            p0 = ((int)P0 + (int)P1 + 1 ) >> 1;
            p1 = ((int)P0 - (int)P1     ) >> 1;
            r1 = (R1*C6 - R0*C2 + 0x8000) >> 16;
            r0 = (R1*C2 + R0*C6 + 0x8000) >> 16;

            s1 = (S1 + S0);         q1 = (Q1 + Q0);
            s0 = (S1 - S0);         q0 = (Q1 - Q0);

            /* ------------------------------------------------------------ */
            /*  Stage 3 of signal flow graph.                               */
            /* ------------------------------------------------------------ */
            g0 = (p0 + r0);         g1 = (p1 + r1);
            h0 = (p0 - r0);         h1 = (p1 - r1);

            h2 = s1;                g2 = q1;
            g3 = (s0*C0 - q0*C0 + 0x8000) >> 16;
            h3 = (s0*C0 + q0*C0 + 0x8000) >> 16;

            /* ------------------------------------------------------------ */
            /*  Stage 4 of signal flow graph.                               */
            /* ------------------------------------------------------------ */
            f0 = (g0 + h2);         f7 = (g0 - h2);
            f1 = (g1 + h3);         f6 = (g1 - h3);
            f2 = (h1 + g3);         f5 = (h1 - g3);
            f3 = (h0 + g2);         f4 = (h0 - g2);

            /* ------------------------------------------------------------ */
            /*  Stage 5:  Write sample-domain results.                      */
            /* ------------------------------------------------------------ */
            idct[i][0][j] = f0;
            idct[i][1][j] = f1;
            idct[i][2][j] = f2;
            idct[i][3][j] = f3;
            idct[i][4][j] = f4;
            idct[i][5][j] = f5;
            idct[i][6][j] = f6;
            idct[i][7][j] = f7;
        }
    }

    /* -------------------------------------------------------------------- */
    /*  Horizontal Pass                                                     */
    /*                                                                      */
    /*  This performs one IDCT per iteration on the 11Q5 results from       */
    /*  the previous pass.  Both horizontal and vertical passes are         */
    /*  scaled down by sqrt(2) -- the net effect of which is that the       */
    /*  IDCT results generated by this pass (prior to saturation) are       */
    /*  also 11Q5 results, only with no sqrt(2) factors remaining.          */
    /*                                                                      */
    /*  The IDCT butterflies in this pass are identical to the ones in      */
    /*  the vertical pass, except for an additional rounding value          */
    /*  which is added into the DC term early in the flow graph.            */
    /*                                                                      */
    /*  The 11Q5 sample-domain terms are saturated to 9Q7 values, and       */
    /*  then truncated to 9Q0 results before storing.                       */
    /*                                                                      */
    /*  The outer loop steps between IDCT blocks, whereas the inner         */
    /*  loop focuses on rows within each IDCT block.                        */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < num_idcts; i++)
    {
        for (j = 0; j < 8; j++)
        {
            /* ------------------------------------------------------------ */
            /*  Stage 0:  Load in frequency-domain coefficients.            */
            /* ------------------------------------------------------------ */
            F0 = idct[i][j][0];
            F1 = idct[i][j][1];
            F2 = idct[i][j][2];
            F3 = idct[i][j][3];
            F4 = idct[i][j][4];
            F5 = idct[i][j][5];
            F6 = idct[i][j][6];
            F7 = idct[i][j][7];

            /* ------------------------------------------------------------ */
            /*  Stage 1 of signal flow graph.                               */
            /* ------------------------------------------------------------ */
            P0 = F0;                P1 = F4;
            R1 = F2;                R0 = F6;

            Q1 = (F1*C7 - F7*C1 + 0x8000) >> 16;
            Q0 = (F5*C3 - F3*C5 + 0x8000) >> 16;
            S0 = (F5*C5 + F3*C3 + 0x8000) >> 16;
            S1 = (F1*C1 + F7*C7 + 0x8000) >> 16;

            /* ------------------------------------------------------------ */
            /*  Stage 2 of signal flow graph.                               */
            /* ------------------------------------------------------------ */
            p0 = (((int)P0 + (int)P1 + 1) >> 1) + 15;
            p1 = (((int)P0 - (int)P1    ) >> 1) + 16;
            r1 = (R1*C6 - R0*C2 + 0x8000) >> 16;
            r0 = (R1*C2 + R0*C6 + 0x8000) >> 16;

            s1 = (S1 + S0);         q1 = (Q1 + Q0);
            s0 = (S1 - S0);         q0 = (Q1 - Q0);

            /* ------------------------------------------------------------ */
            /*  Stage 3 of signal flow graph.                               */
            /* ------------------------------------------------------------ */
            g0 = (p0 + r0);         g1 = (p1 + r1);
            h0 = (p0 - r0);         h1 = (p1 - r1);

            h2 = s1;                g2 = q1;
            g3 = (s0*C0 - q0*C0 + 0x8000) >> 16;
            h3 = (s0*C0 + q0*C0 + 0x8000) >> 16;

            /* ------------------------------------------------------------ */
            /*  Stage 4 of signal flow graph.                               */
            /* ------------------------------------------------------------ */
            f0 = (g0 + h2);         f7 = (g0 - h2);
            f1 = (g1 + h3);         f6 = (g1 - h3);
            f2 = (h1 + g3);         f5 = (h1 - g3);
            f3 = (h0 + g2);         f4 = (h0 - g2);

            /* ------------------------------------------------------------ */
            /*  Stage 4.1:  Q-pt adjust: Bit 15 is a don't-care.            */
            /* ------------------------------------------------------------ */
            f0r = f0 + f0;          f7r = f7 + f7;
            f1r = f1 + f1;          f6r = f6 + f6;
            f2r = f2 + f2;          f5r = f5 + f5;
            f3r = f3 + f3;          f4r = f4 + f4;

            /* ------------------------------------------------------------ */
            /*  Stage 4.2:  Saturate results to 9Q6.                        */
            /* ------------------------------------------------------------ */
            f0s = f0r > 0x3FFF ? 0x3FFF : f0r < -0x4000 ? -0x4000 : f0r;
            f1s = f1r > 0x3FFF ? 0x3FFF : f1r < -0x4000 ? -0x4000 : f1r;
            f2s = f2r > 0x3FFF ? 0x3FFF : f2r < -0x4000 ? -0x4000 : f2r;
            f3s = f3r > 0x3FFF ? 0x3FFF : f3r < -0x4000 ? -0x4000 : f3r;
            f4s = f4r > 0x3FFF ? 0x3FFF : f4r < -0x4000 ? -0x4000 : f4r;
            f5s = f5r > 0x3FFF ? 0x3FFF : f5r < -0x4000 ? -0x4000 : f5r;
            f6s = f6r > 0x3FFF ? 0x3FFF : f6r < -0x4000 ? -0x4000 : f6r;
            f7s = f7r > 0x3FFF ? 0x3FFF : f7r < -0x4000 ? -0x4000 : f7r;

            /* ------------------------------------------------------------ */
            /*  Stage 4.3:  Truncate results to 9Q0.                        */
            /* ------------------------------------------------------------ */
            f0t = f0s >> 6;         f7t = f7s >> 6;
            f1t = f1s >> 6;         f6t = f6s >> 6;
            f2t = f2s >> 6;         f5t = f5s >> 6;
            f3t = f3s >> 6;         f4t = f4s >> 6;

            /* ------------------------------------------------------------ */
            /*  Stage 5:  Store sample-domain results.                      */
            /* ------------------------------------------------------------ */
            idct[i][j][0] = f0t;
            idct[i][j][1] = f1t;
            idct[i][j][2] = f2t;
            idct[i][j][3] = f3t;
            idct[i][j][4] = f4t;
            idct[i][j][5] = f5t;
            idct[i][j][6] = f6t;
            idct[i][j][7] = f7t;
        }
    }

    return;
}

/* ======================================================================== */
/*  End of file:  img_idct_8x8_12q4.c                                       */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.8     Sun Mar 10 01:01:06 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================= */
/*                                                                         */
/*  TEXAS INSTRUMENTS, INC.                                                */
/*                                                                         */
/*  NAME                                                                   */
/*      IMG_mad_16x16                                                      */
/*                                                                         */
/*  REVISION DATE                                                          */
/*      05-Jul-2001                                                        */
/*                                                                         */
/*  USAGE                                                                  */
/*      This routine is C-callable and can be called as:                   */
/*                                                                         */
/*      void IMG_mad_16x16                                                 */
/*      (                                                                  */
/*          const unsigned char *restrict refImg,                          */
/*          const unsigned char *restrict srcImg,                          */
/*          int pitch,                                                     */
/*          int                 h,                                         */
/*          int                 v,                                         */
/*          unsigned            *restrict match                            */
/*      )                                                                  */
/*                                                                         */
/*      refImg          Reference image.                                   */
/*      srcImg[256]     16x16 block image to look for.                     */
/*      pitch           Width of reference image.                          */
/*      h               Horiz. size of search area.                        */
/*      v               Vert.  size of search area. Must be multiple of 2. */
/*      match[2]        Result:                                            */
/*                          match[0] is packed x, y.                       */
/*                          match[1] is MAD value.                         */
/*                                                                         */
/*  DESCRIPTION                                                            */
/*      This routine returns the location of the minimum absolute          */
/*      difference between a 16x16 search block and some block in a        */
/*      (h + 16) x (v + 16) search area. h and v are the sizes of the      */
/*      search space for the top left coordinate of the search block.      */
/*      refImg points to the top left pixel of the search area.            */
/*                                                                         */
/*           (0,0)          (h,0)      (h+16,0)                            */
/*             ;--------------+--------;                                   */
/*             ;    search    |        ;                                   */
/*             ;    space     |        ;                                   */
/*             ;              |        ;        search area                */
/*             ;--------------+        ;        within reference image     */
/*           (0,v)          (h,v)      ;                                   */
/*             ;                       ;                                   */
/*             ;-----------------------;                                   */
/*           (0, v+16)                 (v+16,h+16)                         */
/*                                                                         */
/*      The location is returned relative to the above coordinate system   */
/*      as x and y packed in two 16-bit quantities in a 32-bit word:       */
/*                                                                         */
/*                  31             16 15             0                     */
/*                  +----------------+----------------+                    */
/*       match[0]:  |       x        |       y        |                    */
/*                  +----------------+----------------+                    */
/*                                                                         */
/*                  31                               0                     */
/*                  +---------------------------------+                    */
/*       match[1]:  |   SAD value at location x, y    |                    */
/*                  +---------------------------------+                    */
/*                                                                         */
/*  ASSUMPTIONS                                                            */
/*      srcImg and refImg do not alias in memory.                          */
/*      The routine is written for Little Endian configuration.            */
/*                                                                         */
/*  MEMORY NOTE                                                            */
/*      None.                                                              */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================= */

#include "IMG_mad_16x16.h"

void IMG_mad_16x16
(
    const unsigned char *restrict refImg,
    const unsigned char *restrict srcImg,
    int pitch,
    int h, int v,
    unsigned int *restrict match
)
{
    int i, j, x, y, matx, maty;
    unsigned matpos, matval;

    matval = ~0U;
    matx   = maty = 0;

    for (x = 0; x < h; x++)
        for (y = 0; y < v; y++)
        {
            unsigned acc = 0;

            for (i = 0; i < 16; i++)
                for (j = 0; j < 16; j++)
                    acc += abs(srcImg[i*16 + j] - refImg[(i+y)*pitch + x + j]);

            if (acc < matval)
            {
                matval = acc;
                matx   = x;
                maty   = y;
            }
        }

    matpos    = (0xffff0000 & (matx << 16)) | (0x0000ffff & maty);
    match[0] = matpos;
    match[1] = matval;
}

/* ======================================================================= */
/*  End of file:  img_mad_16x16.c                                          */
/* ----------------------------------------------------------------------- */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================= */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.7     Thu Jul 19 22:35:26 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  NAME                                                                    */
/*      IMG_mad_8x8 -- Minimum Absolute Difference motion search            */
/*                                                                          */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      20-Oct-2000                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C callable and has the following C prototype.       */
/*                                                                          */
/*      void IMG_mad_8x8                                                    */
/*      (                                                                   */
/*          const unsigned char *refImg,    // Ref. image to search //      */
/*          const unsigned char *srcImg,    // Source 8x8 block     //      */
/*          int                 pitch,      // Width of ref image   //      */
/*          int sx, int sy,                 // Search window size   //      */
/*          unsigned int        *motvec     // Motion vector result //      */
/*      );                                                                  */
/*                                                                          */
/*      This routine accepts an 8x8 source block and a pointer to           */
/*      a window to search within a bitmap.  The pointer "refImg"           */
/*      points to the upper left corner of the search window.  The          */
/*      parameters "sx" and "sy" describe the dimensions of the search      */
/*      area.  The bitmap itself may be wider than the search window.       */
/*      It's width is described by the "pitch" argument.                    */
/*                                                                          */
/*      The search area dimensions specify the range of positions that      */
/*      the 8x8 source block is compared to.  This means that the           */
/*      actual bitmap area examined extends 7 pixels beyond the right       */
/*      and bottom edges of the search area within the reference image.     */
/*                                                                          */
/*      The best match position and its absolute difference are returned    */
/*      in motvec, packed as follows:                                       */
/*                                                                          */
/*                     31             16 15             0                   */
/*                     +----------------+----------------+                  */
/*          motvec[0]: |    X offset    |    Y offset    |                  */
/*                     +----------------+----------------+                  */
/*                                                                          */
/*                     31                               0                   */
/*                     +---------------------------------+                  */
/*          motvec[1]: |   Sum of absolute differences   |                  */
/*                     +---------------------------------+                  */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The IMG_mad_8x8 function performs a full search for an 8x8 block    */
/*      within a specified search window.  It finds the position within     */
/*      the search window that has the Minimum Absolute Difference with     */
/*      respect to the given source block.                                  */
/*                                                                          */
/*      This type of search is useful for video algorithms which use        */
/*      motion compensation.  The search performed by this routine is a     */
/*      full search, meaning that all possible starting positions from      */
/*      [0, 0] to [sx-1, sy-1] are checked for potential matches.           */
/*                                                                          */
/*      The Absolute Difference metric is calculated by summing the         */
/*      absolute values of the differences between pixels in the            */
/*      source block and their corresponding pixels for the match           */
/*      point being evaluated in the reference image.  Smaller sums         */
/*      denote better matches--that is, less overall difference between     */
/*      the source block and match point in the reference block.            */
/*                                                                          */
/*      The algorithm returns the X and Y offsets of the best match         */
/*      point, as well as the calculated Absolute Difference for that       */
/*      position.  If two match points have equal Absolute Differences,     */
/*      the earlier block in the search sequence is returned.  The          */
/*      search presently checks in vertical stripes from top to bottom,     */
/*      moving from 1 column to the right after each stripe.                */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      The input pointers do not alias the output pointer for motvec.      */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

#include "IMG_mad_8x8.h"

void IMG_mad_8x8
(
    const unsigned char *restrict refImg,
    const unsigned char *restrict srcImg,
    int pitch, int sx, int sy,
    unsigned int *restrict motvec
)
{
    int i, j, x, y, matx, maty;
    unsigned matpos, matval;

    matval = ~0U;
    matx   = maty = 0;

    for (x = 0; x < sx; x++)
        for (y = 0; y < sy; y++)
        {
            unsigned acc = 0;

            for (i = 0; i < 8; i++)
                for (j = 0; j < 8; j++)
                    acc += abs(srcImg[i*8 + j] - refImg[(i+y)*pitch + x + j]);

            if (acc < matval)
            {
                matval = acc;
                matx   = x;
                maty   = y;
            }
        }

    matpos    = (0xffff0000 & (matx << 16)) | (0x0000ffff & maty);
    motvec[0] = matpos;
    motvec[1] = matval;
}

/* ======================================================================== */
/*  End of file:  img_mad_8x8.c                                             */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.5     Thu Mar 21 20:15:47 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_median_3x3                                                      */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      16-Oct-2000                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*          void IMG_median_3x3                                             */
/*          (                                                               */
/*             const unsigned char *restrict i_data, // Input image     //  */
/*             int n,                                // Length of input //  */
/*             unsigned char       *restrict o_data  // Output image    //  */
/*          )                                                               */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      This kernel performs a 3x3 median filter operation on 8-bit         */
/*      unsigned values.  The median filter comes under the class           */
/*      of non-linear signal processing algorithms.                         */
/*                                                                          */
/*      Rather than replace the grey level at a pixel by a weighted         */
/*      average of the nine pixels including and surrounding it, the        */
/*      grey level at each pixel is replaced by the median of the nine      */
/*      values.  The median of a set of nine numbers is the middle          */
/*      element so that half of the elements in the list are larger and     */
/*      half are smaller.  Median filters remove the effects of extreme     */
/*      values from data, such as salt and pepper noise, although using     */
/*      a wide may result in unacceptable blurring of sharp edges in        */
/*      the original image.                                                 */
/*                                                                          */
/*  C CODE                                                                  */
/*      The following is a C code description of the algorithm without      */
/*      restrictions.  The optimized implementations may have               */
/*      restrictions, as noted under the "ASSUMPTIONS" below.               */
/*                                                                          */
/*      void IMG_median_3x3                                                 */
/*      (                                                                   */
/*          const unsigned char *restrict i_data,                           */
/*          int n,                                                          */
/*          unsigned char       *restrict o_data                            */
/*      )                                                                   */
/*      {                                                                   */
/*          unsigned char c0h, c1h, c2h; // "hi",  columns 0..2 //          */
/*          unsigned char c0m, c1m, c2m; // "mid", columns 0..2 //          */
/*          unsigned char c0l, c1l, c2l; // "lo",  columns 0..2 //          */
/*          unsigned char h_min;         // "min" //                        */
/*          unsigned char m_mid;         // "mid" //                        */
/*          unsigned char l_max;         // "max" //                        */
/*          unsigned char m_h, m_l, t, out;                                 */
/*                                                                          */
/*          int i;                                                          */
/*                                                                          */
/*          // ---------------------------------------------------- //      */
/*          //  Start off with a well-defined initial state.        //      */
/*          // ---------------------------------------------------- //      */
/*          c1h = c2h = c1m = c2m = c1l = c2l = 127;                        */
/*                                                                          */
/*          // ---------------------------------------------------- //      */
/*          //  Iterate over the input row.                         //      */
/*          // ---------------------------------------------------- //      */
/*          for (i = 0; i < n; i++)                                         */
/*          {                                                               */
/*              // ------------------------------------------------ //      */
/*              //  Slide the two previous columns of sorted        //      */
/*              //  pixels over by 1.                               //      */
/*              // ------------------------------------------------ //      */
/*              c0h = c1h;    c1h = c2h;                                    */
/*              c0m = c1m;    c1m = c2m;                                    */
/*              c0l = c1l;    c1l = c2l;                                    */
/*                                                                          */
/*              // ------------------------------------------------ //      */
/*              //  Load in a new column of pixels, and sort into   //      */
/*              //  low, medium, high.                              //      */
/*              // ------------------------------------------------ //      */
/*              c2h = i_data[i      ];                                      */
/*              c2m = i_data[i +   n];                                      */
/*              c2l = i_data[i + 2*n];                                      */
/*                                                                          */
/*              if (c2l > c2h) { t = c2l; c2l = c2h; c2h = t; }             */
/*              if (c2l > c2m) { t = c2l; c2l = c2m; c2m = t; }             */
/*              if (c2m > c2h) { t = c2m; c2m = c2h; c2h = t; }             */
/*                                                                          */
/*              // ------------------------------------------------ //      */
/*              //  Find the minimum value of the "hi" terms.       //      */
/*              // ------------------------------------------------ //      */
/*              h_min = c2h;                                                */
/*              if (c1h < h_min) { h_min = c1h; }                           */
/*              if (c0h < h_min) { h_min = c0h; }                           */
/*                                                                          */
/*              // ------------------------------------------------ //      */
/*              //  Find the middle value of the "mid" terms.       //      */
/*              // ------------------------------------------------ //      */
/*              m_l   = c0m;                                                */
/*              m_mid = c1m;                                                */
/*              m_h   = c2m;                                                */
/*                                                                          */
/*              if (m_l   > m_h  ) { t = m_l; m_l = m_h; m_h = t; }         */
/*              if (m_l   > m_mid) { m_mid = m_l; }                         */
/*              if (m_mid > m_h  ) { m_mid = m_h; }                         */
/*                                                                          */
/*              // ------------------------------------------------ //      */
/*              //  Find the maximum value of the "lo" terms.       //      */
/*              // ------------------------------------------------ //      */
/*              l_max = c2l;                                                */
/*              if (c1l > l_max) { l_max = c1l; }                           */
/*              if (c0l > l_max) { l_max = c0l; }                           */
/*                                                                          */
/*              // ------------------------------------------------ //      */
/*              //  Find the middle value of "h_min", "m_mid",      //      */
/*              //  "l_max" into "out".                             //      */
/*              // ------------------------------------------------ //      */
/*              out = m_mid;                                                */
/*                                                                          */
/*              if (h_min > l_max) { t=h_min; h_min = l_max; l_max=t; }     */
/*              if (h_min > out  ) { out = h_min; }                         */
/*              if (out   > l_max) { out = l_max; }                         */
/*                                                                          */
/*              // ------------------------------------------------ //      */
/*              //  Store the resulting pixel.                      //      */
/*              // ------------------------------------------------ //      */
/*              o_data[i] = out;                                            */
/*          }                                                               */
/*      }                                                                   */
/*                                                                          */
/*  TECHNIQUES                                                              */
/*      This implementation uses an incremental sorting technique to        */
/*      greatly reduce the number of compares and exchanges necessary       */
/*      to sort the image pixels.                                           */
/*                                                                          */
/*      The main loop reads three new pixels from the input image each      */
/*      iteration.  These three pixels form the right edge of the filter    */
/*      mask.  The filter data from the previous iteration is "slid         */
/*      over" by one pixel to form the complete 3x3 mask.                   */
/*                                                                          */
/*      As 3-pixel is read in from the image, the pixels are sorted,        */
/*      resulting in a "lo", "medium" and "hi" pixel value for that         */
/*      column.  The result is that the filter mask is sorted into          */
/*      three rows -- a row of "minimums", a row of "middle values",        */
/*      and a row of "maximums".                                            */
/*                                                                          */
/*      The median filter operates from this partially ordered mask.        */
/*      It finds the smallest element in the row of "maximums",             */
/*      the middle element in the row of "middle values", and               */
/*      the largest element in the row of "minimums".  The median           */
/*      value of these three values is the median for the entire 3x3        */
/*      mask.                                                               */
/*                                                                          */
/*      This process minimizes compares, as the whole mask does not         */
/*      need to be sorted between iterations.  Rather, the partial          */
/*      ordering for two of the three columns from one iteration is         */
/*      used directly for the next iteration.                               */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  BIBLIOGRAPHY                                                            */
/*      Knuth, Donald E.  The_Art_of_Computer_Programming, Vol 3,           */
/*          Pg. 180:  "Minimum Comparison Sorting."                         */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_median_3x3
(
    const unsigned char *restrict i_data,
    int n,
    unsigned char       *restrict o_data
)
{
    unsigned char c0h, c1h, c2h, h_min;  /* "hi",  columns 0..2, and "min" */
    unsigned char c0m, c1m, c2m, m_mid;  /* "mid", columns 0..2, and "mid" */
    unsigned char c0l, c1l, c2l, l_max;  /* "lo",  columns 0..2, and "max" */
    unsigned char m_h, m_l, tmp, out;

    int i;

    /* -------------------------------------------------------------------- */
    /*  Start off with a well-defined initial state.                        */
    /* -------------------------------------------------------------------- */
    c1h = c2h = c1m = c2m = c1l = c2l = 127;

    /* -------------------------------------------------------------------- */
    /*  Iterate over the input row.                                         */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < n; i++)
    {
        /* ---------------------------------------------------------------- */
        /*  Slide our two previous columns of sorted pixels over by 1.      */
        /* ---------------------------------------------------------------- */
        c0h = c1h;    c1h = c2h;
        c0m = c1m;    c1m = c2m;
        c0l = c1l;    c1l = c2l;

        /* ---------------------------------------------------------------- */
        /*  Load in a new column of pixels, and sort into lo, med, high.    */
        /* ---------------------------------------------------------------- */
        c2h = i_data[i      ];
        c2m = i_data[i +   n];
        c2l = i_data[i + 2*n];

        if (c2l > c2h) { tmp = c2l; c2l = c2h; c2h = tmp; }
        if (c2l > c2m) { tmp = c2l; c2l = c2m; c2m = tmp; }
        if (c2m > c2h) { tmp = c2m; c2m = c2h; c2h = tmp; }

        /* ---------------------------------------------------------------- */
        /*  Find the minimum value of the "hi" terms.                       */
        /* ---------------------------------------------------------------- */
        h_min = c2h;
        if (c1h < h_min) { h_min = c1h; }
        if (c0h < h_min) { h_min = c0h; }

        /* ---------------------------------------------------------------- */
        /*  Find the middle value of the "mid" terms.                       */
        /* ---------------------------------------------------------------- */
        m_l   = c0m;
        m_mid = c1m;
        m_h   = c2m;

        if (m_l   > m_h  ) { tmp = m_l; m_l = m_h; m_h = tmp; }
        if (m_l   > m_mid) { m_mid = m_l; }
        if (m_mid > m_h  ) { m_mid = m_h; }

        /* ---------------------------------------------------------------- */
        /*  Find the maximum value of the "lo" terms.                       */
        /* ---------------------------------------------------------------- */
        l_max = c2l;
        if (c1l > l_max) { l_max = c1l; }
        if (c0l > l_max) { l_max = c0l; }

        /* ---------------------------------------------------------------- */
        /*  Find the middle value of "h_min", "m_mid", "l_max" into "out".  */
        /* ---------------------------------------------------------------- */
        out = m_mid;

        if (h_min > l_max) { tmp   = h_min; h_min = l_max; l_max = tmp; }
        if (h_min > out  ) { out   = h_min; }
        if (out   > l_max) { out   = l_max; }

        /* ---------------------------------------------------------------- */
        /*  Store the resulting pixel.                                      */
        /* ---------------------------------------------------------------- */
        o_data[i] = out;
    }
}

/* ======================================================================== */
/*  End of file:  img_median_3x3.c                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.8     Thu May 23 16:48:21 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_mpeg2_vld_inter                                                 */
/*                                                                          */
/*  PLATFORM                                                                */
/*      C6400                                                               */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      12-Dec-2001                                                         */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      This routine takes a bitstream of an MPEG-2 non-intra coded         */
/*      macroblock and returns the decoded IDCT coefficients. The routine   */
/*      is implemented as specified in the MPEG-2 standard text (ISO/IEC    */
/*      13818-2). The routine checks the coded block pattern (cbp),         */
/*      performs coefficient decoding inlcuding, variable length decode,    */
/*      run-length expansion, inverse zigzag, dequantization, saturation    */
/*      and mismatch control.                                               */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C callable, and has the following C prototype:      */
/*                                                                          */
/*      void IMG_mpeg2_vld_inter                                            */
/*      (                                                                   */
/*          const short    *restrict Wptr,                                  */
/*          short          *restrict outi,                                  */
/*          IMG_mpeg2_vld  *restrict Mpeg2v,                                */
/*          int            mode_12Q4,                                       */
/*          int            num_blocks,                                      */
/*          int            bsbuf_words                                      */
/*      );                                                                  */
/*                                                                          */
/*        Wptr:   Pointer to array that contains quantization matrix. The   */
/*                elements of the quantization matrix in *Wptr must be      */
/*                ordered according to the scan pattern used (zigzag or     */
/*                alternate scan). Video format 4:2:0 requires one          */
/*                quantization matrix (64 array elements).  For formats     */
/*                4:2:2 and 4:4:4 two quantization matrices (one for luma   */
/*                and one for chroma) must specified in the array (128      */
/*                array elements).                                          */
/*                                                                          */
/*        outi:   Pointer to the IDCT coefficients output array             */
/*                (6*64 elements), elements must be set to zero prior to    */
/*                function call.                                            */
/*                                                                          */
/*        Mpeg2v: Pointer to the context object containing the coding       */
/*                parameters of the MB to be decoded and the current state  */
/*                of the bitstream buffer. The structure is described       */
/*                below.                                                    */
/*                                                                          */
/*     mode_12Q4: 0: Coefficients are returned in normal 16-bit integer     */
/*                format.                                                   */
/*                Otherwise: Coefficients are returned in 12Q4 format       */
/*                (normal 16-bit integer format left shifted by 4). This    */
/*                mode is useful for directly passing the coefficients      */
/*                into the IMG_idct_8x8 routine.                            */
/*                                                                          */
/*    num_blocks: Number of blocks that the MB contains. Valid values are   */
/*                6 for 4:2:0, 8 for 4:2:2 and 12 for 4:4:4 format.         */
/*                                                                          */
/*   bsbuf_words: Size of bitstream buffer in words. Must be a power of 2.  */
/*                Bitstream buffer must be aligned at an address boundary   */
/*                equal to its size in bytes (bitstream buffer is           */
/*                addressed circularly by this routine.)                    */
/*                                                                          */
/*      The structure Mpeg2v is defined as follows:                         */
/*                                                                          */
/*C         #ifndef IMG_MPEG2_VLD_STRUCT_                                  C*/
/*C         #define IMG_MPEG2_VLD_STRUCT_ 1                                C*/
/*C                                                                        C*/
/*C         typedef struct {                                               C*/
/*C           unsigned int  *bsbuf;      // pointer to bitstream buffer    C*/
/*C           unsigned int  next_wptr;   // next word to read from buffer  C*/
/*C           unsigned int  bptr;        // bit position within word       C*/
/*C           unsigned int  word1;       // word aligned buffer            C*/
/*C           unsigned int  word2;       // word aligned buffer            C*/
/*C           unsigned int  top0;        // top 32 bits of bitstream       C*/
/*C           unsigned int  top1;        // next 32 bits of bitstream      C*/
/*C           const unsigned char *scan; // inverse zigzag scan matrix     C*/
/*C           unsigned int  intravlc;    // intra_vlc_format               C*/
/*C           unsigned int  quant_scale; // quant_scale                    C*/
/*C           unsigned int  dc_prec;     // intra_dc_precision             C*/
/*C           unsigned int  cbp;         // coded_block_pattern            C*/
/*C           unsigned int  fault;       // fault condition (returned)     C*/
/*C           unsigned int  reserved;    // reserved                       C*/
/*C         } IMG_mpeg2_vld;                                               C*/
/*C                                                                        C*/
/*C         #endif                                                         C*/
/*                                                                          */
/*      The Mpeg2v variables should  have a fixed layout since they are     */
/*      accessed by this routine. If the layout is changed, the             */
/*      corresponding changes have to be made in the assembly code too.     */
/*                                                                          */
/*      The routine sets the fault flag Mpeg2v.fault to 1 if an invalid     */
/*      VLC code was encountered or the total run went beyond 63. In        */
/*      theses cases the decoder has to resynchronize.                      */
/*                                                                          */
/*      The required lookup tables for this routine are provided in         */
/*      IMGLIB and are linked in automatically when linking against         */
/*      IMGLIB.                                                             */
/*                                                                          */
/*      Before calling the routine the bitstream varaibles in Mpeg2v        */
/*      have to be initialized. If bsbuf is a circular buffer and bsptr     */
/*      contains the number of bits in the buffer that already have         */
/*      been consumed, then next_wptr, bptr, word1, word2, top0 and         */
/*      top1 are initialized as follows:                                    */
/*                                                                          */
/*      1. nextwptr: bsptr may not be a multiple of 32, therefore obtain    */
/*      the next lower multiple of 32.                                      */
/*                                                                          */
/*          next_wptr = (bsptr >> 5);                                       */
/*                                                                          */
/*      2. bptr: bptr is the bit pointer which points to the current        */
/*      bit WITHIN the word pointed to by next_wptr.                        */
/*                                                                          */
/*          bptr = bsptr & 31;                                              */
/*          bptr_cmpl = 32 - bptr;                                          */
/*                                                                          */
/*      3. word1 and word2: read next 3 words from the bitstream buffer     */
/*      (word0 is a temporary variable). bsbuf_words is the size of the     */
/*      bitstream buffer in words.                                          */
/*                                                                          */
/*          word0 = bsbuf[next_wptr];                                       */
/*          next_wptr = (next_wptr + 1) & (bsbuf_words-1);                  */
/*                                                                          */
/*          word1 = bsbuf[next_wptr];                                       */
/*          next_wptr = (next_wptr + 1) & (bsbuf_words-1);                  */
/*                                                                          */
/*          word2 = bsbuf[next_wptr];                                       */
/*          next_wptr = (next_wptr + 1) & (bsbuf_words-1);                  */
/*                                                                          */
/*      4. top0 and top1: Shift words word0, word1, word2 by bptr to the    */
/*      left so that the current bit becomes the MSB in word0. word0 can    */
/*      simply be shifted by bptr; the then empty LSBs of word0 have to be  */
/*      filled with the MSBs of word1. To do that the required MSBs are     */
/*      brought into the position of empty LSBs of word0 by shifting word1  */
/*      to the right by (32-bptr). The result is then copied into word0 by  */
/*      an addition. Rather than overwriting word0, top0 is used to hold    */
/*      the new bit aligned word. The same procedure is used to obtain      */
/*      top1. top0 and top1 contain the next 64 bits of the bitstream.      */
/*                                                                          */
/*          s1 = word0 << bptr;                                             */
/*          s2 = word1 >> bptr_cmpl;  // unsigned right-shift //            */
/*          top0 = s1 + s2;                                                 */
/*                                                                          */
/*          s3 = word1 << bptr;                                             */
/*          s4 = word2 >> bptr_cmpl;  // unsigned right-shift //            */
/*          top1 = s3 + s4;                                                 */
/*                                                                          */
/*      Note that the routine returns the updated state of the bitstream    */
/*      buffer variables, top0, top1, word1, word2, bptr and next_wptr. If  */
/*      all other functions which access the bitstream in a decoder system  */
/*      maintain the buffer variables in the same way, then the above       */
/*      initialization procedure has to be performed only once at the       */
/*      beginning.                                                          */
/*                                                                          */
/*                                                                          */
/*  TECHNIQUES                                                              */
/*      The instruction NORM is used to detect the number of leading zeros  */
/*      or ones in a code word. This value together with additional bits    */
/*      extracted from the codeword is then used as an index into look-up   */
/*      tables to determine the length, run, level and sign. Escape code    */
/*      sequences are directly extracted from the code word.                */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      The bitstream must be stored in memory in 32-bit words which are    */
/*      in little endian byte order.                                        */
/*                                                                          */
/*      Wptr is allowed to overrun once (to detect total run overrun), so   */
/*      maximum overrun that can occur is 66 (Error mark). Therefore,       */
/*      in memory 66+1 halfwords behind the weighting matrix should be      */
/*      valid (e.g. peripherals). No memory is overwritten,                 */
/*      only loads occurr.                                                  */
/*                                                                          */
/*      Note that the AMR register is set to zero on exit.                  */
/*                                                                          */
/*  MEMORY REQUIREMENTS                                                     */
/*      1792 bytes for the lookup tables                                    */
/*      (can be shared with mpeg2_vld_intra)                                */
/*                                                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

typedef struct {
    unsigned int  *bsbuf;      // pointer to bitstream buffer
    unsigned int  next_wptr;   // next word to read from buffer
    unsigned int  bptr;        // bit position within word
    unsigned int  word1;       // word aligned buffer
    unsigned int  word2;       // word aligned buffer
    unsigned int  top0;        // top 32 bits of bitstream
    unsigned int  top1;        // next 32 bits of bitstream
    const unsigned char *scan; // inverse zigzag scan matrix
    unsigned int  intravlc;    // intra_vlc_format
    unsigned int  quant_scale; // quant_scale
    unsigned int  dc_prec;     // intra_dc_precision
    unsigned int  cbp;         // coded_block_pattern
    unsigned int  fault;       // fault condition (returned)
    unsigned int  reserved;    // reserved
} IMG_mpeg2_vld;

/*********************************************************************/
/* C6000 functions                                                   */
/*********************************************************************/
#ifdef _TMS320C6X
# define SHL(x,y) ( (x) << (y) )
# define SHR(x,y) ( (x) >> (y) )

#else

# define SHL(x,y) ( ((y) & 32) ? 0 : ((x) << (y)) )
# define SHR(x,y) ( ((y) & 32) ? 0 : ((x) >> (y)) )

unsigned int _lmbd(int bit, unsigned num);
unsigned int _norm(unsigned int x);
unsigned int _ext(int x, unsigned int a, unsigned int b);
unsigned int _extu(unsigned int x, unsigned int a, unsigned int b);
int _sshl(int x, unsigned int s);

#endif

/* ------------------------------------------------------------------------ */
/*  Run-Level Tables                                                        */
/* ------------------------------------------------------------------------ */
extern unsigned short IMG_rld_table0[576];

/* ------------------------------------------------------------------------ */
/*  Length tables                                                           */
/* ------------------------------------------------------------------------ */
extern const unsigned char IMG_len_tbl0[640];

/* ------------------------------------------------------------------------ */
/*  Function                                                                */
/* ------------------------------------------------------------------------ */
void IMG_mpeg2_vld_inter
(
    const short *restrict Wptr,      // quantization matrix
    short *restrict outi,            // IDCT coefficients output array
    IMG_mpeg2_vld *restrict Mpeg2v,  // bitstream context structure
    int mode_12Q4,                   // if !=0 returns coeffs in 12Q4 format
    int num_blocks,                  // number of blocks in the MB
    int bsbuf_words                  // bitstream buffer size
)
{

/* ------------------------------------------------------------------------ */
/*  get bitstream info                                                      */
/* ------------------------------------------------------------------------ */
    unsigned int         *bsbuf     = Mpeg2v->bsbuf;
             int          next_wptr = Mpeg2v->next_wptr;
             int          bptr      = Mpeg2v->bptr;
    unsigned int          word1     = Mpeg2v->word1;
    unsigned int          word2     = Mpeg2v->word2;
    unsigned int          top0      = Mpeg2v->top0;
    unsigned int          top1      = Mpeg2v->top1;
    const unsigned char  *zzptr     = Mpeg2v->scan;
    unsigned int          qscl      = Mpeg2v->quant_scale;
    unsigned int          cbp       = Mpeg2v->cbp;

    unsigned int bptr_cmpl, top0_bk, word1_rw;

    unsigned long ltop0, t1, t5; /* 40-bit registers */

/* ------------------------------------------------------------------------ */
/*  block number (0-3: lum, 4, 5: chrom)                                    */
/* ------------------------------------------------------------------------ */
    int block;

/* ------------------------------------------------------------------------ */
/*  Variables for length, run and level decoding                            */
/* ------------------------------------------------------------------------ */
    unsigned int t2, t4, t7, t8, t9;
    int rld_left, rld_index;
    unsigned short run_level;
    unsigned char run, len, len_c;
    const unsigned char *t3;
    short level;
    unsigned int fault, eob_err, nrm;
    unsigned int test1, test2;

/* ------------------------------------------------------------------------ */
/*  Variables for de-quantization                                           */
/* ------------------------------------------------------------------------ */
    int neg, pos, f1, f3, f5, qW, sum, cc;
#ifdef _TMS320C6X
    int f4;
#else
    _int64 f4;
#endif
    const short *Wptr_origin=Wptr;
    const short *Wptr_end;
    short W;
    unsigned char cnum;

/* ------------------------------------------------------------------------ */
/*  Variables for 12Q4 mode                                                 */
/* ------------------------------------------------------------------------ */
    int mask, shr, lsb;

/* ------------------------------------------------------------------------ */
/*  In 12Q4 mode the returned DCT coefficients are left shifted by 4,       */
/*  i.e. in 12Q4 format.                                                    */
/* ------------------------------------------------------------------------ */
    if (mode_12Q4)
    {
        mask = 0xFFF0; /* clear 4 LSBs of 12Q4 number */
        shr  = 16;     /* shift right after saturate shift */
        lsb  = 16;     /* LSB for mismatch control */
    }
    else
    {
        mask = 0xFFFF; /* keep 4 LSBs */
        shr  = 20;     /* shift right after saturate shift */
        lsb  = 1;      /* LSB for mismatch control */
    }

/* ------------------------------------------------------------------------ */
/*  Advance bitsream by 8 bit to convert to 40-bit top0                     */
/* ------------------------------------------------------------------------ */
    ltop0 = ((long)top0 << 8) + (top1 >> 24);

    bptr += 8;
    test2 = (bptr >= 32);
    if (test2)
    {
        word1_rw = word1;
        word1 = word2;
        word2 = bsbuf[next_wptr];
        next_wptr += 1;
        next_wptr &= (bsbuf_words-1);
    }
    bptr = bptr & 31;
    bptr_cmpl = 32 - bptr;
    t8 = SHL(word1, bptr);
    t9 = SHR(word2, bptr_cmpl);                   /* unsigned shift */
    top1 = t8 + t9;

    fault = 0;

/* ------------------------------------------------------------------------ */
/*  block loop                                                              */
/* ------------------------------------------------------------------------ */
    for (block=0; block<num_blocks; block++)
    {

    /* -------------------------------------------------------------------- */
    /*  cbp: Bit 5 4 3 2 1 0 , if the corresponding bit is zero block no.   */
    /*  (5-bit pos) is not coded                                            */
    /* -------------------------------------------------------------------- */
        if (!(cbp & (1 << (num_blocks-block-1))))
            continue;

        zzptr = Mpeg2v->scan;

        cc = (block<4) ? 0 : (block&1)+1;

        if (cc!=0 && num_blocks>6)
            Wptr=Wptr_origin + 64;   /* weighting matrix for chrominance */
        else
            Wptr=Wptr_origin;        /* weighting matrix for luminance */

        Wptr_end = Wptr + 64;

        sum = 0;
        eob_err = 0;

    /* -------------------------------------------------------------------- */
    /*  Decode first coefficient                                            */
    /*                                                                      */
    /*  First code is special case: when the MSB of the first code is '1',  */
    /*  we know it can only be VLC '1s' (because EOB cannot occur and the   */
    /*  only other code is '11s' which is not valid for the first code.)    */
    /*  If MSB=0 skip to normal AC loop, i.e. use NORM+4 extra bits as      */
    /*  table index; if MSB=1 execute special case, do not use              */
    /*  NORM because it may have overrun but set LEN=2, LEN_C=30.           */
    /* -------------------------------------------------------------------- */
        if (ltop0>>39)
        {

        /* ---------------------------------------------------------------- */
        /*  Length computation, not required since we know LEN is 2.        */
        /* ---------------------------------------------------------------- */
           len  = 2;
           len_c = 30;

        /* ---------------------------------------------------------------- */
        /*  now that we know the length of the current VL code we can       */
        /*  advance bitstream to next one:                                  */
        /*                                                                  */
        /*  1. update ltop0 from ltop0 and top1                             */
        /* ---------------------------------------------------------------- */
            t5  = ltop0 << len;
            t7  = top1 >> len_c;
            top0_bk = (unsigned) (ltop0>>8);
            ltop0 = t5 + t7;

        /* ---------------------------------------------------------------- */
        /*  2. update top1 from word1 and word2 after increasing bptr by    */
        /*  len.  if neccesary (i.e. if new bptr is greater than 32)        */
        /*  update word1 and word2 from memory first. Don't forget that     */
        /*  bptr is always relative to the next lower word boundary and     */
        /*  therefore needs to be ANDed with 31 in case it become >=32.     */
        /* ---------------------------------------------------------------- */
            bptr += len;
            test2 = (bptr >= 32);
            if (test2) {
                word1_rw = word1;
                word1 = word2;
                word2 = bsbuf[next_wptr];
                next_wptr += 1;
                next_wptr &= (bsbuf_words-1);
            }
            bptr = bptr & 31;
            bptr_cmpl = 32 - bptr;
            t8 = SHL(word1, bptr);
            t9 = SHR(word2, bptr_cmpl);                   /* unsigned shift */
            top1 = t8 + t9;

        /* ---------------------------------------------------------------- */
        /*  Run-Level Decode: run = 0, level = 1                            */
        /* ---------------------------------------------------------------- */
            neg = _extu(top0_bk,1,31);

        /* ---------------------------------------------------------------- */
        /*  Run-lengh expansion and DQ                                      */
        /* ---------------------------------------------------------------- */

        /* ---------------------------------------------------------------- */
        /*  Dequantisation: *out_i = ((2*level + Sign(level)) * W * qscl)   */
        /*  / 32. Sign(x)=-1 for x<0, 0 for x=0, +1 for x>0. Division '/':  */
        /*  "Integer division with truncation of the result toward zero.    */
        /*  For example, 7/4 and -7/-4 are truncated to 1 and -7/4 and      */
        /*  7/-4 are truncated to -1." (MPEG-2 Standard Text)               */
        /* ---------------------------------------------------------------- */
            if (neg)
                f1=-3;                                          /* 2*(-1)-1 */
            else
                f1=3;                                              /* 2*1+1 */

        /* ---------------------------------------------------------------- */
        /*  find quantization matrix element at zigzag position and         */
        /*  multiply f1 with W * qscl                                       */
        /* ---------------------------------------------------------------- */
            W = *Wptr++;
            qW = qscl * W;
            f3 = f1 * qW;

        /* ---------------------------------------------------------------- */
        /*  for negative numbers we first need to add 31 before dividing    */
        /*  by 32 to achieve truncation towards zero as required by the     */
        /*  standard.                                                       */
        /* ---------------------------------------------------------------- */
            if (neg) f3 += 31;

        /* ---------------------------------------------------------------- */
        /*  saturate to signed 12 bit word (with /32 <-> >>5 incorporated)  */
        /*  SSHL: shift left and saturate to 32 bits                        */
        /* ---------------------------------------------------------------- */
            f4 = _sshl(f3, 15);
            f5 = (int)(f4 >> shr);
            f5 &= mask;

        /* ---------------------------------------------------------------- */
        /*  mismatch control: determine if sum of coefficents is odd or     */
        /*  even                                                            */
        /* ---------------------------------------------------------------- */
            sum += f5;

        /* ---------------------------------------------------------------- */
        /*  find un-zigzag position of DCT coefficient                      */
        /* ---------------------------------------------------------------- */
            zzptr++;                                            /* always 0 */
            outi[block*64] = f5;
        }

    /* -------------------------------------------------------------------- */
    /*  Decode AC coefficients                                              */
    /* -------------------------------------------------------------------- */
        while (!eob_err)
        {

        /* ---------------------------------------------------------------- */
        /*  Length computation                                              */
        /* ---------------------------------------------------------------- */

        /* ---------------------------------------------------------------- */
        /*  _lnorm returns the number of redundant sign bits in ltop0, e.g. */
        /*  0001 xxxx ... xxxx returns 2, 1111 10xx ... xxxx returns 4.     */
        /*  Example: top=0000 101s xxxx xxxx, then nrm=3                    */
        /* ---------------------------------------------------------------- */
            nrm = _lnorm(ltop0);

        /* ---------------------------------------------------------------- */
        /*  get rid of the leading all 0s/1s.                               */
        /*  Example: t1=0101 sxxx xxxx xxxx                                 */
        /* ---------------------------------------------------------------- */
            t1  = ltop0 << nrm;

        /* ---------------------------------------------------------------- */
        /*  use the number of leading bits (norm) as index,                 */
        /*  Example: t2= xxxx xxxx 0011 rrrr                                */
        /* ---------------------------------------------------------------- */
            t2  = nrm << 4;
            t3 = &IMG_len_tbl0[t2];

        /* ---------------------------------------------------------------- */
        /*  use 4 extra bits after leading bits to distinguish special      */
        /*  cases (40-4=36)                                                 */
        /* ---------------------------------------------------------------- */
            t4  = (unsigned) (t1 >> 36);
        /* ---------------------------------------------------------------- */
        /*  get len and 32-len from tables                                  */
        /* ---------------------------------------------------------------- */
            len  = t3[t4];
            len_c = 32 - len;

        /* ---------------------------------------------------------------- */
        /*  now that we know the length of the current VL code we can       */
        /*  advance bitstream to next one:                                  */
        /*                                                                  */
        /*  1. update ltop0 from ltop0 and top1                             */
        /* ---------------------------------------------------------------- */
            t5  = ltop0 << len;
            t7  = top1 >> len_c;
            top0_bk = (unsigned)(ltop0 >> 8);
            ltop0 = t5 + t7;

        /* ---------------------------------------------------------------- */
        /*  2. update top1 from word1 and word2 after increasing bptr by    */
        /*  len.  if neccesary (i.e. if new bptr is greater than 32)        */
        /*  update word1 and word2 from memory first. Don't forget that     */
        /*  bptr is always relative to the next lower word boundary and     */
        /*  therefore needs to be ANDed with 31 in case it become >=32.     */
        /* ---------------------------------------------------------------- */
            bptr += len;
            test2 = (bptr >= 32);
            if (test2) {
                word1_rw = word1;
                word1 = word2;
                word2 = bsbuf[next_wptr];
                next_wptr += 1;
                next_wptr &= (bsbuf_words-1);
            }
            bptr = bptr & 31;
            bptr_cmpl = 32 - bptr;
            t8 = SHL(word1, bptr);
            t9 = SHR(word2, bptr_cmpl);                   /* unsigned shift */
            top1 = t8 + t9;

        /* ---------------------------------------------------------------- */
        /*  Run-Level Decode                                                */
        /* ---------------------------------------------------------------- */

        /* ---------------------------------------------------------------- */
        /*  check if it is an ESCAPE code which has a unique fixed length   */
        /*  of 24 bits                                                      */
        /* ---------------------------------------------------------------- */
            test1 = len - 24;

            if (!test1)
            {
            /* ------------------------------------------------------------ */
            /* ESCAPE code: no look up required, just extract bits: 6 bits  */
            /* for ESCAPE, 6 bits for RUN, then 12 bits for LEVEL           */
            /* ------------------------------------------------------------ */
                run = _extu(top0_bk, 6, 26);
                level = _ext(top0_bk, 12, 20);
            }
            else
            {
                rld_left = len-5;
                if (len<5) rld_left=0;

                /* -------------------------------------------------------- */
                /*  last 5 bits of VLC incl. sign form 2nd part of index    */
                /* -------------------------------------------------------- */
                rld_index = ((len)<<5) + _extu(top0_bk, rld_left, 32-5);
                run_level = IMG_rld_table0[rld_index];

                run   = run_level >> 8;
                level = (char)run_level;
            }

            eob_err = (run >= 64);

        /* ---------------------------------------------------------------- */
        /*  Run-lengh expansion and DQ                                      */
        /* ---------------------------------------------------------------- */

        /* ---------------------------------------------------------------- */
        /*  Dequantisation: *out_i = ((2*level + Sign(level)) * W * qscl)   */
        /*  / 32. Sign(x)=-1 for x<0, 0 for x=0, +1 for x>0. Division '/':  */
        /*  "Integer division with truncation of the result toward zero.    */
        /*  For example, 7/4 and -7/-4 are truncated to 1 and -7/4 and      */
        /*  7/-4 are truncated to -1." (MPEG-2 Standard Text)               */
        /* ---------------------------------------------------------------- */
            neg = (level < 0);
            f1 = 2*level;

        /* ---------------------------------------------------------------- */
        /*  This needs to be added over intra                               */
        /* ---------------------------------------------------------------- */
            pos = (level > 0);
            f1 = f1 - neg;
            f1 = f1 + pos;

        /* ---------------------------------------------------------------- */
        /*  find quantization matrix element at zigzag position and         */
        /*  multiply f1 with W * qscl                                       */
        /* ---------------------------------------------------------------- */
            if (!eob_err)         /* prevent from accessing memory when EOB */
            {
                W = *(Wptr += run);
                Wptr++;
                /* detect total run overrun */
                eob_err = (Wptr > Wptr_end); //- Wptr_origin)>64;
            }

            qW = qscl * W;
            f3 = f1 * qW;

        /* ---------------------------------------------------------------- */
        /*  for negative numbers we first need to add 31 before dividing    */
        /*  by 32 to achieve truncation towards zero as required by the     */
        /*  standard.                                                       */
        /* ---------------------------------------------------------------- */
            if (neg) f3 += 31;

        /* ---------------------------------------------------------------- */
        /*  saturate to signed 12 bit word (with /32 <-> >>5 incorporated)  */
        /*  SSHL: shift left and saturate to 32 bits                        */
        /* ---------------------------------------------------------------- */
            f4 = _sshl(f3, 15);
            f5 = (int)(f4 >> shr);
            f5 &= mask;

        /* ---------------------------------------------------------------- */
        /*  mismatch control: determine if sum of coefficents is odd or     */
        /*  even                                                            */
        /* ---------------------------------------------------------------- */
            if (!eob_err)
                sum += f5;

        /* ---------------------------------------------------------------- */
        /*  find un-zigzag position of DCT coefficient                      */
        /* ---------------------------------------------------------------- */
            if (!eob_err)         /* prevent from accessing memory when EOB */
                cnum = *(zzptr += run);
            zzptr++;
            if (!eob_err)
                outi[block*64+cnum] = f5;

        } /* while */

    /* -------------------------------------------------------------------- */
    /*  mismatch control: toggle last bit of last coefficient if sum of     */
    /*  coefficents is even.                                                */
    /* -------------------------------------------------------------------- */

        if ((sum & lsb)==0)            // 12Q4
     {
            outi[block*64+63] ^= lsb;  // 12Q4
     }

    /* -------------------------------------------------------------------- */
    /*  Determine nature of fault, invalid code word or exceeding of the    */
    /*  allowed total run of 64.                                            */
    /* -------------------------------------------------------------------- */
    fault = (run>65) || (Wptr > Wptr_end); //- Wptr_origin)>64);

    if (fault) break;

    } /* for */

    /* -------------------------------------------------------------------- */
    /*  rewind bitstream by 8 bits to convert back to 32-bit top0           */
    /* -------------------------------------------------------------------- */
    top0 = (unsigned) (ltop0 >> 8);
    top1 = (top1 >> 8) + (unsigned)(ltop0 << 24);
    bptr = bptr - 8;
    if (bptr<0)
    {
        word2 = word1;
        word1 = word1_rw;
        bptr += 32;
        next_wptr -= 1;
        next_wptr &= (bsbuf_words-1);
    }

    /* -------------------------------------------------------------------- */
    /*  Update bitstream variables                                          */
    /* -------------------------------------------------------------------- */
    Mpeg2v->next_wptr = next_wptr;
    Mpeg2v->bptr      = bptr;
    Mpeg2v->word1     = word1;
    Mpeg2v->word2     = word2;
    Mpeg2v->top0      = top0;
    Mpeg2v->top1      = top1;
    Mpeg2v->fault     = fault;
}

/* ======================================================================== */
/*  End of file:  img_mpeg2_vld_inter.c                                     */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.10    Thu May 23 02:46:47 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================= */
/*  TEXAS INSTRUMENTS, INC.                                                */
/*                                                                         */
/*  NAME                                                                   */
/*      IMG_mpeg2_vld_intra                                                */
/*                                                                         */
/*  PLATFORM                                                               */
/*      C6400                                                              */
/*                                                                         */
/*  REVISION DATE                                                          */
/*      12-Dec-2001                                                        */
/*                                                                         */
/*  DESCRIPTION                                                             */
/*      This routine takes a bitstream of an MPEG-2 intra coded macroblock  */
/*      and returns the decoded IDCT coefficients. The routine is           */
/*      implemented as specified in the MPEG-2 standard text (ISO/IEC       */
/*      13818-2). The routine checks the coded block pattern (cbp),         */
/*      performs DC and AC decoding inlcuding, variable length decode,      */
/*      run-length expansion, inverse zigzag, dequantization, saturation    */
/*      and mismatch control.                                               */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C callable, and has the following C prototype:      */
/*                                                                          */
/*      void IMG_mpeg2_vld_intra                                            */
/*      (                                                                   */
/*          const short    *restrict Wptr,                                  */
/*          short          *restrict outi,                                  */
/*          IMG_mpeg2_vld  *restrict Mpeg2v,                                */
/*          int            dc_pred[3],                                      */
/*          int            mode_12Q4,                                       */
/*          int            num_blocks,                                      */
/*          int            bsbuf_words                                      */
/*      );                                                                  */
/*                                                                          */
/*        Wptr:   Pointer to array that contains quantization matrix. The   */
/*                elements of the quantization matrix in *Wptr must be      */
/*                ordered according to the scan pattern used (zigzag or     */
/*                alternate scan). Video format 4:2:0 requires one          */
/*                quantization matrix (64 array elements).  For formats     */
/*                4:2:2 and 4:4:4 two quantization matrices (one for luma   */
/*                and one for chroma) must specified in the array (128      */
/*                array elements).                                          */
/*                                                                          */
/*        outi:   Pointer to the IDCT coefficients output array             */
/*                (6*64 elements), elements must be set to zero prior to    */
/*                function call.                                            */
/*                                                                          */
/*        Mpeg2v: Pointer to the context object containing the coding       */
/*                parameters of the MB to be decoded and the current state  */
/*                of the bitstream buffer. The structure is described       */
/*                below.                                                    */
/*                                                                          */
/*       dc_pred: Intra DC prediction array, the first element of dc_pred   */
/*                is the DC prediction for Y, the second for Cr and the     */
/*                third for Cb.                                             */
/*                                                                          */
/*     mode_12Q4: 0: Coefficients are returned in normal 16-bit integer     */
/*                format.                                                   */
/*                Otherwise: Coefficients are returned in 12Q4 format       */
/*                (normal 16-bit integer format left shifted by 4). This    */
/*                mode is useful for directly passing the coefficients      */
/*                into the IMG_idct_8x8 routine.                            */
/*                                                                          */
/*    num_blocks: Number of blocks that the MB contains. Valid values are   */
/*                6 for 4:2:0, 8 for 4:2:2 and 12 for 4:4:4 format.         */
/*                                                                          */
/*   bsbuf_words: Size of bitstream buffer in words. Must be a power of 2.  */
/*                Bitstream buffer must be aligned at an address boundary   */
/*                equal to its size in bytes (bitstream buffer is           */
/*                addressed circularly by this routine.)                    */
/*                                                                          */
/*      The structure Mpeg2v is defined as follows:                         */
/*                                                                          */
/*C       #ifndef IMG_MPEG2_VLD_STRUCT_                                    C*/
/*C       #define IMG_MPEG2_VLD_STRUCT_ 1                                  C*/
/*C                                                                        C*/
/*C       typedef struct {                                                 C*/
/*C           unsigned int  *bsbuf;      // pointer to bitstream buffer    C*/
/*C           unsigned int  next_wptr;   // next word to read from buffer  C*/
/*C           unsigned int  bptr;        // bit position within word       C*/
/*C           unsigned int  word1;       // word aligned buffer            C*/
/*C           unsigned int  word2;       // word aligned buffer            C*/
/*C           unsigned int  top0;        // top 32 bits of bitstream       C*/
/*C           unsigned int  top1;        // next 32 bits of bitstream      C*/
/*C           const unsigned char *scan; // inverse zigzag scan matrix     C*/
/*C           unsigned int  intravlc;    // intra_vlc_format               C*/
/*C           unsigned int  quant_scale; // quant_scale                    C*/
/*C           unsigned int  dc_prec;     // intra_dc_precision             C*/
/*C           unsigned int  cbp;         // coded_block_pattern            C*/
/*C           unsigned int  fault;       // fault condition (returned)     C*/
/*C           unsigned int  reserved;    // reserved                       C*/
/*C       } IMG_mpeg2_vld;                                                 C*/
/*C                                                                        C*/
/*C       #endif                                                           C*/
/*                                                                          */
/*      The Mpeg2v variables should have a fixed layout since they are      */
/*      accessed by this routine.  If the layout is changed, the            */
/*      corresponding changes have to be made in the assembly code too.     */
/*                                                                          */
/*      The routine sets the fault flag Mpeg2v.fault to 1 if an invalid     */
/*      VLC code was encountered or the total run went beyond 63. In        */
/*      theses cases the decoder has to resynchronize.                      */
/*                                                                          */
/*      The required lookup tables for this routine are provided in         */
/*      IMGLIB and are linked in automatically when linking against         */
/*      IMGLIB.                                                             */
/*                                                                          */
/*      Before calling the routine the bitstream variables in Mpeg2v        */
/*      have to be initialized. If bsbuf is a circular buffer and bsptr     */
/*      contains the number of bits in the buffer that already have         */
/*      been consumed, then next_wptr, bptr, word1, word2, top0 and         */
/*      top1 are initialized as follows:                                    */
/*                                                                          */
/*      1. nextwptr: bsptr may not be a multiple of 32, therefore obtain    */
/*      the next lower multiple of 32.                                      */
/*                                                                          */
/*          next_wptr = (bsptr >> 5);                                       */
/*                                                                          */
/*      2. bptr: bptr is the bit pointer which points to the current        */
/*      bit WITHIN the word pointed to by next_wptr.                        */
/*                                                                          */
/*          bptr = bsptr & 31;                                              */
/*          bptr_cmpl = 32 - bptr;                                          */
/*                                                                          */
/*      3. word1 and word2: read next 3 words from the bitstream buffer     */
/*      (word0 is a temporary variable). bsbuf_words is the size of the     */
/*      bitstream buffer in words.                                          */
/*                                                                          */
/*          word0 = bsbuf[next_wptr];                                       */
/*          next_wptr = (next_wptr + 1) & (bsbuf_words-1);                  */
/*                                                                          */
/*          word1 = bsbuf[next_wptr];                                       */
/*          next_wptr = (next_wptr + 1) & (bsbuf_words-1);                  */
/*                                                                          */
/*          word2 = bsbuf[next_wptr];                                       */
/*          next_wptr = (next_wptr + 1) & (bsbuf_words-1);                  */
/*                                                                          */
/*      4. top0 and top1: Shift words word0, word1, word2 by bptr to the    */
/*      left so that the current bit becomes the MSB in word0. word0 can    */
/*      simply be shifted by bptr; the then empty LSBs of word0 have to be  */
/*      filled with the MSBs of word1. To do that the required MSBs are     */
/*      brought into the position of empty LSBs of word0 by shifting word1  */
/*      to the right by (32-bptr). The result is then copied into word0 by  */
/*      an addition. Rather than overwriting word0, top0 is used to hold    */
/*      the new bit aligned word. The same procedure is used to obtain      */
/*      top1. top0 and top1 contain the next 64 bits of the bitstream.      */
/*                                                                          */
/*          s1 = word0 << bptr;                                             */
/*          s2 = word1 >> bptr_cmpl;  // unsigned right-shift //            */
/*          top0 = s1 + s2;                                                 */
/*                                                                          */
/*          s3 = word1 << bptr;                                             */
/*          s4 = word2 >> bptr_cmpl;  // unsigned right-shift //            */
/*          top1 = s3 + s4;                                                 */
/*                                                                          */
/*      Note that the routine returns the updated state of the bitstream    */
/*      buffer variables, top0, top1, word1, word2, bptr and next_wptr. If  */
/*      all other functions which access the bitstream in a decoder system  */
/*      maintain the buffer variables in the same way, then the above       */
/*      initialization procedure has to be performed only once at the       */
/*      beginning.                                                          */
/*                                                                          */
/*  TECHNIQUES                                                              */
/*      The instruction NORM is used to detect the number of leading zeros  */
/*      or ones in a code word. This value together with additional bits    */
/*      extracted from the codeword is then used as an index into look-up   */
/*      tables to determine the length, run, level and sign. Escape code    */
/*      sequences are directly extracted from the code word.                */
/*                                                                          */
/*      DC coefficients are decoded without lookup tables by exploiting     */
/*      the relatively simple relationship between the number of leading    */
/*      zeros and dc_size and the length of the code word.                  */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      The bitstream must be stored in memory in 32-bit words which are    */
/*      in little endian byte order.                                        */
/*                                                                          */
/*      Wptr is allowed to overrun once (to detect total run overrun), so   */
/*      maximum overrun that can occur is 66 (Error mark). Therefore,       */
/*      in memory 66+1 halfwords behind the weighting matrix should be      */
/*      valid (e.g. peripherals). No memory is overwritten,                 */
/*      only loads occurr.                                                  */
/*                                                                          */
/*      Note that the AMR register is set to zero on exit.                  */
/*                                                                          */
/*  DATA SIZE                                                               */
/*      3584 bytes for the lookup tables                                    */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/*********************************************************************/
/* C6000 functions                                                   */
/*********************************************************************/
#ifdef _TMS320C6X
# define SHL(x,y) ( (x) << (y) )
# define SHR(x,y) ( (x) >> (y) )

#else

# define SHL(x,y) ( ((y) & 32) ? 0 : ((x) << (y)) )
# define SHR(x,y) ( ((y) & 32) ? 0 : ((x) >> (y)) )

unsigned int _lmbd(int bit, unsigned num);
unsigned int _norm(unsigned int x);
unsigned int _ext(int x, unsigned int a, unsigned int b);
unsigned int _extu(unsigned int x, unsigned int a, unsigned int b);
int _sshl(int x, unsigned int s);

#endif

typedef struct {
    unsigned int  *bsbuf;      // pointer to bitstream buffer
    unsigned int  next_wptr;   // next word to read from buffer
    unsigned int  bptr;        // bit position within word
    unsigned int  word1;       // word aligned buffer
    unsigned int  word2;       // word aligned buffer
    unsigned int  top0;        // top 32 bits of bitstream
    unsigned int  top1;        // next 32 bits of bitstream
    const unsigned char *scan; // inverse zigzag scan matrix
    unsigned int  intravlc;    // intra_vlc_format
    unsigned int  quant_scale; // quant_scale
    unsigned int  dc_prec;     // intra_dc_precision
    unsigned int  cbp;         // coded_block_pattern
    unsigned int  fault;       // fault condition (returned)
    unsigned int  reserved;    // reserved
} IMG_mpeg2_vld;

/* ------------------------------------------------------------------------ */
/*  Run-Level Tables                                                        */
/* ------------------------------------------------------------------------ */
extern unsigned short IMG_rld_table0[576];
extern unsigned short IMG_rld_table1[576];

/* ------------------------------------------------------------------------ */
/*  Length tables                                                           */
/* ------------------------------------------------------------------------ */
extern const unsigned char IMG_len_tbl0[640];
extern const unsigned char IMG_len_tbl1[640];

/* ------------------------------------------------------------------------ */
/*  Function                                                                */
/* ------------------------------------------------------------------------ */
void IMG_mpeg2_vld_intra
(
    const short *restrict Wptr,      // quantization matrix
    short *restrict outi,            // IDCT coefficients output array
    IMG_mpeg2_vld *restrict Mpeg2v,  // bitstream context structure
    int dc_pred[3],                  // DC prediction array
    int mode_12Q4,                   // if !=0 returns coeffs in 12Q4 format
    int num_blocks,                  // number of blocks in the MB
    int bsbuf_words                  // bitstream buffer size
)
{

/* ------------------------------------------------------------------------ */
/*  get bitstream info                                                      */
/* ------------------------------------------------------------------------ */
    unsigned int         *bsbuf     = Mpeg2v->bsbuf;
             int          next_wptr = Mpeg2v->next_wptr;
             int          bptr      = Mpeg2v->bptr;
    unsigned int          word1     = Mpeg2v->word1;
    unsigned int          word2     = Mpeg2v->word2;
    unsigned int          top0      = Mpeg2v->top0;
    unsigned int          top1      = Mpeg2v->top1;
    const unsigned char  *zzptr     = Mpeg2v->scan;
    unsigned int   intra_vlc_format = Mpeg2v->intravlc;
    unsigned int          qscl      = Mpeg2v->quant_scale;
             int intra_dc_precision = Mpeg2v->dc_prec;
    unsigned int          cbp       = Mpeg2v->cbp;

    unsigned int bptr_cmpl, top0_bk, word1_rw;

    unsigned long ltop0, t1, t5; /* 40-bit registers */

/* ------------------------------------------------------------------------ */
/*   block number (0-3: lum, 4, 5: chrom)                                   */
/* ------------------------------------------------------------------------ */
    int block;

/* ------------------------------------------------------------------------ */
/*  Variables for intra DC decoding                                         */
/* ------------------------------------------------------------------------ */
    unsigned int cc, a_cc0, a_cc1, b, c, d, dc_size;
    int dc_diff, val, half_range;

/* ------------------------------------------------------------------------ */
/*  Variables for length, run and level decoding                            */
/* ------------------------------------------------------------------------ */
    unsigned int t2, t4, t7, t8, t9;
    int rld_left, rld_index;
    unsigned short run_level;
    unsigned char run, len, len_c;
    const unsigned char *t3;
    short level;
    unsigned int eob_err, fault, nrm;
    unsigned int test1, test2;

/* ------------------------------------------------------------------------ */
/*  Variables for de-quantization                                           */
/* ------------------------------------------------------------------------ */
    int neg, f1, f3, f5, qW, sum;
#ifdef _TMS320C6X
    int f4;
#else
    _int64 f4;
#endif
    const short *Wptr_origin=Wptr;
    const short *Wptr_end;
    short W;
    unsigned char cnum;

/* ------------------------------------------------------------------------ */
/*  Variables for 12Q4 mode                                                 */
/* ------------------------------------------------------------------------ */
    int mask, shr, lsb;

/* ------------------------------------------------------------------------ */
/*  In 12Q4 mode the returned DCT coefficients are left shifted by 4,       */
/*  i.e. in 12Q4 format.                                                    */
/* ------------------------------------------------------------------------ */
    if (mode_12Q4)
    {
        intra_dc_precision -= 4;     /* shift left DC by 4 */
        mask = 0xFFF0;      /* clear 4 LSBs of 12Q4 number */
        shr  = 16;     /* shift right after saturate shift */
        lsb  = 16;             /* LSB for mismatch control */
    }
    else
    {
        mask = 0xFFFF;                      /* keep 4 LSBs */
        shr  = 20;     /* shift right after saturate shift */
        lsb  = 1;              /* LSB for mismatch control */
    }

/* ------------------------------------------------------------------------ */
/*  Advance bitsream by 8 bit to convert to 40-bit ltop0                    */
/* ------------------------------------------------------------------------ */
    ltop0 = ((long)top0 << 8) + (top1 >> 24);

    bptr += 8;
    test2 = (bptr >= 32);
    if (test2)
    {
        word1_rw = word1;
        word1 = word2;
        word2 = bsbuf[next_wptr];
        next_wptr += 1;
        next_wptr &= (bsbuf_words-1);
    }
    bptr = bptr & 31;
    bptr_cmpl = 32 - bptr;
    t8 = SHL(word1, bptr);
    t9 = SHR(word2, bptr_cmpl);                   /* unsigned shift */
    top1 = t8 + t9;

    fault = 0;

    for (block=0; block<num_blocks; block++)
    {

    /* -------------------------------------------------------------------- */
    /*  cbp: Bit 5 4 3 2 1 0 , if the corresponding bit is zero block no.   */
    /*  (5-bit pos) is not coded                                            */
    /* -------------------------------------------------------------------- */
        if (!(cbp & (1 << (num_blocks-block-1))))
            continue;

        zzptr = Mpeg2v->scan;


        cc = (block<4) ? 0 : (block&1)+1;

        if (cc!=0 && num_blocks>6)
            Wptr=Wptr_origin + 64;   /* weighting matrix for chrominance */
        else
            Wptr=Wptr_origin;        /* weighting matrix for luminance */

        Wptr_end = Wptr + 64;

        sum = 0;
        eob_err = 0;

    /* -------------------------------------------------------------------- */
    /*  Decode first coefficient (DC coefficient) for INTRA block           */
    /* -------------------------------------------------------------------- */
        top0 = (unsigned) (ltop0>>8);

    /* -------------------------------------------------------------------- */
    /*  Intra DC: decode dct_size and len (luminance and chrominance)       */
    /* -------------------------------------------------------------------- */
        a_cc0 = a_cc1 = 0;
        b = _lmbd(0, top0);
        c = top0 >> 30;
        d = top0 >> 29;

        len = b + 1;
        if (cc==0)
        {
            dc_size = b + 2;
            a_cc0 = (b>=9);         /* the last VLC doesn't have a final 0! */
            b = b >> 1;
            d -= 4;
        }
        else
        {
            dc_size = b + 1;
            a_cc1 = (b>=10);        /* the last VLC doesn't have a final 0! */
            d = 1;                                          /* anything !=0 */
        }

        if (!b) len++;
        if (!c) dc_size--;
        if (!d) dc_size-=3;
        if (a_cc0)
        {
            len=9;
            dc_size=11;
        }
        if (a_cc1)
        {
            len=10;
            dc_size=11;
        }

    /* -------------------------------------------------------------------- */
    /*  Intra DC: obtain QFS[0] from dc_size and dc_differential            */
    /* -------------------------------------------------------------------- */
        dc_diff=0;
        t1 = top0 << len;
        if (dc_size!=0)
        {
            half_range = 1 << (dc_size-1);
            dc_diff = t1 >> (32-dc_size);
            if (dc_diff < half_range)
                dc_diff = (dc_diff+1)-(2*half_range);
        }
        val = (dc_pred[cc]+= dc_diff);

    /* -------------------------------------------------------------------- */
    /*  Intra DC: de-quantization and store result                          */
    /* -------------------------------------------------------------------- */
        outi[block*64+0] = val << (3-intra_dc_precision);

    /* -------------------------------------------------------------------- */
    /*  Intra DC: mismatch control                                          */
    /* -------------------------------------------------------------------- */
        sum += outi[block*64+0];

    /* -------------------------------------------------------------------- */
    /*  Intra DC: now that we know the length of the current VL code we     */
    /*  can advance bitstream to next one, i.e. by len+dc_size:             */
    /* -------------------------------------------------------------------- */
        len+=dc_size;

    /* -------------------------------------------------------------------- */
    /*  from here it's the same code as used in the loop for the other      */
    /*  coefficients                                                        */
    /* -------------------------------------------------------------------- */

    /* -------------------------------------------------------------------- */
    /*  1. update ltop0 from ltop0 and top1                                 */
    /* -------------------------------------------------------------------- */
        t5  = ltop0 << len;
        t7  = top1 >> (32-len);
        ltop0 = t5 + t7;

    /* -------------------------------------------------------------------- */
    /*  2. update top1 from word1 and word2 after increasing bptr by len.   */
    /*  if neccesary (i.e. if new bptr is greater than 32) update word1     */
    /*  and word2 from memory first. Don't forget that bptr is always       */
    /*  relative to the next lower word boundary and therefore needs to be  */
    /*  ANDed with 31 in case it become >=32.                               */
    /* -------------------------------------------------------------------- */
        bptr += len;
        test2 = (bptr >= 32);
        if (test2) {
            word1_rw = word1;
            word1 = word2;
            word2 = bsbuf[next_wptr];
            next_wptr += 1;
            next_wptr &= (bsbuf_words-1);
        }
        bptr = bptr & 31;
        bptr_cmpl = 32 - bptr;
        t8 = SHL(word1, bptr);
        t9 = SHR(word2, bptr_cmpl);                      /* unsigned shift */
        top1 = t8 + t9;

        Wptr++;
        zzptr++;

    /* -------------------------------------------------------------------- */
    /*  Decode AC coefficients                                              */
    /* -------------------------------------------------------------------- */
        while (!eob_err)
        {

        /* ---------------------------------------------------------------- */
        /*  Length computation                                              */
        /* ---------------------------------------------------------------- */

        /* ---------------------------------------------------------------- */
        /*  _lnorm returns the number of redundant sign bits in ltop0, e.g. */
        /*  0001 xxxx ... xxxx returns 2, 1111 10xx ... xxxx returns 4.     */
        /*  Example: top=0000 101s xxxx xxxx, then nrm=3                    */
        /* ---------------------------------------------------------------- */
            nrm = _lnorm(ltop0);

        /* ---------------------------------------------------------------- */
        /*  get rid of the leading all 0s/1s.                               */
        /*  Example: t1=0101 sxxx xxxx xxxx                                 */
        /* ---------------------------------------------------------------- */
            t1  = ltop0 << nrm;

        /* ---------------------------------------------------------------- */
        /*  use the number of leading bits (norm) as index,                 */
        /*  Example: t2= xxxx xxxx 0011 rrrr                                */
        /* ---------------------------------------------------------------- */
            t2  = nrm << 4;

            if (intra_vlc_format==0)
                t3 = &IMG_len_tbl0[t2];
            else /* intra_vlc_format==1 */
                t3 = &IMG_len_tbl1[t2];

        /* ---------------------------------------------------------------- */
        /*  use 4 extra bits after leading bits to distinguish special      */
        /*  cases (40-4=36)                                                 */
        /* ---------------------------------------------------------------- */
            t4  = (unsigned) (t1 >> 36);
        /* ---------------------------------------------------------------- */
        /*  get len from table and calculate 32-len                         */
        /* ---------------------------------------------------------------- */
            len  = t3[t4];
            len_c = 32 - len;

        /* ---------------------------------------------------------------- */
        /*  now that we know the length of the current VL code we can       */
        /*  advance bitstream to next one:                                  */
        /*                                                                  */
        /*  1. update ltop0 from ltop0 and top1                             */
        /* ---------------------------------------------------------------- */
            t5  = ltop0 << len;
            t7  = top1 >> len_c;
            top0_bk = (unsigned)(ltop0 >> 8);
            ltop0 = t5 + t7;

        /* ---------------------------------------------------------------- */
        /*  2. update top1 from word1 and word2 after increasing bptr by    */
        /*  len.  if neccesary (i.e. if new bptr is greater than 32)        */
        /*  update word1 and word2 from memory first. Don't forget that     */
        /*  bptr is always relative to the next lower word boundary and     */
        /*  therefore needs to be ANDed with 31 in case it become >=32.     */
        /* ---------------------------------------------------------------- */
            bptr += len;
            test2 = (bptr >= 32);
            if (test2) {
                word1_rw = word1;
                word1 = word2;
                word2 = bsbuf[next_wptr];
                next_wptr += 1;
                next_wptr &= (bsbuf_words-1);
            }
            bptr = bptr & 31;
            bptr_cmpl = 32 - bptr;
            t8 = SHL(word1, bptr);
            t9 = SHR(word2, bptr_cmpl);                   /* unsigned shift */
            top1 = t8 + t9;

        /* ---------------------------------------------------------------- */
        /*  Run-Level Decode                                                */
        /* ---------------------------------------------------------------- */

        /* ---------------------------------------------------------------- */
        /*  check if it is an ESCAPE code which has a unique fixed length   */
        /*  of 24 bits                                                      */
        /* ---------------------------------------------------------------- */
            test1 = len - 24;

            if (!test1)
            {
            /* ------------------------------------------------------------ */
            /* ESCAPE code: no look up required, just extract bits: 6 bits  */
            /* for ESCAPE, 6 bits for RUN, then 12 bits for LEVEL           */
            /* ------------------------------------------------------------ */
                run = _extu(top0_bk, 6, 26);
                level = _ext(top0_bk, 12, 20);
            }
            else
            {
                rld_left = len-5;
                if (len<5) rld_left=0;

                /* -------------------------------------------------------- */
                /*  last 5 bits of VLC incl. sign form 2nd part of index    */
                /* -------------------------------------------------------- */
                rld_index = ((len)<<5) + _extu(top0_bk, rld_left, 32-5);

                if (intra_vlc_format==0)
                   run_level = IMG_rld_table0[rld_index];
                else /* intra_vlc_format==1 */
                   run_level = IMG_rld_table1[rld_index];

                run   = run_level >> 8;
                level = (char)run_level;
            }

            eob_err = (run >= 64);

        /* ---------------------------------------------------------------- */
        /*  Run-lengh expansion and DQ                                      */
        /* ---------------------------------------------------------------- */

        /* ---------------------------------------------------------------- */
        /*  Dequantisation: *out_i = ((2*level + Sign(level)) * W * qscl)   */
        /*  / 32. Sign(x)=-1 for x<0, 0 for x=0, +1 for x>0. Division '/':  */
        /*  "Integer division with truncation of the result toward zero.    */
        /*  For example, 7/4 and -7/-4 are truncated to 1 and -7/4 and      */
        /*  7/-4 are truncated to -1." (MPEG-2 Standard Text)               */
        /* ---------------------------------------------------------------- */
            neg = (level < 0);
            f1 = 2*level;

        /* ---------------------------------------------------------------- */
        /*  find quantization matrix element at zigzag position and         */
        /*  multiply f1 with W * qscl                                       */
        /* ---------------------------------------------------------------- */
            if (!eob_err)         /* prevent from accessing memory when EOB */
            {
                W = *(Wptr += run);
                Wptr++;
                /* detect total run overrun */
                eob_err = (Wptr > Wptr_end);
            }

            qW = qscl * W;
            f3 = f1 * qW;

        /* ---------------------------------------------------------------- */
        /*  for negative numbers we first need to add 31 before dividing    */
        /*  by 32 to achieve truncation towards zero as required by the     */
        /*  standard.                                                       */
        /* ---------------------------------------------------------------- */
            if (neg) f3 += 31;

        /* ---------------------------------------------------------------- */
        /*  saturate to signed 12 bit word (with /32 <-> >>5 incorporated)  */
        /*  SSHL: shift left and saturate to 32 bits                        */
        /* ---------------------------------------------------------------- */
            f4 = _sshl(f3, 15);

            f5 = (int)(f4 >> shr);
            f5 &= mask;

        /* ---------------------------------------------------------------- */
        /*  mismatch control: determine if sum of coefficents is odd or     */
        /*  even                                                            */
        /* ---------------------------------------------------------------- */
            if (!eob_err)
                sum += f5;

        /* ---------------------------------------------------------------- */
        /*  find un-zigzag position of DCT coefficient                      */
        /* ---------------------------------------------------------------- */
            if (!eob_err)         /* prevent from accessing memory when EOB */
                cnum = *(zzptr += run);
            zzptr++;
            if (!eob_err)
                outi[block*64+cnum] = f5;

        } /* while */

    /* -------------------------------------------------------------------- */
    /*  mismatch control: toggle last bit of last coefficient if sum of     */
    /*  coefficents is even.                                                */
    /* -------------------------------------------------------------------- */

        if ((sum & lsb)==0)            // 12Q4
     {
            outi[block*64+63] ^= lsb;  // 12Q4
     }

    /* -------------------------------------------------------------------- */
    /*  Determine nature of fault, invalid code word or exceeding of the    */
    /*  allowed total run of 64.                                            */
    /* -------------------------------------------------------------------- */
    fault = (run>65) || (Wptr > Wptr_end); //- Wptr_origin)>64);

    if (fault) break;

    } /* for */

    /* -------------------------------------------------------------------- */
    /*  rewind bitstream by 8 bits to convert back to 32-bit top0           */
    /* -------------------------------------------------------------------- */
    top0 = (unsigned) (ltop0 >> 8);
    top1 = (top1 >> 8) + (unsigned)(ltop0 << 24);
    bptr = bptr - 8;
    if (bptr<0)
    {
        word2 = word1;
        word1 = word1_rw;
        bptr += 32;
        next_wptr -= 1;
        next_wptr &= (bsbuf_words-1);
    }

    /* -------------------------------------------------------------------- */
    /*  Update bitstream variables                                          */
    /* -------------------------------------------------------------------- */
    Mpeg2v->next_wptr = next_wptr;
    Mpeg2v->bptr      = bptr;
    Mpeg2v->word1     = word1;
    Mpeg2v->word2     = word2;
    Mpeg2v->top0      = top0;
    Mpeg2v->top1      = top1;
    Mpeg2v->fault     = fault;
}

/* ======================================================================== */
/*  End of file:  img_mpeg2_vld_intra.c                                     */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.4     Thu Jul 19 22:35:27 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_perimeter                                                       */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      09-Apr-2001                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This function is C callable, and has the following prototype:       */
/*                                                                          */
/*          int IMG_perimeter                                               */
/*          (                                                               */
/*              const unsigned char *restrict in,  // Input image    //     */
/*              int cols,                          // Width of input //     */
/*              unsigned char       *restrict out  // Output image   //     */
/*          );                                                              */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      This function returns the boundary pixels of an image.              */
/*      Each call of IMG_perimeter() calculates one new line of output      */
/*      from a three line window of input.                                  */
/*                                                                          */
/*      The input pointer "in" points to the middle line of a three-line    */
/*      window of the image.  The IMG_perimeter function scans this window  */
/*      looking for pixels in the middle row which are on the IMG_perimeter */
/*      of the image.  A pixel is on the IMG_perimeter of the image if      */
/*      the pixel itself is non-zero, but one of its neighbors is zero.     */
/*      The total count of IMG_perimeter pixels is returned.                */
/*                                                                          */
/*      This particular implementation evaluates four neighbors for         */
/*      the IMG_perimeter test:  The neighbors to the left, right, top      */
/*      and bottom.                                                         */
/*                                                                          */
/*      Perimeter pixels retain their original grey level in the            */
/*      output.  Non-IMG_perimeter pixels are set to zero in the output.    */
/*      Pixels on the far left and right edges of a row are defined         */
/*      as *not* being on the IMG_perimeter, and are always forced to       */
/*      zero.                                                               */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      The optimized implementations assume that the input and output      */
/*      arrays are double-word aligned.  They also assume that the          */
/*      input is a multiple of 16 pixels long.                              */
/*                                                                          */
/*  BIBLIOGRAPHY                                                            */
/*      Image Processing by Gonzalez and Woods.                             */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */





int IMG_perimeter
(
    const unsigned char *restrict in,
    int cols,
    unsigned char       *restrict out
)
{
    int i, count = 0;
    unsigned char pix_lft, pix_rgt, pix_top, pix_bot, pix_mid;

    for(i = 0; i < cols; i++)
    {
        pix_lft = in[i - 1];
        pix_mid = in[i + 0];
        pix_rgt = in[i + 1];

        pix_top = in[i - cols];
        pix_bot = in[i + cols];

        if (((pix_lft == 0) || (pix_rgt == 0) ||
             (pix_top == 0) || (pix_bot == 0)) && (pix_mid > 0))
        {
            out[i] = pix_mid;
            count++;
        } else
        {
            out[i] = 0;
        }
    }
    if (out[cols-1]) count--;
    if (out[0])      count--;
    out[0] = out[cols-1] = 0;

    return count;
}

/* ======================================================================== */
/*  End of file:  img_perimeter.c                                           */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.1     Thu Jul 19 22:35:37 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_pix_expand                                                      */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      14-Oct-2000                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*      void IMG_pix_expand                                                 */
/*      (                                                                   */
/*          int n,                                    // # of elements //   */
/*          const unsigned char *restrict in_data,    // Input data    //   */
/*          short               *restrict out_data    // Output data   //   */
/*      )                                                                   */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      Reads an array of unsigned 8-bit values and store them to a         */
/*      16-bit array.  This step is often used as the first step            */
/*      when processing pixels or other low-precision data at 16-bit        */
/*      intermediate precision.                                             */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_pix_expand
(
    int n,
    const unsigned char *restrict in_data,
    short               *restrict out_data
)
{
    int i;

    for (i = 0; i < n; i++)
        out_data[i] =  in_data[i];
}

/* ======================================================================== */
/*  End of file:  img_pix_expand.c                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.2     Thu Jul 19 22:35:36 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_pix_sat                                                         */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      02-Oct-2000                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C callable, and has the following C prototype:      */
/*                                                                          */
/*          void IMG_pix_sat                                                */
/*          (                                                               */
/*              int n,                            // Number of pixels //    */
/*              const short   *restrict in_data,  // Incoming data    //    */
/*              unsigned char *restrict out_data  // Outgoing data    //    */
/*          );                                                              */
/*                                                                          */
/*      This code performs the saturation of 16 bit signed numbers          */
/*      to 8 bit unsigned numbers. If the data is over 255 it is            */
/*      saturated to 255, if it is less than 0 it is saturated to 0.        */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The function IMG_pix_sat() takes signed 16-bit input pixels and     */
/*      saturates them to unsigned 8-bit results.  Pixel values above       */
/*      255 are clamped to 255, and values below 0 are clamped to 0.        */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
void IMG_pix_sat
(
    int                     n,
    const short   *restrict in_data,
    unsigned char *restrict out_data
)
{
    int i, pixel;

    for (i = 0; i < n; i++)
    {
        pixel = in_data[i];
        if (pixel > 0xFF)
        {
            out_data[i] = 0xFF;
        } else if (pixel < 0x00)
        {
            out_data[i] = 0x00;
        } else
        {
            out_data[i] = pixel;
        }
    }
}

/* ======================================================================== */
/*  End of file:  img_pix_sat.c                                             */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.7     Tue Mar 12 15:59:16 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*                                                                          */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_quantize -- Matrix Quantization w/ Rounding, Little Endian      */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      02-Oct-2000                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C callable, and has the following C prototype:      */
/*                                                                          */
/*      void IMG_quantize                                                   */
/*      (                                                                   */
/*          short           *data,      (* Data to be quantized.        *)  */
/*          unsigned short  num_blks,   (* Number of 64-element blocks. *)  */
/*          unsigned short  blk_sz,     (* Block size (multiple of 16). *)  */
/*          const short     *recip_tbl, (* Quant. values (reciprocals). *)  */
/*          int             q_pt        (* Q-point of Quant values.     *)  */
/*      )                                                                   */
/*                                                                          */
/*      The number of blocks, num_blks, must be at least 1.  The block      */
/*      size (number of elements in each block) must be at least 1.         */
/*      The Q-point, q_pt, controls rounding and final truncation; it       */
/*      must be in the range from 0 <= q_pt <= 31.                          */
/*                                                                          */
/*      The data[] array must be 'num_blks * blk_sz' elements, and the      */
/*      recip_tbl[] array must be 'blk_sz' elements.                        */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The function IMG_quantize() quantizes matrices by multiplying their */
/*      contents with a second matrix that contains reciprocals of the      */
/*      quantization terms.  This step corresponds to the quantization      */
/*      that is performed in 2-D DCT-based compression techniques,          */
/*      although IMG_quantize() may be used on any signed 16-bit data using */
/*      signed 16-bit quantization terms.                                   */
/*                                                                          */
/*      IMG_quantize() merely multiplies the contents of the quantization   */
/*      matrix with the data being quantized.  Therefore, it may be used    */
/*      for inverse quantization as well, by setting the Q-point            */
/*      appropriately.                                                      */
/*                                                                          */
/*  TECHNIQUES                                                              */
/*      The inner loop steps through individual blocks, while the           */
/*      outer loop steps through reciprocals for quantization.  This        */
/*      eliminates redundant loads for the quantization terms.              */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      The number of blocks, num_blks, may be zero.                        */
/*      The Q-point, q_pt, must be in the range 0 <= q_pt <= 31.            */
/*                                                                          */
/*  NOTES                                                                   */
/*      No checking is performed on the input arguments for correctness.    */
/*                                                                          */
/*  SOURCE                                                                  */
/*      n/a                                                                 */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

#include "IMG_quantize.h"

void IMG_quantize
(
    short           *restrict data,      /* Data to be quantized.         */
    unsigned short  num_blks,            /* Number of 64-element blocks.  */
    unsigned short  blk_size,            /* Block size (multiple of 16).  */
    const short     *restrict recip_tbl, /* Quant. values (reciprocals).  */
    int             q_pt                 /* Q-point of Quant values.      */
)
{
    short recip;                /* Reciprocal term.                 */
    int   quot;                 /* Quotient (data * reciprocal)     */
    int   round;                /* Rounding value.                  */
    int   i, j, k;              /* Loop counters.                   */

    round = 1 << (q_pt - 1);

    if (!num_blks) return;

    /* -------------------------------------------------------------------- */
    /*  Outer loop:  Step between quantization matrix elements.             */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < blk_size; i++)
    {
        /* ---------------------------------------------------------------- */
        /*  Load a reciprocal and point to appropriate element of block.    */
        /* ---------------------------------------------------------------- */
        recip   = recip_tbl[i];
        k       = i;

        /* ---------------------------------------------------------------- */
        /*  Inner loop:  Step between blocks of elements, quantizing.       */
        /* ---------------------------------------------------------------- */
        for (j = 0; j < num_blks; j++)
        {
            quot    = data[k] * recip + round;
            data[k] = quot >> q_pt;
            k      += blk_size;
        }
    }
}

/* ======================================================================== */
/*  End of file:  img_quantize.c                                            */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.1     Thu Jul 19 22:35:39 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  NAME                                                                    */
/*      IMG_sad_16x16 -- Sum of Absolute Differences on single 16x16 block  */
/*                                                                          */
/*  USAGE                                                                   */
/*      unsigned IMG_sad_16x16                                              */
/*      (                                                                   */
/*          const unsigned char *restrict srcImg,  // 16x16 source block // */
/*          const unsigned char *restrict refImg,  // Reference image    // */
/*          int pitch                              // Width of ref image // */
/*      );                                                                  */
/*                                                                          */
/*      The code accepts a pointer to the 16x16 source block (srcImg),      */
/*      and a pointer to the upper-left corner of a target position in      */
/*      a reference image (refImg).  The width of the reference image       */
/*      is given by the pitch argument.                                     */
/*                                                                          */
/*      The function returns the sum of the absolute differences            */
/*      between the source block and the 16x16 region pointed to in the     */
/*      reference image.                                                    */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The algorithm takes the difference between the pixel values in      */
/*      the source image block and the corresponding pixels in the          */
/*      reference image.  It then takes the absolute values of these        */
/*      differences, and accumulates them over the entire 16x16 region.     */
/*      It returns the final accumulation.                                  */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      Some versions of this kernel may assume that srcImg is double-      */
/*      word aligned.                                                       */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

unsigned IMG_sad_16x16
(
    const unsigned char *restrict srcImg,
    const unsigned char *restrict refImg,
    int pitch
)
{
    int i, j;
    unsigned sad = 0;


    for (i = 0; i < 16; i++)
        for (j = 0; j < 16; j++)
            sad += abs(srcImg[j + i*16] - refImg[j + i*pitch]);

    return sad;
}

/* ======================================================================== */
/*  End of file: img_sad_16x16.c                                            */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.1     Thu Jul 19 22:35:28 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  NAME                                                                    */
/*      IMG_sad_8x8   -- Sum of Absolute Differences on single 8x8 block    */
/*                                                                          */
/*  USAGE                                                                   */
/*      unsigned IMG_sad_8x8                                                */
/*      (                                                                   */
/*          const unsigned char *restrict srcImg,  // 8x8 source block   // */
/*          const unsigned char *restrict refImg,  // Reference image    // */
/*          int pitch                              // Width of ref image // */
/*      );                                                                  */
/*                                                                          */
/*      The code accepts a pointer to the 8x8 source block (srcImg),        */
/*      and a pointer to the upper-left corner of a target position in      */
/*      a reference image (refImg).  The width of the reference image       */
/*      is given by the pitch argument.                                     */
/*                                                                          */
/*      The function returns the sum of the absolute differences            */
/*      between the source block and the 8x8 region pointed to in the       */
/*      reference image.                                                    */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The algorithm takes the difference between the pixel values in      */
/*      the source image block and the corresponding pixels in the          */
/*      reference image.  It then takes the absolute values of these        */
/*      differences, and accumulates them over the entire 8x8 region.       */
/*      It returns the final accumulation.                                  */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      Some versions of this kernel may assume that srcImg is double-      */
/*      word aligned.                                                       */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

unsigned IMG_sad_8x8
(
    const unsigned char *restrict srcImg,
    const unsigned char *restrict refImg,
    int pitch
)
{
    int i, j;
    unsigned sad = 0;


    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++)
            sad += abs(srcImg[j + i*8] - refImg[j + i*pitch]);

    return sad;
}

/* ======================================================================== */
/*  End of file: img_sad_8x8.c                                              */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.1     Thu Jul 19 22:35:19 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_sobel                                                           */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      22-Jan-2001                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*      void IMG_sobel                                                      */
/*      (                                                                   */
/*          const unsigned char *in_data,      // Input image data  //      */
/*          unsigned char       *out_data,     // Output image data //      */
/*          short cols, short rows             // Image dimensions  //      */
/*      )                                                                   */
/*                                                                          */
/*      The IMG_sobel filter is applied to the input image. The input image */
/*      dimensions are given by the arguments 'cols' and 'rows'.  The       */
/*      output image is 'cols' pixels wide and 'rows - 2' pixels tall.      */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*                                                                          */
/*      To see how the implementation is going to work on the input         */
/*      buffer, lets imagine we have the following input buffer:            */
/*      lets imagine we have the following input buffer:                    */
/*                                                                          */
/*              yyyyyyyyyyyyyyyy                                            */
/*              yxxxxxxxxxxxxxxy                                            */
/*              yxxxxxxxxxxxxxxy                                            */
/*              yxxxxxxxxxxxxxxy                                            */
/*              yxxxxxxxxxxxxxxy                                            */
/*              yyyyyyyyyyyyyyyy                                            */
/*                                                                          */
/*      The output buffer would be:                                         */
/*                                                                          */
/*              tXXXXXXXXXXXXXXz                                            */
/*              zXXXXXXXXXXXXXXz                                            */
/*              zXXXXXXXXXXXXXXz                                            */
/*              zXXXXXXXXXXXXXXt                                            */
/*                                                                          */
/*      Where:                                                              */
/*                                                                          */
/*          X = IMG_sobel(x)    The algorithm is applied to that pixel.     */
/*                          The correct output is obtained, the data        */
/*                          around the pixels we work on is used            */
/*                                                                          */
/*          t               Whatever was in the output buffer in that       */
/*                          position is kept there.                         */
/*                                                                          */
/*          z = IMG_sobel(y)    The algorithm is applied to that pixel.     */
/*                          The output is not meaningful, because the       */
/*                          necessary data to process the pixel is not      */
/*                          available.  This is because for each output     */
/*                          pixel we need input pixels from the right and   */
/*                          from the left of the output pixel.  But this    */
/*                          data doesn't exist.                             */
/*                                                                          */
/*      This means that we will only process (rows-2) lines.  Then, we      */
/*      will process all the pixels inside each line. Even though the       */
/*      results for the first and last pixels in each line will not         */
/*      be relevant, it makes the control much simpler and ends up          */
/*      saving cycles.                                                      */
/*                                                                          */
/*      Also the fist pixel in the first processed line and the             */
/*      last pixel in the last processed line will not be computed.         */
/*      It is not necessary, since the results would be bogus, and          */
/*      not computing them saves some time.                                 */
/*                                                                          */
/*      The following horizontal and vertical masks that are                */
/*      applied to the input buffer to obtain one output pixel.             */
/*                                                                          */
/*          Horizontal:                                                     */
/*              -1 -2 -1                                                    */
/*               0  0  0                                                    */
/*               1  2  1                                                    */
/*                                                                          */
/*          Vertical:                                                       */
/*              -1  0  1                                                    */
/*              -2  0  2                                                    */
/*              -1  0  1                                                    */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      n/a                                                                 */
/*                                                                          */
/*  NOTES                                                                   */
/*      The values of the left-most and right-most pixels on each line      */
/*      of the output are not well-defined.                                 */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_sobel
(
    const unsigned char *restrict in,   /* Input image data   */
    unsigned char       *restrict out,  /* Output image data  */
    short cols, short rows              /* Image dimensions   */
)
{
    int H, O, V, i;
    int i00, i01, i02;
    int i10,      i12;
    int i20, i21, i22;
    int w = cols;

    /* -------------------------------------------------------------------- */
    /*  Iterate over entire image as a single, continuous raster line.      */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < cols*(rows-2) - 2; i++)
    {
        /* ---------------------------------------------------------------- */
        /*  Read in the required 3x3 region from the input.                 */
        /* ---------------------------------------------------------------- */
        i00=in[i    ]; i01=in[i    +1]; i02=in[i    +2];
        i10=in[i+  w];                  i12=in[i+  w+2];
        i20=in[i+2*w]; i21=in[i+2*w+1]; i22=in[i+2*w+2];

        /* ---------------------------------------------------------------- */
        /*  Apply horizontal and vertical filter masks.  The final filter   */
        /*  output is the sum of the absolute values of these filters.      */
        /* ---------------------------------------------------------------- */

        H = -   i00 - 2*i01 -   i02 +
            +   i20 + 2*i21 +   i22;

        V = -   i00         +   i02
            - 2*i10         + 2*i12
            -   i20         +   i22;

        O = abs(H) + abs(V);

        /* ---------------------------------------------------------------- */
        /*  Clamp to 8-bit range.  The output is always positive due to     */
        /*  the absolute value, so we only need to check for overflow.      */
        /* ---------------------------------------------------------------- */
        if (O > 255) O = 255;

        /* ---------------------------------------------------------------- */
        /*  Store it.                                                       */
        /* ---------------------------------------------------------------- */
        out[i + 1] = O;
    }
}

/* ======================================================================== */
/*  End of file:  img_sobel.c                                               */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.7     Fri Sep 13 20:46:22 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_thr_gt2max                                                      */
/*                                                                          */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      13-Mar-2002                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine has the following C prototype:                         */
/*                                                                          */
/*      void IMG_thr_gt2max                                                 */
/*      (                                                                   */
/*          const unsigned char *in_data,     //  Input image data  //      */
/*          unsigned char *restrict out_data, //  Output image data //      */
/*          short cols, short rows,           //  Image dimensions  //      */
/*          unsigned char       threshold     //  Threshold value   //      */
/*      )                                                                   */
/*                                                                          */
/*      This routine performs a thresholding operation on an input          */
/*      image in in_data[] whose dimensions are given in the arguments      */
/*      'cols' and 'rows'.  The thresholded pixels are written to the       */
/*      output image pointed to by out_data[].  The input and output        */
/*      are exactly the same dimensions.                                    */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      Pixels that are above the threshold value are written to the        */
/*      output unmodified.  Pixels that are greater than the threshold      */
/*      are set to 255 in the output image.                                 */
/*                                                                          */
/*      The exact thresholding function performed is described by           */
/*      the following transfer function diagram:                            */
/*                                                                          */
/*                                                                          */
/*                 255_|          _________                                 */
/*                     |         |                                          */
/*                     |         |                                          */
/*            O        |         |                                          */
/*            U        |         |                                          */
/*            T    th _|. . . . .|                                          */
/*            P        |        /.                                          */
/*            U        |      /  .                                          */
/*            T        |    /    .                                          */
/*                     |  /      .                                          */
/*                   0_|/________.__________                                */
/*                     |         |        |                                 */
/*                     0        th       255                                */
/*                                                                          */
/*                             INPUT                                        */
/*                                                                          */
/*      Please see the IMGLIB functions IMG_thr_gt2thr, IMG_thr_le2thr      */
/*      and IMG_thr_le2min for other thresholding functions.                */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      The input and output buffers do not alias.                          */
/*                                                                          */
/*  MEMORY NOTE                                                             */
/*      This code is ENDIAN NEUTRAL.                                        */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_thr_gt2max
(
    const unsigned char *in_data,                /*  Input image data    */
    unsigned char       *restrict out_data,      /*  Output image data   */
    short cols, short rows,                      /*  Image dimensions    */
    unsigned char       threshold                /*  Threshold value     */
)
{
    int i, pixels = rows * cols;

    /* -------------------------------------------------------------------- */
    /*  Step through input image copying pixels to the output.  If the      */
    /*  pixels are above our threshold, set them to 255.                    */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < pixels; i++)
        out_data[i] = in_data[i] > threshold ? 255 : in_data[i];
}

/* ======================================================================== */
/*  End of file:  img_thr_gt2max.c                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.3     Fri Sep 13 20:48:07 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_thr_gt2thr                                                      */
/*                                                                          */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      14-Mar-2002                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine has the following C prototype:                         */
/*                                                                          */
/*      void IMG_thr_gt2thr                                                 */
/*      (                                                                   */
/*          const unsigned char *in_data,     //  Input image data  //      */
/*          unsigned char *restrict out_data, //  Output image data //      */
/*          short cols, short rows,           //  Image dimensions  //      */
/*          unsigned char       threshold     //  Threshold value   //      */
/*      )                                                                   */
/*                                                                          */
/*      This routine performs a thresholding operation on an input          */
/*      image in in_data[] whose dimensions are given in the arguments      */
/*      'cols' and 'rows'.  The thresholded pixels are written to the       */
/*      output image pointed to by out_data[].  The input and output        */
/*      are exactly the same dimensions.                                    */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      Pixels that are above the threshold value are written to the        */
/*      output unmodified.  Pixels that are greater than the threshold      */
/*      are set to the threshold value in the output image.                 */
/*                                                                          */
/*      The exact thresholding function performed is described by           */
/*      the following transfer function diagram:                            */
/*                                                                          */
/*                                                                          */
/*                 255_|                                                    */
/*                     |                                                    */
/*                     |                                                    */
/*            O        |                                                    */
/*            U        |                                                    */
/*            T    th _|. . . . . _________                                 */
/*            P        |        /.                                          */
/*            U        |      /  .                                          */
/*            T        |    /    .                                          */
/*                     |  /      .                                          */
/*                   0_|/________.__________                                */
/*                     |         |        |                                 */
/*                     0        th       255                                */
/*                                                                          */
/*                             INPUT                                        */
/*                                                                          */
/*      Please see the IMGLIB functions IMG_thr_le2thr, IMG_thr_gt2max      */
/*      and IMG_thr_le2min for other thresholding functions.                */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      The input and output buffers do not alias.                          */
/*                                                                          */
/*  MEMORY NOTE                                                             */
/*      This code is ENDIAN NEUTRAL.                                        */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_thr_gt2thr
(
    const unsigned char *in_data,                /*  Input image data    */
    unsigned char       *restrict out_data,      /*  Output image data   */
    short cols, short rows,                      /*  Image dimensions    */
    unsigned char       threshold                /*  Threshold value     */
)
{
    int i, pixels = rows * cols;

    /* -------------------------------------------------------------------- */
    /*  Step through input image copying pixels to the output.  If the      */
    /*  pixels are above our threshold, set them to the threshold value.    */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < pixels; i++)
        out_data[i] = in_data[i] > threshold ? threshold : in_data[i];
}

/* ======================================================================== */
/*  End of file:  img_thr_gt2thr.c                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.5     Fri Sep 13 20:49:02 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_thr_le2min                                                      */
/*                                                                          */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      13-Mar-2002                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine has the following C prototype:                         */
/*                                                                          */
/*      void IMG_thr_le2min                                                 */
/*      (                                                                   */
/*          const unsigned char *in_data,     //  Input image data  //      */
/*          unsigned char *restrict out_data, //  Output image data //      */
/*          short cols, short rows,           //  Image dimensions  //      */
/*          unsigned char       threshold     //  Threshold value   //      */
/*      )                                                                   */
/*                                                                          */
/*      This routine performs a thresholding operation on an input          */
/*      image in in_data[] whose dimensions are given in the arguments      */
/*      'cols' and 'rows'.  The thresholded pixels are written to the       */
/*      output image pointed to by out_data[].  The input and output        */
/*      are exactly the same dimensions.                                    */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      Pixels that are above the threshold value are written to the        */
/*      output unmodified.  Pixels that are less than or equal to the       */
/*      threshold are set to 0 in the output image.                         */
/*                                                                          */
/*      The exact thresholding function performed is described by           */
/*      the following transfer function diagram:                            */
/*                                                                          */
/*                                                                          */
/*                 255_|                                                    */
/*                     |                  /                                 */
/*                     |                /                                   */
/*            O        |              /                                     */
/*            U        |            /                                       */
/*            T    th _|. . . . . /                                         */
/*            P        |         |                                          */
/*            U        |         |                                          */
/*            T        |         |                                          */
/*                     |         |                                          */
/*                   0_|_________|__________                                */
/*                     |         |        |                                 */
/*                     0        th       255                                */
/*                                                                          */
/*                             INPUT                                        */
/*                                                                          */
/*      Please see the IMGLIB functions IMG_thr_gt2thr, IMG_thr_le2thr,     */
/*      and IMG_thr_gt2max for other thresholding functions.                */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      The input and output buffers do not alias.                          */
/*                                                                          */
/*  MEMORY NOTE                                                             */
/*      This code is ENDIAN NEUTRAL.                                        */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_thr_le2min
(
    const unsigned char *in_data,                /*  Input image data    */
    unsigned char       *restrict out_data,      /*  Output image data   */
    short cols, short rows,                      /*  Image dimensions    */
    unsigned char       threshold                /*  Threshold value     */
)
{
    int i, pixels = rows * cols;

    /* -------------------------------------------------------------------- */
    /*  Step through input image copying pixels to the output.  If the      */
    /*  pixels are below or equal to our threshold, set them to 0.          */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < pixels; i++)
        out_data[i] = in_data[i] <= threshold ? 0 : in_data[i];
}

/* ======================================================================== */
/*  End of file:  img_thr_le2min.c                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.2     Fri Sep 13 20:39:06 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_thr_le2thr                                                      */
/*                                                                          */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      14-Mar-2002                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine has the following C prototype:                         */
/*                                                                          */
/*      void IMG_thr_le2thr                                                 */
/*      (                                                                   */
/*          const unsigned char *in_data,     //  Input image data  //      */
/*          unsigned char *restrict out_data, //  Output image data //      */
/*          short cols, short rows,           //  Image dimensions  //      */
/*          unsigned char       threshold     //  Threshold value   //      */
/*      )                                                                   */
/*                                                                          */
/*      This routine performs a thresholding operation on an input          */
/*      image in in_data[] whose dimensions are given in the arguments      */
/*      'cols' and 'rows'.  The thresholded pixels are written to the       */
/*      output image pointed to by out_data[].  The input and output        */
/*      are exactly the same dimensions.                                    */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      Pixels that are above the threshold value are written to the        */
/*      output unmodified.  Pixels that are greater than the threshold      */
/*      are set to the threshold value in the output image.                 */
/*                                                                          */
/*      The exact thresholding function performed is described by           */
/*      the following transfer function diagram:                            */
/*                                                                          */
/*                                                                          */
/*                 255_|                                                    */
/*                     |                  /                                 */
/*                     |                /                                   */
/*            O        |              /                                     */
/*            U        |            /                                       */
/*            T    th _|_________ /                                         */
/*            P        |         .                                          */
/*            U        |         .                                          */
/*            T        |         .                                          */
/*                     |         .                                          */
/*                   0_|_________.__________                                */
/*                     |         |        |                                 */
/*                     0        th       255                                */
/*                                                                          */
/*                             INPUT                                        */
/*                                                                          */
/*      Please see the IMGLIB functions IMG_thr_gt2thr, IMG_thr_le2min      */
/*      and IMG_thr_gt2max for other thresholding functions.                */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      The input and output buffers do not alias.                          */
/*                                                                          */
/*  MEMORY NOTE                                                             */
/*      This code is ENDIAN NEUTRAL.                                        */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_thr_le2thr
(
    const unsigned char *in_data,                /*  Input image data    */
    unsigned char       *restrict out_data,      /*  Output image data   */
    short cols, short rows,                      /*  Image dimensions    */
    unsigned char       threshold                /*  Threshold value     */
)
{
    int i, pixels = rows * cols;

    /* -------------------------------------------------------------------- */
    /*  Step through input image copying pixels to the output.  If the      */
    /*  pixels are below our threshold, set them to the threshold value.    */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < pixels; i++)
        out_data[i] = in_data[i] <= threshold ? threshold : in_data[i];
}

/* ======================================================================== */
/*  End of file:  img_thr_le2thr.c                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.5     Thu Sep  6 18:51:03 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_wave_horz                                                       */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      15-Oct-2000                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*      i_data  = input row of data                                         */
/*      lp_filt = Low-pass quadrature mirror filter                         */
/*      hp_filt = High-pass quadrature mirror filter                        */
/*      o_data  = output row of detailed/reference decimated outputs        */
/*      x_dim   = width of the input row                                    */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      This kernel performs a 1D Periodic Orthogonal Wavelet               */
/*      decomposition.  This also performs athe row decomposition in a      */
/*      2D wavelet transform.  An in put signal x[n] is first low pass      */
/*      and high pass filterd and decimated by two.  This results in a      */
/*      reference signal r1[n] which is the decimated output obtained       */
/*      by dropping the odd samples of the low pass filtered output and     */
/*      a detail signal d[n] obtained by dropping the odd samples of        */
/*      the high-pass output.  A circular convolution algorithm is          */
/*      implemented and hence the wavelet transform is periodic.  The       */
/*      reference signal and the detail signal are half the size of the     */
/*      original signal.  The reference signal may then be iterated         */
/*      again to perform another scale of multi-resolution analysis.        */
/*                                                                          */
/*  BIBLIOGRAPHY                                                            */
/*      A Wavelet Tour of Signal Processing Stephane' Mallat. Academic      */
/*      Press                                                               */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_wave_horz
(
    const short *restrict i_data,    /* Row of input pixels  */
    const short *restrict lp_filt,   /* Low-pass QMF filter  */
    const short *restrict hp_filt,   /* High-pass QMF filter */
    short       *restrict o_data,    /* Row of output data   */
    int                   x_dim      /* Length of input.     */
)
{
    int i, j;

    for (i = 0; i < x_dim; i += 2)
    {
        int lp_sum = 1 << 14;

        for (j = 0; j < 8; j++)
            lp_sum += i_data[(i + j) % x_dim] * lp_filt[j];

        o_data[i >> 1] = lp_sum >> 15;
    }

    for (i = 0; i < x_dim; i += 2)
    {
        int hp_sum = 1 << 14;

        for (j = 0; j < 8; j++)
            hp_sum += i_data[(i + x_dim - 6 + j) % x_dim] * hp_filt[7 - j];

        o_data[(i + x_dim) >> 1] = hp_sum >> 15;
    }
}
/* ======================================================================== */
/*  End of file:  img_wave_horz.c                                           */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.3     Thu Jul 19 22:35:31 2001 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_wave_vert                                                       */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      18-Oct-2000                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*          void IMG_wave_vert                                              */
/*          (                                                               */
/*              const short *restrict                                       */
/*                    const *restrict in_data,                              */
/*              const short *restrict lp_filt,                              */
/*              const short *restrict hp_filt,                              */
/*              short       *restrict out_ldata,                            */
/*              short       *restrict out_hdata,                            */
/*              int cols                                                    */
/*          )                                                               */
/*                                                                          */
/*          in_data   = Eight pointers to input image rows.                 */
/*          lp_filt   = Low-pass quadrature mirror filter coeffs (8 taps)   */
/*          hp_filt   = High-pass quadrature mirror filter coeffs (8 taps)  */
/*          out_ldata = Outgoing low-pass image data.                       */
/*          out_hdata = Outgoing high-pass image data.                      */
/*          cols      = Total number of colums in row of image.             */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      This function performs the vertical pass of 2D wavelet              */
/*      transform It performs a vertical filter on 8 rows which are         */
/*      pointed to by the pointers contained in an array of pointers.       */
/*      It produces two lines worth of output, one being the low-pass       */
/*      and the other being the high pass result.  Instead of               */
/*      transposing the input image and re-using the horizontal wavelet     */
/*      function, the vertical filter is applied directly to the image      */
/*      data as-is, producing a single line of high pass and a single       */
/*      line of low pass outputs.                                           */
/*                                                                          */
/*      This function accepts eight row pointers for the eight rows         */
/*      that form the context for the two filters.  It also accepts         */
/*      two output pointers, one for the high-pass filtered output          */
/*      and one for the low-pass filtered output.  Finally, it also         */
/*      accepts two input pointers for the low- and high-pass filters.      */
/*                                                                          */
/*      In a traditional wavelet implementation, for a given pair of        */
/*      output lines, the input context for the low-pass filter is          */
/*      offset by a number of lines from the input context for the          */
/*      high-pass filter.  The amount of offset is determined by the        */
/*      number of filter taps and is generally "num_taps - 2" rows.         */
/*      This implementation is fixed at 8 taps, so the offset would be      */
/*      6 rows.                                                             */
/*                                                                          */
/*      This implementation breaks from the traditional model so that       */
/*      it can reuse the same input context for both low-pass and           */
/*      high-pass filters simultaneously.  The result is that the           */
/*      low-pass and high-pass *outputs* must instead be offset by the      */
/*      calling function.  The following sample pseudo-code illustrates     */
/*      one possible method for producing the desired result.               */
/*                                                                          */
/*          // ------------------------------------------------------ //    */
/*          //  Iterate over whole input image, starting at the top.  //    */
/*          // ------------------------------------------------------ //    */
/*          for (y = 0; y < y_dim; y += 2)                                  */
/*          {                                                               */
/*              // -------------------------------------------------- //    */
/*              //  Set up our eight input pointers, wrapping around  //    */
/*              //  the bottom as necessary.                          //    */
/*              // -------------------------------------------------- //    */
/*              for (y0 = 0; y0 < 8; y0++)                                  */
/*                  in_data[y0] = in_image + ((y + y0) % y_dim) * x_dim;    */
/*                                                                          */
/*              // -------------------------------------------------- //    */
/*              //  Set up our output pointers.  The high-pass data   //    */
/*              //  is three rows ahead of the low-pass data.         //    */
/*              // -------------------------------------------------- //    */
/*              out_ldata = out_image + (y / 2);                            */
/*              out_hdata = out_image + ((y + 6) % y_dim) / 2 + y_dim;      */
/*                                                                          */
/*              // -------------------------------------------------- //    */
/*              //  Perform the wavelet.                              //    */
/*              // -------------------------------------------------- //    */
/*              IMG_wave_vert(in_data, lpf, hpf, out_ldata, out_hdata, x_dim); */
/*          }                                                               */
/*                                                                          */
/*  BIBLIOGRAPHY                                                            */
/*      A Wavelet Tour of Signal Processing Stephane Mallat                 */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

 static const char Copyright[] = "Copyright (C) 1999 Texas Instruments "

#include "IMG_wave_vert.h"

void IMG_wave_vert
(
    const short *restrict
    const       *restrict in_data,   /* Array of row pointers */
    const short *restrict lp_filt,   /* Low pass QMF filter   */
    const short *restrict hp_filt,   /* High pass QMF filter  */
    short       *restrict out_ldata, /* Low pass output data  */
    short       *restrict out_hdata, /* High pass output data */
    int cols                     /* Length of rows to process */
)
{
    int   i, j;

    /* -------------------------------------------------------------------- */
    /*  First, perform the low-pass filter on the eight input rows.         */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < cols; i++)
    {
        int sum = 1 << 14;

        for (j = 0; j < 8; j++)
            sum += in_data[j][i] * lp_filt[j];

        out_ldata[i] = sum >> 15;
    }

    /* -------------------------------------------------------------------- */
    /*  Next, perform the high-pass filter on the same eight input rows.    */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < cols; i++)
    {
        int sum = 1 << 14;

        for (j = 0; j < 8; j++)
            sum += in_data[j][i] * hp_filt[7 - j];

        out_hdata[i] = sum >> 15;
    }
}
/* ======================================================================== */
/*  End of file:  img_wave_vert.c                                           */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.4     Sat Mar 16 02:47:50 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_yc_demux_be16 -- De-interleave a 4:2:2 BIG ENDIAN video stream  */
/*                       into three separate LITTLE ENDIAN 16-bit planes    */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      28-Sep-2001                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This function is C callable, and is called as follows:              */
/*                                                                          */
/*      void IMG_yc_demux_be16                                              */
/*      (                                                                   */
/*          int n,                       // Number of luma pixels    //     */
/*          const unsigned char * yc,    // Interleaved luma/chroma  //     */
/*          short *restrict y,           // Luma plane (16-bit)      //     */
/*          short *restrict cr,          // Cr chroma plane (16-bit) //     */
/*          short *restrict cb           // Cb chroma plane (16-bit) //     */
/*      );                                                                  */
/*                                                                          */
/*      The input array 'yc' is expected to be an interleaved 4:2:2         */
/*      video stream.  The input is expected in BIG ENDIAN byte order       */
/*      within each 4-byte word.  This is consistent with reading the       */
/*      video stream from a word-oriented BIG ENDIAN device while the       */
/*      C6000 device is in a LITTLE ENDIAN configuration.                   */
/*                                                                          */
/*      In other words, the expected pixel order is:                        */
/*                                                                          */
/*                  Word 0           Word 1          Word 2                 */
/*             +---------------+---------------+---------------+--          */
/*       Byte# | 0   1   2   3 | 4   5   6   7 | 8   9  10  11 |...         */
/*             |cb0 y1  cr0 y0 |cb2 y3  cr2 y2 |cb4 y5  cr4 y4 |...         */
/*             +---------------+---------------+---------------+--          */
/*                                                                          */
/*      The output arrays 'y', 'cr', and 'cb' are expected to not           */
/*      overlap.  The de-interleaved pixels are written as half-words       */
/*      in LITTLE ENDIAN order.                                             */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      This function reads the byte-oriented pixel data, zero-extends      */
/*      it, and then writes it to the appropriate result array.  Both       */
/*      the luma and chroma values are expected to be unsigned.             */
/*                                                                          */
/*      The data is expected to be in an order consistent with reading      */
/*      byte oriented data from a word-oriented peripheral that is          */
/*      operating in BIG ENDIAN mode, while the CPU is in LITTLE ENDIAN     */
/*      mode.  This results in a pixel ordering which is not                */
/*      immediately obvious.  This function correctly reorders the          */
/*      pixel values so that further processing may proceed in LITTLE       */
/*      ENDIAN mode.                                                        */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

#include "IMG_yc_demux_be16.h"

void IMG_yc_demux_be16
(
    int n,                    /* Number of luma pixels    */
    const unsigned char *yc,  /* Interleaved luma/chroma  */
    short *restrict y,        /* Luma plane (16-bit)      */
    short *restrict cr,       /* Cr chroma plane (16-bit) */
    short *restrict cb        /* Cb chroma plane (16-bit) */
)
{
    int i;

    for (i = 0; i < (n >> 1); i++)
    {
        /*  0   1   2   3  */
        /* cb0 y1  cr0  y0 */

        y[2*i+0] = yc[4*i + 3];
        y[2*i+1] = yc[4*i + 1];
        cr[i]    = yc[4*i + 2];
        cb[i]    = yc[4*i + 0];
    }
}

/* ======================================================================== */
/*  End of file:  img_yc_demux_be16.c                                       */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.2     Sat Mar 16 03:05:50 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_yc_demux_le16 -- De-interleave a 4:2:2 LITTLE ENDIAN video stream */
/*                       into three separate LITTLE ENDIAN 16-bit planes    */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      28-Sep-2001                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This function is C callable, and is called as follows:              */
/*                                                                          */
/*      void IMG_yc_demux_le16                                              */
/*      (                                                                   */
/*          int n,                       // Number of luma pixels    //     */
/*          const unsigned char * yc,    // Interleaved luma/chroma  //     */
/*          short *restrict y,           // Luma plane (16-bit)      //     */
/*          short *restrict cr,          // Cr chroma plane (16-bit) //     */
/*          short *restrict cb           // Cb chroma plane (16-bit) //     */
/*      );                                                                  */
/*                                                                          */
/*      The input array 'yc' is expected to be an interleaved 4:2:2         */
/*      video stream.  The input is expected in LITTLE ENDIAN byte          */
/*      order within each 4-byte word.  This is consistent with reading     */
/*      the video stream from a word-oriented LITTLE ENDIAN device          */
/*      while the C6000 device is in a LITTLE ENDIAN configuration.         */
/*                                                                          */
/*      In other words, the expected pixel order is:                        */
/*                                                                          */
/*                  Word 0           Word 1          Word 2                 */
/*             +---------------+---------------+---------------+--          */
/*       Byte# | 0   1   2   3 | 4   5   6   7 | 8   9  10  11 |...         */
/*             | y0 cr0 y1 cb0 | y2 cr2 y3 cb2 | y4 cr4 y5 cb4 |...         */
/*             +---------------+---------------+---------------+--          */
/*                                                                          */
/*      The output arrays 'y', 'cr', and 'cb' are expected to not           */
/*      overlap.  The de-interleaved pixels are written as half-words       */
/*      in LITTLE ENDIAN order.                                             */
/*                                                                          */
/*      Please see the IMGLIB function IMB_yc_demux_be16 for code which     */
/*      handles input coming from a BIG ENDIAN device.                      */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      This function reads the byte-oriented pixel data, zero-extends      */
/*      it, and then writes it to the appropriate result array.  Both       */
/*      the luma and chroma values are expected to be unsigned.             */
/*                                                                          */
/*      The data is expected to be in an order consistent with reading      */
/*      byte oriented data from a word-oriented peripheral that is          */
/*      operating in LITTLE ENDIAN mode, while the CPU is in LITTLE         */
/*      ENDIAN mode.  This function unpacks the byte-oriented data          */
/*      so that further processing may proceed in LITTLE ENDIAN mode.       */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

#include "IMG_yc_demux_le16.h"

void IMG_yc_demux_le16
(
    int n,                    /* Number of luma pixels    */
    const unsigned char *yc,  /* Interleaved luma/chroma  */
    short *restrict y,        /* Luma plane (16-bit)      */
    short *restrict cr,       /* Cr chroma plane (16-bit) */
    short *restrict cb        /* Cb chroma plane (16-bit) */
)
{
    int i;

    for (i = 0; i < (n >> 1); i++)
    {
        /*  0   1   2   3  */
        /*  y0 cr0 y1 cb0  */

        y[2*i+0] = yc[4*i + 0];
        y[2*i+1] = yc[4*i + 2];
        cr[i]    = yc[4*i + 1];
        cb[i]    = yc[4*i + 3];
    }
}

/* ======================================================================== */
/*  End of file:  img_yc_demux_le16.c                                       */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.7     Thu May 23 23:35:00 2002 (UTC)              */
/*      Snapshot date:  28-Oct-2002                                         */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2002 Texas Instruments, Incorporated.             */
/*                          All Rights Reserved.                            */
/* ======================================================================== */


/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_ycbcr422p_rgb565 -- Planarized YCbCr 4:2:2/4:2:0 to 16-bit      */
/*                              RGB 5:6:5 color space conversion.           */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      26-Aug-2001                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This function is C callable, and is called according to this        */
/*      C prototype:                                                        */
/*                                                                          */
/*      void IMG_ycbcr422p_rgb565                                           */
/*      (                                                                   */
/*        const short         coeff[5],  // Matrix coefficients.        //  */
/*        const unsigned char *y_data,   // Luminence data  (Y')        //  */
/*        const unsigned char *cb_data,  // Blue color-diff (B'-Y')     //  */
/*        const unsigned char *cr_data,  // Red color-diff  (R'-Y')     //  */
/*        unsigned short                                                    */
/*                   *restrict rgb_data, // RGB 5:6:5 packed pixel out. //  */
/*        unsigned            num_pixels // # of luma pixels to process //  */
/*      );                                                                  */
/*                                                                          */
/*      The 'coeff[]' array contains the color-space-conversion matrix      */
/*      coefficients.  The 'y_data', 'cb_data' and 'cr_data' pointers       */
/*      point to the separate input image planes.  The 'rgb_data' pointer   */
/*      points to the output image buffer.                                  */
/*                                                                          */
/*      The kernel is designed to process arbitrary amounts of 4:2:2        */
/*      image data, although 4:2:0 image data may be processed as well.     */
/*      For 4:2:2 input data, the 'y_data', 'cb_data' and 'cr_data'         */
/*      arrays may hold an arbitrary amount of image data, including        */
/*      multiple scan lines of image data.                                  */
/*                                                                          */
/*      For 4:2:0 input data, only a single scan-line (or portion           */
/*      thereof) may be processed at a time.  This is achieved by           */
/*      calling the function twice using the same row data for              */
/*      'cr_data' and 'cb_data', and providing new row data for             */
/*      'y_data'.  This is numerically equivalent to replicating the Cr     */
/*      and Cb pixels vertically.                                           */
/*                                                                          */
/*      The coefficients in the coeff array must be in signed Q13 form.     */
/*      These coefficients correspond to the following matrix equation:     */
/*                                                                          */
/*          [ coeff[0] 0.0000   coeff[1] ]   [ Y' -  16 ]     [ R']         */
/*          [ coeff[0] coeff[2] coeff[3] ] * [ Cb - 128 ]  =  [ G']         */
/*          [ coeff[0] coeff[4] 0.0000   ]   [ Cr - 128 ]     [ B']         */
/*                                                                          */
/*      The output from this kernel is 16-bit RGB in 5:6:5 format.          */
/*      The RGB components are packed into halfwords as shown below.        */
/*                                                                          */
/*                     15      11 10       5 4        0                     */
/*                    +----------+----------+----------+                    */
/*                    |   Red    |  Green   |   Blue   |                    */
/*                    +----------+----------+----------+                    */
/*                                                                          */
/*      This kernel can also return the red, green, and blue values in      */
/*      the opposite order if a particular application requires it.         */
/*      This is achieved by exchanging the 'cb_data' and 'cr_data'          */
/*      arguments when calling the function, and by reversing the order     */
/*      of coefficients in coeff[1] through coeff[4].  This essentially     */
/*      implements the following matrix multiply:                           */
/*                                                                          */
/*          [ coeff[0] 0.0000   coeff[4] ]   [ Y' -  16 ]     [ B']         */
/*          [ coeff[0] coeff[3] coeff[2] ] * [ Cr - 128 ]  =  [ G']         */
/*          [ coeff[0] coeff[1] 0.0000   ]   [ Cb - 128 ]     [ R']         */
/*                                                                          */
/*      The reversed RGB ordering output by this mode is as follows:        */
/*                                                                          */
/*                     15      11 10       5 4        0                     */
/*                    +----------+----------+----------+                    */
/*                    |   Blue   |  Green   |   Red    |                    */
/*                    +----------+----------+----------+                    */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      This kernel performs Y'CbCr to RGB conversion.  From the Color      */
/*      FAQ, http://home.inforamp.net/~poynton/ColorFAQ.html :              */
/*                                                                          */
/*          Various scale factors are applied to (B'-Y') and (R'-Y')        */
/*          for different applications.  The Y'PbPr scale factors are       */
/*          optimized for component analog video.  The Y'CbCr scaling       */
/*          is appropriate for component digital video, JPEG and MPEG.      */
/*          Kodak's PhotoYCC(tm) uses scale factors optimized for the       */
/*          gamut of film colors.  Y'UV scaling is appropriate as an        */
/*          intermediate step in the formation of composite NTSC or PAL     */
/*          video signals, but is not appropriate when the components       */
/*          are keps separate.  Y'UV nomenclature is now used rather        */
/*          loosely, and it sometimes denotes any scaling of (B'-Y')        */
/*          and (R'-Y').  Y'IQ coding is obsolete.                          */
/*                                                                          */
/*      This code can perform various flavors of Y'CbCr to RGB              */
/*      conversion as long as the offsets on Y, Cb, and Cr are -16,         */
/*      -128, and -128, respectively, and the coefficients match the        */
/*      pattern shown.                                                      */
/*                                                                          */
/*      The kernel implements the following matrix form, which involves 5   */
/*      unique coefficients:                                                */
/*                                                                          */
/*          [ coeff[0] 0.0000   coeff[1] ]   [ Y' -  16 ]     [ R']         */
/*          [ coeff[0] coeff[2] coeff[3] ] * [ Cb - 128 ]  =  [ G']         */
/*          [ coeff[0] coeff[4] 0.0000   ]   [ Cr - 128 ]     [ B']         */
/*                                                                          */
/*                                                                          */
/*      Below are some common coefficient sets, along with the matrix       */
/*      equation that they correspond to.   Coefficients are in signed      */
/*      Q13 notation, which gives a suitable balance between precision      */
/*      and range.                                                          */
/*                                                                          */
/*      1.  Y'CbCr -> RGB conversion with RGB levels that correspond to     */
/*          the 219-level range of Y'.  Expected ranges are [16..235] for   */
/*          Y' and [16..240] for Cb and Cr.                                 */
/*                                                                          */
/*          coeff[] = { 0x2000, 0x2BDD, -0x0AC5, -0x1658, 0x3770 };         */
/*                                                                          */
/*          [ 1.0000    0.0000    1.3707 ]   [ Y' -  16 ]     [ R']         */
/*          [ 1.0000   -0.3365   -0.6982 ] * [ Cb - 128 ]  =  [ G']         */
/*          [ 1.0000    1.7324    0.0000 ]   [ Cr - 128 ]     [ B']         */
/*                                                                          */
/*      2.  Y'CbCr -> RGB conversion with the 219-level range of Y'         */
/*          expanded to fill the full RGB dynamic range.  (The matrix       */
/*          has been scaled by 255/219.)  Expected ranges are [16..235]     */
/*          for Y' and [16..240] for Cb and Cr.                             */
/*                                                                          */
/*          coeff[] = { 0x2543, 0x3313, -0x0C8A, -0x1A04, 0x408D };         */
/*                                                                          */
/*          [ 1.1644    0.0000    1.5960 ]   [ Y' -  16 ]     [ R']         */
/*          [ 1.1644   -0.3918   -0.8130 ] * [ Cb - 128 ]  =  [ G']         */
/*          [ 1.1644    2.0172    0.0000 ]   [ Cr - 128 ]     [ B']         */
/*                                                                          */
/*      3.  Y'CbCr -> BGR conversion with RGB levels that correspond to     */
/*          the 219-level range of Y'.  This is equivalent to #1 above,     */
/*          except that the R, G, and B output order in the packed          */
/*          pixels is reversed.  Note:  The 'cr_data' and 'cb_data'         */
/*          input arguments must be exchanged for this example as           */
/*          indicated under USAGE above.                                    */
/*                                                                          */
/*          coeff[] = { 0x2000, 0x3770, -0x1658, -0x0AC5, 0x2BDD };         */
/*                                                                          */
/*          [ 1.0000    0.0000    1.7324 ]   [ Y' -  16 ]     [ B']         */
/*          [ 1.0000   -0.6982   -0.3365 ] * [ Cr - 128 ]  =  [ G']         */
/*          [ 1.0000    1.3707    0.0000 ]   [ Cb - 128 ]     [ R']         */
/*                                                                          */
/*      4.  Y'CbCr -> BGR conversion with the 219-level range of Y'         */
/*          expanded to fill the full RGB dynamic range.  This is           */
/*          equivalent to #2 above, except that the R, G, and B output      */
/*          order in the packed pixels is reversed.  Note:  The             */
/*          'cr_data' and 'cb_data' input arguments must be exchanged       */
/*          for this example as indicated under USAGE above.                */
/*                                                                          */
/*          coeff[] = { 0x2000, 0x408D, -0x1A04, -0x0C8A, 0x3313 };         */
/*                                                                          */
/*          [ 1.0000    0.0000    2.0172 ]   [ Y' -  16 ]     [ B']         */
/*          [ 1.0000   -0.8130   -0.3918 ] * [ Cr - 128 ]  =  [ G']         */
/*          [ 1.0000    1.5960    0.0000 ]   [ Cb - 128 ]     [ R']         */
/*                                                                          */
/*      Other scalings of the color differences (B'-Y') and (R'-Y')         */
/*      (sometimes incorrectly referred to as U and V) are supported, as    */
/*      long as the color differences are unsigned values centered around   */
/*      128 rather than signed values centered around 0, as noted above.    */
/*                                                                          */
/*      In addition to performing plain color-space conversion, color       */
/*      saturation can be adjusted by scaling coeff[1] through coeff[4].    */
/*      Similarly, brightness can be adjusted by scaling coeff[0].          */
/*      General hue adjustment can not be performed, however, due to the    */
/*      two zeros hard-coded in the matrix.                                 */
/*                                                                          */
/*  TECHNIQUES                                                              */
/*      Pixel replication is performed implicitly on chroma data to         */
/*      reduce the total number of multiplies required.  The chroma         */
/*      portion of the matrix is calculated once for each Cb, Cr pair,      */
/*      and the result is added to both Y' samples.                         */
/*                                                                          */
/*  SOURCE                                                                  */
/*      Poynton, Charles et al.  "The Color FAQ,"  1999.                    */
/*          http://home.inforamp.net/~poynton/ColorFAQ.html                 */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

void IMG_ycbcr422p_rgb565
(
    const short             coeff[5],   /* Matrix coefficients.             */
    const unsigned char     *y_data,    /* Luminence data        (Y')       */
    const unsigned char     *cb_data,   /* Blue color-difference (B'-Y')    */
    const unsigned char     *cr_data,   /* Red color-difference  (R'-Y')    */
    unsigned short *restrict rgb_data,  /* RGB 5:6:5 packed pixel output.   */
    unsigned                num_pixels  /* # of luma pixels to process.     */
)
{
    int     i;                      /* Loop counter                     */
    int     y0, y1;                 /* Individual Y components          */
    int     cb, cr;                 /* Color difference components      */
    int     y0t,y1t;                /* Temporary Y values               */
    int     rt, gt, bt;             /* Temporary RGB values             */
    int     r0, g0, b0;             /* Individual RGB components        */
    int     r1, g1, b1;             /* Individual RGB components        */
    int     r0t,g0t,b0t;            /* Truncated RGB components         */
    int     r1t,g1t,b1t;            /* Truncated RGB components         */
    int     r0s,g0s,b0s;            /* Saturated RGB components         */
    int     r1s,g1s,b1s;            /* Saturated RGB components         */

    short   luma = coeff[0];        /* Luma scaling coefficient.        */
    short   r_cr = coeff[1];        /* Cr's contribution to Red.        */
    short   g_cb = coeff[2];        /* Cb's contribution to Green.      */
    short   g_cr = coeff[3];        /* Cr's contribution to Green.      */
    short   b_cb = coeff[4];        /* Cb's contribution to Blue.       */

    unsigned short  rgb0, rgb1;     /* Packed RGB pixel data            */

    /* -------------------------------------------------------------------- */
    /*  Iterate for num_pixels/2 iters, since we process pixels in pairs.   */
    /* -------------------------------------------------------------------- */
    i = num_pixels >> 1;

    while (i-->0)
    {
        /* ---------------------------------------------------------------- */
        /*  Read in YCbCr data from the separate data planes.               */
        /*                                                                  */
        /*  The Cb and Cr channels come in biased upwards by 128, so        */
        /*  subtract the bias here before performing the multiplies for     */
        /*  the color space conversion itself.  Also handle Y's upward      */
        /*  bias of 16 here.                                                */
        /* ---------------------------------------------------------------- */

        y0 = *y_data++  - 16;
        y1 = *y_data++  - 16;
        cb = *cb_data++ - 128;
        cr = *cr_data++ - 128;

        /* ================================================================ */
        /*  Convert YCrCb data to RGB format using the following matrix:    */
        /*                                                                  */
        /*      [ coeff[0] 0.0000   coeff[1] ]   [ Y' -  16 ]     [ R']     */
        /*      [ coeff[0] coeff[2] coeff[3] ] * [ Cb - 128 ]  =  [ G']     */
        /*      [ coeff[0] coeff[4] 0.0000   ]   [ Cr - 128 ]     [ B']     */
        /*                                                                  */
        /*  We use signed Q13 coefficients for the coefficients to make     */
        /*  good use of our 16-bit multiplier.  Although a larger Q-point   */
        /*  may be used with unsigned coefficients, signed coefficients     */
        /*  add a bit of flexibility to the kernel without significant      */
        /*  loss of precision.                                              */
        /* ================================================================ */

        /* ---------------------------------------------------------------- */
        /*  Calculate chroma channel's contribution to RGB.                 */
        /* ---------------------------------------------------------------- */
        rt  = r_cr * (short)cr;
        gt  = g_cb * (short)cb + g_cr * (short)cr;
        bt  = b_cb * (short)cb;

        /* ---------------------------------------------------------------- */
        /*  Calculate intermediate luma values.  Include bias of 16 here.   */
        /* ---------------------------------------------------------------- */
        y0t = luma * (short)y0;
        y1t = luma * (short)y1;

        /* ---------------------------------------------------------------- */
        /*  Mix luma, chroma channels.                                      */
        /* ---------------------------------------------------------------- */
        r0  = y0t + rt; r1 = y1t + rt;
        g0  = y0t + gt; g1 = y1t + gt;
        b0  = y0t + bt; b1 = y1t + bt;

        /* ================================================================ */
        /*  At this point in the calculation, the RGB components are        */
        /*  nominally in the format below.  If the color is outside the     */
        /*  our RGB gamut, some of the sign bits may be non-zero,           */
        /*  triggering saturation.                                          */
        /*                                                                  */
        /*                  3     2 2        1 1                            */
        /*                  1     1 0        3 2         0                  */
        /*                 [ SIGN  | COLOR    | FRACTION ]                  */
        /*                                                                  */
        /*  This gives us an 8-bit range for each of the R, G, and B        */
        /*  components.  (The transform matrix is designed to transform     */
        /*  8-bit Y/C values into 8-bit R,G,B values.)  To get our final    */
        /*  5:6:5 result, we "divide" our R, G and B components by 4, 8,    */
        /*  and 4, respectively, by reinterpreting the numbers in the       */
        /*  format below:                                                   */
        /*                                                                  */
        /*          Red,    3     2 2     1 1                               */
        /*          Blue    1     1 0     6 5            0                  */
        /*                 [ SIGN  | COLOR | FRACTION    ]                  */
        /*                                                                  */
        /*                  3     2 2      1 1                              */
        /*          Green   1     1 0      5 4           0                  */
        /*                 [ SIGN  | COLOR  | FRACTION   ]                  */
        /*                                                                  */
        /*  "Divide" is in quotation marks because this step requires no    */
        /*  actual work.  The code merely treats the numbers as having a    */
        /*  different Q-point.                                              */
        /* ================================================================ */

        /* ---------------------------------------------------------------- */
        /*  Shift away the fractional portion, and then saturate to the     */
        /*  RGB 5:6:5 gamut.                                                */
        /* ---------------------------------------------------------------- */
        r0t = r0 >> 16;
        g0t = g0 >> 15;
        b0t = b0 >> 16;
        r1t = r1 >> 16;
        g1t = g1 >> 15;
        b1t = b1 >> 16;

        r0s = r0t < 0 ? 0 : r0t > 31 ? 31 : r0t;
        g0s = g0t < 0 ? 0 : g0t > 63 ? 63 : g0t;
        b0s = b0t < 0 ? 0 : b0t > 31 ? 31 : b0t;
        r1s = r1t < 0 ? 0 : r1t > 31 ? 31 : r1t;
        g1s = g1t < 0 ? 0 : g1t > 63 ? 63 : g1t;
        b1s = b1t < 0 ? 0 : b1t > 31 ? 31 : b1t;

        /* ---------------------------------------------------------------- */
        /*  Merge values into output pixels.                                */
        /* ---------------------------------------------------------------- */
        rgb0 = (r0s << 11) + (g0s <<  5) + (b0s <<  0);
        rgb1 = (r1s << 11) + (g1s <<  5) + (b1s <<  0);

        /* ---------------------------------------------------------------- */
        /*  Store resulting pixels to memory.                               */
        /* ---------------------------------------------------------------- */
        *rgb_data++ = rgb0;
        *rgb_data++ = rgb1;
    }

    return;
}

/* ======================================================================== */
/*  End of file:  img_ycbcr422p_rgb565.c                                    */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

