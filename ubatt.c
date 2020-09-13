#include <stdio.h>
#include <glib.h>
#include <upower.h>
#include <math.h>

static gboolean opt_ver, opt_print, opt_ret, opt_retcharge;

static GOptionEntry entries[] =
{
    {"version",         'V', 0, G_OPTION_ARG_NONE, &opt_ver,      "Show UPower daemon version", NULL},
    {"print-charge",    'p', 0, G_OPTION_ARG_NONE, &opt_print,    "Print charge level on stamdard output", NULL},
    {"return-battery",  'r', 0, G_OPTION_ARG_NONE, &opt_ret,      "Return 1 if running on battery", NULL},
    {"return-charge",   'R', 0, G_OPTION_ARG_NONE, &opt_retcharge,"Return charge level as exit status", NULL},
    NULL
};

void ubatt_show_on_battery(UpClient *upower)
{
    if (up_client_get_on_battery(upower))
    {
        g_print("On battery power\n");
    }
    else
    {
        g_print("Not on battery power");
        GPtrArray *devices = up_client_get_devices2(upower);
        UpDevice *device = NULL;

        gboolean ac_online = FALSE, usbc_online = FALSE;

        for (int i=0; i<devices->len; i++)
        {
            UpDevice *this_dev = g_ptr_array_index(devices, i);
            UpDeviceKind kind;
            gboolean supply, online;
            gchar *path;
            g_object_get(this_dev,
                         "power-supply", &supply,
                         "kind", &kind,
                         "online", &online,
                         "native-path", &path,
                         NULL);
            if (supply && online)
            {
                if (strstr(path, "USBC") != NULL)
                {
                    usbc_online = TRUE;
                }
                else
                {
                    ac_online = TRUE;
                }
            }
        }
        if (usbc_online)
        {
            g_print(" (USB-C connected)");
        }
        else if (ac_online)
        {
            g_print(" (AC adapter connected)");
        }
        g_ptr_array_unref(devices);

        g_print("\n");
    }
}

UpDevice *ubatt_find_laptop_battery(UpClient *upower)
{
    GPtrArray *devices = up_client_get_devices2(upower);
    UpDevice *device = NULL;

    for (int i=0; i<devices->len; i++)
    {
        UpDevice *this_dev = g_ptr_array_index(devices, i);
        gchar *text = up_device_to_text (this_dev);
        gboolean power_supply;
        UpDeviceKind kind;
        g_object_get(this_dev,
                     "power-supply", &power_supply,
                     "kind", &kind,
                     NULL);
        if (power_supply == TRUE && kind == UP_DEVICE_KIND_BATTERY)
            device = this_dev;
        g_free (text);
    }

    if (device != NULL)
        g_object_ref(device);
    g_ptr_array_unref(devices);
    return device;
}

gchar *ubatt_format_time(gint64 seconds)
{
    if (seconds > 1800)
    {
        gint hours = seconds / 3600;
        gint minutes = (seconds - (hours*3600)) / 60;
        minutes = round(minutes / 5.0) * 5;
        if (minutes != 0 && hours != 0)
            return g_strdup_printf("%dh %dm", hours, minutes);
        else if (hours == 0)
            return g_strdup_printf("%dm", minutes);
        else
            return g_strdup_printf("%dh", hours);
    }
    else
    {
        return g_strdup_printf("%ldm", seconds / 60);
    }
}

void ubatt_show_battery(UpDevice *battery)
{
    UpDeviceKind kind;
    gdouble percent;
    UpDeviceState state;
    const gchar *label, *statelabel;
    gint64 time_empty, time_full;

    g_object_get(battery,
                 "kind", &kind,
                 "percentage", &percent,
                 "state", &state,
                 "time-to-empty", &time_empty,
                 "time-to-full", &time_full,
                 NULL);

    switch (kind)
    {
    case UP_DEVICE_KIND_BATTERY:
        label = "Battery";
        break;
    case UP_DEVICE_KIND_UPS:
        label = "UPS";
        break;
    case UP_DEVICE_KIND_MOUSE:
        label = "Mouse";
        break;
    default:
        label = up_device_kind_to_string(kind);
    }

    switch (state)
    {
    case UP_DEVICE_STATE_CHARGING:
        statelabel = "Charging";
        break;
    case UP_DEVICE_STATE_DISCHARGING:
        statelabel = "Discharging";
        break;
    case UP_DEVICE_STATE_EMPTY:
        statelabel = "Empty";
        break;
    case UP_DEVICE_STATE_FULLY_CHARGED:
        statelabel = "Fully charged";
        break;
    default:
        statelabel = up_device_state_to_string(state);
    }

    g_print("%s: %d%%, %s", label, (int)percent, statelabel);
    gchar *time_string = NULL;
    if (state == UP_DEVICE_STATE_DISCHARGING)
    {
        if (time_empty != 0)
            time_string = ubatt_format_time(time_empty);
    }
    else if (state == UP_DEVICE_STATE_CHARGING)
    {
        if (time_full != 0)
            time_string = ubatt_format_time(time_full);
    }
    if (time_string != NULL)
    {
        g_print(" %s remaining", time_string);
        g_free(time_string);
    }
    g_print("\n");
}

int main(int argc, char **argv)
{
    UpClient *upower = NULL;
    GError *error = NULL;
    GOptionContext *opts;
    int ret;

    opts = g_option_context_new("- show upower battery status");
    g_option_context_add_main_entries(opts, entries, NULL);
    if (!g_option_context_parse(opts, &argc, &argv, &error))
    {
        g_print("command line error: %s\n", error->message);
        ret = 1;

        return ret;
    }

    upower = up_client_new();

    if (upower == NULL)
    {
        g_print("Could not connect to upower");
        ret = 2;

        return ret;
    }

    if (opt_ver)
    {
        const gchar *up_ver = up_client_get_daemon_version(upower);

        g_print("UPower version: %s\n", up_ver);

        ret = 0;
    }
    else
    {
        UpDevice *battery = ubatt_find_laptop_battery(upower);
        gdouble percent;
        UpDeviceState state;
        if (battery)
        {
            g_object_get(battery,
                         "percentage", &percent,
                         "state", &state,
                         NULL);
        }
        if (opt_print)
        {
            if (battery)
            {
                g_print("%.0f\n", percent);
                ret = 0;
            }
            else
            {
                g_print("no battery\n");
                ret = 1;
            }
        }
        else if (opt_ret)
        {
            if (up_client_get_on_battery(upower))
                ret = 1;
            else
                ret = 0;
        }
        else if (opt_retcharge)
        {
            if (battery)
                ret = percent;
            else
                ret = 255;
        }
        else
        {
            ubatt_show_on_battery(upower);

            if (battery != NULL)
            {
                ubatt_show_battery(battery);
            }
        }
        if (battery != NULL)
        {
            g_object_unref(battery);
        }
    }

    g_object_unref(upower);
    return ret;
}
