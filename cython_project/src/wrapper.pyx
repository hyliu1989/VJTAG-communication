from cpython.mem cimport PyMem_Malloc, PyMem_Free
from libcpp cimport bool
from libc.stdint cimport uintptr_t, uint8_t


cdef extern from "ftd2xx.h":
    ctypedef uint8_t BYTE
    ctypedef void* FT_HANDLE

cdef extern from "device.h":
    FT_HANDLE open_jtag_device()
    void close_jtag_device(FT_HANDLE ftHandle)

cdef extern from "jtag_tap.h":
    void common_functions_ANY_to_RST_to_IDL(BYTE *buf, int &cnt)
    void common_functions_IDL_to_SIR_to_IDL(BYTE *buf, int &cnt, BYTE *bits, int length, bool to_read)
    void common_functions_IDL_to_SDR_to_IDL(BYTE *buf, int &cnt, BYTE *bits, int length, bool to_read)

