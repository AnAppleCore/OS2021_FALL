##load("@rules_cc//cc:defs.bzl", "cc_test")  #load the test target

cc_binary(
    name = "q0",
    srcs = [
        "q0.cc"
            ],
    deps = [
        "//lib:embedding_lib",
        "//lib:instruction_lib",
        "//lib:utils_lib",
        "//lib:model_lib",
        "//scheduler:scheduler"
    ],
    copts = [
        "-std=c++11",
    ],
    data = glob(["data/q0*"]),
    
)

cc_binary(
    name = "format",
    srcs = [
        "format.cc"
            ],
)

cc_binary(
    name = "q1",
    srcs = [
        "q1.cc"
            ],
            deps = [
                "//lib:embedding_lib",
                "//lib:instruction_lib",
                "//lib:utils_lib",
                "//lib:model_lib",
                "//scheduler:scheduler"
            ],
            copts = [
                "-std=c++11",
            ],
            data = glob(["data/q1*"]),
)

cc_binary(
    name = "q2",
    srcs = [
        "q2.cc"
            ],
            deps = [
                "//lib:embedding_lib",
                "//lib:instruction_lib",
                "//lib:utils_lib",
                "//lib:model_lib",
                "//scheduler:scheduler"
            ],
            copts = [
                "-std=c++11",
            ],
            data = glob(["data/q2*"]),
)

cc_binary(
    name = "q3",
    srcs = [
        "q3.cc"
            ],
            deps = [
                "//lib:embedding_lib",
                "//lib:instruction_lib",
                "//lib:utils_lib",
                "//lib:model_lib",
                "//scheduler:scheduler"
            ],
            copts = [
                "-std=c++11",
            ],
            data = glob(["data/q3*"]),
)

cc_test(
  name = "benchmark",
  size = "small",
  srcs = ["benchmark.cc"],
  deps = [
      "@gbench//:benchmark",
      ],
  copts = [
        "-O3",
        "-std=c++11",
  ],
  data = glob(["*"]),
)

cc_binary(
    name = "q4",
    srcs = [
        "q4.cc"
            ],
    deps = [
        "//lib:embedding_lib",
        "//lib:instruction_lib",
        "//lib:utils_lib",
        "//lib:model_lib",
        "//scheduler:scheduler"
    ],
    copts = [
        "-std=c++11",
    ],
    data = glob(["data/q4*"]),
    
)

cc_test(
  name = "scheduler_test",
  size = "small",
  srcs = ["scheduler_test.cc"],
  deps = [
      "@gtest//:gtest_main",
      "//scheduler:scheduler",
      ],
  data = glob(["data/q3*"]),
)

cc_binary(
    name = "q5",
    srcs = [
        "q5.cc"
            ],
    deps = [
        "//lib:embedding_lib",
        "//lib:instruction_lib",
        "//lib:utils_lib",
        "//lib:model_lib",
        "//q5_lib:q5_optimizer"
    ],
    copts = [
        "-std=c++11",
    ],
    data = glob(["data/q4*"]),
    
)

cc_test(
  name = "q5_optimizer_test",
  size = "small",
  srcs = ["q5_optimizer_test.cc"],
  deps = [
      "@gtest//:gtest_main",
	  "//q5_lib:q5_optimizer",
      ],
  data = glob(["data/q3*"]),
)