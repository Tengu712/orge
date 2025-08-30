#pragma once

#ifdef _MSC_VER
// NOTE: MSVCで警告が出てビルドが失敗するので、stb_vorbis.cだけ例外的に警告を無効化する。
#pragma warning(push)
#pragma warning(disable : 4244 4245 4456 4457 4701)
#elif defined(__clang__)
// NOTE: Clangで警告が出てビルドが失敗するので、stb_vorbis.cだけ例外的に警告を無効化する。
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#endif

#define STB_VORBIS_IMPLEMENTATION
#include <stb_vorbis.c>

#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#endif
