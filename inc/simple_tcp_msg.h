#ifndef _SIMPLE_TCP_MSG_HEADER_
#define _SIMPLE_TCP_MSG_HEADER_

// pic is a 8x5 ascii art 
#define profile_pic_string_len 45 
#define profile_pic_height 5 
#define profile_pic_width  8 

typedef enum{
    msg_type_error = 0,
    msg_type_ping = 1,
    msg_type_info,
    msg_type_message,
    msg_type_status,
    
    msg_type_count,
}msg_type_t;

// error
// {
//     "type": 0
// }

// ping
// {
//     "type": 1
// }

// info
// {
//     "type": 2,
//     "info": {
//         "name": "node_name"
//     }
// }

#endif