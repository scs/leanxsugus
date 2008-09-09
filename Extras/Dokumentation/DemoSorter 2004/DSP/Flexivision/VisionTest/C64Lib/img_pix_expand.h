/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.8     Sun Sep 29 03:26:45 2002 (UTC)              */
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
/*      IMG_pix_expand                                                      */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      19-Jan-2001                                                         */
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
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The code takes an array of bytes and promotes them to half-words    */
/*      by zero-extension.                                                  */
/*                                                                          */
/*      This is the C equivalent of the assembly code, without              */
/*      restrictions.  The assembly code has restrictions, as noted below.  */
/*                                                                          */
/*      void IMG_pix_expand                                                 */
/*      (                                                                   */
/*          int n,                                                          */
/*          const unsigned char *restrict in_data,                          */
/*          short               *restrict out_data                          */
/*      )                                                                   */
/*      {                                                                   */
/*          int i;                                                          */
/*                                                                          */
/*          for (i = 0; i < n; i++)                                         */
/*              out_data[i] =  in_data[i];                                  */
/*      }                                                                   */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      Input and output arrays must be double-word (8-byte) aligned.       */
/*                                                                          */
/*      The input must be at least 16 elements long and contain a           */
/*      multiple of 16 elements.                                            */
/*                                                                          */
/*  NOTE                                                                    */
/*      Interrupts are masked during the entire duration of this            */
/*      function, as the entire function occurs within branch delay slots.  */
/*                                                                          */
/*  MEMORY NOTE                                                             */
/*      No bank conflicts occur.  This is a LITTLE ENDIAN implementation.   */
/*                                                                          */
/*  TECHNIQUES                                                              */
/*      The loop is unrolled 16 times, loading bytes with LDDW.  It uses    */
/*      UNPKHU4 and UNPKLU4 to unpack the data and store the results with   */
/*      STDW.                                                               */
/*                                                                          */
/*      To shave a few extra cycles from the function, the return branch    */
/*      is issued from within the kernel.                                   */
/*                                                                          */
/*  CYCLES                                                                  */
/*      cycles = 3 * n/16 + 15.                                             */
/*      For n = 1072, cycles = 216.                                         */
/*                                                                          */
/*  CODESIZE                                                                */
/*      108 bytes.                                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef IMG_PIX_EXPAND_H_
#define IMG_PIX_EXPAND_H_ 1

void IMG_pix_expand
(
    int n,                                    /* # of elements */
    const unsigned char *restrict in_data,    /* Input data    */
    short               *restrict out_data    /* Output data   */
);

#endif
/* ======================================================================== */
/*  End of file:  img_pix_expand.h                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
