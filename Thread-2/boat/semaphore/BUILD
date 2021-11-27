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