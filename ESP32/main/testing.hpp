#include "packet.hpp"
#include <string>
static const char* TAG = "TESTING";
struct TestingMethods{
    static void init_test_packet(){
        packet_to_send.rpm_set(0);
    }

    static void log_packet_data(){
         ESP_LOGI(TAG,
            "PACKET_TO_SEND_DATA: \n%d - rpm, \n%d - weight \n%d - "
            "adc_voltage,\n%d - "
            "adc_current,\n%d - adc_disturbance,\n%d - adc_temp_1,\n%d - "
            "adc_temp_2",
            int(packet_to_send.rpm()), int(packet_to_send.adc(0)),
            int(packet_to_send.adc(1)), int(packet_to_send.adc(2)),
            int(packet_to_send.adc(3)), int(packet_to_send.adc(4)),
            int(packet_to_send.adc(5)));
    };
};

void show_packet_task(void* arg){
    for(;;){
        TestingMethods::log_packet_data();
        vTaskDelay(1000);
    }
}