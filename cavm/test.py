import unittest
import subprocess

def run_cmd(popen_cmd):
    proc = subprocess.Popen(popen_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    proc_stdout, _ = proc.communicate()
    return proc.returncode, proc_stdout

class TestCAVMCheck(unittest.TestCase):
    def test_triangle(self):
        cmd = ['python3', 'cavm/main.py', 'sample/triangle.c', '-f', 'get_type', '--min', '1']
        rc, stdout = run_cmd(cmd)
        print('triangle')
        self.assertTrue(b"[0, False]" in stdout)
        self.assertTrue(b"[0, True]" in stdout)
        self.assertTrue(b"[1, False]" in stdout)
        self.assertTrue(b"[1, True]" in stdout)
        self.assertTrue(b"[2, False]" in stdout)
        self.assertTrue(b"[2, True]" in stdout)
        self.assertTrue(b"[3, False]" in stdout)
        self.assertTrue(b"[3, True]" in stdout)
        self.assertTrue(b"[4, False]" in stdout)
        self.assertTrue(b"[4, True]" in stdout)
        self.assertTrue(b"[5, False]" in stdout)
        self.assertTrue(b"[5, True]" in stdout)
        self.assertTrue(b"[6, False]" in stdout)
        self.assertTrue(b"[6, True]" in stdout)
        self.assertTrue(b"[7, False]" in stdout)
        self.assertTrue(b"[7, True]" in stdout)
        self.assertTrue(b"[0, False] FAIL" not in stdout)
        self.assertTrue(b"[0, True] FAIL" not in stdout)
        self.assertTrue(b"[1, False] FAIL" not in stdout)
        self.assertTrue(b"[1, True] FAIL" not in stdout)
        self.assertTrue(b"[2, False] FAIL" not in stdout)
        self.assertTrue(b"[2, True] FAIL" not in stdout)
        self.assertTrue(b"[3, False] FAIL" not in stdout)
        self.assertTrue(b"[3, True] FAIL" not in stdout)
        self.assertTrue(b"[4, False] FAIL" not in stdout)
        self.assertTrue(b"[4, True] FAIL" not in stdout)
        self.assertTrue(b"[5, False] FAIL" not in stdout)
        self.assertTrue(b"[5, True] FAIL" not in stdout)
        self.assertTrue(b"[6, False] FAIL" not in stdout)
        self.assertTrue(b"[6, True] FAIL" in stdout)
        self.assertTrue(b"[7, False] FAIL" not in stdout)
        self.assertTrue(b"[7, True] FAIL" not in stdout)
    def test_case1(self):
        cmd = ['python3', 'cavm/main.py', 'sample/case1.c', '-f', 'case1']
        rc, stdout = run_cmd(cmd)
        print('case1')
        self.assertTrue(b"[0, False]" in stdout)
        self.assertTrue(b"[0, True]" in stdout)
        self.assertTrue(b"[1, False]" in stdout)
        self.assertTrue(b"[1, True]" in stdout)
        self.assertTrue(b"[0, False] FAIL" not in stdout)
        self.assertTrue(b"[0, True] FAIL" not in stdout)
        self.assertTrue(b"[1, False] FAIL" not in stdout)
        self.assertTrue(b"[1, True] FAIL" not in stdout)
    def test_case3(self):
        cmd = ['python3', 'cavm/main.py', 'sample/case3.c', '-f', 'case3']
        rc, stdout = run_cmd(cmd)
        print('case3')
        self.assertTrue(b"[0, False]" in stdout)
        self.assertTrue(b"[0, True]" in stdout)
        self.assertTrue(b"[0, False] FAIL" not in stdout)
        self.assertTrue(b"[0, True] FAIL" not in stdout)
    def test_case5(self):
        cmd = ['python3', 'cavm/main.py', 'sample/case5.c', '-f', 'case5']
        rc, stdout = run_cmd(cmd)
        print('case5')
        self.assertTrue(b"[0, False]" in stdout)
        self.assertTrue(b"[0, True]" in stdout)
        self.assertTrue(b"[0, False] FAIL" not in stdout)
        self.assertTrue(b"[0, True] FAIL" not in stdout)

if __name__ == '__main__':
    unittest.main()
