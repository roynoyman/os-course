//
// Created by Roy Noyman on 02/12/2021.
//
#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/unistd.h>
#include "message_slot.h"
#include <linux/module.h>


MODULE_LICENSE("GPL");

typedef struct data {
    unsigned long channel_num;
    unsigned int minor;
} data;

typedef struct msg {
    unsigned char buffer[BUFFER_LENGTH];
    int len;
} msg;

typedef struct channel_node {
    long channel_id;
    int msg_size;
    msg *message;
    struct channel_node *next;
} channel_node;

static channel_node channel_minors[MAX_MINORS];

//================== DEVICE FUNCTIONS ===========================
static int device_open(struct inode *inode, struct file *file) {
    data *open_file_path;
    open_file_path = kmalloc(sizeof(data), GFP_KERNEL);
    if (open_file_path == NULL) {
        return 1;
    }
    file->private_data = (void *) open_file_path;
    ((data *) (file->private_data))->channel_num = 0;
    ((data *) (file->private_data))->minor = iminor(inode);
    printk("open is not broken \n");
    return 0;
}
//---------------------------------------------------------------
static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long ioctl_param) {
    if (ioctl_command_id != MSG_SLOT_CHANNEL || ioctl_param == 0) {
        return -EINVAL;
    }
    printk("ioctl is not broken");
    ((data *) (file->private_data))->channel_num = ioctl_param;
    return 0;
}
//---------------------------------------------------------------
static ssize_t device_read(struct file *file, char __user *buf, size_t length, loff_t *lof) {
    struct channel_node *curr;
    int check;
    size_t msg_len;
    if (((data *) (file->private_data))->channel_num == 0 || buf == NULL) {
        return -EINVAL;
    }
    curr = &channel_minors[((data *) (file->private_data))->minor];
    while (curr->next != NULL && curr->channel_id != ((data *) (file->private_data))->channel_num) {
        curr = (curr->next);
    }
    if (curr->channel_id != ((data *) (file->private_data))->channel_num || curr->message == NULL) {
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
        return 0;
    }
}
//---------------------------------------------------------------
static ssize_t device_write(struct file *file, const char __user *buf, size_t length, loff_t *lof) {
    struct channel_node *curr;
    int check;
    if (length < 1 || length > BUFFER_LENGTH) {
        return -EMSGSIZE;
    }
    if (((data *) (file->private_data))->channel_num == 0 || buf == NULL) {
        return -EINVAL;
    }
    curr = &(channel_minors[((data *) (file->private_data))->minor]);
    while (curr->next != NULL && curr->channel_id != ((data *) (file->private_data))->channel_num) {
        curr = (curr->next);
    }
    if (curr->channel_id != ((data *) (file->private_data))->channel_num) {
        curr->next = kcalloc(1, sizeof(struct channel_node), GFP_KERNEL);
        if (curr->next == NULL) {
            return -1;
        }
        curr = curr->next;
        curr->channel_id = ((data *) (file->private_data))->channel_num;
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
        return 0;
    }
}
//---------------------------------------------------------------
static int device_release(struct inode *inode, struct file *file){
    data *release_file = (data *)(file->private_data);
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
        printk(KERN_ERR "Device_init failed %d\n", MAJOR_NUMBER);
        return major_num;
    }
    return 0;
}

static void __exit device_cleanup(void){
    struct channel_node *curr;
    struct channel_node *next;
    int i;
    for(i=0; i<MAX_MINORS; i++){
        if(&channel_minors[i] != NULL){
            curr = channel_minors[i].next;
            next = curr->next;
            while(next!=NULL){
                if(curr->message != NULL){
                    kfree(curr->message);
                }
                kfree(curr);
                curr = next;
                next = curr->next;
            }
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
