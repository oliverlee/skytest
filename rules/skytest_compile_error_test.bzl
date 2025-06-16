"""
Test stderr of an expected compilation failure with skytest.
"""

load("@rules_build_error//lang/cc:defs.bzl", "cc_build_error_test")
load("@rules_build_error//matcher:defs.bzl", "matcher")

def skytest_compile_error_test(*, name, src, stderr = None, cxxstd = [17, 20, 23]):
    """
    Builds a skytest unit test and checks that a compilation error occurs.

    Args:
      name: string
        Name for `skytest_test` rule.
      cxxstd: int_list
        List of C++ standard versions to test with.
      src: string
        Source used to build a C++ binary target.
      stderr: string_list or dict(config_string, string_list)
        Strings to search for in terminal stderr.
    """
    if type(stderr) == "list":
        stderr = {"//conditions:default": stderr}

    tests = []
    for std in [str(std) for std in cxxstd]:
        suffix = "." + std

        cc_build_error_test(
            name = name + suffix,
            src = src,
            copts = [
                "-std=c++" + std,
                "-fno-diagnostics-color",
            ],
            compile_stderr = matcher.contains_basic_regex(select({
                config: ("(.|\n)*").join(patterns)
                for (config, patterns) in stderr.items()
            })),
            deps = ["//:skytest"],
        )
        tests.append(name + suffix)

    native.test_suite(
        name = name,
        tests = tests,
    )
