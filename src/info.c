#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>

#include"info.h"
//extern int pmsg(char *);

tag_frame_head *get_first_frame(tag_head *tag){
	return (tag_frame_head *)((char *)tag + 10);
}

tag_frame_head *next_frame(tag_head *head, tag_frame_head *cur){

	tag_frame_head *next = (tag_frame_head *)((char *)cur + 10 + SIZE_OF_FRAME_BODY(cur -> size));
	if((unsigned long)next - (unsigned long)head > SIZE_OF_TAG(head -> size))
	{
		next = NULL;
	}
	return next;

}

int pre_check(char *buf){

	int i;
	for(i = 0; buf[i] == 0xff && i < 100; i++){
//		printf("%c ", buf[i]);
	}
	if(i == 100){
		printf("All if 0xff\n");
		return -1;		/* ill */
	}

	tag_head *ptr = (tag_head *)(buf + i);
	if(strncmp(ptr -> header, "ID3", 3) && strncmp(ptr -> header, "id3", 3))
	{
		//pmsg("ID3 No Found\n");
		return -1;		/* header is not 'ID3' or 'tag' */
	}


	return SIZE_OF_TAG(ptr -> size);
}

int info_fmt(song_info *song, tag_head *tag){
	
	tag_frame *frame = (tag_frame *)get_first_frame(tag);
//	tag_frame *frame = (tag_frame *)(tag + 10);
	if(frame == NULL)return -1;
	int body_size;
	while(frame != NULL){
		body_size = SIZE_OF_FRAME_BODY(frame -> head.size) - 1;
#ifdef DEBUG_TAG
		printf("\n%s\t", frame -> head.frame_id);
		printf(">>>body_size = %d\n", body_size);
#endif
		if(!strncmp(frame -> head.frame_id, "TIT2", 4)){
			if(body_size >= 50)body_size = 49;
			memcpy(song -> title, (char *)&(frame -> body) + 1, body_size);
			song -> title[body_size] = 0;
		}
		else if(!strncmp(frame -> head.frame_id, "TPE1", 4)){
			if(body_size >= 50)body_size = 49;
			memcpy(song -> artist, (char *)&(frame -> body) + 1, body_size);
			song -> artist[body_size] = 0;
		}
		else if(!strncmp(frame -> head.frame_id, "TALB", 4)){
			if(body_size >= 50)body_size = 49;
			memcpy(song -> album, (char *)&(frame -> body) + 1, body_size);
			song -> album[body_size] = 0;
		}
		else if(!strncmp(frame -> head.frame_id, "TYER", 4)){
			if(body_size >= 50)body_size = 49;
			memcpy(song -> born, (char *)&(frame -> body) + 1, body_size);
			song -> born[body_size] = 0;
		}
		else if(!strncmp(frame -> head.frame_id, "TCON", 4)){
			if(body_size >= 50)body_size = 49;
			memcpy(song -> type, (char *)&(frame -> body) + 1, body_size);
			song -> type[body_size] = 0;
		}
		else if(frame -> head.frame_id[0] == 0){
#ifdef DEBUG_TAG
			printf("tag end\n");
#endif
			break;
		}else{
#ifdef DEBUG_TAG
			printf("No Use Frame : %s\n", frame -> head.frame_id);
#endif
		}

		frame = (tag_frame *)next_frame(tag, frame);
	}

	return 0;
}


int song_init(song_info *song){
	char *target = "Unknown\0";	/* <= 50 bytes */
	strncpy(song -> title, target, strlen(target) + 1);
	strncpy(song -> artist, target, strlen(target) + 1);
	strncpy(song -> album, target, strlen(target) + 1);
	strncpy(song -> type, target, strlen(target) + 1);
	strncpy(song -> born, target, strlen(target) + 1);
	song -> time = 0x00000000;
}


int cy_rate(char key){
	char tmp = key & 0x0c;
#ifdef DEBUG_TIME
	printf("\nDEBUG INFO <cy-rate()> key = %x, tmp = %x\n", key, tmp);
#endif
	switch(tmp){
		case 0x00:		return 44100;
		case 0x04:	return 48000;
		case 0x08:	return 32000;
		case 0x0c:
		default:	return -1;
	}
}

int bit_rate(char key){
	char tmp = (key & 0xf0) >> 4;
#ifdef DEBUG_TIME
	printf("\nDEBUG INFO <bt-rate()> key = %x, tmp = %x\n", key, tmp);
#endif
	switch(tmp){
		case 0x01:	return 32;
		case 0x02:	return 40;
		case 0x03:	return 48;
		case 0x04:	return 56;
		case 0x05:	return 64;
		case 0x06:	return 80;
		case 0x07:	return 96;
		case 0x08:	return 112;
		case 0x09:	return 128;
		case 0x0a:	return 160;
		case 0x0b:	return 192;
		case 0x0c:	return 224;
		case 0x0d:	return 256;
		case 0x0e:	return 320;
		case 0x00:
		case 0xff:
		default:	return -1;
	}
}

int compute_time_cbr(char *start, int fd, int tag_size){
	int pos = 2;
	int bt = bit_rate(start[pos]);

	struct stat fs;
	if(fstat(fd, &fs) < 0)return -1;

	long f_size = (long)fs.st_size;
#ifdef DEBUG_TIME
	printf("File size : %ldBytes\n", f_size);
	printf("BitRate : %dKbps\n", bt);
#endif

	float time = (f_size - tag_size) * 8 / bt;	/* cannot * 1000, bt is "Kbps"*/
	int mstime = (int)time;
	return mstime;
}

int compute_time_xing(char *start){
	int pos = 2;
	int cy = cy_rate(start[pos]);
	pos = 0;
	pos = pos + 4 + 32;
#ifdef DEBUG_TIME
	printf("XING : [%x]\n", start[pos]);
#endif
	if(strncmp(&start[pos], "Info", 4)){
#ifdef DEBUG_TIME
		printf("No Xing Head\n");
#endif
		return -1;
	}

	int *tmp = start + pos + 8;

	int tmp1 = *tmp;
	int frame_num = 0;
	char *tmp2 = &tmp1;
	char *tmp3 = &frame_num;
	tmp3[3] = tmp2[0];
	tmp3[2] = tmp2[1];
	tmp3[1] = tmp2[2];
	tmp3[0] = tmp2[3];
	float fn = frame_num;
#ifdef DEBUG_TIME
	printf("frame_num = %x\n", frame_num);
#endif

	float time = 1000 * fn * 1152 / cy;
	int mstime = (int)time;
	return mstime;
}

song_info *song_info_init(int argc, char **argv){
	if(argc != 2){
		printf("Need One Argv\n");
		return 1;
	}

	char *song_buf = (char *)malloc(sizeof(song_info));
	song_info *song = (song_info*)song_buf;
	song_init(song);

	int fd = open(argv[1], O_RDONLY);
	if(fd == -1)return song;
	int i;

	char buf[200];

	if(read(fd, buf, 200) < 0)return 1;
	buf[199] = 0;

#ifdef DEBUG_TAG
	char *tmp = buf;
	for(i = 0; i < 200; i++)printf("%2x(%c) ",tmp[i], (tmp[i] > 31 && tmp[i] < 127) ? tmp[i] : ' ');
#endif

	int tag_size = pre_check(buf);
	if(tag_size <= 0)
		return song;

	char *tag_buf = (char *)malloc(sizeof(char) * tag_size);
	tag_head *tag = (tag_head *)tag_buf;
	if(tag == NULL){
		//pmsg("error: malloc\n");
		return song;
	}
	memcpy(tag, buf, 200);

	if(tag_size > 200){
		if(read(fd, tag_buf + 200, tag_size - 200) < 0){
			//pmsg("error: read\n");
			close(fd);
			return song;
		}
	}


	if(info_fmt(song_buf, tag_buf) < 0){
		//pmsg("error:info_fmt");
		close(fd);
		return song;
	}

	if(lseek(fd, tag_size, SEEK_SET) == -1)return song;

	if(read(fd, buf, 200) == -1)return song;
#ifdef DEBUG_TIME
	char *tmp = buf;
	for(i = 0; i < 200; i++)printf("%2x(%c) ",tmp[i], (tmp[i] > 31 && tmp[i] < 127) ? tmp[i] : ' ');
#endif

	for(i = 0; i < 200; i++){
		if(!(~buf[i]) && (buf[i + 1] & 0xe0) == 0xe0)break;
	}
	if(i == 200 )return song;
	
	int time = compute_time_xing(buf + i);
	if(time < 0)time = compute_time_cbr(buf + i, fd, tag_size);
	if(time < 0)time = 0;
	song -> time = time;

	close(fd);

	//output_info(song);

	free(tag_buf);
	return song;
}

int output_info(song_info *song){
	printf("\n>>>>>>\n");
	printf("Title : %s\n", song -> title);
	printf("Title : %x\n", song -> title[0]);
	printf("Artist : %s\n", song -> artist);
	printf("Album : %s\n", song -> album);
	printf("Born : %s\n", song -> born);
	printf("Type : %s\n", song -> type);
	printf("Time : %dms\n", song -> time);
}
