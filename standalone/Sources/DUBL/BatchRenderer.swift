import Foundation

enum BatchRenderer {
    static func render(engine: URL, lead: URL, doubles: [URL], mode: String, directory: URL) throws -> [URL] {
        guard !doubles.isEmpty, ["natural", "tight", "locked"].contains(mode) else {
            throw NSError(domain: "DUBL", code: 1, userInfo: [NSLocalizedDescriptionKey: "Выберите дубли и режим обработки"])
        }
        // Require a new directory to keep existing data outside rollback scope.
        guard !FileManager.default.fileExists(atPath: directory.path) else {
            throw CocoaError(.fileWriteFileExists)
        }
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: false)
        do {
            var outputs: [URL] = []
            for (index, double) in doubles.enumerated() {
                let output = directory.appendingPathComponent("\(index + 1)-\(double.deletingPathExtension().lastPathComponent)-aligned.wav")
                let process = Process()
                process.executableURL = engine
                process.arguments = ["--lead", lead.path, "--double", double.path, "--output", output.path, "--report", directory.appendingPathComponent("report-\(index).json").path, "--mode", mode]
                let errors = Pipe()
                process.standardError = errors
                try process.run()
                let data = errors.fileHandleForReading.readDataToEndOfFile()
                process.waitUntilExit()
                guard process.terminationStatus == 0 else {
                    throw NSError(domain: "DUBL", code: Int(process.terminationStatus), userInfo: [NSLocalizedDescriptionKey: "\(double.lastPathComponent): " + (String(data: data, encoding: .utf8) ?? "Ошибка обработки")])
                }
                outputs.append(output)
            }
            return outputs
        } catch {
            try? FileManager.default.removeItem(at: directory)
            throw error
        }
    }
}
