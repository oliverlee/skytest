"""
Rule for generating and testing log files for a binary's output
"""

load("@bazel_skylib//rules:diff_test.bzl", "diff_test")

def binary_log(
        name,
        src,
        log):
    """
    Create a log file from running a binary.

    Args:
      name: string
        Name for `binary_log` rule
      src: string_label
        Source binary file to run
      log: string
        Filename for created log.
    """
    native.genrule(
        name = name,
        srcs = [src],
        outs = [log],
        cmd = "$(execpath {}) > $@ || true".format(src),
        testonly = True,
        tags = ["manual"],
        visibility = ["//visibility:private"],
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
    )

    label = native.package_relative_label(name)
    log = log or src_name + ".log"

    native.genrule(
        name = name + "_update_sh",
        srcs = [src, log],
        outs = [name + ".update.sh"],
        cmd = """
set -euo pipefail
echo "set -euo pipefail" > $@
echo "" >> $@
echo "cd \\$${{BUILD_WORKSPACE_DIRECTORY}}" >> $@
echo "$(execpath {src}) > $(rootpath {log}) || true" >> $@
""".format(
            src = src,
            log = log,
        ),
        tags = ["manual"],
        visibility = ["//visibility:private"],
    )

    native.sh_binary(
        name = name + ".update",
        srcs = [name + ".update.sh"],
    )

    diff_test(
        name = name,
        file1 = log,
        file2 = generated_log,
        failure_message = "To update, run:\n\nbazel run {}.update".format(
            str(label).replace("@//", "//"),
        ),
    )
