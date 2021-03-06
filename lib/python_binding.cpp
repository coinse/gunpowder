/*
 * Copyright (C) 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
 * Copyright (C) 2017 by Byeonghyeon You <byou@kaist.ac.kr>
 * Copyright (C) 2017 by Simon Bihel <simon.bihel@ens-rennes.fr>
 *
 * Licensed under the MIT License:
 * See the LICENSE file at the top-level directory of this distribution.
 */

#include <Python.h>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "Cavm.h"
#include "firstRound.h"

struct Parser {
  PyObject_HEAD Cavm *cavm;
};

static void Parser_dealloc(Parser *self) {
  if (self->cavm)
    delete self->cavm;
}

static int Parser_init(Parser *self, PyObject *args, PyObject *kwds) {
  const char *filename, *s;
  std::vector<const char *> opts;
  PyObject *obj, *iterator, *item;

  if (!PyArg_ParseTuple(args, "sO", &filename, &obj))
    return -1;
  iterator = PyObject_GetIter(obj);
  if (iterator == NULL)
    return -1;

  opts.push_back("clang");
  opts.push_back("input.c");
  opts.push_back("--");
  while ((item = PyIter_Next(iterator))) {
    s = PyUnicode_AsUTF8(item);
    opts.push_back(s);
    Py_DECREF(item);
  }
  Py_DECREF(iterator);

  firstRound(filename);
  std::string newfilename(filename);
  newfilename.append("_first_round.c");
  self->cavm = new Cavm(newfilename, opts);
  return 0;
}

static PyObject *Parser_instrument(Parser *self, PyObject *args) {
  const char *functionName;

  if (!PyArg_ParseTuple(args, "s", &functionName))
    return NULL;
  ControlDependency cfg = self->cavm->instrument(functionName);
  PyObject *list = PyList_New(0);
  for (const auto &i : cfg) {
    PyObject *item = PyTuple_New(3);
    PyTuple_SetItem(item, 0, PyLong_FromLong(i.first));
    PyTuple_SetItem(item, 1, PyLong_FromLong((i.second).first));
    PyTuple_SetItem(item, 2, (i.second).second ? Py_True : Py_False);
    PyList_Append(list, item);
  }

  return list;
}

static PyObject *Parser_getDecl(Parser *self, PyObject *args) {
  const char *functionName;

  if (!PyArg_ParseTuple(args, "s", &functionName))
    return NULL;
  std::tuple<std::string, std::vector<std::string>> ret =
      self->cavm->getDeclaration(functionName);

  PyObject *tp = PyTuple_New(2);
  PyTuple_SetItem(tp, 0, PyUnicode_FromString(std::get<0>(ret).c_str()));

  PyObject *params = PyList_New(0);
  for (const auto &i : std::get<1>(ret))
    PyList_Append(params, PyUnicode_FromString(i.c_str()));
  PyTuple_SetItem(tp, 1, params);

  return tp;
}

static PyObject *Parser_printFunctions(Parser *self) {
  self->cavm->printFunctions();
  return Py_None;
}

static PyMethodDef methods[] = {
    {"instrument", (PyCFunction)Parser_instrument, METH_VARARGS,
     "Instrument given c code."},
    {"get_decl", (PyCFunction)Parser_getDecl, METH_VARARGS,
     "Get declaration given function."},
    {"print_functions", (PyCFunction)Parser_printFunctions, METH_NOARGS,
     "Get list of functions."},
    {}};

static PyType_Slot CAVMTypeSlots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *>(Parser_dealloc)},
    {Py_tp_init, reinterpret_cast<void *>(Parser_init)},
    {Py_tp_methods, reinterpret_cast<void *>(methods)},
    {0, NULL}};

static PyType_Spec CAVMTypeSpec = {"gunpowder.clang.Parser", sizeof(Parser), 0,
                                   Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
                                   CAVMTypeSlots};

static struct PyModuleDef module = {PyModuleDef_HEAD_INIT, "gunpowder.clang", NULL,
                                    -1, NULL, NULL, NULL, NULL, NULL};

static PyTypeObject *ParserType = NULL;

PyMODINIT_FUNC PyInit_clang(void) {
  ParserType = reinterpret_cast<PyTypeObject *>(PyType_FromSpec(&CAVMTypeSpec));
  if (PyType_Ready(ParserType) < 0)
    return NULL;
  PyObject *m = PyModule_Create(&module);
  if (!m)
    return NULL;
  Py_INCREF(ParserType);
  PyModule_AddObject(m, "Parser", reinterpret_cast<PyObject *>(ParserType));
  return m;
}
