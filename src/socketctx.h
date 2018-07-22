#ifndef SOCKETCTX_H
#define SOCKETCTX_H

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SOCKETCTX_CLIENT_BUFSZ 1024

#define SOCKETCTX_CLIENT_GETSOCKET(X) (((struct socketctx_client *)X)->sock)

#define SOCKETCTX_CLIENT_STATE_BEGIN     0
#define SOCKETCTX_CLIENT_STATE_ACCEPT    1
#define SOCKETCTX_CLIENT_STATE_CONNECTED 2
#define SOCKETCTX_CLIENT_STATE_IO        4
#define SOCKETCTX_CLIENT_STATE_CLOSE     8

#define SOCKETCTX_CLIENT_IONEXT_NONE  0
#define SOCKETCTX_CLIENT_IONEXT_READ  1
#define SOCKETCTX_CLIENT_IONEXT_WRITE 2

struct socketctx_client {
	void * ssl;
	int sock;
	int state;
	int ionext;

	char buf[SOCKETCTX_CLIENT_BUFSZ];
	int urlbufsz;
	char * urlbuf;
	char * urlptr;
	int reqtype;
	int finished;
	struct sockaddr_in6 addr;
};

int socketctx_createsocket(int port);

void socketctx_openssl_init();

void socketctx_openssl_cleanup();

SSL_CTX * socketctx_openssl_createcontext();

int socketctx_openssl_sendfile(SSL * ctx, const char * filepath);

int socketctx_openssl_configurecontext(SSL_CTX * ctx);

struct socketctx_client * socketctx_openssl_accept_prepare(int svrsock, SSL_CTX * ctx);
int socketctx_openssl_accept(struct socketctx_client * clictx);
int socketctx_openssl_close(struct socketctx_client * clictx);
int socketctx_openssl_read(struct socketctx_client * clictx, char * ptr, size_t length);

#endif // SOCKETCTX_H

