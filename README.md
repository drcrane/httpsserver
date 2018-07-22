Simple HTTPS Server Supporting SNI
==================================

Cloning this Repository
-----------------------

This repository uses the `http-parser` library from node, it is
included as a sub-module so has to be cloned separately.

    $ git submodule init     # register the Git submodule in .git/config
    $ $EDITOR .git/config    # if you want to specify a different url for git
    $ git submodule update   # clone/fetch and checkout http-parser

Quick Start
-----------

To send a server name using SNI with openssl s_client:

    openssl s_client -connect localhost:4433 -servername myserver.com

Simples.

Reading certificates from file:

    SSL_CTX_use_certificate

For reading the certificate from a server or the client (peers)...

    SSL_get_peer_certificate
    d2i_X509
    PEM_read_bio_X509

These should be enough to get going.

Take the public key out of the pem certificate file and place it in
a separate pem file.

    openssl x509 -inform PEM -in cert.pem -pubkey -noout > publickey.pem

Getting a public key from a file:

[certpubkey.html](http://fm4dd.com/openssl/certpubkey.htm)

