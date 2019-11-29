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

typedef hb_log_level_e hb_log_level_t;

static const char * hb_log_level_string[] = {"all", "info", "warning", "error", "critical"};

hb_result_t hb_log_initialize();
void hb_log_finalize();

typedef void(*hb_log_observer_t)(const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _message);

hb_result_t hb_log_add_observer( const char * _category, hb_log_level_t _level, hb_log_observer_t _observer );
hb_result_t hb_log_remove_observer( hb_log_observer_t _observer );

void hb_log_message( const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _format, ... );

#define HB_LOG_MESSAGE_INFO(category, format, ...)\
    hb_log_message(category, HB_LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__)

#define HB_LOG_MESSAGE_WARNING(category, format, ...)\
    hb_log_message(category, HB_LOG_WARNING, __FILE__, __LINE__, format, __VA_ARGS__)

#define HB_LOG_MESSAGE_ERROR(category, format, ...)\
    hb_log_message(category, HB_LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)

#define HB_LOG_MESSAGE_CRITICAL(category, format, ...)\
    hb_log_message(category, HB_LOG_CRITICAL, __FILE__, __LINE__, format, __VA_ARGS__)

#endif
