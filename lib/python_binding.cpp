#include <iostream>
#include <Python.h>
#include "./buildcfg.cpp"

struct Parser
{
  PyObject_HEAD
  CAVM *cavm;
};

static void
Parser_dealloc(Parser *self)
{
  if (self->cavm)
    delete self->cavm;
}

static int
Parser_init(Parser *self, PyObject *args, PyObject *kwds)
{
  const char *filename;

  if (!PyArg_ParseTuple(args, "s", &filename))
    return -1;
  self->cavm = new CAVM(filename);
  return 0;
}

static PyObject *
Parser_instrument(Parser *self, PyObject *args)
{
  const char *functionName;

  if (!PyArg_ParseTuple(args, "s", &functionName))
    return NULL;
  ControlDependency cfg = self->cavm->instrument(functionName);
  PyObject* list = PyList_New(0);
  for (const auto &i : cfg) {
    PyObject *item = PyTuple_New(3);
    PyTuple_SetItem(item, 0, PyLong_FromLong(std::get<0>(i)));
    PyTuple_SetItem(item, 1, PyLong_FromLong(std::get<1>(i)));
    PyTuple_SetItem(item, 2, std::get<2>(i) ? Py_True : Py_False);
    PyList_Append(list, item);
  }

  return list;
}

static PyMethodDef methods[] = {
  {"instrument", (PyCFunction)Parser_instrument, METH_VARARGS, "Instrument given c code."},
  {NULL}
};

static PyType_Slot CAVMTypeSlots[] = {
  {Py_tp_dealloc, (void *)Parser_dealloc},
  {Py_tp_init,    (void *)Parser_init},
  {Py_tp_methods, (void *)methods},
  {0, NULL}
};

static PyType_Spec CAVMTypeSpec = {
  "cavm.Parser",
  sizeof(Parser),
  0,
  Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
  CAVMTypeSlots
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "cavm",
  NULL,
  -1,
  NULL
};

static PyTypeObject *ParserType = NULL;

PyMODINIT_FUNC
PyInit_cavm(void)
{
  ParserType = (PyTypeObject *)PyType_FromSpec(&CAVMTypeSpec);
  if (PyType_Ready(ParserType) < 0)
    return NULL;
  PyObject *m = PyModule_Create(&module);
  if (!m)
    return NULL;
  Py_INCREF(ParserType);
  PyModule_AddObject(m, "Parser", (PyObject *)ParserType);
  return m;
}

