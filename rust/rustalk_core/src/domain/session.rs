use once_cell::sync::OnceCell;
use std::sync::Mutex;

static CURRENT_USER: OnceCell<Mutex<Option<i64>>> = OnceCell::new();

pub fn set_current_user(user_id: i64) {
    let cell = CURRENT_USER.get_or_init(|| Mutex::new(None));
    let mut lock = cell.lock().unwrap();
    *lock = Some(user_id);
}

pub fn clear_current_user() {
    let cell = CURRENT_USER.get_or_init(|| Mutex::new(None));
    let mut lock = cell.lock().unwrap();
    *lock = None;
}

pub fn get_current_user() -> Option<i64> {
    CURRENT_USER
        .get()
        .and_then(|m| m.lock().ok().and_then(|g| *g))
}
