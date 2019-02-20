#ifndef __FILTER_TIME_H__
#define __FILTER_TIME_H__

#include <time.h>

void filter_time_init( const time_t p_tmStart, const time_t p_tmStop, const time_t p_tmToleranceBefore, const time_t p_tmToleranceAfter );
int filter_time_file( const time_t p_tmFileTime );
int filter_time_data( const time_t p_timDataTime );

#endif /* __FILTER_TIME_H__ */
