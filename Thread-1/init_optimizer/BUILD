##load("@rules_cc//cc:defs.bzl", "cc_test")  #load the test target

cc_library(
    name = "init_optimizer",
    srcs = [
        "init_optimizer.cc",
        ],
    hdrs = [
        "init_optimizer.h",
        ],
    deps = [
        "//lib:embedding_lib",
        "//lib:model_lib"
    ],
    linkopts = [
        "-pthread"
    ],

	visibility = [
		"//visibility:public",
	],
)

cc_test(
  name = "init_optimizer_test",
  size = "small",
  srcs = ["init_optimizer_test.cc"],
  deps = [
      "@gtest//:gtest_main",
	  ":init_optimizer",
      ],
)