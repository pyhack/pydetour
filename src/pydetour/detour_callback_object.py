import _detour
import logging
import ctypes

log = logging.getLogger(__name__)

DEBUG_LOG_SPAM = False

class DetourCallbackObject:
    """This is the object passed to function that are registed as callbacks. It should be the only way to interact with pydetour"""

    def __init__(self, detour, registers, caller):
        if DEBUG_LOG_SPAM:
            log.debug("init CallBackObject for %s"%(detour))
        self.address = detour.address
        self.registers = registers
        self.caller = caller
        self.detour = detour

    def applyRegisters(self):
        """This function is automaticly called after the callback function.
        It will set the registers to what was specified in the object.

        It is possible you will .remove() the detour before this is done.
        In that case, call this function manually before doing so."""
        r = (
            self.registers.eax,
            self.registers.ecx,
            self.registers.edx,
            self.registers.ebx,
            self.registers.esp,
            self.registers.ebp,
            self.registers.esi,
            self.registers.edi,
            )
        if DEBUG_LOG_SPAM:
            log.debug("Changing registers: %s" %(self.registers))
        return _detour.setRegisters(self.address, r, self.registers.flags, self.caller)

    @staticmethod
    def read(address, length):
        """Reads length bytes from address"""
        return _detour.util.read(address, length)

    @staticmethod
    def write(address, length, bytes):
        """Writes length bytes to address"""
        return _detour.util.write(address, length, bytes)

        
    def debug_break(self):
        _detour.break_into_debugger()
        
    def dump(self):
        """Convient utility function to dump information about this function call"""
        print "Dump for call to 0x%08x from 0x%08x:"%(self.address, self.caller)
        print "\tRegisters:"
        print "\t\tEAX: 0x%08x" % (self.registers.eax)
        print "\t\tECX: 0x%08x" % (self.registers.ecx)
        print "\t\tEDX: 0x%08x" % (self.registers.edx)
        print "\t\tEBX: 0x%08x" % (self.registers.ebx)
        print "\t\tESP: 0x%08x" % (self.registers.esp)
        print "\t\tEBP: 0x%08x" % (self.registers.ebp)
        print "\t\tESI: 0x%08x" % (self.registers.esi)
        print "\t\tEDI: 0x%08x" % (self.registers.edi)
        print "\t\tflags: 0x%08x" % (self.registers.flags)
        self.dump_stack()
    def dump_stack(self, dword_offset=0, count=8):
        return self._dump_memory(self.registers.esp, dword_offset, count, "ESP")
        
    def _dump_memory(self, addr, dword_offset=0, count=8, addr_lbl="X"):
        #TODO: use addr variable
        for i in xrange(dword_offset, dword_offset+count):
            if i == 0:
                t = "   "
            else:
                t = "+%02X"%((i)*4)
            try:
                a = self.getStackValue(i)
            except _detour.DetourAccessViolationException:
                print "\t[ESP%s]: Access Violation"% (t)

            try:
                b = self.getStringStackValue(i)
                if len(b) == 1:
                    #might be unicode
                    b = self.getUnicodeStackValue(i)
                    b = b.encode("ascii", "replace");
                    print "\t[%s%s]: 0x%08x (U: '%s')" % (addr_lbl, t, a, b)
                else:
                    print "\t[%s%s]: 0x%08x ('%s')" % (addr_lbl, t, a, b)
            except _detour.DetourAccessViolationException:
                print "\t[ESP%s]: 0x%08x" % (t, a)
    @property
    def memory(self):
        return _detour.memory
        
    def getStackValue(self, stackNum):
        """Returns a value from the stack. 0 = ESP."""
        add = stackNum * 4 #4 byte paramters
        return _detour.util.readDWORD(self.registers.esp+add)

    def getStackValues(self, stackNum, count):
        """Returns a value from the stack. 0 = ESP."""
        ret = []
        for i in xrange(0, count):
            add = (stackNum+i) * 4 #4 byte paramters
            ret.append(_detour.util.readDWORD(self.registers.esp+add))
        return ret

    def setStackValue(self, stackNum, dword):
        """Returns a value from the stack. 0 = ESP."""
        add = stackNum * 4 #4 byte paramters
        return _detour.util.writeDWORD(self.registers.esp+add, dword)
    
    def getStringStackValue(self, stackNum):
        """Returns a string from a stack pointer. 0 = ESP. Looks up an ASCII string pointed at by stack offset."""
        addr = self.getStackValue(stackNum)
        return _detour.util.readASCIIZ(addr)

    def getUnicodeStackValue(self, stackNum):
        """Returns a unicode string from a stack pointer. 0 = ESP. Looks up a wchar_t string pointed at by stack offset."""
        addr = self.getStackValue(stackNum)
        return _detour.util.readUnicodeZ(addr)
        
    def getConfiguration(self):
        n = ["bytesToPop", "executeOriginal", "originalCodeAddress", "int3_after_call"]
        return dict(zip(n, _detour.getDetourSettings(self.address)))

    def setConfiguration(self, settingsDict):
        n = ["bytesToPop", "executeOriginal", "int3_after_call"]
        p = []
        for x in n:
            p.append(settingsDict[x])
        _detour.setDetourSettings(self.address, p)

    def set_break(self, b=True):
        self.changeConfiguration("int3_after_call", b)

    def changeConfiguration(self, settingname, settingvalue):
        """Helper funtion to change configuration settings on the fly."""
        d = self.getConfiguration()
        d[settingname] = settingvalue
        self.setConfiguration(d)

    def callOriginal(self, *params):
        return self.callOriginalEx(None, None, *params)
        
    #If this was python 3.0, i'd have declared it as `def callOriginalEx(self, *params, registers=None, functiontype=None):`
    def callOriginalEx(self, registers=None, functiontype=None, *params):
        from .common_detours import getProcAddress
    
        if (functiontype == None):
            functiontype = self.detour.config.function_type
        if (registers == None):
            registers = self.registers

        dllname = "_detour_d.pyd"
        if functiontype == "cdecl":
            funcname = dllname + "::call_cdecl_func_with_registers"
            addr = getProcAddress(funcname)
            funct = ctypes.CFUNCTYPE(ctypes.c_long)
        elif functiontype == "stdcall":
            funcname = dllname + "::call_stdcall_func_with_registers"
            addr = getProcAddress(funcname)
            funct = ctypes.WINFUNCTYPE(ctypes.c_long)
        else:
            raise Exception("Unsupported function type %s"%(functiontype))
       
        if len(params) != (self.detour.config.bytes_to_pop / 4):
            raise Exception("Expected %s args, got %s"%(
                (self.detour.config.bytes_to_pop / 4),
                len(params)
            ))
            
        dest_ptr = self.getConfiguration()['originalCodeAddress']
        call_obj = funct(addr)
        
        
        reg = registers._get_ctypes_instance()
        
        #yay for abusing type checking
        at = [type(reg), ctypes.c_long]
        for i in range(0, len(params)):
            at.append(ctypes.c_long)
            
        call_obj.argtypes = tuple(at)

        
        log.debug("!!! calling %s at %#x", funcname, addr)
        log.debug("!!! regs %s", registers)
        log.debug("!!! target at %#x", dest_ptr)
        
        log.debug("!!! params are %s", params)
        log.debug("!!! i think this is a %s func"%(functiontype))
       
        #self.debug_break()
        
        ret = call_obj(reg, dest_ptr, *params)
        log.debug("!!! called, ret = %s"%(ret))
        
        return ret