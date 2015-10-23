#include<stdio.h>

#define SIZE_OF_TAG(size) (size[0]&0x7F)*0x200000+(size[1]&0x7F)*0x4000+(size[2]&0x7F)*0x80+(size[3]&0x7F)
#define SIZE_OF_FRAME_BODY(size) size[0]*0x100000000+size[1]*0x10000+size[2]*0x100+size[3]

typedef struct song_info{
	char title[50];
	char artist[50];
	char album[50];
	char born[50];	/* format yyyy-mm-dd */
	char type[50];
	int time;	/* ms */
} song_info;

typedef struct tag_head{
	char header[3];	/* "ID3" */
	char version; /* ID3V2.x */
	char reversion; /* 0 */
	char flag;
	char size[4];	/* size of total tag (head and frames )*/

} tag_head;

typedef struct tag_frame_head{
	char frame_id[4];
	char size[4];	/* size of frame body (exclude head)*/
	char flag[2];
} tag_frame_head;

typedef struct tag_frame{
	tag_frame_head head;
	char body;
} tag_frame;

inline tag_frame_head *get_first_frame(tag_head *tag){
	return (tag_frame_head *)((char *)tag + 10);
}

inline tag_frame_head *next_frame(tag_head *head, tag_frame_head *cur){

	tag_frame_head *next = (tag_frame_head *)((char *)cur + 10 + SIZE_OF_FRAME_BODY(cur -> size));
	if((int)next - (int)head > SIZE_OF_TAG(head -> size))
	{
		next = NULL;
	}
	return next;

}

extern int song_init(song_info *);

extern song_info *song_info_init(int, char**);
