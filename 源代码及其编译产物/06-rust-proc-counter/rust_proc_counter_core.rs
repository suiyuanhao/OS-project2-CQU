// SPDX-License-Identifier: GPL-2.0
//! Rust procfs counter lab module.

use core::ffi::{c_char, c_int, c_long};
use core::fmt::{self, Write};
use core::slice;
use core::str;
use core::sync::atomic::{AtomicI64, AtomicU64, Ordering};
use kernel::prelude::*;

module! {
    type: RustProcCounter,
    name: "rust_proc_counter",
    author: "student",
    description: "A Rust /proc counter module for openEuler",
    license: "GPL",
}

extern "C" {
    fn rust_proc_counter_register() -> c_int;
    fn rust_proc_counter_unregister();
}

static COUNTER: AtomicI64 = AtomicI64::new(0);
static READ_COUNT: AtomicU64 = AtomicU64::new(0);
static WRITE_COUNT: AtomicU64 = AtomicU64::new(0);
static INVALID_COUNT: AtomicU64 = AtomicU64::new(0);

const EINVAL_NEG: c_long = -22;

struct KernelBuf {
    ptr: *mut u8,
    cap: usize,
    len: usize,
}

impl KernelBuf {
    fn new(ptr: *mut u8, cap: usize) -> Self {
        Self { ptr, cap, len: 0 }
    }
}

impl Write for KernelBuf {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        let bytes = s.as_bytes();
        if self.len + bytes.len() > self.cap {
            return Err(fmt::Error);
        }

        unsafe {
            core::ptr::copy_nonoverlapping(
                bytes.as_ptr(),
                self.ptr.add(self.len),
                bytes.len(),
            );
        }

        self.len += bytes.len();
        Ok(())
    }
}

#[no_mangle]
pub unsafe extern "C" fn rust_proc_counter_render(buf: *mut c_char, size: usize) -> c_long {
    if buf.is_null() || size == 0 {
        return EINVAL_NEG;
    }

    let read_count = READ_COUNT.fetch_add(1, Ordering::Relaxed) + 1;
    let mut out = KernelBuf::new(buf as *mut u8, size);

    let _ = write!(
        &mut out,
        "counter = {}\nread_count = {}\nwrite_count = {}\ninvalid_count = {}\n",
        COUNTER.load(Ordering::Relaxed),
        read_count,
        WRITE_COUNT.load(Ordering::Relaxed),
        INVALID_COUNT.load(Ordering::Relaxed),
    );

    out.len as c_long
}

#[no_mangle]
pub unsafe extern "C" fn rust_proc_counter_store(buf: *const c_char, len: usize) -> c_long {
    if buf.is_null() || len == 0 {
        return EINVAL_NEG;
    }

    let bytes = unsafe { slice::from_raw_parts(buf as *const u8, len) };
    let input = match str::from_utf8(bytes) {
        Ok(s) => s.trim(),
        Err(_) => {
            INVALID_COUNT.fetch_add(1, Ordering::Relaxed);
            return EINVAL_NEG;
        }
    };

    match input.parse::<i64>() {
        Ok(value) => {
            COUNTER.store(value, Ordering::Relaxed);
            WRITE_COUNT.fetch_add(1, Ordering::Relaxed);
            pr_info!("rust_proc_counter: counter set to {}\n", value);
            len as c_long
        }
        Err(_) => {
            INVALID_COUNT.fetch_add(1, Ordering::Relaxed);
            pr_info!("rust_proc_counter: invalid input\n");
            EINVAL_NEG
        }
    }
}

struct RustProcCounter;

impl kernel::Module for RustProcCounter {
    fn init(_module: &'static ThisModule) -> Result<Self> {
        let ret = unsafe { rust_proc_counter_register() };
        if ret != 0 {
            return Err(ENOMEM);
        }

        pr_info!("rust_proc_counter: module loaded, /proc/rust_lab_counter created\n");
        Ok(RustProcCounter)
    }
}

impl Drop for RustProcCounter {
    fn drop(&mut self) {
        unsafe {
            rust_proc_counter_unregister();
        }

        pr_info!("rust_proc_counter: module unloaded, /proc/rust_lab_counter removed\n");
    }
}