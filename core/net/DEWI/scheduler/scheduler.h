/*
 * Copyright (c) 2015, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *         Scheduler header file
 *
 * \author Conrad Dandelski <conrad.dandelski@mycit.ie>
 */
#ifndef DEWI_NIMBUS_CONTIKI_CORE_NET_DEWI_SCHEDULER_SCHEDULER_H_
#define DEWI_NIMBUS_CONTIKI_CORE_NET_DEWI_SCHEDULER_SCHEDULER_H_

#include "contiki.h"
//#include "cpu.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "dev/serial-line.h"
//#include "dev/sys-ctrl.h"
#include "net/netstack.h"
#include "net/rime/broadcast.h"
#include "net/mac/tsch/tsch-schedule.h"

#include "net/DEWI/common/type_defs.h"
#include "net/DEWI/common/packet_types.h"
#include "net/DEWI/cider/cider.h"
#include "net/DEWI/rll/rll.h"
#include "net/DEWI/colouring/colouring.h"


uint16_t setSchedule();
void setRLLSchedule();
void clearSchedule();
void setActiveSchedule(uint8_t schedule);
uint8_t getActiveSchedule();

void setCoord(uint8_t isCoordinator);
uint8_t getCoord();

uint8_t initScheduler();

void scheduler_reset();

#endif /* DEWI_NIMBUS_CONTIKI_CORE_NET_DEWI_SCHEDULER_SCHEDULER_H_ */
