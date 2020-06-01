#ifndef HB_CONTRACTS_H_
#define HB_CONTRACTS_H_

#include "hb_config/hb_config.h"
#include "hb_cache/hb_cache.h"
#include "hb_db/hb_db.h"

typedef struct hb_contracts_handle_t hb_contracts_handle_t;

hb_result_t hb_contracts_create( hb_contracts_handle_t ** _handle );
void hb_contracts_destroy( hb_contracts_handle_t * _handle );

hb_result_t hb_contracts_new_bank( hb_contracts_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _uuid, const void * _data, size_t _datasize, hb_uid_t * _uid );
hb_result_t hb_contracts_new_records( hb_contracts_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, const void * _data, size_t _datasize );
hb_result_t hb_contracts_new_contract( hb_contracts_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _uuid, hb_uid_t _buid, const char * _category, const char * _name );

#endif