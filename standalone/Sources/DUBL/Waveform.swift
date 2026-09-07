import SwiftUI
import AVFoundation

struct Waveform: View {
    let url: URL
    let color: Color
    @State private var peaks: [Float] = []

    var body: some View {
        Canvas { context, size in
            guard !peaks.isEmpty else { return }
            let center = size.height / 2
            var path = Path()
            for (index, peak) in peaks.enumerated() {
                let x = CGFloat(index) * size.width / CGFloat(peaks.count)
                let height = max(0.75, CGFloat(peak) * size.height * 0.44)
                path.move(to: CGPoint(x: x, y: center - height))
                path.addLine(to: CGPoint(x: x, y: center + height))
            }
            context.stroke(path, with: .color(color), lineWidth: 1)
        }
        .frame(minHeight: 48)
        .task(id: url) {
            peaks = await Task.detached(priority: .utility) {
                guard let file = try? AVAudioFile(forReading: url), file.length > 0,
                      let buffer = AVAudioPCMBuffer(pcmFormat: file.processingFormat, frameCapacity: 8192) else { return [Float]() }
                var bins = [Float](repeating: 0, count: 500)
                var offset: Int64 = 0
                while offset < file.length {
                    do { try file.read(into: buffer) } catch { return [Float]() }
                    guard buffer.frameLength > 0, let samples = buffer.floatChannelData else { break }
                    for i in 0..<Int(buffer.frameLength) {
                        let bin = min(bins.count - 1, Int(Double(offset + Int64(i)) * Double(bins.count) / Double(file.length)))
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
