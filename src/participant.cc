/**
 * Multiparty Off-the-Record Messaging library
 * Copyright (C) 2014, eQualit.ie
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of version 3 of the GNU Lesser General
 * Public License as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "src/participant.h"
#include "src/logger.h"

// Global logger
Logger log(INFO);

/**
 * To be used in std::sort to sort the particpant list
 * in a way that is consistent way between all participants
 */
bool sort_by_long_term_pub_key(const np1secAsymmetricKey lhs, const np1secAsymmetricKey rhs)
{
  return Cryptic::public_key_to_stringbuff(lhs) < Cryptic::public_key_to_stringbuff(rhs);

}

/**
 * operator < needed by map class not clear why but it doesn't compile
 * It first does nick name check then public key check. in reality
 * public key check is not needed as the nickname are supposed to be 
 * unique (that is why nickname is more approperiate for sorting than
 * public key)
 */
bool operator<(const Participant& lhs, const Participant& rhs)
{
  if (lhs.id.nickname < rhs.id.nickname) return true;

  return sort_by_long_term_pub_key(lhs.long_term_pub_key, rhs.long_term_pub_key);
  
}
 
/**
 * Generate the approperiate authentication token check its equality
 * to authenticate the alleged participant
 *
 * @param auth_token authentication token received as a message
 * @param authenicator_id running thread user id  //TODO 
 *  can give it to youget rid of this as thread_user_as_partcipant 
 * @param thread_user_id_key the key (pub & prive) of the user running the 
 *        thread
 * 
 * @return true if peer's authenticity could be established
 */
bool Participant::be_authenticated(const std::string authenticator_id, const HashBlock auth_token, const np1secAsymmetricKey thread_user_id_key, Cryptic* thread_user_crypto) {
  if (!compute_p2p_private(thread_user_id_key, thread_user_crypto))
    return false;
  
  std::string to_be_hashed(reinterpret_cast<const char*>(p2p_key), sizeof(HashBlock));
  to_be_hashed+= authenticator_id;
  HashBlock regenerated_auth_token;

  Cryptic::hash(to_be_hashed.c_str(), to_be_hashed.size(), regenerated_auth_token);

  authenticated = !Cryptic::compare_hash(regenerated_auth_token, auth_token);
  return (authenticated);

}

/**
 * Generate the approperiate authentication token check its equality
 * to authenticate the alleged participant
 *
 * @param auth_token authentication token received as a message
 * @param thread_user_id_key the key (pub & prive) of the user running the 
 *        thread
 * 
 * @return true if peer's authenticity could be established
 */
bool Participant::authenticate_to(HashBlock auth_token, const np1secAsymmetricKey thread_user_id_key, Cryptic* thread_user_crypto) {

  if (!compute_p2p_private(thread_user_id_key, thread_user_crypto))
    return false;

  std::string to_be_hashed(reinterpret_cast<const char*>(p2p_key), sizeof(HashBlock));
  to_be_hashed += id.id_to_stringbuffer(); //the question is that why should we include the public
  //key here? 
  Cryptic::hash(to_be_hashed.c_str(), to_be_hashed.size(), auth_token);

  return true;

}

/**
 * computes the p2p triple dh secret between participants
 *
 * @return true on success
 */
bool Participant::compute_p2p_private(np1secAsymmetricKey thread_user_id_key, Cryptic* thread_user_crypto)
{
  log.info("Before calling triple_ed_dh, p2p_key = " + Cryptic::hash_to_string_buff(p2p_key));
  thread_user_crypto->triple_ed_dh(ephemeral_key, long_term_pub_key, thread_user_id_key, sort_by_long_term_pub_key(this->long_term_pub_key, thread_user_id_key), &p2p_key);
  log.info("After calling triple_ed_dh, p2p_key = " + Cryptic::hash_to_string_buff(p2p_key));
                      
}
