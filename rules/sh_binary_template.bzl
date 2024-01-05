"""
Create a sh_binary target from a template
"""

load("@bazel_skylib//rules:expand_template.bzl", "expand_template")

def sh_binary_template(
        name,
        substitutions,
        template = None,
        out = None,
        srcs = [],
        deps = []):
    """
    Combines expand_template with sh_binary

    Args:
      name: string
        Name for `sh_binary_template` rule.
      out: string
        Filename of generated sh_binary src.
      template: string_label
        Input template file.
      substitutions: string_string_dict
        Key-value mappings.
      srcs: string_label_list
        `srcs` used for `sh_binary`
      deps: string_label_list
        `deps` used for `sh_binary`
    """
    if len(srcs) == 1 and template == None:
        template = srcs[0]

    if out == None:
        out = name + ".sh"

    expand_template(
        name = "gen_" + name,
        out = out,
        template = template,
        substitutions = substitutions,
        tags = ["manual"],
        visibility = ["//visibility:private"],
    )

    binary_src = lambda src: src if src != template else out

    native.sh_binary(
        name = name,
        srcs = [binary_src(src) for src in srcs],
        deps = deps,
    )
