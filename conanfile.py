from conans import ConanFile, CMake, tools

class EmulatorNetworkConan(ConanFile):
    name = "emulator_network"
    version = "0.1.0"
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of Qxcpp here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake"
    exports_sources = ["src/**", "CMakeLists.txt"]
    def requirements(self):
        self.requires("boost/1.77.0")

    def configure(self):
        self.options["boost"].zlib = False
        self.options["boost"].bzip2 = False
        self.options["boost"].lzma = False
        self.options["boost"].zstd = False
        self.options["boost"].without_graph = True
        self.options["boost"].without_iostreams = True
        self.options["boost"].without_log = True
        self.options["boost"].without_locale = True
        self.options["boost"].without_math = True
        self.options["boost"].without_random = True
        self.options["boost"].without_stacktrace = True

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.dll", src="bin")
        self.copy("*.a", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.hpp", dst="include/network", keep_path=False)

    def package_info(self):
        if self.options.shared == False:
            self.cpp_info.libs = ["emulator_network"]
