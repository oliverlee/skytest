"""
Test stdout and return code of a unit test for skytest.
"""

load("@rules_cc//cc:defs.bzl", "cc_binary")

def skytest_test(
        name,
        srcs = [],
        malloc = "@bazel_tools//tools/cpp:malloc",
        cxxstd = [17, 20, 23],
        stdout = [],
        stderr = [],
        return_code = 0,
        **kwargs):
    """
    Runs a skytest unit test and checks the return code and output to stdout.

    Args:
      name: string
        Name for `skytest_test` rule.
      cxxstd: int_list
        List of C++ standard versions to test with.

      srcs: string_list
        Executable sources used to build a C++ binary target.
      malloc: string_label
        malloc library to use.

      stdout: string_list
        Strings to search for in terminal stdout.
      stderr: string_list
        Strings to search for in terminal stderr.
      return_code: int
        Expected return code.

      **kwargs: dict
        additional args passed to the underlying binary
    """

    # args are passed to a genrule instead of directly to an sh_test to avoid
    # arg tokenization
    #
    # see:
    # https://github.com/bazelbuild/bazel/issues/11587
    # https://github.com/bazelbuild/bazel/issues/12313
    # https://bazel.build/reference/be/common-definitions#sh-tokenization
    gentest = "gen_" + name
    native.genrule(
        name = gentest,
        outs = ["gen_" + name + ".sh"],
        cmd = r"""
set -euo pipefail
echo "set -euo pipefail" > $@
echo "" >> $@
echo "OPTS=\$$([[ \"\$${{VERBOSE:-0}}\" -eq 1 ]] && echo \"-a\" || echo \"-aq\")" >> $@
echo "[[ \"\$${{VERBOSE:-0}}\" -eq 1 ]] && set -x || true" >> $@
echo "" >> $@
echo "binary=\"\$$1\"" >> $@
echo "stdout=({stdout})" >> $@
echo "stderr=({stderr})" >> $@
echo "" >> $@
echo "contains () {{" >> $@
echo "  grep \"\$$OPTS\" \"\$$1\" \"\$$2\" || (echo ERROR: \\\"\"\$$line\"\\\" not found && false)" >> $@
echo "}}" >> $@
echo "" >> $@
echo "ret=0" >> $@
echo "(\$$binary | tee log.out) 3>&1 1>&2 2>&3 | tee log.err || ret=\$$?" >> $@
echo "" >> $@
echo "[ \$$ret -eq {return_code} ]" >> $@
echo "for line in \"\$${{stdout[@]+\"\$${{stdout[@]}}\"}}\"; do" >> $@
echo "  contains \"\$$line\" log.out" >> $@
echo "done" >> $@
echo "for line in \"\$${{stderr[@]+\"\$${{stderr[@]}}\"}}\"; do" >> $@
echo "  contains \"\$$line\" log.err" >> $@
echo "done" >> $@
""".format(
            stdout = " ".join(["'{}'".format(line) for line in stdout]),
            stderr = " ".join(["'{}'".format(line) for line in stderr]),
            return_code = return_code,
        ),
        tags = ["manual"],
        visibility = ["//visibility:private"],
        testonly = True,
    )

    deps = kwargs.pop("deps", []) + ["//:skytest"]
    copts = kwargs.pop("copts", [])

    tests = []
    for std in [str(std) for std in cxxstd]:
        suffix = "." + std
        binary = name + "_bin" + suffix

        cc_binary(
            name = binary,
            srcs = srcs,
            deps = deps,
            copts = copts + ["-std=c++" + std],
            malloc = malloc,
            **kwargs
        )

        native.sh_test(
            name = name + suffix,
            srcs = [gentest],
            data = [binary],
            args = ["$(rootpath {})".format(binary)],
        )

        tests.append(name + suffix)

    native.test_suite(
        name = name,
        tests = tests,
    )
