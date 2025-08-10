fn main() {
    println!("cargo:rustc-link-search=native=.");

    #[cfg(target_os = "macos")]
    println!("cargo:rustc-link-arg=-Wl,-rpath,.");
}
