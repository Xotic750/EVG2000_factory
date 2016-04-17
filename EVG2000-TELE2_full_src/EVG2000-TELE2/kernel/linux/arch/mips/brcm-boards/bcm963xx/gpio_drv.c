/***************************************************************************
***
***    Copyright 2007  Hon Hai Precision Ind. Co. Ltd.
***    All Rights Reserved.
***    No portions of this material shall be reproduced in any form without the
***    written permission of Hon Hai Precision Ind. Co. Ltd.
***
***    All information contained in this document is Hon Hai Precision Ind.  
***    Co. Ltd. company private, proprietary, and trade secret property and 
***    are protected by international intellectual property laws and treaties.
***
****************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include "gpio_drv.h"

#define DEV_GPIO_DRV_MAJOR_NUM      124

#define _DEBUG

extern int lan_led_state;
extern int wan_led_state;
extern int usb_led_state;

extern int voip_led_state;   
extern int voip_led_state_bs;
extern int voip_led_state_bf;

extern int wps_led_state;
extern char lan_if_name[4][32];
extern char wan_if_name[8][32];

static int
gpio_drv_open(struct inode *inode, struct file * file)
{
    //MOD_INC_USE_COUNT;
    return 0;
}

static int
gpio_drv_release(struct inode *inode, struct file * file)
{
    //MOD_DEC_USE_COUNT;
    return 0;
}

static int
gpio_drv_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    unsigned long index;
    
    switch (cmd)
    {
        case IOCTL_LAN_LED_STATE:
            lan_led_state = (int)arg;
//#ifdef _DEBUG
            printk("%s: lan_led_state = 0x%x\n", __FUNCTION__, lan_led_state);
//#endif
            break;
            
        case IOCTL_USB_LED_STATE:
            usb_led_state = (int)arg;
//#ifdef _DEBUG
            printk("%s: usb_led_state = 0x%x\n", __FUNCTION__, usb_led_state);
//#endif
            break;

        case IOCTL_WPS_LED_STATE:
        
            if ( (int)arg == 10 )
            {
                return wps_led_state; // get WPS state
            }
        
            wps_led_state = (int)arg;
//#ifdef _DEBUG
            printk("%s: wps_led_state = 0x%x\n", __FUNCTION__, wps_led_state);
//#endif
            break;

        case IOCTL_VOIP_LED_OFF:
            voip_led_state = voip_led_state & (~(1 << arg));
            voip_led_state_bs = voip_led_state_bs & (~(1 << arg));
            voip_led_state_bf = voip_led_state_bf & (~(1 << arg));
            break;

        case IOCTL_VOIP_LED_ON:
            voip_led_state = voip_led_state | (1 << arg);
            voip_led_state_bs = voip_led_state_bs & (~(1 << arg));
            voip_led_state_bf = voip_led_state_bf & (~(1 << arg));
//#ifdef _DEBUG
            printk("%s: voip_led_state = 0x%x\n", __FUNCTION__, voip_led_state);
//#endif
            break;
            
        case IOCTL_VOIP_LED_BS:
            voip_led_state = voip_led_state & (~(1 << arg));
            voip_led_state_bs = voip_led_state_bs | (1 << arg);
            voip_led_state_bf = voip_led_state_bf & (~(1 << arg));
            break;
            
        case IOCTL_VOIP_LED_BF:
            voip_led_state = voip_led_state & (~(1 << arg));
            voip_led_state_bs = voip_led_state_bs & (~(1 << arg));
            voip_led_state_bf = voip_led_state_bf | (1 << arg);
            break;
            
        case IOCTL_WAN_LED_STATE:
            wan_led_state = (int)arg;
#ifdef _DEBUG
            //printk("%s: wan_led_state = 0x%x\n", __FUNCTION__, wan_led_state);
#endif
            break;
            
        case IOCTL_LAN_VLAN_ID:
            index = arg >> 16;
            sprintf(lan_if_name[index], "vlan%d", arg & 0x0000ffff);
#ifdef _DEBUG            
            printk("%s: lan_if_name[%d] = %s\n", __FUNCTION__, index, lan_if_name[index]);
#endif
            break;
        case IOCTL_WAN_VLAN_ID:
            index = arg >> 16;
            sprintf(wan_if_name[index], "eth%d", arg & 0x0000ffff);
#ifdef _DEBUG            
            printk("%s: wan_if_name[%d] = %s\n", __FUNCTION__, index, wan_if_name[index]);
#endif 
            break;
       default:
            break;
    }

    return 0;

}
static struct file_operations gpio_drv_fops = {
    owner:      THIS_MODULE,
    open:       gpio_drv_open,
    release:    gpio_drv_release,
    ioctl:      gpio_drv_ioctl,
    };

static int __init
gpio_drv_init(void)
{
    int ret_val;
        
    if ((ret_val = register_chrdev(DEV_GPIO_DRV_MAJOR_NUM, DEV_GPIO_DRV, &gpio_drv_fops)) < 0)
    {
        printk("Failed to register dev '%s'\n", DEV_GPIO_DRV);
        return ret_val;
    }

    return 0;
}

static void __exit
gpio_drv_exit(void)
{
    int ret_val;
    
    ret_val = unregister_chrdev(DEV_GPIO_DRV_MAJOR_NUM, DEV_GPIO_DRV);
    if (ret_val < 0)
    {
        printk("Failed to unregister dev '%s'\n", DEV_GPIO_DRV);
    }
}

module_init(gpio_drv_init);
module_exit(gpio_drv_exit);
