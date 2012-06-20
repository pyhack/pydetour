from detour_callback_object import DetourCallbackObject
import logging
log = logging.getLogger(__name__)

class _DetourManager(dict):
    """DetourManager is a class that managers a group of detours. It's keys are addresses, and it's values are Detour instances"""
    def __init__(self):
        log.warning("Error to see this more than once")

    def do_callback(self, address, registers, caller):
        """
            This function gets slightly massaged arguments from the initial dispatcher.
            It's job is to call the function set as the callback for this particular detour.
        """
        if address not in self:
            raise Exception("Detour of function at %#x... doesn't exist...?"%(address))
            
        detour = self[address]
            
        if callable(detour.config.callback):
            obj = None
            try:
                obj = detour.config.callback_class(detour, registers, caller)
                if obj is None:
                    obj = DetourCallbackObject(detour, registers, caller)
                detour.config.callback(obj)
            except Exception, e:
                import traceback
                log.exception("Exception in callback for function at address 0x%08x:\n"%(address))
                traceback.print_exc()
                print ""
                if getattr(detour.config.callback, "debug_on_exception", False):
                    import pdb
                    import sys
                    pdb.post_mortem(sys.exc_info()[2])
                    #raise e
            try:
                obj.applyRegisters()
            except LookupError: #could have removed the detour from inside the callback function
                pass
try:
    DetourManager
except Exception:
    DetourManager = _DetourManager()