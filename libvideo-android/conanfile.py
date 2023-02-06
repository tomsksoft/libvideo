import os
from conans import ConanFile, CMake

class libvideo_android(ConanFile):
    name = "libvideo_android"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    exports_sources = ["CMakeLists.txt"]

    def requirements(self):
        self.requires("libvideo/1.0.0@ccor/libvideo")

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder='.')
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include", src="src")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["libvideo_android"]
