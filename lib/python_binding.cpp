#include <Python.h>
#include "./buildcfg.cpp"

static PyObject *
clang_instrument(PyObject *self, PyObject *args)
{
  const char *filename;
  int sts;

  if (!PyArg_ParseTuple(args, "s", &filename))
    return NULL;
  instrument(filename);
  Py_INCREF(Py_None);
  return Py_None;
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


