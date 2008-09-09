/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.5     Sun Sep 29 03:26:45 2002 (UTC)              */
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
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  NAME                                                                    */
/*      IMG_pix_sat -- Pixel saturate and pack.                             */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      23-May-2000                                                         */
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
/*  DESCRIPTION                                                             */
/*      The function IMG_pix_sat() takes signed 16-bit input pixels and     */
/*      saturates them to unsigned 8-bit results.  Pixel values above       */
/*      255 are clamped to 255, and values below 0 are clamped to 0.        */
/*                                                                          */
/*  TECHNIQUES                                                              */
/*      The inner loop has been unrolled to fill a 6 cycle loop.  This      */
/*      allows the code to be interruptible.                                */
/*                                                                          */
/*      The prolog and epilog have been collapsed into the kernel.  Also,   */
/*      most of the setup and all of the exit code has been overlapped      */
/*      with the kernel.  The result is that the function runs with a       */
/*      very minimum amount of overhead.                                    */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      The input size must be a multiple of 32 pixels.  The code behaves   */
/*      correctly if a pixel count of zero is passed in.                    */
/*                                                                          */
/*  NOTES                                                                   */
/*      This code is fully interruptible.                                   */
/*                                                                          */
/*  MEMORY NOTE                                                             */
/*      The input and output data must be double-word aligned.              */
/*                                                                          */
/*      This code accesses 128 bits every cycle.  No bank conflicts occur.  */
/*                                                                          */
/*  CYCLES                                                                  */
/*      cycles = (pixels / 16) * 3 + 13                                     */
/*      (This includes 6 cycles of function call overhead.)                 */
/*      For pixels = 640, cycles = 133.                                     */
/*                                                                          */
/*  CODESIZE                                                                */
/*      116 bytes.                                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef IMG_PIX_SAT_H_
#define IMG_PIX_SAT_H_ 1

void IMG_pix_sat
(
    int n,                            /* Number of pixels */
    const short   *restrict in_data,  /* Incoming data    */
    unsigned char *restrict out_data  /* Outgoing data    */
);

#endif
/* ======================================================================== */
/*  End of file:  img_pix_sat.h                                             */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
