#pragma once

#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include "proto/ambilight.pb.h"

#include <WiFiUdp.h>

// Function Prototypes
bool parse_pb(WiFiUDP *udp, int packet_size, Message *message);
bool create_pb(MessageType type, uint8_t *buffer, size_t *message_length);
bool create_pb(MessageType type, uint8_t *buffer, size_t *message_length, IPAddress local_ip, uint16_t local_port);