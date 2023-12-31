/*
 *  Copyright (c) 2022, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>

// Zephyr OpenThread integration Library
#include <zephyr/net/openthread.h>


// OpenThread API
#include <openthread/platform/ble.h>
#include <openthread/ble_secure.h>

#include <openthread/coap.h>
#include <openthread/coap_secure.h>

// Mbed TLS
#include <mbedtls/oid.h>

#include "x509_cert_key.h"

LOG_MODULE_REGISTER(tcat_cli_sample, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);

struct openthread_context *myOpenThreadContext;
otInstance *myOpenThreadInstance = NULL;

otTcatVendorInfo myVendorInfo = { .mVendorName = "My Vendor", .mPskdString = "SECRET"};

// BleSecure callback functions

void HandleBleSecureClientConnect(otInstance *aInstance, bool aConnected, bool aBleConnectionOpen, void *aContext)
{
	LOG_INF("TLS Connected: %s, BLE connection open: %s", aConnected ? "YES" : " NO",
		aBleConnectionOpen ? "YES" : " NO");

	/*
	if(!aConnected && aBleConnectionOpen)
	{
		// Connect as client
		otBleSecureConnect(aInstance;
	}
	*/
	
	if (aConnected) 
	{
		char buf[20];
		size_t len;

		len = sizeof(buf) - 1;
		otBleSecureGetPeerSubjectAttributeByOid(aInstance, MBEDTLS_OID_AT_CN,
							sizeof(MBEDTLS_OID_AT_CN) - 1, buf, &len, NULL);

		buf[len] = 0;
		LOG_INF("Peer cert. Common Name:%s", buf);

		len = sizeof(buf);
		otBleSecureGetThreadAttributeFromPeerCertificate(aInstance, 3, buf, &len);
		if (len > 0) 
		{
			LOG_INF("Peer OID 1.3.6.1.4.1.44970.3: %02X%02X%02X%02X%02X (len = %d)",
				buf[0], buf[1], buf[2], buf[3], buf[4], len);
		}

		len = sizeof(buf);
		otBleSecureGetThreadAttributeFromOwnCertificate(aInstance, 3, buf, &len);
		if (len > 0) 
		{
			LOG_INF("Own OID 1.3.6.1.4.1.44970.3: %02X%02X%02X%02X%02X (len = %d)",
				buf[0], buf[1], buf[2], buf[3], buf[4], len);
		}

		LOG_INF("Command class commissioning authorized: %s", otBleSecureIsCommandClassAuthorized(aInstance, OT_TCAT_COMMAND_CLASS_COMMISSIONING) ? "YES" : " NO");
		LOG_INF("Command class extraxtion authorized: %s", otBleSecureIsCommandClassAuthorized(aInstance, OT_TCAT_COMMAND_CLASS_EXTRACTION) ? "YES" : " NO");
		LOG_INF("Command class de-commissioning authorized: %s", otBleSecureIsCommandClassAuthorized(aInstance, OT_TCAT_COMMAND_CLASS_DECOMMISSIONING) ? "YES" : " NO");
		LOG_INF("Command class application authorized: %s", otBleSecureIsCommandClassAuthorized(aInstance, OT_TCAT_COMMAND_CLASS_APPLICATION) ? "YES" : " NO");
	}
}

void HandleBleSecureReceive(otInstance *aInstance, const otMessage *aMessage, int32_t aOffset, otTcatMessageType aTcatMessageType, const char *aServiceName, void *aContext)
{
	uint16_t nLen;
	char buf[100];

	LOG_INF("TLS Data Received len:%d offset:%d", (int)otMessageGetLength(aMessage),
		(int)aOffset);

	//nLen = otMessageGetLength(aMessage) - aOffset;
	nLen = otMessageRead(aMessage, aOffset, buf + 5, sizeof(buf) - 6);
	buf[nLen + 5] = 0;

	LOG_INF("Received:%s", buf + 5);
	memcpy(buf, "RECV:", 5);
	//otBleSecureSend(aInstance, buf, strlen(buf));
	otBleSecureSendApplicationTlv(aInstance, buf, strlen(buf));
	otBleSecureFlush(aInstance);
}

int main(void)
{
	myOpenThreadContext = openthread_get_default_context();
	myOpenThreadInstance = myOpenThreadContext->instance;

#if DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_shell_uart), zephyr_cdc_acm_uart)

	int ret;
	const struct device *dev;
	uint32_t dtr = 0U;

	ret = usb_enable(NULL);
	if (ret != 0) {
			LOG_ERR("Failed to enable USB");
			return 0;
	}

	dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
	if (dev == NULL) {
			LOG_ERR("Failed to find specific UART device");
			return 0;
	}

	LOG_INF("Waiting for host to be ready to communicate");

	/* Data Terminal Ready - check if host is ready to communicate */
	while (!dtr) {
			ret = uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
			if (ret) {
					LOG_ERR("Failed to get Data Terminal Ready line state: %d",
							ret);
					continue;
			}
			k_msleep(100);
	}

	/* Data Carrier Detect Modem - mark connection as established */
	(void)uart_line_ctrl_set(dev, UART_LINE_CTRL_DCD, 1);
	/* Data Set Ready - the NCP SoC is ready to communicate */
	(void)uart_line_ctrl_set(dev, UART_LINE_CTRL_DSR, 1);
#endif

	// TEST TEST TEST  ---->
	otCoapResource res1;
	otCoapResource res2;

	res1.mUriPath = "dali/d";
	res1.mHandler = NULL;
	res1.mNext = NULL;
	res1.mContext = NULL;
	res2.mUriPath = "dali/e";
	res2.mHandler = NULL;
	res2.mNext = NULL;
	res2.mContext = NULL;


	LOG_INF("otCoapStart(): %s", otCoapStart(myOpenThreadInstance, OT_DEFAULT_COAP_PORT) == OT_ERROR_NONE ? "OK" : "FAIL");
	otCoapSecureSetPsk(myOpenThreadInstance, "SECRET", 6, "12345678", 8);
    otCoapSecureSetSslAuthMode(myOpenThreadInstance, false);
	// <---- TEST TEST TEST

	otBleSecureSetCertificate(myOpenThreadInstance, (const uint8_t *)(OT_CLI_TCAT_X509_CERT),
				  sizeof(OT_CLI_TCAT_X509_CERT),
				  (const uint8_t *)(OT_CLI_TCAT_PRIV_KEY),
				  sizeof(OT_CLI_TCAT_PRIV_KEY));

	otBleSecureSetCaCertificateChain(myOpenThreadInstance,
					 (const uint8_t *)(OT_CLI_TCAT_TRUSTED_ROOT_CERTIFICATE),
					 sizeof(OT_CLI_TCAT_TRUSTED_ROOT_CERTIFICATE));

	otBleSecureSetSslAuthMode(myOpenThreadInstance, true);

	//otBleSecureSetPsk(myOpenThreadInstance, "SECRET", 6, "MyDevice", 8);
	otBleSecureStart(myOpenThreadInstance, HandleBleSecureClientConnect, HandleBleSecureReceive,
			 true, NULL);
	otBleSecureTcatStart(myOpenThreadInstance, &myVendorInfo, NULL);

	return 0;
}
