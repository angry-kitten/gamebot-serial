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

// constants
#define ECHO_TIMES 3
#define ECHO_INTERVAL 2
#define LED_DURATION 50

// indexed variables and inline functions
#define Max(a, b) ((a > b) ? (a) : (b))
#define Min(a, b) ((a < b) ? (a) : (b))

// global variables

// single-byte variables
uint8_t _report_echo=0;
uint8_t _script_running=0;

// timers
volatile uint32_t timer_ms = 0;                      // script timer
volatile uint8_t echo_ms = 0;                        // echo counter
volatile uint32_t wait_ms = 0;                       // waiting counter
volatile uint8_t led_ms = 0;                          // transmission LED countdown

// yyy
// Main entry point.
int main(void)
{
    // We'll start by performing hardware and peripheral setup.
    SetupHardware();
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
    
    // Initialize report.
    ResetReport();
    
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
    // increment timer
    timer_ms++;
    // decrement echo counter
    if (echo_ms != 0)
        echo_ms--;
    // decrement waiting counter
    if (wait_ms != 0 && (_report_echo == 0 || wait_ms >1))
        wait_ms--;
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
}

// Fired to indicate that the device is no longer connected to a host.
void EVENT_USB_Device_Disconnect(void)
{
    // We can indicate that our device is not ready (via status LEDs, sound, etc.).
}

// Fired when the host set the current configuration of the USB device after enumeration.
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    // We setup the HID report endpoints.
    ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
    ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);

    // We can read ConfigSuccess to indicate a success or failure at this point.
}

// Process control requests sent to the device from the USB host.
void EVENT_USB_Device_ControlRequest(void)
{
    // We can handle two control requests: a GetReport and a SetReport.

    // Not used here, it looks like we don't receive control request from the Switch.
}
USB_JoystickReport_Input_t next_report;

// Process and deliver data from IN and OUT endpoints.
void HID_Task(void)
{
    // If the device isn't connected and properly configured, we can't do anything here.
    if (USB_DeviceState != DEVICE_STATE_Configured)
        return;

    // [Optimized] We don't need to receive data at all.
    if (true)
    {
        // We'll start with the OUT endpoint.
        Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);
        // We'll check to see if we received something on the OUT endpoint.
        if (Endpoint_IsOUTReceived())
        {
            // If we did, and the packet has data, we'll react to it.
            if (false && Endpoint_IsReadWriteAllowed())
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
        }
    }
    
    // [Optimized] Only send data when changed.
    if (echo_ms == 0)
    {
        // We'll then move on to the IN endpoint.
        Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);
        // We first check to see if the host is ready to accept data.
        if (Endpoint_IsINReady())
        {
            // Once populated, we can output this data to the host. We do this by first writing the data to the control stream.
            if(Endpoint_Write_Stream_LE(&next_report, sizeof(next_report), NULL) == ENDPOINT_RWSTREAM_NoError)
            {
                // We then send an IN packet on this endpoint.
                Endpoint_ClearIN();
                // decrement echo counter
                if (!_script_running || _report_echo > 1 || wait_ms < 2)
                {
                    _report_echo = Max(0,_report_echo--);
                }
                // set interval
                echo_ms = ECHO_INTERVAL;
            }
        }
    }
}

// Reset report to default.
void ResetReport(void)
{
    memset(&next_report, 0, sizeof(USB_JoystickReport_Input_t));
    next_report.LX = STICK_CENTER;
    next_report.LY = STICK_CENTER;
    next_report.RX = STICK_CENTER;
    next_report.RY = STICK_CENTER;
    next_report.HAT = HAT_CENTER;
    _report_echo = ECHO_TIMES;
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

#if 0
        // echo the byte to the output ring
        if( '\n' == byte || '\r' == byte )
        {
            SerialRingAdd(&sro,(uint8_t)'\r');
            SerialRingAdd(&sro,(uint8_t)'\n');
        }
        else
        {
            SerialRingAdd(&sro,(uint8_t)byte);
        }
#endif

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

void Serial_Send(const char DataByte)
{
    Serial_SendByte(DataByte);
    BlinkLED();
}
