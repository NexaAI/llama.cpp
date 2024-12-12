// swift-tools-version:5.5

import PackageDescription

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
        .library(name: "llava", targets: ["llava"]),  // 添加 llava 库
    ],
    targets: [
        .systemLibrary(name: "llama", pkgConfig: "llama"),
        .target(
            name: "llava",
            dependencies: ["llama"],  // 依赖 llama
            path: "examples/llava",   // 指定 llava 源码路径
            exclude: [
                "android",           // 排除不需要的文件和目录
                "llava-cli.cpp"
            ],
            sources: [              // 指定源文件
                "llava.cpp",
                "clip.cpp"
            ],
            cSettings: [            // C 编译设置
                .headerSearchPath("."),
                .headerSearchPath("../."),
                .headerSearchPath("../../common"),
                .headerSearchPath("../../")
            ],
            cxxSettings: [          // C++ 编译设置
                .unsafeFlags(["-std=c++11"])
            ]
        )
    ]
)
