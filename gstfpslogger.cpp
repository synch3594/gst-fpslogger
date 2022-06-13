#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gstfpslogger.h"
#include "fpslogger.h"

GST_DEBUG_CATEGORY_STATIC(gst_fpslogger_debug_category);
#define GST_CAT_DEFAULT gst_fpslogger_debug_category

#define FPSLOGGER_PROP_DEFAULT_INTERVAL 1.0
#define FPSLOGGER_PROP_DEFAULT_LOGTYPE  0
#define FPSLOGGER_PROP_DEFAULT_LOGPATH  NULL

static void gst_fpslogger_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void gst_fpslogger_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void gst_fpslogger_dispose(GObject *object);
static void gst_fpslogger_finalize(GObject *object);

static gboolean gst_fpslogger_start(GstBaseTransform *trans);
static gboolean gst_fpslogger_stop(GstBaseTransform *trans);
static gboolean gst_fpslogger_sink_event(GstBaseTransform *trans, GstEvent *event);
static gboolean gst_fpslogger_src_event(GstBaseTransform *trans, GstEvent *event);
static GstFlowReturn gst_fpslogger_transform_ip(GstBaseTransform *trans, GstBuffer *buf);

enum {
    PROP_0,
    PROP_LOGTYPE,
    PROP_LOGPATH,
    PROP_INTERVAL
};

static GstStaticPadTemplate gst_fpslogger_src_template =
    GST_STATIC_PAD_TEMPLATE(
        "src",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS("ANY")
    );

static GstStaticPadTemplate gst_fpslogger_sink_template =
    GST_STATIC_PAD_TEMPLATE(
        "sink",
        GST_PAD_SINK,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS("ANY")
    );

static fpslogger::FPSLogger *logger;

G_DEFINE_TYPE_WITH_CODE(
    GstFPSLogger, gst_fpslogger,
    GST_TYPE_BASE_TRANSFORM,
    GST_DEBUG_CATEGORY_INIT(gst_fpslogger_debug_category, "fpslogger", 0, "debug category for fpslogger element")
);

static void gst_fpslogger_class_init(GstFPSLoggerClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);

    gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass), &gst_fpslogger_src_template);
    gst_element_class_add_static_pad_template(GST_ELEMENT_CLASS(klass), &gst_fpslogger_sink_template);

    gst_element_class_set_static_metadata(
        GST_ELEMENT_CLASS(klass),
        "FIXME Long name", "Generic", "FIXME Description",
        "FIXME <fixme@example.com>"
    );

    gobject_class->set_property = gst_fpslogger_set_property;
    gobject_class->get_property = gst_fpslogger_get_property;
    gobject_class->dispose = gst_fpslogger_dispose;
    gobject_class->finalize = gst_fpslogger_finalize;

    base_transform_class->start = GST_DEBUG_FUNCPTR(gst_fpslogger_start);
    base_transform_class->stop = GST_DEBUG_FUNCPTR(gst_fpslogger_stop);
    base_transform_class->sink_event = GST_DEBUG_FUNCPTR(gst_fpslogger_sink_event);
    base_transform_class->src_event = GST_DEBUG_FUNCPTR (gst_fpslogger_src_event);
    base_transform_class->transform = NULL;
    base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_fpslogger_transform_ip);

    g_object_class_install_property(
        gobject_class, PROP_INTERVAL,
        g_param_spec_double(
            "interval",
            "Interval at which fps is measured",
            "Specify the interval at which fps is measured. (second)",
            0.0, G_MAXDOUBLE, FPSLOGGER_PROP_DEFAULT_INTERVAL, (GParamFlags)((G_PARAM_READWRITE) | (G_PARAM_STATIC_STRINGS))
        )
    );

    g_object_class_install_property(
        gobject_class, PROP_LOGTYPE,
        g_param_spec_int(
            "log-type",
            "Log Type",
            "Specify log output destination."
            " (0:stdout, 1:file, 2:syslog) "
            "If a file is specified, use in conjunction with the log-path property.",
            0, 2, FPSLOGGER_PROP_DEFAULT_LOGTYPE, (GParamFlags)((G_PARAM_READWRITE) | (G_PARAM_STATIC_STRINGS))
        )
    );

    g_object_class_install_property(
        gobject_class, PROP_LOGPATH,
        g_param_spec_string(
            "log-path",
            "Log file path",
            "Specify log output file path. (Only if log-type is file)",
            NULL, (GParamFlags)((G_PARAM_READWRITE) | (G_PARAM_STATIC_STRINGS))
        )
    );
}

static void gst_fpslogger_init(GstFPSLogger *fpslogger)
{
    fpslogger->interval = FPSLOGGER_PROP_DEFAULT_INTERVAL;
    fpslogger->log_type = FPSLOGGER_PROP_DEFAULT_LOGTYPE;
    fpslogger->log_path = FPSLOGGER_PROP_DEFAULT_LOGPATH;
}

static void gst_fpslogger_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    GstFPSLogger *fpslogger = GST_FPSLOGGER(object);

    GST_DEBUG_OBJECT(fpslogger, "set_property");
    g_print("set_property");

    switch (property_id) {
        case PROP_LOGTYPE:
            fpslogger->log_type = g_value_get_int(value);
            break;
        case PROP_LOGPATH:
            fpslogger->log_path = (gchar *)g_value_dup_string(value);
            break;
        case PROP_INTERVAL:
            fpslogger->interval = g_value_get_double(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void gst_fpslogger_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
    GstFPSLogger *fpslogger = GST_FPSLOGGER(object);

    GST_DEBUG_OBJECT(fpslogger, "get_property");

    switch (property_id) {
        case PROP_LOGTYPE:
            g_value_set_int(value, fpslogger->log_type);
            break;
        case PROP_LOGPATH:
            g_value_set_string(value, fpslogger->log_path);
            break;
        case PROP_INTERVAL:
            g_value_set_double(value, fpslogger->interval);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void gst_fpslogger_dispose(GObject *object)
{
    GstFPSLogger *fpslogger = GST_FPSLOGGER(object);

    GST_DEBUG_OBJECT(fpslogger, "dispose");

    G_OBJECT_CLASS(gst_fpslogger_parent_class)->dispose(object);
}

static void gst_fpslogger_finalize(GObject *object)
{
    GstFPSLogger *fpslogger = GST_FPSLOGGER(object);

    GST_DEBUG_OBJECT(fpslogger, "finalize");

    if (fpslogger->log_path) {
        g_free(fpslogger->log_path);
    }

    G_OBJECT_CLASS(gst_fpslogger_parent_class)->finalize(object);
}

static gboolean gst_fpslogger_start(GstBaseTransform *trans)
{
    GstFPSLogger *fpslogger = GST_FPSLOGGER(trans);

    GST_DEBUG_OBJECT(fpslogger, "start");

    logger = new fpslogger::FPSLogger(fpslogger->interval, (fpslogger::LogType)fpslogger->log_type, fpslogger->log_path);

    return TRUE;
}

static gboolean gst_fpslogger_stop(GstBaseTransform *trans)
{
    GstFPSLogger *fpslogger = GST_FPSLOGGER(trans);

    GST_DEBUG_OBJECT(fpslogger, "stop");

    if (logger) {
         delete(logger);
    }

    return TRUE;
}

static gboolean gst_fpslogger_sink_event(GstBaseTransform *trans, GstEvent *event)
{
    GstFPSLogger *fpslogger = GST_FPSLOGGER(trans);

    GST_DEBUG_OBJECT(fpslogger, "sink_event");

    return GST_BASE_TRANSFORM_CLASS(gst_fpslogger_parent_class)->sink_event(trans, event);
}

static gboolean gst_fpslogger_src_event(GstBaseTransform *trans, GstEvent *event)
{
    GstFPSLogger *fpslogger = GST_FPSLOGGER(trans);

    GST_DEBUG_OBJECT(fpslogger, "src_event");

    return GST_BASE_TRANSFORM_CLASS(gst_fpslogger_parent_class)->src_event(trans, event);
}

static GstFlowReturn gst_fpslogger_transform_ip(GstBaseTransform *trans, GstBuffer *buf)
{
    GstFPSLogger *fpslogger = GST_FPSLOGGER (trans);

    GST_DEBUG_OBJECT(fpslogger, "transform_ip");

    logger->count();

    return GST_FLOW_OK;
}

static gboolean plugin_init(GstPlugin *plugin)
{
    return gst_element_register(plugin, "fpslogger", GST_RANK_NONE, GST_TYPE_FPSLOGGER);
}

#ifndef VERSION
#define VERSION "0.1.0"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    fpslogger,
    "FPS logger plugin.",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN
)