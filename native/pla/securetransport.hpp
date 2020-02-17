/*************************************************************************
 *   Copyright (C) 2011-2014 by Paul-Louis Ageneau                       *
 *   paul-louis (at) ageneau (dot) org                                   *
 *                                                                       *
 *   This file is part of Plateform.                                     *
 *                                                                       *
 *   Plateform is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU Affero General Public License as      *
 *   published by the Free Software Foundation, either version 3 of      *
 *   the License, or (at your option) any later version.                 *
 *                                                                       *
 *   Plateform is distributed in the hope that it will be useful, but    *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        *
 *   GNU Affero General Public License for more details.                 *
 *                                                                       *
 *   You should have received a copy of the GNU Affero General Public    *
 *   License along with Plateform.                                       *
 *   If not, see <http://www.gnu.org/licenses/>.                         *
 *************************************************************************/

#ifndef PLA_SECURETRANSPORT_H
#define PLA_SECURETRANSPORT_H

#include "pla/binary.hpp"
#include "pla/crypto.hpp"
#include "pla/include.hpp"
#include "pla/serversocket.hpp"
#include "pla/stream.hpp"
#include "pla/string.hpp"

#include <gnutls/abstract.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

namespace pla {

class SecureTransport : public Stream {
public:
	static string DefaultPriorities;

	static void Init(void);
	static void Cleanup(void);

	class Credentials {
	public:
		Credentials(void) {}
		virtual ~Credentials(void) {}
		void install(SecureTransport *st);

	protected:
		virtual void install(gnutls_session_t session, string &priorities) = 0;
	};

	class Certificate : public Credentials {
	public:
		Certificate(void);
		Certificate(const string &certFilename, const string &keyFilename); // PEM encoded
		~Certificate(void);

	protected:
		void install(gnutls_session_t session, string &priorities);
		gnutls_certificate_credentials_t mCreds;
	};

	class RsaCertificate;
	class RsaCertificateChain : public Certificate {
	public:
		RsaCertificateChain(const std::vector<SecureTransport::RsaCertificate *> &chain);
		~RsaCertificateChain(void);
	};

	class RsaCertificate : public Certificate {
	public:
		RsaCertificate(const Rsa::PrivateKey &priv, const string &name,
		               const RsaCertificate *issuer = NULL);
		~RsaCertificate(void);

	protected:
		gnutls_x509_crt_t mCrt;
		gnutls_x509_privkey_t mKey;

		friend class RsaCertificateChain;
	};

	virtual ~SecureTransport(void);

	void addCredentials(Credentials *creds,
	                    bool ownership = false); // creds will be deleted if ownership == true
	void setHostname(const string &hostname);     // remote hostname for client

	void handshake(void);
	void close(void);

	// Stream
	size_t readSome(byte *buffer, size_t size);
	size_t writeSome(const byte *data, size_t size);

	virtual bool isClient(void) const;
	bool isHandshakeDone(void) const;
	bool isMessage(void) const;
	bool isAnonymous(void) const;
	bool hasPrivateSharedKey(void) const;
	bool hasCertificate(void) const;
	string getPrivateSharedKeyHint(void) const; // only valid on client side

	struct Verifier {
		virtual bool verifyPublicKey(const std::vector<Rsa::PublicKey> &chain) { return false; }
		virtual bool verifyPrivateSharedKey(const string &username, binary &key) { return false; }
		virtual bool verifyPrivateSharedKey(string &username, binary &key, const string &hint) {
			return verifyPrivateSharedKey(username, key);
		}
		virtual bool verifyName(const string &name, SecureTransport *transport) {
			return true;
		} // default is true
	};

	void setVerifier(Verifier *verifier);

protected:
	static ssize_t DirectWriteCallback(gnutls_transport_ptr_t ptr, const void *data, size_t len);
	static ssize_t WriteCallback(gnutls_transport_ptr_t ptr, const void *data, size_t len);
	static ssize_t ReadCallback(gnutls_transport_ptr_t ptr, void *data, size_t maxlen);
	static int TimeoutCallback(gnutls_transport_ptr_t ptr, unsigned int ms);

	static int CertificateCallback(gnutls_session_t session);
	static int PrivateSharedKeyCallback(gnutls_session_t session, const char *username,
	                                    gnutls_datum_t *datum);
	static int PrivateSharedKeyClientCallback(gnutls_session_t session, char **username,
	                                          gnutls_datum_t *datum);

	static string Errorstring(int code);

	SecureTransport(Stream *stream, bool server); // stream will be deleted on success

	gnutls_session_t mSession;
	Stream *mStream;
	Verifier *mVerifier;
	string mPriorities;
	string mHostname;

	std::list<Credentials *> mCredsToDelete;
	bool mIsHandshakeDone;
	bool mIsByeDone;
};

class SecureTransportClient : public SecureTransport {
public:
	class Anonymous : public Credentials {
	public:
		Anonymous(void);
		~Anonymous(void);

	protected:
		void install(gnutls_session_t session, string &priorities);
		gnutls_anon_client_credentials_t mCreds;
	};

	class PrivateSharedKey : public Credentials {
	public:
		PrivateSharedKey(void);
		PrivateSharedKey(const string &name, const binary &key);
		~PrivateSharedKey(void);

	protected:
		void install(gnutls_session_t session, string &priorities);
		gnutls_psk_client_credentials_t mCreds;
	};

	SecureTransportClient(Stream *stream, Credentials *creds = NULL,
	                      const string &hostname = ""); // creds will be deleted
	~SecureTransportClient(void);
};

class SecureTransportServer : public SecureTransport {
public:
	class Anonymous : public Credentials {
	public:
		Anonymous(void);
		~Anonymous(void);

	protected:
		void install(gnutls_session_t session, string &priorities);
		gnutls_anon_server_credentials_t mCreds;
	};

	class PrivateSharedKey : public Credentials {
	public:
		PrivateSharedKey(const string &hint = "");
		~PrivateSharedKey(void);

	protected:
		void install(gnutls_session_t session, string &priorities);
		gnutls_psk_server_credentials_t mCreds;
	};

	/*
	// These functions are preferred, especially for datagrams (protection against DoS)
	static SecureTransport *Listen(ServerSocket &sock, Address *remote = NULL, bool
	requestClientCertificate = false); static SecureTransport *Listen(DatagramSocket &sock, Address
	*remote = NULL, bool requestClientCertificate = false);
	*/

	SecureTransportServer(Stream *stream, Credentials *creds = NULL,
	                      bool requestClientCertificate = false); // creds will be deleted
	~SecureTransportServer(void);

	bool isClient(void) const;

protected:
	static int PostClientHelloCallback(gnutls_session_t session);
};

} // namespace pla

#endif
