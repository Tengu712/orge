use std::ffi::CString;
use std::os::raw::c_char;

#[link(name = "orge", kind = "dylib")]
unsafe extern "C" {
    fn orgeShowDialog(dtype: u32, title: *const c_char, message: *const c_char);
    fn orgeGetErrorMessage() -> *const c_char;
    fn orgeInitializeWith(yamlFilePath: *const c_char) -> u8;
    fn orgeTerminate();
    fn orgeUpdate() -> u8;
    fn orgeBeginRender() -> u8;
    fn orgeEndRender() -> u8;
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

    while unsafe { orgeUpdate() } != 0 {
        if unsafe { orgeBeginRender() } == 0 {
            eprintln!("failed to begin rendering.");
        }
        if unsafe { orgeEndRender() } == 0 {
            eprintln!("failed to end rendering.");
        }
    }

    unsafe { orgeTerminate() };
}
