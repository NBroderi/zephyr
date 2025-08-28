/**
 * crypt_zephyr.c
 *
 * Port of cryptographic plugin functions for Zephyr.
 * Uses pure C (no C++ STL), mbedTLS for RSA, and obtains randomness
 * Replace the maxq1065_get_random() stub with actual cryptoprocessor call.
 *
 * This code uses local instances of the entropy and CTR-DRBG contexts inside
 * key creation and signing for simplicity. Might want to create shared contexts
 * (while keeping thread safety in mind) to reduce stack usage. via the MAXQ1065
 * cryptoprocessor.
 *
 * SDK’s header "sdk/plugins/crypt.h" defines:
 *   - The TtCrypt structure,
 *   - The TtError type and error codes (TT_E_OK, TT_E_FAULT,
 * TT_E_NOT_SUPPORTED, TT_E_NOT_FOUND, TT_E_INVALID_STATE, TT_E_INVALID_ARG),
 *   - The TtCryptAlgorithm and TtCryptKey enumerations (with at least
 * TT_CRYPT_ALGO_SHA256_RSA2048_PKCS1 and TT_CRYPT_KEY_PROOF).
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "mbedtls/ecdsa.h"
#include "mbedtls/asn1.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/md.h"
#include "mbedtls/pk.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"

#include "../../../include/tt_sdk/plugins/crypt.h" // SDK header defining TtCrypt, TtError, etc.
#include "../../../include/tt_sdk/plugins/logging.h"
#include "../../../include/tt_sdk/core/error.h" // SDK header defining TtError codes
#include "../../../include/tt_sdk/core/compiler.h" // SDK header defining TT_NO_INPUT
#include "../../../include/tt_sdk/core/common.h" // SDK header defining TT_E_OK, TT_E_FAULT, etc.

#include <tt_sdk/plugins/flashfs.h>
#include <zephyr/fs/fs.h>

#define BASE_DIR "/tt_flashfs"
#define FILENAME_PREFIX "crypt."
#define FILENAME_SUFFIX ".bin"
#define INIT_FILENAME "cryptinit.bin"
#define MAX_PATH_LEN 64
#define MAX_KEY_LEN 2048
#define FILENAME_FMT_2 "%s/%s"

extern TtState newState_local;
static char filename[MAX_PATH_LEN] = "";
static const char *pers = "tt_crypt_rng";

struct Key {
  int used;
  mbedtls_pk_context pkey;
};

static struct Key keyProof = {0};
char key_buf_private[MAX_KEY_LEN];
char key_buf_public[MAX_KEY_LEN];

static TtError FreeRTOSGetRand(void *ctx, void *buf, size_t len);
static TtError FreeRTOSHashSHA256(void *ctx, const void *data, size_t dataLen,
                                  uint8_t *hash);
static TtError FreeRTOSSupports(void *ctx, TtCryptAlgorithm algo);
static TtError FreeRTOSKeyInfo(void *ctx, TtCryptKey key,
                               TtCryptAlgorithm *algo, char *pubKey,
                               size_t pubLen);
static TtError FreeRTOSKeyCreate(void *ctx, TtCryptKey key,
                                 TtCryptAlgorithm algo);
static TtError FreeRTOSSign(void *ctx, TtCryptKey key, const void *data,
                            size_t dataLen, void *output, size_t *outLen);
static TtError FreeRTOSCryptInit(void *ctx);

const TtCrypt ttPluginCrypt = {
    .context = NULL,
    .name = "freertos",
    .init = &FreeRTOSCryptInit,
    .getRand = &FreeRTOSGetRand,
    .hashSHA265 = &FreeRTOSHashSHA256,
    .supports = &FreeRTOSSupports,
    .keyInfo = &FreeRTOSKeyInfo,
    .keyCreate = &FreeRTOSKeyCreate,
    .sign = &FreeRTOSSign,
};

static struct Key *getKeyPtr(TtCryptKey key) {
  switch (key) {
  case TT_CRYPT_KEY_PROOF:
    return &keyProof;
  }
  return NULL;
}

// static int initLittleFS(void) {
//   // Assume LittleFS is initialized externally and lfs pointer is set
//   // This is a placeholder; actual initialization depends on MAX32650 setup
//   return 0;
// }

static int askUseInitFile(void) {
#ifdef TT_NO_INPUT
  printf("Ignoring file with initial crypt content...\n");
  return 0;
#else
  // Embedded systems typically don't have interactive input
  // Default to not using init file; override via configuration if needed
  return 0;
#endif
}

static int extractKey(const char *content, size_t content_len,
                      const char *begin, const char *end, char *out,
                      size_t out_len) {
  const char *start = strstr(content, begin);
  if (!start)
    return -1;

  const char *end_ptr = strstr(start, end);
  if (!end_ptr)
    return -1;

  size_t len = end_ptr - start + strlen(end);
  if (len >= out_len)
    return -1;

  strncpy(out, start, len);
  out[len] = '\0';
  return 0;
}

static int loadKeys(void) {
  struct fs_file_t file;
  struct fs_dirent info;
  char content[MAX_KEY_LEN];
  int res;

  // Check if the file exists and has content
  if (fs_stat(filename, &info) == 0 && info.size > 0) {
    // Open in read mode
    res = fs_open(&file, filename, FS_O_READ);
    if (res < 0) {
      zephyrLog(NULL, TT_LL_INFO, "INIT_FILENAME \"cryptinit.bin\" does not exist or is empty");
      return -1;
    }
  }
  else {
    zephyrLog(NULL, TT_LL_INFO, "INIT_FILENAME \"cryptinit.bin\" does not exist or is empty");
    return -1;
  }

  int16_t content_len = fs_file_read(&file, content, sizeof(content) - 1);
  fs_close(&file);
  if (content_len <= 0)
    return -1;
  content[content_len] = '\0';
  zephyrLog(NULL, TT_LL_INFO, "Loaded content from cryptinit.bin");
  // Check if the content is empty
  if (content_len == 0) {
    zephyrLog(NULL, TT_LL_INFO, "No content in cryptinit.bin, skipping key loading.");
    return -1;
  }
  // Check if the content is valid
  if (strstr(content, "-----BEGIN EC PRIVATE KEY-----") == NULL ||
      strstr(content, "-----END EC PRIVATE KEY-----") == NULL ||
      strstr(content, "-----BEGIN PUBLIC KEY-----") == NULL ||
      strstr(content, "-----END PUBLIC KEY-----") == NULL) {
    zephyrLog(NULL, TT_LL_ERROR, "Invalid content in cryptinit.bin, expected PEM format.");
    return -1;
  }

  // Prepare buffer for keys
  // Ensure the buffer is large enough for both keys
  if (content_len >= MAX_KEY_LEN) {
    zephyrLog(NULL, TT_LL_ERROR, "Content length exceeds maximum key length.");
    return -1;
  }

  // Extract private and public keys from the content
  // Use a fixed-size buffer for keys
  // This assumes the keys are in PEM format and fit within MAX_KEY_LEN
  // If the keys are larger, this will need to be adjusted
  // Initialize keyProof
  keyProof.used = 0; // Reset used flag
  mbedtls_pk_free(&keyProof.pkey); // Free any existing key context
  mbedtls_pk_init(&keyProof.pkey); // Initialize the key context

  memset(key_buf_private, 0, sizeof(key_buf_private));
  mbedtls_pk_context pkey_private;
  mbedtls_pk_init(&pkey_private);
  mbedtls_pk_context pkey_public;
  mbedtls_pk_init(&pkey_public);

  // Load private key
  if (extractKey(content, content_len, "-----BEGIN EC PRIVATE KEY-----\n",
                 "-----END EC PRIVATE KEY-----\n", key_buf_private,
                 sizeof(key_buf_private)) == 0) 
  {
    if (mbedtls_pk_parse_key(&keyProof.pkey, (const unsigned char *)key_buf_private,
                             strlen(key_buf_private) + 1, NULL, 0, FreeRTOSGetRand, 0) == 0) 
    {
      // log success
      zephyrLog(NULL, TT_LL_INFO, "Private key read successfully from cryptinit.bin");
    } 
    else 
    {
      zephyrLog(NULL, TT_LL_ERROR, "Invalid private key read!");
    }
  }
  else 
  {
      zephyrLog(NULL, TT_LL_ERROR, "Invalid private read!");
  }

  memset(key_buf_public, 0, sizeof(key_buf_public));
  // Load public key
  if (extractKey(content, content_len, "-----BEGIN PUBLIC KEY-----\n",
          "-----END PUBLIC KEY-----\n", key_buf_public, sizeof(key_buf_public)) == 0) 
  {
    if (mbedtls_pk_parse_public_key(&pkey_public, (const unsigned char *)key_buf_public,
          strlen(key_buf_public) + 1) == 0) 
    {
      zephyrLog(NULL, TT_LL_INFO, "Keys loaded and valid from cryptinit.bin");
    } 
    else 
    {
      zephyrLog(NULL, TT_LL_ERROR, "Failed to public key components!");
    }
  } 
  else 
  {
      zephyrLog(NULL, TT_LL_ERROR, "Invalid public key read!");
  }
  
  
//res = combine_ec_keys_into_pk_context(&pkey_private, &pkey_public, &keyPtr->pkey);

//if (res == 0) {
    //zephyrLog(NULL, TT_LL_INFO, "Combined EC key successfully!");
    // Assign the combined key to keyProof
    //if (mbedtls_pk_setup(&keyPtr->pkey, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)) == 0) {
        keyProof.used = 1; // Mark key as used
        zephyrLog(NULL, TT_LL_INFO, "Keys loaded successfully from cryptinit.bin");
    //} else {
    //    zephyrLog(NULL, TT_LL_ERROR, "Failed to set up keyProof.pkey");
    //}
//} else {
//    char err_buf[128];
//    mbedtls_strerror(res, err_buf, sizeof(err_buf));
//    zephyrLog(NULL, TT_LL_ERROR, "Error combining keys: %s\n", err_buf);
//}

  mbedtls_pk_free(&pkey_private);
  mbedtls_pk_free(&pkey_public);
  // Free the key context if it was not used
  if (!keyProof.used) {
    mbedtls_pk_free(&keyProof.pkey);
    mbedtls_pk_init(&keyProof.pkey); // Reinitialize to avoid dangling pointer
  }


  // Check if keyProof is used
  if (keyProof.used) {
    return 0; // Return 0 if keys loaded successfully
  } else {
    return -1; // Return -1 if keys failed to load
  }

}

int combine_ec_keys_into_pk_context(
    const mbedtls_pk_context *pkey_private,
    const mbedtls_pk_context *pkey_public,
    mbedtls_pk_context *pkey_combined)
{
    int ret;
    mbedtls_ecp_keypair *ec_priv, *ec_pub, *ec_combined;

    mbedtls_pk_init(pkey_combined);

    if (!mbedtls_pk_can_do(pkey_private, MBEDTLS_PK_ECKEY) ||
        !mbedtls_pk_can_do(pkey_public, MBEDTLS_PK_ECKEY)) {
        return MBEDTLS_ERR_PK_TYPE_MISMATCH;
    }

    ec_priv = mbedtls_pk_ec(*pkey_private);
    ec_pub  = mbedtls_pk_ec(*pkey_public);

    ec_combined = pvPortCalloc(1, sizeof(mbedtls_ecp_keypair));
    if (ec_combined == NULL)
        return MBEDTLS_ERR_PK_ALLOC_FAILED;

    mbedtls_ecp_keypair_init(ec_combined);

    // Copy curve group
    ret = mbedtls_ecp_group_copy(&ec_combined->MBEDTLS_PRIVATE(grp),
                                 &ec_priv->MBEDTLS_PRIVATE(grp));
    if (ret != 0) goto cleanup;

    // Copy private key
    ret = mbedtls_mpi_copy(&ec_combined->MBEDTLS_PRIVATE(d),
                           &ec_priv->MBEDTLS_PRIVATE(d));
    if (ret != 0) goto cleanup;

    // Copy public key
    ret = mbedtls_ecp_copy(&ec_combined->MBEDTLS_PRIVATE(Q),
                           &ec_pub->MBEDTLS_PRIVATE(Q));
    if (ret != 0) goto cleanup;

    // Setup pk_context with EC key type
    ret = mbedtls_pk_setup(pkey_combined,
                           mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if (ret != 0) goto cleanup;

    // Manually assign the EC keypair to the internal context
    *((mbedtls_ecp_keypair **) pkey_combined->MBEDTLS_PRIVATE(pk_ctx)) = ec_combined;

    return 0;

cleanup:
    mbedtls_ecp_keypair_free(ec_combined);
    vPortFree(ec_combined);
    return ret;
}

static int saveKeys(void) {
  struct fs_file_t file;
  struct fs_dirent info;
  char buf[MAX_KEY_LEN];
  size_t len;
  int res;
  // Write public key
  len = 0;

  // Check if the file exists and has content
  // Only save new keys if the file is empty or does not exist
  // Or if device is expecting new certificate (newState_local == TT_SDK_STATE_NEXT_CERT)
  if (fs_stat(filename, &info) == 0 && info.size > 0) {
    // File exists with content 
    if (newState_local == TT_SDK_STATE_RUNNING) {
      zephyrLog(NULL, TT_LL_INFO, "Device is operational.");
      zephyrLog(NULL, TT_LL_INFO, "File existed with content, not overwriting.");
      keyProof.used = 1; // Mark key as used
      return 0;      
    } 
    else if (newState_local == TT_SDK_STATE_NEXT_CERT) {
      // Open in truncate mode (clear content)
      zephyrLog(NULL, TT_LL_INFO, "File existed with content, opened in TRUNC mode.");
      zephyrLog(NULL, TT_LL_INFO, "Device is not operational because waiting for new certificate.");
      res = fs_open(&file, filename, FS_O_WRITE | FS_O_TRUNC);
      if (res < 0) {
        zephyrLog(NULL, TT_LL_ERROR, "Failed to open cryptinit.bin file for writing.");
        return TT_E_NOT_FOUND;
      }
    }   
  }

  // File does not exist or is empty, create new
  zephyrLog(NULL, TT_LL_INFO, "File does not exist or is empty, create new cryptinit.bin file.");
  res = fs_open(&file, filename, FS_O_WRITE | FS_O_CREATE);
  if (res < 0)
    return TT_E_NOT_FOUND;

  if (mbedtls_pk_write_pubkey_pem(&keyProof.pkey, buf, sizeof(buf)) == 0) {
    if (fs_file_write(&file, buf, strlen((char *)buf)) < 0) {
      zephyrLog(NULL, TT_LL_ERROR, "Failed to write public key to file cryptinit.bin.");
      fs_file_close(&file);
      return -1;
    }
  } else {
    zephyrLog(NULL, TT_LL_ERROR, "Failed to write public key to PEM format.");
    fs_file_close(&file);
    return -1;  
  }

  // Write private key
  len = 0;
  if (mbedtls_pk_write_key_pem(&keyProof.pkey, buf, sizeof(buf)) == 0) {
    if (fs_file_write(&file, buf, strlen((char *)buf)) < 0) {
      zephyrLog(NULL, TT_LL_ERROR, "Failed to write private key to file cryptinit.bin.");
      fs_file_close(&file);
      return -1;
    }
  } else {
    zephyrLog(NULL, TT_LL_ERROR, "Failed to write private key to PEM format.");
    fs_file_close(&file);
    return -1;
  }

  keyProof.used = 1; // Mark key as used

  fs_file_close(&file);
  zephyrLog(NULL, TT_LL_INFO, "Keys saved to cryptinit.bin");
  return 0;
}

static int initKeys(void) {
  // Initialize filename to save keys
  int ret = snprintf(filename, TT_ARRAY_SIZE(filename), FILENAME_FMT_2,
                       BASE_DIR, INIT_FILENAME);

  if (ret <= 0 || ret >= (int)TT_ARRAY_SIZE(filename)) {
    return TT_E_FAULT;
  }

  // Check for init file
  if (loadKeys() != 0) {
    zephyrLog(NULL, TT_LL_INFO, "No keys to load keys from cryptinit.bin file.");
    return 0;
  }
  
  // If not using init file, create empty keys
  // If keys loaded successfully, mark them as used
  //if (saveKeys() != 0) {
  //  zephyrLog(NULL, TT_LL_ERROR, "Failed to save keys after loading from init file.");
  //  return 0;
  //}
  
  return 1; // Keys initialized successfully
}

static TtCryptAlgorithm getAlgoFromKey(const mbedtls_pk_context *key) {
  switch (mbedtls_pk_get_type(key)) {
  case MBEDTLS_PK_RSA:
    return TT_CRYPT_ALGO_SHA256_RSA2048_PKCS1;
  case MBEDTLS_PK_ECKEY:
    return TT_CRYPT_ALGO_SHA256_ECDSAP256_IEEEP1363;
  }
  return TT_E_NOT_SUPPORTED; // Invalid
}

static int fileLoaded = 0;

static TtError FreeRTOSCryptInit(void *ctx) {
  // Initialize keys if previous key was not loaded
  TT_UNUSED(ctx);

  return TT_E_OK;
}

static TtError FreeRTOSHashSHA256(void *ctx, const void *data, size_t dataLen,
                                  uint8_t *hash) {
  TT_UNUSED(ctx);
  return TT_E_NOT_SUPPORTED;
}

static TtError FreeRTOSSupports(void *ctx, TtCryptAlgorithm algo) {
  switch (algo) {
  case TT_CRYPT_ALGO_SHA256_RSA2048_PKCS1:
  case TT_CRYPT_ALGO_SHA256_ECDSAP256_IEEEP1363:
    return TT_E_OK;
  }
  return TT_E_NOT_SUPPORTED;
}

static TtError FreeRTOSKeyInfo(void *ctx, TtCryptKey key,
                               TtCryptAlgorithm *algo, char *pubKey,
                               size_t pubLen) {
  TT_UNUSED(ctx);

  struct Key *keyPtr = getKeyPtr(key);
  if (!keyPtr)
    return TT_E_NOT_SUPPORTED;

  if (!keyPtr->used) {
    // If keys not loaded, try to initialize them with previous stored keys  
    if (initKeys()) {
      zephyrLog(NULL, TT_LL_INFO, "Keys initialized and found on embedded filesystem successfully.");
      fileLoaded = 1;
      keyPtr = getKeyPtr(key);
      if (pubKey) {
        strncpy(pubKey, (char *)key_buf_public, pubLen);
        pubKey[pubLen - 1] = '\0';
      }
      if (algo) {
        *algo = getAlgoFromKey(&keyPtr->pkey);
        if (*algo == 0)
          return TT_E_FAULT;
      }
      return TT_E_OK;
    }
    // If still not used, return not found
    if (!keyPtr->used) {
      zephyrLog(NULL, TT_LL_INFO, "Key is NOT initialized with cryptinit.bin --> no file found on system so will create new ECDSA key.\n");
      return TT_E_NOT_FOUND;
    }
  }

  if (algo) {
    *algo = getAlgoFromKey(&keyPtr->pkey);
    if (*algo == 0)
      return TT_E_FAULT;
  }

  if (pubKey) {
    unsigned char buf[MAX_KEY_LEN];
    if (mbedtls_pk_write_pubkey_pem(&keyPtr->pkey, (unsigned char *)buf,
                                    sizeof(buf)) != 0) {
      return TT_E_FAULT;
    }
    strncpy(pubKey, (char *)buf, pubLen);
    pubKey[pubLen - 1] = '\0';
  }

  return TT_E_OK;
}

static TtError FreeRTOSKeyCreate(void *ctx, TtCryptKey key,
                                 TtCryptAlgorithm algo) {
  TT_UNUSED(ctx);
  struct Key *keyPtr = getKeyPtr(key);
  if (!keyPtr)
    return TT_E_NOT_SUPPORTED;

  if (keyPtr->used)
    return TT_E_INVALID_STATE;

  mbedtls_pk_init(&keyPtr->pkey);

  switch (algo) {
    case TT_CRYPT_ALGO_SHA256_RSA2048_PKCS1:
      if (mbedtls_pk_setup(&keyPtr->pkey,
                          mbedtls_pk_info_from_type(MBEDTLS_PK_RSA)) != 0) {
        return TT_E_FAULT;
      }
      if (mbedtls_rsa_gen_key(mbedtls_pk_rsa(keyPtr->pkey), FreeRTOSGetRand, NULL,
                              2048, 65537) != 0) {
        mbedtls_pk_free(&keyPtr->pkey);
        return TT_E_FAULT;
      }
      break;
    case TT_CRYPT_ALGO_SHA256_ECDSAP256_IEEEP1363:
      if (mbedtls_pk_setup(&keyPtr->pkey,
                          mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)) != 0) {
        return TT_E_FAULT;
      }
      if (mbedtls_ecdsa_genkey(mbedtls_pk_ec(keyPtr->pkey),
                              MBEDTLS_ECP_DP_SECP256R1, FreeRTOSGetRand,
                              NULL) != 0) {
        mbedtls_pk_free(&keyPtr->pkey);
        return TT_E_FAULT;
      }
      break;
    default:
      return TT_E_NOT_SUPPORTED;
  }

  keyPtr->used = 1;
  saveKeys();

  return TT_E_OK;
}

//------------------------------------------------------------------------------
// Random number generator using the MAXQ1065 cryptoprocessor.
//
static int maxq1065_get_random(unsigned char *buf, size_t len) {
  // This function gets random bytes from the MAXQ1065.
  int ret = MXC_TRNG_Init();
  if (ret) {
    printf("MXC_TRNG_Init failed\n");
    return TT_E_FAULT;
  }
  ret = MXC_TRNG_Random(buf, len);
  if (ret != 0) {
    printf("MXC_TRNG_Random failed\n");
    MXC_TRNG_Shutdown();
    return TT_E_FAULT;
  }
  MXC_TRNG_Shutdown();
  return TT_E_OK;
}

//------------------------------------------------------------------------------
// Fill 'buf' with 'len' random bytes from the MAXQ1065 cryptoprocessor.
static TtError FreeRTOSGetRand(void *context, void *buf, size_t len) {
  int ret;
  (void)context; // Unused in this implementation.
  ret = maxq1065_get_random((unsigned char *)buf, len);
  if (ret != 0)
    return TT_E_FAULT;
  return TT_E_OK;
}

static TtError FreeRTOSSign(void *ctx, TtCryptKey key, const void *data,
                            size_t dataLen, void *output, size_t *outLen) {
  struct Key *keyPtr = getKeyPtr(key);
  if (!keyPtr)
    return TT_E_NOT_SUPPORTED;

  if (!keyPtr->used)
    return TT_E_NOT_FOUND;

  TtCryptAlgorithm algo = getAlgoFromKey(&keyPtr->pkey);
  if (algo == 0)
    return TT_E_FAULT;

  size_t sigLen = *outLen;
  if (algo == TT_CRYPT_ALGO_SHA256_RSA2048_PKCS1) {
    if (mbedtls_pk_sign(&keyPtr->pkey, MBEDTLS_MD_SHA256, data, dataLen,
                        (unsigned char *)output, sizeof(output), &sigLen,
                        FreeRTOSGetRand, NULL) != 0) {
      return TT_E_FAULT;
    }
  } else if (algo == TT_CRYPT_ALGO_SHA256_ECDSAP256_IEEEP1363) {
    unsigned char sig[MBEDTLS_ECDSA_MAX_LEN];
    size_t sig_len = sizeof(sig);
    if (mbedtls_pk_sign(&keyPtr->pkey, MBEDTLS_MD_SHA256, data, dataLen,
                        sig, sizeof(sig), &sig_len,  FreeRTOSGetRand, NULL) != 0) {
      return TT_E_FAULT;
    }
    // Convert ASN.1 DER signature to IEEE P1363 (r || s, each 32 bytes)
    mbedtls_mpi r, s;
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);
    const unsigned char *p = sig;
    const unsigned char *end = sig + sig_len;
    int ret;
    // Parse ASN.1 DER signature to extract r and s
    if ((ret = mbedtls_asn1_get_tag(&p, end, &sig_len,
                                    MBEDTLS_ASN1_CONSTRUCTED |
                                        MBEDTLS_ASN1_SEQUENCE)) != 0 ||
        (ret = mbedtls_asn1_get_mpi(&p, end, &r)) != 0 ||
        (ret = mbedtls_asn1_get_mpi(&p, end, &s)) != 0) {
      mbedtls_mpi_free(&r);
      mbedtls_mpi_free(&s);
      return TT_E_FAULT;
    }
    // Write r and s as 32-byte big-endian values
    if (mbedtls_mpi_write_binary(&r, (unsigned char *)output, 32) != 0 ||
        mbedtls_mpi_write_binary(&s, (unsigned char *)output + 32, 32) != 0) {
      mbedtls_mpi_free(&r);
      mbedtls_mpi_free(&s);
      return TT_E_FAULT;
    }
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);
    sigLen = 64;
    if (sigLen != 64)
      return TT_E_FAULT;
  } else {
    return TT_E_FAULT;
  }

  *outLen = sigLen;

  printf("Sign output len: %zu (data len: %zu)\n", *outLen, dataLen);
  for (size_t i = 0; i < *outLen; i++) {
    printf("%02x ", ((unsigned char *)output)[i]);
    if (i > 0 && (i % 16) == 0)
      printf("\n");
  }
  printf("\n");

  return TT_E_OK;
}
