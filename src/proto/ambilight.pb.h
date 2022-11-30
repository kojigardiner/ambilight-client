/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.6 */

#ifndef PB_AMBILIGHT_PB_H_INCLUDED
#define PB_AMBILIGHT_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Enum definitions */
typedef enum _MessageType { 
    MessageType_ACK = 0, 
    MessageType_DISCOVERY = 1, 
    MessageType_CONFIG = 2, 
    MessageType_DATA = 3 
} MessageType;

typedef enum _Sender { 
    Sender_SERVER = 0, 
    Sender_CLIENT_AMBILIGHT = 1, 
    Sender_CLIENT_AUDIOBOX = 2 
} Sender;

typedef enum _LedFormat { 
    LedFormat_SERPENTINE_GRID = 0, 
    LedFormat_RECTANGULAR_PERIMETER = 1 
} LedFormat;

/* Struct definitions */
typedef struct _Message_Config { 
    bool has_ipv4;
    char ipv4[16];
    bool has_port;
    int32_t port;
    bool has_num_leds;
    int32_t num_leds;
    bool has_led_format;
    LedFormat led_format;
} Message_Config;

typedef PB_BYTES_ARRAY_T(768) Message_Data_led_data_t;
typedef PB_BYTES_ARRAY_T(64) Message_Data_led_palette_t;
typedef struct _Message_Data { 
    bool has_led_data;
    Message_Data_led_data_t led_data;
    bool has_led_palette;
    Message_Data_led_palette_t led_palette;
    bool has_led_position;
    int32_t led_position;
} Message_Data;

typedef struct _Message { 
    bool has_sender;
    Sender sender;
    bool has_type;
    MessageType type;
    bool has_sequence_number;
    int32_t sequence_number;
    bool has_timestamp;
    int32_t timestamp;
    bool has_config;
    Message_Config config;
    bool has_data;
    Message_Data data;
} Message;


/* Helper constants for enums */
#define _MessageType_MIN MessageType_ACK
#define _MessageType_MAX MessageType_DATA
#define _MessageType_ARRAYSIZE ((MessageType)(MessageType_DATA+1))

#define _Sender_MIN Sender_SERVER
#define _Sender_MAX Sender_CLIENT_AUDIOBOX
#define _Sender_ARRAYSIZE ((Sender)(Sender_CLIENT_AUDIOBOX+1))

#define _LedFormat_MIN LedFormat_SERPENTINE_GRID
#define _LedFormat_MAX LedFormat_RECTANGULAR_PERIMETER
#define _LedFormat_ARRAYSIZE ((LedFormat)(LedFormat_RECTANGULAR_PERIMETER+1))


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define Message_init_default                     {false, _Sender_MIN, false, _MessageType_MIN, false, 0, false, 0, false, Message_Config_init_default, false, Message_Data_init_default}
#define Message_Config_init_default              {false, "", false, 0, false, 0, false, _LedFormat_MIN}
#define Message_Data_init_default                {false, {0, {0}}, false, {0, {0}}, false, 0}
#define Message_init_zero                        {false, _Sender_MIN, false, _MessageType_MIN, false, 0, false, 0, false, Message_Config_init_zero, false, Message_Data_init_zero}
#define Message_Config_init_zero                 {false, "", false, 0, false, 0, false, _LedFormat_MIN}
#define Message_Data_init_zero                   {false, {0, {0}}, false, {0, {0}}, false, 0}

/* Field tags (for use in manual encoding/decoding) */
#define Message_Config_ipv4_tag                  1
#define Message_Config_port_tag                  2
#define Message_Config_num_leds_tag              3
#define Message_Config_led_format_tag            4
#define Message_Data_led_data_tag                1
#define Message_Data_led_palette_tag             2
#define Message_Data_led_position_tag            3
#define Message_sender_tag                       1
#define Message_type_tag                         2
#define Message_sequence_number_tag              3
#define Message_timestamp_tag                    4
#define Message_config_tag                       5
#define Message_data_tag                         6

/* Struct field encoding specification for nanopb */
#define Message_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, UENUM,    sender,            1) \
X(a, STATIC,   OPTIONAL, UENUM,    type,              2) \
X(a, STATIC,   OPTIONAL, INT32,    sequence_number,   3) \
X(a, STATIC,   OPTIONAL, INT32,    timestamp,         4) \
X(a, STATIC,   OPTIONAL, MESSAGE,  config,            5) \
X(a, STATIC,   OPTIONAL, MESSAGE,  data,              6)
#define Message_CALLBACK NULL
#define Message_DEFAULT NULL
#define Message_config_MSGTYPE Message_Config
#define Message_data_MSGTYPE Message_Data

#define Message_Config_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, STRING,   ipv4,              1) \
X(a, STATIC,   OPTIONAL, INT32,    port,              2) \
X(a, STATIC,   OPTIONAL, INT32,    num_leds,          3) \
X(a, STATIC,   OPTIONAL, UENUM,    led_format,        4)
#define Message_Config_CALLBACK NULL
#define Message_Config_DEFAULT NULL

#define Message_Data_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, BYTES,    led_data,          1) \
X(a, STATIC,   OPTIONAL, BYTES,    led_palette,       2) \
X(a, STATIC,   OPTIONAL, INT32,    led_position,      3)
#define Message_Data_CALLBACK NULL
#define Message_Data_DEFAULT NULL

extern const pb_msgdesc_t Message_msg;
extern const pb_msgdesc_t Message_Config_msg;
extern const pb_msgdesc_t Message_Data_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define Message_fields &Message_msg
#define Message_Config_fields &Message_Config_msg
#define Message_Data_fields &Message_Data_msg

/* Maximum encoded size of messages (where known) */
#define Message_Config_size                      41
#define Message_Data_size                        848
#define Message_size                             920

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
