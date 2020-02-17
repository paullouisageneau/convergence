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

#include "pla/securetransport.hpp"
#include "pla/datagramsocket.hpp"
#include "pla/random.hpp"

#include <gnutls/dtls.h>

namespace pla {

static bool check_gnutls(int ret, const std::string &message = "GnuTLS error") {
	if (ret < 0) {
		if (!gnutls_error_is_fatal(ret))
			return false;
		if (ret == GNUTLS_E_TIMEDOUT)
			throw timeout();
		throw std::runtime_error(message + ": " + gnutls_strerror(ret));
	}
	return true;
}

// Force 128+ bits cipher, disable SSL3.0 and TLS1.0, disable RC4
string SecureTransport::DefaultPriorities = "SECURE128:-VERS-SSL3.0:-VERS-TLS1.0:-ARCFOUR-128";

SecureTransport::SecureTransport(Stream *stream, bool server)
    : mStream(stream), mVerifier(NULL), mPriorities(DefaultPriorities), mIsHandshakeDone(false),
      mIsByeDone(false) {
	Assert(stream);

	// Init session
	unsigned int flags = (server ? GNUTLS_SERVER : GNUTLS_CLIENT);
	if (stream->isMessage())
		flags |= GNUTLS_DATAGRAM;
	check_gnutls(gnutls_init(&mSession, flags));

	try {
		// Set session pointer
		gnutls_session_set_ptr(mSession, this);

		// Set callbacks
		gnutls_transport_set_ptr(mSession, this);
		gnutls_transport_set_push_function(mSession, WriteCallback);
		gnutls_transport_set_pull_function(mSession, ReadCallback);
		gnutls_transport_set_pull_timeout_function(mSession, TimeoutCallback);

		if (stream->isMessage())
			gnutls_dtls_set_mtu(mSession, 1452); // Defaults to UDP over IPv6 on ethernet
	} catch (...) {
		gnutls_deinit(mSession);
		throw;
	}
}

SecureTransport::~SecureTransport(void) {
	NOEXCEPTION(close()); // calls gnutls_bye() if necessary

	gnutls_deinit(mSession);

	delete mStream;

	for (auto c : mCredsToDelete)
		delete c;
}

void SecureTransport::addCredentials(Credentials *creds, bool ownership) {
	// Install credentials
	creds->install(this);

	if (ownership)
		mCredsToDelete.push_back(creds);
}

void SecureTransport::setHostname(const string &hostname) {
	if (isHandshakeDone())
		throw std::runtime_error("Unable to set secure transport hostname: handshake is done");

	mHostname = hostname;
}

void SecureTransport::handshake(void) {
	// Set priorities
	// LogDebug("SecureTransport::handshake", "Setting priorities: " + mPriorities);
	const char *err_pos = NULL;
	check_gnutls(gnutls_priority_set_direct(mSession, mPriorities.c_str(), &err_pos),
	             "Unable to set TLS priorities: " + mPriorities);

	if (isClient()) {
		// Set server name
		if (!mHostname.empty())
			gnutls_server_name_set(mSession, GNUTLS_NAME_DNS, mHostname.data(), mHostname.size());
	}

	// Perform the TLS handshake
	// LogDebug("SecureTransport::handshake", "Performing handshake...");
	while (!check_gnutls(gnutls_handshake(mSession), "TLS handshake failed")) {
	}

	mIsHandshakeDone = true;
	mIsByeDone = false;
}

void SecureTransport::close(void) {
	if (!mIsByeDone) {
		gnutls_bye(mSession, GNUTLS_SHUT_RDWR);

		mIsByeDone = true;

		if (mStream)
			mStream->close();
	}
}

bool SecureTransport::isClient(void) const { return true; }

bool SecureTransport::isHandshakeDone(void) const { return mIsHandshakeDone; }

bool SecureTransport::isAnonymous(void) const {
	return gnutls_auth_get_type(mSession) == GNUTLS_CRD_ANON;
}

bool SecureTransport::hasPrivateSharedKey(void) const {
	return gnutls_auth_get_type(mSession) == GNUTLS_CRD_PSK;
}

bool SecureTransport::hasCertificate(void) const {
	return gnutls_auth_get_type(mSession) == GNUTLS_CRD_CERTIFICATE;
}

string SecureTransport::getPrivateSharedKeyHint(void) const {
	const char *serverHint = gnutls_psk_client_get_hint(mSession);
	if (serverHint)
		return string(serverHint);
	else
		return "";
}

size_t SecureTransport::readSome(byte *buffer, size_t size) {
	ssize_t ret;
	do {
		ret = gnutls_record_recv(mSession, buffer, size);
	} while (!check_gnutls(ret));

	if (ret == 0 || ret == GNUTLS_E_PREMATURE_TERMINATION)
		return 0;
	return size_t(ret);
}

size_t SecureTransport::writeSome(const byte *data, size_t size) {
	ssize_t ret;
	do {
		ret = gnutls_record_send(mSession, data, size);
	} while (!check_gnutls(ret));

	if (ret == GNUTLS_E_PREMATURE_TERMINATION)
		return 0;
	return size_t(ret);
}

bool SecureTransport::isMessage(void) const { return mStream->isMessage(); }

void SecureTransport::setVerifier(Verifier *verifier) { mVerifier = verifier; }

ssize_t SecureTransport::DirectWriteCallback(gnutls_transport_ptr_t ptr, const void *data,
                                             size_t len) {
	try {
		Stream *s = static_cast<Stream *>(ptr);
		s->write(static_cast<const byte *>(data), len);
		return ssize_t(len);
	} catch (const std::exception &e) {
		LogDebug("SecureTransport::DirectWriteCallback", e.what());
		return -1;
	}
}

ssize_t SecureTransport::WriteCallback(gnutls_transport_ptr_t ptr, const void *data, size_t len) {
	SecureTransport *st = static_cast<SecureTransport *>(ptr);
	if (!st->mStream)
		return 0;

	try {
		st->mStream->write(static_cast<const byte *>(data), len);
		gnutls_transport_set_errno(st->mSession, 0);
		return ssize_t(len);
	} catch (const timeout &t) {
		LogDebug("SecureTransport::WriteCallback", "Timeout");
		gnutls_transport_set_errno(st->mSession, ETIMEDOUT);
	} catch (const std::exception &e) {
		LogDebug("SecureTransport::WriteCallback", e.what());
		gnutls_transport_set_errno(st->mSession, ECONNRESET);
	}

	return -1;
}

ssize_t SecureTransport::ReadCallback(gnutls_transport_ptr_t ptr, void *data, size_t maxlen) {
	SecureTransport *st = static_cast<SecureTransport *>(ptr);
	if (!st->mStream)
		return 0;

	try {
		size_t ret = st->mStream->read(static_cast<byte *>(data), maxlen);
		gnutls_transport_set_errno(st->mSession, 0);
		return ssize_t(ret);
	} catch (const timeout &t) {
		LogDebug("SecureTransport::ReadCallback", "Timeout");
		gnutls_transport_set_errno(st->mSession, ETIMEDOUT);
	} catch (const std::exception &e) {
		LogDebug("SecureTransport::ReadCallback", e.what());
		gnutls_transport_set_errno(st->mSession, ECONNRESET);
	}

	return -1;
}

int SecureTransport::TimeoutCallback(gnutls_transport_ptr_t ptr, unsigned int ms) {
	SecureTransport *st = static_cast<SecureTransport *>(ptr);
	try {
		gnutls_transport_set_errno(st->mSession, 0);
		if (st->mStream->wait(milliseconds(ms)))
			return 1;
		else
			return 0;
	} catch (const std::exception &e) {
		LogDebug("SecureTransport::TimeoutCallback", e.what());
		gnutls_transport_set_errno(st->mSession, ECONNRESET);
	}

	return -1;
}

int SecureTransport::CertificateCallback(gnutls_session_t session) {
	// LogDebug("SecureTransport::CertificateCallback", "Entering certificate callback");

	SecureTransport *transport = static_cast<SecureTransport *>(gnutls_session_get_ptr(session));
	if (!transport) {
		LogWarn("SecureTransport::CertificateCallback",
		        "TLS certificate verification callback called with unknown session");
		return GNUTLS_E_CERTIFICATE_ERROR;
	}

	try {
		if (!transport->mVerifier) {
			unsigned int status;
			check_gnutls(gnutls_certificate_verify_peers2(session, &status));
			if (status) {
				string reasons;
				if (status & GNUTLS_CERT_SIGNER_NOT_FOUND)
					reasons += "unknown issuer,";
				else if (status & GNUTLS_CERT_REVOKED)
					reasons += "revoked,";
				else if (status & GNUTLS_CERT_EXPIRED)
					reasons += "expired,";
				else if (status & GNUTLS_CERT_NOT_ACTIVATED)
					reasons += "not yet activated,";
				else if (status & GNUTLS_CERT_INVALID)
					reasons += "not trusted,";
				reasons.pop_back();

				LogWarn("SecureTransport::CertificateCallback", "Invalid certificate: " + reasons);
				return GNUTLS_E_CERTIFICATE_ERROR;
			}

			return 0;
		}

		// We assume certificates are X.509
		if (gnutls_certificate_type_get(session) != GNUTLS_CRT_X509) {
			LogWarn("SecureTransport::CertificateCallback", "Peer certificate is not X.509");
			return GNUTLS_E_CERTIFICATE_ERROR;
		}

		// Get peer's certificate
		unsigned count = 0;
		const gnutls_datum_t *array = gnutls_certificate_get_peers(session, &count);
		if (!array || count == 0) {
			LogWarn("SecureTransport::CertificateCallback", "No peer certificate");
			return GNUTLS_E_CERTIFICATE_ERROR;
		}

		std::vector<Rsa::PublicKey> chain;
		chain.reserve(count);
		for (int i = 0; i < count; ++i) {
			gnutls_x509_crt_t crt;
			check_gnutls(gnutls_x509_crt_init(&crt));

			try {
				// Reimport certificate
				int ret = gnutls_x509_crt_import(crt, array + i, GNUTLS_X509_FMT_DER);
				if (ret != GNUTLS_E_SUCCESS)
					throw std::runtime_error(string("Unable to retrieve X509 certificate: ") +
					                         Errorstring(ret));

				if (i == 0 && !transport->mHostname.empty()) {
					if (!gnutls_x509_crt_check_hostname(crt, transport->mHostname.c_str())) {
						LogWarn("SecureTransport::CertificateCallback",
						        "The certificate's owner does not match the expected name: " +
						            transport->mHostname);
						return GNUTLS_E_CERTIFICATE_ERROR;
					}
				}

				chain.push_back(Rsa::PublicKey(crt));
			} catch (...) {
				gnutls_x509_crt_deinit(crt);
				throw;
			}

			gnutls_x509_crt_deinit(crt);
		}

		if (transport->mVerifier->verifyPublicKey(chain))
			return 0;
	} catch (const std::exception &e) {
		LogWarn("SecureTransport::CertificateCallback",
		        string("TLS certificate verification failed: ") + e.what());
	}

	return GNUTLS_E_CERTIFICATE_ERROR;
}

int SecureTransport::PrivateSharedKeyCallback(gnutls_session_t session, const char *username,
                                              gnutls_datum_t *datum) {
	// LogDebug("SecureTransport", "Entering PSK callback");

	SecureTransport *transport = static_cast<SecureTransport *>(gnutls_session_get_ptr(session));
	if (!transport) {
		LogWarn("SecureTransport::PrivateSharedKeyCallback",
		        "TLS PSK callback called with unknown session");
		return -1;
	}

	if (!transport->mVerifier) {
		LogWarn("SecureTransport::PrivateSharedKeyCallback", "No PSK verifier specified");
		return -1;
	}

	string name(username);
	binary key;
	try {

		if (!transport->mVerifier->verifyPrivateSharedKey(name, key, ""))
			return -1;
		if (name != username)
			return -1;
	} catch (const std::exception &e) {
		LogWarn("SecureTransport::PrivateSharedKeyCallback",
		        string("TLS PSK verification failed: ") + e.what());
		return -1;
	}

	datum->size = key.size();
	datum->data = static_cast<unsigned char *>(gnutls_malloc(datum->size));
	std::memcpy(datum->data, key.data(), datum->size);
	return 0;
}

int SecureTransport::PrivateSharedKeyClientCallback(gnutls_session_t session, char **username,
                                                    gnutls_datum_t *datum) {
	// LogDebug("SecureTransport", "Entering PSK client callback");

	SecureTransport *transport = static_cast<SecureTransport *>(gnutls_session_get_ptr(session));
	if (!transport) {
		LogWarn("SecureTransport::PrivateSharedKeyCallback",
		        "TLS PSK client callback called with unknown session");
		return -1;
	}

	if (!transport->mVerifier) {
		LogWarn("SecureTransport::PrivateSharedKeyCallback", "No PSK verifier specified");
		return -1;
	}

	string name, hint;
	binary key;

	const char *serverHint = gnutls_psk_client_get_hint(session);
	if (serverHint)
		hint = serverHint;

	try {
		if (!transport->mVerifier->verifyPrivateSharedKey(name, key, hint))
			return -1;
	} catch (const std::exception &e) {
		LogWarn("SecureTransport::PrivateSharedKeyCallback",
		        string("TLS PSK verification failed: ") + e.what());
		return -1;
	}

	*username = static_cast<char *>(gnutls_malloc(name.size() + 1));
	std::strcpy(*username, name.c_str());

	datum->size = key.size();
	datum->data = static_cast<unsigned char *>(gnutls_malloc(datum->size));
	std::memcpy(datum->data, key.data(), datum->size);
	return 0;
}

string SecureTransport::Errorstring(int code) {
	switch (code) {
	case GNUTLS_E_PULL_ERROR:
		return "Connection failed";
	case GNUTLS_E_PUSH_ERROR:
		return "Connection failed";
	default:
		return gnutls_strerror(code);
	}
}

void SecureTransport::Credentials::install(SecureTransport *st) {
	install(st->mSession, st->mPriorities);
}

SecureTransport::Certificate::Certificate(void) {
	// Allocate certificate credentials
	check_gnutls(gnutls_certificate_allocate_credentials(&mCreds));

	// gnutls_certificate_set_verify_flags(mCreds, GNUTLS_VERIFY_DISABLE_CA_SIGN
	// 		| GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT
	// 		| GNUTLS_VERIFY_ALLOW_ANY_X509_V1_CA_CRT);

	gnutls_certificate_set_verify_function(mCreds, SecureTransport::CertificateCallback);

	// Set system CA
	gnutls_certificate_set_x509_system_trust(mCreds);
}

SecureTransport::Certificate::Certificate(const string &certFilename, const string &keyFilename)
    : Certificate() {
	// Import certificate and private key
	check_gnutls(gnutls_certificate_set_x509_key_file2(mCreds, certFilename.c_str(),
	                                                   keyFilename.c_str(), GNUTLS_X509_FMT_PEM,
	                                                   NULL, GNUTLS_PKCS_PLAIN));
}

SecureTransport::Certificate::~Certificate(void) { gnutls_certificate_free_credentials(mCreds); }

void SecureTransport::Certificate::install(gnutls_session_t session, string &priorities) {
	check_gnutls(gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, mCreds));
	// No modification of priorities
}

SecureTransport::RsaCertificate::RsaCertificate(const Rsa::PrivateKey &priv, const string &name,
                                                const SecureTransport::RsaCertificate *issuer) {
	// Init certificate and key
	check_gnutls(gnutls_x509_crt_init(&mCrt));
	check_gnutls(gnutls_x509_privkey_init(&mKey));

	try {
		Rsa::CreateCertificate(mCrt, mKey, priv, name);
		if (issuer)
			Rsa::SignCertificate(mCrt, issuer->mCrt, issuer->mKey);
		else
			Rsa::SignCertificate(mCrt, mCrt, mKey); // self-signed

		int ret = gnutls_certificate_set_x509_key(mCreds, &mCrt, 1, mKey);
		if (ret != GNUTLS_E_SUCCESS)
			throw std::runtime_error(
			    string("Unable to set certificate and key pair in credentials: ") +
			    Errorstring(ret));
	} catch (...) {
		gnutls_x509_crt_deinit(mCrt);
		gnutls_x509_privkey_deinit(mKey);
		throw;
	}
}

SecureTransport::RsaCertificate::~RsaCertificate(void) {
	// Keys are freed by gnutls_certificate_free_credentials
}

SecureTransport::RsaCertificateChain::RsaCertificateChain(
    const std::vector<SecureTransport::RsaCertificate *> &chain) {
	if (chain.empty())
		throw std::runtime_error("Empty certificate chain provided");

	gnutls_x509_crt_t *crts = new gnutls_x509_crt_t[chain.size()];
	for (int i = 0; i < chain.size(); ++i)
		crts[i] = chain[i]->mCrt;

	int ret = gnutls_certificate_set_x509_key(mCreds, crts, chain.size(), chain[0]->mKey);
	if (ret != GNUTLS_E_SUCCESS)
		throw std::runtime_error(string("Unable to set certificate and key pair in credentials: ") +
		                         Errorstring(ret));
}

SecureTransport::RsaCertificateChain::~RsaCertificateChain(void) {
	// Nothing to do
}

SecureTransportClient::SecureTransportClient(Stream *stream, Credentials *creds,
                                             const string &hostname)
    : SecureTransport(stream, false) {
	try {
		setHostname(hostname);

		if (creds) {
			addCredentials(creds, true);
			handshake();
		}
	} catch (...) {
		mStream = NULL; // so stream won't be deleted
		throw;
	}
}

SecureTransportClient::~SecureTransportClient(void) {}

SecureTransportClient::Anonymous::Anonymous(void) {
	// Allocate anonymous credentials
	check_gnutls(gnutls_anon_allocate_client_credentials(&mCreds));
}

SecureTransportClient::Anonymous::~Anonymous(void) { gnutls_anon_free_client_credentials(mCreds); }

void SecureTransportClient::Anonymous::install(gnutls_session_t session, string &priorities) {
	check_gnutls(gnutls_credentials_set(session, GNUTLS_CRD_ANON, mCreds));
	priorities += ":+ANON-DH:+ANON-ECDH";
}

SecureTransportClient::PrivateSharedKey::PrivateSharedKey(void) {
	// Allocate PSK credentials
	check_gnutls(gnutls_psk_allocate_client_credentials(&mCreds));

	// Set PSK credentials client-side callback
	gnutls_psk_set_client_credentials_function(mCreds, PrivateSharedKeyClientCallback);
}

SecureTransportClient::PrivateSharedKey::PrivateSharedKey(const string &username,
                                                          const binary &key) {
	// Allocate PSK credentials
	check_gnutls(gnutls_psk_allocate_client_credentials(&mCreds));

	// Set PSK credentials
	gnutls_datum_t datum;
	datum.size = key.size();
	datum.data = static_cast<unsigned char *>(gnutls_malloc(datum.size));
	std::memcpy(datum.data, key.data(), key.size());

	try {
		check_gnutls(
		    gnutls_psk_set_client_credentials(mCreds, username.c_str(), &datum, GNUTLS_PSK_KEY_RAW),
		    "Unable to set credentials");
	} catch (...) {
		gnutls_free(datum.data);
		throw;
	}

	gnutls_free(datum.data);
}

SecureTransportClient::PrivateSharedKey::~PrivateSharedKey(void) {
	gnutls_psk_free_client_credentials(mCreds);
}

void SecureTransportClient::PrivateSharedKey::install(gnutls_session_t session,
                                                      string &priorities) {
	check_gnutls(gnutls_credentials_set(session, GNUTLS_CRD_PSK, mCreds));
	priorities += ":+PSK:+DHE-PSK";
}

SecureTransportServer::SecureTransportServer(Stream *stream, Credentials *creds,
                                             bool requestClientCertificate)
    : SecureTransport(stream, true) {
	try {
		gnutls_handshake_set_post_client_hello_function(mSession, PostClientHelloCallback);

		if (requestClientCertificate) {
			gnutls_certificate_server_set_request(mSession, GNUTLS_CERT_REQUEST);
			gnutls_certificate_send_x509_rdn_sequence(mSession, 1); // do not advertise trusted CAs
		}

		if (creds) {
			addCredentials(creds, true);
			handshake();
		}
	} catch (...) {
		mStream = NULL; // so stream won't be deleted
		throw;
	}
}

SecureTransportServer::~SecureTransportServer(void) {}

bool SecureTransportServer::isClient(void) const { return false; }

int SecureTransportServer::PostClientHelloCallback(gnutls_session_t session) {
	// LogDebug("SecureTransportServer", "Entering post client hello callback");

	SecureTransportServer *transport =
	    static_cast<SecureTransportServer *>(gnutls_session_get_ptr(session));
	if (!transport) {
		LogWarn("SecureTransportServer::PostClientHelloCallback",
		        "TLS post client hello callback called with unknown session");
		return -1;
	}

	try {
		char buffer[BufferSize];
		size_t size = BufferSize;
		unsigned int type = GNUTLS_NAME_DNS;
		if (gnutls_server_name_get(session, buffer, &size, &type, 0) == GNUTLS_E_SUCCESS) {
			string name(buffer, size);

			if (!transport->mHostname.empty() && transport->mHostname != name)
				return GNUTLS_E_NO_CERTIFICATE_FOUND;

			if (transport->mVerifier) {
				if (!transport->mVerifier->verifyName(name, transport))
					return GNUTLS_E_NO_CERTIFICATE_FOUND;
			}
		}
	} catch (const std::exception &e) {
		LogWarn("SecureTransportServer::PostClientHelloCallback",
		        string("TLS client hello callback failed: ") + e.what());
		return -1;
	}

	return 0;
}

/*
SecureTransport *SecureTransportServer::Listen(ServerSocket &lsock, Address *remote, bool
requestClientCertificate, duration connectionTimeout)
{
    while(true)
    {
        Socket *sock = NULL;
        try {
            sock = new Socket;
            lsock.accept(*sock);
            if(remote)
                *remote = sock->getRemoteAddress();
            if(connectionTimeout > duration::zero())
                sock->settimeout(connectionTimeout);
        }
        catch(...)
        {
            delete sock;
            throw;
        }

        SecureTransportServer *transport = NULL;
        try {
            transport = new SecureTransportServer(sock, NULL, requestClientCertificate);
        }
        catch(const std::exception &e)
        {
            LogWarn("SecureTransportServer::Listen(stream)", e.what());
            delete sock;
            return NULL;
        }

        return transport;
    }
}

SecureTransport *SecureTransportServer::Listen(DatagramSocket &sock, Address *remote, bool
requestClientCertificate, duration streamTimeout)
{
    gnutls_datum_t cookieKey;
    gnutls_key_generate(&cookieKey, GNUTLS_COOKIE_KEY_SIZE);

    char *buffer = new char[DatagramSocket::MaxDatagramSize];
    try {
        while(true)
        {
            Address sender;
            int len = sock.peek(buffer, DatagramSocket::MaxDatagramSize, sender);
            if(len < 0) throw Netstd::runtime_error("Unable to listen on datagram socket");

            gnutls_dtls_prestate_st prestate;
            std::memset(&prestate, 0, sizeof(prestate));

            int ret = gnutls_dtls_cookie_verify(&cookieKey,
                            const_cast<sockaddr*>(sender.addr()),	// WTF ?
                            sender.addrLen(),
                            buffer, len,
                            &prestate);

            if(ret == GNUTLS_E_SUCCESS)	// valid cookie
            {
                if(remote)
                    *remote = sender;

                DatagramStream *stream = NULL;
                SecureTransportServer *transport = NULL;
                try {
                    stream = new DatagramStream(&sock, sender);
                    if(streamTimeout > duration::zero()) stream->settimeout(streamTimeout);
                    transport = new SecureTransportServer(stream, NULL, requestClientCertificate);
                }
                catch(...)
                {
                    delete stream;
                    delete transport;
                    throw;
                }

                gnutls_dtls_prestate_set(transport->mSession, &prestate);
                delete[] buffer;
                return transport;
            }

            NOEXCEPTION(sock.read(buffer, DatagramSocket::MaxDatagramSize, sender));

            DatagramStream stream(&sock, sender);
            gnutls_dtls_cookie_send(&cookieKey,
                        const_cast<sockaddr*>(sender.addr()),	// WTF ?
                        sender.addrLen(),
                        &prestate,
                        static_cast<gnutls_transport_ptr_t>(&stream),
                        DirectWriteCallback);

            // discard peeked data
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    catch(...)
    {
        delete[] buffer;
        throw;
    }
}
*/

SecureTransportServer::Anonymous::Anonymous(void) {
	// Allocate anonymous credentials
	check_gnutls(gnutls_anon_allocate_server_credentials(&mCreds));
}

SecureTransportServer::Anonymous::~Anonymous(void) { gnutls_anon_free_server_credentials(mCreds); }

void SecureTransportServer::Anonymous::install(gnutls_session_t session, string &priorities) {
	check_gnutls(gnutls_credentials_set(session, GNUTLS_CRD_ANON, mCreds));
	priorities += ":+ANON-DH:+ANON-ECDH";
}

SecureTransportServer::PrivateSharedKey::PrivateSharedKey(const string &hint) {
	// Allocate PSK credentials
	check_gnutls(gnutls_psk_allocate_server_credentials(&mCreds));

	// Set hint if supplied
	if (!hint.empty())
		check_gnutls(gnutls_psk_set_server_credentials_hint(mCreds, hint.c_str()));

	// Set PSK callback
	gnutls_psk_set_server_credentials_function(mCreds, SecureTransport::PrivateSharedKeyCallback);
}

SecureTransportServer::PrivateSharedKey::~PrivateSharedKey(void) {
	gnutls_psk_free_server_credentials(mCreds);
}

void SecureTransportServer::PrivateSharedKey::install(gnutls_session_t session,
                                                      string &priorities) {
	check_gnutls(gnutls_credentials_set(session, GNUTLS_CRD_PSK, mCreds));
	priorities += ":+PSK:+DHE-PSK";
}

} // namespace pla
