import random

class CType:
    def __init__(self):
        self._name = None
        self._min = None
        self._max = None
        self._step = 1
        self._precision = 0

    def get_name(self):
        return self._name
    
    def get_min(self):
        return self._min

    def get_max(self):
        return self._max

    def get_step(self):
        return self._step

    def get_prec(self):
        return self._precision

    def set_min(self, val):
        self._min = max(__self._min, val)

    def set_max(self, val):
        self._max = min(__self._max, val)

    def set_step(self, step):
        self._step = step

    def set_prec(self, prec):
        self._precicision = prec
        set_step(10 ** -prec)

    def is_floating(self):
        return False

    def is_integer(self):
        return False

class CTypeChar(CType):
    def __init__(self):
        self._name = 'char'
        self._min = -128
        self._max = 127

    def is_integer(self):
        return True

class CTypeUChar(CType):
    def __init__(self):
        self._name = 'unsigned char'
        self._min = 0
        self._max = 255

    def is_integer(self):
        return True

class CTypeShort(CType):
    def __init__(self):
        self._name = 'short'
        self._min = -32768
        self._max = 32767

    def is_integer(self):
        return True

class CTypeUShort(CType):
    def __init__(self):
        self._name = 'unsigned short'
        self._min = 0
        self._max = 65535

    def is_integer(self):
        return True

class CTypeInt(CType):
    def __init__(self):
        self._name = 'int'
        self._min = -2147483648
        self._max = 2147483647

    def is_integer(self):
        return True

class CTypeUInt(CType):
    def __init__(self):
        self._name = 'unsigned int'
        self._min = 0
        self._max = 4294967295

    def is_integer(self):
        return True

class CTypeLong(CType):
    def __init__(self):
        self._name = 'long'
        self._min = -2147483648
        self._max = 2147483637

    def is_integer(self):
        return True

class CTypeULong(CType):
    def __init__(self):
        self._name = 'unsigned long'
        self._min = 0
        self._max = 4294967295

    def is_integer(self):
        return True

class CTypeLongLong(CType):
    def __init__(self):
        self._name = 'long long'
        self._min = -9223372036854775808
        self._max = 9223372036854775807

    def is_integer(self):
        return True

class CTypeULongLong(CType):
    def __init__(self):
        self._name = 'unsigned long long'
        self._min = 0
        self._max = 18446744073709551615

    def is_integer(self):
        return True

class CTypeFloat(CType):
    def __init__(self):
        self._name = 'float'
        self._min = float('-inf')
        self._max = float('inf')

    def is_floating(self):
        return True

class CTypeDouble(CType):
    def __init__(self):
        self._name = 'double'
        self._min = float('-inf')
        self._max = float('inf')

    def is_floating(self):
        return True

class CTypeLongDouble(CType):
    def __init__(self):
        self._name = 'long double'
        self._min = float('-inf')
        self._max = float('inf')

    def is_floating(self):
        return True

class CTypeBool(CType):
    def __init__(self):
        self._name = '_Bool'
        self._min = 0
        self._max = 1

    def is_integer(self):
        return True

def c_type_factory(raw_type):
    if raw_type == 'void':
        raise NotImplementedError
    elif raw_type in ['char', 'signed char']:
        return CTypeChar()
    elif raw_type in ['unsigned char']:
        return CTypeUChar()
    elif raw_type in ['short', 'signed short', 'short int', 'signed short int']:
        return CTypeShort()
    elif raw_type in ['unsigned short', 'unsigned short int']:
        return CTypeUShort()
    elif raw_type in ['int', 'signed', 'signed int']:
        return CTypeInt()
    elif raw_type in ['unsigned', 'unsigned int']:
        return CTypeUInt()
    elif raw_type in ['long', 'signed long', 'long int', 'signed long int']:
        return CTypeLong()
    elif raw_type in ['unsigned long', 'unsigned long int']:
        return CTypeULong()
    elif raw_type in [
            'long long', 'signed long long', 'long long int',
            'signed long long int']:
        return CTypeLongLong()
    elif raw_type in ['unsigned long long', 'unsigned long long int']:
        return CTypeULongLong()
    elif raw_type == 'float':
        return CTypeFloat()
    elif raw_type == 'double':
        return CTypeDouble()
    elif raw_type == 'long double':
        return CTypeLongDouble()
    elif raw_type == '_Bool':
        return CTypeBool()
    else:
        raise NotImplementedError


class CVariable:
    def __init__(self, c_type):
        self._var = None
        self._type = c_type
        self._min = c_type.get_min()
        self._max = c_type.get_max()

    def set_min(self, minimum):
        self._min = max(self._min, minimum)

    def set_max(self, maximum):
        self._max = min(self._max, maximum)

    def get_var(self):
        return self._var
 
    def init_random(self):
        if self._type.is_integer():
            self._var = random.randint(self._min, self._max)
        elif self._type.is_floating():
            self._var = round(random.uniform(self._min, self._max), c_type.get_prec())
        else:
            raise NotImplementedError
