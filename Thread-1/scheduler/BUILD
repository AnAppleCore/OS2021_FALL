##load("@rules_cc//cc:defs.bzl", "cc_test")  #load the test target

cc_library(
    name = "scheduler",
    srcs = [
        "scheduler.cc",
        ],
    hdrs = [
        "scheduler.h",
        ],
    deps = [
        "//lib:embedding_lib",
        "//lib:instruction_lib",
        "//lib:model_lib",
        "//update_optimizer:update_optimizer",
        "//init_optimizer:init_optimizer"
    ],
    linkopts = [
        "-pthread"
    ],

	visibility = [
		"//visibility:public",
	],
)
