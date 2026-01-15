use std::ffi::{c_char, CStr};
use std::time::{SystemTime, UNIX_EPOCH};
use crate::domain::message::Message;
use crate::runtime::{channels, dispatcher::CoreEvent};
use std::ffi::CString;

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_send_message(from: i64, to: i64, content: *const c_char) -> i32 {
    if content.is_null() {
        return -1;
    }
    let cstr = unsafe { CStr::from_ptr(content) };
    let text = match cstr.to_str() {
        Ok(s) => s.to_string(),
        Err(_) => return -2,
    };
    let ts = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .ok()
        .map(|d| d.as_secs() as i64)
        .unwrap_or(0);
    let msg = Message { from, to, content: text, timestamp: ts };
    match channels::send(CoreEvent::SendMessage(msg)) {
        Ok(_) => 0,
        Err(_) => -3,
    }
}

#[repr(C)]
pub struct MessageFFI {
    pub from: i64,
    pub to: i64,
    pub content: *mut c_char,
    pub timestamp: i64,
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_fetch_history(peer: i64, limit: i32, out_len: *mut i32) -> *mut MessageFFI {
    let msgs = crate::storage::sqlite::load_messages(peer, limit);
    let mut ffi: Vec<MessageFFI> = Vec::with_capacity(msgs.len());
    for m in msgs {
        let s = CString::new(m.content).unwrap_or_else(|_| CString::new("").unwrap());
        ffi.push(MessageFFI {
            from: m.from,
            to: m.to,
            content: s.into_raw(),
            timestamp: m.timestamp,
        });
    }
    unsafe {
        if !out_len.is_null() {
            *out_len = ffi.len() as i32;
        }
    }
    let boxed = ffi.into_boxed_slice();
    Box::into_raw(boxed) as *mut MessageFFI
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_free_messages(ptr: *mut MessageFFI, len: i32) {
    if ptr.is_null() || len <= 0 {
        return;
    }
    unsafe {
        let slice = std::slice::from_raw_parts_mut(ptr, len as usize);
        for m in slice.iter_mut() {
            if !m.content.is_null() {
                let _ = CString::from_raw(m.content);
                m.content = std::ptr::null_mut();
            }
        }
        let _ = Box::from_raw(slice as *mut [MessageFFI]);
    }
}
