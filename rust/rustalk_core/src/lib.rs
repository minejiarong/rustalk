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

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_init() -> i32 {
    api::init::init_runtime() + api::init::init_db()
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_login() -> i32 {
    api::user::login()
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_logout() -> i32 {
    api::user::logout()
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_register() -> i32 {
    api::user::register()
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_send_message() -> i32 {
    api::chat::send_message()
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_fetch_messages() -> i32 {
    api::chat::fetch_messages()
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_shutdown() -> i32 {
    api::system::shutdown()
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_status() -> i32 {
    api::system::status()
}
