#include <stdio.h>
#include <glib.h>
#include <upower.h>

static gboolean opt_ver;

static GOptionEntry entries[] =
{
    {"version", 'V', 0, G_OPTION_ARG_NONE, &opt_ver, "Show UPower daemon version", NULL},
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

    g_object_ref(device);
    g_ptr_array_unref(devices);
    return device;
}

void ubatt_show_battery(UpDevice *battery)
{
    UpDeviceKind kind;
    gdouble percent;
    UpDeviceState state;
    const gchar *label, *statelabel;

    g_object_get(battery,
                 "kind", &kind,
                 "percentage", &percent,
                 "state", &state,
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

    g_print("%s: %d%%, %s\n", label, (int)percent, statelabel);
}

int main(int argc, char **argv)
{
    UpClient *upower;
    GError *error = NULL;
    GOptionContext *opts;

    opts = g_option_context_new("- show upower battery status");
    g_option_context_add_main_entries(opts, entries, NULL);
    if (!g_option_context_parse(opts, &argc, &argv, &error))
    {
        g_print("command line error: %s\n", error->message);
        return 1;
    }

    upower = up_client_new();

    if (upower == NULL)
    {
        g_print("Could not connect to upower");
        return 2;
    }

    if (opt_ver)
    {
        const gchar *up_ver = up_client_get_daemon_version(upower);

        g_print("UPower version: %s\n", up_ver);

        return 0;
    }

    ubatt_show_on_battery(upower);

    UpDevice *battery = ubatt_find_laptop_battery(upower);
    if (battery)
    {
        gdouble percent;
        UpDeviceState state;
        g_object_get(battery,
                     "percentage", &percent,
                     "state", &state,
                     NULL);
        //g_print("Found battery: %.1f%% %s\n", percent, up_device_state_to_string(state));

        ubatt_show_battery(battery);

        g_object_unref(battery);
    }
    
    return 0;
}
