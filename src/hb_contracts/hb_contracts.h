#ifndef HB_CONTRACTS_H_
#define HB_CONTRACTS_H_

#include "hb_config/hb_config.h"
#include "hb_cache/hb_cache.h"
#include "hb_db/hb_db.h"

typedef struct hb_contracts_handle_t hb_contracts_handle_t;

hb_result_t hb_contracts_create( hb_contracts_handle_t ** _handle );
void hb_contracts_destroy( hb_contracts_handle_t * _handle );

hb_result_t hb_contracts_new_records( hb_contracts_handle_t * _handle, hb_uid_t _puid, const void * _data, size_t _datasize );

#endif