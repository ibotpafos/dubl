import SwiftUI
import AppKit
import AVFoundation
import UniformTypeIdentifiers

@MainActor final class Session: ObservableObject {
    @Published var lead: URL?
    @Published var doubles: [URL] = []
    @Published var mode = "natural"
    @Published var busy = false
    @Published var status = "Выберите лид и дубль: mono WAV, 44.1 или 48 kHz."
    @Published var results: [URL] = []
    private var player: AVAudioPlayer?
    private var ensemble: [AVAudioPlayer] = []

    func choose(leadTrack: Bool) {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [.wav]
        panel.allowsMultipleSelection = !leadTrack
        if panel.runModal() == .OK {
            if leadTrack { lead = panel.url } else { doubles = panel.urls }
            results = []
            stop()
        }
    }
    func play(_ url: URL?) {
        guard let url else { return }
        do { stop(); player = try AVAudioPlayer(contentsOf: url); player?.play() }
        catch { status = error.localizedDescription }
    }
    func stop() { player?.stop(); ensemble.forEach { $0.stop() }; ensemble = [] }
    func playAll(processed: Bool) {
        let tracks = (lead.map { [$0] } ?? []) + (processed ? results : doubles)
        guard !tracks.isEmpty else { return }
        do {
            stop()
            ensemble = try tracks.map { try AVAudioPlayer(contentsOf: $0) }
            let gain = 1.0 / Float(ensemble.count)
            ensemble.forEach { $0.volume = gain; $0.prepareToPlay() }
            let time = ensemble[0].deviceCurrentTime + 0.15
            ensemble.forEach { $0.play(atTime: time) }
        } catch { stop(); status = error.localizedDescription }
    }
    func align() {
        guard let lead, !doubles.isEmpty, !busy else { return }
        let tracks = doubles
        let engine = Bundle.main.bundleURL.appendingPathComponent("Contents/MacOS/dubl_render")
        guard FileManager.default.isExecutableFile(atPath: engine.path) else {
            status = "Не найден движок dubl_render. Запустите собранный DUBL.app."
            return
        }
        busy = true
        results = []
        stop()
        status = "Анализ и выравнивание…"
        let selectedMode = mode
        Task {
            let outcome: Result<[URL], Error> = await Task.detached {
                do {
                    let directory = FileManager.default.temporaryDirectory.appendingPathComponent("dubl-" + UUID().uuidString)
                    try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
                    var outputs: [URL] = []
                    for (index, double) in tracks.enumerated() {
                    let output = directory.appendingPathComponent("\(index + 1)-\(double.deletingPathExtension().lastPathComponent)-aligned.wav")
                    let process = Process()
                    process.executableURL = engine
                    process.arguments = ["--lead", lead.path, "--double", double.path, "--output", output.path, "--report", directory.appendingPathComponent("report-\(index).json").path, "--mode", selectedMode]
                    let errors = Pipe()
                    process.standardError = errors
                    try process.run()
                    let data = errors.fileHandleForReading.readDataToEndOfFile()
                    process.waitUntilExit()
                    guard process.terminationStatus == 0 else {
                        throw NSError(domain: "DUBL", code: Int(process.terminationStatus), userInfo: [NSLocalizedDescriptionKey: String(data: data, encoding: .utf8) ?? "Ошибка обработки"])
                    }
                    outputs.append(output)
                    }
                    return .success(outputs)
                } catch { return .failure(error) }
            }.value
            busy = false
            switch outcome {
            case .success(let urls): results = urls; status = "Готово: \(urls.count) дублей. Сравните результаты перед сохранением."
            case .failure(let error): status = error.localizedDescription
            }
        }
    }
    func save() {
        guard let result = results.first else { return }
        let panel = NSSavePanel()
        panel.nameFieldStringValue = "DUBL-results"
        guard panel.runModal() == .OK, let target = panel.url else { return }
        do {
            guard !FileManager.default.fileExists(atPath: target.path) else {
                status = "Выберите новое имя: существующие файлы не перезаписываются."
                return
            }
            try FileManager.default.copyItem(at: result.deletingLastPathComponent(), to: target)
            status = "Сохранено: " + target.lastPathComponent
        } catch { status = error.localizedDescription }
    }
}

struct Content: View {
    @StateObject private var session = Session()
    var body: some View {
        VStack(alignment: .leading, spacing: 22) {
            Text("ДУБЛЬ").font(.system(size: 38, weight: .bold, design: .rounded))
            Text("Выравнивание вокальных дублей · локально на Mac").foregroundStyle(.secondary)
            Group {
                Button("Лид: " + (session.lead?.lastPathComponent ?? "выбрать WAV")) { session.choose(leadTrack: true) }
                Button("Выбрать дубли (\(session.doubles.count))") { session.choose(leadTrack: false) }
                Picker("Режим", selection: $session.mode) {
                    Text("Natural").tag("natural")
                    Text("Tight").tag("tight")
                    Text("Locked").tag("locked")
                }.pickerStyle(.segmented)
            }.disabled(session.busy)
            ScrollView {
                ForEach(Array(session.doubles.enumerated()), id: \.offset) { index, url in
                    HStack {
                        Text(url.lastPathComponent).lineLimit(1)
                        Spacer()
                        Button("Оригинал") { session.play(url) }
                        Button("Результат") { session.play(session.results[index]) }
                            .disabled(index >= session.results.count)
                    }
                }
            }.frame(height: 120)
            Button(session.busy ? "Обработка…" : "Выровнять всё") { session.align() }
                .buttonStyle(.borderedProminent).controlSize(.large)
                .disabled(session.busy || session.lead == nil || session.doubles.isEmpty)
            HStack {
                Button("Все до") { session.playAll(processed: false) }.disabled(session.doubles.isEmpty)
                Button("Все после") { session.playAll(processed: true) }.disabled(session.results.isEmpty)
                Button("Стоп") { session.stop() }
                Spacer()
                Button("Сохранить все WAV") { session.save() }.disabled(session.results.isEmpty)
            }
            Text(session.status).font(.callout).textSelection(.enabled)
            Text("Прототип: тайминг и общая коррекция питча. Качество на реальном вокале ещё проверяется.")
                .font(.caption).foregroundStyle(.secondary)
        }.padding(32).frame(minWidth: 650, minHeight: 420)
    }
}

@main struct DublApp: App {
    var body: some Scene { WindowGroup { Content() } }
}
