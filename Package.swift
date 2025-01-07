// swift-tools-version:5.5

import PackageDescription

var sources = [
    "src/llama.cpp",
    "src/llama-vocab.cpp",
    "src/llama-grammar.cpp",
    "src/llama-sampling.cpp",
    "src/unicode.cpp",
    "src/unicode-data.cpp",
    "ggml/src/ggml.c",
    "ggml/src/ggml-aarch64.c",
    "ggml/src/ggml-alloc.c",
    "ggml/src/ggml-backend.cpp",
    "ggml/src/ggml-backend-reg.cpp",
    "ggml/src/ggml-cpu/ggml-cpu.c",
    "ggml/src/ggml-cpu/ggml-cpu.cpp",
    "ggml/src/ggml-cpu/ggml-cpu-aarch64.c",
    "ggml/src/ggml-cpu/ggml-cpu-quants.c",
    "ggml/src/ggml-threading.cpp",
    "ggml/src/ggml-quants.c",
]

var resources: [Resource] = []
var linkerSettings: [LinkerSetting] = []
var cSettings: [CSetting] = [
    .unsafeFlags(["-Wno-shorten-64-to-32", "-O3", "-DNDEBUG"]),
    .unsafeFlags(["-fno-objc-arc"]),
    .headerSearchPath("ggml/src"),
    .unsafeFlags(["-framework", "Foundation"]),  // Ensures Foundation is linked
    .unsafeFlags(["-framework", "Accelerate"]), // Link Accelerate if used
    .unsafeFlags(["-framework", "Metal"]), // Link Metal if used
    .define("GGML_USE_ACCELERATE"),
    .define("GGML_USE_METAL")
]

#if canImport(Darwin)
sources.append("ggml/src/ggml-metal/ggml-metal.mm")  // Use Objective-C++ (.mm) file
resources.append(.process("ggml/src/ggml-metal/ggml-metal.metal"))
linkerSettings.append(.linkedFramework("Accelerate"))
cSettings.append(contentsOf: [
    .define("GGML_USE_ACCELERATE"),
    .define("GGML_USE_METAL")
])
#endif

#if os(Linux)
    cSettings.append(.define("_GNU_SOURCE"))
#endif

let package = Package(
    name: "llama",
    platforms: [
        .macOS(.v12),
        .iOS(.v14),
        .watchOS(.v4),
        .tvOS(.v14)
    ],
    products: [
        .library(name: "llama", targets: ["llama"]),
    ],
    targets: [
        .target(
            name: "llama",
            path: ".",
            exclude: [
                "build",
                "cmake",
                "examples",
                "scripts",
                "models",
                "tests",
                "CMakeLists.txt",
                "Makefile",
                "ggml/src/ggml-metal-embed.metal"
            ],
            sources: sources,
            resources: resources,
            publicHeadersPath: "spm-headers",
            cSettings: cSettings,
            linkerSettings: linkerSettings
        )
    ],
    cxxLanguageStandard: .cxx11
)
