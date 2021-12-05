##load("@rules_cc//cc:defs.bzl", "cc_test")  #load the test target

cc_library(
    name = "utils_lib",
    srcs = [
        "utils.cc",
        ],
    hdrs = [
        "utils.h",
        ],
	visibility = [
		"//visibility:public",
	],
)

cc_library(
    name = "memory_manager_lib",
    srcs = [
        "memory_manager.cc",
        "array_list.cc",
        ],
    hdrs = [
        "memory_manager.h",
        "array_list.h",
        ],
    deps = [
        "semaphore",
    ],
	visibility = [
		"//visibility:public",
	],
)

cc_library(
    name = "semaphore",
    srcs = [
        "semaphore.cc",
        ],
    hdrs = [
        "semaphore.h",
        ],
	visibility = [
		"//visibility:public",
	],
    copts = [
        "-O3",
        "-std=c++11",
    ],
	linkopts = [
        "-pthread",
    ],
)

