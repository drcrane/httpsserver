#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include "http-parser/http_parser.h"

//#define HTTPHANDLER_PROCESSING 0
//#define HTTPHANDLER_FINISHED 1

#define HTTPHANDLER_PROCESS_WAITFORREAD 1
#define HTTPHANDLER_PROCESS_WAITFORWRITE 2
#define HTTPHANDLER_PROCESS_CLOSESOCKET 3

#define HTTPHANDLER_WAITINGTOREAD   1
#define HTTPHANDLER_WAITINGTOWRITE  2
#define HTTPHANDLER_WAITINGFORREUSE 0

struct httphandler_client {
	int state;
	http_parser parser;
	size_t readbufsz;
	char * readbuf;
	size_t urlbufsz;
	char * urlptr;
	char * urlbuf;
	int finished;
	void * readptr;
	int (*read)(void * ctx, void * buf, size_t maxlen);
};

int httphandler_init(http_parser_settings * settings);
http_parser_settings * httphandler_getdefaultsettings();
struct httphandler_client * httphandler_new();
int httphandler_process(struct httphandler_client * client);
int httphandler_destroy(struct httphandler_client * client);

#endif // HTTPHANDLER_H

