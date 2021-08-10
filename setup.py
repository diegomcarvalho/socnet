from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys
import setuptools
from codecs import open  # To use a consistent encoding
from os import path

here = path.abspath(path.dirname(__file__))

import os


def get_c_sources(folder, include_headers=False):
    """Find all C/C++ source files in the `folder` directory."""
    allowed_extensions = [".c", ".C", ".cc", ".cpp", ".cxx", ".c++"]
    if include_headers:
        allowed_extensions.extend([".h", ".hpp"])
    sources = []
    for root, dirs, files in os.walk(folder):
        for name in files:
            ext = os.path.splitext(name)[1]
            if ext in allowed_extensions:
                sources.append(os.path.join(root, name))
    return sources


# Get the long description from the relevant file
with open(path.join(here, "README.md"), encoding="utf-8") as f:
    long_description = f.read()
    f.close()

with open("src/version.h") as f:
    line = f.read()
    __version__ = line.split('"')[1]
    f.close()

__package_name__ = "socnet"
__src__ = get_c_sources("src", include_headers=(sys.argv[1] == "sdist"))


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked."""

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11

        return pybind11.get_include(self.user)


ext_modules = [
    Extension(
        __package_name__,
        sources=__src__,
        include_dirs=[
            # Path to pybind11 headers
            get_pybind_include(),
            get_pybind_include(user=True),
        ],
        language="c++",
    ),
]


# As of Python 3.6, CCompiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile

    with tempfile.NamedTemporaryFile("w", suffix=".cpp") as f:
        f.write("int main (int argc, char **argv) { return 0; }")
        try:
            compiler.compile([f.name], extra_postargs=[flagname])
        except setuptools.distutils.errors.CompileError:
            return False
    return True


def cpp_flag(compiler):
    """Return the -std=c++[11/14] compiler flag.
    The c++14 is prefered over c++11 (when it is available).
    """
    if has_flag(compiler, "-std=c++17"):
        return "-std=c++17"
    elif has_flag(compiler, "-std=c++14"):
        return "-std=c++14"
    elif has_flag(compiler, "-std=c++11"):
        return "-std=c++11"
    else:
        raise RuntimeError(
            "Unsupported compiler -- at least C++11 support " "is needed!"
        )


class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""

    c_opts = {
        "msvc": ["/EHsc"],
        "unix": [],
    }

    if sys.platform == "darwin":
        c_opts["unix"] += ["-stdlib=libc++", "-mmacosx-version-min=10.7"]

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        if ct == "unix":
            opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())
            opts.append(cpp_flag(self.compiler))
            if has_flag(self.compiler, "-fvisibility=hidden"):
                opts.append("-fvisibility=hidden")
        elif ct == "msvc":
            opts.append('/DVERSION_INFO=\\"%s\\"' % self.distribution.get_version())
        for ext in self.extensions:
            ext.extra_compile_args = opts
        build_ext.build_extensions(self)


setup(
    name=__package_name__,
    version=__version__,
    author="Diego Carvalho",
    author_email="d.carvalho@ieee.org",
    url="https://github.com/diegomcarvalho/socnet",
    download_url="https://github.com/diegomcarvalho/socnet/archive/v"
    + str(__version__)
    + ".tar.gz",
    description="Social Network Model for COVID-19 infection",
    long_description=long_description,
    long_description_content_type="text/markdown",
    ext_modules=ext_modules,
    install_requires=["pybind11>=2.2"],
    cmdclass={"build_ext": BuildExt},
    zip_safe=False,
    keywords=["covid19", "social network", "complex network", "net"],
    classifiers=["License :: OSI Approved :: MIT License"],
    headers=["src/population.hpp", "src/statistics.hpp"],
)
