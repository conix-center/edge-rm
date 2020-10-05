/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */
#include <logging/log.h>
LOG_MODULE_REGISTER(net_echo_client_sample, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <errno.h>
#include <stdio.h>

#include <net/socket.h>
#include <net/tls_credentials.h>

#include <net/net_mgmt.h>
#include <net/net_event.h>
#include <net/net_conn_mgr.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include <net/openthread.h>
#include <openthread/cli.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/link_raw.h>
#include <openthread/ncp.h>
#include <openthread/message.h>
#include <openthread/platform/diag.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <openthread/dataset.h>
#include <openthread/joiner.h>
#include <openthread-system.h>
#include <openthread-config-generic.h>
//#include "openthread/mqttsn.h"

#include "bh_platform.h"
#include "bh_assert.h"
#include "bh_log.h"
#include "test_wasm.h"

#include "agent_library.h"

static void on_thread_state_changed(uint32_t flags, void *context)
{
	struct openthread_context *ot_context =(struct openthread_context*) context;

	if (flags & OT_CHANGED_THREAD_ROLE) {
		switch (otThreadGetDeviceRole(ot_context->instance)) {
		case OT_DEVICE_ROLE_CHILD:
                     NET_INFO("ID: 2\n");
		     // Start the agent code
		     agent_start(60);
                     break;
		case OT_DEVICE_ROLE_ROUTER:
                     NET_INFO("ID: 3\n");
		     //run_wasm_module();
                     break;
		case OT_DEVICE_ROLE_LEADER:
		     NET_INFO("ID: 4\n");
                     break;
		case OT_DEVICE_ROLE_DISABLED:
		case OT_DEVICE_ROLE_DETACHED:
                     NET_INFO("ID: 1\n");
                     break;
		default:
	             NET_INFO("ID: Unknown\n");
                     break;
		}
	}
}

void main(void)
{
    // Setup the openthread state change callback to log changes in the network info
    //openthread_set_state_changed_cb(on_thread_state_changed);

    // Start openthread
    //openthread_start(openthread_get_default_context());

    // Initialize the  agent code
    agent_init("test.com");
    agent_start(60);
}
