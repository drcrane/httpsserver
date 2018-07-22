#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#include <fcntl.h>

#include "socketctx.h"
#include "dbg.h"

int socketctx_createsocket(int port) {
	int s;
	int res;
	struct sockaddr_in addr;
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		return -1;
	}

	res = bind(s, (struct sockaddr *)&addr, sizeof(addr));
	if (res < 0) {
		return -1;
	}

	res = listen(s, 1);
	if (res < 0) {
		return -1;
	}

	return s;
}

// Very likely these do not do enough...
// TODO: fix it!

void socketctx_openssl_init() {
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
}

void socketctx_openssl_cleanup() {
	EVP_cleanup();
}

static SSL_CTX * socketctx_loadctxbyhostname(const char * hostname) {
	//
	return NULL;
}

static int socketctx_openssl_snicallback(SSL * ssl, int * int_ptr, void * void_ptr) {
	const char * hostname;
	hostname = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
	//fprintf(stdout, "client provided hostname %s\n", hostname);
	debug("socketctx_openssl_snicallback() client provided hostname %s", hostname);
	// Use this to change the context so it matches the hostname of
	// the certificate.
	SSL_CTX * ssl_ctx;
	ssl_ctx = socketctx_loadctxbyhostname(hostname);
	if (ssl_ctx != NULL) {
		SSL_set_SSL_CTX(ssl, ssl_ctx);
	}
	return 0;
}

SSL_CTX * socketctx_openssl_createcontext() {
	const SSL_METHOD * method;
	SSL_CTX * ctx;

	method = TLSv1_2_method();
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		return NULL;
	}
	return ctx;
}

int socketctx_openssl_configurecontext(SSL_CTX * ctx) {
	int res;

	SSL_CTX_set_ecdh_auto(ctx, 1);

	res = SSL_CTX_use_certificate_file(ctx, "server.cert", SSL_FILETYPE_PEM);
	if (res <= 0) {
		return -1;
	}
	res = SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM);
	if (res <= 0) {
		return -1;
	}
	res = SSL_CTX_set_tlsext_servername_callback(ctx, socketctx_openssl_snicallback);
	if (res <= 0) {
		return -1;
	}
	return 0;
}

struct socketctx_client * socketctx_openssl_accept_prepare(int svrsock, SSL_CTX * ctx) {
	struct socketctx_client * clictx;
	int clisock;
	int res;
	socklen_t len;
	SSL * ssl;

	ssl = SSL_new(ctx);
	if (ssl == NULL) {
		return NULL;
	}
	clictx = malloc(sizeof(struct socketctx_client));
	if (clictx == NULL) {
		goto error_freessl;
	}
	len = sizeof(clictx->addr);
	clisock = accept(svrsock, (struct sockaddr *)&clictx->addr, &len);
	if (clisock <= 0) {
		goto error_freeclictx;
	}

	int flags = fcntl(clisock, F_GETFL, 0);
	if (fcntl(clisock, F_SETFL, flags | O_NONBLOCK)) {
		debug("failed in fcntl()");
		close(clisock);
		goto error_freeclictx;
	}

	SSL_set_fd(ssl, clisock);

	clictx->sock = clisock;
	clictx->ssl = ssl;
	clictx->state = SOCKETCTX_CLIENT_STATE_ACCEPT;
	clictx->ionext = SOCKETCTX_CLIENT_IONEXT_NONE;
	return clictx;

error_freeclictx:
	free(clictx);
error_freessl:
	SSL_free(ssl);
	return NULL;
}

int socketctx_openssl_accept(struct socketctx_client * clictx) {
	int res;

	res = SSL_accept(clictx->ssl);
	switch(SSL_get_error((SSL *)clictx->ssl, res)) {
		case SSL_ERROR_NONE:
			clictx->state = SOCKETCTX_CLIENT_STATE_CONNECTED;
			res = 0;
			break;
		case SSL_ERROR_ZERO_RETURN:
			res = -1;
			break;
		case SSL_ERROR_WANT_READ:
			debug("SSL_ERROR_WANT_READ");
			clictx->ionext = SOCKETCTX_CLIENT_IONEXT_READ;
			res = 0;
			break;
		case SSL_ERROR_WANT_WRITE:
			debug("SSL_ERROR_WANT_WRITE");
			clictx->ionext = SOCKETCTX_CLIENT_IONEXT_WRITE;
			res = 0;
			break;
		default:
			debug("default");
			res = -1;
			break;
	}

	return res;
}

int socketctx_openssl_close(struct socketctx_client * clictx) {
	SSL_free(clictx->ssl);
	close(clictx->sock);
	free(clictx);
	return 0;
}

int socketctx_openssl_read(struct socketctx_client * clictx, char * ptr, size_t length) {
	int bytesRecvd;
	int res;
	bytesRecvd = SSL_read((SSL *)clictx->ssl, ptr, length);
	res = SSL_get_error((SSL *)clictx->ssl, bytesRecvd);
	switch (res) {
		case SSL_ERROR_NONE:
			break;
		case SSL_ERROR_ZERO_RETURN:
			// Client has gone away.
			return 0;
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			return -2;
		default:
			return -1;
	}
	return bytesRecvd;
}

int socketctx_openssl_write(SSL * ctx, char * ptr, size_t length) {
	size_t bw, res, sslerr;
	bw = 0;
try_again:
	res = SSL_write(ctx, ptr + bw, length - bw);
	if (res <= 0) {
		sslerr = SSL_get_error(ctx, res);
		switch (sslerr) {
			case SSL_ERROR_NONE:
				break;
			case SSL_ERROR_ZERO_RETURN:
				debug("connection closed");
				res = 0;
				goto end;
			default:
				res = -1;
				goto end;
		}
	} else {
		bw += res;
		if (bw == length) {
			res = 0;
			goto end;
		}
	}
	goto try_again;
end:
	return res;
}

int socketctx_openssl_sendfile(SSL * ctx, const char * filepath) {
	struct stat st;
	char * headerbuffer;
	uint64_t filesize;
	size_t bytesread;
	FILE * file;

	headerbuffer = malloc(8192);
	if (headerbuffer == NULL) {
		return -1;
	}
	stat(filepath + 1, &st);
	filesize = st.st_size;
	file = fopen(filepath + 1, "rb");
	if (file == NULL) {
		sprintf(headerbuffer, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: keep-alive\r\n\r\n");
		socketctx_openssl_write(ctx, headerbuffer, strlen(headerbuffer));
	} else {
		sprintf(headerbuffer, "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-Length: %llu\r\nConnection: keep-alive\r\n\r\n", (unsigned long long)filesize);
		socketctx_openssl_write(ctx, headerbuffer, strlen(headerbuffer));
		if (filesize < 8192) {
			bytesread = fread(headerbuffer, 1, filesize, file);
			if (bytesread != filesize) {
				debug("bytes read from file not right (%d vs %d)", (int)bytesread, (int)filesize);
			}
			socketctx_openssl_write(ctx, headerbuffer, bytesread);
		}
		fclose(file);
	}
	return 0;
}

