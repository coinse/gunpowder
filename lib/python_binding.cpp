#include <iostream>
#include <Python.h>
#include "./buildcfg.cpp"

static PyObject *
clang_instrument(PyObject *self, PyObject *args)
{
  const char *filename;

  if (!PyArg_ParseTuple(args, "s", &filename))
    return NULL;
  ControlDependency cfg = instrument(filename);
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
  {"instrument", clang_instrument, METH_VARARGS, "Instrument given c code."},
  {NULL, NULL, 0, NULL}
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "clang",
  NULL,
  -1,
  methods
};

PyMODINIT_FUNC
PyInit_clang(void)
{
  return PyModule_Create(&module);
}

