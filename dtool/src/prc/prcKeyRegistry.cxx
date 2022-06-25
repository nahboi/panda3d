/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file prcKeyRegistry.cxx
 * @author drose
 * @date 2004-10-19
 */

#include "prcKeyRegistry.h"
#include "config_prc.h"

// This file requires OpenSSL to compile, because we use routines in the
// OpenSSL library to manage keys and to sign and validate signatures.

#ifdef HAVE_OPENSSL

#include <openssl/evp.h>
#include <openssl/pem.h>

// Some versions of OpenSSL appear to define this as a macro.  Yucky.
#undef set_key

PrcKeyRegistry *PrcKeyRegistry::_global_ptr = nullptr;

/**
 * There is only one PrcKeyRegistry in the world; use get_global_ptr() to get
 * it.
 */
PrcKeyRegistry::
PrcKeyRegistry() {
}

/**
 *
 */
PrcKeyRegistry::
~PrcKeyRegistry() {
  prc_cat.error()
    << "Internal error--PrcKeyRegistry destructor called!\n";
}

/**
 * Records the list of public keys that are compiled into this executable.
 * The pointer is assumed to be to an area of static memory that will not be
 * destructed, so the data is not copied, but only the pointer is assigned.
 *
 * This method is normally called after including the code generated by the
 * make-prc-key utility.
 */
void PrcKeyRegistry::
record_keys(const KeyDef *key_def, size_t num_keys) {
  for (size_t i = 0; i < num_keys; i++) {
    const KeyDef *def = &key_def[i];
    if (def->_data != nullptr) {
      // Clear the ith key.
      while (_keys.size() <= i) {
        Key key;
        key._def = nullptr;
        key._pkey = nullptr;
        key._generated_time = 0;
        _keys.push_back(key);
      }
      if (_keys[i]._def != def) {
        if (_keys[i]._pkey != nullptr) {
          EVP_PKEY_free(_keys[i]._pkey);
          _keys[i]._pkey = nullptr;
        }
        _keys[i]._def = def;
        _keys[i]._generated_time = def->_generated_time;
      }
    }
  }
}

/**
 * Sets the nth public key in the registry to the given value.  The EVP_PKEY
 * structure must have been properly allocated view EVP_PKEY_new(); its
 * ownership is transferred to the registry and it will eventually be freed
 * via EVP_PKEY_free().
 */
void PrcKeyRegistry::
set_key(size_t n, EVP_PKEY *pkey, time_t generated_time) {
  // Clear the nth key.
  while (_keys.size() <= n) {
    Key key;
    key._def = nullptr;
    key._pkey = nullptr;
    key._generated_time = 0;
    _keys.push_back(key);
  }
  _keys[n]._def = nullptr;
  if (_keys[n]._pkey != nullptr) {
    EVP_PKEY_free(_keys[n]._pkey);
    _keys[n]._pkey = nullptr;
  }
  _keys[n]._pkey = pkey;
  _keys[n]._generated_time = generated_time;
}

/**
 * Returns the number of public keys in the registry.  This is actually the
 * highest index number + 1, which might not strictly be the number of keys,
 * since there may be holes in the list.
 */
size_t PrcKeyRegistry::
get_num_keys() const {
  return _keys.size();
}

/**
 * Returns the nth public key, or NULL if the nth key is not defined.
 */
EVP_PKEY *PrcKeyRegistry::
get_key(size_t n) const {
  nassertr(n < _keys.size(), nullptr);

  if (_keys[n]._def != nullptr) {
    if (_keys[n]._pkey == nullptr) {
      // Convert the def to a EVP_PKEY structure.
      const KeyDef *def = _keys[n]._def;
      BIO *mbio = BIO_new_mem_buf((void *)def->_data, def->_length);
      EVP_PKEY *pkey = PEM_read_bio_PUBKEY(mbio, nullptr, nullptr, nullptr);
      ((PrcKeyRegistry *)this)->_keys[n]._pkey = pkey;
      BIO_free(mbio);

      if (pkey == nullptr) {
        // Couldn't read the bio for some reason.
        ((PrcKeyRegistry *)this)->_keys[n]._def = nullptr;
      }
    }
  }

  return _keys[n]._pkey;
}

/**
 * Returns the timestamp at which the indicated key was generated, or 0 if the
 * key is not defined.
 */
time_t PrcKeyRegistry::
get_generated_time(size_t n) const {
  nassertr(n < _keys.size(), 0);

  return _keys[n]._generated_time;
}

/**
 *
 */
PrcKeyRegistry *PrcKeyRegistry::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new PrcKeyRegistry;
  }
  return _global_ptr;
}

#endif  // HAVE_OPENSSL