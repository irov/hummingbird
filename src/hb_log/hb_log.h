#ifndef HB_LOG_H_
#define HB_LOG_H_

#define HB_LOG_INFO 0
#define HB_LOG_WARNING 1
#define HB_LOG_ERROR 2
#define HB_LOG_CRITICAL 4

typedef void(*hb_log_observer_t)(int _level, const char * _category, const char * _message);

int hb_log_add_observer( int _level, const char * _category, hb_log_observer_t _observer );
int hb_log_remove_observer( hb_log_observer_t _observer );

void hb_log_message( int _level, const char * _category, const char * _format, ... );

#endif
