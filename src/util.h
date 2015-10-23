
#ifndef UTIL_H
	#define UTIL_H
#endif
#include <pthread.h>

typedef struct line_info{
	long time;	//ms
	char comment[100];
} line_info;

typedef struct song_list{
	int id;
	char filename[100];
} song_list;


#define TOP 8

#define __COLS  (COLS - 30)

#define msleep(time) usleep(time * 1000)

extern pthread_t ttid;	/*timmer thread id*/
extern pthread_mutex_t cur_time_mutex;
extern pthread_mutex_t refresh_mutex;
extern pthread_mutex_t addstr_mutex;
extern pthread_mutex_t lrc_mutex;
extern pthread_mutex_t exit_mutex;

/*functions*/


inline int end_with(char *str, char *pattern);
inline int add_end(char *str, char *pattern);
inline int split_end(char *str, char *pattern);

int line_fmt(char *src, line_info *dst);
