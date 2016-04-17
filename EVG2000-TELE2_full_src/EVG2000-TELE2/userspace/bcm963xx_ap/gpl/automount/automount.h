
#define MAX_DEVICES 416 
#define MAX_STR_LEN 256

typedef struct usb_dev_s {
  int major;
  int minor;
  int numBlocks;
  char name[16];
  int freeSlot;
  int primary;
  int mounted;
}usb_dev_t;

static void init_free_slots(usb_dev_t dev[]);
static int get_free_slot(void );
static void *parse_tokens(char *string, usb_dev_t *dev_ptr);
static int auto_mount_usb_dev(usb_dev_t *dev_ptr);
static int auto_umount_usb_dev(usb_dev_t *dev_ptr);
static int check_is_this_only_partition(unsigned int index);

static void auto_mount(int num);
static void auto_umount(int num);


