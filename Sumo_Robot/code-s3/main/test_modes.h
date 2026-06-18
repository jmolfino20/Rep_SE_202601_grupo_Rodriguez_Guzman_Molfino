#ifndef TEST_MODES_H
#define TEST_MODES_H

/* Tareas para modos de prueba individual.
 * Seleccionadas en tiempo de compilación con ROBOT_MODE en config.h. */

void test_border_task(void *arg);   /* MODE_BORDER  */
void test_id_task(void *arg);       /* MODE_ID      */
void test_button_task(void *arg);   /* MODE_BUTTON  */

#endif
