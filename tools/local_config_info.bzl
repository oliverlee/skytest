"""
Repository rule for determining Bazel directories.
"""

def _local_config_info_impl(rctx):
    rctx.file("BUILD.bazel")

    external = str(rctx.path(".").realpath).removesuffix("/" + rctx.name)

    env = rctx.execute(["env"]).stdout.splitlines()

    def env_var(pattern):
        values = [
            line.removeprefix(pattern)
            for line in env
            if line.startswith(pattern)
        ]

        if len(values) > 1:
            fail()

        return values[0] if values else None

    bazel_bin = env_var("_=")
    xdg_cache_home = env_var("XDG_CACHE_HOME=")
    home = env_var("HOME=")

    rctx.file(
        "defs.bzl",
        executable = False,
        content = """
BAZEL_BIN = "{}"
BAZEL_EXTERNAL_DIR = "{}"
BAZEL_WORKSPACE_ROOT = "{}"
XDG_CACHE_HOME = "{}"
              """.format(
            bazel_bin,
            external,
            rctx.workspace_root,
            xdg_cache_home or (home + "/.cache"),
        ),
    )

local_config_info = repository_rule(
    implementation = _local_config_info_impl,
    doc = "A repository rule for determining Bazel directories",
)
