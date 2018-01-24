"""
Copyright (C) 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
Copyright (C) 2017 by Byeonghyeon You <byou@kaist.ac.kr>
Copyright (C) 2017 by Gabin An <agb94@kaist.ac.kr>

Licensed under the MIT License:
See the LICENSE file at the top-level directory of this distribution.
"""

import shutil
from os import path

from gunpowder import CAVM_HEADER, STRCMP2
from gunpowder import clang


class Analyzer:
    """A class to instrument C function"""

    def __init__(self, file_path, cflags):
        if not path.isfile(file_path):
            raise FileNotFoundError("No such file: '{}'".format(file_path))
        self.path = file_path
        self.parser = clang.Parser(file_path, cflags)

    def instrument(self, target_function):
        if target_function:
            cfg = self.parser.instrument(target_function)
            dest = path.dirname(self.path)
            shutil.copy(CAVM_HEADER, dest)
            shutil.copy(STRCMP2, dest)
        else:
            ## Raise Exception here
            raise SystemExit('no function')
        return self._get_dep_map(cfg)

    def _get_dep_map(self, dep_list):
        """get dependecy map from list"""
        dep_map = {}
        wte = {}
        for i in dep_list:
            # i is (target_branch_id, parent_bid, condition)
            if i[0] > 0:
                dep_map[i[0]] = tuple((i[1], i[2]))
            elif i[0] == i[1]:
                wte[-i[0]] = [tuple((-i[1], i[2]))]
        for b in reversed(sorted(dep_map)):
            if b not in wte:
                # initialize
                wte[b] = []
            dep_size = len(wte[b])
            while True:
                for k, v in dep_map.items():
                    if v[0] == b or v[0] in list(map(lambda x: x[0], wte[b])):
                        wte[b] += filter(
                            lambda x, idx=b: x not in wte[idx], wte[k])
                if len(wte[b]) > dep_size:
                    dep_size = len(wte[b])
                else:
                    break
        return dep_map, wte

    def get_function_decl(self, target_function):
        decl, params = self.parser.get_decl(target_function)
        decl_dict = self._get_decl_dict(params)
        decl_dict[target_function] = (decl, params)
        return decl_dict

    def _get_decl(self, type_name, decl_dict):
        if not type_name in decl_dict:
            if type_name[-1:] == '*':
                return self._get_decl(type_name[:-1].strip(), decl_dict)
            elif type_name[:6] == 'struct':
                decl, members = self.parser.get_decl(type_name[6:].strip())
                decl_dict[type_name] = (decl, members)
                for member in members:
                    self._get_decl(member, decl_dict)

    def _get_decl_dict(self, parameters):
        decl_dict = {}
        for param in parameters:
            self._get_decl(param, decl_dict)
        return decl_dict
