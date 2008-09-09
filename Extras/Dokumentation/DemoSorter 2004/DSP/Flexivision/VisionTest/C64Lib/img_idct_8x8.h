/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.3     Fri Oct 25 00:22:37 2002 (UTC)              */
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
/*      IMG_idct_8x8 -- Wrapper for idct_8x8_12q4, Little Endian.           */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      24-Oct-2002                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C callable, and has the following C prototype:      */
/*                                                                          */
/*          void IMG_idct_8x8(short idct_data[], unsigned num_idcts)        */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      This function provides a wrapper around the IMG_idct_8x8_12q4       */
/*      function.  It reads 12Q0 values in from the input array, and        */
/*      writes them back as 12Q4.  Once all the values have been adjusted,  */
/*      the function then branches directly to IMG_idct_8x8_12q4.           */
/*                                                                          */
/*      It is strongly recommended that you adapt your code to call         */
/*      IMG_idct_8x8_12q4 directly.  Calling this wrapper adds an extra     */
/*      16 cycles per block of processing overhead, as well as 116          */
/*      bytes of additional code.                                           */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      This is a LITTLE ENDIAN implementation.                             */
/*      The input array must be aligned on a double-word boundary.          */
/*                                                                          */
/*  MEMORY NOTE                                                             */
/*      No bank conflicts occur.                                            */
/*                                                                          */
/*  NOTES                                                                   */
/*      This code locks out interrupts for its entire duration.             */
/*      It is, however, fully interrupt tolerant.                           */
/*                                                                          */
/*  CYCLES                                                                  */
/*      cycles = 16 + 16 * num_idcts, for num_idcts > 0                     */
/*                                                                          */
/*  CODESIZE                                                                */
/*      116 bytes                                                           */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef IMG_IDCT_8X8_H_
#define IMG_IDCT_8X8_H_ 1

void IMG_idct_8x8(short idct_data[], unsigned num_idcts);

#endif
/* ======================================================================== */
/*  End of file:  img_idct_8x8.h                                            */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
