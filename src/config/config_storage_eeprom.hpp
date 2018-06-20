/*
 * Copyright (c) 2015 Zubax, zubax.com
 * Distributed under the MIT License, available in the file LICENSE.
 * Author: Pavel Kirienko <pavel.kirienko@zubax.com>
 */

#pragma once
#if HAL_USE_I2C == TRUE
#include <config/config.hpp>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <cerrno>

#include <ch.h>
#include <hal.h>

namespace os
{
namespace stm32
{
/**
 * See os::config::IStorageBackend.
 */
class ConfigStorageBackend_eeprom : public os::config::IStorageBackend
{
    I2CDriver* driver_;
    const uint8_t address_;
    const std::size_t size_;
    const std::size_t page_size_;

public:
    ConfigStorageBackend_eeprom(I2CDriver* bus, uint8_t storage_address,
                         std::size_t storage_size, std::size_t page_size) :
        driver_(bus),
        address_(storage_address),
        size_(storage_size),
        page_size_(page_size)
    {
        assert(size_ > 0);
        assert(page_size > 0);
    }

    int read(std::size_t offset, void* data, std::size_t len) override
    {
        if ((data == nullptr) ||
            (offset + len) > size_)
        {
            assert(false);
            return -EINVAL;
        }

        msg_t res;
        uint8_t *b = (uint8_t *)data;
        uint8_t buff[1];
        buff[0] = offset & 0xFF;
        res = i2cMasterTransmitTimeout(driver_, address_, buff, 1, b, len, MS2ST(5));

        if (res != 0)
            return -EINVAL;
        return 0;
    }

    int write(std::size_t offset_tmp, const void* data, std::size_t len) override
    {
        if ((data == nullptr) ||
            (offset_tmp + len) > size_)
        {
            assert(false);
            return -EINVAL;
        }

        uint16_t addr = offset_tmp;
        uint8_t send_buff[page_size_ + 2];
        uint8_t offset = (uint8_t) addr % page_size_;
        uint8_t *b = (uint8_t *)data;
        size_t data_size = len;
        msg_t res;

        if(addr+len > size_) {
            chSysHalt("EEPROM full");
        }

        if(data_size > page_size_ || len == 0) {
            return -EINVAL; //can't write more than one page at time or write 0 data
        }
        if(offset+data_size <= page_size_) { //we do not cross page boundary send all data
            send_buff[0] = addr & 0xFF;
            memcpy(&send_buff[1], b, len);
            res = i2cMasterTransmitTimeout(driver_, address_, send_buff, len+1, NULL, 0, MS2ST(10));
            if(res != MSG_OK)
                return -EINVAL;
            wait_for_write_end();

        } else { //crossing page boundary max data supported is page_size_
            data_size = page_size_ - offset;
            send_buff[0] = addr & 0xFF;
            memcpy(&send_buff[1], b, data_size);
            res = i2cMasterTransmitTimeout(driver_, address_, send_buff, data_size+1, NULL, 0, MS2ST(10));
            if(res != MSG_OK)
                return -EINVAL;
            wait_for_write_end();

            addr += data_size;
            data_size = len - data_size;
            send_buff[0] = addr & 0xFF;
            memcpy(&send_buff[1], b, data_size);
            res = i2cMasterTransmitTimeout(driver_, address_, send_buff, data_size+1, NULL, 0, MS2ST(10));
            if(res != MSG_OK)
                return -EINVAL;
            wait_for_write_end();


           // chSysHalt("EEPROM boundry");
            return 0;
        }
        return 0;
    }

    void wait_for_write_end(void) {
        msg_t res;
        uint8_t n = 0;
        do {
            res = i2cMasterTransmitTimeout(driver_, address_, (uint8_t *) 0xFF, 1, NULL, 0, MS2ST(5));
            n++;
            chThdSleepMicroseconds(500); //poling at 0.5ms interval
        } while(res != MSG_OK);
    }

    int erase() override
    {
        return 0;
        //return FlashWriter().erase(reinterpret_cast<void*>(address_), size_) ? 0 : -EIO;
    }
};

}
}

#endif
