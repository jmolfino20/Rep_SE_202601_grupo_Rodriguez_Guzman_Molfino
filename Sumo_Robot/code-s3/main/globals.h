#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>

/* Set by uart_rx tasks, read by state_machine and audio */
extern volatile uint8_t g_id_detected;
extern volatile uint8_t g_border_detected;

/* Set by audio_task, read by state_machine */
extern volatile uint8_t g_audio_override;

#endif /* GLOBALS_H */
