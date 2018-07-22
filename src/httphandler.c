#include <stdio.h>
#include <stdlib.h>
#include "httphandler.h"
#include "http-parser/http_parser.h"

#include "dbg.h"

static http_parser_settings http_settings;

static int on_message_begin(http_parser * parser) {
	return 0;
}

#define URLBUFFER_ALLOCATION_MULTIPLE 1024

static int on_url(http_parser * parser, const char * at, size_t length) {
	struct httphandler_client * clictx = parser->data;
	size_t datasz = clictx->urlptr - clictx->urlbuf;
	size_t minreq = length + datasz + 1;
	if (minreq > clictx->urlbufsz) {
		size_t toalloc = clictx->urlbufsz + URLBUFFER_ALLOCATION_MULTIPLE;
		while (minreq > toalloc) {
			toalloc += URLBUFFER_ALLOCATION_MULTIPLE;
		}
		clictx->urlbuf = realloc(clictx->urlbuf, toalloc);
		if (clictx->urlbuf == NULL) {
			return -1;
		}
		clictx->urlptr = clictx->urlbuf + datasz;
		debug("realloced");
	}
	memcpy(clictx->urlptr, at, length);
	clictx->urlptr += length;
	*clictx->urlptr = 0;
	debug("on_url() %d (total %d)", (int)length, (int)(datasz + length));
	debug("on_url() %s", clictx->urlbuf);
	return 0;
}

static int on_status(http_parser * parser, const char * at, size_t length) {
	debug("on_status()");
	return 0;
}

static int on_header_field(http_parser * parser, const char * at, size_t length) {
	debug("on_header_field()");
	return 0;
}

static int on_header_value(http_parser * parser, const char * at, size_t length) {
	debug("on_header_value()");
	return 0;
}

static int on_headers_complete(http_parser * parser) {
	debug("on_headers_complete()");
	return 0;
}

static int on_body(http_parser * parser, const char * at, size_t length) {
	debug("on_body()");
	return 0;
}

static int on_message_complete(http_parser * parser) {
	struct httphandler_client * clictx = parser->data;
	clictx->finished = 1;
	if (parser->method == HTTP_GET) {
		debug("message was a GET");
	}
	debug("on_message_complete()");
	return 0;
}

static int on_chunk_header(http_parser * parser) {
	debug("on_chunk_header()");
	return 0;
}

static int on_chunk_complete(http_parser * parser) {
	debug("on_chunk_complete()");
	return 0;
}

int httphandler_init(http_parser_settings * settings) {
	if (settings == NULL) {
		settings = &http_settings;
	}
	memset(settings, 0, sizeof(http_parser_settings));
	settings->on_message_begin = on_message_begin;
	settings->on_url = on_url;
	settings->on_status = on_status;
	settings->on_header_field = on_header_field;
	settings->on_header_value = on_header_value;
	settings->on_headers_complete = on_headers_complete;
	settings->on_body = on_body;
	settings->on_message_complete = on_message_complete;
	settings->on_chunk_header = on_chunk_header;
	settings->on_chunk_complete = on_chunk_complete;
	return 0;
}

http_parser_settings * httphandler_getdefaultsettings() {
	return &http_settings;
}

#define READBUFFER_ALLOCATION_MULTIPLE 8192

struct httphandler_client * httphandler_new() {
	struct httphandler_client * client;
	client = malloc(sizeof(struct httphandler_client));
	if (client == NULL) {
		return NULL;
	}
	http_parser_init(&client->parser, HTTP_REQUEST);
	client->parser.data = client;

	client->urlbuf = malloc(URLBUFFER_ALLOCATION_MULTIPLE);
	if (client->urlbuf == NULL) {
		free(client);
		return NULL;
	}
	client->urlptr = client->urlbuf;
	client->urlbufsz = URLBUFFER_ALLOCATION_MULTIPLE;
	client->finished = 0;
	client->readbuf = malloc(READBUFFER_ALLOCATION_MULTIPLE);
	if (client->readbuf == NULL) {
		free(client->urlbuf);
		free(client);
		return NULL;
	}
	client->readbufsz = READBUFFER_ALLOCATION_MULTIPLE;
	client->state = HTTPHANDLER_WAITINGTOREAD;

	return client;
}

int httphandler_process(struct httphandler_client * client) {
	int res;
	int bytesParsed;

	if (client->state == HTTPHANDLER_WAITINGTOREAD) {
		
		res = client->read(client->readptr, client->readbuf, client->readbufsz);
	
		if (res <= 0) {
			debug("httphandler_process() closesocket()");
			client->state = HTTPHANDLER_WAITINGFORREUSE;
			return 0;
		}
	
		bytesParsed = http_parser_execute(&client->parser, &http_settings, client->readbuf, res);
	
		if (client->parser.upgrade) {
	
		} else if (bytesParsed != res) {
			debug("Something...");
		}
			
		if (client->parser.method == HTTP_POST) {
			//
		}
	
		if (client->finished) {
			debug("we finished this one");
			client->urlptr = client->urlbuf;
			client->finished = 0;
		}

		if (client->parser.method == HTTP_GET) {
			//
		}

		debug("parse complete... %d from %d", bytesParsed, res);
	}

	return 0;
}

int httphandler_destroy(struct httphandler_client * client) {
	free(client->readbuf);
	free(client->urlbuf);
	free(client);
	return 0;
}

