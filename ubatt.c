#include <stdio.h>
#include <glib.h>
#include <upower.h>

void ubatt_show_on_battery(UpClient *upower)
{
    if (up_client_get_on_battery(upower))
    {
        g_print("On battery power\n");
    }
    else
    {
        g_print("Not on battery power\n");
    }
}

UpDevice *ubatt_find_laptop_battery(UpClient *upower)
{
    GPtrArray *devices = up_client_get_devices2(upower);
    UpDevice *device = NULL;

    for (int i=0; i<devices->len; i++)
    {
        g_print("Device %d\n", i);
        UpDevice *this_dev = g_ptr_array_index(devices, i);
        gchar *text = up_device_to_text (this_dev);
        //g_print ("%s\n", text);
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

int main(int argc, char **argv)
{
    UpClient *upower;

    upower = up_client_new();

    if (upower == NULL)
    {
        g_print("Could not connect to upower");
        return 2;
    }

    const gchar *up_ver = up_client_get_daemon_version(upower);

    g_print("Upower version: %s\n", up_ver);

    ubatt_show_on_battery(upower);

    UpDevice *battery = ubatt_find_laptop_battery(upower);
    if (battery)
    {
        gdouble percent;
        g_object_get(battery, "percentage", &percent, NULL);
        g_print("Found battery: %.1f%%\n", percent);

        g_object_unref(battery);
    }
    
    return 0;
}
