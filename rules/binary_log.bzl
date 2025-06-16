"""
Rule for generating and testing log files for a binary's output
"""

load("@bazel_skylib//rules:diff_test.bzl", "diff_test")
load("@rules_build_error//lang/cc:defs.bzl", "CcBuildErrorInfo")

def _log_from_build_error(ctx):
    output = ctx.actions.declare_file(ctx.attr.log)
    input = ctx.attr.src[CcBuildErrorInfo].compile_stderr

    # remove lines with platform-specific information
    # e.g.
    # In file included from bazel-out/darwin_arm64-fastbuild/bin/_virtual_includes/skytest/skytest/skytest.hpp:12:
    ctx.actions.run_shell(
        inputs = [input],
        outputs = [output],
        command = "grep -v '^In file included from' {input} > {output}".format(
            input = input.path,
            output = output.path,
        ),
        progress_message = "Generating log from {}".format(ctx.attr.src.label),
    )

    return [DefaultInfo(files = depset([output]))]

def _log_from_binary(ctx):
    output = ctx.actions.declare_file(ctx.attr.log)
    bin = ctx.attr.src.files.to_list()[0]

    ctx.actions.run_shell(
        inputs = [bin],
        outputs = [output],
        command = "{bin} &> {output} || true".format(
            bin = bin.path,
            output = output.path,
        ),
        progress_message = "Generating log from {}".format(ctx.attr.src.label),
    )

    return [DefaultInfo(files = depset([output]))]

def _binary_log_impl(ctx):
    if CcBuildErrorInfo in ctx.attr.src:
        return _log_from_build_error(ctx)
    return _log_from_binary(ctx)

binary_log = rule(
    implementation = _binary_log_impl,
    attrs = {
        "src": attr.label(mandatory = True),
        "log": attr.string(mandatory = True),
    },
    provides = [DefaultInfo],
)

def synchronized_binary_log(
        name,
        src,
        log = None):
    """
    Test that a log file generated from running a binary is up to date.

    Args:
      name: string
        Name for `synchronized_binary_log` rule
      src: string_label
        Source binary file to run
      log: string_label
        Checked in log file.
    """
    src_name = native.package_relative_label(src).name
    generated_log = src_name + ".log.generated"
    binary_log(
        name = name + "_gen",
        src = src,
        log = generated_log,
        tags = ["manual"],
        visibility = ["//visibility:private"],
        target_compatible_with = select({
            # log files will differ for Clang and GCC due to how types are
            # printed.
            "@rules_cc//cc/compiler:gcc": ["@platforms//:incompatible"],
            "//conditions:default": [],
        }),
    )

    log = log or src_name + ".log"

    diff_test(
        name = name,
        file1 = log,
        file2 = name + "_gen",
        failure_message = "To update, run:\n\nbazel run {}.update".format(
            str(native.package_relative_label(name)).replace("@//", "//"),
        ),
    )

    native.sh_binary(
        name = name + ".update",
        srcs = [Label("copy_to_workspace.bash")],
        data = [
            log,
            name + "_gen",
        ],
        args = [
            "$(rootpath :{})".format(name + "_gen"),
            "$(rootpath :{})".format(log),
        ],
    )
