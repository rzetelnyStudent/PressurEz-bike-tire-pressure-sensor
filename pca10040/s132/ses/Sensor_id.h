#ifndef SENSOR_ID_H
#define SENSOR_ID_H


#include <stdbool.h>
#include <stdint.h>

const uint8_t SENSOR_ID_BYTE_COUNT = 3;      // each sensor is signed with 3 byte long id


// Sensor id struct
struct Sensor_id
{
    uint8_t id_hex[SENSOR_ID_BYTE_COUNT];
};

#endif