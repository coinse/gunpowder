import random

class CType:
    def __init__(self):
        self._name = None
        self._min = None
        self._max = None
        self._step = 1

    def get_name(self):
        return self._name
    
    def get_min(self):
        return self._min

    def get_max(self):
        return self._max

    def get_step(self):
        return self._step

    def set_min(self, val):
        self._min = max(__self._min, val)

    def set_max(self, val):
        self._max = min(__self._max, val)

    def set_step(self, step):
        self._step = step

    def is_floating(self):
        return False

    def is_integer(self):
        return False

class CTypeChar(CType):
    _name = 'char'
    _repr = ['char', 'signed char']
    def __init__(self):
        self._min = -128
        self._max = 127

    def is_integer(self):
        return True

class CTypeUChar(CType):
    _name = 'unsigned char'
    _repr = ['unsigned char']
    def __init__(self):
        self._min = 0
        self._max = 255

    def is_integer(self):
        return True

class CTypeShort(CType):
    _name = 'short'
    _repr = ['short', 'signed short', 'short int', 'signed short int']
    def __init__(self):
        self._min = -32768
        self._max = 32767

    def is_integer(self):
        return True

class CTypeUShort(CType):
    _name = 'unsigned short'
    _repr = ['unsigned short', 'unsigned short int']
    def __init__(self):
        self._min = 0
        self._max = 65535

    def is_integer(self):
        return True

class CTypeInt(CType):
    _name = 'int'
    _repr = ['int', 'signed', 'signed int']
    def __init__(self):
        self._min = -2147483648
        self._max = 2147483647

    def is_integer(self):
        return True

class CTypeUInt(CType):
    _name = 'unsigned int'
    _repr = ['unsigned', 'unsigned int']
    def __init__(self):
        self._min = 0
        self._max = 4294967295

    def is_integer(self):
        return True

class CTypeLong(CType):
    _name = 'long'
    _repr = ['long', 'signed long', 'long int', 'signed long int']
    def __init__(self):
        self._min = -2147483648
        self._max = 2147483637

    def is_integer(self):
        return True

class CTypeULong(CType):
    _name = 'unsigned long'
    _repr = ['unsigned long', 'unsigned long int']
    def __init__(self):
        self._min = 0
        self._max = 4294967295

    def is_integer(self):
        return True

class CTypeLongLong(CType):
    _name = 'long long'
    _repr = ['long long', 'signed long long', 'long long int',
            'signed long long int']
    def __init__(self):
        self._min = -9223372036854775808
        self._max = 9223372036854775807

    def is_integer(self):
        return True

class CTypeULongLong(CType):
    _name = 'unsigned long long'
    _repr = ['unsigned long long', 'unsigned long long int']
    def __init__(self):
        self._min = 0
        self._max = 18446744073709551615

    def is_integer(self):
        return True

class CTypeFloat(CType):
    _name = 'float'
    _repr = ['float']
    def __init__(self):
        self._min = float('-inf')
        self._max = float('inf')

    def get_prec(self):
        return self._precision

    def set_prec(self, prec):
        self._precicision = prec
        set_step(10 ** -prec)

    def is_floating(self):
        return True

class CTypeDouble(CType):
    _name = 'double'
    _repr = ['double']
    def __init__(self):
        self._min = float('-inf')
        self._max = float('inf')
        self._precision = 1

    def get_prec(self):
        return self._precision

    def set_prec(self, prec):
        self._precicision = prec
        set_step(10 ** -prec)

    def is_floating(self):
        return True

class CTypeLongDouble(CType):
    _name = 'long double'
    _repr = ['long double']
    def __init__(self):
        self._min = float('-inf')
        self._max = float('inf')
        self._precision = 1

    def get_prec(self):
        return self._precision

    def set_prec(self, prec):
        self._precicision = prec
        set_step(10 ** -prec)

    def is_floating(self):
        return True

class CTypeBool(CType):
    _name = '_Bool'
    _repr = ['_Bool']
    def __init__(self):
        self._min = 0
        self._max = 1
        self._precision = 1

    def is_integer(self):
        return True

def c_type_factory(raw_type):
    for type in CType.__subclasses__():
        if raw_type in type._repr:
            return type()
    else:
        raise NotImplementedError


class CVariable:
    def __init__(self, c_type, minimum = float('-inf'), maximum = float('inf')):
        self._type = c_type
        self._min = max(c_type.get_min(), minimum)
        self._max = min(c_type.get_max(), maximum)
        if self._type.is_integer():
            self._var = random.randint(self._min, self._max)
        elif self._type.is_floating():
            self._var = round(random.uniform(self._min, self._max), c_type.get_prec())
        else:
            raise NotImplementedError

    def get_var(self):
        return self._var
