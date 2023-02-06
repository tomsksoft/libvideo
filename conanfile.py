import os
from conans import ConanFile, CMake

class libvideo(ConanFile):
    name = "libvideo"
    version = "1.0.0"
    description = "Library used to encode/decode video"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    exports_sources = ["src/*", "CMakeLists.txt", "test_package/*", "Include/*"]

    options = {
        "with_amf": [True, False],
        "with_clangtidy": [True, False],
    }

    default_options = {
        "with_amf": False,
        "with_clangtidy": False,
    }

    def configure(self):
        self.options["ffmpeg"].shared = False
        self.options["ffmpeg"].with_openh264 = False
        self.options["ffmpeg"].with_libx264 = True
        self.options["libx264"].shared = False
        self.options["ffmpeg"].with_zlib = False
        self.options["ffmpeg"].with_bzip2 = False
        self.options["ffmpeg"].with_lzma = False
        self.options["ffmpeg"].with_freetype = False
        self.options["ffmpeg"].with_openjpeg = False
        self.options["ffmpeg"].with_opus = False
        self.options["ffmpeg"].with_vorbis = False
        self.options["ffmpeg"].with_sdl = False
        self.options["ffmpeg"].with_libx265 = False
        self.options["ffmpeg"].with_libvpx = False
        self.options["ffmpeg"].with_libmp3lame = False
        self.options["ffmpeg"].with_libfdk_aac = False
        self.options["ffmpeg"].with_libwebp = False
        self.options["ffmpeg"].with_ssl = False
        self.options["ffmpeg"].with_asm = False
        self.options["ffmpeg"].with_amf = self.options.with_amf
        
        if self.settings.os == "Android":
            self.options["ffmpeg"].enable_decoders = "h264_mediacodec"
            self.options["ffmpeg"].enable_decoders = "hevc_mediacodec"
            self.options["ffmpeg"].enable_decoders = "mpeg4_mediacodec"

    def requirements(self):
        self.requires("ffmpeg/5.0@libvideo/stable")
        self.requires("libx264/cci.20220602@libvideo/stable")
        if self.settings.os == "Windows":
            self.requires("gtest/1.8.1")
        else:
            pass

    def build(self):
        cmake = CMake(self)
        if self.options.with_clangtidy:
            cmake.definitions["LIBVIDEO_USE_CLANGTIDY"] = "TRUE"
        else:
            cmake.definitions["LIBVIDEO_USE_CLANGTIDY"] = ""
        cmake.configure(source_folder='.')
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include", src="Include")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["libvideo"]
