#include "Constants.h"
#include "UDPClient.h"
#include "Utils/Utils.h"

/*
    Creates a serialized message, stores it in the buffer pointer and updates 
    the message_length variable.
*/
bool create_pb(MessageType type, uint8_t *buffer, size_t *message_length) {
    return create_pb(type, buffer, message_length, IPAddress(), 0);
}

/*
    Creates a serialized message, stores it in the buffer pointer and updates 
    the message_length variable.
*/
bool create_pb(MessageType type, uint8_t *buffer, size_t *message_length, IPAddress local_ip, uint16_t local_port) {
    bool status;

    // Initialize the message memory
    Message message = Message_init_zero;

    // Create a stream that will write to our buffer
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, MAX_MESSAGE_BYTES);

    // Fill the message. IMPORTANT: the has_ fields must be explicitly set to
    // true, otherwise they will not be encoded.
    message.has_sender = true;
    message.sender = Sender_CLIENT_AMBILIGHT;

    switch (type) {
        case MessageType_CONFIG: {
            message.has_type = true;
            message.type = MessageType_CONFIG;

            message.has_config = true;
            message.config.has_ipv4 = true;
            strlcpy(message.config.ipv4, local_ip.toString().c_str(), sizeof(message.config.ipv4) - 1);

            message.config.has_port = true;
            message.config.port = local_port;

            message.config.has_led_format = true;
            message.config.led_format = LedFormat_RECTANGULAR_PERIMETER;

            message.config.has_num_leds = true;
            message.config.num_leds = NUM_LEDS;
            break;
        }
        case MessageType_HEARTBEAT: {
            message.has_type = true;
            message.type = MessageType_HEARTBEAT;
            break;
        }
        default: {
            print("pb type not recognized!\n");
            return false;
        }
    }
    // Encode the message
    status = pb_encode(&stream, Message_fields, &message);
    *message_length = stream.bytes_written;

    if (!status) {
        print("Encoding failed: %s\n", PB_GET_ERROR(&stream));
    }

    return status;
}

/*
    Parses a protobuf packet received via UDP and stores the contents in memory
    at the passed in message pointer. Returns true is successful, false
    otherwise.
*/
bool parse_pb(WiFiUDP *udp, int packet_size, Message *message) {
    bool status;
    uint8_t buffer[MAX_MESSAGE_BYTES];
    int len = udp->read(buffer, MAX_MESSAGE_BYTES - 1);

    // Null-terminate
    if (len > 0) {
        buffer[len] = 0;
    }

    // Create a stream that reads from the buffer
    pb_istream_t stream = pb_istream_from_buffer(buffer, packet_size);

    // Now decode the message
    status = pb_decode(&stream, Message_fields, message);

    if (!status) {
        printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
    }

    return status;
}