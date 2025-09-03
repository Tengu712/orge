use std::ffi::CString;
use std::os::raw::c_char;

#[link(name = "orge", kind = "dylib")]
unsafe extern "C" {
    fn orgeShowDialog(dtype: u32, title: *const c_char, message: *const c_char);
    fn orgeGetErrorMessage() -> *const c_char;
    fn orgeInitialize() -> u8;
    fn orgeTerminate();
    fn orgeUpdate() -> u8;
    fn orgeLoadMesh(id: *const c_char) -> u8;
    fn orgeBeginRender() -> u8;
    fn orgeEndRender() -> u8;
    fn orgeBindMesh(id: *const c_char) -> u8;
    fn orgeBeginRenderPass(id: *const c_char) -> u8;
    fn orgeEndRenderPass() -> u8;
    fn orgeBindPipeline(id: *const c_char, indices: *const u32) -> u8;
    fn orgeDraw(instanceCount: u32, instanceOffset: u32) -> u8;
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
    if unsafe { orgeInitialize() == 0 } {
        show_error_dialog();
        panic!("failed to initialize orge.");
    }
    if unsafe { orgeLoadMesh(CString::new("triangle").unwrap().as_ptr()) == 0 } {
        show_error_dialog();
        panic!("failed to load a triangle.");
    }

    while unsafe { orgeUpdate() != 0 } {
        if unsafe { orgeBeginRender() == 0 } {
            eprintln!("failed to begin rendering.");
        }
        if unsafe { orgeBindMesh(CString::new("triangle").unwrap().as_ptr()) == 0 } {
            eprintln!("failed to bind a mesh.");
        }
        if unsafe { orgeBeginRenderPass(CString::new("RP").unwrap().as_ptr()) == 0 } {
            eprintln!("failed to begin a render pass.");
        }
        if unsafe { orgeBindPipeline(CString::new("PL").unwrap().as_ptr(), std::ptr::null()) == 0 }
        {
            eprintln!("failed to bind a pipeline.");
        }
        if unsafe { orgeDraw(1, 0) == 0 } {
            eprintln!("failed to draw a mesh.");
        }
        if unsafe { orgeEndRenderPass() == 0 } {
            eprintln!("failed to end a render pass.");
        }
        if unsafe { orgeEndRender() == 0 } {
            eprintln!("failed to end rendering.");
        }
    }

    unsafe { orgeTerminate() };
}
