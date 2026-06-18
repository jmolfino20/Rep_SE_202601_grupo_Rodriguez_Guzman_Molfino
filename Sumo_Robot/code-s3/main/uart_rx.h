#ifndef UART_RX_H
#define UART_RX_H

void uart_border_init(void);   /* solo UART_NUM_1 (CamBorder) */
void uart_id_init(void);       /* solo UART_NUM_2 (CamID)     */
void uart_rx_init(void);       /* ambos: border + id          */

void uart_border_task(void *arg);
void uart_id_task(void *arg);

#endif
