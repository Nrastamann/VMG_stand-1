#pragma once

class vmg_baro_driver;

static constexpr uint8_t BARO_AMOUNT {3};
static constexpr uint8_t BARO_DRIVERS_AMOUNT {3};


class vmg_baro{
    friend class vmg_baro_driver;
public: 
    void add_baro_driver(vmg_baro_driver* driver);
    void update();
    void update_calibration();
    void calibrate();

    float get_pressure(){return baro_physical[primary].pressure;}
    float get_pressure(uint8_t instance){return baro_physical[instance].pressure;}

    float get_temp(){return baro_physical[primary].temperature;}
    float get_temp(uint8_t instance){return baro_physical[instance].temperature;}

    float get_altitude(){return baro_physical[primary].altitude;}
    float get_altitude(uint8_t instance){return baro_physical[instance].altitude;}

    float get_last_change_ms(){return baro_physical[primary].last_change_ms;}
    float get_last_change_ms(uint8_t instance){return baro_physical[instance].last_change_ms;}

    float get_last_update_ms(){return baro_physical[primary].last_update_ms;}
    float get_last_update_ms(uint8_t instance){return baro_physical[instance].last_update_ms;}

private:
    void _probe_sensor(uint8_t pin, uint8_t address);
    

    std::array<vmg_baro_driver*, BARO_DRIVERS_AMOUNT> baro_drivers;
    uint8_t amount_drivers;

    class sensor{
        uint32_t last_update_ms;
        uint32_t last_change_ms;
        
        float pressure;
        float temperature;
        float altitude;

        //bool healthy;
        //bool calibrated;
    };
    
    uint8_t primary;
    uint8_t pin;
    
    std::array<sensor, BARO_AMOUNT> baro_physical;
    uint8_t amount_sensors;
};
