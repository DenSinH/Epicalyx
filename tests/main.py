import random
import os
import subprocess
import re


class Generator:

    INTEGRAL_TYPES = {
        "int",
        "long long",
        "short",
        "char",
    }

    FLOAT_TYPES = {
        "float",
        "double",
    }

    STATEMENTS = {
        "decl",
        "assignment",
        "crement",
    }

    EXPRESSION_TYPES = {
        "integral",
        "float",
    }

    EXPRESSIONS = {
        "unop",
        "binop",
    }

    NUMBER_BOUND = 100

    def __init__(self):
        self.vars = []

    def has_vars(self):
        return len(self.vars) >= 2

    def max_var_idx(self):
        return len(self.vars) - 2

    def rand_var_idx(self):
        return random.randint(0, self.max_var_idx())

    def rand_bool(self):
        return bool(random.getrandbits(1))

    def generate_expression(self, depth, type):
        if depth == 0:
            if self.rand_bool() or not self.has_vars():
                if type == "integral":
                    return str(random.randint(-Generator.NUMBER_BOUND, Generator.NUMBER_BOUND))
                else:
                    return str(Generator.NUMBER_BOUND * random.uniform(-1, 1))
            else:
                if type == "integral":
                    if self.has_vars():
                        var = self.rand_var_idx()
                        for i in range(var, self.max_var_idx() + 1):
                            if self.vars[i] in Generator.INTEGRAL_TYPES:
                                if self.rand_bool():
                                    return f"v{i}"
                                elif self.rand_bool():
                                    return f"v{i}++"
                                elif self.rand_bool():
                                    return f"v{i}--"
                                elif self.rand_bool():
                                    return f"++v{i}"
                                else:
                                    return f"--v{i}"
                    # no integral variable found
                    return str(random.randint(-Generator.NUMBER_BOUND, Generator.NUMBER_BOUND))
                else:
                    var = self.rand_var_idx()
                    if self.rand_bool():
                        return f"v{var}"
                    elif self.rand_bool():
                        return f"v{var}++"
                    elif self.rand_bool():
                        return f"v{var}--"
                    elif self.rand_bool():
                        return f"++v{var}"
                    else:
                        return f"--v{var}"

        expr_type = random.choice(list(Generator.EXPRESSIONS))
        if expr_type == "binop":
            left = self.generate_expression(depth - 1, type)
            if type == "integral":
                op = random.choice([
                    "+", "-", "*", "/", "%", "|", "^", "&", ">>", "<<"
                ])
            else:
                op = random.choice([
                    "+", "-", "*", "/",
                ])

            if op == "/" or op == "%":
                # prevent division by zero
                right = str(random.randint(1, 20))
            elif op == ">>" or op == "<<":
                right = str(random.randint(1, 7))
            else:
                right = self.generate_expression(depth - 1, type)

            return f"({left}) {op} ({right})"
        elif expr_type == "unop":
            right = self.generate_expression(depth - 1, type)
            if type == "integral":
                op = random.choice(["-", "+", "~"])
                return f"{op}({right})"
            else:
                op = random.choice(["-", "+"])
                return f"{op}({right})"
        else:
            return "0"

    def generate_statement(self):
        type = random.choice(list(Generator.STATEMENTS))
        if type == "decl":
            result = ""
            if self.rand_bool():
                var = random.choice(list(Generator.INTEGRAL_TYPES))
                if self.rand_bool():
                    result += "unsigned "
                elif self.rand_bool():
                    result += "signed "
            else:
                var = random.choice(list(Generator.FLOAT_TYPES))
            result += f"{var}"

            amount = random.randint(1, 3)
            for i in range(amount):
                self.vars.append(var)
                result += f" v{len(self.vars) - 1}"
                # with initializer
                result += " = "
                if self.rand_bool():
                    if self.vars[-1] in Generator.INTEGRAL_TYPES:
                        result += f"{{ { self.generate_expression(random.randint(0, 4), 'integral') } }}"
                    else:
                        result += f"{{ { self.generate_expression(random.randint(0, 4), 'float') } }}"
                else:
                    if self.vars[-1] in Generator.INTEGRAL_TYPES:
                        result += f"{ self.generate_expression(random.randint(0, 4), 'integral') }"
                    else:
                        result += f"{ self.generate_expression(random.randint(0, 4), 'float') }"
                if i < amount - 1:
                    result += ","
            return result + ";"
        elif type == "assignment":
            if not self.has_vars():
                return ""
            var = random.randint(0, self.max_var_idx())
            expr_depth = random.randint(2, 4)
            type = random.choice(list(Generator.EXPRESSION_TYPES))
            return f"v{var} = {self.generate_expression(expr_depth, type)};"
        elif type == "crement":
            if not self.has_vars():
                return ""
            var = random.randint(0, self.max_var_idx())
            if self.rand_bool():
                return f"v{var};"
            elif self.rand_bool():
                return f"v{var}++;"
            elif self.rand_bool():
                return f"v{var}--;"
            elif self.rand_bool():
                return f"++v{var};"
            else:
                return f"--v{var};"
        else:
            return ""

    def generate_main(self):
        try:
            result = "int main() {"
            for i in range(random.randint(10, 20)):
                result += "\n    " + self.generate_statement()
            result += "\n    return " + " + ".join(f"v{i}" for i in random.sample(range(len(self.vars)), 3)) + ";"
            result += "\n}"
            return result
        except Exception:
            return "int main() { return 0; }"


if __name__ == "__main__":
    for i in range(100):
        with open("test.c", "w+") as f:
            f.write(Generator().generate_main())
        print("running test", i)
        subprocess.check_output("clang -O0 test.c -o test.exe")
        proc0 = subprocess.run("test")
        subprocess.check_output("clang -O2 test.c -o test.exe")
        proc2 = subprocess.run("test")
        if proc0.returncode != proc2.returncode:
            print("Bad clang output")
            continue

        epicalyx = subprocess.run(r".\cmake-build-debug\epicalyx.exe", stdout=subprocess.PIPE, cwd=os.path.abspath("../"), shell=True)
        result = int(re.findall(r"return (.*)", epicalyx.stdout.decode("utf-8"))[-1])
        print("expect", proc0.returncode)
        if result < 0:
            result = 0x1_0000_0000 + result

        print("got", result)
        if result != proc0.returncode:
            raise Exception("Bad output!")