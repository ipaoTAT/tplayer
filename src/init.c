#include <cursesw.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <pthread.h>

#include"init.h"


int init_screen(){

	initscr();//初始化 curses 包
	keypad(stdscr, TRUE);//允许键盘映射
	(void)nonl();
	(void)cbreak();
	(void)noecho();
	//判断是否支持彩色
#ifdef COLORFUL
	if(has_colors())
	{
		start_color();
		//初始化颜色配对表
		init_pair(0, COLOR_BLACK, COLOR_BLACK);
		init_pair(1, COLOR_GREEN, COLOR_BLACK);
		init_pair(2, COLOR_RED, COLOR_BLACK);
		init_pair(3, COLOR_CYAN, COLOR_BLACK);
		init_pair(4, COLOR_WHITE, COLOR_BLACK);
		init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(6, COLOR_BLUE, COLOR_BLACK);
		init_pair(7, COLOR_YELLOW, COLOR_BLACK);
	}
#endif
	move(1,COLS - 20); 
	add_str(-1, -1, "w  : list go up", 1); 
	move(2,COLS - 20); 
	add_str(-1, -1, "s  : list go down", 1); 
	move(3,COLS - 20); 
	add_str(-1, -1, "a/d: change view", 1); 
	move(4,COLS - 20); 
	add_str(-1, -1, "=  : change Play mod", 1); 
	move(5,COLS - 20); 
	add_str(-1, -1, "<Enter>: Play", 1); 
	move(6,COLS - 20); 
	add_str(-1, -1, "q: exit", 1); 
}

int init_list(char *dir){
	DIR *dp = opendir(dir);
	if(dp == NULL)return -1;
	struct dirent *de = NULL;
	while((de = readdir(dp)) != NULL){
		if(strcmp(de -> d_name, ".") == 0 || strcmp(de -> d_name, "..") == 0)
			continue;
		if(!end_with(de -> d_name, ".mp3"))continue;
		songs[song_num].id = song_num;
		strcpy(songs[song_num].filename, de -> d_name);
		split_end(songs[song_num].filename, ".mp3");
		song_num++;
		if(song_num == 100)break;
	}
	cur_song = 0;
}

int init_lrc(char *name){
	if(name == NULL){
		buf[0].time = 0;
		strcpy(buf[0].comment, "[00:00:00]请选择要播放的歌曲\0");
		buf[1].time = -1;
		buf[1].comment[0] = 0;
		total_cols = 0;
		total_time = 0;
		cur_time = 0;
		count = 0;
		slide_pos = 0;
		return 0;
	}
	char comment[10000];
	char filename[100];
	strcpy(filename, name);
	filename[strlen(name)] = '\0';
	if(end_with(filename, ".lrc") == 0)add_end(filename, ".lrc");
	int n, i;
	FILE *fp = fopen(filename, "r");
	int len;
	if(fp != NULL){
		len = fread(comment, 1, 10000, fp);
		fclose(fp);
	}else {
		strcpy(comment, "[00:00:00]没有找到歌词文件（.lrc)\n\0");
		len = strlen(comment);
		buf[1].time = -1;
		buf[1].comment[0] = 0;
		total_cols = 0;
	}
	
	total_cols = 0;
	char *comment_ptr = comment;
	i = 0; n = 0;
	char line_buf[100];
	pthread_mutex_lock(&lrc_mutex);
	char tmp;
	do{
		n = 0;
		do{
			tmp = ( line_buf[n++] = *comment_ptr++);
			len--;
		}while(tmp != '\n' && len > 0);
		line_buf[--n] = '\0';
		if(!line_fmt(line_buf, &buf[i])){
			total_cols++;
			i++;
		}
	}while(len > 0);
	buf[i].time = -1;
	buf[i].comment[0] = 0;
	pthread_mutex_unlock(&lrc_mutex);
	char argvv[20];
	char *argvvv[2];
	strcpy(argvv, filename);
	split_end(argvv, ".lrc");
	add_end(argvv, ".mp3");
	argvvv[1] = argvv;

	song = song_info_init(2, argvvv);
	if(song == NULL){
//		printf("Cannot find MP3 File (%s)!\n", argvv[1]);
		return 1;
	}else if(song == (song_info *)1){
//		printf("Error\n");
		return 1;
	}

	move(1,2); 
	addstr("Title:\t"); 
	addstr(song -> title);
	add_str(-1, -1, " ", 50); 
	move(2,2); 
	addstr("Artist:\t"); 
	addstr(song -> artist);
	add_str(-1, -1, " ", 50); 
	move(3,2); 
	addstr("Album:\t"); 
	addstr(song -> album);
	add_str(-1, -1, " ", 50); 
	move(4,2); 
	addstr("File:\t\t"); 
	addstr(argvv);
	add_str(-1, -1, " ", 50); 
	move(6, 7);
	char time[20];
	sprintf(time, "\/%2d:%2d", (song -> time)/60000, (song -> time) % 60000 / 1000);
	addstr(time);
	move(6, 2);
	addstr("00:00");
	total_time = song -> time;
	cur_time = 0;
	slide_pos = 0;
	count = 0;
	return 0;
}


