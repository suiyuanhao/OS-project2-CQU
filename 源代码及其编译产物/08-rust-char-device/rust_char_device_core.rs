//! Rust character device lab module.

use core::ffi::{c_char, c_int, c_long};
use core::fmt::{self, Write};
use core::slice;
use core::str;
use core::sync::atomic::{AtomicI64, AtomicU64, Ordering};

use kernel::prelude::*;

const EINVAL: c_long = 22;

static COUNTER: AtomicI64 = AtomicI64::new(0);
static OPEN_COUNT: AtomicU64 = AtomicU64::new(0);
static RELEASE_COUNT: AtomicU64 = AtomicU64::new(0);
static READ_COUNT: AtomicU64 = AtomicU64::new(0);
static WRITE_COUNT: AtomicU64 = AtomicU64::new(0);
static INVALID_COUNT: AtomicU64 = AtomicU64::new(0);

extern "C" {
    fn rust_char_device_init() -> c_int;
    fn rust_char_device_exit();
}

module! {
    type: RustCharDevice,
    name: "rust_char_device",
    author: "OS Lab",
    description: "Rust character device lab module",
    license: "GPL",
}

struct RustCharDevice;

struct BufferWriter {
    ptr: *mut u8,
    capacity: usize,
    len: usize,
}

impl BufferWriter {
    unsafe fn new(ptr: *mut c_char, capacity: usize) -> Self {
        Self { ptr: ptr as *mut u8, capacity, len: 0 }
    }
}

impl Write for BufferWriter {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        let bytes = s.as_bytes();
        if self.len + bytes.len() > self.capacity {
            return Err(fmt::Error);
        }

        unsafe {
            core::ptr::copy_nonoverlapping(bytes.as_ptr(), self.ptr.add(self.len), bytes.len());
        }

        self.len += bytes.len();
        Ok(())
    }
}

#[no_mangle]
pub extern "C" fn rust_char_opened() {
    OPEN_COUNT.fetch_add(1, Ordering::Relaxed);
    pr_info!("rust_char_device: opened\n");
}

#[no_mangle]
pub extern "C" fn rust_char_released() {
    RELEASE_COUNT.fetch_add(1, Ordering::Relaxed);
    pr_info!("rust_char_device: released\n");
}

#[no_mangle]
pub unsafe extern "C" fn rust_char_render(buf: *mut c_char, size: usize) -> c_long {
    if buf.is_null() || size == 0 {
        return -EINVAL;
    }

    READ_COUNT.fetch_add(1, Ordering::Relaxed);

    let mut out = unsafe { BufferWriter::new(buf, size) };

    let _ = write!(
        out,
        "device = /dev/rust_lab_char\ncounter = {}\nopen_count = {}\nrelease_count = {}\nread_count = {}\nwrite_count = {}\ninvalid_count = {}\ncommands: inc, dec, reset, or a number\n",
        COUNTER.load(Ordering::Relaxed),
        OPEN_COUNT.load(Ordering::Relaxed),
        RELEASE_COUNT.load(Ordering::Relaxed),
        READ_COUNT.load(Ordering::Relaxed),
        WRITE_COUNT.load(Ordering::Relaxed),
        INVALID_COUNT.load(Ordering::Relaxed),
    );

    out.len as c_long
}

#[no_mangle]
pub unsafe extern "C" fn rust_char_store(buf: *const c_char, len: usize) -> c_long {
    if buf.is_null() || len == 0 {
        return -EINVAL;
    }

    WRITE_COUNT.fetch_add(1, Ordering::Relaxed);

    let bytes = unsafe { slice::from_raw_parts(buf as *const u8, len) };
    let input = match str::from_utf8(bytes) {
        Ok(s) => s.trim(),
        Err(_) => {
            INVALID_COUNT.fetch_add(1, Ordering::Relaxed);
            return -EINVAL;
        }
    };

    if input == "inc" {
        COUNTER.fetch_add(1, Ordering::Relaxed);
    } else if input == "dec" {
        COUNTER.fetch_sub(1, Ordering::Relaxed);
    } else if input == "reset" {
        COUNTER.store(0, Ordering::Relaxed);
    } else if let Ok(value) = input.parse::<i64>() {
        COUNTER.store(value, Ordering::Relaxed);
    } else {
        INVALID_COUNT.fetch_add(1, Ordering::Relaxed);
        pr_info!("rust_char_device: invalid input\n");
        return -EINVAL;
    }

    pr_info!("rust_char_device: command accepted: {}\n", input);
    len as c_long
}

impl kernel::Module for RustCharDevice {
    fn init(_module: &'static ThisModule) -> Result<Self> {
        let ret = unsafe { rust_char_device_init() };
        if ret != 0 {
            return Err(ENOMEM);
        }

        pr_info!("rust_char_device: module loaded, /dev/rust_lab_char created\n");
        Ok(RustCharDevice)
    }
}

impl Drop for RustCharDevice {
    fn drop(&mut self) {
        unsafe {
            rust_char_device_exit();
        }

        pr_info!("rust_char_device: module unloaded, /dev/rust_lab_char removed\n");
    }
}