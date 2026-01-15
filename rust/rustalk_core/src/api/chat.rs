use std::ffi::{c_char, CStr};
use std::time::{SystemTime, UNIX_EPOCH};
use crate::domain::message::Message;
use crate::runtime::{channels, dispatcher::CoreEvent};

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

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_fetch_history(_peer: i64, _limit: i32) -> *mut core::ffi::c_void {
    core::ptr::null_mut()
}
