#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/unistd.h>
#include "message_slot.h"

MODULE_LICENSE("GPL");

typedef struct data_device {
    unsigned long channel_num;
    unsigned int minor;
} data_device;

typedef struct msg {
    unsigned char buffer[BUFFER_LENGTH];
    int len;
} msg;

typedef struct channel_node {
    long channel_id;
    msg *message;
    struct channel_node *next;
} channel_node;

static channel_node channel_minors[MAX_MINORS]; //data structure (I chose linked list) to describe indiviaul message slots.

unsigned int get_device_minor(struct file *file){
    return ((data_device *) (file->private_data))->minor;
}

unsigned long get_device_channel_num(struct file *file){
    return ((data_device *) (file->private_data))->channel_num;
}
//================== DEVICE FUNCTIONS ===========================
static int device_open(struct inode *inode, struct file *file) {
    data_device *open_file_path;
    open_file_path = kmalloc(sizeof(data_device), GFP_KERNEL);
    if (open_file_path == NULL) {
        return 1;
    }
    file->private_data = (void *) open_file_path;
    ((data_device *) (file->private_data))->channel_num = 0;
    ((data_device *) (file->private_data))->minor = iminor(inode);
    printk("open is not broken \n");
    return 0;
}
//---------------------------------------------------------------
static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long ioctl_param) {
    if (ioctl_command_id != MSG_SLOT_CHANNEL || ioctl_param == 0) {
        return -EINVAL;
    }
    ((data_device *) (file->private_data))->channel_num = ioctl_param;
    return 0;
}
//---------------------------------------------------------------
static ssize_t device_read(struct file *file, char __user *buf, size_t length, loff_t *lof) {
    struct channel_node *curr;
    int check;
    size_t msg_len;
    if (get_device_channel_num(file) == 0 || buf == NULL) {
        return -EINVAL;
    }
    curr = &channel_minors[get_device_minor(file)];
    while (curr->next != NULL && curr->channel_id != get_device_channel_num(file)) { //search over the linked list
        curr = (curr->next);
    }
    if (curr->channel_id != get_device_channel_num(file) || curr->message == NULL) {
        return -EWOULDBLOCK;
    }
    if (length < (curr->message)->len) {
        return -ENOSPC;
    }
    if ((curr->message)->len == 0) {
        return -1;
    }
    msg_len = curr->message->len;
    check = copy_to_user(buf, curr->message->buffer, msg_len);
    if (check == 0) {
        return msg_len;
    } else {
        curr->message->len = 0;
        return -1;
    }
}
//---------------------------------------------------------------
static ssize_t device_write(struct file *file, const char __user *buf, size_t length, loff_t *lof) {
    struct channel_node *curr;
    int check;
    if (length < 1 || length > BUFFER_LENGTH) {
        return -EMSGSIZE;
    }
    if (get_device_channel_num(file)== 0 || buf == NULL) {
        return -EINVAL;
    }
    curr = &(channel_minors[get_device_minor(file)]);
    while (curr->next != NULL && curr->channel_id != get_device_channel_num(file)) { //search over the linked list
        curr = (curr->next);
    }
    if (curr->channel_id != get_device_channel_num(file)) {
        curr->next = kcalloc(1, sizeof(struct channel_node), GFP_KERNEL);
        if (curr->next == NULL) {
            return -1;
        }
        curr = curr->next;
        curr->channel_id =  get_device_channel_num(file);
    }
    if (curr->message != NULL) {
        kfree(curr->message);
    }
    curr->message = kcalloc(1, sizeof(struct msg), GFP_KERNEL);
    if (curr->message == NULL) {
        return -1;
    }
    check = copy_from_user(curr->message->buffer, buf, length);
    if (check == 0) {
        curr->message->len = length;
        return length;
    } else {
        curr->message->len = 0;
        return -1;
    }
}
//---------------------------------------------------------------
static int device_release(struct inode *inode, struct file *file){
    data_device *release_file = (data_device *)(file->private_data);
    kfree(release_file);
    return 0;
}
//==================== DEVICE SETUP =============================
struct file_operations Fops =
{
    .owner = THIS_MODULE,
    .write = device_write,
    .read = device_read,
    .open = device_open,
    .unlocked_ioctl = device_ioctl,
    .release = device_release,
};

static int __init device_init(void){
    int major_num;
    major_num = register_chrdev(MAJOR_NUMBER, MY_CHAR_DEV ,&Fops);
    if(major_num < 0){
        printk(KERN_ERR "DEVICE_INIT FAILED %d\n", MAJOR_NUMBER);
        return major_num;
    }
    return 0;
}

static void __exit device_cleanup(void){
    struct channel_node *curr;
    struct channel_node *next;
    int i;
    for(i=0; i<MAX_MINORS; i++){
        curr = (&channel_minors[i])->next; //Head of linked list
        if(curr != NULL){
            next = curr->next;
            while(next!=NULL){
                if(curr->message != NULL){
                    kfree(curr->message);
                }
                kfree(curr);
                curr = next;
                next = curr->next;
            }
            // last curr is not null
            if(curr->message != NULL){
                kfree(curr->message);
            }
            kfree(curr);
        }
    }
    unregister_chrdev(MAJOR_NUMBER,MY_CHAR_DEV);
}

module_init(device_init);
module_exit(device_cleanup);
