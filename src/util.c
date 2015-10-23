#include <pthread.h>
#include <string.h>

#include "util.h"


inline int end_with(char *str, char *pattern){	/*flag = 0 Noting to do , flag = 1, */
	return strncmp(str + strlen(str) - strlen(pattern), pattern, strlen(pattern)) == 0 ? 1 : 0;
}

inline int split_end(char *str, char *pattern){
	if(!end_with(str, pattern))return 1;
	str[strlen(str) - strlen(pattern)] = '\0';
	return 0;
}

inline int add_end(char *str, char *pattern){
	//please make sure str has enouth capacity
	strncpy(str + strlen(str), pattern, strlen(pattern) + 1);
	return 0;
}


int line_fmt(char *buf, line_info *info){
	int pos = 0;
	int mm = 0, ss = 0, ms = 0;
	if(buf[pos] == '\0')return 1;
	while(buf[pos++] == ' ');
	pos--;
	if(buf[pos] == '[' && buf[pos + 9] == ']'){
		pos++;
		sscanf(buf+pos, "%2d:%2d.%2d", &mm, &ss, &ms);
//		printf("mm:%d, ss:%d, ms:%d\n",mm,ss,ms);
		info -> time = mm * 60000 + ss * 1000 + ms * 10;
		pos += 9;
	}else return 1;

	strcpy(info -> comment, buf + pos);
	return 0;
}

