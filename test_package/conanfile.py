import os

from conans import ConanFile, CMake, tools
from pkg_resources import require


class libvideo(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    requires = "libvideo/1.0.0@ccor/libvideo"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dylib*", dst="bin", src="lib")
        self.copy('*.so*', dst='bin', src='lib')

    def test(self):
        if not tools.cross_building(self):
            os.chdir("bin")
            self.run(".%slibvideo_test" % os.sep)
