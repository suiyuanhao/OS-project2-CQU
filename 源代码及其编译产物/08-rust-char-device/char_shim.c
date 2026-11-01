// SPDX-License-Identifier: GPL-2.0
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#define LOCAL_BUF_SIZE 128
#define OUTPUT_BUF_SIZE 512

extern void rust_char_opened(void);
extern void rust_char_released(void);
extern ssize_t rust_char_render(char *buf, size_t size);
extern ssize_t rust_char_store(const char *buf, size_t len);

static int lab_open(struct inode *inode, struct file *file)
{
    rust_char_opened();
    return 0;
}

static int lab_release(struct inode *inode, struct file *file)
{
    rust_char_released();
    return 0;
}

static ssize_t lab_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    char buf[OUTPUT_BUF_SIZE];
    ssize_t len;

    if (*ppos > 0)
        return 0;

    len = rust_char_render(buf, sizeof(buf));
    if (len < 0)
        return len;

    return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static ssize_t lab_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    char buf[LOCAL_BUF_SIZE];
    size_t len;
    ssize_t ret;

    len = min(count, sizeof(buf) - 1);

    if (copy_from_user(buf, ubuf, len))
        return -EFAULT;

    buf[len] = '\0';

    ret = rust_char_store(buf, len);
    if (ret < 0)
        return ret;

    return count;
}

static const struct file_operations lab_fops = {
    .owner = THIS_MODULE,
    .open = lab_open,
    .read = lab_read,
    .write = lab_write,
    .release = lab_release,
};

static struct miscdevice lab_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "rust_lab_char",
    .fops = &lab_fops,
    .mode = 0666,
};

int rust_char_device_init(void)
{
    return misc_register(&lab_misc_device);
}

void rust_char_device_exit(void)
{
    misc_deregister(&lab_misc_device);
}