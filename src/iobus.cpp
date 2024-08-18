#include <Arduino.h>
#include "iobus.h"

#define EZ80RD  12
#define EZ80WR  10
#define IORDYn  9
#define D7      11

static QueueHandle_t gpio_evt_queue = NULL;

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}

void IOBus::begin ()
{
    log_i ("Assign IOBus pins");
    pinMode (EZ80RD,INPUT);
    pinMode (EZ80WR,INPUT);
    pinMode (IORDYn,OUTPUT);
    pinMode (D7,INPUT);

    digitalWrite (IORDYn,LOW); // HALT cleared
    delay (100);
    digitalWrite (IORDYn,HIGH); // arm waitstate flip flop

    attachInterrupt (digitalPinToInterrupt(EZ80RD),io_ez80_rd,FALLING);
    attachInterrupt (digitalPinToInterrupt(EZ80WR),io_ez80_wr,FALLING);
}
void IOBus::end ()
{
    log_i ("Release IOBus pins");
    pinMode (EZ80WR,INPUT);
    pinMode (EZ80RD,INPUT);
    pinMode (IORDYn,INPUT);
    pinMode (D7,INPUT);
}

static int prev_d7=0;
static int count=0;
static int d7=0;
void IOBus::io_ez80_rd ()
{
    // clear EZ80 OUT status
    count = 0;
    prev_d7 = 0;

    //log_i ("EZ80 RD trigger");
    pinMode (D7,OUTPUT);
    d7 = d7 ^ 0x80;
    digitalWrite (D7,d7);
        
    // clear HALT
    digitalWrite (IORDYn,LOW); // HALT cleared
    digitalWrite (IORDYn,HIGH); // arm waitstate flip flop
}


void IOBus::io_ez80_wr ()
{
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    
    pinMode (D7,INPUT);
    int d7 = digitalRead (D7);
    if ((prev_d7 ^ d7)==0)
        log_i ("Error received: %d and previously: %d at count: %d",d7,prev_d7,count);  
    prev_d7 = d7;
    count++;
    // clear HALT
    digitalWrite (IORDYn,LOW); // HALT cleared
    digitalWrite (IORDYn,HIGH); // arm waitstate flip flop
}