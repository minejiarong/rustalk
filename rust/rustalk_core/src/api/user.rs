use std::ffi::{c_char, CStr};
use crate::domain::session;
use crate::storage::sqlite;
use rand::RngCore;
use sha2::{Digest, Sha256};

fn hash_password_with_salt(password: &str, salt: &str) -> String {
    let mut hasher = Sha256::new();
    hasher.update(salt.as_bytes());
    hasher.update(password.as_bytes());
    let res = hasher.finalize();
    hex::encode(res)
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_register(username: *const c_char, password: *const c_char, out_user_id: *mut i64) -> i32 {
    if username.is_null() || password.is_null() {
        return -1;
    }
    let uname = match unsafe { CStr::from_ptr(username) }.to_str() {
        Ok(s) => s,
        Err(_) => return -2,
    };
    let pwd = match unsafe { CStr::from_ptr(password) }.to_str() {
        Ok(s) => s,
        Err(_) => return -3,
    };
    let mut salt_bytes = [0u8; 16];
    rand::thread_rng().fill_bytes(&mut salt_bytes);
    let salt = base64::encode(salt_bytes);
    let hash = hash_password_with_salt(pwd, &salt);
    match sqlite::create_user(uname, &hash, &salt) {
        Ok(uid) => {
            unsafe {
                if !out_user_id.is_null() {
                    *out_user_id = uid;
                }
            }
            0
        }
        Err(e) => {
            let msg = format!("{e}");
            if msg.contains("UNIQUE") {
                -10 // username exists
            } else {
                -11 // DB error
            }
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_login(username: *const c_char, password: *const c_char, out_user_id: *mut i64) -> i32 {
    if username.is_null() || password.is_null() {
        return -1;
    }
    let uname = match unsafe { CStr::from_ptr(username) }.to_str() {
        Ok(s) => s,
        Err(_) => return -2,
    };
    let pwd = match unsafe { CStr::from_ptr(password) }.to_str() {
        Ok(s) => s,
        Err(_) => return -3,
    };
    match sqlite::find_user(uname) {
        Ok(Some((uid, hash, salt))) => {
            let calc = hash_password_with_salt(pwd, &salt);
            if calc == hash {
                session::set_current_user(uid);
                unsafe {
                    if !out_user_id.is_null() {
                        *out_user_id = uid;
                    }
                }
                0
            } else {
                -20 // wrong password
            }
        }
        Ok(None) => -21, // user not found
        Err(_) => -11,   // DB error
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_logout() -> i32 {
    session::clear_current_user();
    0
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::env;
    use std::fs;
    use std::ffi::CString;
    use crate::storage::sqlite;

    fn db_path(name: &str) -> String {
        let mut p = env::temp_dir();
        p.push(format!("rustalk_user_{}.db", name));
        p.to_string_lossy().to_string()
    }

    #[test]
    fn register_and_login() {
        let path = db_path("reglogin");
        let _ = fs::remove_file(&path);
        sqlite::init(&path);
        let uname = CString::new(format!("tester_{}", rand::thread_rng().next_u32())).unwrap();
        let pwd = CString::new("secret").unwrap();
        let mut uid: i64 = 0;
        let rc = rustalk_register(uname.as_ptr(), pwd.as_ptr(), &mut uid as *mut i64);
        assert_eq!(rc, 0);
        assert!(uid > 0);

        let wrong = CString::new("bad").unwrap();
        let mut out: i64 = 0;
        let rc_bad = rustalk_login(uname.as_ptr(), wrong.as_ptr(), &mut out as *mut i64);
        assert_eq!(rc_bad, -20);

        let rc_ok = rustalk_login(uname.as_ptr(), pwd.as_ptr(), &mut out as *mut i64);
        assert_eq!(rc_ok, 0);
        assert_eq!(out, uid);
    }
}
