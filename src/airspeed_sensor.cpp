#include <ch.hpp>
#include <driver_4525D.hpp>
#include <node.hpp>
#include <config.hpp>

#include <uavcan/equipment/air_data/RawAirData.hpp>

namespace airspeed_sensor {

float last_press_pa;
float last_temp;

const float kelvin_offset = 273.15f;

os::config::Param<float> psi_range("air_speed.psi_range", 1.0f, 0.0f, 100.0f); //REBOOT to take effect
os::config::Param<uint32_t> pub_perion_usec("air_speed.pub_per", 20000, 10000, 1000000);

class AirspeedSenorThread : public chibios_rt::BaseStaticThread<1024> {
    driver_4525D _driver;

    void publish(float press_pa, float temp) {

        //TODO: check if node is initialized
        last_press_pa = press_pa;
        last_temp = temp;

        static uavcan::equipment::air_data::RawAirData raw_air_data;
        raw_air_data.differential_pressure = press_pa;
        raw_air_data.differential_pressure_sensor_temperature = temp;

        auto& node = Node::getNode();
        static uavcan::Publisher<uavcan::equipment::air_data::RawAirData> raw_air_pub(node);

        raw_air_pub.broadcast(raw_air_data);
    }

public:
    AirspeedSenorThread():
        _driver(&I2CD1) {}

    void main() override {
        setName("airspeed_sensor");

        while(true) {
            if(!_driver.init()) {
                //Driver init failed.
                continue;
            }

            _driver.set_psi_range(psi_range.get());

            systime_t slp_until = chVTGetSystemTime();
            while(true) {
                slp_until += US2ST(pub_perion_usec.get());

                float press, temp;
                _driver.poll();
                bool res = _driver.get_pressure(press);
                _driver.get_temperature(temp);

                if(res) {
                    temp += kelvin_offset;
                    Node::getNode().setHealthOk();
                    publish(press, temp);
                } else {
                    Node::getNode().setHealthError();
                }
                chThdSleepUntil(slp_until);
            }

        }
    }
} airspeed_sensor_thread;

void init() {
    airspeed_sensor_thread.start(NORMALPRIO);
}

}
