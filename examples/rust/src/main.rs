use std::ffi::CString;
use std::os::raw::c_char;

#[link(name = "orge", kind = "dylib")]
unsafe extern "C" {
    fn orgeShowDialog(dtype: u32, title: *const c_char, message: *const c_char);
    fn orgeGetErrorMessage() -> *const c_char;
    fn orgeInitializeWith(yamlFilePath: *const c_char) -> u8;
    fn orgeTerminate();
    fn orgeUpdate() -> u8;
    fn orgeCreateMesh(
        id: *const c_char,
        vertexCount: u32,
        vertices: *const f32,
        indexCount: u32,
        indices: *const u32,
    ) -> u8;
    fn orgeBeginRender() -> u8;
    fn orgeDraw(
        pipelineId: *const c_char,
        meshId: *const c_char,
        instanceCount: u32,
        instanceOffset: u32,
    ) -> u8;
    fn orgeEndRender() -> u8;
}

const VERTICES: &[f32] = &[
    // 左下
    -1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, // 上
    0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 1.0, // 右下
    1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0,
];
const INDICES: &[u32] = &[0, 1, 2];

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
    if unsafe { orgeInitializeWith(CString::new("config.yml").unwrap().as_ptr()) == 0 } {
        show_error_dialog();
        panic!("failed to initialize orge.");
    }
    if unsafe {
        orgeCreateMesh(
            CString::new("triangle").unwrap().as_ptr(),
            VERTICES.len() as u32,
            VERTICES.as_ptr(),
            INDICES.len() as u32,
            INDICES.as_ptr(),
        ) == 0
    } {
        show_error_dialog();
        panic!("failed to create a triangle.");
    }

    while unsafe { orgeUpdate() != 0 } {
        if unsafe { orgeBeginRender() == 0 } {
            eprintln!("failed to begin rendering.");
        }
        if unsafe {
            orgeDraw(
                CString::new("PL").unwrap().as_ptr(),
                CString::new("triangle").unwrap().as_ptr(),
                1,
                0,
            ) == 0
        } {
            eprintln!("failed to begin rendering.");
        }
        if unsafe { orgeEndRender() == 0 } {
            eprintln!("failed to end rendering.");
        }
    }

    unsafe { orgeTerminate() };
}
