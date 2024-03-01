#ifndef HB_LOG_H_
#define HB_LOG_H_

#include "hb_config/hb_config.h"

typedef enum hb_log_level_e
{
    HB_LOG_ALL = 0,
    HB_LOG_DEBUG = 1,
    HB_LOG_INFO = 2,
    HB_LOG_WARNING = 3,
    HB_LOG_ERROR = 4,
    HB_LOG_CRITICAL = 5,
} hb_log_level_e;

typedef hb_log_level_e hb_log_level_t;

hb_result_t hb_log_initialize();
void hb_log_finalize();

const char * hb_log_level_stringize( hb_log_level_t _level );
hb_result_t hb_log_level_parse( const char * _string, hb_log_level_t * const _level );

void hb_log_set_verbose_level( hb_log_level_t _level );
hb_log_level_t hb_log_get_verbose_level();
hb_bool_t hb_log_check_verbose_level( hb_log_level_t _level );

typedef void(*hb_log_observer_t)(const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _message, void * _ud);

hb_result_t hb_log_add_observer( const char * _category, hb_log_level_t _level, hb_log_observer_t _observer, void * _ud );
hb_result_t hb_log_remove_observer( hb_log_observer_t _observer, void ** _ud );

void hb_log_message( const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _format, ... );

#define HB_LOG_MESSAGE_DEBUG(category, ...)\
    do { if( hb_log_check_verbose_level( HB_LOG_DEBUG ) == HB_TRUE ) { hb_log_message(category, HB_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__); } } while(0)

#define HB_LOG_MESSAGE_INFO(category, ...)\
    do { if( hb_log_check_verbose_level( HB_LOG_INFO ) == HB_TRUE ) { hb_log_message(category, HB_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__); } } while(0)

#define HB_LOG_MESSAGE_WARNING(category, ...)\
    do { if( hb_log_check_verbose_level( HB_LOG_WARNING ) == HB_TRUE ) { hb_log_message(category, HB_LOG_WARNING, __FILE__, __LINE__, __VA_ARGS__); } } while(0)

#define HB_LOG_MESSAGE_ERROR(category, ...)\
    do { if( hb_log_check_verbose_level( HB_LOG_ERROR ) == HB_TRUE ) { hb_log_message(category, HB_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__); } } while(0)

#define HB_LOG_MESSAGE_CRITICAL(category, ...)\
    hb_log_message(category, HB_LOG_CRITICAL, __FILE__, __LINE__, __VA_ARGS__)

#endif
