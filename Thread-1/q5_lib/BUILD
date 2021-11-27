##load("@rules_cc//cc:defs.bzl", "cc_test")  #load the test target

cc_library(
    name = "q5_optimizer",
    srcs = [
        "q5_optimizer.cc",
        ],
    hdrs = [
        "q5_optimizer.h",
        ],
    deps = [
        "//lib:embedding_lib",
        "//lib:instruction_lib",
        "//lib:model_lib",
    ],
    linkopts = [
        "-pthread"
    ],

	visibility = [
		"//visibility:public",
	],
)
