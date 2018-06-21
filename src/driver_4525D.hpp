#pragma once

#include "ch.h"
#include "hal.h"


class driver_4525D {
private:
    bool _initialized = false;
    bool _healthy = false;

    I2CDriver *_driver;

    uint8_t _address = 0x00;
    const uint8_t _posibble_address[3] = { 0x28, 0x36, 0x46 };

    systime_t _last_sample_time = 0;
    systime_t _measurement_started = 0;

    float _psi_range = 1.0f;

    float _last_pressure_pa = 0.0f; //Last measurement in Pa
    float _last_temperature = 0.0f; //Last temperature measurement

    void _start_measurement() {
        _measurement_started = 0;
        i2cAcquireBus(_driver);
        uint8_t cmd[2];
        if(i2cMasterReceiveTimeout(_driver, _address, cmd, 2, MS2ST(10)) == MSG_OK) {
            _measurement_started = chVTGetSystemTime();
        }
        i2cReleaseBus(_driver);
    }

    void _get_data() {
        uint8_t data[4];

        _measurement_started = 0;
        i2cAcquireBus(_driver);
        msg_t res = i2cMasterReceiveTimeout(_driver, _address, data, 4, MS2ST(5));
        i2cReleaseBus(_driver);
        if(res != MSG_OK) {
            _healthy = false;
            return;
        }

        uint8_t status = (data[0] & 0xC0) >> 6;
        if(status == 2 || status == 3) {
            _healthy = false;
            return;
        } else {
            _healthy = true;
        }

        int16_t dp, temp;
        dp = (data[0] << 8) + data[1];
        dp = dp & 0x3FFF;
        temp = (data[2] << 8) + data[3];
        temp = (temp & 0xFFE0) >> 5;

        //Reject possibly bad values
        if(dp == 0 || dp == 0x3FFF || temp == 0 || temp == 0x7FF) {
            _healthy = false;
            return;
        }

        _last_pressure_pa = _get_pressure(dp);
        _last_temperature = _get_temperature(temp);

        _last_sample_time = chVTGetSystemTime();
    }

    float _get_pressure(int16_t raw) const {
        const float p_max = _psi_range;
        const float p_min = -p_max;
        const float psi_to_pa = 6894.757f;

        float press_psi = -((raw - 0.1f*16383)*(p_max-p_min)/(0.8f*16383) + p_min);
        float press_pa = press_psi * psi_to_pa;
        return press_pa;
    }

    float _get_temperature(int16_t raw_t) const {
        float temp = ((200.0f * raw_t) / 2047) - 50;
        return temp;
    }

public:
    driver_4525D(I2CDriver *driver) :
        _driver(driver) { }

    bool init() {
        if(_driver == NULL) { //No driver specified
            return false;
        }

        if(_initialized && _address != 0x00) {
            return true; //Already initialized
        }

        for(uint8_t i = 0; i < sizeof(_posibble_address); i++) {
            _address = _posibble_address[i];
            _start_measurement();
            if(_measurement_started != 0) {
                _initialized = true;
                return true;
            }
        }
        _address = 0x00;
        return false; //No device found
    }

    void poll() {
        if(_measurement_started == 0) {
            _start_measurement();
            return;
        }
        if((systime_t)(chVTGetSystemTime() - _measurement_started) > MS2ST(5)) {
            _get_data();
            _start_measurement();
        }
    }

    bool get_temperature(float &temp) {
        if((systime_t)(chVTGetSystemTime() - _last_sample_time) > MS2ST(50)) {
            return false;
        }
        temp = _last_temperature;
        return true;
    }

    bool get_pressure(float &press) {
        if((systime_t)(chVTGetSystemTime() - _last_sample_time) > MS2ST(100)) {
            return false;
        }
        press = _last_pressure_pa;
        return true;
    }

    void set_psi_range(float psi_range) {
        _psi_range = psi_range;
    }
};

