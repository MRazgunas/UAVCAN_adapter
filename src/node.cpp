#include <node.hpp>
#include <uavcan/uavcan.hpp>
#include <uavcan_stm32/uavcan_stm32.hpp>

#include <uavcan/protocol/param_server.hpp>
#include <uavcan/protocol/file/BeginFirmwareUpdate.hpp>

#include <config/config.hpp>

#include <ch.hpp>
#include "hardware.hpp"

namespace Node {

    os::config::Param<uint8_t> node_id("node.id", 1, 0, 127); //0 - automatic detection (not supported)
    os::config::Param<uint32_t> bus_speed("node.speed", 1000000, 125000, 1000000);

    uavcan_stm32::CanInitHelper<> can;

    uavcan::protocol::SoftwareVersion firmware_version;

    FirmwareUpdateRequestCallback g_on_firmware_update_requested;

    uavcan::Node<NodePoolSize>& getNode() {
        static uavcan::Node<NodePoolSize> node(can.driver, uavcan_stm32::SystemClock::instance());
        return node;
    }

    /*
     * Param access server
     */
    class ParamManager : public uavcan::IParamManager
    {
        void convert(float native_value, ConfigDataType native_type, Value& out_value) const
        {
            if (native_type == CONFIG_TYPE_BOOL)
            {
              out_value.to<Value::Tag::boolean_value>() = !uavcan::isCloseToZero(native_value);
            }
            else if (native_type == CONFIG_TYPE_INT)
            {
              out_value.to<Value::Tag::integer_value>() = static_cast<std::int64_t>(native_value);
            }
            else if (native_type == CONFIG_TYPE_FLOAT)
            {
              out_value.to<Value::Tag::real_value>() = native_value;
            }
            else
            {
              ; // Invalid type
            }
        }

        void convert(float native_value, ConfigDataType native_type, NumericValue& out_value) const
        {
            if (native_type == CONFIG_TYPE_INT)
            {
              out_value.to<NumericValue::Tag::integer_value>() = static_cast<std::int64_t>(native_value);
            }
            else if (native_type == CONFIG_TYPE_FLOAT)
            {
              out_value.to<NumericValue::Tag::real_value>() = native_value;
            }
            else
            {
              ; // Not applicable
            }
        }

        void getParamNameByIndex(Index index, Name& out_name) const override
        {
            const char* name = configNameByIndex(index);
            if (name != nullptr)
            {
              out_name = name;
            }
        }

        void assignParamValue(const Name& name, const Value& value) override
        {
            float native_value = 0.F;

            if (value.is(Value::Tag::boolean_value))
            {
              native_value = (*value.as<Value::Tag::boolean_value>()) ? 1.F : 0.F;
            }
            else if (value.is(Value::Tag::integer_value))
            {
              native_value = static_cast<float>(*value.as<Value::Tag::integer_value>());
            }
            else if (value.is(Value::Tag::real_value))
            {
              native_value = *value.as<Value::Tag::real_value>();
            }
            else
            {
              return;
            }

            (void)configSet(name.c_str(), native_value);
        }

        void readParamValue(const Name& name, Value& out_value) const override
        {
            ConfigParam descr;
            const int res = configGetDescr(name.c_str(), &descr);
            if (res >= 0)
            {
              convert(configGet(name.c_str()), descr.type, out_value);
            }
        }

        void readParamDefaultMaxMin(const Name& name, Value& out_default,
                                  NumericValue& out_max, NumericValue& out_min) const override
        {
            ConfigParam descr;
            const int res = configGetDescr(name.c_str(), &descr);
            if (res >= 0)
            {
              convert(descr.default_, descr.type, out_default);
              convert(descr.max, descr.type, out_max);
              convert(descr.min, descr.type, out_min);
            }
        }

        int saveAllParams() override
        {
            return configSave();
        }

        int eraseAllParams() override
        {
            return configErase();
        }
    } param_manager;

  /*
   * Firmware update handler
   */
  typedef uavcan::ServiceServer<uavcan::protocol::file::BeginFirmwareUpdate,
      void (*)(const uavcan::ReceivedDataStructure<uavcan::protocol::file::BeginFirmwareUpdate::Request>&,
               uavcan::protocol::file::BeginFirmwareUpdate::Response&)>
      BeginFirmwareUpdateServer;

    BeginFirmwareUpdateServer& getBeginFirmwareUpdateServer()
    {
      static BeginFirmwareUpdateServer srv(getNode());
      return srv;
    }

    void handleBeginFirmwareUpdateRequest(
      const uavcan::ReceivedDataStructure<uavcan::protocol::file::BeginFirmwareUpdate::Request>& request,
      uavcan::protocol::file::BeginFirmwareUpdate::Response& response)
    {
        //assert(bus_speed.get() > 0);
        //assert(g_node_id.isUnicast());

    //      os::lowsyslog("BeginFirmwareUpdate request from %d\n", int(request.getSrcNodeID().get()));

        if (g_on_firmware_update_requested)
        {
            response.error = g_on_firmware_update_requested(request);
        }
        else
        {
          //os::lowsyslog("UAVCAN FIRMWARE UPDATE HANDLER NOT SET\n");
          response.error = response.ERROR_UNKNOWN;
          response.optional_error_message = "Not supported by application";
        }
    }

    void uavcanNodeThread::main() {
        can.init(bus_speed.get());

        getNode().setName("org.mr.uavcan_adapter");
        if(node_id.get() > 0) {
            getNode().setNodeID(node_id.get());
        }

        getNode().setSoftwareVersion(firmware_version);

        uavcan::protocol::HardwareVersion hw_ver;

        hw_ver.major = 1;
        hw_ver.minor = 0;

        Hardware::UUID uuid = Hardware::readUniqueID();
        memcpy(&hw_ver.unique_id, &uuid, sizeof(uuid));

        getNode().setHardwareVersion(hw_ver);

        if (getNode().start() < 0) {
          chSysHalt("UAVCAN init fail");
        }
        //    kv_pub.init();

        uavcan::ParamServer server(getNode());
        server.start(&param_manager);

        const int begin_firmware_update_res = getBeginFirmwareUpdateServer().start(&handleBeginFirmwareUpdateRequest);


        getNode().setModeOperational();

        while(true) {
            if (getNode().spin(uavcan::MonotonicDuration::fromMSec(500)) < 0) {
              chSysHalt("UAVCAN spin fail");
            }
            //kv_pub.broadcast(kv_msg);
        }
    }

    uint32_t getCANBitRate() {
        return bus_speed.get();
    }

    uavcanNodeThread node_thread;

    void init(uint32_t bit_rate,
            uint8_t node_id,
            uint8_t firmware_vers_major,
            uint8_t firmware_vers_minor,
            uint32_t vcs_commit,
            uint64_t crc64,
            const FirmwareUpdateRequestCallback& on_firmware_update_requested) {
        firmware_version.major = firmware_vers_major;
        firmware_version.minor = firmware_vers_minor;
        firmware_version.vcs_commit = vcs_commit;
        firmware_version.image_crc = crc64;
        firmware_version.optional_field_flags =
              firmware_version.OPTIONAL_FIELD_FLAG_IMAGE_CRC | firmware_version.OPTIONAL_FIELD_FLAG_VCS_COMMIT;

        g_on_firmware_update_requested = on_firmware_update_requested;

        node_thread.start(NORMALPRIO);

    }

}

