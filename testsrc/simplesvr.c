#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stddef.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "socketctx.h"
#include "dbg.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

fd_set socks_read;
fd_set socks_write;

int main(int argc, char *argv[]) {
	int sock;
	int maxfd;
	int res;
	struct socketctx_client * client;
	SSL_CTX * ctx;
	struct timeval tv;

	socketctx_openssl_init();
	ctx = socketctx_openssl_createcontext();
	check_debug(ctx, "context initialisation failed");

	res = socketctx_openssl_configurecontext(ctx);
	check_debug(res == 0, "could not configure context");

	sock = socketctx_createsocket(4443);
	check_debug(sock > 0, "failed to create socket");

	FD_ZERO(&socks_read);
	FD_ZERO(&socks_write);

	client = NULL;

next_activity:
	maxfd = sock;
	FD_SET(sock, &socks_read);
	if (client != NULL) {
		int clisock = SOCKETCTX_CLIENT_GETSOCKET(client);
		if (client->ionext == ) {
			FD_SET(clisock, &socks_read);
			maxfd = MAX(clisock, maxfd);
		}
		if (client->ionext == ) {
			FD_SET(clisock, &socks_write);
			maxfd = MAX(clisock, maxfd);
		}
	}

	maxfd = MAX(sock, maxfd);
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	res = select(maxfd + 1, &socks_read, &socks_write, NULL, &tv);

	if (res == -1) {
		debug("select() error!");
	} else if (res) {
		// There has been some activity!
		debug("select() returned %d", res);
	} else {
		debug("timeout!");
		goto next_activity;
	}

	if (FD_ISSET(sock, &socks_read)) {
		//client = acceptclientconnection(sock, ctx);
		FD_CLR(sock, &socks_read);
	}

	if (client != NULL) {
		int clisock = SOCKETCTX_CLIENT_GETSOCKET(client);
		if (FD_ISSET(clisock, &socks_read)) {
			//httphandler_process(client);
			FD_CLR(clisock, &socks_read);
			if (client->state == HTTPHANDLER_WAITINGFORREUSE) {
				socketctx_openssl_close((struct socketctx_client *)client->readptr);
				//httphandler_destroy(client);
				client = NULL;
				debug("destroyed client");
			}
		}
	}

	debug("...");
	sleep(1);

	goto next_activity;

	close(sock);

	socketctx_openssl_cleanup();
	return 0;
error:
	return 1;
}

