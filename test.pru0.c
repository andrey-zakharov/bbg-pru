// clpru -fe /tmp/cloud9-examples/ws281x.pru0.o ws281x.pru0.asm

#include <stdint.h>
#include <pru_cfg.h>
#include "resource_table_0.h"
#include "prugpio.h"
#include <pru_rpmsg.h>
#include <pru_intc.h>
#include <sys_mailbox.h>
/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK	4

/* The PRU-ICSS system events used for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 * virt and kick interrupts
 */
#define TO_ARM_HOST			16
#define FROM_ARM_HOST		17

/*
 * Using the name 'rpmsg-pru' will probe the rpmsg_pru driver found
 * at linux-x.y.z/drivers/rpmsg/rpmsg_pru.c
 */
#define CHAN_NAME			"rpmsg-pru"
#define CHAN_DESC			"Channel 30"
#define CHAN_PORT			30

#define LEDS_NUM 2
// RES low voltage time Above 50µs
// µs = 1_000_000 s
// ns = 1000 µs
#define FREQ            200_000_000 // Hz
#define CYCLE_TIME      5 // specs 5 nanosec
// find you specs, mine
// https://www.bestlightingbuy.com/pdf/UCS1903%20datasheet.pdf
// http://www.packever.com/pdf/2016-8-24-11-1-30.pdf
/*
 Low-speed mode time
Name Description Typ. value Allowable error
T0H code 0, high level time 0.5µs ±150ns
T1H code 1, high level time 2.0µs ±150ns
T0L code 0, low level time 2.0µs ±150ns
T1L code 1, low level time 0.5µs ±150ns
Note: In the high-speed mode, only half of the above time is needed (the time for code RESET is
not changed).
 RESET code longer than 24µs 
*/
#define T0H	    (300 / CYCLE_TIME)
#define T0L     (900 / CYCLE_TIME)

#define	T1H		(900 / CYCLE_TIME)
#define T1L     (300 / CYCLE_TIME)

#define TRESET		(100000 / CYCLE_TIME)
#define SPEED 5000 // msecs on 200MHz
#define UPDATE_EACH_N_FRAMES 100000 //(SPEED* / (TRESET+(24*(T0H+T0L)))	// Time to wait between updates

volatile register uint32_t __R30;
volatile register uint32_t __R31;
extern inline void write24(const uint32_t v);

// extern void draw();
#pragma FUNC_ALWAYS_INLINE(writeOneTo)
static inline void writeOneTo(const uint32_t gpio) {
	__R30 |= gpio;		// Set the GPIO pin to 1
	__delay_cycles(T1H-1);
	__R30 &= ~gpio;		// Clear the GPIO pin
	__delay_cycles(T1L-2);
}
#pragma FUNC_ALWAYS_INLINE(writeZeroTo)
static inline void writeZeroTo(const uint32_t gpio) {
    __R30 |= gpio;		// Set the GPIO pin to 1
    __delay_cycles(T0H-1);
    __R30 &= ~gpio;		// Clear the GPIO pin
    __delay_cycles(T0L-2);
}

void main(void)
{
	struct pru_rpmsg_transport transport;
	uint16_t src, dst, len;
	int i, k, j;
	volatile uint8_t *status;
	// draw();
	// return;
	
	uint32_t background = 0;
	uint32_t foreground = 0x000000;

	/* Clear SYSCFG[STANDBY_INIT] to enable OCP master port */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	//status = &resourceTable.rpmsg_vdev.status;
	//while( !(*status & VIRTIO_CONFIG_S_DRIVER_OK) );

	//pru_virtqueue_init(&transport.virtqueue0, &resourceTable.rpmsg_vring0, TO_ARM_HOST, FROM_ARM_HOST);

	//while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS);

	// Select which pins to output to.  These are all on pru1_1
	uint32_t gpio = P9_29;
	
	uint32_t color[LEDS_NUM];	// green, red, blue
	uint32_t frame;
	int oldk = 0;
	// Set everything to background
	for (i=0; i<LEDS_NUM; i++) {
		color[i] = background;
	}
	
	frame = 0;
	while (1) {
		/* Check bit 30 of register R31 to see if the ARM has kicked us */
		// if (__R31 & HOST_INT) {
		// 	/* Clear the event status */
		// 	CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
		// 	/* Receive all available messages, multiple messages can be sent per kick */
		// 	while (pru_rpmsg_receive(&transport, &src, &dst, payload, &len) == PRU_RPMSG_SUCCESS) {
		// 		/* Echo the message back to the same address from which we just received */
		// 		pru_rpmsg_send(&transport, dst, src, payload, len);
		// 	}
		// }
		// if ( frame >= UPDATE_EACH_N_FRAMES ) {
		// 	// Move forward one position
		// 	for ( k = 0; k < LEDS_NUM; k++ ) {
		// 		color[oldk] = background;
		// 		color[k]    = foreground;
		// 		oldk = k;
		// 	}
		// 	frame = 0;
		// }

		// Output the string
		for ( j = 0; j < LEDS_NUM; j++ ) {
			write24(color[j]);
			/*for (i = 23; i >= 0; i-- ) {
               	__R30 |= gpio;		// Set the GPIO pin to 1
				__delay_cycles(T0H-1);
				__R30 &= ~gpio;		// Clear the GPIO pin
				__delay_cycles(T0L-2);
			}*/
			//__R30 |= gpio;		// Set the GPIO pin to 1
			//__delay_cycles(60000);
		    // color 24-GRB encode
		    /* DOES NOT WORK:
		     Optimizer terminated abnormally*/
	     	//#pragma UNROLL(1)
			/*for (i = 23; i >= 0; i-- ) {
				if (color[j] & (1<<i)) {
                   	__R30 |= gpio;		// Set the GPIO pin to 1
					__delay_cycles(T1H-7);
					__R30 &= ~gpio;		// Clear the GPIO pin
					__delay_cycles(T1L-8);
				} else {
                 	__R30 |= gpio;		// Set the GPIO pin to 1
					__delay_cycles(T0H-6);
					__R30 &= ~gpio;		// Clear the GPIO pin
					__delay_cycles(T0L-7);
				}
			}*/

			/*
			__R30 |= gpio;__delay_cycles(delays[23]);__R30 &= ~gpio;__delay_cycles(2400-delays[23]);
			(color[j] & (1<<23) ? writeOneTo : writeZeroTo)(gpio);
			(color[j] & (1<<22) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<21) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<20) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<19) ? writeOneTo : writeZeroTo)(gpio);							
			(color[j] & (1<<18) ? writeOneTo : writeZeroTo)(gpio);
			(color[j] & (1<<17) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<16) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<15) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<14) ? writeOneTo : writeZeroTo)(gpio);
			(color[j] & (1<<13) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<12) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<11) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<10) ? writeOneTo : writeZeroTo)(gpio);							
			(color[j] & (1<<9) ? writeOneTo : writeZeroTo)(gpio);
			(color[j] & (1<<8) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<7) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<6) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<5) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<4) ? writeOneTo : writeZeroTo)(gpio);
			(color[j] & (1<<3) ? writeOneTo : writeZeroTo)(gpio);				
			(color[j] & (1<<2) ? writeOneTo : writeZeroTo)(gpio);
			(color[j] & (1<<1) ? writeOneTo : writeZeroTo)(gpio);
			(color[j] & 1 ? writeOneTo : writeZeroTo)(gpio);*/
		}
		
						// Send Reset
		__R30 &= ~gpio;	// Clear the GPIO pin
		__delay_cycles(TRESET);
		frame ++;
	}
}

// Sets pinmux
#pragma DATA_SECTION(init_pins, ".init_pins")
#pragma RETAIN(init_pins)
const char init_pins[] =  
	"/sys/devices/platform/ocp/ocp:P9_29_pinmux/state\0pruout\0" \
	"\0\0";
