// Copyright 2017 COINSE Lab.
#include <Python.h>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include "./buildcfg.cpp"

struct Parser {
  PyObject_HEAD std::string filename;
};

static void Parser_dealloc(Parser *self) {}

static int Parser_init(Parser *self, PyObject *args, PyObject *kwds) {
  const char *filename;

  if (!PyArg_ParseTuple(args, "s", &filename)) return -1;
  self->filename = filename;
  return 0;
}

static PyObject *Parser_instrument(Parser *self, PyObject *args) {
  const char *functionName;

  if (!PyArg_ParseTuple(args, "s", &functionName)) return NULL;
  ControlDependency cfg = instrument(self->filename, functionName);
  PyObject *list = PyList_New(0);
  for (const auto &i : cfg) {
    PyObject *item = PyTuple_New(3);
    PyTuple_SetItem(item, 0, PyLong_FromLong(std::get<0>(i)));
    PyTuple_SetItem(item, 1, PyLong_FromLong(std::get<1>(i)));
    PyTuple_SetItem(item, 2, std::get<2>(i) ? Py_True : Py_False);
    PyList_Append(list, item);
  }

  return list;
}

static PyObject *Parser_getDecl(Parser *self, PyObject *args) {
  const char *functionName;

  if (!PyArg_ParseTuple(args, "s", &functionName)) return NULL;
  std::tuple<std::string, std::vector<std::string>> ret =
      getDeclaration(self->filename, functionName);

  PyObject *tp = PyTuple_New(2);
  PyTuple_SetItem(tp, 0, PyUnicode_FromString(std::get<0>(ret).c_str()));

  PyObject *params = PyList_New(0);
  for (const auto &i : std::get<1>(ret))
    PyList_Append(params, PyUnicode_FromString(i.c_str()));
  PyTuple_SetItem(tp, 1, params);

  return tp;
}

static PyObject *Parser_getFunctions(Parser *self) {
  std::vector<std::string> ret = getFunctions(self->filename);

  PyObject *f = PyList_New(0);
  for (const auto &i : ret)
    PyList_Append(f, PyUnicode_FromString(i.c_str()));

  return f;
}

static PyMethodDef methods[] = {
    {"instrument", (PyCFunction)Parser_instrument, METH_VARARGS,
     "Instrument given c code."},
    {"get_decl", (PyCFunction)Parser_getDecl, METH_VARARGS,
     "Get declaration given function."},
    {"get_functions", (PyCFunction)Parser_getFunctions, METH_NOARGS,
     "Get list of functions."},
    {NULL}};

static PyType_Slot CAVMTypeSlots[] = {
    {Py_tp_dealloc, reinterpret_cast<void *>(Parser_dealloc)},
    {Py_tp_init, reinterpret_cast<void *>(Parser_init)},
    {Py_tp_methods, reinterpret_cast<void *>(methods)},
    {0, NULL}};

static PyType_Spec CAVMTypeSpec = {"cavm.clang.Parser", sizeof(Parser), 0,
                                   Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
                                   CAVMTypeSlots};

static struct PyModuleDef module = {PyModuleDef_HEAD_INIT, "cavm.clang", NULL,
                                    -1, NULL};

static PyTypeObject *ParserType = NULL;

PyMODINIT_FUNC PyInit_clang(void) {
  ParserType = reinterpret_cast<PyTypeObject *>(PyType_FromSpec(&CAVMTypeSpec));
  if (PyType_Ready(ParserType) < 0) return NULL;
  PyObject *m = PyModule_Create(&module);
  if (!m) return NULL;
  Py_INCREF(ParserType);
  PyModule_AddObject(m, "Parser", reinterpret_cast<PyObject *>(ParserType));
  return m;
}
