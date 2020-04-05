#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef O_BINARY
#	define O_BINARY	0
#endif
#define WIDTH	80

typedef unsigned char	uint8;

static int readRecord(int fd, int max, char *text)
{
	uint8 len;

	read(fd, &len, 1);
	read(fd, text, max);

	memset(text + len, 0, max - len);
	return len;
}

static void printRecord(const char *text, int len)
{
	const char *p;

	if(len <= 0)
		return;

	printf("%.*s", WIDTH, text);

	for(p = text + WIDTH; p < text + len; p += WIDTH)
		printf("\t%.*s", WIDTH, p);
}

static int decode(const char *file1, const char *file2)
{
	int fd1 = -1, fd2 = -1, len;
	char text[256];

	if((fd1 = open(file1, O_RDONLY | O_BINARY)) < 0) {
		fprintf(stderr, "CANNOT OPEN %s\n", file1);
		return 1;
	}

	if(file2 == NULL) { /* LINE */
		while(!eof(fd1)) {
			len = readRecord(fd1, 80, text);
			printRecord(text, len);
			printf("\n");
		}
	} else { /* ROOMS, SPECIAL */
		if((fd2 = open(file2, O_RDONLY | O_BINARY)) < 0) {
			fprintf(stderr, "CANNOT OPEN %s\n", file2);
			return 1;
		}

		while(!eof(fd1) && !eof(fd2)) {
			len = readRecord(fd1, 240, text);
			printRecord(text, len);
			if((len = readRecord(fd2, 240, text)) > 0) {
				printf("\t");
				printRecord(text, len);
			}
			printf("\n");
		}
	}

	if(fd1 > 0)
		close(fd1);
	if(fd2 > 0)
		close(fd2);
	return 0;
}

static int removeTab(char *text)
{
	int len = strlen(text);
	char *p;

	if(text[len - 1] == '\r' || text[len - 1] == '\n') {
		len--;
		text[len] = 0;
	}

	for(p = text + WIDTH; p < text + strlen(text); p += WIDTH) {
		if(*p != '\t') {
			fprintf(stderr, "WRONG FORMAT\n");
			return 0;
		}
		memmove(p, p + 1, strlen(p) + 1);
	}
	return 1;
}

void writeRecord(int fd1, int fd2, const char *text)
{
	int len = strlen(text);
	const char *p = text;
	uint8 len8;
	char buf[1024];

	memset(buf, ' ', sizeof(buf));
	memcpy(buf, text, len);

	if(fd2 > 0) {
		if(len > 240)
			len8 = 240;
		else
			len8 = len;
		write(fd1, &len8, 1);
		write(fd1, buf, 240);

		if(len > 240)
			len -= 240;
		else
			len = 0;
		len8 = len;
		write(fd2, &len8, 1);
		write(fd2, buf + 240, 240);
	
	} else {
		len8 = len;

		write(fd1, &len8, 1);
		write(fd1, buf, 80);
	}
}

static int encode(const char *file1, const char *file2)
{
	int fd1 = -1, fd2 = -1;
	char text[1024];

	if((fd1 = open(file1, O_CREAT | O_RDWR | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) < 0) {
		fprintf(stderr, "CANNOT OPEN %s\n", file1);
		return 1;
	}
	if(file2 != NULL) {	
		if((fd2 = open(file2, O_CREAT | O_RDWR | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) < 0) {
			fprintf(stderr, "CANNOT OPEN %s\n", file2);
			return 1;
		}
	}

	while(fgets(text, sizeof(text), stdin) != NULL) {
		if(!removeTab(text))
			return 1;
		writeRecord(fd1, fd2, text);
	}

	if(fd1 > 0)
		close(fd1);
	if(fd2 > 0)
		close(fd2);
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc <= 1) {
		fprintf(stderr, "usage:\ntxtwrite -d ROOMS1 ROOMS2 > rooms.txt\n");
		fprintf(stderr, "txtwrite -d SPECIAL1 SPECIAL2 > special.txt\n");
		fprintf(stderr, "txtwrite -d LINE > line.txt\n");
		fprintf(stderr, "txtwrite -e ROOMS1 ROOMS2 < rooms.txt\n");
		fprintf(stderr, "txtwrite -e SPECIAL1 SPECIAL2 < special.txt\n");
		fprintf(stderr, "txtwrite -e LINE < line.txt\n");
		return 0;
	}

	if(stricmp(argv[1], "-d") == 0) {
		if(argc == 3)
			return decode(argv[2], NULL);
		else if(argc == 4)
			return decode(argv[2], argv[3]);
	} else if(stricmp(argv[1], "-e") == 0) {
		if(argc == 3)
			return encode(argv[2], NULL);
		else if(argc == 4)
			return encode(argv[2], argv[3]);
	}

	fprintf(stderr, "ERROR\n");
	return 1;
}

/* eof */
