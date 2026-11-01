// SPDX-License-Identifier: GPL-2.0

//! A minimal Rust kernel module for openEuler.

use kernel::prelude::*;

module! {
    type: RustHello,
    name: "rust_hello",
    author: "student",
    description: "A simple openEuler kernel module written in Rust",
    license: "GPL",
}

struct RustHello;

impl kernel::Module for RustHello {
    fn init(_module: &'static ThisModule) -> Result<Self> {
        pr_info!("rust_hello: module loaded\n");
        Ok(RustHello)
    }
}

impl Drop for RustHello {
    fn drop(&mut self) {
        pr_info!("rust_hello: module unloaded\n");
    }
}