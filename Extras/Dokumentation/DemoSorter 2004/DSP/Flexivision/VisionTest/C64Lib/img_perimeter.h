/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.7     Sun Sep 29 07:01:22 2002 (UTC)              */
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
/*==========================================================================S*/
/*                                                                           */
/*   TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                           */
/*   NAME                                                                    */
/*       Perimeter: Detection of the boundary of a binary image              */
/*                                                                           */
/*   REVISION DATE                                                           */
/*       21-Aug-2001                                                         */
/*                                                                           */
/*   USAGE                                                                   */
/*       This routine is C callable, and has the following C prototype:      */
/*                                                                           */
/*          int IMG_perimeter                                               */
/*          (                                                               */
/*              const unsigned char *restrict in,  // Input image    //     */
/*              int cols,                          // Width of input //     */
/*              unsigned char       *restrict out  // Output image   //     */
/*          );                                                              */
/*                                                                           */
/*    DESCRIPTION                                                            */
/*          This routine produces the boundary of a binary image. It echoes  */
/*    the boundary pixels with a value of 0xFF and sets the other pixels     */
/*    as 0. Detection of the boundary of a binary image is a segmentation    */
/*    problem and is done by examining spatial locality of the neighboring   */
/*    pixels. This is done by using the four connectivity algorithm          */
/*                                                                           */
/*              pix_up                                                       */
/*     pix_lft pix_cent pix_rgt                                              */
/*             pix_dn                                                        */
/*                                                                           */
/*    The output pixel at location pix_cent is echoed as a boundary pixel    */
/*    if pix_cent is non-zero and any one of its four neighbors is zero      */
/*    The four neighbors are shown and stand for the foll:                   */
/*                                                                           */
/*    pix_up:  top pixel                                                     */
/*    pix_lft: left pixel                                                    */
/*    pix_rgt: right pixel                                                   */
/*    pix_dn:  bottom pixel                                                  */
/*                                                                           */
/*    ASSUMPTIONS                                                            */
/*      input image must be double-word aligned                              */
/*      cols must be a multiple of 16                                        */
/*                                                                           */
/*    MEMORY NOTE                                                            */
/*       No bank conflicts are expected for this kernel.                     */
/*                                                                           */
/*    TECHNIQUES                                                             */
/*                                                                           */
/*    Use double word wide loads and bring in pixels along three lines which */
/*    we shall call top, mid and bot. Use split compares to compare if pix-  */
/*    els are greater than or equal to zero. Use the 4 lsb's to find out     */
/*    the result. Prepare an 8 bit mask using the result of 2 such split     */
/*    compares. Perform this operation for the top, middle and botton.       */
/*    Logically invert the result of mid, left shift and right shift and     */
/*    add the context information by setting the 8 th bit or the 1st bit.    */
/*    Use xpnd4 and bitc4 to perform expansion and bit count. Store output   */
/*    pixels as double word. The actual handassembly code is unrolled once   */
/*    and computes 16 output pixels in 10 cycles                             */
/*                                                                           */
/*   CYCLES                                                                  */
/*       10 * cols/16 + 55                                                   */
/*       cols = 720,    505 cycles                                           */
/*                                                                           */
/*   CodeSize: 600 bytes                                                     */
/*                                                                           */
/*==========================================================================S*/
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/*==========================================================================S*/
#ifndef IMG_PERIMETER_H_
#define IMG_PERIMETER_H_ 1

int IMG_perimeter
(
    const unsigned char *restrict in,  /* Input image    */
    int cols,                          /* Width of input */
    unsigned char       *restrict out  /* Output image   */
);

#endif
/* ======================================================================== */
/*  End of file:  img_perimeter.h                                           */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
