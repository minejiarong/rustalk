#[unsafe(no_mangle)]
pub extern "C" fn rustalk_add(a: i32, b: i32) -> i32 {
    a + b
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_status() -> i32 {
    1
}
