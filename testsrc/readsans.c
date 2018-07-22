#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

#include "dbg.h"

int main(int argc, char *argv[]) {

	BIO * bio = NULL;
	X509 * cert = NULL;
	void * certptr;

	FILE * cert_file = NULL;
	size_t cert_file_size, bytes_read;
	void * cert_file_buf = NULL;
	int res;

	cert_file = fopen("serverwsan.cert", "rb");
	if (cert_file == NULL) {
		debug("could not read file");
		return 1;
	}
	fseek(cert_file, 0, SEEK_END);
	cert_file_size = ftell(cert_file);
	fseek(cert_file, 0, SEEK_SET);
	cert_file_buf = malloc(cert_file_size);
	bytes_read = fread(cert_file_buf, 1, cert_file_size, cert_file);
	fclose(cert_file);
	cert_file = NULL;

	debug("read %d bytes", (int)bytes_read);
	if (bytes_read != cert_file_size) {
		return 1;
	}

	//

	bio = BIO_new_mem_buf(cert_file_buf, cert_file_size);
	PEM_read_bio_X509(bio, &cert, NULL, NULL);
	debug("read from bio: %d", cert ? "ERR" : "OK");
	BIO_free(bio);
	if (!cert) {
		return 1;
	}

	GENERAL_NAMES * names = NULL;
	char * utf8 = NULL;

	do {
		names = X509_get_ext_d2i(cert, NID_subject_alt_name, 0, 0);
		if (!names) {
			break;
		}

		int i, count;
		i = 0;
		count = sk_GENERAL_NAME_num(names);
		if (!count) {
			break;
		}
		debug("found %d names", count);
		fflush(stderr);

		for (i = 0; i < count; i ++) {
			GENERAL_NAME * entry = sk_GENERAL_NAME_value(names, i);
			if (!entry) {
				continue;
			}
			if (entry->type == GEN_DNS) {
				int len1 = 0, len2 = -1;
				len1 = ASN1_STRING_to_UTF8((unsigned char **)&utf8, entry->d.dNSName);
				if (utf8) {
					len2 = (int)strlen((const char *)utf8);
				}
				if (len1 != len2) {
					fprintf(stderr, "Problem with ASN1 and UTF8 string length\n");
				}
				if (utf8 && len1 && len2 && (len1 == len2)) {
					fprintf(stdout, " - %s\n", utf8);
				}
				if (utf8) {
					OPENSSL_free(utf8);
					utf8 = NULL;
				}
			}
		}
	} while(0);

	if (names) {
		GENERAL_NAMES_free(names);
	}

	if (utf8) {
		OPENSSL_free(utf8);
	}

	//
}


