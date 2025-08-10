use std::ffi::CString;
use std::os::raw::c_char;

#[link(name = "orge", kind = "dylib")]
unsafe extern "C" {
    fn orgeShowDialog(dtype: u32, title: *const c_char, message: *const c_char);
    fn orgeGetErrorMessage() -> *const c_char;
    fn orgeInitializeWith(yamlFilePath: *const c_char) -> u8;
    fn orgeTerminate();
}

fn show_error_dialog() {
    unsafe {
        orgeShowDialog(
            0,
            CString::new("error").unwrap().as_ptr(),
            orgeGetErrorMessage(),
        )
    }
}

fn main() {
    if unsafe { orgeInitializeWith(CString::new("config.yml").unwrap().as_ptr()) } == 0 {
        show_error_dialog();
        panic!("failed to initialize orge.");
    }

    unsafe { orgeTerminate() };
}
