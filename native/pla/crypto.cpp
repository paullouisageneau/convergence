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

#include "pla/crypto.hpp"
#include "pla/binary.hpp"
#include "pla/binaryformatter.hpp"
#include "pla/random.hpp"

#include <gmp.h>
#include <nettle/hmac.h>
#include <nettle/pbkdf2.h>

#ifdef WINDOWS
#include "win32/argon2.h"
#else
#include <argon2.h>
#endif

#ifdef pbkdf2
#undef pbkdf2
#endif

namespace pla {

inline uint8_t *u8(byte *data) { return reinterpret_cast<uint8_t *>(data); }

inline const uint8_t *u8(const byte *data) { return reinterpret_cast<const uint8_t *>(data); }

inline uint8_t *u8(unsigned char *data) { return reinterpret_cast<uint8_t *>(data); }

inline const uint8_t *u8(const unsigned char *data) {
	return reinterpret_cast<const uint8_t *>(data);
}

void Hash::compute(const binary &data, binary &digest) {
	init();
	process(data);
	finalize(digest);
}

binary Hash::compute(const binary &data) {
	binary result;
	compute(data, result);
	return result;
}

bool Hash::rsaVerify(const binary &digest, const struct rsa_public_key *key,
                     const mpz_t signature) {
	throw std::logic_error("Unable to use the specified hash for RSA verification");
}

bool Hash::rsaSign(const binary &digest, const struct rsa_private_key *key, mpz_t signature) {
	throw std::logic_error("Unable to use the specified hash for RSA signature");
}

size_t Sha1::length(void) const { return size_t(SHA1_DIGEST_SIZE); }

void Sha1::init(void) { sha1_init(&mCtx); }

void Sha1::process(const binary &data) { sha1_update(&mCtx, data.size(), u8(data.data())); }

void Sha1::finalize(binary &digest) {
	digest.resize(length());
	sha1_digest(&mCtx, digest.size(), u8(digest.data()));
}

void Sha1::mac(const binary &message, const binary &key, binary &digest) {
	struct hmac_sha1_ctx ctx;
	hmac_sha1_set_key(&ctx, key.size(), u8(key.data()));
	hmac_sha1_update(&ctx, message.size(), u8(message.data()));

	digest.resize(length());
	hmac_sha1_digest(&ctx, digest.size(), u8(digest.data()));
}

void Sha1::pbkdf2(const binary &secret, const binary &salt, binary &key, size_t key_len,
                  unsigned iterations) {
	Assert(iterations > 0);
	key.resize(key_len);
	pbkdf2_hmac_sha1(secret.size(), u8(secret.data()), iterations, salt.size(), u8(salt.data()),
	                 key.size(), u8(key.data()));
}

bool Sha1::rsaVerify(const binary &digest, const struct rsa_public_key *key,
                     const mpz_t signature) {
	if (digest.size() != length())
		return false;
	return rsa_sha1_verify_digest(key, u8(digest.data()), signature) != 0;
}

bool Sha1::rsaSign(const binary &digest, const struct rsa_private_key *key, mpz_t signature) {
	if (digest.size() != length())
		return false;
	return rsa_sha1_sign_digest(key, u8(digest.data()), signature) != 0;
}

size_t Sha256::length(void) const { return size_t(SHA256_DIGEST_SIZE); }

void Sha256::init(void) { sha256_init(&mCtx); }

void Sha256::process(const binary &data) { sha256_update(&mCtx, data.size(), u8(data.data())); }

void Sha256::finalize(binary &digest) {
	digest.resize(length());
	sha256_digest(&mCtx, digest.size(), u8(digest.data()));
}

void Sha256::mac(const binary &message, const binary &key, binary &digest) {
	struct hmac_sha256_ctx ctx;
	hmac_sha256_set_key(&ctx, key.size(), u8(key.data()));
	hmac_sha256_update(&ctx, message.size(), u8(message.data()));

	digest.resize(length());
	hmac_sha256_digest(&ctx, digest.size(), u8(digest.data()));
}

void Sha256::pbkdf2(const binary &secret, const binary &salt, binary &key, size_t key_len,
                    unsigned iterations) {
	Assert(iterations > 0);
	key.resize(key_len);
	pbkdf2_hmac_sha256(secret.size(), u8(secret.data()), iterations, salt.size(), u8(salt.data()),
	                   key.size(), u8(key.data()));
}

bool Sha256::rsaVerify(const binary &digest, const struct rsa_public_key *key,
                       const mpz_t signature) {
	if (digest.size() != length())
		return false;
	return rsa_sha256_verify_digest(key, u8(digest.data()), signature) != 0;
}

bool Sha256::rsaSign(const binary &digest, const struct rsa_private_key *key, mpz_t signature) {
	if (digest.size() != length())
		return false;
	return rsa_sha256_sign_digest(key, u8(digest.data()), signature) != 0;
}

size_t Sha512::length(void) const { return size_t(SHA512_DIGEST_SIZE); }

void Sha512::init(void) { sha512_init(&mCtx); }

void Sha512::process(const binary &data) { sha512_update(&mCtx, data.size(), u8(data.data())); }

void Sha512::finalize(binary &digest) {
	digest.resize(length());
	sha512_digest(&mCtx, digest.size(), u8(digest.data()));
}

void Sha512::mac(const binary &message, const binary &key, binary &digest) {
	struct hmac_sha512_ctx ctx;
	hmac_sha512_set_key(&ctx, key.size(), u8(key.data()));
	hmac_sha512_update(&ctx, message.size(), u8(message.data()));

	digest.resize(length());
	hmac_sha512_digest(&ctx, digest.size(), u8(digest.data()));
}

void Sha512::pbkdf2(const binary &secret, const binary &salt, binary &key, size_t key_len,
                    unsigned iterations) {
	Assert(iterations != 0);

	struct hmac_sha512_ctx ctx;
	hmac_sha512_set_key(&ctx, secret.size(), u8(secret.data()));

	key.resize(key_len);
#define pbkdf2 nettle_pbkdf2 // fix name conflict with nettle
	PBKDF2(&ctx, hmac_sha512_update, hmac_sha512_digest, 64, iterations, salt.size(),
	       u8(salt.data()), key.size(), u8(key.data()));
#undef pbkdf2
}

bool Sha512::rsaVerify(const binary &digest, const struct rsa_public_key *key,
                       const mpz_t signature) {
	if (digest.size() != length())
		return false;
	return rsa_sha512_verify_digest(key, u8(digest.data()), signature) != 0;
}

bool Sha512::rsaSign(const binary &digest, const struct rsa_private_key *key, mpz_t signature) {
	if (digest.size() != length())
		return false;
	return rsa_sha512_sign_digest(key, u8(digest.data()), signature) != 0;
}

size_t Sha3_256::length(void) const { return size_t(SHA3_256_DIGEST_SIZE); }

void Sha3_256::init(void) { sha3_256_init(&mCtx); }

void Sha3_256::process(const binary &data) { sha3_256_update(&mCtx, data.size(), u8(data.data())); }

void Sha3_256::finalize(binary &digest) {
	digest.resize(length());
	sha3_256_digest(&mCtx, digest.size(), u8(digest.data()));
}

void Sha3_256::mac(const binary &message, const binary &key, binary &digest) {
	init();
	process(key); // key first
	process(message);
	finalize(digest);
}

size_t Sha3_512::length(void) const { return size_t(SHA3_256_DIGEST_SIZE); }

void Sha3_512::init(void) { sha3_512_init(&mCtx); }

void Sha3_512::process(const binary &data) { sha3_512_update(&mCtx, data.size(), u8(data.data())); }

void Sha3_512::finalize(binary &digest) {
	digest.resize(length());
	sha3_512_digest(&mCtx, digest.size(), u8(digest.data()));
}

void Sha3_512::mac(const binary &message, const binary &key, binary &digest) {
	init();
	process(key); // key first
	process(message);
	finalize(digest);
}

Cipher::Cipher(Stream *stream, bool ownership) : mStream(stream), mOwnership(ownership) {
	Assert(mStream);
}

Cipher::~Cipher(void) {
	if (mOwnership)
		delete mStream;
}

size_t Cipher::readSome(byte *buffer, size_t size) {
	if (mReadBlock.empty()) {
		mStream->read(mReadBlock, blockSize());
		decryptBlock(mReadBlock);
	}

	auto begin = mReadBlock.begin();
	size = std::min(size, mReadBlock.size());
	std::copy(begin, begin + size, buffer);
	mReadBlock.erase(begin, begin + size);
	return size;
}

size_t Cipher::writeSome(const byte *data, size_t size) {
	mWriteBlock.insert(mWriteBlock.end(), data, data + size);

	auto it = mWriteBlock.begin();
	while (it != mWriteBlock.end()) {
		size_t left = mWriteBlock.end() - it;
		if (left < blockSize())
			break;

		binary block(it, it + blockSize());
		encryptBlock(block);
		mStream->write(block);

		it += blockSize();
	}
	mWriteBlock.assign(it, mWriteBlock.end());

	return size;
}

void Cipher::close(void) {
	// Finish encryption
	if (!mWriteBlock.empty()) {
		encryptBlock(mWriteBlock);
		mStream->write(mWriteBlock);
		mWriteBlock.clear();
	}

	mStream->close();
}

AesGcm256::AesGcm256(Stream *stream, bool ownership) : Cipher(stream, ownership) {}

AesGcm256::~AesGcm256(void) {
	close(); // must be called here and not in Cipher
}

void AesGcm256::setEncryptionKey(const binary &key) {
	if (key.size() != 32)
		throw std::runtime_error("Incorrect AES256 key length");
	gcm_aes256_set_key(&mCtx, u8(key.data()));
}

void AesGcm256::setDecryptionKey(const binary &key) {
	if (key.size() != 32)
		throw std::runtime_error("Incorrect AES256 key length");
	gcm_aes256_set_key(&mCtx, u8(key.data()));
}

void AesGcm256::setInitializationVector(const binary &iv) {
	if (iv.size() < GCM_IV_SIZE)
		throw std::runtime_error("Insufficient AES IV length");
	gcm_aes256_set_iv(&mCtx, GCM_IV_SIZE, u8(iv.data()));
}

bool AesGcm256::getAuthenticationTag(binary &tag) {
	tag.resize(GCM_DIGEST_SIZE);
	gcm_aes256_digest(&mCtx, GCM_DIGEST_SIZE, u8(tag.data()));
	return true;
}

size_t AesGcm256::blockSize(void) const { return GCM_BLOCK_SIZE; }

void AesGcm256::encryptBlock(binary &block) {
	uint8_t *ptr = u8(block.data());
	gcm_aes256_update(&mCtx, block.size(), ptr);
	gcm_aes256_encrypt(&mCtx, block.size(), ptr, ptr);
}

void AesGcm256::decryptBlock(binary &block) {
	uint8_t *ptr = u8(block.data());
	gcm_aes256_update(&mCtx, block.size(), ptr);
	gcm_aes256_decrypt(&mCtx, block.size(), ptr, ptr);
}

Rsa::PublicKey::PublicKey(void) {
	rsa_public_key_init(&mPub);
	mPub.size = 0;
}

Rsa::PublicKey::PublicKey(const Rsa::PublicKey &key) : PublicKey() { *this = key; }

Rsa::PublicKey::PublicKey(gnutls_x509_crt_t crt) {
	if (gnutls_x509_crt_get_pk_algorithm(crt, NULL) != GNUTLS_PK_RSA)
		throw std::runtime_error("Certificate public key algorithm is not RSA");

	rsa_public_key_init(&mPub);
	mPub.size = 0;

	try {
		gnutls_datum_t n, e;

		int ret = gnutls_x509_crt_get_pk_rsa_raw(crt, &n, &e);
		if (ret != GNUTLS_E_SUCCESS)
			throw std::runtime_error("Key exportation failed: " + string(gnutls_strerror(ret)));

		mpz_import(mPub.n, n.size, 1, 1, 1, 0, n.data); // big endian
		mpz_import(mPub.e, e.size, 1, 1, 1, 0, e.data); // big endian
		gnutls_free(n.data);
		gnutls_free(e.data);

		if (!rsa_public_key_prepare(&mPub))
			throw std::runtime_error("Invalid parameters");
	} catch (const std::exception &e) {
		rsa_public_key_clear(&mPub);
		throw std::runtime_error("Unable to get RSA public key from x509 certificate: " +
		                         string(e.what()));
	}
}

Rsa::PublicKey::~PublicKey(void) { rsa_public_key_clear(&mPub); }

Rsa::PublicKey &Rsa::PublicKey::operator=(const Rsa::PublicKey &key) {
	mPub.size = key.mPub.size;
	mpz_set(mPub.n, key.mPub.n);
	mpz_set(mPub.e, key.mPub.e);
	return *this;
}

bool Rsa::PublicKey::operator==(const PublicKey &key) const {
	return mpz_cmp(mPub.n, key.mPub.n) == 0 && mpz_cmp(mPub.e, key.mPub.e) == 0;
}

bool Rsa::PublicKey::isNull(void) const { return (mPub.size == 0); }

void Rsa::PublicKey::clear(void) {
	rsa_public_key_clear(&mPub);
	rsa_public_key_init(&mPub);
	mPub.size = 0;
}

binary Rsa::PublicKey::fingerprint(Hash &hash) const {
	binary digest;
	hash.compute(toBinary(), digest);
	return digest;
}

bool Rsa::PublicKey::verify(Hash &hash, const binary &digest, const binary &signature) const {
	if (digest.empty())
		throw std::runtime_error("Empty digest used for RSA verification");

	mpz_t s;
	mpz_init(s);
	bool success = false;
	try {
		mpz_import_binary(s, signature);
		success = hash.rsaVerify(digest, &mPub, s);
	} catch (...) {
		mpz_clear(s);
		throw;
	}

	mpz_clear(s);
	return success;
}

string Rsa::PublicKey::toString() const {
	// PEM
	return to_base64(toBinary());
}

void Rsa::PublicKey::fromString(string str) { fromBinary(from_base64(str)); }

binary Rsa::PublicKey::toBinary() const {
	// DER
	DerSequence sequence;
	sequence << mPub.n << mPub.e;
	return sequence.data();
}

void Rsa::PublicKey::fromBinary(binary b) {
	clear();

	DerSequence sequence(b);
	sequence >> mPub.n >> mPub.e;

	if (!rsa_public_key_prepare(&mPub))
		throw std::runtime_error("Invalid parameters");
}

Rsa::PrivateKey::PrivateKey(void) {
	rsa_private_key_init(&mPriv);
	mPriv.size = 0;
}

Rsa::PrivateKey::PrivateKey(const PrivateKey &key) {
	rsa_private_key_init(&mPriv);
	mPriv.size = 0;
	*this = key;
}

Rsa::PrivateKey::~PrivateKey(void) { rsa_private_key_clear(&mPriv); }

Rsa::PrivateKey &Rsa::PrivateKey::operator=(const Rsa::PrivateKey &key) {
	mPriv.size = key.mPriv.size;
	mpz_set(mPriv.d, key.mPriv.d);
	mpz_set(mPriv.p, key.mPriv.p);
	mpz_set(mPriv.q, key.mPriv.q);
	mpz_set(mPriv.a, key.mPriv.a);
	mpz_set(mPriv.b, key.mPriv.b);
	mpz_set(mPriv.c, key.mPriv.c);
	return *this;
}

void Rsa::PrivateKey::clear(void) {
	rsa_private_key_clear(&mPriv);
	rsa_private_key_init(&mPriv);
	mPriv.size = 0;
}

void Rsa::PrivateKey::sign(Hash &hash, const binary &digest, binary &signature) const {
	if (digest.empty())
		throw std::runtime_error("Empty digest used for RSA signature");

	mpz_t s;
	mpz_init(s);

	try {
		bool success = hash.rsaSign(digest, &mPriv, s);
		if (!success)
			throw std::runtime_error("RSA signature failed");
		mpz_export_binary(s, signature);
	} catch (...) {
		mpz_clear(s);
		throw;
	}

	mpz_clear(s);
}

string Rsa::PrivateKey::toString() const { return to_base64(toBinary()); }

void Rsa::PrivateKey::fromString(string str) { fromBinary(from_base64(str)); }

binary Rsa::PrivateKey::toBinary() const {
	unsigned long version = 0;
	DerSequence sequence;
	sequence << version;
	sequence << mPub.n << mPub.e;
	sequence << mPriv.d << mPriv.p << mPriv.q << mPriv.a << mPriv.b << mPriv.c;
	return sequence.data();
}

void Rsa::PrivateKey::fromBinary(binary b) {
	clear();

	unsigned long version = 0;
	DerSequence sequence(b);
	sequence >> version;
	sequence >> mPub.n >> mPub.e;
	sequence >> mPriv.d >> mPriv.p >> mPriv.q >> mPriv.a >> mPriv.b >> mPriv.c;

	if (!rsa_public_key_prepare(&mPub))
		throw std::runtime_error("Invalid parameters");

	if (!rsa_private_key_prepare(&mPriv))
		throw std::runtime_error("Invalid parameters");
}

Rsa::Rsa(unsigned bits) : mBits(bits) {
	Assert(bits >= 1024);
	Assert(bits <= 16384);
}

Rsa::~Rsa(void) {}

void Rsa::generate(PrivateKey &priv) {
	// Use exponent 65537 for compatibility and performance
	const unsigned long exponent = 65537;
	mpz_set_ui(priv.mPub.e, exponent);

	if (!rsa_generate_keypair(&priv.mPub, &priv.mPriv, NULL, Random::wrapperKey, NULL, NULL, mBits,
	                          0 /*e already set*/))
		throw std::runtime_error("RSA keypair generation failed (size=" + to_string(mBits) + ")");
}

void Rsa::CreateCertificate(gnutls_x509_crt_t crt, gnutls_x509_privkey_t key,
                            const PrivateKey &priv, const string &name) {
	if (priv.isNull())
		throw std::runtime_error("Creating certificate from null key pair");

	binary b_n;
	mpz_export_binary(priv.mPub.n, b_n);
	binary b_e;
	mpz_export_binary(priv.mPub.e, b_e);
	binary b_d;
	mpz_export_binary(priv.mPriv.d, b_d);
	binary b_p;
	mpz_export_binary(priv.mPriv.p, b_p);
	binary b_q;
	mpz_export_binary(priv.mPriv.q, b_q);
	binary b_c;
	mpz_export_binary(priv.mPriv.c, b_c);

	gnutls_datum_t n;
	n.data = u8(b_n.data());
	n.size = b_n.size();
	gnutls_datum_t e;
	e.data = u8(b_e.data());
	e.size = b_e.size();
	gnutls_datum_t d;
	d.data = u8(b_d.data());
	d.size = b_d.size();
	gnutls_datum_t p;
	p.data = u8(b_p.data());
	p.size = b_p.size();
	gnutls_datum_t q;
	q.data = u8(b_q.data());
	q.size = b_q.size();
	gnutls_datum_t c;
	c.data = u8(b_c.data());
	c.size = b_c.size();

	int ret = gnutls_x509_privkey_import_rsa_raw(key, &n, &e, &d, &p, &q, &c);
	if (ret != GNUTLS_E_SUCCESS)
		throw std::runtime_error("Unable to convert RSA key pair to X509: " +
		                         string(gnutls_strerror(ret)));

	using clock = std::chrono::system_clock;
	gnutls_x509_crt_set_activation_time(crt,
	                                    clock::to_time_t(clock::now() - std::chrono::hours(1)));
	gnutls_x509_crt_set_expiration_time(
	    crt, clock::to_time_t(clock::now() + std::chrono::hours(24 * 365)));
	gnutls_x509_crt_set_version(crt, 1);
	gnutls_x509_crt_set_key(crt, key);
	gnutls_x509_crt_set_dn_by_oid(crt, GNUTLS_OID_X520_COMMON_NAME, 0, name.data(), name.size());

	const size_t serialSize = 16;
	byte serial[serialSize];
	Random(Random::Nonce).read(serial, serialSize);
	gnutls_x509_crt_set_serial(crt, serial, serialSize);
}

void Rsa::SignCertificate(gnutls_x509_crt_t crt, gnutls_x509_crt_t issuer,
                          gnutls_x509_privkey_t issuerKey) {
	int ret = gnutls_x509_crt_sign2(crt, issuer, issuerKey, GNUTLS_DIG_SHA256, 0);
	if (ret != GNUTLS_E_SUCCESS)
		throw std::runtime_error("Unable to sign X509 certificate: " +
		                         string(gnutls_strerror(ret)));
}

unsigned Argon2::DefaultTimeCost = 3;         // 3 pass
unsigned Argon2::DefaultMemoryCost = 1 << 16; // 64 MiB
unsigned Argon2::DefaultParallelism = 2;      // 2 threads

Argon2::Argon2(void)
    : mTimeCost(DefaultTimeCost), mMemoryCost(DefaultMemoryCost), mParallelism(DefaultParallelism) {

}

Argon2::Argon2(unsigned tcost, unsigned mcost, unsigned parallelism)
    : mTimeCost(tcost), mMemoryCost(mcost), mParallelism(parallelism) {}

Argon2::~Argon2(void) {}

void Argon2::compute(const binary &secret, const binary &salt, binary &key, size_t key_len) {
	key.resize(key_len);
	if (argon2i_hash_raw(uint32_t(mTimeCost), uint32_t(mMemoryCost), uint32_t(mParallelism),
	                     secret.data(), secret.size(), salt.data(), salt.size(), key.data(),
	                     key.size()) != ARGON2_OK)
		throw std::runtime_error("Argon2 password hashing failed");
}

DerSequence::DerSequence(void) {}

DerSequence::DerSequence(const binary &b) {
	BinaryFormatter sequence(b);

	uint8_t type = 0;
	if (!(sequence >> type))
		throw std::invalid_argument("Truncated DER sequence");
	if (type != 0x30)
		throw std::invalid_argument("Expected DER sequence");

	size_t length = 0;
	if (!readLength(sequence, length))
		throw std::invalid_argument("Truncated DER sequence");

	binary content = sequence.remaining();
	if (content.size() > length)
		content.resize(length);
	mFormatter.data(content);
}

binary DerSequence::data(void) const {
	const binary content = mFormatter.data();

	BinaryFormatter sequence;
	sequence << uint8_t(0x30); // SEQUENCE
	writeLength(sequence, content.size());
	sequence << content;

	return sequence.data();
}

DerSequence &DerSequence::operator>>(mpz_t n) {
	uint8_t type = 0;
	if (!(mFormatter >> type))
		throw std::invalid_argument("Truncated DER sequence");
	if (type != 0x02)
		throw std::invalid_argument("Expected DER integer");

	size_t length = 0;
	if (!readLength(mFormatter, length))
		throw std::invalid_argument("Truncated DER sequence");
	if (length == size_t(-1))
		throw std::invalid_argument("Unexpected DER indefinite length");

	binary b(length);
	if (!(mFormatter >> b))
		throw std::invalid_argument("Truncated DER sequence");
	mpz_import_binary(n, b);
	return *this;
}

DerSequence &DerSequence::operator>>(unsigned long &n) {
	mpz_t m;
	mpz_init(m);
	try {
		*this >> m;
		n = mpz_get_ui(m);
	} catch (...) {
		mpz_clear(m);
		throw;
	}
	mpz_clear(m);
	return *this;
}

DerSequence &DerSequence::operator<<(const mpz_t n) {
	mFormatter << uint8_t(0x02); // INTEGER
	binary b;
	mpz_export_binary(n, b);
	writeLength(mFormatter, b.size());
	mFormatter << b;
	return *this;
}

DerSequence &DerSequence::operator<<(unsigned long n) {
	mpz_t m;
	mpz_init(m);
	try {
		mpz_set_ui(m, n);
		*this << m;
	} catch (...) {
		mpz_clear(m);
		throw;
	}
	mpz_clear(m);
	return *this;
}

bool DerSequence::readLength(BinaryFormatter &formatter, size_t &length) {
	uint8_t b = 0;
	if (!(formatter >> b))
		return false;
	if (!(b & 0x80)) // short length
	{
		length = size_t(b);
		return true;
	}

	// long length
	size_t len(b & 0x7F);
	switch (len) {
	case 0: {
		// Indefinite
		length = size_t(-1);
		break;
	}
	case 1: {
		uint8_t tmp = 0;
		if (!(formatter >> tmp))
			throw std::invalid_argument("Truncated DER size");
		length = size_t(tmp);
		break;
	}
	case 2: {
		uint16_t tmp = 0;
		if (!(formatter >> tmp))
			throw std::invalid_argument("Truncated DER size");
		length = size_t(tmp);
		break;
	}
	case 4: {
		uint32_t tmp = 0;
		if (!(formatter >> tmp))
			throw std::invalid_argument("Truncated DER size");
		length = size_t(tmp);
		break;
	}
	default: {
		binary tmp(len);
		if (!(formatter >> tmp))
			throw std::invalid_argument("Truncated DER size");
		mpz_t m;
		mpz_init(m);
		mpz_import_binary(m, tmp);
		length = size_t(mpz_get_ui(m));
		mpz_clear(m);
	}
	}

	return true;
}

void DerSequence::writeLength(BinaryFormatter &formatter, size_t length) {
	if (length <= 0x7F) { // short length
		formatter << uint8_t(length);
	} else if (length <= std::numeric_limits<uint16_t>::max()) {
		formatter << (uint8_t(0x80) & uint8_t(2)); // long length (2 bytes)
		formatter << uint16_t(length);
	} else {
		Assert(length <= std::numeric_limits<uint32_t>::max());
		formatter << (uint8_t(0x80) & uint8_t(4)); // long length (4 bytes)
		formatter << uint32_t(length);
	}
}

size_t mpz_size_binary(const mpz_t n) { return (mpz_sizeinbase(n, 2) + 7) / 8; }

void mpz_import_binary(mpz_t n, const binary &bs) {
	mpz_import(n, bs.size(), 1, 1, 1, 0, bs.data()); // big endian
}

void mpz_export_binary(const mpz_t n, binary &bs) {
	size_t size = mpz_size_binary(n);
	bs.resize(size);

	size_t len = 0;
	mpz_export(bs.data(), &len, 1, 1, 1, 0, n); // big endian

	if (len == 0)
		bs.clear();
	Assert(len == bs.size());
}

void mpz_import_string(mpz_t n, const string &str) {
	if (mpz_set_str(n, str.c_str(), 16))
		throw std::invalid_argument("Invalid hexadecimal number: " + str);
}

void mpz_export_string(const mpz_t n, string &str) {
	size_t size = mpz_sizeinbase(n, 16) + 2;
	str.resize(size);
	mpz_get_str(&str[0], 16, n);
	str.resize(std::strlen(str.c_str()));
}

} // namespace pla
