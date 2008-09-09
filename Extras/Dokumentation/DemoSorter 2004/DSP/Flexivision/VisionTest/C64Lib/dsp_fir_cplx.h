/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  DSPLIB  DSP Signal Processing Library                                   */
/*                                                                          */
/*      Release:        Revision 1.04a                                      */
/*      CVS Revision:   1.11    Sun Sep 29 03:32:21 2002 (UTC)              */
/*      Snapshot date:  10-Sep-2003                                         */
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
/*          Copyright (C) 2003 Texas Instruments, Incorporated.             */
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
/*      DSP_fir_cplx -- Hand-Coded Assembly code for                        */
/*      Complex Filter.                                                     */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      06-Aug-2001                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*      void DSP_fir_cplx                                                   */
/*      (                                                                   */
/*          const short *restrict x,                                        */
/*          const short *restrict h,                                        */
/*          short       *restrict r,                                        */
/*          short                 nh,                                       */
/*          short                 nr                                        */
/*     )                                                                    */
/*                                                                          */
/*     x[2*(nr+nh-1)] : Complex input data. x must point to x[2*(nh-1)].    */
/*     h[2*nh]        : Complex coefficients (in normal order).             */
/*     r[2*nr]        : Complex output data.                                */
/*     nh             : Number of complex coefficients.                     */
/*                      Must be multiple of 2.                              */
/*     nr             : Number of complex output samples.                   */
/*                      Must be multiple of 4.                              */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      This complex FIR computes nr complex output samples using nh        */
/*      complex coefficients. It operates on 16-bit data with a 32-bit      */
/*      accumulate. Each array consists of an even and odd term with even   */
/*      terms representing the real part of the element and the odd terms   */
/*      the imaginary part. The pointer to input array x must point to the  */
/*      (nh)th complex sample, i.e. element 2*(nh-1), upon entry to the     */
/*      function. The coefficients are expected in normal order.            */
/*                                                                          */
/*  C CODE                                                                  */
/*      void DSP_fir_cplx                                                   */
/*      (                                                                   */
/*          const short *restrict x,                                        */
/*          const short *restrict h,                                        */
/*          short *restrict r,                                              */
/*          short nh,                                                       */
/*          short nr                                                        */
/*      )                                                                   */
/*      {                                                                   */
/*          short i,j;                                                      */
/*          int imag, real;                                                 */
/*                                                                          */
/*          for (i = 0; i < 2*nr; i += 2)                                   */
/*          {                                                               */
/*              imag = 0;                                                   */
/*              real = 0;                                                   */
/*              for (j = 0; j < 2*nh; j += 2)                               */
/*              {                                                           */
/*                  real += h[j] * x[i-j]   - h[j+1] * x[i+1-j];            */
/*                  imag += h[j] * x[i+1-j] + h[j+1] * x[i-j];              */
/*              }                                                           */
/*              r[i]   = (real >> 15);                                      */
/*              r[i+1] = (imag >> 15);                                      */
/*          }                                                               */
/*      }                                                                   */
/*                                                                          */
/*  TECHNIQUES                                                              */
/*      Outer loop is unrolled 4 times while inner loop is not unrolled.    */
/*      Both inner and outer loops are collapsed into one loop.             */
/*      ADDAH and SUBAH are used alongwith PACKH2 to perform accumulation,  */
/*      shift and data packing.                                             */
/*      Collpsed one stage of epilog and prolog each.                       */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      nr must be a multiple of 4 and >= 4.                                */
/*      nh must be a multiple of 2 and >= 2.                                */
/*                                                                          */
/*  MEMORY NOTE                                                             */
/*      No memory bank hits under any conditions.                           */
/*                                                                          */
/*  NOTE                                                                    */
/*      This function is little Endian.                                     */
/*      This function is interrupt-tolerant but not interruptible.          */
/*                                                                          */
/*  CYCLES                                                                  */
/*      nh * nr + 24                                                        */
/*                                                                          */
/*      For nh = 24, nr = 40, 984 cycles                                    */
/*                                                                          */
/*  CODESIZE                                                                */
/*     432 bytes                                                            */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2003 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef DSP_FIR_CPLX_H_
#define DSP_FIR_CPLX_H_ 1

void DSP_fir_cplx
(
    const short *restrict x,
    const short *restrict h,
    short       *restrict r,
    short                 nh,
    short                 nr
       );

#endif
/* ======================================================================== */
/*  End of file:  dsp_fir_cplx.h                                            */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2003 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
