use std::ffi::{c_char, CStr, CString};

#[repr(C)]
pub struct ContactFFI {
    pub id: i64,
    pub name: *mut c_char,
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_fetch_contacts(out_len: *mut i32) -> *mut ContactFFI {
    let mut contacts = match crate::storage::sqlite::load_contacts() {
        Ok(list) => list,
        Err(_) => Vec::new(),
    };
    if contacts.is_empty() {
        let _ = crate::storage::sqlite::upsert_contact(1, "Alice");
        let _ = crate::storage::sqlite::upsert_contact(2, "Bob");
        let _ = crate::storage::sqlite::upsert_contact(3, "Charlie");
        contacts = crate::storage::sqlite::load_contacts().unwrap_or_default();
    }
    
    let mut ffi: Vec<ContactFFI> = Vec::with_capacity(contacts.len());
    for c in contacts {
        let s = CString::new(c.name).unwrap_or_else(|_| CString::new("").unwrap());
        ffi.push(ContactFFI {
            id: c.id,
            name: s.into_raw(),
        });
    }
    
    if !out_len.is_null() {
        unsafe { *out_len = ffi.len() as i32; }
    }
    
    let boxed = ffi.into_boxed_slice();
    Box::into_raw(boxed) as *mut ContactFFI
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_free_contacts(ptr: *mut ContactFFI, len: i32) {
    if ptr.is_null() || len <= 0 {
        return;
    }
    unsafe {
        let slice = std::slice::from_raw_parts_mut(ptr, len as usize);
        for c in slice.iter_mut() {
            if !c.name.is_null() {
                let _ = CString::from_raw(c.name);
                c.name = std::ptr::null_mut();
            }
        }
        let _ = Box::from_raw(slice as *mut [ContactFFI]);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn rustalk_upsert_contact(id: i64, name: *const c_char) -> i32 {
    if name.is_null() {
        return -1;
    }
    let cname = match unsafe { CStr::from_ptr(name) }.to_str() {
        Ok(s) => s,
        Err(_) => return -2,
    };
    match crate::storage::sqlite::upsert_contact(id, cname) {
        Ok(_) => 0,
        Err(_) => -3,
    }
}
