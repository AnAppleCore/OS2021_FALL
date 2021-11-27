##load("@rules_cc//cc:defs.bzl", "cc_test")  #load the test target

cc_library(
    name = "update_optimizer",
    srcs = [
        "update_optimizer.cc",
        ],
    hdrs = [
        "update_optimizer.h",
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
  name = "update_optimizer_test",
  size = "small",
  srcs = ["update_optimizer_test.cc"],
  deps = [
      "@gtest//:gtest_main",
	  ":update_optimizer",
      ],
)
