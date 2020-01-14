#ifndef RC_RECEIVER_DECODER_H__
#define RC_RECEIVER_DECODER_H__

typedef void (* rc_receiver_event_handler_t)(uint32_t channel, uint32_t value);

void rc_receiver_init(rc_receiver_event_handler_t event_handler);

#endif // RC_RECEIVER_DECODER_H__