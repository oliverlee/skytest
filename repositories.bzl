"""Repository rules for defining skytest dependencies"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def skytest_repositories():
    """Dependencies used in the implementation of `skytest`."""

    OPTIONAL_COMMIT = "3a1209de8370bf5fe16362934956144b49591565"
    maybe(
        http_archive,
        name = "optional",
        integrity = "sha256-RWpswjUXJd5T74jNV2PYI0KudgAn9fIDdAKOCxCQrmI=",
        build_file_content = """
load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "optional",
    hdrs = ["include/tl/optional.hpp"],
    visibility = ["//visibility:public"],
)
""",
        strip_prefix = "optional-{commit}".format(
            commit = OPTIONAL_COMMIT,
        ),
        url = "https://github.com/TartanLlama/optional/archive/{commit}.tar.gz".format(
            commit = OPTIONAL_COMMIT,
        ),
    )

    STRING_VIEW_LITE_COMMIT = "5b1d95fe2c0ee18e654876487898b9a423a954db"
    maybe(
        http_archive,
        name = "string-view-lite",
        integrity = "sha256-Q/3pKstCzz3HyEttZnHYr7NAGk+/EIZ0++Br2VkW29Y=",
        build_file_content = """
load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "string-view-lite",
    hdrs = ["include/nonstd/string_view.hpp"],
    visibility = ["//visibility:public"],
)
""",
        strip_prefix = "string-view-lite-{commit}".format(
            commit = STRING_VIEW_LITE_COMMIT,
        ),
        url = "https://github.com/martinmoene/string-view-lite/archive/{commit}.tar.gz".format(
            commit = STRING_VIEW_LITE_COMMIT,
        ),
    )
