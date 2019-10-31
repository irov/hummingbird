#ifndef HB_LOG_H_
#define HB_LOG_H_

#include "hb_config/hb_config.h"

typedef enum hb_log_level_e
{
    HB_LOG_ALL,
    HB_LOG_INFO,
    HB_LOG_WARNING,
    HB_LOG_ERROR,
    HB_LOG_CRITICAL,
} hb_log_level_e;

static const char * hb_log_level_string[] = {"all", "info", "warning", "error", "critical"};

hb_result_t hb_log_initialize();
void hb_log_finalize();

typedef void(*hb_log_observer_t)(const char * _category, hb_log_level_e _level, const char * _message);

hb_result_t hb_log_add_observer( const char * _category, hb_log_level_e _level, hb_log_observer_t _observer );
hb_result_t hb_log_remove_observer( hb_log_observer_t _observer );

void hb_log_message( const char * _category, hb_log_level_e _level, const char * _format, ... );

#endif
