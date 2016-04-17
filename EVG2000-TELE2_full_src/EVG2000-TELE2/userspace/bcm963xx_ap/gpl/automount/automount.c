
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include "automount.h"

#include "gpio_drv.h"

#define MNT_DETACH 0x00000002
//#define NTFS_RW 1
#define DEBUG
#define DEBUG_1

extern int errno;

/* Global variables */
static const char partitions[64] = "/proc/partitions";
static const char proc_user_pid[64] = "/proc/proc_user_pid";
usb_dev_t device_array[MAX_DEVICES];

#define MOUNT_PRIMARY

/* Wins add-S 0925-08 */
#if (defined TI_ALICE)
static void sigxcpu_handler(int sig)
{
    signal(SIGXCPU, sigxcpu_handler);
}
#endif /* (TI_ALICE) */
/* Wins add-E 0925-08 */
int main()
{
  int fd = -1;
  char strline[MAX_STR_LEN];
  volatile int indx = -1, ii = 0, ret = 0;
  sigset_t set;
  int pid;
  int signal_var=0;
  pid_t my_pid=0;

  // create - fork 1
  if(fork()) return 0;

  // it separates the son from the father
  chdir("/");
  setsid();
  umask(0);

  // create - fork 2
  pid = fork();

  if(pid)
  {
    return 0;
  }

  memset(device_array, 0, sizeof(device_array));

  /* write the current process pid the proc entry */

  if((fd = open(proc_user_pid,O_RDWR)) == -1)
  {
    printf("Error opening file: %s, Ret: %d\n", proc_user_pid, fd);
    return;
  }

  memset(strline, 0, MAX_STR_LEN);
  my_pid = getpid();
  sprintf(strline,"%d",my_pid);
#ifdef DEBUG
  //printf("Writing PID ::: %d\n",my_pid);
#endif

  ret = write(fd, strline, strlen(strline));

  close(fd);

  signal(SIGUSR1, auto_mount);
  signal(SIGUSR2, auto_umount);
/* Wins add-S 0925-08 */
#if (defined TI_ALICE)
  signal(SIGXCPU, auto_umount);
#endif /* (TI_ALICE) */
/* Wins add-E 0925-08 */

  //empty the signl set
  sigemptyset(&set);

  //add the signal into the set
  sigaddset(&set,SIGUSR1);
  sigaddset(&set,SIGUSR2);
/* Wins add-S 0925-08 */
#if (defined TI_ALICE)
  sigaddset(&set,SIGXCPU);
#endif /* (TI_ALICE) */
/* Wins add-E 0925-08 */

  //Lets Mount devices that are connected already
  auto_mount(0);
  system("/usr/bin/setsmbftpconf");

  // Lets loop around
  while (1)
  {
    if(sigwait(&set,&signal_var) == 0)
    {
      sleep(1);
      if(signal_var == SIGUSR1)
      {
        //mount
        auto_mount(signal_var);

      }
      else if (signal_var == SIGUSR2)
      {
        //umount
        auto_umount(signal_var);
      }
/* Wins add-S 0925-08 */
#if (defined TI_ALICE)
      else if (signal_var == SIGXCPU)
      {
        auto_umount(signal_var);
      }
#endif /* (TI_ALICE) */
/* Wins add-E 0925-08 */
      //usb@1205. zzz, change settings
      system("/usr/bin/setsmbftpconf");

      //add the signal back into the set
      sigaddset(&set,signal_var);

    }
    else
    {
      printf("Error Unknow \n");
    }
  }
  printf("Exiting....");
}

static void auto_mount(int num)
{
  FILE *fptr = NULL;
  char strline[MAX_STR_LEN];
  volatile int indx = -1, ii = 0;

#ifdef DEBUG
  printf("Do Auto mount...\n");
#endif

  sleep(1);
  if((fptr = fopen(partitions,"r"))== NULL)
  {
    printf("Error opening file: %s\n", partitions);
    return;
  }

  mkdir("/var/mnt/", 0755);

  memset(strline, 0, MAX_STR_LEN);

  init_free_slots(device_array);

  while(fgets(strline,MAX_STR_LEN,fptr) != NULL)
  {
    if(strstr(strline, "sd"))
    {
      if((indx = get_free_slot()) != -1)
      {
        parse_tokens(strline, &device_array[indx]);

#ifdef DEBUG_1
        printf("Device Info:%d \n\
            \t\t\tName: %s\
            \t\t\tMajor: %d\n\
            \t\t\tMinor: %d\n\
            \t\t\tBlocks: %d\n",
            indx,
            device_array[indx].name,
            device_array[indx].major,
            device_array[indx].minor,
            device_array[indx].numBlocks);
#endif

      }
    }
    memset(strline, 0, MAX_STR_LEN);
  }

    int mounted = 0;

  for(ii=0;ii<MAX_DEVICES;ii++)
  {
    if (device_array[ii].freeSlot == 0)
    {
#ifndef MOUNT_PRIMARY
        auto_mount_usb_dev(&device_array[ii]);
        /* fiji added start Bob, 2008/09/04 */
        if(device_array[ii].mounted == 1)
        {
            mounted++;
        }
        /* fiji added end Bob, 2008/09/04 */
#else
      if (device_array[ii].primary == 1)
      {
        if (check_is_this_only_partition(ii))
        {
            /* If this is the only partition mount it */
            auto_mount_usb_dev(&device_array[ii]);
            /* fiji added start Bob, 2008/09/04 */
            if(device_array[ii].mounted == 1)
            {
                mounted++;
            }
            /* fiji added end Bob, 2008/09/04 */
        }
        else
        {
          /* If this is not the only partition dont mount it clear it */
          /* This is already done in the check_is_this_only_partition logic */
        }
      }
      else
      {
        auto_mount_usb_dev(&device_array[ii]);
        /* fiji added start Bob, 2008/09/04 */
        if(device_array[ii].mounted == 1)
        {
            mounted++;
        }
        /* fiji added end Bob, 2008/09/04 */
      }
#endif
    }
    else
    {
      /* Fix for hub resets */
      /* unmount already mounted devices & remount it again!*/
      if(device_array[ii].mounted == 1)
        auto_umount_usb_dev(&device_array[ii]);
      auto_mount_usb_dev(&device_array[ii]);
      /* fiji added start Bob, 2008/09/04 */
      if(device_array[ii].mounted == 1)
      {
          mounted++;
      }
      /* fiji added end Bob, 2008/09/04 */
    }
  }
    /* fiji added start Bob, 2008/09/04 */
    /* Fiji modified pling, 2008/10/09 */
    /* TI USB LED is shared with Power LED, so don't change LED status */
#if (!defined TI_ALICE)
    if(mounted > 0)
    {
        int fd;

        fd = open("/dev/gpio_drv", O_RDWR);
        ioctl(fd, IOCTL_USB_LED_STATE, 1);
        close(fd);
    }
#endif
    /* fiji added end Bob, 2008/09/04 */

/* Wins add-S 0925-08 */
#if (defined TI_ALICE)
    if(mounted > 0)
    {
        FILE* fpUsb;
        pid_t my_pid=0;
        int i;

        /* Create the /var/mnt/usbdev1 for Backup/Restore */
        for (i=0; i<MAX_DEVICES; i++)
        {
            /* Get the mounted USB device */
            if (device_array[i].mounted == 1)
            {
                int nLen = 0;
                char nameBuf[32];

                /* Get automount process ID */
                my_pid = getpid();
                /* Remove the redundant "new line" in Name */
                nLen = strlen(device_array[i].name) - 1;
                memset(nameBuf, 0x00, sizeof(nameBuf));
                strncpy(nameBuf, device_array[i].name, nLen);
                /* Save USB device info into file */
                fpUsb = fopen("/var/mnt/usbdev1", "w");
                fprintf(fpUsb, "%d\n", my_pid);
                fprintf(fpUsb, "%s\n", nameBuf);
                fclose(fpUsb);
                break;
            }
        } /* EndFor (i) */
    } /* EndIf (mounted) */
#endif /* (TI_ALICE) */
/* Wins add-E 0925-08 */
}

static void auto_umount(int num)
{
  FILE *fptr = NULL;
  char strline[MAX_STR_LEN];
  int indx = -1, ii = 0, jj = 0;
  usb_dev_t device_curr_array[MAX_DEVICES];

#ifdef DEBUG
  printf("Do auto umount....\n");
#endif

  sleep(1);
  if((fptr = fopen(partitions,"r"))== NULL)
  {
    printf("Error opening file: %s\n", partitions);
    return;
  }

  for(ii=0;ii<MAX_DEVICES;ii++)
  {
    if (device_array[ii].mounted == 1)
    {
      /* do unmount here. */
#ifdef DEBUG
      //printf("Unmount: %s", device_array[ii].name);
#endif
      auto_umount_usb_dev(&device_array[ii]);
    }
  }

    /* fiji added start Bob, 2008/09/04 */
    /* Fiji modified pling, 2008/10/09 */
    /* TI USB LED is shared with Power LED, so don't change LED status */
#if (!defined TI_ALICE)
    int fd;
    fd = open("/dev/gpio_drv", O_RDWR);
    ioctl(fd, IOCTL_USB_LED_STATE, 0);
    close(fd);
#endif
    /* fiji added end Bob, 2008/09/04 */

  /* Fix for hub resets */
  /* unmount already mounted devices & remount it again!*/
/* Wins add-S 0925-08 */
#if (defined TI_ALICE)
    FILE *fp;

    if ((fp = fopen("/var/mnt/usbdev1", "r")) != NULL)
    {
        fclose(fp);
        system("rm -f /var/mnt/usbdev1\n");
    }
    if (num != SIGXCPU)
    {
        /* Do re-mount */
#endif /* (TI_ALICE) */
/* Wins add-E 0925-08 */
  auto_mount(0);
/* Wins add-S 0925-08 */
#if (defined TI_ALICE)
    }
#endif /* (TI_ALICE) */
/* Wins add-E 0925-08 */
}

/* This function checks if sdx is the only entry in the entire list,
 * if so this returns 1, else it clears the entry for sdx, makes
 * it a free slot  & returns 0
 */

static int check_is_this_only_partition(unsigned int index)
{
  int ii=0;

  for(ii=0;ii<MAX_DEVICES;ii++)
  {
    if(ii != index)
    {
      if(!strncmp(device_array[ii].name, device_array[index].name, 3))
      {
        /* Match found , remove the primary entry from the device array */

#ifdef DEBUG_1
        printf("Second entry for primary found, removing from the list\n");
#endif
        memset(&device_array[index], 0, sizeof(usb_dev_t));
        device_array[index].freeSlot = 1;
        return 0;
      }
    }

  }

  if(ii == MAX_DEVICES)
  {
#ifdef DEBUG_1
    printf("No second entry for primary\n");
#endif
    return 1;
  }
}

static void init_free_slots(usb_dev_t dev[])
{
  int ii=0;

  for(ii=0; ii < MAX_DEVICES; ii++)
  {
    dev[ii].freeSlot = 1;
  }

}

static int get_free_slot(void )
{
  int ii=0;

  for(ii=0; ii < MAX_DEVICES; ii++)
  {
    if(device_array[ii].freeSlot == 1)
    {
      break;
    }
  }

  if(ii == MAX_DEVICES)
  {
    printf("Maximum number of devices mounted\n");
    return -1;
  }

  return ii;
}

static void *parse_tokens(char *string, usb_dev_t *dev_ptr)
{
  volatile int ii=0;
  char *pChar = string;
  char *pTmpChar = NULL;

  while((pTmpChar = strtok(pChar, " ")) != NULL)
  {

    /* Fill the device structure */
    if(ii == 0)
    {
      dev_ptr->major = atoi(pTmpChar);
    }
    else if(ii == 1)
    {
      dev_ptr->minor = atoi(pTmpChar);
    }
    else if(ii == 2)
    {
      dev_ptr->numBlocks = atoi(pTmpChar);
    }
    else if(ii == 3)
    {
      strcpy(dev_ptr->name,pTmpChar);

      /* Handle the case where we try to mount sda, sdb etc */
      if(strlen(dev_ptr->name)<=4)
      {
#ifdef DEBUG
        //printf("Not adding :%s to list\n", dev_ptr->name);
#endif
#ifndef MOUNT_PRIMARY
        memset(dev_ptr, 0, sizeof(usb_dev_t));
        dev_ptr->freeSlot = 1;
#else
        dev_ptr->freeSlot = 0;
        dev_ptr->primary = 1;
#endif
      }
      else
      {
        dev_ptr->freeSlot = 0;
#ifdef MOUNT_PRIMARY
        dev_ptr->primary = 0;
#endif
      }
    }

    ii++;
    pChar = NULL;
  }
}

static int auto_mount_usb_dev(usb_dev_t *dev_ptr)
{
  char path[64];
  int ret;
  char mntdir[64];
  if(dev_ptr->mounted == 1)
  {
    printf("Not mounting already mounted: %s", dev_ptr->name);
    return 0;
  }

  if(!strlen(dev_ptr->name))
  {
    return 0;
  }

  memset(path, 0, 64);
  memset(mntdir, 0, 64);

  sprintf(path, "/dev/%s", dev_ptr->name);
  path[strlen(dev_ptr->name)-1+5]=0;

  sprintf(mntdir, "/var/mnt/%s", dev_ptr->name);

  /* Make sure the following line is modified if the above path is modified.
   */
  mntdir[strlen(dev_ptr->name)-1+9]=0;

#if NTFS_RW
  char ntfs_mount[200];
  memset(ntfs_mount, 0, 200);

  sprintf(ntfs_mount, "ntfs-3g ");
  strcat(ntfs_mount, path);
  strcat(ntfs_mount, " ");
  strcat(ntfs_mount, mntdir);
#endif

  if((ret = mknod(path, S_IFBLK, (dev_ptr->major << 8)|(dev_ptr->minor))) == -1)
  {

    if(errno == EEXIST)
    {
#ifdef DEBUG_1
      //printf("mknod failed as the file %s exists, doing mount :%s\n", dev_ptr->name, mntdir);
#endif
      mkdir(mntdir, 0755);
#ifdef DEBUG_1
      //printf("mknod failed as the file %s exists, doing mount :%s\n", dev_ptr->name, mntdir);
      //printf("Finally Mount:: %s %s\n",path,mntdir);
#endif
      if(mount(path, mntdir,"vfat", 0, NULL) == 0)
      {
        dev_ptr->mounted = 1;
      }
      else if(mount(path, mntdir,"ntfs", 0, NULL) == 0) //zzz: mount ntfs
      {
        dev_ptr->mounted = 1;
      }
      else if(mount(path, mntdir,"msdos", 0, NULL) == 0) //zzz: mount fat16
      {
        dev_ptr->mounted = 1;
      }
#if NTFS_RW
      else if(system(ntfs_mount) != -1)
      {
        dev_ptr->mounted = 1;
      }
#endif
      else
      {
        dev_ptr->mounted = 1;
#ifdef DEBUG
        //printf("Mount Failed %s\n",dev_ptr->name);
#endif
      }
      return 0;
    }
    return -1;
  }
  else
  {
    chmod(path,S_IRUSR|S_IWUSR);
#ifdef DEBUG_1
    //printf("mknod for %s: Success\n", dev_ptr->name);
#endif

    mkdir(mntdir, 0755);
#ifdef DEBUG
    //printf("Finally Mount:: %s %s\n",path,mntdir);
#endif
    if(mount(path, mntdir,"vfat", 0, NULL) == 0)
    {
      dev_ptr->mounted = 1;
    }
    else if(mount(path, mntdir,"ntfs", 0, NULL) == 0) //zzz: mount ntfs
    {
      dev_ptr->mounted = 1;
    }
    else if(mount(path, mntdir,"msdos", 0, NULL) == 0) //zzz: mount fat16
    {
      dev_ptr->mounted = 1;
    }
#if NTFS_RW
    else if(system(ntfs_mount) != -1)
    {
      dev_ptr->mounted = 1;
    }
#endif
    else
    {
      dev_ptr->mounted = 1;
#ifdef DEBUG
      //printf("Mount Failed %s\n",dev_ptr->name);
#endif
    }
  }
}


static int auto_umount_usb_dev(usb_dev_t *dev_ptr)
{
  char path[64];
  int ret, flags=0;
  char mntdir[64];

  if(dev_ptr->mounted == 0)
  {
    return -1;
  }

  memset(path, 0, 64);
  memset(mntdir, 0, 64);

  sprintf(path, "/dev/%s", dev_ptr->name);
  path[strlen(dev_ptr->name)-1+5]=0;

  /* If you modify the path update next line accordingly */
  sprintf(mntdir, "/var/mnt/%s", dev_ptr->name);

  /* Make sure the following line is modified if the above path is modified.
   */
  mntdir[strlen(dev_ptr->name)-1+9]=0;

  /* unmount the directory */
#ifdef DEBUG
  //printf("Finally unmount ::: %s\n",mntdir);
#endif

  flags = MNT_DETACH;
  if (umount2(mntdir, flags)  == 0)
  {
    /* remove /dev/sdxx file  after unmounting */
    remove(path);

    /* remove /dev/sdxx file  after unmounting */
    rmdir(mntdir);

    memset(dev_ptr, 0, sizeof(usb_dev_t));
    dev_ptr->freeSlot = 1;
    dev_ptr->mounted = 0;
  }
  else
  {
/////////////
    char cmd[128]="";
    sprintf(cmd, "umount %s", mntdir);
    system(cmd);
    /* remove /dev/sdxx file  after unmounting */
    remove(path);

    /* remove /dev/sdxx file  after unmounting */
    rmdir(mntdir);

    memset(dev_ptr, 0, sizeof(usb_dev_t));
    dev_ptr->freeSlot = 1;
    dev_ptr->mounted = 0;
////////////////////

    printf("Failed to Unmount %s, errno: %d\n",mntdir, errno);
    printf("Failed to Unmount\n");
  }

  return 0;
}

