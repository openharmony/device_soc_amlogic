/*
 * Copyright (C) 2017 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

/**
* @file kplayer.c
* @brief
* @author Xu Hui <hui.xu@amlogic.com>
* @version 1.0.0
* @date 2011-02-28
*/
/*!< Copyright (C) 2007-2011, Amlogic Inc.
* All right reserved
*
*/
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "player.h"
//#include "player_type.h"
//#include "streamsource.h"
typedef enum {
    //EMU_STEP_NONE = 0,
    EMU_STEP_PAUSE = 2,
    EMU_STEP_RESUME = 3,
    EMU_STEP_START = 4,
    EMU_STEP_FF = 5,
    EMU_STEP_RR = 6,
    EMU_STEP_SEEK = 7,
    //EMU_STEP_MUTE= 8,
    //EMU_STEP_SETVOL=9,
    //EMU_STEP_GETVOL = 10,
    //EMU_STEP_SETTONE= 11,
    EMU_STEP_SETLOOP = 14,
    EMU_STEP_STOP = 16,
    //EMU_STEP_SPECTRUM = 17,
    //EMU_STEP_SETSUBT = 19,
    EMU_STEP_MENU = 20,
    EMU_STEP_EXIT = 21,
    //EMU_STEP_ATRACK = 22,
    EMU_STEP_GETAVMEDIAINFO = 25,
    //EMU_STEP_LISTALLMEDIAID = 27,
} EMU_STEP;
#define SCREEN_SPLITER            "***************************************************************************\r\n"

static int amsysfs_set_sysfs_int(const char *path, int val)
{
    int fd;
    int bytes;
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%d", val);
        bytes = write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    } else {
        printf("unable to open file %s,err: %s\n", path, strerror(errno));
    }
    return -1;
}

static int amsysfs_set_sysfs_str(const char *path, const char *val)
{
    int fd;
    int bytes;
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        bytes = write(fd, val, strlen(val));
        close(fd);
        return 0;
    } else {
        printf("unable to open file %s,err: %s\n", path, strerror(errno));
    }
    return -1;
}

static int  amsysfs_get_sysfs_str(const char *path, char *valstr, int size)
{
    int fd;
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        memset(valstr, 0, size);
        read(fd, valstr, size - 1);
        valstr[strlen(valstr)] = '\0';
        close(fd);
    } else {
        printf("unable to open file %s,err: %s\n", path, strerror(errno));
        sprintf(valstr, "%s", "fail");
        return -1;
    };
    //LOGI("get_sysfs_str=%s\n", valstr);
    return 0;
}


static int axis[8] = {0};

int update_player_info(int pid, player_info_t * info)
{
    printf("pid:%d,status:%x,current pos:%d,total:%d,errcode:%x\n", pid, info->status, info->current_time, info->full_time, ~(info->error_no));
    return 0;
}
int _media_info_dump(media_info_t* minfo)
{
    int i = 0;
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    printf("======||file size:%lld\n", minfo->stream_info.file_size);
    printf("======||file type:%d\n", minfo->stream_info.type);
    printf("======||has internal subtitle?:%s\n", minfo->stream_info.has_sub > 0 ? "YES!" : "NO!");
    printf("======||internal subtile counts:%d\n", minfo->stream_info.total_sub_num);
    printf("======||has video track?:%s\n", minfo->stream_info.has_video > 0 ? "YES!" : "NO!");
    printf("======||has audio track?:%s\n", minfo->stream_info.has_audio > 0 ? "YES!" : "NO!");
    printf("======||duration:%d\n", minfo->stream_info.duration);
    if (minfo->stream_info.has_video && minfo->stream_info.total_video_num > 0) {
        printf("======||video counts:%d\n", minfo->stream_info.total_video_num);
        printf("======||video width:%d\n", minfo->video_info[0]->width);
        printf("======||video height:%d\n", minfo->video_info[0]->height);
        printf("======||video bitrate:%d\n", minfo->video_info[0]->bit_rate);
        printf("======||video format:%d\n", minfo->video_info[0]->format);

    }
    if (minfo->stream_info.has_audio && minfo->stream_info.total_audio_num > 0) {
        printf("======||audio counts:%d\n", minfo->stream_info.total_audio_num);

        if (NULL != minfo->audio_info[0]->audio_tag) {
            printf("======||track title:%s", minfo->audio_info[0]->audio_tag->title);
            printf("\n======||track album:%s", minfo->audio_info[0]->audio_tag->album);
            printf("\n======||track author:%s\n", minfo->audio_info[0]->audio_tag->author);
            printf("\n======||track year:%s\n", minfo->audio_info[0]->audio_tag->year);
            printf("\n======||track comment:%s\n", minfo->audio_info[0]->audio_tag->comment);
            printf("\n======||track genre:%s\n", minfo->audio_info[0]->audio_tag->genre);
            printf("\n======||track copyright:%s\n", minfo->audio_info[0]->audio_tag->copyright);
            printf("\n======||track track:%d\n", minfo->audio_info[0]->audio_tag->track);
        }



        for (i = 0; i < minfo->stream_info.total_audio_num; i++) {
            printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
            printf("======||%d 'st audio track codec type:%d\n", i, minfo->audio_info[i]->aformat);
            printf("======||%d 'st audio track audio_channel:%d\n", i, minfo->audio_info[i]->channel);
            printf("======||%d 'st audio track bit_rate:%d\n", i, minfo->audio_info[i]->bit_rate);
            printf("======||%d 'st audio track audio_samplerate:%d\n", i, minfo->audio_info[i]->sample_rate);
            printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

        }

    }
    if (minfo->stream_info.has_sub && minfo->stream_info.total_sub_num > 0) {
        for (i = 0; i < minfo->stream_info.total_sub_num; i++) {
            if (0 == minfo->sub_info[i]->internal_external) {
                printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
                printf("======||%d 'st internal subtitle pid:%d\n", i, minfo->sub_info[i]->id);
                printf("======||%d 'st internal subtitle language:%s\n", i, minfo->sub_info[i]->sub_language);
                printf("======||%d 'st internal subtitle width:%d\n", i, minfo->sub_info[i]->width);
                printf("======||%d 'st internal subtitle height:%d\n", i, minfo->sub_info[i]->height);
                printf("======||%d 'st internal subtitle resolution:%d\n", i, minfo->sub_info[i]->resolution);
                printf("======||%d 'st internal subtitle subtitle size:%lld\n", i, minfo->sub_info[i]->subtitle_size);
                printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
            }
        }
    }
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    return 0;
}

static int get_axis(const char *para, int para_num, int *result)
{
    char *endp;
    const char *startp = para;
    int *out = result;
    int len = 0, count = 0;

    if (!startp) {
        return 0;
    }

    len = strlen(startp);

    do {
        //filter space out
        while (startp && (isspace(*startp) || !isgraph(*startp)) && len) {
            startp++;
            len--;
        }

        if (len == 0) {
            break;
        }

        *out++ = strtol(startp, &endp, 0);

        len -= endp - startp;
        startp = endp;
        count++;

    } while ((endp) && (count < para_num) && (len > 0));

    return count;
}

static int __set_display_axis(int recovery)
{
    int fd;
    char *path = "/sys/class/display/axis";
    char str[128];
    int count;
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        if (!recovery) {
            read(fd, str, 128);
            printf("read axis %s, length %d\n", str, strlen(str));
            count = get_axis(str, 8, axis);
        }
        if (recovery) {
            sprintf(str, "%d %d %d %d %d %d %d %d",
                    axis[0], axis[1], axis[2], axis[3], axis[4], axis[5], axis[6], axis[7]);
        } else {
            sprintf(str, "2048 %d %d %d %d %d %d %d",
                    axis[1], axis[2], axis[3], axis[4], axis[5], axis[6], axis[7]);
        }
        write(fd, str, strlen(str));
        close(fd);
        return 0;
    }

    return -1;
}

static int set_osd_blank(int blank)
{
    char *drm_osd_enable = "/sys/class/meson_crtc/enable_osd";
    char *path1 = "/sys/class/graphics/fb0/blank";
    char *path2 = "/sys/class/graphics/fb1/blank";
    char *wl_path0 = "/sys/kernel/debug/dri/0/vpu/blank";
    char *wl_path1 = "/sys/kernel/debug/dri/64/vpu/blank";

    char *fb_stat0 = "/sys/class/graphics/fb0/osd_status";
    char *fb_stat1 = "/sys/class/graphics/fb1/osd_status";

    static int fb0_enable = -1;
    static int fb1_enable = -1;
    static int use_wayland = 0;
    char osd_status[128]="";

    if (amsysfs_get_sysfs_str(drm_osd_enable, osd_status, sizeof(osd_status)) == 0) {
        amsysfs_set_sysfs_str(drm_osd_enable, blank ? "0" : "1");
        return 0;
    }
	
    if (fb0_enable == -1)
    {
        osd_status[0]='\0';
        if ( !use_wayland && (amsysfs_get_sysfs_str(fb_stat0, osd_status, 128) == 0))
        {
            if (strstr(osd_status, "enable: 1"))
                fb0_enable = 1;
            else
                fb0_enable = 0;
        } else if (amsysfs_get_sysfs_str(wl_path0, osd_status, 128) == 0)
        {
            use_wayland = 1;
            if (strstr(osd_status, "blank_enable: 0"))
                fb0_enable = 1;
            else
                fb0_enable = 0;
        }
        else
            fb0_enable = 1; //In case the osd_status node not exist

    }
    if (fb1_enable == -1)
    {
        osd_status[0]='\0';
        if (!use_wayland && (amsysfs_get_sysfs_str(fb_stat1, osd_status, 32) == 0))
        {
            if (strstr(osd_status, "enable: 1"))
                fb1_enable = 1;
            else
                fb1_enable = 0;
        } else if (amsysfs_get_sysfs_str(wl_path1, osd_status, 128) == 0)
        {
            use_wayland = 1;
            if (strstr(osd_status, "blank_enable: 0"))
                fb0_enable = 1;
            else
                fb0_enable = 0;
        }
        else
            fb1_enable = 1; //In case the osd_status node not exist
    }

    if (fb0_enable)
        amsysfs_set_sysfs_str(use_wayland ? wl_path0 : path1, blank ? "1" : "0");

    if (fb1_enable)
        amsysfs_set_sysfs_str(use_wayland ? wl_path1 : path2, blank ? "1" : "0");
    return 0;
}

static void set_vfm_path(void)
{
    char *vfm_path = "/sys/class/vfm/map";
    char *vfm_rm = "rm default";
    char *vfm_add = "add default decoder ppmgr deinterlace amvideo";

    amsysfs_set_sysfs_str(vfm_path, vfm_rm);
    amsysfs_set_sysfs_str(vfm_path, vfm_add);
}

static void signal_handler(int signum)
{
    printf("Get signum=%x\n", signum);
   // player_progress_exit();
   // __set_display_axis(1);
    set_black_policy(1);
    set_osd_blank(0);
    signal(signum, SIG_DFL);
    raise(signum);
}

static void set_video_display(int val)
{
    char *disable_path = "/sys/class/video/disable_video";

    amsysfs_set_sysfs_int(disable_path, val);
}

#define TMP_COMMAND_MAX 512

int main(int argc, char *argv[])
{
    play_control_t *pCtrl = NULL;
    int pid;
    int pos = 0;
    int speed = 0;
    int tmpneedexit = 0;
    int ret = -1;
    int flag = 1;
    media_info_t minfo;
    char tmpcommand[TMP_COMMAND_MAX];
    EMU_STEP tmpstep = EMU_STEP_MENU;

    pCtrl = (play_control_t*)malloc(sizeof(play_control_t));
    memset(pCtrl, 0, sizeof(play_control_t));
    memset(&minfo, 0, sizeof(media_info_t));
    if (argc < 2) {
        printf("usage:player file\n");
        return -1;
    }

    /*set default path for vfm.*/
    set_vfm_path();
    /*enable video layer*/
    set_video_display(2);

    player_init();
    //__set_display_axis(0);        //move osd out of screen to set video layer out

    player_register_update_callback(&pCtrl->callback_fn, &update_player_info, 1000);
    printf("player callback register....\n");

    pCtrl->file_name = strdup(argv[1]);

    //pCtrl->nosound = 1;   // if disable audio...,must call this api
    pCtrl->video_index = -1;// MUST
    pCtrl->audio_index = -1;// MUST
    pCtrl->sub_index = -1;/// MUST
    pCtrl->hassub = 1;  // enable subtitle

    //just open a buffer,just for p2p,http,etc...
    pCtrl->auto_buffing_enable = 1;
    pCtrl->buffing_min = 0.001;
    pCtrl->buffing_middle = 0.02;
    pCtrl->buffing_max = 0.9;

    pCtrl->t_pos = -1;  // start position, if live streaming, need set to -1
    pCtrl->need_start = 0; // if 0,you can omit player_start_play API.just play video/audio immediately. if 1,need call "player_start_play" API;

    pid = player_start(pCtrl, 0);
    if (pid < 0) {
        printf("player start failed!error=%d\n", pid);
        return -1;
    }
    signal(SIGSEGV, signal_handler);
    signal(SIGINT, signal_handler);
    //SYS_disable_osd0();
    set_osd_blank(1);

    while ((!tmpneedexit) && (!PLAYER_THREAD_IS_STOPPED(player_get_state(pid)))) {
        switch (tmpstep) {
        case EMU_STEP_PAUSE:
            player_pause(pid);
            tmpstep = EMU_STEP_MENU;
            break;
        case EMU_STEP_RESUME:
            player_resume(pid);
            tmpstep = EMU_STEP_MENU;
            break;
        case EMU_STEP_SEEK:
            pos = 0;
            printf("please input seek pos:");
            scanf("%d", &pos);

            printf("will  seek position: %d\n", pos);
            player_timesearch(pid, pos);
            tmpstep = EMU_STEP_MENU;
            break;

            break;
        case EMU_STEP_STOP:
            player_stop(pid);
            tmpstep = EMU_STEP_MENU;
            break;
        case EMU_STEP_FF:
            printf("please input fastforward speed:\n");
            speed = 1;
            player_forward(pid, speed);
            tmpstep = EMU_STEP_MENU;
            break;
        case EMU_STEP_RR:
            printf("please input fastrewind speed:");
            speed = 1;
            player_backward(pid, speed);
            tmpstep = EMU_STEP_MENU;
            break;
        case EMU_STEP_SETLOOP:
            player_loop(pid);
            tmpstep = EMU_STEP_MENU;
            break;
        case EMU_STEP_EXIT:
            player_exit(pid);
            tmpneedexit = 1;
            break;
        case EMU_STEP_START:
            player_start_play(pid);
            //SYS_set_tsync_enable(0);///< f no sound,can set to be 0
            tmpstep = EMU_STEP_MENU;
            break;
        case EMU_STEP_GETAVMEDIAINFO:
            if (pid >= 0) {
                if (player_get_state(pid) > PLAYER_INITOK) {
                    ret = player_get_media_info(pid, &minfo);
                    if (ret == 0) {
                        _media_info_dump(&minfo);
                    }
                }
            }
            tmpstep = EMU_STEP_MENU;
            break;
        case EMU_STEP_MENU:
            do {
                if (flag) {
                    printf(SCREEN_SPLITER);
                    printf("       	     player benchmark tool for android            v2.0\n");
                    printf(SCREEN_SPLITER);
                    printf("* Please choose one option                                 *\r\n");
                    printf("* 0   show main menu                                       *\r\n");
                    printf("* a   start play                                           *\r\n");
                    printf("* s   get media info                                       *\r\n");
                    printf("* 1   Pause play                                           *\r\n");
                    printf("* 2   Resume play                                          *\r\n");
                    printf("* 3   Stop play                                            *\r\n");
                    printf("* 4   Fastforward                                          *\r\n");
                    printf("* 5   Fastrewind                                       	   *\r\n");
                    printf("* 6   Seek                                             	   *\r\n");
                    printf("* 7   Set repeat                                           *\r\n");
                    printf("* 8   Quit tools                                           *\r\n");
                    printf(SCREEN_SPLITER);
                    printf("please input you choice:");
                }
                flag = 1;
                memset(tmpcommand, 0, TMP_COMMAND_MAX);

                scanf("%s", tmpcommand);

                if (strcmp(tmpcommand, "0") == 0) {
                    sleep(1);
                    tmpstep = EMU_STEP_MENU;
                } else if (strcmp(tmpcommand, "1") == 0) {
                    tmpstep = EMU_STEP_PAUSE;
                } else if (strcmp(tmpcommand, "2") == 0) {
                    tmpstep = EMU_STEP_RESUME;
                } else if (strcmp(tmpcommand, "3") == 0) {
                    tmpstep = EMU_STEP_STOP;
                } else if (strcmp(tmpcommand, "4") == 0) {
                    tmpstep = EMU_STEP_FF;
                } else if (strcmp(tmpcommand, "5") == 0) {
                    tmpstep = EMU_STEP_RR;
                } else if (strcmp(tmpcommand, "6") == 0) {
                    tmpstep = EMU_STEP_SEEK;
                } else if (strcmp(tmpcommand, "7") == 0) {
                    tmpstep = EMU_STEP_SETLOOP;
                } else if (strcmp(tmpcommand, "8") == 0) {
                    tmpstep = EMU_STEP_EXIT;
                } else if (strcmp(tmpcommand, "a") == 0) {
                    tmpstep = EMU_STEP_START;
                } else if (strcmp(tmpcommand, "s") == 0) {
                    tmpstep = EMU_STEP_GETAVMEDIAINFO;
                } else {
                    flag = 0;
                }
            } while (0);

            break;
        }
        usleep(100 * 1000);
        signal(SIGCHLD, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGHUP, signal_handler);
        signal(SIGTERM, signal_handler);
        signal(SIGSEGV, signal_handler);
        signal(SIGQUIT, signal_handler);
    }
    __set_display_axis(1);    //recover osd
    set_osd_blank(0);
    free(pCtrl->file_name);
    free(pCtrl);
    printf("...........player exit,~,byeybe...........\n");
    return 0;
}

