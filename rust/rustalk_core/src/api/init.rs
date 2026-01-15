use std::ffi::{c_char, CStr};
use crate::storage::sqlite;
use crate::runtime::executor;

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_init(db_path: *const c_char) -> i32 {
    if db_path.is_null() {
        return -1;
    }
    let cstr = unsafe { CStr::from_ptr(db_path) };
    if let Ok(path) = cstr.to_str() {
        sqlite::init(path);
        executor::init()
    } else {
        -2
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_shutdown() {
    // 这里可以扩展为向 runtime 发送 Shutdown 事件并清理资源
}
