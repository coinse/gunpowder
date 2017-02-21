# Written in 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
# Written in 2017 by Byeonghyeon You <byou@kaist.ac.kr>
"""Ctype
  Type class and factory method
"""

import random


class CType:
    """C type base class"""
    def __init__(self):
        self._min = None
        self._max = None

    def get_min(self):
        """get min"""
        return self._min

    def get_max(self):
        """get max"""
        return self._max

    def is_floating(self):
        """is floating number"""
        return False

    def is_integer(self):
        """is integer"""
        return False


class CTypeChar(CType):
    """C type char class"""
    _repr = ['char', 'signed char']

    def __init__(self):
        super().__init__()
        self._min = -128
        self._max = 127

    def is_integer(self):
        return True


class CTypeUChar(CType):
    """C type unsigned char class"""
    _repr = ['unsigned char']

    def __init__(self):
        super().__init__()
        self._min = 0
        self._max = 255

    def is_integer(self):
        return True


class CTypeShort(CType):
    """C type short class"""
    _repr = ['short', 'signed short', 'short int', 'signed short int']

    def __init__(self):
        super().__init__()
        self._min = -32768
        self._max = 32767

    def is_integer(self):
        return True


class CTypeUShort(CType):
    """C type unsigned short class"""
    _repr = ['unsigned short', 'unsigned short int']

    def __init__(self):
        super().__init__()
        self._min = 0
        self._max = 65535

    def is_integer(self):
        return True


class CTypeInt(CType):
    """C type int class"""
    _repr = ['int', 'signed', 'signed int']

    def __init__(self):
        super().__init__()
        self._min = -2147483648
        self._max = 2147483647

    def is_integer(self):
        return True


class CTypeUInt(CType):
    """C type unsigned int class"""
    _repr = ['unsigned', 'unsigned int']

    def __init__(self):
        super().__init__()
        self._min = 0
        self._max = 4294967295

    def is_integer(self):
        return True


class CTypeLong(CType):
    """C type long class"""
    _repr = ['long', 'signed long', 'long int', 'signed long int']

    def __init__(self):
        super().__init__()
        self._min = -2147483648
        self._max = 2147483637

    def is_integer(self):
        return True


class CTypeULong(CType):
    """C type unsigned long class"""
    _repr = ['unsigned long', 'unsigned long int']

    def __init__(self):
        super().__init__()
        self._min = 0
        self._max = 4294967295

    def is_integer(self):
        return True


class CTypeLongLong(CType):
    """C type long long class"""
    _repr = [
        'long long', 'signed long long', 'long long int',
        'signed long long int'
    ]

    def __init__(self):
        super().__init__()
        self._min = -9223372036854775808
        self._max = 9223372036854775807

    def is_integer(self):
        return True


class CTypeULongLong(CType):
    """C type unsigned long long class"""
    _repr = ['unsigned long long', 'unsigned long long int']

    def __init__(self):
        super().__init__()
        self._min = 0
        self._max = 18446744073709551615

    def is_integer(self):
        return True


class CTypeFloat(CType):
    """C type float class"""
    _repr = ['float']

    def __init__(self):
        super().__init__()
        self._min = float('-inf')
        self._max = float('inf')
        self._precision = 1

    def get_prec(self):
        """get precision"""
        return self._precision

    def set_prec(self, prec):
        """set precision"""
        self._precicision = prec

    def is_floating(self):
        return True


class CTypeDouble(CType):
    """C type double class"""
    _repr = ['double']

    def __init__(self):
        super().__init__()
        self._min = float('-inf')
        self._max = float('inf')
        self._precision = 1

    def get_prec(self):
        """get precision"""
        return self._precision

    def set_prec(self, prec):
        """set precision"""
        self._precicision = prec

    def is_floating(self):
        return True


class CTypeLongDouble(CType):
    """C type long double class"""
    _repr = ['long double']

    def __init__(self):
        super().__init__()
        self._min = float('-inf')
        self._max = float('inf')
        self._precision = 1

    def get_prec(self):
        """get precision"""
        return self._precision

    def set_prec(self, prec):
        """set precision"""
        self._precicision = prec

    def is_floating(self):
        return True


class CTypeBool(CType):
    """C type _Bool class"""
    _repr = ['_Bool']

    def __init__(self):
        super().__init__()
        self._min = 0
        self._max = 1

    def is_integer(self):
        return True


def c_type_factory(raw_type):
    """Factory method of CType"""
    for typeclass in CType.__subclasses__():
        if raw_type in typeclass._repr:
            return typeclass()
    raise NotImplementedError


class CVariable:
    """This class holds Variable and type and bound information"""
    def __init__(self, c_type, minimum=float('-inf'), maximum=float('inf')):
        self._type = c_type
        self._min = max(c_type.get_min(), minimum)
        self._max = min(c_type.get_max(), maximum)
        if self._type.is_integer():
            self._var = random.randint(self._min, self._max)
        elif self._type.is_floating():
            self._var = round(
                random.uniform(self._min, self._max), c_type.get_prec())
        else:
            raise NotImplementedError

    def get_var(self):
        """return vector variable"""
        return self._var

    def get_typeinfo(self):
        """return type, min, and max"""
        return {'type': self._type, 'min': self._min, 'max': self._max}
