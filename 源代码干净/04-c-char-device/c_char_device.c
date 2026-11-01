#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/jiffies.h>
#include <linux/string.h>

#define DEVICE_NAME "os_lab_char"
#define MESSAGE_SIZE 128
#define READ_BUF_SIZE 512

static char message[MESSAGE_SIZE] = "hello from os_lab_char";
static unsigned long open_count;
static unsigned long read_count;
static unsigned long write_count;
static unsigned long close_count;
static unsigned long load_jiffies;

static DEFINE_MUTEX(os_lab_lock);

static void strip_newline(char *s)
{
    size_t len = strlen(s);

    if (len > 0 && s[len - 1] == '\n')
        s[len - 1] = '\0';
}

static int os_lab_open(struct inode *inode, struct file *file)
{
    mutex_lock(&os_lab_lock);
    open_count++;
    mutex_unlock(&os_lab_lock);

    pr_info("c_char_device: device opened\n");
    return 0;
}

static int os_lab_release(struct inode *inode, struct file *file)
{
    mutex_lock(&os_lab_lock);
    close_count++;
    mutex_unlock(&os_lab_lock);

    pr_info("c_char_device: device closed\n");
    return 0;
}

static ssize_t os_lab_read(struct file *file, char __user *buf,
                           size_t count, loff_t *ppos)
{
    char kbuf[READ_BUF_SIZE];
    int len;

    if (*ppos > 0)
        return 0;

    mutex_lock(&os_lab_lock);
    read_count++;

    len = scnprintf(kbuf, sizeof(kbuf),
                    "message = %s\n"
                    "open_count = %lu\n"
                    "read_count = %lu\n"
                    "write_count = %lu\n"
                    "close_count = %lu\n"
                    "load_jiffies = %lu\n"
                    "current_jiffies = %lu\n",
                    message, open_count, read_count, write_count,
                    close_count, load_jiffies, jiffies);

    mutex_unlock(&os_lab_lock);

    return simple_read_from_buffer(buf, count, ppos, kbuf, len);
}

static ssize_t os_lab_write(struct file *file, const char __user *buf,
                            size_t count, loff_t *ppos)
{
    char kbuf[MESSAGE_SIZE];

    if (count == 0)
        return 0;

    if (count >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;

    kbuf[count] = '\0';
    strip_newline(kbuf);

    mutex_lock(&os_lab_lock);
    strscpy(message, kbuf, sizeof(message));
    write_count++;
    mutex_unlock(&os_lab_lock);

    pr_info("c_char_device: message updated: %s\n", message);
    return count;
}

static const struct file_operations os_lab_fops = {
    .owner = THIS_MODULE,
    .open = os_lab_open,
    .release = os_lab_release,
    .read = os_lab_read,
    .write = os_lab_write,
};

static struct miscdevice os_lab_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &os_lab_fops,
    .mode = 0666,
};

static int __init c_char_device_init(void)
{
    int ret;

    load_jiffies = jiffies;

    ret = misc_register(&os_lab_misc_device);
    if (ret) {
        pr_err("c_char_device: failed to register misc device\n");
        return ret;
    }

    pr_info("c_char_device: module loaded, /dev/%s created\n", DEVICE_NAME);
    return 0;
}

static void __exit c_char_device_exit(void)
{
    misc_deregister(&os_lab_misc_device);
    pr_info("c_char_device: module unloaded, /dev/%s removed\n", DEVICE_NAME);
}

module_init(c_char_device_init);
module_exit(c_char_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("student");
MODULE_DESCRIPTION("A simple character device module for openEuler");