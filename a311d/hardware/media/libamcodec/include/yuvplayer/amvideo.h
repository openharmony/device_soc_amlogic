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

#ifndef AMVDEC_AMVIDEO_HEADER_SS
#define  AMVDEC_AMVIDEO_HEADER_SS
#include <stdlib.h>
//#include <linux/videodev2.h>
#include "videodev2.h"
#define FLAGS_OVERLAY_MODE (1)
#define FLAGS_V4L_MODE 	 (2)

struct amvideo_dev;
typedef struct vframebuf{
	char * vbuf;
	int fd;
	int index;
	int offset;
	int length;
	int64_t pts;
	int duration;
    int width;
    int height;
}vframebuf_t;

struct amvideo_dev_ops{
	int (*setparameters)(struct amvideo_dev *dev,int cmd,void*para);
	int (*init)(struct amvideo_dev *dev,int flags,int width,int height,int fmt,int buffernum);
	int (*release)(struct amvideo_dev *dev);
	int (*dequeuebuf)(struct amvideo_dev *dev,vframebuf_t*vf);
	int (*queuebuf)(struct amvideo_dev *dev,vframebuf_t*vf);
	int (*start)(struct amvideo_dev *dev);
	int (*stop)(struct amvideo_dev *dev);
};

typedef struct amvideo_dev{
	char devname[8];
	int mode;
    int display_mode;
	struct amvideo_dev_ops ops;
	void *devpriv;
}amvideo_dev_t;


typedef struct amvideo{
	amvideo_dev_t *dev;
}amvideo_t;


amvideo_dev_t *new_amvideo(int flags);
int amvideo_setparameters(amvideo_dev_t *dev, int cmd, void * parameters);
int amvideo_init(amvideo_dev_t *dev,int flags,int width,int height,int fmt,int buffernum);
int amvideo_start(amvideo_dev_t *dev);
int amvideo_stop(amvideo_dev_t *dev);
int amvideo_release(amvideo_dev_t *dev);
int amlv4l_dequeuebuf(amvideo_dev_t *dev, vframebuf_t*vf);
int amlv4l_queuebuf(amvideo_dev_t *dev, vframebuf_t*vf);
#endif
