mod api;
mod domain;
mod net;
mod storage;
mod runtime;
mod util;

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_add(a: i32, b: i32) -> i32 {
    api::add(a, b)
}
