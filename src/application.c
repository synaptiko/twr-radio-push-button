#include <application.h>

// report status every 5 minutes (comment out if needed)
#define STATUS_REPORT_INTERVAL (5 * 60 * 1000)
// or use 10 seconds for testing (uncomment if needed)
// #define STATUS_REPORT_INTERVAL (10 * 1000)

// update battery status every 1 hour
#define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000)

twr_button_t button;
bool button_pressed = false;
float battery_voltage = 0.0f;
bool battery_low = false;
bool battery_critical = false;

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        button_pressed = true;
    }
    else if (event == TWR_BUTTON_EVENT_RELEASE)
    {
        button_pressed = false;
    }
}

void battery_event_handler(twr_module_battery_event_t event, void *event_param)
{
    if (event == TWR_MODULE_BATTERY_EVENT_UPDATE)
    {
        twr_module_battery_get_voltage(&battery_voltage);
    }
    else if (event == TWR_MODULE_BATTERY_EVENT_LEVEL_LOW)
    {
        battery_low = true;
    }
    else if (event == TWR_MODULE_BATTERY_EVENT_LEVEL_CRITICAL)
    {
        battery_critical = true;
    }
}

void status_report_task(void *param)
{
    twr_radio_pub_bool("button/state", &button_pressed);
    twr_radio_pub_battery(&battery_voltage);

    if (battery_critical)
    {
        twr_radio_pub_bool("battery/critical", &battery_critical);
    }
    else if (battery_low)
    {
        twr_radio_pub_bool("battery/low", &battery_low);
    }

    twr_scheduler_plan_current_from_now(STATUS_REPORT_INTERVAL);
}

void application_init(void)
{
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    twr_module_battery_init();
    twr_module_battery_set_event_handler(battery_event_handler, NULL);
    twr_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);

    twr_radio_pairing_request("cellar-bucket-sensor", FW_VERSION);

    twr_scheduler_register(status_report_task, NULL, STATUS_REPORT_INTERVAL);
}
