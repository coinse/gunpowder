# Written in 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
# Written in 2017 by Byeonghyeon You <byou@kaist.ac.kr>
"""Ctype
  Type class and factory method
"""

import random


class CType:
    """C type base class"""

    def __init__(self):
        self.value = 0
        self.__max = type(self)._max
        self.__min = type(self)._min

    def get_min(self):
      """get min"""
      return self.__min

    def get_max(self):
      """get max"""
      return self.__max

    def set_min(self, value):
      """set min"""
      if value < type(self)._min or value > self.__max:
        raise ValueError
      self.__min = value

    def set_max(self, value):
      """set max"""
      if value > type(self)._max or value < self.__min:
        raise ValueError
      self.__max = value

    def is_floating(self):
        """is floating number"""
        return False

    def is_integer(self):
        """is integer"""
        return False


class CTypeChar(CType):
    """C type char class"""
    _repr = ['char', 'signed char']
    _min = -128
    _max = 127

    def __init__(self):
        super().__init__()

    def is_integer(self):
        return True


class CTypeUChar(CType):
    """C type unsigned char class"""
    _repr = ['unsigned char']
    _min = 0
    _max = 255

    def __init__(self):
        super().__init__()

    def is_integer(self):
        return True


class CTypeShort(CType):
    """C type short class"""
    _repr = ['short', 'signed short', 'short int', 'signed short int']
    _min = -32768
    _max = 32767

    def __init__(self):
        super().__init__()

    def is_integer(self):
        return True


class CTypeUShort(CType):
    """C type unsigned short class"""
    _repr = ['unsigned short', 'unsigned short int']
    _min = 0
    _max = 65535

    def __init__(self):
        super().__init__()

    def is_integer(self):
        return True


class CTypeInt(CType):
    """C type int class"""
    _repr = ['int', 'signed', 'signed int']
    _min = -2147483648
    _max = 2147483647

    def __init__(self):
        super().__init__()

    def is_integer(self):
        return True


class CTypeUInt(CType):
    """C type unsigned int class"""
    _repr = ['unsigned', 'unsigned int']
    _min = 0
    _max = 4294967295

    def __init__(self):
        super().__init__()

    def is_integer(self):
        return True


class CTypeLong(CType):
    """C type long class"""
    _repr = ['long', 'signed long', 'long int', 'signed long int']
    _min = -2147483648
    _max = 2147483637

    def __init__(self):
        super().__init__()

    def is_integer(self):
        return True


class CTypeULong(CType):
    """C type unsigned long class"""
    _repr = ['unsigned long', 'unsigned long int']
    _min = 0
    _max = 4294967295

    def __init__(self):
        super().__init__()

    def is_integer(self):
        return True


class CTypeLongLong(CType):
    """C type long long class"""
    _repr = [
        'long long', 'signed long long', 'long long int',
        'signed long long int'
    ]
    _min = -9223372036854775808
    _max = 9223372036854775807

    def __init__(self):
        super().__init__()

    def is_integer(self):
        return True


class CTypeULongLong(CType):
    """C type unsigned long long class"""
    _repr = ['unsigned long long', 'unsigned long long int']
    _min = 0
    _max = 18446744073709551615

    def __init__(self):
        super().__init__()

    def is_integer(self):
        return True


class CTypeFloat(CType):
    """C type float class"""
    _repr = ['float']
    _min = float('-inf')
    _max = float('inf')

    def __init__(self):
        super().__init__()
        self.precision = 1

    def is_floating(self):
        return True


class CTypeDouble(CType):
    """C type double class"""
    _repr = ['double']
    _min = float('-inf')
    _max = float('inf')

    def __init__(self):
        super().__init__()
        self.precision = 1

    def is_floating(self):
        return True


class CTypeLongDouble(CType):
    """C type long double class"""
    _repr = ['long double']
    _min = float('-inf')
    _max = float('inf')

    def __init__(self):
        super().__init__()
        self.precision = 1

    def is_floating(self):
        return True


class CTypeBool(CType):
    """C type _Bool class"""
    _repr = ['_Bool']
    _min = 0
    _max = 1

    def __init__(self):
        super().__init__()

    def is_integer(self):
        return True


def c_type_factory(raw_type):
    """Factory method of CType"""
    for typeclass in CType.__subclasses__():
        if raw_type in typeclass._repr:
            return typeclass()
    raise NotImplementedError
