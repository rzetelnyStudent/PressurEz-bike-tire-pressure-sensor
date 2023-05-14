#ifndef SENSOR_ID_H
#define SENSOR_ID_H


#include <stdbool.h>
#include <stdint.h>

const uint8_t SENSOR_ID_BYTE_COUNT = 3;      // each sensor is signed with 3 byte long id

// enum representing sensor in wheel position
enum Which_wheel
{
    FRONT_LEFT = 0,
    FRONT_RIGHT = 1,
    REAR_LEFT = 2,
    REAR_RIGHT = 3,
};

// Sensor id struct
struct Sensor_id
{
    uint8_t id_hex[SENSOR_ID_BYTE_COUNT];
    Which_wheel which_wheel;
};

#endif