/*
 * Copyright (C) 2014 Amlogic Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _STREAM_FORMAT_H_
#define _STREAM_FORMAT_H_

#include "amports/vformat.h"
#include "amports/aformat.h"

typedef enum {
    NONE = 0,
    ID3V1,
    ID3V2,
    APEV1,
    APEV2,
    WMATAG,
    MPEG4TAG,
} audio_tag_type;

typedef enum {
    UNKNOWN_FILE    = 0,
    AVI_FILE        = 1,
    MPEG_FILE       = 2,
    WAV_FILE        = 3,
    MP3_FILE        = 4,
    AAC_FILE        = 5,
    AC3_FILE        = 6,
    RM_FILE         = 7,
    DTS_FILE        = 8,
    MKV_FILE        = 9,
    MOV_FILE        = 10,
    MP4_FILE        = 11,
    FLAC_FILE       = 12,
    H264_FILE       = 13,
    M2V_FILE        = 14,
    FLV_FILE        = 15,
    P2P_FILE        = 16,
    ASF_FILE        = 17,
    STREAM_FILE     = 18,
    APE_FILE        = 19,
    AMR_FILE        = 20,
    AVS_FILE        = 21,
    PMP_FILE        = 22,
    OGM_FILE        = 23,
    HEVC_FILE       = 24,
    DRA_FILE	    = 25,
    IVF_FILE        = 26,
    AIFF_FILE        = 27,
    FILE_MAX,
} pfile_type;

#endif
