/*
 * dewiDemo.c
 *
 *  Created on: Oct 17, 2016
 *      Author: root
 */

#include "contiki.h"
#include "cpu.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/adc-zoul.h"
#include "dev/watchdog.h"
#include "dev/zoul-sensors.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "net/rime/rime.h"
#include "sys/stimer.h"
#include "net/rime/broadcast.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-schedule.h"

#include "net/DEWI/scheduler/scheduler.h"
#include "net/DEWI/rll/rll.h"
#include "net/DEWI/cider/cider.h"
#include "net/DEWI/neighTable/neighTable.h"

#include "i2c.h"
#include "project-conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#if APP_LOG_LEVEL >= 1
#define DEBUG DEBUG_PRINT
#else /* TSCH_LOG_LEVEL */
#define DEBUG DEBUG_NONE
#endif /* TSCH_LOG_LEVEL */
#include "net/net-debug.h"

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"		// to convert byte to binary
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0)

// LED colors
#define LED_RED 0b10000000
#define LED_GREEN 0b01100000
#define LED_BLUE 0b01000000

#define LED_BRIGHTNESS 0b00100000
#define BUTTON_PRESS_EVENT_INTERVAL (CLOCK_SECOND)

#define NUM_ADDR_ENTRIES 100		//number of child nodes that a master node can have
#define MAX_RESULTS_ENTRIES 200
static struct etimer button_press_reset;
uint8_t button_press_counter = 0;
uint8_t lastBRIGHTNESS = 0b00000001;
uint16_t seqNo = 1;
uint16_t txPackets = 0;
uint16_t rxPackets = 0;
uint8_t lock = 1;
uint16_t ackAddress;
uint8_t ackType;
uint8_t LQIrx = 0,RSSIrx = 0,TxPowerrx, TXPowerNegativ = 0;

linkaddr_t master_addr1; //parent or master address
char waitForTopologyUpdate;  //is 1 if topology information is ongoing, else 0
char isGateway; // is 1 if this node is a gateway, else 0
struct resultCounter
{ //structure for each child node that the master or parent node has
	/* The ->next pointer is needed since we are placing these on a
	 Contiki list. */
	struct resultCounter *next;
	uint8_t timeslot;
	uint16_t counter;
};
MEMB(result_memb, struct resultCounter, MAX_RESULTS_ENTRIES);
LIST(result_list);

uint8_t tempresult[50] = { 0 };
uint8_t tempResultCounter = 0;
const struct etimer processResults_timer, topologyReply_timer, resultReply_timer, ackTimer;
uint8_t addResult(struct resultCounter *res);
void getResult(uint8_t timeslot, struct resultCounter *res);
uint8_t getAllResults(uint8_t timeslot[], uint16_t values[]);
void sendACK(linkaddr_t addr);
PROCESS(dewiDemo, "DEWI Demonstrator, using CIDER and RLL");	//main process
AUTOSTART_PROCESSES(&dewiDemo);

// function to handle incoming sensor events
// input: the data object associated with the event
void handleSensorsEvent(process_data_t data)
{
	PROCESS_CONTEXT_BEGIN(&dewiDemo)
		;
		if (data == &button_sensor)
		{ // receiving user button event

			if (button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) == BUTTON_SENSOR_PRESSED_LEVEL)
			{

				button_press_counter = button_press_counter + 1;
				etimer_set(&button_press_reset, CLOCK_SECOND * 0.5);
				if (button_press_counter == 10)
				{
					CIDER_start();
				}
				if (button_press_counter == 3)
				{
					printNodeStatusForced();
				}
				if (button_press_counter == 2)
				{

					getMinMaxAVGRSSI();

				}

			}

		}
		PROCESS_CONTEXT_END(&dewiDemo);

}

// function to handle incoming data on serial port
// input: the data object associated with the event
void handleSerialInput(process_data_t data)
{
	char* ch_data, *ptr;
	ch_data = (char*) data;
	char t1[2];
	long i_data;
	uint32_t R, G, B;
	if(LQIrx == 1)
	{
		uint16_t temp = (uint16_t) strtol(ch_data, NULL, 16);
		setLQIRadius(temp);
		sendLQI();
		LQIrx=0;
		return;
	}
	if(RSSIrx == 1)
	{
		uint16_t temp = (int16_t) strtol(ch_data, NULL, 16);
		setRSSIRadius(temp * -1);
		sendRSSI();
		RSSIrx=0;
		return;
	}
	if(TxPowerrx == 1 && strstr(ch_data, "NEGATIV") == NULL){
		uint16_t temp = (int16_t) strtol(ch_data, NULL, 16);
		radio_value_t result = (radio_value_t) temp;
		if(TXPowerNegativ == 1)
			result = result * -1;

		TXPowerNegativ = 0;
		radio_result_t rv = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, result);

		PRINTF("[APP] Set Tx Power to %d, result: %d\n",result,rv);
		sendTXPower();
		TxPowerrx=0;
		return;
	}
	if (strstr(ch_data, "Experiment") != NULL)
	{

//		if(tsch_queue_packet_count(&tsch_broadcast_address) > 0)
//			tsch_queue_reset();

		PRINTF("[APP]: Experiment received, send message\n");
		struct APP_PACKET packet;
		packet.src = linkaddr_node_addr;
		packet.dst = tsch_broadcast_address;
		packet.timeSend = current_asn;
		packet.seqNo = seqNo++;
		packet.subType = APP_EXPERIMENT;

		lock = 0;
		sendRLLDataMessage(packet, 1);

	}
	else if (strstr(ch_data, "topologyrefresh") != NULL)
	{
		checkQueueStatus();
		PRINTF("[APP]: TopologyRefresh received, send message\n");
		struct APP_PACKET packet;
		packet.src = linkaddr_node_addr;
		packet.dst = tsch_broadcast_address;
		packet.subType = APP_TOPOLOGYREQUEST;

		lock = 1;
		sendRLLDataMessage(packet, 0);
	}
	else if (strstr(ch_data, "LQI") != NULL){

		checkQueueStatus();
		PRINTF("[APP]: LQI request\n");
		LQIrx = 1;
	}
	else if (strstr(ch_data, "RSSI") != NULL){

		checkQueueStatus();
		PRINTF("[APP]: RSSI request\n");
		RSSIrx = 1;
	}
	else if (strstr(ch_data, "TXPOWER") != NULL){

		checkQueueStatus();
		PRINTF("[APP]: TXPOWER request\n");
		TxPowerrx = 1;
	}
	else if (strstr(ch_data, "NEGATIV") != NULL){

		checkQueueStatus();
		PRINTF("[APP]: Negativ TXPOWER incoming\n");
		TXPowerNegativ = 1;
	}
	else if (strstr(ch_data, "STARTCIDER") != NULL){
		checkQueueStatus();
		CIDER_start();
	}
	else if (strstr(ch_data, "0x") != NULL)
	{

		checkQueueStatus();
		uint16_t temp = (uint16_t) strtol(ch_data, NULL, 16);
		if (linkaddr_node_addr.u16 == temp)
		{
			PRINTF("RESULTReplyTxPackets:0x%4x,%d\n", linkaddr_node_addr.u16, txPackets);
			txPackets = 0;

		}
		else if (temp == 0)
		{
			sendACK(tsch_broadcast_address);
		}
		else
		{
			PRINTF("[APP]: APP_RESULTREQUEST received, send message\n");
			struct APP_PACKET packet;
			packet.src = linkaddr_node_addr;
			packet.dst.u16 = temp;
			packet.subType = APP_RESULTREQUEST;

			lock = 1;
			sendRLLDataMessage(packet, 0);
		}
	}

}

// callback to handle application data received from the network
// input: the application packet and its sequence number
void applicationDataCallback(struct APP_PACKET *data)
{

	struct asn_t receivedAt = current_asn;
	uint16_t latency = ASN_DIFF(receivedAt, data->timeSend);
	PRINTF("[APP]: MSG received: Type: %d, from: 0x%4x with seqNo: %d\n", data->subType,
			data->src.u16, data->seqNo);
	if (data->subType == APP_EXPERIMENT)
	{ // received color packet, set LED color and update performance stats
	  //here save data
		if (isGateway == 0)
		{
			PRINTF("[APP]: Data received: Type: %d, from: 0x%4x with seqNo: %d, latency: %d\n",
					data->subType, data->src.u16, data->seqNo, latency * 10);
			tempresult[tempResultCounter] = latency;
			tempResultCounter++;
			rxPackets++;
			PROCESS_CONTEXT_BEGIN(&dewiDemo)
				;
				etimer_stop(&processResults_timer);
				etimer_set(&processResults_timer,
				CLOCK_SECOND);
				PROCESS_CONTEXT_END(&dewiDemo);
		}

	}
	else if (data->subType == APP_TOPOLOGYREQUEST)
	{
		if (getCIDERState() == 5 && isGateway == 0)
		{
			PROCESS_CONTEXT_BEGIN(&dewiDemo)
				;
				etimer_stop(&topologyReply_timer);
				etimer_set(&topologyReply_timer,
				CLOCK_SECOND * getColour() + CLOCK_SECOND * 0.2);
				PROCESS_CONTEXT_END(&dewiDemo);
		}
	}
	else if (data->subType == APP_TOPOLOGYREPLY)
	{
		if (isGateway == 1)
		{

			uint8_t temp;
			for (temp = 0; temp < data->count; temp++)
			{
				PRINTF("TPReply:0x%4x,0x%4x,%d,%d\n", data->src.u16, data->values[temp],
						data->timeslot[0], data->timeslot[1]);
			}
		}
	}
	else if (data->subType == APP_RESULTREQUEST)
	{

		PRINTF("[APP]: APP_RESULTREQUEST received: Type: %d, from: 0x%4x for 0x%4x\n",
				data->subType, data->src.u16, data->dst.u16);
		if (data->dst.u16 == linkaddr_node_addr.u16 && isGateway == 0)
		{
			PROCESS_CONTEXT_BEGIN(&dewiDemo)
				;
				etimer_stop(&resultReply_timer);
				etimer_set(&resultReply_timer,
				CLOCK_SECOND);
				PROCESS_CONTEXT_END(&dewiDemo);

		}
	}

	else if (data->subType == APP_RESULTREPLY)
	{
		if (isGateway == 1)
		{

			uint8_t temp;
			PRINTF("[APP]: dataSets received: %d, RxPackets: %d\n", data->remainingData,
					data->count);
			for (temp = 0; temp < data->temperature; temp++)
			{
				PRINTF("RESULTReplyLatency:0x%4x,%d,%d\n", data->src.u16, data->timeslot[temp],
						data->values[temp]);
			}
			if (data->remainingData == 0)
				PRINTF("RESULTReplyRxPackets:0x%4x,%d\n", data->src.u16, data->count);

			PRINTF("[APP]: send Ack to: 0x%4x \n", data->src.u16); PRINTF("[APP]: APP_RESULTREQUEST received, send message\n");
		}
	}
	else if (data->subType == APP_ACK)
	{

		PRINTF("[APP]: Ack received\n");

		rxPackets = 0;
		clearResults();

	}

}

void packetDeletedFromQueue()
{
	if (lock == 0)
	{
		PRINTF("[APP]: Packet sent\n");
		txPackets++;
	}
}

void tsch_dewi_callback_joining_network(void)
{
	PRINTF("[APP]: joining network\n");
	setCoord(0);
	initScheduler();
	initNeighbourTable();
	clearResults();
	leds_off(LEDS_ALL);
	leds_on(LEDS_GREEN);
}
void tsch_dewi_callback_leaving_network(void)
{
	PRINTF("[APP]: Leaving network\n");
	scheduler_reset();
	neighbourTable_reset();
	CIDER_reset();
	clearResults();
	leds_off(LEDS_ALL);
	leds_on(LEDS_RED);
}
void tsch_dewi_callback_ka(void)
{
	PRINTF("[APP]: Keep Alive sent\n");
	leds_off(LEDS_ALL);
	leds_on(LEDS_YELLOW);
}

void sendACK(linkaddr_t addr)
{
	struct APP_PACKET packet;
	packet.src = linkaddr_node_addr;
	packet.dst = addr;
	packet.subType = APP_ACK;
	lock = 1;
	sendRLLDataMessage(packet, 0);
}

void getResult(uint8_t timeslot, struct resultCounter *res)
{
	struct resultCounter *n;
	n = NULL;
	for (n = list_head(result_list); n != NULL; n = list_item_next(n))
	{

		if (n->timeslot == timeslot)
		{
			break;
		}
	}

	if (n != NULL)
	{
		res->timeslot = n->timeslot;
		res->counter = n->counter;
	}
}
uint8_t addResult(struct resultCounter *res)
{
	struct resultCounter *newRes;
	newRes = NULL;
	uint8_t isNewRes = 1;
	for (newRes = list_head(result_list); newRes != NULL; newRes = list_item_next(newRes))
	{

		/* We break out of the loop if the address of the neighbor matches
		 the address of the neighbor from which we received this
		 broadcast message. */
		if (newRes->timeslot == res->timeslot)
		{

			break;
		}
	}

	if (newRes == NULL)
	{
		newRes = memb_alloc(&result_memb);

		/* If we could not allocate a new neighbor entry, we give up. We
		 could have reused an old neighbor entry, but we do not do this
		 for now. */
		if (newRes == NULL)
		{
			return 0;
		}
	}
	newRes->counter = res->counter;
	newRes->timeslot = res->timeslot;
	list_add(result_list, newRes);
	return isNewRes;
}

uint8_t getAllResults(uint8_t timeslot[], uint16_t values[])
{
	struct resultCounter *newRes;
	uint8_t counter = 0;
	for (newRes = list_head(result_list); newRes != NULL; newRes = list_item_next(newRes))
	{
		PRINTF("getAllResults, timeslot: %d, counter: %d\n", newRes->timeslot, newRes->counter);
		timeslot[counter] = newRes->timeslot;
		values[counter] = newRes->counter;
		counter++;
	}
	return counter;
}

void clearResults()
{
	while (list_head(result_list) != NULL)
	{
		memb_free(&result_memb, list_head(result_list));
		list_remove(result_list, list_head(result_list));
	}
}

PROCESS_THREAD(dewiDemo, ev, data)  // main demonstrator process
{
	PROCESS_BEGIN()
		;
		initNeighbourTable();
		setCoord(0);
		initScheduler();

		//configure buttons
		button_sensor.configure(BUTTON_SENSOR_CONFIG_TYPE_INTERVAL,
		BUTTON_PRESS_EVENT_INTERVAL);

		//initialize serial line
		serial_line_init();

		//initialize process event timer
		static struct etimer et;
		etimer_set(&et, 3 * CLOCK_SECOND); //event timer, every 3 seconds , there is a event

		//don't wait for topology updates initially

		//initialize lists for child addresses and topology information
		//list_init(result_list);
		//memb_init(&result_mem);

		//initially set isGateway to 0. Will be set to one when messages are received on serial port
		isGateway = 0;
		;

		// main loop
		while (1)
		{
			PROCESS_YIELD()
			;
			if (ev == sensors_event)
			{ // receiving sensor or button event
				handleSensorsEvent(data);
			}
			else if (ev == serial_line_event_message)
			{ // receiving data on serial port
				if (isGateway == 0)
				{
					// node realizes that it must be a gateway
					// set isGateway to 1 and reset statistics collection in the network
					isGateway = 1;

				}
				handleSerialInput(data);
			}
			else if (ev == PROCESS_EVENT_TIMER)
			{ // receiving a process event
				if (data == &button_press_reset)
				{
					button_press_counter = 0;
				}
				else if (data == &processResults_timer)
				{
					struct resultCounter Result;
					int i = 0;
					for (i = 0; i < tempResultCounter; i++)
					{
						PRINTF("[APP]: Timeslot: %d\n", tempresult[i]);
						Result.counter = 0;
						Result.timeslot = 0;
						getResult(tempresult[i], &Result);
						Result.timeslot = tempresult[i];
						Result.counter++;
						PRINTF("[APP]: process results, timeslot: %d, counter: %d\n",
								Result.timeslot, Result.counter);
						addResult(&Result);
						tempresult[0];
					}
					tempResultCounter = 0;
					checkQueueStatus();

				}
				else if (data == &topologyReply_timer)
				{

					PRINTF("[APP]: send TopologyReply\n");
					checkQueueStatus();
					struct APP_PACKET packet;
					packet.src = linkaddr_node_addr;
					packet.dst = tsch_broadcast_address;
					packet.subType = APP_TOPOLOGYREPLY;
					packet.timeslot[0] = getTier();
					packet.timeslot[1] = getColour();
					uint8_t numChildren = 0; // number of children
					linkaddr_t children[CONF_MAX_NEIGHBOURS];
					numChildren = getChildAddresses(children);
					PRINTF("got topology request, have %d children\r\n", numChildren);

					if (numChildren < 23)
					{
						//send one packet

						uint8_t temp = 0;
						for (temp = 0; temp < numChildren; temp++)
						{
							packet.values[temp] = children[temp].u16;
						}
						packet.count = numChildren;
						packet.remainingData = 0;
						//packet.
						lock = 1;
						sendRLLDataMessage(packet, 0);
					}
					else
					{
						//send more than one packet
						uint8_t temp = 0, sentPacket = 0, lowerBorder;
						uint8_t numPackets = (uint8_t) ceilf((float) numChildren / (float) 23);
						PRINTF("numPackets: %d,numChildren: %d\n", numPackets, numChildren);
						for (sentPacket = 0; sentPacket < numPackets; sentPacket++)
						{
							uint8_t maxBorder = (sentPacket + 1) * 23;
							if (maxBorder > numChildren)
							{
								maxBorder = numChildren;
								packet.remainingData = 0;
							}
							else
							{
								packet.remainingData = 1;

							}
							temp = 0;

							for (lowerBorder = 0 + sentPacket * 23; lowerBorder < maxBorder;
									lowerBorder++)
							{
								packet.values[temp] = children[lowerBorder].u16;
								temp++;
							}

							packet.count = temp;
							lock = 1;
							sendRLLDataMessage(packet, 0);
						}

					}

				}
				else if (data == &resultReply_timer)
				{
					checkQueueStatus();
					uint16_t values[MAX_RESULTS_ENTRIES] = { 0 };
					uint8_t timeslot[MAX_RESULTS_ENTRIES] = { 0 };
					uint8_t numResults = getAllResults(timeslot, values);
					struct APP_PACKET packet;
					packet.src = linkaddr_node_addr;
					packet.dst = tsch_broadcast_address;
					packet.subType = APP_RESULTREPLY;
					PRINTF("[APP]: send APP_RESULTREPLY with %d results\n", numResults);
					if (numResults <= 23)
					{
						//send one packet

						uint8_t temp = 0;
						packet.remainingData = 0;
						packet.temperature = numResults;
						for (temp = 0; temp < numResults; temp++)
						{
							PRINTF("[APP]: Add Result to packet; timeslot: %d, value: %d\n",
									timeslot[temp], values[temp]);
							packet.values[temp] = values[temp];
							packet.timeslot[temp] = timeslot[temp];
						}

						PRINTF("[APP]: #MSG received %d\n", rxPackets);
						packet.count = rxPackets;
						lock = 1;
						sendRLLDataMessage(packet, 0);
					}
					else
					{
						//send more than one packet
						uint8_t temp = 0, sentPacket = 0, lowerBorder;
						uint8_t numPackets = (uint8_t) ceilf((float) numResults / (float) 23);
						PRINTF("numPackets: %d,numChildren: %d\n", numPackets, numResults);
						for (sentPacket = 0; sentPacket < numPackets; sentPacket++)
						{
							uint8_t maxBorder = (sentPacket + 1) * 23;
							if (maxBorder > numResults)
							{
								maxBorder = numResults;
								packet.remainingData = 0;
							}
							else
							{
								packet.remainingData = 1;

							}
							temp = 0;

							for (lowerBorder = 0 + sentPacket * 23; lowerBorder < maxBorder;
									lowerBorder++)
							{
								PRINTF("[APP]: Add Result to packet; timeslot: %d, value: %d\n",
										timeslot[temp], values[temp]);
								packet.values[temp] = values[temp];
								packet.timeslot[temp] = timeslot[temp];
								temp++;
							}

							packet.temperature = temp;
							packet.count = rxPackets;
							lock = 1;
							sendRLLDataMessage(packet, 0);
						}

					}

				}
				else if (data == &ackTimer)
				{

					if (ackType == APP_RESULTREPLY)
					{
						checkQueueStatus();
						uint16_t values[MAX_RESULTS_ENTRIES] = { 0 };
						uint8_t timeslot[MAX_RESULTS_ENTRIES] = { 0 };
						uint8_t numResults = getAllResults(timeslot, values);
						struct APP_PACKET packet;
						packet.src = linkaddr_node_addr;
						packet.dst = tsch_broadcast_address;
						packet.subType = APP_RESULTREPLY;
						PRINTF("[APP]: send APP_RESULTREPLY with %d results\n", numResults);

						uint8_t temp = 0;
						packet.remainingData = numResults;
						for (temp = 0; temp < numResults; temp++)
						{
							PRINTF("[APP]: Add Result to packet; timeslot: %d, value: %d\n",
									timeslot[temp], values[temp]);
							packet.values[temp] = values[temp];
							packet.timeslot[temp] = timeslot[temp];
						}

						PRINTF("[APP]: #MSG received %d\n", rxPackets);
						packet.count = rxPackets;
						lock = 1;
						sendRLLDataMessage(packet, 0);

						ackType = APP_RESULTREPLY;
						etimer_set(&ackTimer, CLOCK_SECOND * 5);

					}

				}
			}
			else if (ev == button_press_duration_exceeded)
			{
				if (button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_PRESS_DURATION) == 10)
				{
					PRINTF("[APP]: Start Node as Coordinator\n");
					leds_off(LEDS_ALL);
					leds_on(LEDS_ALL);
					setCoord(1);
					initScheduler();
					tsch_set_coordinator(1);

				}

			}
		}
	PROCESS_END();
}
