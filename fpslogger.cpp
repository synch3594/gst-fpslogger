#include <syslog.h>
#include <gst/gst.h>
#include "fpslogger.h"

namespace fpslogger {

#define PRINTLOG(format, ...) \
    do { \
        switch(logtype) { \
            case LOGTYPE_STDOUT: { \
                g_print(format, __VA_ARGS__); \
                break; \
            } \
            case LOGTYPE_FILE: { \
                fprintf(fp, format, __VA_ARGS__); \
                break; \
            } \
            case LOGTYPE_SYSLOG: { \
                syslog(LOG_INFO, format, __VA_ARGS__); \
                break; \
            } \
        } \
    } while(0)

FPSLogger::FPSLogger(gdouble interval, LogType logtype, const gchar *logpath) :
    interval(interval),
    logtype(LOGTYPE_STDOUT),
    logpath(),
    frame_count(0),
    frame_count_total(0),
    start_time(),
    last_time(),
    fp(NULL)
{
    memset(this->logpath, 0, sizeof(logpath));

    switch(logtype) {
        case LOGTYPE_FILE: {
            strncpy(this->logpath, logpath, sizeof(this->logpath));
            fp = fopen(this->logpath, "w");
            break;
        }
        case LOGTYPE_SYSLOG: {
            openlog("FPSLogger", LOG_CONS | LOG_PID, LOG_USER);
            break;
        }
    }
}

FPSLogger::~FPSLogger()
{
    switch(logtype) {
        case LOGTYPE_FILE: {
            if (fp != NULL) {
                fclose(fp);
            }
            break;
        }
        case LOGTYPE_SYSLOG: {
            closelog();
            break;
        }
    }
}

void FPSLogger::count()
{
    static bool first = true;

    frame_count++;

    time_point_t current_time = std::chrono::high_resolution_clock::now();
    if (first) {
        last_time = current_time;
        first = false;
    }

    double elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(current_time - last_time).count();
    if (elapsed_time >= interval) {
        double fps = frame_count / elapsed_time;
        PRINTLOG("fpslogger : fps = %lf\n", fps);
        frame_count_total += frame_count;
        frame_count = 0;
        last_time = current_time;
    }
}

} // namespace fpslogger