/*
 * File: tls.c
 *
 * Update: Modified to use OpenSSL directly, without the mbed SSL dependency.
 * Original source copyright was (used as a template for the current code):
 *
 * Copyright (C) 2011 Benjamin Johnson <obeythepenguin@users.sourceforge.net>
 * (for the https code offered from dplus browser that formed the basis...)
 * Copyright 2016 corvid
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 */

/*
 * https://www.ssllabs.com/ssltest/viewMyClient.html
 * https://badssl.com
 *
 * Using TLS in Applications: https://datatracker.ietf.org/wg/uta/documents/
 * TLS: https://datatracker.ietf.org/wg/tls/documents/
 */

#include "config.h"
#include "../msg.h"

#ifndef ENABLE_SSL

void a_Tls_init()
{
   MSG("TLS: Disabled at compilation time.\n");
}

#else

#include <assert.h>
#include <errno.h>

#include "../../dlib/dlib.h"
#include "../dialog.hh"
#include "../klist.h"
#include "iowatch.hh"
#include "tls.h"
#include "Url.h"

#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>

#define CERT_STATUS_NONE 0
#define CERT_STATUS_RECEIVING 1
#define CERT_STATUS_CLEAN 2
#define CERT_STATUS_BAD 3
#define CERT_STATUS_USER_ACCEPTED 4

typedef struct {
   char *hostname;
   int port;
   int cert_status;
} Server_t;

typedef struct {
   char *name;
   Dlist *servers;
} CertAuth_t;

typedef struct {
   int fd;
   int connkey;
} FdMapEntry_t;

/*
 * Data type for TLS connection information
 */
typedef struct {
   int fd;
   DilloUrl *url;
   SSL *ssl;
   bool_t handshaked;
   bool_t connecting;
} Conn_t;

/* List of active TLS connections */
static Klist_t *conn_list = NULL;

/* Shared TLS context */
SSL_CTX *ssl_context = NULL;

static bool_t ssl_enabled = TRUE;

static Dlist *servers;
static Dlist *fd_map;

static void Tls_handshake_cb(int fd, void *vconnkey);

/*
 * Compare by FD.
 */
static int Tls_fd_map_cmp(const void *v1, const void *v2)
{
   int fd = VOIDP2INT(v2);
   const FdMapEntry_t *e = v1;

   return (fd != e->fd);
}

static void Tls_fd_map_add_entry(int fd, int connkey)
{
   FdMapEntry_t *e = dNew0(FdMapEntry_t, 1);
   e->fd = fd;
   e->connkey = connkey;

   if (dList_find_custom(fd_map, INT2VOIDP(e->fd), Tls_fd_map_cmp)) {
      MSG_ERR("TLS FD ENTRY ALREADY FOUND FOR %d\n", e->fd);
      assert(0);
   }

   dList_append(fd_map, e);
//MSG("ADD ENTRY %d %s\n", e->fd, URL_STR(sd->url));
}

/*
 * Remove and free entry from fd_map.
 */
static void Tls_fd_map_remove_entry(int fd)
{
   void *data = dList_find_custom(fd_map, INT2VOIDP(fd), Tls_fd_map_cmp);

//MSG("REMOVE ENTRY %d\n", fd);
   if (data) {
      dList_remove_fast(fd_map, data);
      dFree(data);
   } else {
      MSG("TLS FD ENTRY NOT FOUND FOR %d\n", fd);
   }
}

/*
 * Return TLS connection information for a given file
 * descriptor, or NULL if no TLS connection was found.
 */
void *a_Tls_connection(int fd)
{
   Conn_t *conn;

   if (fd_map) {
      FdMapEntry_t *fme = dList_find_custom(fd_map, INT2VOIDP(fd),
                                            Tls_fd_map_cmp);

      if (fme && (conn = a_Klist_get_data(conn_list, fme->connkey)))
         return conn;
   }
   return NULL;
}

/*
 * Add a new TLS connection information node.
 */
static Conn_t *Tls_conn_new(int fd, const DilloUrl *url,
                            SSL *ssl)
{
   Conn_t *conn = dNew0(Conn_t, 1);
   conn->fd = fd;
   conn->url = a_Url_dup(url);
   conn->ssl = ssl;
   conn->handshaked = FALSE;
   conn->connecting = TRUE;
   return conn;
}

static int Tls_make_conn_key(Conn_t *conn)
{
   int key = a_Klist_insert(&conn_list, conn);

   Tls_fd_map_add_entry(conn->fd, key);

   return key;
}

/*
 * Load certificates from a given filename.
 */
static void Tls_load_certificates_from_file(SSL_CTX *ssl_context, const char *const filename)
{
   int ret = SSL_CTX_load_verify_locations(ssl_context, filename, NULL);

   if (ret == 0) {
      MSG("Failed to parse certificates from %s\n", filename);
   }
}

/*
 * Load certificates from a given pathname.
 */
static void Tls_load_certificates_from_path(SSL_CTX *ssl_context, const char *const pathname)
{
   int ret = SSL_CTX_load_verify_locations(ssl_context, NULL, pathname);

   if (ret == 0) {
      MSG("Failed to parse certificates from %s\n", pathname);
   }
}

/*
 * Load trusted certificates.
 */
static void Tls_load_certificates(SSL_CTX *ssl_context)
{
   /* curl-7.37.1 says that the following bundle locations are used on "Debian
    * systems", "Redhat and Mandriva", "old(er) Redhat", "FreeBSD", and
    * "OpenBSD", respectively -- and that the /etc/ssl/certs/ path is needed on
    * "SUSE". No doubt it's all changed some over time, but this gives us
    * something to work with.
    */
   uint_t u;
   char *userpath;
   //X509 *curr;

   static const char *const ca_files[] = {
      "/etc/ssl/certs/ca-certificates.crt",
      "/etc/pki/tls/certs/ca-bundle.crt",
      "/usr/share/ssl/certs/ca-bundle.crt",
      "/usr/local/share/certs/ca-root.crt",
      "/etc/ssl/cert.pem",
      CA_CERTS_FILE
   };

   static const char *const ca_paths[] = {
      "/etc/ssl/certs/",
      CA_CERTS_DIR
   };

   for (u = 0; u < sizeof(ca_files)/sizeof(ca_files[0]); u++) {
      if (*ca_files[u])
         Tls_load_certificates_from_file(ssl_context, ca_files[u]);
   }

   for (u = 0; u < sizeof(ca_paths)/sizeof(ca_paths[0]); u++) {
      if (*ca_paths[u]) {
         Tls_load_certificates_from_path(ssl_context, ca_paths[u]);
      }
   }

   userpath = dStrconcat(dGethomedir(), "/.dillo/certs/", NULL);
   Tls_load_certificates_from_path(ssl_context, userpath);
   dFree(userpath);

   MSG("Loaded TLS certificates.\n");
}

/*
 * Select safe ciphersuites.
 */
static void Tls_set_cipher_list(SSL_CTX *ssl_context)
{

#if 0

   /* Very strict cipher list */
   
   const char *cipher_list = "EECDH+AESGCM+AES128:EECDH+AESGCM+AES256:EECDH+CHACHA20:EDH+AESGCM+AES128:EDH+AESGCM+AES256:EDH+CHACHA20:EECDH+SHA256+AES128:EECDH+SHA384+AES256:EDH+SHA256+AES128:EDH+SHA256+AES256:EECDH+SHA1+AES128:EECDH+SHA1+AES256:EDH+SHA1+AES128:EDH+SHA1+AES256:EECDH+HIGH:EDH+HIGH:AESGCM+AES128:AESGCM+AES256:CHACHA20:SHA256+AES128:SHA256+AES256:SHA1+AES128:SHA1+AES256:HIGH:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK:!KRB5:!aECDH";

#else

   /* Less strict cipher list, just exclude:
    * eNULL, which has no encryption
    * aNULL, which has no authentication
    * LOW, which as of 2014 use 64 or 56-bit encryption
    * EXPORT40, which uses 40-bit encryption
    * RC4, for which methods were found in 2013 to defeat it somewhat too easily
    * 3DES, not very secure nowadays
    * MD5, broken hash function
    * PSK, shared key algorithms
    * KRB5, kerberos is not needed
    * aECDH, anonymous Elliptic Curve Diffie Hellman cipher suites
    */
   
   const char *cipher_list = "ALL:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK:!KRB5:!aECDH";

#endif

 SSL_CTX_set_cipher_list(ssl_context, cipher_list);
}

/*
 * Initialize a new TLS context.
 */
static SSL_CTX * Tls_context_new(void)
{
   SSL_CTX *ssl_context;
   
   /* Create TLS context */
   ssl_context = SSL_CTX_new(SSLv23_client_method());
   if (ssl_context == NULL){
      MSG("Error creating SSL context\n");
      return NULL;
   }
   
   /* SSL2 has been known to be insecure forever, disabling SSL3 is in response
    * to POODLE, and disabling compression is in response to CRIME. */
   SSL_CTX_set_options(ssl_context, SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3|SSL_OP_NO_COMPRESSION);

   /* Load trusted certificates */
   Tls_load_certificates(ssl_context);

   /* Set safe ciphersuites */
   Tls_set_cipher_list(ssl_context);

   return ssl_context;
}

/*
 * Initialize the TLS library.
 */
void a_Tls_init(void)
{
   /* Initialize library */
   SSL_load_error_strings();
   SSL_library_init();

   /* Initialize entropy */
   if (RAND_status() != 1){
      /*Insufficient entropy.  Deal with it?*/
      MSG("Insufficient random entropy.\n");
   }

   /* Initialize global TLS context */
   ssl_context = Tls_context_new();

   /* Initialize global lists */
   fd_map = dList_new(20);
   servers = dList_new(8);
}

/*
 * Ordered comparison of servers.
 */
static int Tls_servers_cmp(const void *v1, const void *v2)
{
   const Server_t *s1 = (const Server_t *)v1, *s2 = (const Server_t *)v2;
   int cmp = dStrAsciiCasecmp(s1->hostname, s2->hostname);

   if (!cmp)
      cmp = s1->port - s2->port;
   return cmp;
}
/*
 * Ordered comparison of server with URL.
 */
static int Tls_servers_by_url_cmp(const void *v1, const void *v2)
{
   const Server_t *s = (const Server_t *)v1;
   const DilloUrl *url = (const DilloUrl *)v2;

   int cmp = dStrAsciiCasecmp(s->hostname, URL_HOST(url));

   if (!cmp)
      cmp = s->port - URL_PORT(url);
   return cmp;
}

/*
 * The purpose here is to permit a single initial connection to a server.
 * Once we have the certificate, know whether we like it -- and whether the
 * user accepts it -- HTTP can run through queued sockets as normal.
 *
 * Return: TLS_CONNECT_READY or TLS_CONNECT_NOT_YET or TLS_CONNECT_NEVER.
 */
int a_Tls_connect_ready(const DilloUrl *url)
{
   Server_t *s;
   int ret = TLS_CONNECT_READY;

   dReturn_val_if_fail(ssl_enabled, TLS_CONNECT_NEVER);

   if ((s = dList_find_sorted(servers, url, Tls_servers_by_url_cmp))) {
      if (s->cert_status == CERT_STATUS_RECEIVING)
         ret = TLS_CONNECT_NOT_YET;
      else if (s->cert_status == CERT_STATUS_BAD)
         ret = TLS_CONNECT_NEVER;

      if (s->cert_status == CERT_STATUS_NONE)
         s->cert_status = CERT_STATUS_RECEIVING;
   } else {
      s = dNew(Server_t, 1);

      s->hostname = dStrdup(URL_HOST(url));
      s->port = URL_PORT(url);
      s->cert_status = CERT_STATUS_RECEIVING;
      dList_insert_sorted(servers, s, Tls_servers_cmp);
   }
   return ret;
}

static int Tls_cert_status(const DilloUrl *url)
{
   Server_t *s = dList_find_sorted(servers, url, Tls_servers_by_url_cmp);

   return s ? s->cert_status : CERT_STATUS_NONE;
}

/*
 * Did we find problems with the certificate, and did the user proceed to
 * reject the connection?
 */
static int Tls_user_said_no(const DilloUrl *url)
{
   return Tls_cert_status(url) == CERT_STATUS_BAD;
}

/*
 * Did everything seem proper with the certificate -- no warnings to
 * click through?
 */
int a_Tls_certificate_is_clean(const DilloUrl *url)
{
   return Tls_cert_status(url) == CERT_STATUS_CLEAN;
}

/*
 * Generate dialog msg for expired cert.
 */
static void Tls_cert_expired(const X509 *cert, Dstr *ds)
{
   const ASN1_TIME *notAfter = X509_get_notAfter(cert);

   int year = (notAfter->data[ 0] - '0') * 10 + (notAfter->data[ 1] - '0') + 100;
   int mon  = (notAfter->data[ 2] - '0') * 10 + (notAfter->data[ 3] - '0') - 1;
   int mday = (notAfter->data[ 4] - '0') * 10 + (notAfter->data[ 5] - '0');
   int hour = (notAfter->data[ 6] - '0') * 10 + (notAfter->data[ 7] - '0');
   int min  = (notAfter->data[ 8] - '0') * 10 + (notAfter->data[ 9] - '0');
   int sec  = (notAfter->data[10] - '0') * 10 + (notAfter->data[11] - '0');
   
   dStr_sprintfa(ds,"Certificate expired at: %04d/%02d/%02d %02d:%02d:%02d.\n",
                 year, mon, mday, hour, min, sec);
}

/*
 * Generate dialog msg when certificate is not for this host.
 */
static void Tls_cert_cn_mismatch(const X509 *cert, Dstr *ds)
{
   char *subj;

   dStr_append(ds, "This host is not one of the hostnames listed on the TLS "
                   "certificate that it sent:\n");

   subj = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);

   dStr_append(ds, subj);
   dStr_append(ds, "\n");

   OPENSSL_free(subj);
}

/*
 * Generate dialog msg when certificate is not trusted.
 */
static void Tls_cert_trust_chain_failed(const X509 *cert, Dstr *ds)
{
   char *issuer;

   issuer = X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0);

   dStr_sprintfa(ds, "Couldn't reach any trusted root certificate from "
                     "supplied certificate. The issuer of the certificate was:\n"
                     "%s\n", issuer);

   OPENSSL_free(issuer);
}

/*
 * Generate dialog msg when certificate start date is in the future.
 */
static void Tls_cert_not_valid_yet(const X509 *cert, Dstr *ds)
{
   const ASN1_TIME *notBefore = X509_get_notBefore(cert);

   int year = (notBefore->data[ 0] - '0') * 10 + (notBefore->data[ 1] - '0') + 100;
   int mon  = (notBefore->data[ 2] - '0') * 10 + (notBefore->data[ 3] - '0') - 1;
   int mday = (notBefore->data[ 4] - '0') * 10 + (notBefore->data[ 5] - '0');
   int hour = (notBefore->data[ 6] - '0') * 10 + (notBefore->data[ 7] - '0');
   int min  = (notBefore->data[ 8] - '0') * 10 + (notBefore->data[ 9] - '0');
   int sec  = (notBefore->data[10] - '0') * 10 + (notBefore->data[11] - '0');

   dStr_sprintfa(ds, "Certificate validity begins in the future at: "
                     "%04d/%02d/%02d %02d:%02d:%02d.\n",
                     year, mon, mday, hour, min, sec);
}

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
int get_cert_algorithm(const X509 *cert)
{
    ASN1_OBJECT *ppkalg;
    X509_PUBKEY *pubkey = X509_get_X509_PUBKEY(cert);
    X509_PUBKEY_get0_param(&ppkalg, NULL, NULL, NULL, pubkey);
    return OBJ_obj2nid(ppkalg);
}
#endif

/*
 * Generate dialog msg when certificate hash algorithm is not accepted.
 */
static void Tls_cert_bad_hash(const X509 *cert, Dstr *ds)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
   int pkey_nid = get_cert_algorithm(cert);
#else
   int pkey_nid = OBJ_obj2nid(cert->cert_info->key->algor->algorithm);
#endif
   const char* hash;

   if (pkey_nid == NID_undef) {
      hash = "Unrecognized";
   } else {
      hash = OBJ_nid2ln(pkey_nid);
   }
 
   dStr_sprintfa(ds, "This certificate's hash algorithm is not accepted "
                     "(%s).\n", hash);
}

/*
 * Generate dialog msg when public key algorithm (RSA, ECDSA) is not accepted.
 */
static void Tls_cert_bad_pk_alg(const X509 *cert, Dstr *ds)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
   int pubkey_algonid = get_cert_algorithm(cert);
#else
   int pubkey_algonid = OBJ_obj2nid(cert->cert_info->key->algor->algorithm);
#endif
   const char *algoname;

   if (pubkey_algonid == NID_undef) {
      algoname = "Unrecognized";
   } else {
      algoname = OBJ_nid2ln(pubkey_algonid);
   }

   dStr_sprintfa(ds, "This certificate's public key algorithm is not accepted "
                     "(%s).\n", algoname);
}

/*
 * Generate dialog msg when the public key is not acceptable. As of 2016,
 * this was triggered by RSA keys below 2048 bits, if I recall correctly.
 */
static void Tls_cert_bad_key(const X509 *cert, Dstr *ds)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
   int pubkey_algonid = get_cert_algorithm(cert);
#else
   int pubkey_algonid = OBJ_obj2nid(cert->cert_info->key->algor->algorithm);
#endif
   const char *algoname;

   if (pubkey_algonid == NID_undef) {
      algoname = "Unrecognized";
   } else {
      algoname = OBJ_nid2ln(pubkey_algonid);
   }

   dStr_sprintfa(ds, "This certificate's key is not accepted, which generally "
                     "means it's too weak (%s).\n", algoname);
}

/*
 * Make a dialog msg containing warnings about problems with the certificate.
 */
static char *Tls_make_bad_cert_msg(const X509 *cert, uint32_t flags)
{
   char *ret = NULL;
   Dstr *ds = dStr_new(NULL);

   switch (flags) {

   case X509_V_OK:
      /* Everything is ok */
      break;

   case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
      /* Either self signed and untrusted */
      Tls_cert_cn_mismatch(cert, ds);
      break;

   case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
   case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
      Tls_cert_trust_chain_failed(cert, ds);
      break;

   case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
   case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
   case X509_V_ERR_CERT_SIGNATURE_FAILURE:
   case X509_V_ERR_CRL_SIGNATURE_FAILURE:
      Tls_cert_bad_hash(cert, ds);
      break;

   case X509_V_ERR_CERT_NOT_YET_VALID:
   case X509_V_ERR_CRL_NOT_YET_VALID:
      Tls_cert_not_valid_yet(cert, ds);
      break;

   case X509_V_ERR_CERT_HAS_EXPIRED:
   case X509_V_ERR_CRL_HAS_EXPIRED:
      Tls_cert_expired(cert, ds);
      break;
      
   case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
   case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
   case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
   case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
      dStr_sprintfa(ds, "Certificate formatting error (%d)\n", flags);
      break;
      
   case X509_V_ERR_INVALID_CA:
   case X509_V_ERR_INVALID_PURPOSE:
   case X509_V_ERR_CERT_UNTRUSTED:
   case X509_V_ERR_CERT_REJECTED:
   case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
      Tls_cert_trust_chain_failed(cert, ds);
      break;
      
   case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
   case X509_V_ERR_AKID_SKID_MISMATCH:
   case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
      Tls_cert_trust_chain_failed(cert, ds);
      break;
      
   case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
      Tls_cert_trust_chain_failed(cert, ds);
      break;
      
   case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
      Tls_cert_trust_chain_failed(cert, ds);
      break;
      
   default:
      dStr_sprintfa(ds, "The certificate can not be validated: flag value 0x%04x", flags);

   }
   
   ret = ds->str;
   dStr_free(ds, 0);
   return ret;
}

static int Tls_cert_auth_cmp(const void *v1, const void *v2)
{
   const CertAuth_t *c1 = (CertAuth_t *)v1, *c2 = (CertAuth_t *)v2;

   return strcmp(c1->name, c2->name);
}

static int Tls_cert_auth_cmp_by_name(const void *v1, const void *v2)
{
   const CertAuth_t *c = (CertAuth_t *)v1;
   const char *name = (char *)v2;

   return strcmp(c->name, name);
}

/*
 * Examine the certificate, and, if problems are detected, ask the user what
 * to do.
 * Return: -1 if connection should be canceled, or 0 if it should continue.
 */
static int Tls_examine_certificate(SSL *ssl_connection, Server_t *srv)
{
   X509 *cert;
   uint32_t st;
   int choice = -1, ret = -1;
   char *title = dStrconcat("Dillo TLS security warning: ",srv->hostname,NULL);

   cert = SSL_get_peer_certificate(ssl_connection);
   if (cert == NULL){
      /* Inform user that remote system cannot be trusted */
      choice = a_Dialog_choice(title,
         "No certificate received from this site. Can't verify who it is.",
         "Continue", "Cancel", NULL);

      /* Abort on anything but "Continue" */
      if (choice == 1){
         ret = 0;
      }
   } else {
      /* check the certificate */
      st = SSL_get_verify_result(ssl_connection);
      if (st == X509_V_OK) {
	 ret = 0;
      } else {
         char *dialog_warning_msg = Tls_make_bad_cert_msg(cert, st);

         choice = a_Dialog_choice(title, dialog_warning_msg, "Continue",
                                  "Cancel", NULL);
         if (choice == 1) {
            ret = 0;
         }
         dFree(dialog_warning_msg);
      }
   }
   dFree(title);

   X509_free(cert);

   if (choice == -1) {
      srv->cert_status = CERT_STATUS_CLEAN;          /* no warning popups */
   } else if (choice == 1) {
      srv->cert_status = CERT_STATUS_USER_ACCEPTED;  /* clicked Continue */
   } else {
      /* 2 for Cancel, or 0 when window closed. */
      srv->cert_status = CERT_STATUS_BAD;
   }
   return ret;
}

/*
 * If the connection was closed before we got the certificate, we need to
 * reset state so that we'll try again.
 */
void a_Tls_reset_server_state(const DilloUrl *url)
{
   if (servers) {
      Server_t *s = dList_find_sorted(servers, url, Tls_servers_by_url_cmp);

      if (s && s->cert_status == CERT_STATUS_RECEIVING)
         s->cert_status = CERT_STATUS_NONE;
   }
}

/*
 * Close an open TLS connection.
 */
static void Tls_close_by_key(int connkey)
{
   Conn_t *c;

   if ((c = a_Klist_get_data(conn_list, connkey))) {
      a_Tls_reset_server_state(c->url);
      if (c->connecting) {
         a_IOwatch_remove_fd(c->fd, -1);
         dClose(c->fd);
      }

      if(c->ssl != NULL) {
	 SSL_free(c->ssl);
	 c->ssl = NULL;
      }

      a_Url_free(c->url);
      Tls_fd_map_remove_entry(c->fd);
      a_Klist_remove(conn_list, connkey);
      dFree(c);
   }
}

/*
 * Connect, set a callback if it's still not completed. If completed, check
 * the certificate and report back to http.
 */
static void Tls_handshake(int fd, int connkey)
{
   int ret;
   bool_t ongoing = FALSE, failed = TRUE;
   Conn_t *conn;

   if (!(conn = a_Klist_get_data(conn_list, connkey))) {
      MSG("Tls_connect: conn for fd %d not valid\n", fd);
      return;
   }

   if (!conn->handshaked) {
      ret = SSL_connect(conn->ssl);

      if (ret == -1 && (SSL_get_error(conn->ssl, ret) == SSL_ERROR_WANT_READ ||
			SSL_get_error(conn->ssl, ret) == SSL_ERROR_WANT_WRITE)) {
	 int err = SSL_get_error(conn->ssl, ret);
         int want = err == SSL_ERROR_WANT_READ ? DIO_READ : DIO_WRITE;

         _MSG("iowatching fd %d for tls -- want %s\n", fd,
	      err == SSL_ERROR_WANT_READ ? "read" : "write");
         a_IOwatch_remove_fd(fd, -1);
         a_IOwatch_add_fd(fd, want, Tls_handshake_cb, INT2VOIDP(connkey));
         ongoing = TRUE;
         failed = FALSE;
      } else if (ret == 1) {
	 conn->handshaked = TRUE;
         Server_t *srv = dList_find_sorted(servers, conn->url,
                                           Tls_servers_by_url_cmp);
         if (srv->cert_status == CERT_STATUS_RECEIVING) {
            /* Making first connection with the server. Show cipher used. */
            SSL *ssl = conn->ssl;
            const char *version = SSL_get_version(ssl),
                       *cipher = SSL_get_cipher_list(ssl, 0);

            MSG("%s", URL_AUTHORITY(conn->url));
            if (URL_PORT(conn->url) != URL_HTTPS_PORT)
               MSG(":%d", URL_PORT(conn->url));
            MSG(" %s, cipher %s\n", version, cipher);
         }
         if (srv->cert_status == CERT_STATUS_USER_ACCEPTED ||
             (Tls_examine_certificate(conn->ssl, srv) != -1)) {
            failed = FALSE;
         }
      } else if (ret < 0) {
         int err = SSL_get_error(conn->ssl, ret);
         MSG("SSL_connect() failed with error %d.\n", err);
      } else {
         MSG("SSL_connect() failed.\n");
      }
   }

   /*
    * If there were problems with the certificate, the connection may have
    * been closed by the server if the user responded too slowly to a popup.
    */

   if (!ongoing) {
      if (a_Klist_get_data(conn_list, connkey)) {
         conn->connecting = FALSE;
         if (failed) {
            Tls_close_by_key(connkey);
         }
         a_IOwatch_remove_fd(fd, -1);
         a_Http_connect_done(fd, failed ? FALSE : TRUE);
      } else {
         MSG("Connection disappeared. Too long with a popup popped up?\n");
      }
   }
}

static void Tls_handshake_cb(int fd, void *vconnkey)
{
   Tls_handshake(fd, VOIDP2INT(vconnkey));
}

/*
 * Make TLS connection over a connect()ed socket.
 */
void a_Tls_connect(int fd, const DilloUrl *url)
{
   SSL *ssl = SSL_new(ssl_context);
   bool_t success = TRUE;
   int connkey = -1;
   int ret;

   if (ssl == NULL) {
      MSG("Error creating SSL connection\n");
      success = FALSE;
   }
   
   if (!ssl_enabled)
      success = FALSE;

   if (success && Tls_user_said_no(url)) {
      success = FALSE;
   }

   /* Need to do this if we want to have the option of dealing
    * with self-signed certs */
   if (success) {
      SSL_set_verify(ssl, SSL_VERIFY_NONE, 0);
   }

   /* Assign TLS connection to this file descriptor */
   if (success) {
      Conn_t *conn = Tls_conn_new(fd, url, ssl);
      connkey = Tls_make_conn_key(conn);

      if (SSL_set_fd(ssl, fd) == 0) {
         MSG("Error connecting network socket to SSL.\n");
         success = FALSE;
     }
   }
   
   /* Configure SSL to use the servername */
   if (success && SSL_set_tlsext_host_name(ssl, URL_HOST(url)) == 0) {
      MSG("Error setting servername to SSL\n");
      success = FALSE;
   }

   /*MSG("TLS connection initialized, trying to handshake.\n");*/
   
   if (!success) {
      a_Tls_reset_server_state(url);
      a_Http_connect_done(fd, success);
   } else {
      Tls_handshake(fd, connkey);
   }
}

/*
 * Read data from an open TLS connection.
 */
int a_Tls_read(void *conn, void *buf, size_t len)
{
   Conn_t *c = (Conn_t*)conn;
   int ret = SSL_read(c->ssl, buf, len);

   if (ret < 0) {
      int err = SSL_get_error(c->ssl, ret);
      if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
	 MSG("READ failed with %d: a TLS error\n", err);
   }
   return ret;
}

/*
 * Write data to an open TLS connection.
 */
int a_Tls_write(void *conn, void *buf, size_t len)
{
   Conn_t *c = (Conn_t*)conn;
   int ret = SSL_write(c->ssl, buf, len);

   if (ret < 0) {
      MSG("WRITE failed with %d: a TLS error\n", SSL_get_error(c->ssl, ret));
   }
   return ret;
}

void a_Tls_close_by_fd(int fd)
{
   FdMapEntry_t *fme = dList_find_custom(fd_map, INT2VOIDP(fd),
                                         Tls_fd_map_cmp);

   if (fme) {
      Tls_close_by_key(fme->connkey);
   }
}

static void Tls_servers_freeall()
{
   if (servers) {
      Server_t *s;
      int i, n = dList_length(servers);

      for (i = 0; i < n; i++) {
         s = (Server_t *) dList_nth_data(servers, i);
         dFree(s->hostname);
         dFree(s);
      }
      dList_free(servers);
   }
}

static void Tls_fd_map_remove_all()
{
   if (fd_map) {
      FdMapEntry_t *fme;
      int i, n = dList_length(fd_map);

      for (i = 0; i < n; i++) {
         fme = (FdMapEntry_t *) dList_nth_data(fd_map, i);
         dFree(fme);
      }
      dList_free(fd_map);
   }
}

/*
 * Clean up
 */
void a_Tls_freeall(void)
{
   Tls_fd_map_remove_all();
   Tls_servers_freeall();
}

#endif /* ENABLE_SSL */
