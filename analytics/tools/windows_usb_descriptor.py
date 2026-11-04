"""Read USB descriptors for ZMK devices through Windows hub IOCTLs (no writes)."""
import ctypes as C
from ctypes import wintypes as W
import json
import struct
import sys


def inspect():
    if sys.platform != "win32":
        raise RuntimeError("Run with Windows Python")
    setup = C.WinDLL("setupapi", use_last_error=True)
    kernel = C.WinDLL("kernel32", use_last_error=True)

    class GUID(C.Structure):
        _fields_ = [("a", W.DWORD), ("b", W.WORD), ("c", W.WORD), ("d", C.c_ubyte * 8)]

    class Interface(C.Structure):
        _fields_ = [("size", W.DWORD), ("guid", GUID), ("flags", W.DWORD), ("reserved", C.c_void_p)]

    guid = GUID(0xF18A0E88, 0xC30C, 0x11D0, (C.c_ubyte * 8)(0x88, 0x15, 0, 0xA0, 0xC9, 6, 0xBE, 0xD8))
    setup.SetupDiGetClassDevsW.argtypes = [C.POINTER(GUID), W.LPCWSTR, W.HWND, W.DWORD]
    setup.SetupDiGetClassDevsW.restype = W.HANDLE
    setup.SetupDiEnumDeviceInterfaces.argtypes = [W.HANDLE, C.c_void_p, C.POINTER(GUID), W.DWORD, C.POINTER(Interface)]
    setup.SetupDiGetDeviceInterfaceDetailW.argtypes = [W.HANDLE, C.POINTER(Interface), C.c_void_p, W.DWORD, C.POINTER(W.DWORD), C.c_void_p]
    setup.SetupDiDestroyDeviceInfoList.argtypes = [W.HANDLE]
    kernel.CreateFileW.argtypes = [W.LPCWSTR, W.DWORD, W.DWORD, C.c_void_p, W.DWORD, W.DWORD, W.HANDLE]
    kernel.CreateFileW.restype = W.HANDLE
    kernel.DeviceIoControl.argtypes = [W.HANDLE, W.DWORD, C.c_void_p, W.DWORD, C.c_void_p, W.DWORD, C.POINTER(W.DWORD), C.c_void_p]
    kernel.CloseHandle.argtypes = [W.HANDLE]
    invalid = C.c_void_p(-1).value
    devices = setup.SetupDiGetClassDevsW(C.byref(guid), None, None, 0x12)
    if devices == invalid:
        raise C.WinError(C.get_last_error())

    def ioctl(handle, code, request, size):
        buf = C.create_string_buffer(size)
        C.memmove(buf, request, len(request))
        count = W.DWORD()
        if not kernel.DeviceIoControl(handle, code, buf, size, buf, size, C.byref(count), None):
            return None
        return bytes(buf)[:count.value]

    results = []
    try:
        idx = 0
        while True:
            interface = Interface()
            interface.size = C.sizeof(Interface)
            if not setup.SetupDiEnumDeviceInterfaces(devices, None, C.byref(guid), idx, C.byref(interface)):
                if C.get_last_error() != 259:
                    raise C.WinError(C.get_last_error())
                break
            idx += 1
            needed = W.DWORD()
            setup.SetupDiGetDeviceInterfaceDetailW(devices, C.byref(interface), None, 0, C.byref(needed), None)
            detail = C.create_string_buffer(needed.value)
            C.cast(detail, C.POINTER(W.DWORD))[0] = 8 if C.sizeof(C.c_void_p) == 8 else 6
            if not setup.SetupDiGetDeviceInterfaceDetailW(devices, C.byref(interface), detail, needed.value, None, None):
                raise C.WinError(C.get_last_error())
            path = C.wstring_at(C.addressof(detail) + 4)
            handle = kernel.CreateFileW(path, 0, 3, None, 3, 0, None)
            if handle == invalid:
                continue
            try:
                for port in range(1, 33):
                    info = ioctl(handle, 0x220448, struct.pack("<I", port), 4096)
                    if not info or len(info) < 22:
                        continue
                    descriptor = info[4:22]
                    vid, pid = struct.unpack_from("<HH", descriptor, 8)
                    if vid != 0x1D50 or pid != 0x615E:
                        continue
                    request = struct.pack("<IBBHHH", port, 0x80, 6, 0x0200, 0, 4096)
                    raw = ioctl(handle, 0x220410, request, 4108)
                    config = raw[12:] if raw else b""
                    entries = []
                    offset = 0
                    while offset + 2 <= len(config) and config[offset] >= 2:
                        length, kind = config[offset:offset + 2]
                        value = config[offset:offset + length]
                        if len(value) < length:
                            break
                        if kind == 2 and length >= 9:
                            entries.append(dict(type="configuration", length=int.from_bytes(value[2:4], "little"), interfaces=value[4]))
                        if kind == 4 and length >= 9:
                            entries.append(dict(type="interface", number=value[2], endpoints=value[4], usb_class=value[5], subclass=value[6]))
                        if kind == 5 and length >= 7:
                            entries.append(dict(type="endpoint", address=hex(value[2]), attributes=value[3]))
                        offset += length
                    product = None
                    if descriptor[15]:
                        string_request = struct.pack("<IBBHHH", port, 0x80, 6,
                                                     0x0300 | descriptor[15], 0x0409, 255)
                        string_raw = ioctl(handle, 0x220410, string_request, 267)
                        if string_raw and len(string_raw) >= 14 and string_raw[13] == 3:
                            product = string_raw[14:12 + string_raw[12]].decode("utf-16-le")
                    results.append(dict(port=port, product=product, device_descriptor=descriptor.hex(),
                                        config_hex=config.hex(), entries=entries))
            finally:
                kernel.CloseHandle(handle)
    finally:
        setup.SetupDiDestroyDeviceInfoList(devices)
    return results


if __name__ == "__main__":
    print(json.dumps(inspect(), indent=2))
