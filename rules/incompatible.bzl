"""
Defines a rule disabling targets if sanitizers are enabled.

https://bazel.build/extending/platforms#skipping-incompatible-targets
"""

def _set_incompatible_flag_impl(settings, attrs):
    using_sanitizer = any([
        feat in settings["//command_line_option:features"]
        for feat in attrs.sanitizers
    ])

    return {"//tools:incompatible_flag": using_sanitizer}

_set_incompatible_flag = transition(
    implementation = _set_incompatible_flag_impl,
    inputs = ["//command_line_option:features"],
    outputs = ["//tools:incompatible_flag"],
)

def _impl(ctx):
    return [
        cc_common.merge_cc_infos(
            cc_infos = [dep[CcInfo] for dep in ctx.attr.deps],
        ),
    ]

incompatible_with_sanitizers = rule(
    implementation = _impl,
    attrs = {
        "deps": attr.label_list(
            mandatory = True,
            providers = [CcInfo],
            cfg = _set_incompatible_flag,
        ),
        "_toolchain": attr.label(
        ),
        "sanitizers": attr.string_list(
            default = ["asan", "tsan", "ubsan"],
        ),
        "compilers": attr.string_list(
            default = ["gcc", "clang"],
        ),
        "_allowlist_function_transition": attr.label(
            default = "@bazel_tools//tools/allowlists/function_transition_allowlist",
        ),
    },
    doc = ("Target wrapper that becomes incompatible if sanitizers are enabled."),
)
