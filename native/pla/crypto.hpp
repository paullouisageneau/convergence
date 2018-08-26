/*************************************************************************
 *   Copyright (C) 2011-2017 by Paul-Louis Ageneau                       *
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

#ifndef PLA_CRYPTO_H
#define PLA_CRYPTO_H

#include "pla/include.hpp"
#include "pla/stream.hpp"
#include "pla/binaryformatter.hpp"

#include <nettle/sha1.h>
#include <nettle/sha2.h>
#include <nettle/sha3.h>
#include <nettle/aes.h>
#include <nettle/ctr.h>
#include <nettle/gcm.h>
#include <nettle/rsa.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

namespace pla
{

class Rsa;

// Hash function interface
class Hash
{
public:
	Hash(void) {}
	virtual ~Hash(void) {}

	// Implementation
	virtual size_t length(void) const = 0;
	virtual void init(void) = 0;
	virtual void process(const binary &data) = 0;
	virtual void finalize(binary &digest) = 0;

	// Direct hash function call
	void compute(const binary &data, binary &digest);
	binary compute(const binary &data);

	// MAC
	virtual void mac(const binary &message, const binary &key, binary &digest) = 0;

private:
	// RSA signature
	virtual bool rsaVerify(const binary &digest, const struct rsa_public_key *key, const mpz_t signature);
	virtual bool rsaSign(const binary &digest, const struct rsa_private_key *key, mpz_t signature);

	friend class Rsa;
};

// SHA1 hash function implementation
class Sha1 : public Hash
{
public:
	size_t length(void) const;
	void init(void);
	void process(const binary &data);
	void finalize(binary &digest);

	// HMAC-SHA1
	void mac(const binary &message, const binary &key, binary &digest);

	// PBKDF2-HMAC-SHA1
	void pbkdf2(const binary &secret, const binary &salt, binary &key, size_t key_len, unsigned iterations);

private:
	bool rsaVerify(const binary &digest, const struct rsa_public_key *key, const mpz_t signature);
	bool rsaSign(const binary &digest, const struct rsa_private_key *key, mpz_t signature);

	struct sha1_ctx mCtx;
};

// SHA256 hash function implementation
class Sha256 : public Hash
{
public:
	size_t length(void) const;
	void init(void);
	void process(const binary &data);
	void finalize(binary &digest);

	// HMAC-SHA256
	void mac(const binary &message, const binary &key, binary &digest);

	// PBKDF2-HMAC-SHA256
	void pbkdf2(const binary &secret, const binary &salt, binary &key, size_t key_len, unsigned iterations);

private:
	bool rsaVerify(const binary &digest, const struct rsa_public_key *key, const mpz_t signature);
	bool rsaSign(const binary &digest, const struct rsa_private_key *key, mpz_t signature);

	struct sha256_ctx mCtx;
};

// SHA512 hash function implementation
class Sha512 : public Hash
{
public:
	size_t length(void) const;
	void init(void);
	void process(const binary &data);
	void finalize(binary &digest);

	// HMAC-SHA512
	void mac(const binary &message, const binary &key, binary &digest);

	// PBKDF2-HMAC-SHA512
	void pbkdf2(const binary &secret, const binary &salt, binary &key, size_t key_len, unsigned iterations);

private:
	bool rsaVerify(const binary &digest, const struct rsa_public_key *key, const mpz_t signature);
	bool rsaSign(const binary &digest, const struct rsa_private_key *key, mpz_t signature);

	struct sha512_ctx mCtx;
};

typedef Sha256 Sha2;	// SHA2 defaults to SHA256

// SHA3-256 hash function implementation
class Sha3_256 : public Hash
{
public:
	size_t length(void) const;
	void init(void);
	void process(const binary &data);
	void finalize(binary &digest);

	// "Unlike SHA1 and SHA2, Keccak does not have the length-extension weakness,
	// hence does not need the HMAC nested construction. Instead, MAC computation
	// can be performed by simply prepending the message with the key."
	// This is exactly what this function does. It's not standard!
	void mac(const binary &message, const binary &key, binary &digest);

	// TODO: SHAKE and KMAC implementation

private:
	// TODO: SHA3 signatures
	//bool rsaVerify(const binary &digest, struct rsa_public_key *key, const mpz_t signature);
	//bool rsaSign(const binary &digest, struct rsa_private_key *key, mpz_t signature);

	struct sha3_256_ctx mCtx;
};

// SHA3-512 hash function implementation
class Sha3_512 : public Hash
{
public:
	size_t length(void) const;
	void init(void);
	void process(const binary &data);
	void finalize(binary &digest);

	// See Sha3_256::mac()
	void mac(const binary &message, const binary &key, binary &digest);

private:
	// TODO: SHA3 signatures
	//bool rsaVerify(const binary &digest, struct rsa_public_key *key, const mpz_t signature);
	//bool rsaSign(const binary &digest, struct rsa_private_key *key, mpz_t signature);

	struct sha3_512_ctx mCtx;
};

typedef Sha3_256 Sha3;	// SHA3 defaults to SHA3-256

class Cipher : public Stream
{
public:
	Cipher(Stream *stream, bool mustDelete = false);
	virtual ~Cipher(void);

	virtual void setEncryptionKey(const binary &key) = 0;
	virtual void setDecryptionKey(const binary &key) = 0;
	virtual void setInitializationVector(const binary &iv) = 0;
	virtual bool getAuthenticationTag(binary &tag) { return false; }

	// Stream
	size_t readSome(char *buffer, size_t size);
	size_t writeSome(const char *data, size_t size);
	void close(void);

protected:
	virtual size_t blockSize(void) const = 0;
	virtual void encryptBlock(binary &block) = 0;
	virtual void decryptBlock(binary &block) = 0;

private:
	Stream *mStream;
	binary mReadBlock, mWriteBlock;
	bool mMustDelete;
};

// AES-CTR implementation
class AesCtr : public Cipher
{
public:
	AesCtr(Stream *stream, bool mustDelete = false);
	~AesCtr(void);

	void setEncryptionKey(const binary &key);
	void setDecryptionKey(const binary &key);
	void setInitializationVector(const binary &iv);

protected:
	size_t blockSize(void) const;
	void encryptBlock(binary &block);
	void decryptBlock(binary &block);

private:
	struct CTR_CTX(struct aes_ctx, AES_BLOCK_SIZE) mCtx;
};

// AES-GCM implementation
class AesGcm : public Cipher
{
public:
	AesGcm(Stream *stream, bool mustDelete = false);
	~AesGcm(void);

	void setEncryptionKey(const binary &key);
	void setDecryptionKey(const binary &key);
	void setInitializationVector(const binary &iv);
	bool getAuthenticationTag(binary &tag);

protected:
	size_t blockSize(void) const;
	void encryptBlock(binary &block);
	void decryptBlock(binary &block);

private:
	struct gcm_aes_ctx mCtx;
};

class Rsa
{
public:
	class PublicKey
	{
	public:
		PublicKey(void);
		PublicKey(const PublicKey &key);
		PublicKey(gnutls_x509_crt_t crt);
		virtual ~PublicKey(void);
		
		virtual PublicKey &operator=(const PublicKey &key);
		virtual void clear(void);
		
		bool operator==(const PublicKey &key) const;
		bool isNull(void) const;

		// Fingerprint
		binary fingerprint(Hash &hash) const;
		template<class H> binary fingerprint(void) const
			{ H hash; return fingerprint(hash); }

		// Verification
		bool verify(Hash &hash, const binary &digest, const binary &signature) const;
		template<class H> bool verify(const binary &digest, const binary &signature) const
			{ H hash; return verify(hash, digest, signature); }

		// Serialization
		virtual string toString() const;
		virtual void fromString(string str);
		virtual binary toBinary() const;
		virtual void fromBinary(binary b);

	private:
		struct rsa_public_key mPub;
		friend class Rsa;
	};

	class PrivateKey : public PublicKey
	{
	public:
		PrivateKey(void);
		PrivateKey(const PrivateKey &key);
		~PrivateKey(void);
		
		PrivateKey &operator=(const PrivateKey &key);
		void clear(void);

		// Signature
		void sign(Hash &hash, const binary &digest, binary &signature) const;
		template<class H> void sign(const binary &digest, binary &signature) const
			{ H hash; sign(hash, digest, signature); }

		// Serialization
		string toString() const;
		void fromString(string str);
		binary toBinary() const;
		void fromBinary(binary b);

	private:
		struct rsa_private_key mPriv;
		friend class Rsa;
	};

	Rsa(unsigned bits = 4096);
	~Rsa(void);

	void generate(PrivateKey &priv);

	static void CreateCertificate(gnutls_x509_crt_t crt, gnutls_x509_privkey_t key, const PrivateKey &priv, const string &name);
	static void SignCertificate(gnutls_x509_crt_t crt, gnutls_x509_crt_t issuer, gnutls_x509_privkey_t issuerKey);

private:
	unsigned mBits;
};

// Argon2 key derivation function for password hashing
class Argon2
{
public:
	static unsigned DefaultTimeCost;
	static unsigned DefaultMemoryCost;
	static unsigned DefaultParallelism;

	Argon2(void);
	Argon2(unsigned tcost, unsigned mcost, unsigned parallelism);
	~Argon2(void);

	void compute(const binary &secret, const binary &salt, binary &key, size_t key_len);

private:
	unsigned mTimeCost;
	unsigned mMemoryCost;
	unsigned mParallelism;
};

class DerSequence
{
public:
	DerSequence(void);
	DerSequence(const binary &b);
	
	binary data(void) const;
	
	DerSequence &operator>> (mpz_t n);
	DerSequence &operator>> (unsigned long &n);
	
	DerSequence &operator<< (const mpz_t n);
	DerSequence &operator<< (unsigned long n);

private:
	static bool readLength(BinaryFormatter &formatter, size_t &length);
	static void writeLength(BinaryFormatter &formatter, size_t length);
	
	BinaryFormatter mFormatter;
};

// Add-on functions for custom mpz import/export
size_t mpz_size_binary(const mpz_t n);
void mpz_import_binary(mpz_t n, const binary &bs);
void mpz_export_binary(const mpz_t n, binary &bs);
void mpz_import_string(mpz_t n, const string &str);
void mpz_export_string(const mpz_t n, string &str);

}

#endif
