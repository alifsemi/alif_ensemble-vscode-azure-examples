/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "tx_api.h"
#include "Driver_GPIO.h"
#include "pinconf.h"
#include <stdio.h>
#include "RTE_Components.h"
#include "board.h"

#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */


#define LED_BLINK_THREAD_STACK_SIZE     (1024)

TX_THREAD                               led_thread;
TX_EVENT_FLAGS_GROUP                    event_flags_button;

#define BUTTON_EVENT                    0x01

/* GPIO port used for button */
extern  ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO15_PORT);
ARM_DRIVER_GPIO *gpioDrv15 = &ARM_Driver_GPIO_(GPIO15_PORT);

static void button_callback(uint32_t event)
{
    if (event == ARM_GPIO_IRQ_EVENT_EXTERNAL) {
        tx_event_flags_set(&event_flags_button, BUTTON_EVENT, TX_OR);
    }
}

/**
  \fn         void led_blink_app(ULONG thread_input)
  \brief      LED blinky function
  \param[in]  thread_input : thread input
  \return     none
*/
void led_blink_app (ULONG thread_input)
{
    printf("led blink demo application for ThreadX started\n\n");

    BOARD_BUTTON2_Init(&button_callback);
    BOARD_BUTTON2_Control(BOARD_BUTTON_ENABLE_INTERRUPT);

    /* Set LED ON and start waiting for button */
    BOARD_LED1_Control(BOARD_LED_STATE_HIGH);

    while (1)
    {
        UINT status;
        ULONG events_button = 0;
        UINT button_state = 0;

        /* wait for button press event */
        status = tx_event_flags_get(&event_flags_button, BUTTON_EVENT, TX_OR_CLEAR, &events_button, TX_WAIT_FOREVER);
        /* Toggle LED */
        BOARD_LED1_Control(BOARD_LED_STATE_TOGGLE);
    }
}

/* Define main entry point.  */
int main ()
{
    BOARD_Pinmux_Init();

    #if defined(RTE_Compiler_IO_STDOUT_User)
    int32_t ret;
    ret = stdout_init();
    if(ret != ARM_DRIVER_OK)
    {
        while(1)
        {
        }
    }
    #endif

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}

/* Define what the initial system looks like.  */
void tx_application_define (void *first_unused_memory)
{
    UINT ret;

    /* Create the event flags group used by button press */
    ret = tx_event_flags_create(&event_flags_button, "event flags UART");
    if (ret != TX_SUCCESS)
    {
        printf("Could not create event flags\n");
        return;
    }

    /* Create the main thread.  */
    ret = tx_thread_create (&led_thread, "LED BLINK DEMO", led_blink_app, 0,
            first_unused_memory, LED_BLINK_THREAD_STACK_SIZE, 1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);

    if (ret != TX_SUCCESS) {
        printf("failed to create led blink demo thread\r\n");
    }
}
