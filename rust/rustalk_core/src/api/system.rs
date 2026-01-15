#[unsafe(no_mangle)]
pub extern "C" fn rustalk_add(a: i32, b: i32) -> i32 {
    a + b
}

use std::ffi::{c_char, CStr};
use crate::runtime::{channels, dispatcher::CoreEvent};

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_connect(url: *const c_char) -> i32 {
    if url.is_null() {
        return -1;
    }
    let cstr = unsafe { CStr::from_ptr(url) };
    match cstr.to_str() {
        Ok(u) => match channels::send(CoreEvent::Connect(u.to_string())) {
            Ok(_) => 0,
            Err(_) => -2,
        },
        Err(_) => -3,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_status() -> i32 {
    1
}
