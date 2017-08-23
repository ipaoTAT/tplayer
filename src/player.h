#ifndef PLAYER_H	
#define PLAYER_H
#include"init.h"



/* Public Variable */

int _COLS;
int offset = 0;

int cur_time;	/*ms*/
int cur_song;	/*ms*/
int cur_play_song;	/*ms*/
int total_time;/*ms*/ 
int slide_pos = 0; /* time slide position*/
int total_cols = 0;/* total cols of lrc*/
int count = 0; /*current line of lrc */
int song_num = 0;

int next_song_flag = 0;
int circal_play_flag = 0;
int play_mod = 1;

pthread_t ttid;	/*timmer thread id*/
pthread_mutex_t cur_time_mutex;
pthread_mutex_t refresh_mutex;
pthread_mutex_t addstr_mutex;
pthread_mutex_t lrc_mutex;
pthread_mutex_t exit_mutex;

line_info buf[100];
song_list songs[100];
song_info *song = NULL;

int msg_time = 0;

/*functions*/

void *lrc_timer(void *);

void *get_key_in(void *);

int print_lrc();

int print_slide_line(int position);
int print_list();
int play_song(char *);

static void finish(int sig);
#endif
