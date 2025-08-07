#pragma once

#ifdef _MSC_VER
// NOTE: MSVCで警告が出てビルドが失敗するので、stb_vorbis.cだけ例外的に警告を無効化する。
#pragma warning(push)
#pragma warning(disable : 4244 4456 4457 4245)
#endif

#define STB_VORBIS_IMPLEMENTATION
#include <stb_vorbis.c>

#ifdef _MSC_VER
#pragma warning(pop)
#endif
