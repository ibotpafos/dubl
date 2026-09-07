import Foundation
import AVFoundation
func XCTUnwrap<T>(_ value: T?) throws -> T { guard let value else { throw CocoaError(.fileReadUnknown) }; return value }
func XCTAssertEqual<T: Equatable>(_ lhs: T, _ rhs: T) { precondition(lhs == rhs) }
func XCTAssertFalse(_ value: Bool) { precondition(!value) }
func XCTAssertThrowsError<T>(_ operation: @autoclosure () throws -> T) {
    do { _ = try operation() } catch { return }
    fatalError("Expected batch to fail")
}
@main struct BatchTests {
    static func main() throws {
        let enginePath = try XCTUnwrap(ProcessInfo.processInfo.environment["DUBL_TEST_ENGINE"])
        let engine = URL(fileURLWithPath: enginePath)
        let root = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: false)
        defer { try? FileManager.default.removeItem(at: root) }
        let format = try XCTUnwrap(AVAudioFormat(standardFormatWithSampleRate: 48000, channels: 1))
        var tracks: [URL] = []
        for index in 0..<5 {
            let url = root.appendingPathComponent("voice-\(index).wav")
            let file = try AVAudioFile(forWriting: url, settings: [AVFormatIDKey: kAudioFormatLinearPCM, AVSampleRateKey: 48000, AVNumberOfChannelsKey: 1, AVLinearPCMBitDepthKey: 16, AVLinearPCMIsFloatKey: false])
            let buffer = try XCTUnwrap(AVAudioPCMBuffer(pcmFormat: format, frameCapacity: 24000))
            buffer.frameLength = 24000
            for i in 0..<24000 { buffer.floatChannelData![0][i] = Float(0.2 * sin(2 * Double.pi * Double(220 + index) * Double(i) / 48000)) }
            try file.write(from: buffer)
            tracks.append(url)
        }
        let originals = try tracks.map { try Data(contentsOf: $0) }
        let outputs = try BatchRenderer.render(engine: engine, lead: tracks[0], doubles: Array(tracks.dropFirst()), mode: "natural", directory: root.appendingPathComponent("results"))
        XCTAssertEqual(outputs.count, 4)
        for output in outputs { XCTAssertEqual(try AVAudioFile(forReading: output).length, 24000) }
        XCTAssertEqual(try tracks.map { try Data(contentsOf: $0) }, originals)
        let failed = root.appendingPathComponent("failed")
        XCTAssertThrowsError(try BatchRenderer.render(engine: engine, lead: tracks[0], doubles: [tracks[1], root.appendingPathComponent("missing.wav")], mode: "natural", directory: failed))
        XCTAssertFalse(FileManager.default.fileExists(atPath: failed.path))
        print("PASS: four outputs, exact lengths, unchanged inputs, failed-batch rollback")
    }
}
