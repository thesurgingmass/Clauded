// swift-tools-version:5.9
import PackageDescription

let package = Package(
    name: "OctaClone",
    platforms: [
        .macOS(.v13)
    ],
    targets: [
        .executableTarget(
            name: "OctaClone",
            path: "Sources/OctaClone"
        )
    ]
)
