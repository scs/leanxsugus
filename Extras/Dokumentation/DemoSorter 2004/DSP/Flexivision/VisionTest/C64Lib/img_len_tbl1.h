/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  IMGLIB  DSP Image/Video Processing Library                              */
/*                                                                          */
/*      Release:        Version 1.03                                        */
/*      CVS Revision:   1.2     Fri Oct 25 00:29:51 2002 (UTC)              */
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
/*      IMG_len_tbl1                                                        */
/*                                                                          */
/*  REVISION DATE                                                           */
/*      10-Apr-2001                                                         */
/*                                                                          */
/*  USAGE                                                                   */
/*      extern far const unsigned char IMG_len_tbl1[640];                   */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      MPEG-2 VLD look-up table for VLC code length decoding (Table B-15)  */
/*      To be used with mpeg2_vld_intra and mpeg2_vld_inter routines.       */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef IMG_LEN_TBL1_H_
#define IMG_LEN_TBL1_H_ 1

extern far const unsigned char IMG_len_tbl1[640];

#endif
/* ======================================================================== */
/*  End of file:  img_len_tbl1.h                                            */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2002 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
