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



var llavaSources = [
    "examples/llava/llava.cpp",
    "examples/llava/llava.h",
    "examples/llava/clip.cpp",
    "examples/llava/clip.h",
]

var resources: [Resource] = []
var linkerSettings: [LinkerSetting] = []
var cSettings: [CSetting] = [
    .unsafeFlags(["-Wno-shorten-64-to-32", "-O3", "-DNDEBUG"]),
    .unsafeFlags(["-fno-objc-arc"]),
    .headerSearchPath("ggml/src"),
    .headerSearchPath("common"),
    .unsafeFlags(["-framework", "Foundation"]),  
    .unsafeFlags(["-framework", "Accelerate"]), 
    .unsafeFlags(["-framework", "Metal"]),
    .define("GGML_USE_METAL")
]

#if canImport(Darwin)
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
        .library(name: "llava", targets: ["llava"]),
    ],
    targets: [
        
        .target(
            name: "llama",
            dependencies: [],
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
        ),
        .target(
            name: "common",
            dependencies: ["llama"],
            path: ".",
            sources: [
                "common/arg.cpp",
                "common/arg.h",
            ],
            publicHeadersPath: "spm/common", 
            cSettings: [
            ]
        ),
        .target(
            name: "llava",
            dependencies: ["common", "llama"],
            path: ".",  
            sources: llavaSources,
            resources: resources,
            publicHeadersPath: "spm/llava",
            cSettings: cSettings + [
                .headerSearchPath("common") 
            ],
            linkerSettings: linkerSettings
        ),
    ],
    cxxLanguageStandard: .cxx11
)
