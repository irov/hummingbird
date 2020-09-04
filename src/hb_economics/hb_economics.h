#ifndef HB_ECONOMICS_H_
#define HB_ECONOMICS_H_

#include "hb_config/hb_config.h"
#include "hb_cache/hb_cache.h"
#include "hb_db/hb_db.h"

typedef struct hb_economics_handle_t hb_economics_handle_t;

hb_result_t hb_economics_create( hb_economics_handle_t ** _handle );
void hb_economics_destroy( hb_economics_handle_t * _handle );

hb_result_t hb_economics_new_bank( hb_economics_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _uuid, const void * _data, hb_size_t _datasize, hb_uid_t * _buid );
hb_result_t hb_economics_new_records( hb_economics_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, const void * _data, hb_size_t _datasize );
hb_result_t hb_economics_new_contract( hb_economics_handle_t * _handle, const hb_db_client_handle_t * _client, hb_uid_t _puid, hb_uid_t _uuid, hb_uid_t _buid, const char * _category, const char * _name, hb_error_code_t * _code );

#endif