from conans import ConanFile, CMake, tools

class EmulatorNetworkConan(ConanFile):
    name = "emulator_network"
    version = "0.2.4"
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of Qxcpp here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "executable": [True, False]}
    default_options = {"shared": False, "executable": False}
    generators = "cmake"
    exports_sources = ["src/**", "CMakeLists.txt"]
    def requirements(self):
        self.requires("boost/1.78.0")

    def configure(self):
        pass

    def build(self):
        cmake = CMake(self)
        if self.options.executable:
            cmake.definitions["RUN_SERVER"] = "True"

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
