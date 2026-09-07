// swift-tools-version: 5.9
import PackageDescription
let package = Package(name: "DUBL", platforms: [.macOS(.v13)], products: [.executable(name: "DUBL", targets: ["DUBL"])], targets: [.executableTarget(name: "DUBL")])
