from setuptools import setup, Extension
from Cython.Distutils import build_ext

NAME = "vjtag"
VERSION = "0.1"
DESCR = "Python wrapper of C++ codes to communicate with DE0-Nano through Altera Virtual JTAG."
URL = ""
REQUIRES = ['cython']

AUTHOR = "Hsiou-Yuan Liu"
EMAIL = "hyhliu1989@gmail.com"

LICENSE = "BSD"

PACKAGES = [NAME]
C_PROJECT_SRC_DIR = '../cpp_project/src_pure_c/'
SRC_DIR = 'src/'
SRC_FILES = [
    SRC_DIR + "wrapper.pyx",
    C_PROJECT_SRC_DIR + "device.cpp",
    C_PROJECT_SRC_DIR + "jtag_tap.cpp",
]


ext_1 = Extension(NAME+"._wrapper", SRC_FILES, libraries=[], include_dirs=[C_PROJECT_SRC_DIR])

EXTENSIONS = [ext_1]

if __name__ == "__main__":
    setup(
        install_requires=REQUIRES,
        packages=PACKAGES,
        zip_safe=False,
        name=NAME,
        version=VERSION,
        description=DESCR,
        author=AUTHOR,
        author_email=EMAIL,
        url=URL,
        license=LICENSE,
        cmdclass={"build_ext": build_ext},
        ext_modules=EXTENSIONS
    )
