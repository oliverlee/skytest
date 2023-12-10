"""
Test stdout and return code of a unit test for skytest.
"""

load("@rules_cc//cc:defs.bzl", "cc_binary")
load(
    "@local_config_info//:defs.bzl",
    "BAZEL_BIN",
    "BAZEL_EXTERNAL_DIR",
    "BAZEL_WORKSPACE_ROOT",
    "XDG_CACHE_HOME",
)
load(":sh_binary_template.bzl", "sh_binary_template")

def skytest_test(
        name,
        srcs = [],
        binary_type = cc_binary,
        malloc = "@bazel_tools//tools/cpp:malloc",
        cxxstd = [17, 20, 23],
        stdout = [],
        stderr = [],
        return_code = 0):
    """
    Runs a skytest unit test and checks the return code and output to stdout.

    Args:
      name: string
        Name for `skytest_test` rule.
      cxxstd: int_list
        List of C++ standard versions to test with.
      binary_type: string
        Type of binary to build

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
echo "for line in \"\$${{stdout[@]}}\"; do" >> $@
echo "  contains \"\$$line\" log.out" >> $@
echo "done" >> $@
echo "for line in \"\$${{stderr[@]}}\"; do" >> $@
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

    tests = []
    for std in [str(std) for std in cxxstd]:
        suffix = "." + std

        if binary_type == cc_binary:
            binary_kwargs = {
                "deps": ["//:skytest"],
                "copts": ["-std=c++" + std],
                "malloc": malloc,
            }
        elif binary_type == sh_binary_template:
            binary_kwargs = {
                "substitutions": {
                    "$BAZEL_BIN": BAZEL_BIN,
                    "$BAZEL_EXTERNAL_DIR": BAZEL_EXTERNAL_DIR,
                    "$BAZEL_WORKSPACE_ROOT": BAZEL_WORKSPACE_ROOT,
                    "$XDG_CACHE_HOME": XDG_CACHE_HOME,
                    "$CC_BINARY_CXXSTD": std,
                    "$CC_BINARY_MALLOC": malloc,
                },
            }
        else:
            binary_kwargs = {}
            fail("unhandled binary_type: {}".format(binary_type))

        binary = name + "_bin" + suffix
        binary_type(
            name = binary,
            srcs = srcs,
            **binary_kwargs
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
