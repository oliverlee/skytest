"""
Test stdout and return code of a unit test for skytest.
"""

load("@rules_cc//cc:defs.bzl", "cc_binary")

def skytest_test(
        name,
        srcs = [],
        binary = None,
        stdout = [],
        stderr = [],
        return_code = 0,
        copts = []):
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
        Strings to search for in terminal stdout.
      stderr: string_list
        Strings to search for in terminal stderr.
      return_code: int
        Expected return code.
      copts: string_list
        List of strings used with the cc_binary `copts` attribute.
    """

    if srcs and binary:
        fail("`srcs` and `binary` cannot both be set.")

    if not binary:
        cc_binary(
            name = name + "_bin",
            srcs = srcs,
            deps = ["//:skytest"],
            copts = copts,
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
echo "set -euxo pipefail" > $@
echo "" >> $@
echo "binary=$(rootpath {binary})" >> $@
echo "stdout=({stdout})" >> $@
echo "stderr=({stderr})" >> $@
echo "" >> $@
echo "ret=0" >> $@
echo "(\$$binary | tee log.out) 3>&1 1>&2 2>&3 | tee log.err || ret=\$$?" >> $@
echo "" >> $@
echo "[ \$$ret -eq {return_code} ]" >> $@
echo "for line in \"\$${{stdout[@]}}\"; do" >> $@
echo "  grep -a \"\$$line\" log.out" >> $@
echo "done" >> $@
echo "for line in \"\$${{stderr[@]}}\"; do" >> $@
echo "  grep -a \"\$$line\" log.err" >> $@
echo "done" >> $@
""".format(
            binary = binary,
            stdout = " ".join(["'{}'".format(line) for line in stdout]),
            stderr = " ".join(["'{}'".format(line) for line in stderr]),
            return_code = return_code,
        ),
        tags = ["manual"],
        visibility = ["//visibility:private"],
        testonly = True,
    )

    native.sh_test(
        name = name,
        srcs = ["gen_" + name],
        data = [binary],
    )
