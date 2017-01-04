import clang
import argparse
from subprocess import Popen, PIPE

parser = argparse.ArgumentParser(description='Instrument and execute given c file')
parser.add_argument('target', metavar='file', type=str, help='location of target c file')
args = parser.parse_args()

clang.instrument(args.target)
print(args.target[:-2])

with Popen(['gcc', args.target[:-2]+'.inst.c', '-o', args.target[:-2]], stdout=PIPE) as proc:
    print(proc.stdout.read())

with Popen([args.target[:-2]], stdout=PIPE) as proc:
    with open('trace', 'r') as f:
      print(f.read())
