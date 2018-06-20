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

        _last_sample_time = chVTGetSystemTime();
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
        if((systime_t)(chVTGetSystemTime() - _measurement_started) > MS2ST(10)) {
            _get_data();
            _start_measurement();
        }
    }
};

