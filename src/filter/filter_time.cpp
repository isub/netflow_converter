#include "filter_time.h"

static time_t g_tmTimeStart;
static time_t g_tmTimeStop;
static time_t g_tmToleranceBefore;
static time_t g_tmToleranceAfter;

static int filter_time_check_time( time_t p_tmTime, time_t p_tmToleranceBefore, time_t p_tmToleranceAfter );

void filter_time_init( const time_t p_tmStart, const time_t p_tmStop, const time_t p_tmToleranceBefore, const time_t p_tmToleranceAfter )
{
  g_tmTimeStart = p_tmStart;
  g_tmTimeStop = p_tmStop;
  g_tmToleranceBefore = p_tmToleranceBefore;
  g_tmToleranceAfter = p_tmToleranceAfter;
}

int filter_time_file( const time_t p_tmFileTime )
{
  return filter_time_check_time( p_tmFileTime, g_tmToleranceBefore, g_tmToleranceAfter );
}

int filter_time_data( const time_t p_timDataTime )
{
  return filter_time_check_time( p_timDataTime, 0, 0 );
}

static int filter_time_check_time( time_t p_tmTime, time_t p_tmToleranceBefore, time_t p_tmToleranceAfter )
{
  if ( g_tmTimeStart - p_tmToleranceBefore <= p_tmTime && g_tmTimeStop + p_tmToleranceAfter > p_tmTime ) {
    return 1;
  } else {
    return 0;
  }
}
