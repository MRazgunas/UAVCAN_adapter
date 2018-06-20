#ifndef NODE_HPP
#define NODE_HPP

#include <uavcan/uavcan.hpp>
#include <ch.hpp>

#include <uavcan/protocol/file/BeginFirmwareUpdate.hpp>
#include <uavcan_stm32/uavcan_stm32.hpp>

namespace Node {

    constexpr unsigned NodePoolSize = 2048;
    uavcan::Node<NodePoolSize>& getNode();
    void publishKeyValue(const char *key, float value);

    uint32_t getCANBitRate();

    struct Lock : uavcan_stm32::MutexLocker
    {
        Lock();
    };

    using FirmwareUpdateRequestCallback =
        std::function<uavcan::StorageType<uavcan::protocol::file::BeginFirmwareUpdate::Response::FieldTypes::error>::Type
    (const uavcan::ReceivedDataStructure<uavcan::protocol::file::BeginFirmwareUpdate::Request>&)>;

void init(uint32_t bit_rate,
        uint8_t node_id,
        uint8_t firmware_vers_major,
        uint8_t firmware_vers_minor,
        uint32_t vcs_commit,
        uint64_t crc64,
        const FirmwareUpdateRequestCallback& on_firmware_update_requested);

    class uavcanNodeThread : public chibios_rt::BaseStaticThread<4096> {
    public:
        void main();
    };

}

#endif

