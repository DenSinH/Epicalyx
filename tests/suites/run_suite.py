import subprocess
import os
import sys
from typing import NamedTuple, List


class Test(NamedTuple):
    file: str
    cmd: List[str]
    proc: subprocess.Popen


def run_tests(base_command, root, output_file, error_file):
    output = open(output_file, "w+")
    errors = open(error_file, "w+", newline='')
    
    tests = []
    for file in os.listdir(root):
        if not file.endswith(".c"):
            continue
        
        print(f"Running test {file}...")
        cmd = [*base_command, os.path.abspath(os.path.join(root, file)).replace("\\", "/")]
        proc = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        tests.append(Test(
            file=file,
            cmd=cmd,
            proc=proc
        ))
    
    passed = 0
    failed = 0
    unimplemented = 0
    for test in tests:
        stdout, stderr = test.proc.communicate()
        print(f"{test.file: <30} {test.proc.returncode}", file=output, flush=True)
        print(f"{test.file: <30} {test.proc.returncode}")
        if test.proc.returncode or stderr:
            print("=" * 50, file=errors)
            print(f"When testing {test.file}", file=errors)
            print(f"Command \"{' '.join(test.cmd)}\"", file=errors)
            err = stderr.decode("utf-8", errors="ignore").strip()
            if err.lower().startswith("unimplemented"):
                unimplemented += 1
            print(err)
            print(err, file=errors, flush=True)
            failed += 1
        else:
            passed += 1
    print("=" * 50, file=output)
    print(f"Total tests:   {passed + failed}", file=output)
    print(f"Passed:        {passed}", file=output)
    print(f"Failed:        {failed}", file=output)
    print(f"Unimplemented: {unimplemented}", file=output)
    print("=" * 50, file=output, flush=True)
    


if __name__ == "__main__":
    _, epicalyx_path, suite_root, output_file, error_file = sys.argv 
    run_tests(
        [epicalyx_path, "-novisualize", "-catch-errors"],
        suite_root,
        output_file,
        error_file
    )
