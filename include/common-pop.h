#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define DEBUG 42
#define MAXLINE 256
#define MAXFILENAME 256

#define H_CONTTYPE "Content-Type: "
#define MIME_MULT "multipart"
#define DEFAULT_EXT "txt"
#define MAIL_BASE_FOLDER "./obj"

#define SERVER_OK "+OK"
#define SERVER_ERR "-ERR"
#define CRLF "\n"
#define END "."

#define CMD_USER "USER"
#define CMD_PASS "PASS"
#define CMD_LIST "LIST"
#define CMD_TOP  "TOP"
#define CMD_QUIT "QUIT"
#define CMD_RETR "RETR"

typedef struct mime{
	char* mimetype;
	char* canonical;
	struct mime *next;
}
mimes;

typedef struct mail{
	int 	id;
	char* 	from;
	char*  date;
	mimes* mime;
	char* mixed_boundary;
	struct mail *next;
}
mails;

typedef struct {
	mimes *first_mime;
	mimes *last_mime;
}pop;

#include <textuel-pop.h>
#include <utils-pop.h>
#include <mimes-pop.h>
#include <main-pop.h>
