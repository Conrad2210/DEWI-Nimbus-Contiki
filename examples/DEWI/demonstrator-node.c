
#include "demonstrator.h"


/*---------------------------------------------------------------------------*/


#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0)

#define LOOP_PERIOD         8
#define LOOP_INTERVAL       (CLOCK_SECOND * LOOP_PERIOD)
#define SCHEDULE_INTERVAL       (CLOCK_SECOND * 5)
#define LEDS_OFF_HYSTERISIS ((RTIMER_SECOND * LOOP_PERIOD) >> 1)
#define LEDS_PERIODIC       LEDS_BLUE
#define LEDS_BUTTON         LEDS_RED
#define LEDS_SERIAL_IN      LEDS_GREEN
#define LEDS_REBOOT         LEDS_ALL
#define LEDS_RF_RX          (LEDS_ALL)
#define BUTTON_PRESS_EVENT_INTERVAL (CLOCK_SECOND)

static struct etimer randomColorTimer;
#define TIMER       (CLOCK_SECOND)

#define LED_RED 0b10000000
#define LED_GREEN 0b01100000
#define LED_BLUE 0b01000000
#define LED_BRIGHTNESS 0b00100000
int counter = 0;
uint8_t lastBRIGHTNESS = 0b00000001;

void tsch_dewi_callback_joining_network(void);
void tsch_dewi_callback_leaving_network(void);
/*---------------------------------------------------------------------------*/

//static struct rtimer rt;
/*---------------------------------------------------------------------------*/

PROCESS(dewi_demo_process, "DEWI DEMO PROCESS");
AUTOSTART_PROCESSES(&dewi_demo_process);
/*---------------------------------------------------------------------------*/

static void app_netflood_packet_received(struct broadcast_conn *c, const linkaddr_t *from)
{
	struct APP_PACKET *temp = packetbuf_dataptr();
	printf("[APP]: Received APP Packet %u bytes from %u:%u: '0x%04x'\n", packetbuf_datalen(), from->u8[0], from->u8[1], *(uint16_t *) packetbuf_dataptr());
	if(temp->subType == APP_RESET)
	{
		tsch_dewi_callback_leaving_network();
		tsch_dewi_callback_joining_network();
	}
}

static const struct netflood_callbacks app_netflood_rx = { app_netflood_packet_received };
static struct netflood_conn app_netflood;


PROCESS_THREAD(dewi_demo_process, ev, data)
{
PROCESS_BEGIN()
	;


	netflood_open(&app_netflood, CLOCK_SECOND, NETFLOOD_CHANNEL_APP, &app_netflood_rx);

	radio_result_t rv = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, -24);
	printf("DEWI Application\n");
	while (1)
		{

			PROCESS_YIELD()
			;


		}


PROCESS_END();
}

void init_coordinator(void)
{

}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */

void uip_ds6_link_neighbor_callback(int status, int numtx)
{

}

void uip_debug_lladdr_print()
{
}


void tsch_dewi_callback_joining_network(void)
{
	initNeighbourTable();
	printf("[APP]: joining network\n");
	setCoord(0);
	initScheduler();
	leds_off(LEDS_ALL);
	leds_on(LEDS_GREEN);
}
void tsch_dewi_callback_leaving_network(void){
	printf("[APP]: Leaving network\n");
	scheduler_reset();
	neighbourTable_reset();
	CIDER_reset();
	leds_off(LEDS_ALL);
	leds_on(LEDS_RED);
}

void tsch_dewi_callback_ka(void){
	printf("[APP]: Keep Alive sent\n");
	leds_off(LEDS_ALL);
	leds_on(LEDS_YELLOW);
}