#ifndef _GST_FPSLOGGER_H_
#define _GST_FPSLOGGER_H_

#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS

#define GST_TYPE_FPSLOGGER           (gst_fpslogger_get_type())
#define GST_FPSLOGGER(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_FPSLOGGER, GstFPSLogger))
#define GST_FPSLOGGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_FPSLOGGER, GstFPSLoggerClass))
#define GST_IS_FPSLOGGER(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_FPSLOGGER))
#define GST_IS_FPSLOGGER_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_FPSLOGGER))

typedef struct _GstFPSLogger GstFPSLogger;
typedef struct _GstFPSLoggerClass GstFPSLoggerClass;

struct _GstFPSLogger
{
    GstBaseTransform base_fpslogger;
    gint log_type;
    gchar *log_path;
    gdouble interval;
};

struct _GstFPSLoggerClass
{
    GstBaseTransformClass base_fpslogger_class;
};

GType gst_fpslogger_get_type(void);

G_END_DECLS

#endif // _GST_FPSLOGGER_H_