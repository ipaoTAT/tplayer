#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <cursesw.h>

#include "player.h"
int song_count = 0;
char *mp3_dir = "../mp3";

inline int add_str(int x, int y, char *str, int times){
	int i;
	if(!pthread_mutex_trylock(&refresh_mutex)){
		pthread_mutex_lock(&addstr_mutex);
		if(x >=0 && y >= 0)move(x, y);
		for(i = 0; i < times; i++)addstr(str);
		pthread_mutex_unlock(&addstr_mutex);
		pthread_mutex_unlock(&refresh_mutex);
	}
}

int pmsg(char *msg){
	if(msg != NULL){
		add_str(2, LINES, msg, 1);
		msg_time = 2000;
	}else if(msg_time <= 0){
		add_str(2, LINES, " ", COLS - 2);
	}
}


int main(int argc, char **argv)
{
	if(chdir(mp3_dir) == -1)return -1;

	(void)signal(SIGINT, finish);
	setlocale(LC_ALL,"");
	init_screen();
	if(argc == 2 && init_lrc(argv[1]) == 0);
	else if(argc == 1 && init_lrc(NULL) == 0);
	else return 1;
	init_list("./");

	pthread_mutex_init(&cur_time_mutex, NULL);
	pthread_mutex_init(&refresh_mutex, NULL);
	pthread_mutex_init(&addstr_mutex, NULL);
	pthread_mutex_init(&exit_mutex, NULL);

	if(0 != pthread_create(&ttid, NULL, lrc_timer, NULL)){
		printf("error: pthread_create\n");
	}else pthread_detach(ttid);
	if(0 != pthread_create(&ttid, NULL, get_key_in, NULL)){
		printf("error: pthread_create\n");
	}else pthread_detach(ttid);
	sleep(1);
	pthread_mutex_lock(&exit_mutex);
	finish(0);
	return 0;
}
void *lrc_timer(void *argv){
	char time[20];
	int ctime = 0, ttime = -1;
	pthread_mutex_lock(&exit_mutex);
	while(1){
		while(cur_time <= total_time || total_time == 0){
			if(COLS < 80)offset = 1;
			switch(offset){
				case -1: _COLS = COLS; break;
				case 0:  _COLS = __COLS; break;
				case 1:  _COLS = 0; break;
				default: break;
			}
			usleep(98500);
			if(total_time > 0)cur_time += 100;
			ctime = cur_time;
			if(buf[count + 1].time >= 0 && ctime >= buf[count + 1].time)count++;
			sprintf(time, "%02d:%02d", (ctime)/60000, (ctime) % 60000 / 1000);
			if(total_time != 0)slide_pos = (ctime * COLS) / total_time;
			print_slide_line(slide_pos);
			add_str(6, 2, time, 1);
			if(offset <= 0)print_lrc();
			if(offset >= 0)print_list();
			if(msg_time > 0)
			{
				msg_time -= 100;
				if(msg_time < 0)pmsg(NULL);
			}
			pthread_mutex_lock(&refresh_mutex);
			refresh();
			pthread_mutex_unlock(&refresh_mutex);
		}
		sleep(1);
		if(play_mod == 0 || (play_mod == 1 && cur_play_song + 1 == song_num)){
			init_lrc(NULL);
			continue;
		}
		if(cur_play_song == song_num - 1)cur_play_song = -1;
		cur_play_song++;
		play_song("");
		play_song(songs[cur_play_song].filename);
		init_lrc(songs[cur_play_song].filename); 
		cur_song = cur_play_song;
		if((cur_song - song_count) * 2 > (LINES - TOP - 5))song_count++;	  
		if(cur_song < song_count)song_count = cur_song;	  
	}
	add_str(0,0,"end",3);
	sleep(1);
	pthread_mutex_unlock(&exit_mutex);
	pthread_exit(NULL);
}

void *get_key_in(void *argv){
	while(1){
		int key = getch();
		//if(key == 27 && getch() == 91){
		//	key = getch();
		//	if(key == 65)key = 'w';
		//	else if(key == 66)key = 's';
		//	else continue;
		//}
		int cols = COLS;
		switch(key){
			case 's': if(cur_song < song_num - 1)cur_song++; 
				  if((cur_song - song_count) * 2 > (LINES - TOP - 5))song_count++;	  
				  break;
			case 'w': if(cur_song > 0)cur_song--;
				  if(cur_song < song_count)song_count = cur_song;	  
				  break;
			case 'a': if(offset == -1)offset = 0;
				  else if(offset == 0)offset = 1;
				  break;
			case 'd': if(offset == 1)offset = 0;
				  else if(offset == 0)offset = -1;
				  break;
			case '=':  play_mod++;
				   play_mod %= 3;
				   break;
			case '\r': play_song(songs[cur_song].filename);
				   msleep(500);
				   init_lrc(songs[cur_song].filename); 
				   cur_play_song = cur_song;
				   break;
			case ' ':  play_song("");
				   init_lrc(NULL);
				   break;
			case 'q':  
				   play_song("");
				   msleep(500);
				   finish(0);
				   exit(0);
			default: break;
		}
	}
}

int print_list(){
	int i, n;
	for(i = TOP; i < LINES; i++)
		add_str(i, _COLS + 1, "|", 1);
	i = TOP;
	add_str(i, _COLS + 2, "共", 1);
	char tmp[10];
	sprintf(tmp, "%d\n", song_num);
	add_str(i, _COLS + 3, tmp, 1);
	add_str(i, _COLS + 5, "首", 1);
	switch(play_mod){
		case 0: strncpy(tmp, "单曲\0", 10);break;
		case 1: strncpy(tmp, "顺序\0", 10);break;
		case 2: strncpy(tmp, "循环\0", 10);break;
	}
	add_str(i, COLS - 6, tmp, 1);
	i++;
	add_str(i++, _COLS + 2, " ", COLS - _COLS - 3);
	add_str(i++, _COLS + 2, " ", COLS - _COLS - 3);
	add_str(i++, _COLS + 2, "-", COLS - _COLS - 3);
	for(n = song_count; n < song_num; n++){
		if((n - song_count) * 2 > (LINES - TOP - 5))break;
#ifdef COLORFUL
		if(n == cur_song)attron(COLOR_PAIR(2));
#else
		if(n == cur_song)standout();
#endif
		sprintf(tmp, "%d. \0", n + 1);
		add_str(i, _COLS + 2, " ", COLS - _COLS - 3);
		add_str(i++, _COLS + 3, tmp, 1);
		add_str(-1, _COLS + 3, songs[n].filename, 1);
#ifdef COLORFUL
		if(n == cur_song)attroff(COLOR_PAIR(2));
#else
		if(n == cur_song)standend();
#endif
		add_str(i++, _COLS + 2, "-", COLS - _COLS - 3);
	}
	for(; i < LINES; i++)
		add_str(i, _COLS + 2, " ", COLS - _COLS - 3);
	for(i = TOP; i < LINES; i++)
		add_str(i, COLS - 1, "|", 1);
	return 0;
}

int print_slide_line(int pos){
	if(pos > COLS - 2)return 0;
	add_str(7, 0, "=", COLS);
	add_str(7, pos, "[]", 1);
	return 0;
}


int print_lrc(){
	int cur = 8;
	int i = count - 10;
	for(; i < 0 ; i++){
		add_str(cur, 0, " ", _COLS + 1);
		cur++;
	}
	int n;

	while(i <= total_cols)
	{
		pthread_mutex_lock(&lrc_mutex);
		n = _COLS/2 - strlen(buf[i].comment)/2;
		pthread_mutex_unlock(&lrc_mutex);
		if(cur >= LINES)break;
		add_str(cur, 0, " ", n);
		if(i == count)add_str(cur, 0, "-", n);
#ifdef COLORFUL
		if(i == count)attron(COLOR_PAIR(2));
else
		if(i == count)standout();
#endif
		pthread_mutex_lock(&lrc_mutex);
		add_str(cur, n, buf[i].comment, 1);
		pthread_mutex_unlock(&lrc_mutex);
#ifndef COLORFUL
		if(i == count)standend();
#else
		if(i == count)attroff(COLOR_PAIR(2));
#endif
		if(i == count)add_str(-1, -1, "-", n + _COLS);
		else add_str(-1, -1, " ", n + _COLS);
		cur++;
		i++;
	}
	move(cur, 0);
	for(; cur < LINES; cur++)
		add_str(cur, 0, " ", _COLS + 1);
	curs_set(0);
}

int play_song(char *name){
        char cmd[100];
	int pid;
	if((pid = fork()) == 0)
	{
		if(execlp("../bin/play", "play", name) == -1)exit(1);
		exit(0);
	}
	else if(pid == -1)return -1;
	else return 0;
}



void finish(int sig){
	endwin();
	exit(0);
}


