"""
Test stdout and return code of a unit test for skytest.
"""

load("@rules_cc//cc:defs.bzl", "cc_binary")

def skytest_test(
        name,
        srcs = [],
        binary = None,
        stdout = [],
        return_code = 0):
    """
    Runs a skytest unit test and checks the return code and output to stdout.

    Args:
      name: string
        Name for `skytest_test` rule.
      srcs: string_list
        Executable sources used to build a C++ binary target. Cannot be used with `binary`.
      binary: string_label
        C++ binary target to use. Cannot be used with `srcs`.
      stdout: string_list
        Strings to search for in terminal output.
      return_code: int
        Expected return code.
    """

    if srcs and binary:
        fail("`srcs` and `binary` cannot both be set.")

    if not binary:
        cc_binary(
            name = name + "_bin",
            srcs = srcs,
            deps = ["//:skytest"],
        )
        binary = name + "_bin"

    # args are passed to a genrule instead of directly to an sh_test to avoid
    # arg tokenization
    #
    # see:
    # https://github.com/bazelbuild/bazel/issues/11587
    # https://github.com/bazelbuild/bazel/issues/12313
    # https://bazel.build/reference/be/common-definitions#sh-tokenization
    native.genrule(
        name = "gen_" + name,
        outs = ["gen_" + name + ".sh"],
        tools = [binary],
        cmd = r"""
set -euo pipefail
echo "set -euo pipefail" > $@
echo "" >> $@
echo "binary=$(rootpath {binary})" >> $@
echo "stdout=({stdout})" >> $@
echo "" >> $@
echo "ret=0" >> $@
echo "\$$binary 1> log.out || ret=\$$?" >> $@
echo "" >> $@
echo "[ \$$ret -eq {return_code} ]" >> $@
echo "for line in \$${{stdout[@]}}; do" >> $@
echo "  grep -q \"\$$line\" log.out" >> $@
echo "done" >> $@
""".format(
            binary = binary,
            stdout = " ".join(["'{}'".format(line) for line in stdout]),
            return_code = return_code,
        ),
    )

    native.sh_test(
        name = name,
        srcs = ["gen_" + name],
        data = [binary],
    )
