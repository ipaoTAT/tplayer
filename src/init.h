#ifndef INIT_H
#define INIT_H

extern "C"{
#include <pthread.h>
#include "info.h"
#include "util.h"
}

extern int _COLS;
extern int offset;

extern int cur_time;	/*ms*/
extern int cur_song;	/*ms*/
extern int cur_play_song;	/*ms*/
extern int total_time;/*ms*/ 
extern int slide_pos; /* time slide position*/
extern int total_cols; /* total cols of lrc*/
extern int count; 	/*current line of lrc */
extern int song_num;

extern int next_song_flag;
extern int circal_play_flag;

extern pthread_t ttid;	/*timmer thread id*/
extern pthread_mutex_t cur_time_mutex;
extern pthread_mutex_t refresh_mutex;
extern pthread_mutex_t addstr_mutex;
extern pthread_mutex_t lrc_mutex;
extern pthread_mutex_t exit_mutex;

extern line_info buf[100];
extern song_list songs[100];
extern song_info *song;
extern inline int add_str(int, int, char *, int);


int init_screen();
int init_lrc(char *name);
int init_list(char *dirname);
#endif
