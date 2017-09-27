# Gunpowder

**Gunpowder** is an open source software library that provides control dependency analysis and branch distance instrumentation. You can use **Gunpowder** as the execution infrastructure to your automated test data generation technique.

# Simple example

```python
import subprocess
import sys

from gunpowder.ctype import *
from gunpowder import analyze
from gunpowder import Session
from gunpowder import evaluate

NAME = './triangle'
FUNC = 'get_type'
# Instrument target C code
a = analyze.Analyzer(NAME+'.c', '')
control_dep, ways_to_exit = a.instrument(FUNC)
declarations = a.get_function_decl(FUNC)

# Build
proc = subprocess.run([
    'gcc',
    '-fPIC',
    '-Wno-absolute-value',
    '-shared',
    '-o',
    NAME+'.so',
    NAME+'.inst.c',
])
if proc.returncode != 0:
    sys.exit(proc.returncode)

# Run
sess = Session(NAME+'.so', declarations)
args = [
    CTypeInt(),
    CTypeInt(),
    CTypeInt(),
]
trace = sess.run(args, FUNC)
sess.close()

"""Using context manager
with Session(NAME+'.so', declarations) as sess:
    trace = sess.run(...)
"""

print(trace)

# Check the fitness
dep = evaluate.get_dep_chain(control_dep, ways_to_exit, (5, True))
fitness = evaluate.get_fitness(trace, dep)
print(fitness)
```
