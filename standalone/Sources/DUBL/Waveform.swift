import SwiftUI
import AVFoundation

struct Waveform: View {
    let url: URL
    @State private var peaks: [Float] = []
    var body: some View {
        Canvas { context, size in
            guard !peaks.isEmpty else { return }
            var path = Path()
            for (index, peak) in peaks.enumerated() {
                let x = CGFloat(index) * size.width / CGFloat(peaks.count)
                let height = CGFloat(peak) * size.height * 0.48
                path.move(to: CGPoint(x: x, y: size.height / 2 - height))
                path.addLine(to: CGPoint(x: x, y: size.height / 2 + height))
            }
            context.stroke(path, with: .color(.mint), lineWidth: 1)
        }.frame(height: 48).task(id: url) {
            peaks = await Task.detached(priority: .utility) {
                guard let file = try? AVAudioFile(forReading: url), file.length > 0,
                      let buffer = AVAudioPCMBuffer(pcmFormat: file.processingFormat, frameCapacity: 8192) else { return [Float]() }
                var bins = [Float](repeating: 0, count: 400)
                var offset: Int64 = 0
                while offset < file.length {
                    do { try file.read(into: buffer) } catch { return [Float]() }
                    guard buffer.frameLength > 0, let samples = buffer.floatChannelData else { break }
                    for i in 0..<Int(buffer.frameLength) {
                        let bin = min(399, Int(Double(offset + Int64(i)) * 400 / Double(file.length)))
                        for channel in 0..<Int(buffer.format.channelCount) {
                            let value = abs(samples[channel][i])
                            if value.isFinite { bins[bin] = min(1, max(bins[bin], value)) }
                        }
                    }
                    offset += Int64(buffer.frameLength)
                }
                return bins
            }.value
        }
    }
}
