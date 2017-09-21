"""
Copyright (C) 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
Copyright (C) 2017 by Byeonghyeon You <byou@kaist.ac.kr>
Copyright (C) 2017 by Gabin An <agb94@kaist.ac.kr>

Licensed under the MIT License:
See the LICENSE file at the top-level directory of this distribution.
"""

from multiprocessing import Process, Queue
import sys
import weakref
from gunpowder import CAVM_HEADER
from gunpowder import ctype
from cffi import FFI


class Session:
    """A class for running C function"""

    def __init__(self, bin_path, decl_dict, fork=True):
        self.path = bin_path
        self.sandbox = fork
        # Global dict is needed to keep all objects alive.
        # https://cffi.readthedocs.io/en/latest/using.html#working-with-pointers-structures-and-arrays
        self.heap = weakref.WeakKeyDictionary()
        ffi = FFI()
        ffi.cdef("void *malloc(size_t size);")  # necessary for ObjFunc
        for decl in decl_dict:
            ffi.cdef(decl_dict[decl][0])
        with open(CAVM_HEADER, 'r') as f:
            lines = f.readlines()
            ffi.cdef(''.join(lines[21:30]))
        self.ffi = ffi
        self.lib = ffi.dlopen(bin_path)

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()

    def close(self):
        # TODO: Do clean up
        # Free WeakKeyDictionary, dlopen
        pass

    def _malloc(self, c_type, val):
        obj = self.ffi.new(c_type, val)
        size = self.ffi.sizeof(obj[0])
        p = self.lib.malloc(size)
        if p == self.ffi.NULL:
            print("Out of memory")
            sys.exit()
        p = self.ffi.cast(c_type, p)
        self.ffi.memmove(p, obj, size)
        self.heap[p] = val
        return p

    def _make_cffi_input(self, c_input):
        params = []
        for c_type in c_input:
            if isinstance(c_type, ctype.CType):
                params.append(
                    c_type.value.to_bytes(1, 'big', signed=True) if isinstance(
                        c_type, ctype.CTypeChar) else c_type.value)
            elif isinstance(c_type, ctype.CStruct):
                members = self._make_cffi_input(c_type.members)
                params.append(self.ffi.new(c_type.name + '*', members)[0])
            elif isinstance(c_type, ctype.CPointer):
                if c_type.pointee:
                    if isinstance(c_type.pointee, ctype.CType):
                        val = c_type.pointee.value.to_bytes(
                            1, 'big', signed=True) if isinstance(
                                c_type.pointee,
                                ctype.CTypeChar) else c_type.pointee.value
                        p = self._malloc(c_type.underlying_type + '*', val)
                    elif isinstance(c_type.pointee, ctype.CStruct):
                        val = self._make_cffi_input(c_type.pointee.members)
                        p = self._malloc(c_type.underlying_type + '*', val)
                    elif isinstance(c_type.pointee, list):
                        if isinstance(c_type.pointee[0], ctype.CTypeChar):
                            val = b''
                            for x in c_type.pointee:
                                val = val + x.value.to_bytes(
                                    1, 'big', signed=True)
                        else:
                            val = [x.value for x in c_type.pointee]
                        p = self.ffi.new(c_type.underlying_type + '[]', val)
                    self.heap[p] = val
                    params.append(p)
                else:
                    params.append(self.ffi.NULL)
        return params

    def run(self, c_input, target_function, no_trace=False):
        """execute target function"""
        # disable caching
        # inputtuple = tuple(inputvector)
        # if inputtuple in self.dictionary:
        #     return self.dictionary[inputtuple]
        # else:
        cffi_input = self._make_cffi_input(c_input)
        if self.sandbox:

            def sandbox(q, lib, target, inputs):
                c_function = getattr(lib, target)
                c_function(*inputs)
                if no_trace:
                    q.put([])
                else:
                    q.put(self._get_trace(lib))

            q = Queue()
            p = Process(
                target=sandbox,
                args=(q, self.lib, target_function, cffi_input))
            p.start()
            p.join()

            trace = q.get() if p.exitcode == 0 else []
        else:
            c_function = getattr(self.lib, target_function)
            c_function(*cffi_input)
            if no_trace:
                trace = []
            else:
                trace = self._get_trace(self.lib)

        return trace

    def _get_trace(self, dynamic_lib):
        """get trace from dynamic library"""
        trace_list = []
        while True:
            trace = dynamic_lib.get_trace()
            if trace.stmtid == -1:
                break
            else:
                trace_list.append((trace.stmtid, trace.result,
                                   trace.true_distance, trace.false_distance))
        return trace_list
