/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.4     Sun Sep 29 03:26:45 2002 (UTC)              */
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
/*  Assembler compatibility shim for assembling 4.30 and later code on      */
/*  tools prior to 4.30.                                                    */
/* ======================================================================== */
/* ======================================================================== */
/*  End of assembler compatibility shim.                                    */
/* ======================================================================== */
/* ======================================================================== */
/*  NAME                                                                    */
/*      IMG_sad_8x8 -- Sum of Absolute Differences on single 16x16 block    */
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
/*  C CODE                                                                  */
/*      The following is a C code description of the algorithm that lacks   */
/*      restrictions.  The assembly code may have additional restrictions   */
/*      as noted below.                                                     */
/*                                                                          */
/*          unsigned IMG_sad_8x8                                            */
/*          (                                                               */
/*              const unsigned char *restrict srcImg,                       */
/*              const unsigned char *restrict refImg,                       */
/*              int pitch                                                   */
/*          )                                                               */
/*          {                                                               */
/*              int i, j;                                                   */
/*              unsigned sad = 0;                                           */
/*                                                                          */
/*              for (i = 0; i < 8; i++)                                     */
/*                  for (j = 0; j < 8; j++)                                 */
/*                      sad += abs(srcImg[j+i*8] - refImg[j+i*pitch]);      */
/*                                                                          */
/*              return sad;                                                 */
/*          }                                                               */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      Some versions of this kernel may assume that srcImg is double-      */
/*      word aligned.                                                       */
/*                                                                          */
/*  MEMORY NOTE                                                             */
/*      No bank conflicts occur.                                            */
/*      Endian Neutral.                                                     */
/*                                                                          */
/*  NOTES                                                                   */
/*      This kernel blocks interrupts for 25 cycles.                        */
/*                                                                          */
/*  CYCLES                                                                  */
/*      31 cycles                                                           */
/*                                                                          */
/*  CODESIZE                                                                */
/*      164 bytes                                                           */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef IMG_SAD_8X8_H_
#define IMG_SAD_8X8_H_ 1

unsigned IMG_sad_8x8
(
    const unsigned char *restrict srcImg,  /* 8x8 source block   */
    const unsigned char *restrict refImg,  /* Reference image    */
    int pitch                              /* Width of ref image */
);

#endif
/* ======================================================================== */
/*  End of file:  img_sad_8x8.h                                             */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
