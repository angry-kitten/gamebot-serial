/*
Nintendo Switch Fightstick - Proof-of-

Based on the LUFA library's Low-Level Joystick 
    (C) Dean 
Based on the HORI's Pokken Tournament Pro Pad 
    (C) 

This project implements a modified version of HORI's Pokken Tournament Pro 
USB descriptors to allow for the creation of custom controllers for 
Nintendo Switch. This also works to a limited degree on the PS3.

Since System Update v3.0.0, the Nintendo Switch recognizes the 
Tournament Pro Pad as a Pro Controller. Physical design limitations 
the Pokken Controller from functioning at the same level as the 
Controller. However, by default most of the descriptors are there, with 
exception of Home and Capture. Descriptor modification allows us to 
these buttons for our use.
*/

#include "Joystick.h"
#include "gamebotserial.h"
#include "packetserial.h"
#include "cmdqueue.h"

// constants
#define ECHO_TIMES 3
#define LED_DURATION 50

// global variables
cmdqueue_element_t *gpe=NULL;  // global pointer to element
uint8_t echo_count=0;  // how many times to send the same report

// timers
volatile uint32_t interrupt_count=0;
volatile uint32_t cmd_elapsed_msec=0;
volatile uint8_t led_ms = 0;                          // transmission LED countdown

// yyy
// Main entry point.
int main(void)
{
    // We'll start by performing hardware and peripheral setup.
    SetupHardware();

    // Send a default state report that has no buttons pressed, etc.
    SetDefaultStateReport();

    // We'll then enable global interrupts for our use.
    GlobalInterruptEnable();

    // send a banner to show that serial is working
    Serial_SendString((const char *)"\r\ngamebot-serial "GB_VERSION_STRING"\r\n");

    // Once that's done, we'll enter an infinite loop.
    for (;;)
    {
        // We need to run our task to process and deliver data for our IN and OUT endpoints.
        HID_Task();
        // We also need to run the main USB management task.
        USB_USBTask();
        // Manage data from/to serial port.
        Serial_Task();

        // Process the commands in the queue.
        CMDQueue_Task();
    }
}

// Configures hardware and peripherals, such as the USB peripherals.
void SetupHardware(void)
{
    // We need to disable watchdog if enabled by bootloader/fuses.
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    // We need to disable clock division before initializing the USB hardware.
    clock_prescale_set(clock_div_1);

    // Initialize LEDs.
    LEDs_Init();

    // Initialize timer interrupt
    TIMSK0 |= (1 << TOIE0);
    //enable interrupts
    sei();
    // set prescaler to 64 and start the timer
    TCCR0B |= (1 << CS01) | (1 << CS00);

    // We can then initialize our hardware and peripherals, including the USB stack.

    // The USB stack should be initialized last.
    USB_Init();

    // Initialize serial port.
    Serial_Init(9600, false);
}

ISR (TIMER0_OVF_vect) // timer0 overflow interrupt
{
    // add 6 to the register (our work around)
    TCNT0 += 6;

    interrupt_count++;
    cmd_elapsed_msec++;

    // decrement LED counter
    if (led_ms != 0)
    {
        led_ms--;
        if (led_ms == 0)
            LEDs_TurnOffLEDs(LEDMASK_TX);
    }
}

// Fired to indicate that the device is enumerating.
void EVENT_USB_Device_Connect(void)
{
    // We can indicate that we're enumerating here (via status LEDs, sound, etc.).
    SerialRingAdd(&sro,'e');
}

// Fired to indicate that the device is no longer connected to a host.
void EVENT_USB_Device_Disconnect(void)
{
    // We can indicate that our device is not ready (via status LEDs, sound, etc.).
    SerialRingAdd(&sro,'d');
}

// Fired when the host set the current configuration of the USB device after enumeration.
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    // We setup the HID report endpoints.
    ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
    ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);

    // We can read ConfigSuccess to indicate a success or failure at this point.
    SerialRingAdd(&sro,'c');
}

// Process control requests sent to the device from the USB host.
void EVENT_USB_Device_ControlRequest(void)
{
    // We can handle two control requests: a GetReport and a SetReport.

    // Not used here, it looks like we don't receive control request from the Switch.
}

// Process and deliver data from IN and OUT endpoints.
void HID_Task(void)
{
    // If the device isn't connected and properly configured, we can't do anything here.
    if (USB_DeviceState != DEVICE_STATE_Configured)
        return;

    // We'll start with the OUT endpoint.
    Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);
    // We'll check to see if we received something on the OUT endpoint.
    if (Endpoint_IsOUTReceived())
    {
        // If we did, and the packet has data, we'll react to it.
        if (Endpoint_IsReadWriteAllowed())
        {
            // We'll create a place to store our data received from the host.
            USB_JoystickReport_Output_t JoystickOutputData;
            // We'll then take in that data, setting it up in our storage.
            while(Endpoint_Read_Stream_LE(&JoystickOutputData, sizeof(JoystickOutputData), NULL) != ENDPOINT_RWSTREAM_NoError);
            // At this point, we can react to this data.

            // However, since we're not doing anything with this data, we abandon it.
        }
        // Regardless of whether we reacted to the data, we acknowledge an OUT packet on this endpoint.
        Endpoint_ClearOUT();
SerialRingAdd(&sro,'o');
    }

    // The report to send is at gpe.
    // Send the report echo_count times.
    if ( echo_count > 0 && gpe )
    {
        // We'll then move on to the IN endpoint.
        Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);
        // We first check to see if the host is ready to accept data.
        if (Endpoint_IsINReady())
        {
            // Once populated, we can output this data to the host. We do this by first writing the data to the control stream.
            if(Endpoint_Write_Stream_LE(&(gpe->i), sizeof(gpe->i), NULL) == ENDPOINT_RWSTREAM_NoError)
            {
                // We then send an IN packet on this endpoint.
                Endpoint_ClearIN();
                // decrement echo counter
                echo_count--;
SerialRingAdd(&sro,'i');
            }
        }
    }
}

// Process data from serial port.
// yyy
void Serial_Task(void)
{
    // read all incoming bytes available
    while (true)
    {
        // read next byte
        int16_t byte = Serial_ReceiveByte();
        if (byte < 0)
            break;
        // save the byte in input ring
        SerialRingAdd(&sri,(uint8_t)byte);

        BlinkLED();
    }
    SerialPacketTask();
    SerialOutRingTask(); // yyy
}

void BlinkLED(void)
{
    led_ms = LED_DURATION;
    LEDs_TurnOnLEDs(LEDMASK_TX);
}

void CMDQueueClear_gpe(void)
{
    gpe=NULL;
    SetDefaultStateReport();
}

void CMDQueue_Task(void)
{
    if( gpe )
    {
        // Continue or complete the current command.

#if 0
        if( echo_count > 0 )
        {
            // It still needs to be echoed more. Do nothing.
            return;
        }
#endif

        uint32_t cem=cmd_elapsed_msec;
        if( cem >= gpe->duration_msec )
        {
            // The command is done.
            gpe=NULL;
            // Fall through to the next section.
        }
    }

    if( ! gpe )
    {
        // Start the next command, if any.
        CMDQueuePop(&gpe);

        if( gpe )
        {
            // There was a new command. Start it.
            echo_count=ECHO_TIMES;
            cmd_elapsed_msec=0;
        }
    }
}

void SetDefaultStateReport(void)
{
    cmdqueue_element_t *pe=NULL;
    CMDQueueAdd(&pe);
}
