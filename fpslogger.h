#ifndef _FPSLOGGER_H_
#define _FPSLOGGER_H_

#include <cstdio>
#include <chrono>
#include <glib.h>

namespace fpslogger {

enum LogType {
    LOGTYPE_STDOUT,
    LOGTYPE_FILE,
    LOGTYPE_SYSLOG
};

using time_point_t = std::chrono::high_resolution_clock::time_point;

class FPSLogger
{
public:
    FPSLogger(gdouble interval = 1.0, LogType logtype = LOGTYPE_STDOUT, const gchar *logpath = NULL);
    virtual ~FPSLogger();
    void count();

private:
    gdouble interval;
    LogType logtype;
    gchar logpath[1024];
    guint64 frame_count;
    guint64 frame_count_total;
    time_point_t start_time;
    time_point_t last_time;    
    FILE *fp;
};

} // namespace fpslogger

#endif // _FPSLOGGER_H_