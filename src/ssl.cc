extern "C" {
#include <stdio.h>
#include <stdlib.h>

#include <lua5.2/lua.h>
#include <lua5.2/lualib.h>
#include <lua5.2/lauxlib.h>

#include <openssl/err.h>
#include <openssl/dh.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
}

/* SSL debug */
#define SSL_WHERE_INFO(ssl, w, flag, msg) {                \
    if(w & flag) {                                         \
      printf("+ %s: ", name);                              \
      printf("%20.20s", msg);                              \
      printf(" - %40.40s ", SSL_state_string_long(ssl));   \
      printf(" - %5.10s ", SSL_state_string(ssl));         \
      printf("\n");                                        \
    }                                                      \
  }

typedef void(*info_callback)();

namespace {
	// C/C++ functions
	// debug
	void ssl_server_info_callback(const SSL* ssl, int where, int ret) {
		krx_ssl_info_callback(ssl, where, ret, "server");
	}

	void ssl_client_info_callback(const SSL* ssl, int where, int ret) {
		krx_ssl_info_callback(ssl, where, ret, "client");
	}

	void ssl_info_callback(const SSL* ssl, int where, int ret, const char* name) {

		if(ret == 0) {
			printf("-- krx_ssl_info_callback: error occured.\n");
			return;
		}

		SSL_WHERE_INFO(ssl, where, SSL_CB_LOOP, "LOOP");
		SSL_WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_START, "HANDSHAKE START");
		SSL_WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_DONE, "HANDSHAKE DONE");
	}

	// implementation
	void initOpensslLib() {
		SSL_library_init();
		SSL_load_error_strings();
		ERR_load_BIO_strings();
		OpenSSL_add_all_algorithms();
	}

	int ssl_verify_peer(int ok, X509_STORE_CTX* ctx) {
		return 1;
	}

	void cleanupOpensslLib() {
		ERR_remove_state(0);
		ENGINE_cleanup();
		CONF_modules_unload(1);
		ERR_free_strings();
		EVP_cleanup();
		sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
		CRYPTO_cleanup_all_ex_data();
	}

	int initSSLContext(SSL_CTX* ctx) {
		int r = 0;

  		/* create a new context using DTLS */
		ctx = SSL_CTX_new(DTLSv1_method());
		if(!ctx) {
			printf("Error: cannot create SSL_CTX.\n");
			ERR_print_errors_fp(stderr);
    		return -1;
		}

		/* set our supported ciphers */
		r = SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
		if(r != 1) {
			printf("Error: cannot set the cipher list.\n");
			ERR_print_errors_fp(stderr);
			return -2;
  		}

		/* the client doesn't have to send it's certificate */
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, ssl_verify_peer);

		/* enable srtp */
		r = SSL_CTX_set_tlsext_use_srtp(ctx, "SRTP_AES128_CM_SHA1_80");
		if(r != 0) {
			printf("Error: cannot setup srtp.\n");
			ERR_print_errors_fp(stderr);
			return -3;
		}
		/* load key and certificate */
		char certfile[1024];
		char keyfile[1024];
		sprintf(certfile, "./server-cert.pem");
		sprintf(keyfile, "./server-key.pem");

		/* certificate file; contains also the public key */
		r = SSL_CTX_use_certificate_file(ctx, certfile, SSL_FILETYPE_PEM);
		if(r != 1) {
			printf("Error: cannot load certificate file.\n");
			ERR_print_errors_fp(stderr);
			return -4;
		}

		/* load private key */
		r = SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM);
		if(r != 1) {
			printf("Error: cannot load private key file.\n");
			ERR_print_errors_fp(stderr);
			return -5;
		}

		/* check if the private key is valid */
		r = SSL_CTX_check_private_key(ctx);
		if(r != 1) {
			printf("Error: checking the private key failed. \n");
			ERR_print_errors_fp(stderr);
			return -6;
		}
		return 0;
	}

	void freeSSLContext(SSL_CTX* ctx) {
		SSL_CTX_free(ctx);
	}

	int createSSL(SSL_CTX* ctx, SSL* ssl) {
		ssl = SSL_new(ctx);
		if(!k->ssl) {
			printf("Error: cannot create new SSL*.\n");
			return -1;
		}
		return 0;
	}

	void freeSSL(SSL* ssl) {
		SSL_free(ssl);
	}

	int newBIO(BIO* bio, BIO_METHOD* type) {
		bio = BIO_new(type);
		if(bio == NULL) {
			printf("Error: cannot allocate read bio.\n");
			return -2;
		}
		if (type == BIO_s_mem()) {
			BIO_set_mem_eof_return(bio, -1);
		}
		return 0;
	}

	void setInfoCallbackForSSL(SSL* ssl, info_callback cb) {
		SSL_set_info_callback(ssl, cb);
	}

	void setBIOSForSSL(SSL* ssl, BIO* bioIn, BIO* bioOut) {
		SSL_set_bio(ssl, bioIn, bioOut);
	}

	void prepareSSLToHandshake(SSL* ssl, int isServer) {
		if (isServer == 1) {
			SSL_set_accept_state(ssl);
		}
		else {
			SSL_set_connect_state(ssl);
		}
	}

	int isHandshakeFinished(SSL* ssl) {
		return SSL_is_init_finished(ssl);
	}
	int doHandshake(SSL* ssl) {
		return SSL_do_handshake(ssl);
	}

	int getSSLErrorInfo(const SSL* ssl, int returnOfSSLOperation) {
		return SSL_get_error(ssl, returnOfSSLOperation);
	}

	int checkDecriptedDataInSSL(SSL* ssl) {
		return SSL_pending(ssl);
	}

	int readDecriptedDataFromSSL(SSL* ssl, char* buf int bufSize) {
		return SSL_read(ssl, buf, bufSize);
	}

	int writeEncriptedDataIntoSSL(SSL* ssl, char* buf int bufSize) {
		return SSL_write(ssl, buf, bufSize);
	}

	int checkDataToReadInBIO(BIO* bio) {
		return BIO_ctrl_pending(bio);
	}

	int readDatafromBIO(BIO* bio, char* buf int bufSize) {
		return BIO_read(bio, buf, bufSize);
	}

	int writeDataIntoBIO(BIO* bio, char* buf int bufSize) {
		return BIO_write(bio, buf, bufSize);
	}

	// Lua interfaces
	//OPENSSL
	int openssl_initSslLib(lua_State* L) {
		initOpensslLib();
		return 0;
	}

	int openssl_cleanupSslLib(lua_State* L) {
		cleanupOpensslLib();
		return 0;
	}

	int openssl_prepareSslContext(lua_State* L) {
		SSL_CTX* ctx = (SSL_CTX*)lua_newuserdata(L, sizeof(SSL_CTX));
		luaL_getmetatable(L, "openssl.ssl_ctx");
		lua_setmetatable(L, -2);

		if(initSSLContext(ctx) < 0) {
			exit(EXIT_FAILURE);
		}
		return 1;
	}

	int openssl_createBio(lua_State* L) {
		enum BIOTypes {
			SOURCE_BIO = 0,
			FILTER_BIO = 1
		};

		BIOTypes bioType = (BIOTypes)luaL_checknumber(L, 1);
		BIO_METHOD* methodType;

		switch (type) {
			case SOURCE_BIO:
				methodType = BIO_s_mem();
				break;
			case FILTER_BIO:
				methodType = BIO_f_ssl();
				break;
			default:
				exit(EXIT_FAILURE);
		}

		BIO* bio = (BIO*)lua_newuserdata(L, sizeof(BIO));

		if(newBIO(bio, methodType) < 0) {
			exit(EXIT_FAILURE);
		}
		return 1;
	}

	//SSL_CTX
	int ctx_newSsl(lua_State* L) {
		SSL_CTX* ctx = (SSL_CTX*)luaL_checkudata(L, 1, "openssl.ssl_ctx");
		SSL* ssl = (SSL*)lua_newuserdata(L, sizeof(SSL));
		luaL_getmetatable(L, "openssl.ssl");
		lua_setmetatable(L, -2);

		if(createSSL(ctx, ssl) < 0) {
			exit(EXIT_FAILURE);
		}
		return 1;
	}

	int ctx_free(lua_State* L) {
		SSL_CTX* ctx = (SSL_CTX*)luaL_checkudata(L, 1, "openssl.ssl_ctx");
		freeSSLContext(ctx);
		return 0;
	}

	//SSL
	int ssl_setInfoCallback(lua_State* L) {
		SSL* ssl = (SSL*)luaL_checkudata(L, 1, "openssl.ssl");
		setInfoCallbackToSSL(ssl, ssl_server_info_callback);
		return 0;
	}

	int ssl_setBios(lua_State* L) {
		SSL* ssl = (SSL*)luaL_checkudata(L, 1, "openssl.ssl");
		BIO* bioIn = (BIO*)luaL_checkudata(L, 2, "openssl.bio");
		BIO* bioOut = (BIO*)luaL_checkudata(L, 3, "openssl.bio");
		setBIOSForSSL(SSL* ssl, BIO* bioIn, BIO* bioOut)
		return 0;
	}

	int ssl_prepareToHandshake(lua_State* L) {
		SSL* ssl = (SSL*)luaL_checkudata(L, 1, "openssl.ssl");
		int isServer = luaL_checknumber(L, 2);
		prepareSSLToHandshake(int isServer)
	}

	int ssl_isHandshakeFinished(lua_State* L) {
		SSL* ssl = (SSL*)luaL_checkudata(L, 1, "openssl.ssl");
		int isFinished = isHandshakeFinished(ssl);
		lua_pushboolean(L, isFinished);
		return 1;
	}

	int ssl_performHandshake(lua_State* L) {
		SSL* ssl = (SSL*)luaL_checkudata(L, 1, "openssl.ssl");
		int result = doHandshake(ssl);
		int info = getSSLErrorInfo(ssl, result);
		lua_pushinteger(L, result);
		lua_pushinteger(L, info);
		return 2;
	}

	int ssl_checkDataToDecript(lua_State* L) {
		SSL* ssl = (SSL*)luaL_checkudata(L, 1, "openssl.ssl");
		int datSize = checkDecriptedDataInSSL(ssl);
		lua_pushinteger(L, datSize);
		return 1;
	}

	int ssl_decrypt(lua_State* L) {
		SSL* ssl = (SSL*)luaL_checkudata(L, 1, "openssl.ssl");
		int size = luaL_checknumber(L, 2);
		char* buffer = new char[size];
		int readSize = readDecriptedDataFromSSL(ssl, buffer, size);
		lua_pushlstring(L, buffer, readSize);
		delete[] buffer;
		return 1;
	}

	int ssl_encrypt(lua_State* L) {
		SSL* ssl = (SSL*)luaL_checkudata(L, 1, "openssl.ssl");
		size_t size;
		const char* data = luaL_checklstring(L, 2, &size);
		int writeSize = writeEncriptedDataIntoSSL(ssl, data, size);
		return 0;
	}

	int ssl_free(lua_State* L) {
		SSL* ssl = (SSL*)luaL_checkudata(L, 1, "openssl.ssl");
		freeSSL(ssl);
		return 0;
	}

	//BIO
	int bio_checkDataToRead(lua_State* L) {
		BIO* bio = (BIO*)luaL_checkudata(L, 1, "openssl.bio");
		int datSize = checkDataToReadInBIO(bio);
		lua_pushinteger(L, datSize);
		return 1;
	}

	int bio_readData(lua_State* L) {
		BIO* bio = (BIO*)luaL_checkudata(L, 1, "openssl.bio");
		int size = luaL_checknumber(L, 2);
		char* buffer = new char[size];
		int readSize = readDatafromBIO(bio, buffer, size);
		lua_pushlstring(L, buffer, readSize);
		delete[] buffer;
		return 1;
	}

	int bio_writeData(lua_State* L) {
		BIO* bio = (BIO*)luaL_checkudata(L, 1, "openssl.bio");
		size_t size;
		const char* data = luaL_checklstring(L, 2, &size);
		int writeSize = writeDataIntoBIO(bio, data, size);
		return 0;
	}

}

extern "C"
int luaopen_openssl(lua_State *L) {
	luaL_Reg openssl_functions[] = {
		{ "initSslLibrary", &openssl_initSslLib },
		{ "prepareSslContext", &openssl_prepareSslContext },
		{ "createBio", &openssl_createBio },
		{ "cleanupSslLibrary", &openssl_cleanupSslLib },
		{ NULL, NULL }
	};

	//SSL_CTX
	luaL_newmetatable(L, "openssl.ssl_ctx");
	lua_newtable(L);
		luaL_Reg ctx_functions[] = {
		{ "newSsl", &ctx_newSsl },
		{ NULL, NULL }
	};
	luaL_setfuncs(L, ctx_functions, 0);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, ctx_free);
	lua_setfield(L, -2, "__gc");

	//SSL
	luaL_newmetatable(L, "openssl.ssl");
	lua_newtable(L);
		luaL_Reg ssl_functions[] = {
		{ "setInfoCallback", &ssl_setInfoCallback },
		{ "setBios", &ssl_setBios },
		{ "prepareToHandshake", &ssl_prepareToHandshake },
		{ "performHandshake", &ssl_performHandshake },
		{ "isHandshakeFinished", &ssl_isHandshakeFinished },
		{ "checkData", &ssl_checkDataToDecript },
		{ "decrypt", &ssl_decrypt },
		{ "encrypt", &ssl_encrypt },
		{ NULL, NULL }
	};
	luaL_setfuncs(L, ssl_functions, 0);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, ssl_free);
	lua_setfield(L, -2, "__gc");

	//BIO
	luaL_newmetatable(L, "openssl.bio");
	lua_newtable(L);
		luaL_Reg bio_functions[] = {
		{ "checkData", &bio_checkDataToRead },
		{ "read", &bio_readData },
		{ "write", &bio_writeData },
		{ NULL, NULL }
	};
	luaL_setfuncs(L, bio_functions, 0);
	lua_setfield(L, -2, "__index");

	//OPENSSL
	luaL_newlib(L, openssl_functions);
	return 1;
}