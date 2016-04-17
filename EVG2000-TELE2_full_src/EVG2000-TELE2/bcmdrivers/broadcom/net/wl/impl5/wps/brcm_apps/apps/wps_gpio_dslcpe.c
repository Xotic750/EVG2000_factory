/*
 * WPS GPIO functions
 *
 * Copyright 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <cms_boardioctl.h>
//#include <portability.h>
#include <bcmnvram.h>
#include <wps_gpio.h>
//#include <wps_wl.h>
#include "Wsc_dslcpe_led.h"
#include "board.h"

#include <cms_boardioctl.h>
#include "cms.h"
//#include "cms_log.h"
#include "cms_eid.h"
//#include "cms_util.h"
#include "cms_core.h"
#include "cms_msg.h"


static struct timespec led_change_time;
static int board_fp = 0;

void wps_user_reset_wlan(void)
{
    
    printf("--- wps_user_reset_wlan --- \n");
    sleep(1); /* wait for CGI ... */
    system("killall -SIGUSR2 wlanconfigd");
    
#if 0    /* foxconn removed , 05/01/2009 */
   static void *msgHandle = NULL;
   static char buf[sizeof(CmsMsgHeader) + 32]={0};
   CmsMsgHeader *msg=(CmsMsgHeader *) buf;

   char *ptr;
   int unit, subunit;
   CmsRet ret = CMSRET_INTERNAL_ERROR;

   ptr = wps_get_conf("wl_unit");
   unit = 0;

   if (ptr)
      get_ifname_unit(ptr, &unit, &subunit);

   if (unit < 0)
      unit = 0;


   if ((ret = cmsMsg_init(EID_WLWPS, &msgHandle)) != CMSRET_SUCCESS)
   {
      printf("could not initialize msg, ret=%d", ret);
      return;
   }

   sprintf((char *)(msg + 1), "Modify:%d", unit + 1);

   msg->dataLength = 32;
   msg->type = CMS_MSG_WLAN_CHANGED;
   msg->src = EID_WLWPS;
   msg->dst = EID_WLMNGR;
   msg->flags_event = 1;
   msg->flags_request = 0;

   if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
      printf("could not send BOUNDIFNAME_CHANGED msg to ssk, ret=%d", ret);
   else
      printf("message CMS_MSG_WLAN_CHANGED sent successfully");

   cmsMsg_cleanup(&msgHandle);
#endif
   return;
 }

int wps_gpio_btn_init()
{
    board_fp = open("/dev/brcmboard", O_RDWR);

    if (board_fp <= 0)
    {
        printf("Open /dev/brcmboard failed!\n");
        return -1;
    }

    memset(&led_change_time, 0, sizeof(led_change_time));
    return 0;
}

void wps_gpio_btn_cleanup()
{
    if (board_fp > 0)
        close(board_fp);
    return;
}

/* Called from wps_monitor only */
wps_btnpress_t wps_gpio_btn_pressed()
{
    #define SES_EVENTS 0x00000001

    int trigger = SES_EVENTS;
    int poll_ret  = 0;
    int timeout = 0;

    wps_btnpress_t btn_event = WPS_NO_BTNPRESS;
    struct pollfd gpiofd          = {0};
    BOARD_IOCTL_PARMS IoctlParms  = {0};

    if (board_fp < 0) 
        goto failure;

    IoctlParms.result = -1;
    IoctlParms.string = (char *)&trigger;
    IoctlParms.strLen = sizeof(trigger);

    if (ioctl(board_fp, BOARD_IOCTL_SET_TRIGGER_EVENT, &IoctlParms) < 0)
        goto failure;

    if(IoctlParms.result < 0)
        goto failure;

    timeout = 0;
    gpiofd.fd = board_fp;
    gpiofd.events |= POLLIN;

    poll_ret = poll(&gpiofd, 1, timeout);

    if (poll_ret < 0)
        goto failure;
   
    if(poll_ret > 0)
    {  
        /* button pressed, check short or long push */
        int count = 0, lbpcount = 0, val = 0, len = 0;
        struct timeval time;
        lbpcount = (WPS_LONG_PRESSTIME * 1000000) / WPS_BTNSAMPLE_PERIOD;

        while ((len = read(board_fp, (char*)&val, sizeof(val))) > 0)
        {
            time.tv_sec = 0;
            time.tv_usec = WPS_BTNSAMPLE_PERIOD;
            select(0, NULL, NULL, NULL, &time);

            if (val & trigger)
                count++;

            if(count >= lbpcount)
                break;
        }

        if (count < lbpcount)
        {
            btn_event = WPS_SHORT_BTNPRESS;
            nvram_set("wps_sta_pin", "00000000");
        }
        else
        {
            btn_event = WPS_LONG_BTNPRESS;
            nvram_set("wps_sta_pin", "00000000");
        }

        /* Physical Button is only used for wl0 */
        nvram_set("wl_unit", "0");
        printf("WPS Button is Pressed!\n");
    }
            
failure :
  return btn_event;
}

/*If WPS LED from 43XX GPIO, enable compiler flag WPS_LED_FROM_43XX*/
//#define WPS_LED_FROM_43XX

static void wscLedSet(int led_action, int led_blink_type, int led_event, int led_status)
{
    BOARD_IOCTL_PARMS IoctlParms = {0};

#ifdef WPS_LED_FROM_43XX    
    /* Control 43xx GPIO LED */

    switch(led_action)
    {
        case WSC_LED_BLINK:
            wlctl_cmd("wlctl led 0 34 0 100 100");
            break;
        case WSC_LED_ON:
            wlctl_cmd("wlctl led 0 33 0");
            break;
        case WSC_LED_OFF:
            wlctl_cmd("wlctl led 0 32 0");
            break;
    }
#else
    /* Control 63xx GPIO LED */

    if (board_fp <= 0)
        return;

    led_action &= WSC_LED_MASK;
    led_action |= ((led_status      << WSC_STATUS_OFFSET) & WSC_STATUS_MASK);
    led_action |= ((led_event       << WSC_EVENT_OFFSET)  & WSC_EVENT_MASK);
    led_action |= ((led_blink_type  << WSC_BLINK_OFFSET)  & WSC_BLINK_MASK);

    IoctlParms.result = -1;
    IoctlParms.string = (char *)&led_action;
    IoctlParms.strLen = sizeof(led_action);
    ioctl(board_fp, BOARD_IOCTL_SET_SES_LED, &IoctlParms);
#endif
    return;
}

/* Called from wps_monitor every 10ms */
void wps_led_blink_timer()
{
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);
 
    if ((now.tv_sec - led_change_time.tv_sec > 300) && (led_change_time.tv_sec != 0))
    {
    	printf("stop LED\n");
        wscLedSet(WSC_LED_OFF, 0, 0, 0);
        led_change_time.tv_sec = 0;
    }

    return;
}

/* Called from wps_monitor */
int wps_led_init(char *etname, char *vlan0ports)
{
    wscLedSet(WSC_LED_OFF, 0, 0, 0);

    /* initial osl gpio led blinking timer */
    wps_osl_led_blink_init(wps_led_blink_timer);
    
    return 0;
}

/* Called from wps_monitor */
int
wps_led_cleanup()
{

    wps_osl_led_blink_cleanup();
	
    wscLedSet(WSC_LED_OFF, 0, 0, 0);
    return 0;
}

/* Called from wps_monitor to change led blinking pattern */
void wps_led_blink(wps_blinktype_t blinktype)
{
    switch ((int)blinktype) 
    {
        case WPS_BLINKTYPE_STOP:
            wscLedSet(WSC_LED_OFF, 0, 0, 0);
            break;

        case WPS_BLINKTYPE_INPROGRESS:
            led_change_time.tv_sec = 0;
            //clock_gettime(CLOCK_REALTIME, &led_change_time);
            wscLedSet(WSC_LED_BLINK, kLedStateUserWpsInProgress , WSC_EVENTS_PROC_WAITING, 0);
            break;

        case WPS_BLINKTYPE_ERROR:
            led_change_time.tv_sec = 0;
            //clock_gettime(CLOCK_REALTIME, &led_change_time);
            wscLedSet(WSC_LED_BLINK, kLedStateUserWpsError, WSC_EVENTS_PROC_FAIL, 0);
            break;

        case WPS_BLINKTYPE_OVERLAP:
            led_change_time.tv_sec = 0;
            //clock_gettime(CLOCK_REALTIME, &led_change_time);
            wscLedSet(WSC_LED_BLINK, kLedStateUserWpsSessionOverLap, WSC_EVENTS_PROC_PBC_OVERLAP, 0);
            break;

        case WPS_BLINKTYPE_SUCCESS:
            clock_gettime(CLOCK_REALTIME, &led_change_time);
            wscLedSet(WSC_LED_ON, kLedStateOn, WSC_EVENTS_PROC_SUCC, 0);
            break;

        default:
            break;
    }
    return;
}

/* End of file */
