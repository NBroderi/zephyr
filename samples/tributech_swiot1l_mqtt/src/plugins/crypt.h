/**
 * @file
 *
 * This file provides the interface and data structures for the crypt plugin type.
 */

#ifndef TT_SDK_PLUGINS_CRYPT_H
#define TT_SDK_PLUGINS_CRYPT_H

#include "../core/compiler.h"
#include "../core/error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the different cryptographic algorithms.
 *
 * Each entry in this list specifies an algorithm (or a combination of algorithms)
 * that may be implemented by the crypto plugin/module.
 */
typedef enum {
    /**
     * @brief RSA with 2048 bit keys, SHA256 message digest, and PKCS1 padding.
     */
    TT_CRYPT_ALGO_SHA256_RSA2048_PKCS1,

    /**
     * @brief ECDSA with P-256 curve, SHA256 message digest, and IEEE P1363.
     */
    TT_CRYPT_ALGO_SHA256_ECDSAP256_IEEEP1363,
} TtCryptAlgorithm;

/**
 * @brief Defines the different key types stored in the crypto module.
 *
 * Each key type has a specific use-case and the plugin must support all defined types.
 */
typedef enum {
    /**
     * @brief The key used to sign the proof hashes.
     */
    TT_CRYPT_KEY_PROOF,
} TtCryptKey;

/**
 * @brief Represents a crypt plugin.
 *
 * A crypt plugin is responsible for multiple tasks related to cryptographic:
 *   - Create new public/private key pairs using a true random number generator.
 *   - Store public/private key pairs securely.
 *   - Encrypt and decrypt data messages.
 *   - Sign arbitrary data.
 *
 * For ideal security the functionality must be implemented in a cryptographic hardware module!
 */
typedef struct
{
    /**
     * @brief A context pointer passed to all functions in the structure.
     *
     * The content of the pointer is never read nor modified by the SDK itself.
     * It is for private use by the plugin itself.
     */
    void* context;

    /**
     * @brief The name of the plugin.
     */
    const char* name;

    /**
     * @brief Initializes the crypto plugin.
     *
     * This can be @c NULL if no initialization of the plugin is needed.
     *
     * @param [in] ctx The context of the plugin.
     * @return An error code.
     */
    TtError (*init)(void* ctx);

    /**
     * @brief Generates random data.
     *
     * @param [in] ctx The context of the plugin.
     * @param [out] buf The buffer to fill.
     * @param [in] len The size of the buffer.
     * @return An error code.
     */
    TtError (*getRand)(void* ctx, void* buf, size_t len);

    /**
     * @brief Calculates the SHA256 hash for the given data.
     *
     * This functionality is optional.
     * The SDK will fall back to a CPU implementation if needed.
     *
     * Since a SHA256 hash has a fixed length there is no need to specify the size of the @ref hash parameter.
     * The caller is responsible to allocate enough data.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] data The data to hash.
     * @param [in] dataLen The @ref data length in bytes.
     * @param [out] hash The buffer to write the hash to.
     * @return @ref TT_E_OK if successful,
     *         @ref TT_E_NOT_SUPPORTED if not supported,
     *         or an error code.
     */
    TtError (*hashSHA265)(void* ctx, const void* data, size_t dataLen, uint8_t* hash);

    /**
     * @brief Checks if the crypto module supports a specific algorithm.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] algo The algorithm to check.
     * @return @ref TT_E_OK if supported,
     *         @ref TT_E_NOT_SUPPORTED if not supported,
     *         or an error code.
     */
    TtError (*supports)(void* ctx, TtCryptAlgorithm algo);

    /**
     * @brief Provides different information about a specific key (if it exists).
     *
     * This function allows some parameters to be @c NULL or @c 0 if the provided information is not needed.
     * The return value reports both errors and different allowed states
     * (the return value @ref TT_E_NOT_FOUND does not indicate an error).
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] key The key type to query the information for.
     * @param [out] algo Set to the key's algorithm (can be @c NULL if not needed).
     * @param [out] pubKey Filled with the public key (can be @c NULL if not needed).
     * @param [in] pubLen The length of @ref pubKey in bytes.
     * @return @ref TT_E_OK if the key exists,
     *         @ref TT_E_NOT_FOUND if the key does not exist,
     *         or an error code.
     */
    TtError (*keyInfo)(void* ctx, TtCryptKey key, TtCryptAlgorithm* algo, char* pubKey, size_t pubLen);

    /**
     * @brief Creates a new public/private key pair.
     *
     * If the given key type already exist the function must fail!
     * The key pair must be created using a true random number generator to be considered secure.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] key The key type to write the generated pair to.
     * @param [in] algo The algorithm to use.
     * @return @ref TT_E_OK if the key pair was created,
     *         @ref TT_E_INVALID_STATE if the key type already exists,
     *         or an error code.
     */
    TtError (*keyCreate)(void* ctx, TtCryptKey key, TtCryptAlgorithm algo);

    /**
     * @brief Signs data using a specific key.
     *
     * This function expects @ref TT_HASH_BYTES bytes as input.
     *
     * @param [in] ctx The context of the plugin.
     * @param [in] key The key to use.
     * @param [in] data The data to sign.
     * @param [in] dataLen The number of bytes in @ref data to sign.
     * @param [out] output The output buffer for the signature.
     * @param [in,out] outLen The length of the output buffer @ref output in bytes,
     *                        updated with the actual output length.
     * @return An error code.
     */
    TtError (*sign)(void* ctx, TtCryptKey key, const void* data, size_t dataLen, void* output, size_t* outLen);
} TtCrypt;

/**
 * @copydoc TtCrypt::getRand()
 *
 * @param buf The buffer to fill.
 * @param len The size of the buffer.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttCryptGetRand(void* buf, size_t len);

/**
 * @copydoc TtCrypt::hashSHA265()
 *
 * @param data The data to hash.
 * @param dataLen The data length in bytes.
 * @param hash The buffer to write the hash to.
 * @return @ref TT_E_OK if successful,
 *         @ref TT_E_NOT_SUPPORTED if not supported,
 *         or an error code.
 */
TtError ttCryptHashSHA265(const void* data, size_t dataLen, uint8_t* hash);

/**
 * @copydoc TtCrypt::supports()
 *
 * @param algo The algorithm to check.
 * @return @ref TT_E_OK if supported,
 *         @ref TT_E_NOT_SUPPORTED if not supported,
 *         or an error code.
 */
TT_NO_DISCARD TtError ttCryptSupports(TtCryptAlgorithm algo);

/**
 * @copydoc TtCrypt::keyInfo()
 *
 * @param key The key type to query the information for.
 * @param algo Set to the key's algorithm (can be @c NULL if not needed).
 * @param pubKey Filled with the public key (can be @c NULL if not needed).
 * @param pubLen The length of @ref pubKey in bytes.
 * @return @ref TT_E_OK if the key exists,
 *         @ref TT_E_NOT_FOUND if the key does not exist,
 *         or an error code.
 */
TT_NO_DISCARD TtError ttCryptKeyInfo(TtCryptKey key, TtCryptAlgorithm* algo, char* pubKey, size_t pubLen);

/**
 * @copydoc TtCrypt::keyCreate()
 *
 * @param key The key type to write the generated pair to.
 * @param algo The algorithm to use.
 * @return @ref TT_E_OK if the key pair was created,
 *         @ref TT_E_INVALID_STATE if the key type already exists,
 *         or an error code.
 */
TT_NO_DISCARD TtError ttCryptKeyCreate(TtCryptKey key, TtCryptAlgorithm algo);

/**
 * @copydoc TtCrypt::sign()
 *
 * @param key The key to use.
 * @param data The data to sign.
 * @param dataLen The number of bytes in @ref data to sign.
 * @param output The output buffer for the signature.
 * @param outLen The length of the output buffer @ref output in bytes,
 *               updated with the actual output length.
 * @return An error code.
 */
TT_NO_DISCARD TtError ttCryptSign(TtCryptKey key, const void* data, size_t dataLen, void* output, size_t* outLen);

/**
 * @brief Helper function to convert a cryptographic algorithm into a string.
 *
 * This function always returns a valid string and can therefore be used without additional checks.
 * A special string is returned for unknown cryptographic algorithms.
 *
 * @param algo The algorithm to convert.
 * @return The string representation.
 */
TT_NO_DISCARD const char* ttCryptAlgoToString(TtCryptAlgorithm algo);

#ifdef __cplusplus
}
#endif

#endif
