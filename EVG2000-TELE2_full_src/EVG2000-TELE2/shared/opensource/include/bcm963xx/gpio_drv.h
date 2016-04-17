/***************************************************************************
***
***    Copyright 2008  Hon Hai Precision Ind. Co. Ltd.
***    All Rights Reserved.
***    No portions of this material shall be reproduced in any form without the
***    written permission of Hon Hai Precision Ind. Co. Ltd.
***
***    All information contained in this document is Hon Hai Precision Ind.  
***    Co. Ltd. company private, proprietary, and trade secret property and 
***    are protected by international intellectual property laws and treaties.
***
****************************************************************************/

#ifndef __GPIO_DRV_H__
#define __GPIO_DRV_H__

#define DEV_GPIO_DRV        "gpio_drv"

#define GPIO_IOCTL_NUM   'W'

#define IOCTL_LAN_LED_STATE       _IOWR(GPIO_IOCTL_NUM, 0, int *)
#define IOCTL_USB_LED_STATE       _IOWR(GPIO_IOCTL_NUM, 1, int *)
#define IOCTL_WPS_LED_STATE       _IOWR(GPIO_IOCTL_NUM, 2, int *)
#define IOCTL_VOIP_LED_OFF        _IOWR(GPIO_IOCTL_NUM, 3, int *)
#define IOCTL_VOIP_LED_ON         _IOWR(GPIO_IOCTL_NUM, 4, int *)
#define IOCTL_VOIP_LED_BS         _IOWR(GPIO_IOCTL_NUM, 5, int *)
#define IOCTL_VOIP_LED_BF         _IOWR(GPIO_IOCTL_NUM, 6, int *)
#define IOCTL_VOIP_LED_BN         _IOWR(GPIO_IOCTL_NUM, 7, int *)
#define IOCTL_WAN_LED_STATE       _IOWR(GPIO_IOCTL_NUM, 8, int *)
#define IOCTL_LAN_VLAN_ID         _IOWR(GPIO_IOCTL_NUM, 9, int *)
#define IOCTL_WAN_VLAN_ID         _IOWR(GPIO_IOCTL_NUM, 10, int *)

#endif  /* __GPIO_DRV_H__ */
