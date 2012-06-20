import logging
log = logging.getLogger(__name__)


__all__ = [
    'ValidReadPtr',
    'ValidWritePtr',
    'ValidPtr',
    'RequireValidPtr',
    'Detour',
    'DetourManager'
]


import _detour
import ctypes
import struct

try:
    import pydasm
except ImportError, e:
    print "Could not load pydasm."
    class pydasm(object):
        def __getattr__(self, attr):
            raise AttributeError(e)

from detour_manager import DetourManager
from detour_callback_object import DetourCallbackObject

from ctypes import Structure, c_long
class _registers(Structure):
    _fields_ = [
        ("edi", c_long),
        ("esi", c_long),
        ("ebp", c_long),
        ("esp", c_long),
        ("ebx", c_long),
        ("edx", c_long),
        ("ecx", c_long),
        ("eax", c_long),
    ]


class register_list:
    """Helpful register_list class. Holds the 8 main registers and the flags register"""
    def __init__(self, registertuple, flags):
        self.eax = registertuple[0]
        self.ecx = registertuple[1]
        self.edx = registertuple[2]
        self.ebx = registertuple[3]
        self.esp = registertuple[4]
        self.ebp = registertuple[5]
        self.esi = registertuple[6]
        self.edi = registertuple[7]
        self.flags = flags
    def _get_ctypes_instance(self):
        x = _registers()
        x.edi = self.edi
        x.esi = self.esi
        x.ebp = self.ebp
        x.esp = self.esp
        x.ebx = self.ebx
        x.edx = self.edx
        x.ecx = self.ecx
        x.eax = self.eax
        return x
    def __str__(self):
        return "(EAX: 0x%08x, ECX: 0x%08x, EDX: 0x%08x, EBX: 0x%08x, ESP: 0x%08x, EBP: 0x%08x, ESI: 0x%08x, EDI: 0x%08x, flags: 0x%08x)"%(self.eax, self.ecx, self.edx, self.ebx, self.esp, self.ebp, self.esi, self.edi, self.flags)


##############################################################
####Helper functions
def findOptimalTrampolineLength(address, minlen=5, maxlen=12, noisy=False):
    if noisy: log.debug("Determining optimal tramploine size for address 0x%08x:"%(address))
    buffer = _detour.util.read(address, maxlen+5)

    l = 0
    ic = 0
    offset = 0
    while l < maxlen:
        i = pydasm.get_instruction(buffer[offset:], pydasm.MODE_32)
        if not i:
            break
        if noisy: log.debug("%d bytes: %s"%(i.length, pydasm.get_instruction_string(i, pydasm.FORMAT_INTEL, 0)))
        ic += 1
        offset += i.length
        l += i.length
        if l >= minlen:
            break
    if noisy: log.debug("optimal size is %d bytes (%d instructions)"%(l, ic))
    return l

def findBytesToPop(address, maxlen=512, noisy=False):
    t = None
    if noisy: log.debug("Determining bytes to pop for function at address 0x%08x:"%(address))
    buffer = _detour.util.read(address, maxlen+5)
    #buffer = "\xC3" #ret
    #buffer = "\xC2\x04" #retn 4
    l = 0
    ic = 0
    offset = 0
    num = None
    while l < maxlen:
        i = pydasm.get_instruction(buffer[offset:], pydasm.MODE_32)
        if not i:
            break
        istr = pydasm.get_instruction_string(i, pydasm.FORMAT_INTEL, 0)
        if noisy: log.debug("%d bytes: %s"%(i.length, istr))
        ic += 1
        offset += i.length
        l += i.length
        if istr.strip() == "ret":
            if noisy: log.debug("found ret instruction (no bytes to pop)")
            num = 0
            t = "cdecl"
            break
        if istr.startswith("retn"):
            if noisy: log.debug(i)
            num = istr[5:]
            num = int(num, 16)
            t = "stdcall"
            if noisy: log.debug("found retn instruction, bytes to pop = %s"%(num))
            break
    if num is None:
        if noisy: log.debug("warning, no retn instruction found")
    else:
        if noisy: log.debug("bytes to pop is %d bytes (found after %d instructions)"%(num, ic))
    return (t, num)

def ValidReadPtr(addr, len=1):
    return not ctypes.windll.kernel32.IsBadReadPtr(addr, len)
def ValidWritePtr(addr, len=1):
    return not ctypes.windll.kernel32.IsBadWritePtr(addr, len)
def ValidPtr(addr, len=1):
    return ValidWritePtr(addr, len)
def RequireValidPtr(addr, len=1):
    if not ValidPtr(addr, len):
        raise Exception("Invalid Pointer 0x%08x"%(addr))

##############################################################

##############################################################
##############################################################

class pyDetourConfig:
    """Configuration of a pydetour"""
    def __init__(self, addr):
        self.address = addr
        self.return_to_original = True
        self.callback = None                 #Python function ready to receive callback
        self.callback_class = None            #Custom class to act as paramater to above callback, if desired
        self.bytes_to_pop = 0
        self.overwrite_len = None
        self.function_type = "cdecl"
    def __repr__(self):
        return "<DetourConfig for 0x%08x (%s%s), callback %r>"%(    self.address,
                                                                    self.function_type,
                                                                    (""," 0x%x bytes"%(self.bytes_to_pop))[self.function_type=="stdcall"],
                                                                    self.callback
                                                                )

        
##############################################################

class Detour:
    def __init__(    self, 
                    address,
                    return_to_original,
                    callback=None,
                    bytes_to_pop=None,
                    overwrite_len=None,
                    type=0, #unused, passed to C
                    callback_class=None
                ):
        """
            If return_to_original is true, then bytes_to_pop is not important.
            Conversely, if it is false, overwrite_len is not important.
        """
        self.applied = False
        if address in DetourManager:
            raise Exception, "Detour already exists!"
            
        self.name = "Detour 0x%08x"%(address)
        
        if callable(return_to_original) and callback == None:
            #this if supports constructs like this:
            #x = Detour(0x123, returnTrue)
            callback = return_to_original
            return_to_original = False

        try:
            if overwrite_len is None:
                overwrite_len = findOptimalTrampolineLength(address)
                if overwrite_len < 5 or overwrite_len > 12:
                    log.warning("Warning: guessed overwrite_len is %d for function at address 0x%08x"%(overwrite_len, address))

            if bytes_to_pop is None:
                (t, bytes_to_pop) = findBytesToPop(address)
                if bytes_to_pop is None:
                    raise Exception("Could not determine number of bytes to pop on return from function at 0x%08x"%(address))
            else:
                t = "stdcall"
            

            self.address = address
            self.config = pyDetourConfig(address)
            self.config.callback = callback
            self.config.function_type = t
            self.config.overwrite_len = overwrite_len
            self.config.bytes_to_pop = bytes_to_pop
            self.config.return_to_original = return_to_original
            if callback_class is None:
                callback_class = DetourCallbackObject
            self.config.callback_class = callback_class
            self.type = type
            log.debug("Creating python Detour object for function at 0x%08x (%s)"%(self.address, self.config))
        except _detour.DetourAccessViolationException:
            raise _detour.DetourAccessViolationException("Invalid detour address 0x%08x"%(address))

    def apply(self):
        if self.applied:
            return self
        log.debug("Applying detour at 0x%08x (%s)"%(self.address, self.config))
        try:
            self.applied = True
            DetourManager[self.address] = self
            _detour.createDetour(self.address, self.config.overwrite_len, self.config.bytes_to_pop, self.type)
            _detour.setDetourSettings(self.address, (self.config.bytes_to_pop, self.config.return_to_original, 0))
        except _detour.DetourAccessViolationException:
            self.applied=False
            del DetourManager[self.address]
            raise _detour.DetourAccessViolationException("Invalid detour address 0x%08x"%(self.address))
        return self

    def remove(self):
        if not self.applied:
            return self
        self.applied = False
        log.debug("Removing detour from 0x%08x (%s)"%(self.address, self.config))
        _detour.removeDetour(self.address)
        del DetourManager[self.address]
        return self
        
    def __repr__(self):
        return "<Detour '%s' detouring 0x%08x, %s>"%(self.name,
                                                    self.address,
                                                    ["Inactive", "Active"][self.applied]
                                                    )



def _main_callback(*args, **kwargs):
    """This function is the first to get control when any C level detour is hit. It is responsible for delegating the detour to the correct handler."""
    detouraddr, r, flags, caller = args[0], args[1], args[2], args[3]
    registers = register_list(r, flags)
    DetourManager.do_callback(detouraddr, registers, caller)
_detour.callback = _main_callback


##############################################################
##############################################################
##############################################################
##############################################################

def interact():
    import code
    code.interact(banner="\nIn Python Interactive Loop. Enter Ctrl-Z to continue.", local=globals())

def returnTrue(d):
    d.registers.eax = 1

def returnFalse(d):
    d.registers.eax = 0

flag = False
def returnTrueOnce(d):
    if not globals()['flag']:
        d.registers.eax = 1
        globals()['flag'] = True
    else:
        d.registers.eax = 0
    
def testcb(d):
    d.dump()
    d.registers.eax = 0;
    #d.applyRegisters()
    #d.detour.remove()
    d.callOriginal(("lol whut"))

    
def dword(x):
    if isinstance(x, basestring): #Converting /from/ memory to python int
        if len(x) % 4 != 0:
            raise TypeError("dword() must be called with a byte strings whose size is a multiple of 4. (Got %i bytes)"%(len(x)))
        n = len(x) / 4
        r = struct.unpack("L"*n, x)
        if len(r) == 1:
            return r[0]
        return r
    #Converting /to/ memory layout
    packed = ""
    try:
        for item in x:
            try:
                if item < 0 :
                    packed += struct.pack("l", item)
                else:
                    packed += struct.pack("L", item)
            except:
                raise TypeError("Could not pack %r into dword"%(item))
    except TypeError:
        item = x
        try:
            if item < 0 :
                packed += struct.pack("l", item)
            else:
                packed += struct.pack("L", item)
        except:
            raise TypeError("Could not pack %r into dword"%(x))
    return packed
def qword(x):
    if isinstance(x, basestring): #Converting /from/ memory to python int
        if len(x) % 8 != 0:
            raise TypeError("qword() must be called with a byte strings whose size is a multiple of 8. (Got %i bytes)"%(len(x)))
        n = len(x) / 8
        r = struct.unpack("Q"*n, x)
        if len(r) == 1:
            return r[0]
        return r
    #Converting /to/ memory layout
    packed = ""
    try:
        for item in x:
            try:
                if item < 0 :
                    packed += struct.pack("q", item)
                else:
                    packed += struct.pack("Q", item)
            except:
                raise TypeError("Could not pack %r into qword"%(item))
    except TypeError:
        item = x
        try:
            if item < 0 :
                packed += struct.pack("q", item)
            else:
                packed += struct.pack("Q", item)
        except:
            raise TypeError("Could not pack %r into qword"%(x))
    return packed



