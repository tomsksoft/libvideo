import sys, os
from sys import platform

build_type = "--build-release"
cflags_path = ""
enable_amf = "--disable-amf"
enable_clang_tidy = "--disable-tidy"
enable_android_build = False

for i in range(1, len(sys.argv)):
    if (sys.argv[i] == "--build-release"):
        build_type = "--build-release"

    if (sys.argv[i] == "--build-debug"):
        build_type = "--build-debug"

    if (sys.argv[i] == "-cflags"):
        cflags_path = sys.argv[i + 1]

    if (sys.argv[i] == "--enable-amf"):
        enable_amf = sys.argv[i]
    
    if (sys.argv[i] == "--enable-tidy"):
        enable_clang_tidy = sys.argv[i]

    if (sys.argv[i] == "--android"):
        enable_android_build = True

if (platform == "linux" or platform == "linux2"):
    if enable_android_build:
        archs = ["arm64-v8a", "armeabi-v7a", "x86_64", "x86"]

        profile_build = os.getcwd() + "/.ci/conan_profiles/conan_profile_linux_release"

        if (build_type == "--build-debug"):
            profile_build = os.getcwd() + "/.ci/conan_profiles/conan_profile_linux_debug"

        for arch in archs:
            profile_host = os.getcwd() + "/.ci/conan_profiles/Android/Release/host_android_" + arch + "_release"

            if (build_type == "--build-debug"):
                profile_host = os.getcwd() + "/.ci/conan_profiles/Android/Debug/host_android_" + arch + "_debug"

            optinons = "-o "
            if (enable_clang_tidy == "--enable-tidy"):
                optinons += "with_clangtidy=True"
            else:
                optinons += "with_clangtidy=False"

            libx264DeployCommand = "conan create .ci/libx264_conan_recipe libx264/cci.20220602@libvideo/stable -pr:h " + profile_host + " -pr:b=" + profile_build + " --build=missing"
            ffmpegDeployCommand = "conan create .ci/ffmpeg_conan_recipe ffmpeg/5.0@libvideo/stable -pr:h " + profile_host + " -pr:b=" + profile_build + " --build=missing"
            libvideoDeployCommand = "conan create . libvideo/1.0.0@ccor/libvideo -tf None -pr:h " + profile_host + " -pr:b=" + profile_build + " " + optinons + " --build=missing"

            os.system(libx264DeployCommand)
            os.system(ffmpegDeployCommand)
            os.system(libvideoDeployCommand)
    else:
        profile = os.getcwd() + "/.ci/conan_profiles/conan_profile_linux_release"

        if (build_type == "--build-debug"):
            profile = os.getcwd() + "/.ci/conan_profiles/conan_profile_linux_debug"

        optinons = "-o "
        if (enable_clang_tidy == "--enable-tidy"):
            optinons += "with_clangtidy=True"
        else:
            optinons += "with_clangtidy=False"
        
        libx264DeployCommand = "conan create .ci/libx264_conan_recipe libx264/cci.20220602@libvideo/stable -pr=" + profile + " --build=missing"
        ffmpegDeployCommand = "conan create .ci/ffmpeg_conan_recipe ffmpeg/5.0@libvideo/stable -pr=" + profile + " --build=missing"
        libvideoDeployCommand = "conan create . libvideo/1.0.0@ccor/libvideo -tf None -pr=" + profile + " --build=missing"
        libvideoInstallCommand = "conan install . --install-folder build_ -pr=" + profile + " " + optinons + " --build=missing"

        os.system(libx264DeployCommand)
        os.system(ffmpegDeployCommand)
        os.system(libvideoDeployCommand)
        os.system(libvideoInstallCommand)

if (platform == "win32"):
    profile = os.getcwd() + "\.ci\conan_profiles\conan_profile_release"

    if (build_type == "--build-debug"):
        profile = os.getcwd() + "\.ci\conan_profiles\conan_profile_debug"

    if (cflags_path != ""):
        cflags_path = "-e " + "CFLAGS=" + "-I" + cflags_path

    optinons = "-o "
    if (enable_amf == "--enable-amf"):
        optinons += "with_amf=True "
        enable_amf = "-o with_amf=True"
    else:
        optinons += "with_amf=False "
        enable_amf = "-o with_amf=False"

    if (enable_clang_tidy == "--enable-tidy"):
        optinons += "-o with_clangtidy=True"
    else:
        optinons += "-o with_clangtidy=False"

    libx264DeployCommand = "conan create .ci/libx264_conan_recipe libx264/cci.20220602@libvideo/stable -pr=" + profile + " --build=missing"
    ffmpegDeployCommand = "conan create .ci/ffmpeg_conan_recipe ffmpeg/5.0@libvideo/stable -pr=" + profile + " " + cflags_path + " " + enable_amf + " --build=missing"
    libvideoInstallCommand = "conan install . --install-folder build_ -pr=" + profile + " " + cflags_path + " " + optinons + " --build=missing"

    os.system(libx264DeployCommand)
    os.system(ffmpegDeployCommand)
    os.system(libvideoInstallCommand)