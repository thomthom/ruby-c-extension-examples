from conans import ConanFile, tools


class SketchUpRubyConan(ConanFile):
    name = "SketchUpRuby"
    version = "2018"
    license = "<Put the package license here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "SketchUp Ruby headers and binaries"
    settings = { "os": ["Windows", "Macos"], "arch": ["x86_64", "x86"] }
    options = { "ruby": ["1.8","2.0", "2.2"] }
    no_copy_source = True
    exports_sources = "ThirdParty/*", "!ThirdParty/bin/*", "Hello World/src/RubyUtils/RubyLib.h"
    generators = "cmake"

    def build(self):
        pass

    def package(self):
        # Headers
        if self.settings.os == "Windows":
            arch = "win32_x64" if self.settings.arch == "x86_64" else "win32"
            if self.options.ruby == "1.8":
                if self.settings.arch == "x86":
                    lib = "msvcrt-ruby18.lib"
                else:
                    raise Exception("Ruby 1.8 is 32bit only")
            elif self.options.ruby == "2.0":
                if self.settings.arch == "x86_64":
                    lib = "x64-msvcrt-ruby200.lib"
                else:
                    lib = "msvcrt-ruby200.lib"
            elif self.options.ruby == "2.2":
                if self.settings.arch == "x86_64":
                    lib = "x64-msvcrt-ruby220.lib"
                else:
                    raise Exception("Ruby 2.0 is 64bit only")
        else:
            arch = "mac"
            lib = "%s/Ruby.framework" % self.options.ruby
        header_source = "ThirdParty/include/ruby/%s/%s" % (self.options.ruby, arch)
        self.copy("*.h", dst="include", src=header_source)
        # Helper include header
        # TODO: Rename to sketchupruby.h
        ruby_utils_source = "Hello World/src/RubyUtils"
        self.copy("RubyLib.h", dst="include", src=ruby_utils_source)
        # Binaries
        if self.settings.os == "Windows":
            self.copy(lib, dst="lib", src="ThirdParty/lib/win32", keep_path=False)
        else:
            self.copy(lib, dst="lib", src="ThirdParty/lib/mac", keep_path=False)

    def package_info(self):
        if self.settings.os == "Windows":
            if self.options.ruby == "1.8":
                # 32bit only
                self.cpp_info.libs = ["msvcrt-ruby18"]
            elif self.options.ruby == "2.0":
                # 32bit and 64bit
                if self.settings.arch == "x86_64":
                    self.cpp_info.libs = ["x64-msvcrt-ruby200"]
                    self.cpp_info.includedirs.append("include/x64-mswin64_100")
                else:
                    self.cpp_info.libs = ["msvcrt-ruby200"]
                    self.cpp_info.includedirs.append("include/i386-mswin32_100")
            elif self.options.ruby == "2.2":
                # 64bit only
                self.cpp_info.libs = ["x64-msvcrt-ruby220"]
                self.cpp_info.includedirs.append("include/x64-mswin64_140")
        else:
            self.cpp_info.libs = ["Ruby"]
            if self.options.ruby == "2.0":
                self.cpp_info.includedirs.append("include/universal-darwin12.5.0")
            elif self.options.ruby == "2.2":
                self.cpp_info.includedirs.append("include/x86_64-darwin14")
