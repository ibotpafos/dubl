import SwiftUI
import AppKit
import AVFoundation
import UniformTypeIdentifiers

@MainActor final class Session: ObservableObject {
    @Published var lead: URL?
    @Published var doubles: [URL] = []
    @Published var mode = "natural"
    @Published var busy = false
    @Published var status = "Добавьте две или больше вокальных дорожек в формате WAV."
    @Published var results: [URL] = []
    @Published var selected: Set<URL> = []
    @Published var renderedSources: [URL] = []
    @Published var showingProcessed = true
    private var player: AVAudioPlayer?
    private var ensemble: [AVAudioPlayer] = []

    init() {
        // Used only by local visual QA; regular launches have no fixture environment.
        let fixtureList = ProcessInfo.processInfo.environment["DUBL_QA_FIXTURES"]
            ?? UserDefaults.standard.string(forKey: "DUBL_QA_FIXTURES")
        let fixtures = fixtureList?
            .split(separator: ":")
            .map { URL(fileURLWithPath: String($0)) } ?? []
        if let first = fixtures.first {
            lead = first
            doubles = Array(fixtures.dropFirst())
            selected = Set(fixtures)
            status = "Тестовая сессия загружена. Можно запускать выравнивание."
        }
    }

    var tracks: [URL] { (lead.map { [$0] } ?? []) + doubles }
    var selectedTracks: [URL] { tracks.filter { selected.contains($0) } }
    var modeTitle: String { mode == "tight" ? "Tight" : mode == "locked" ? "Locked" : "Natural" }
    var timingAmount: Double { mode == "tight" ? 0.78 : mode == "locked" ? 1 : 0.48 }
    var pitchAmount: Double { mode == "tight" ? 0.65 : mode == "locked" ? 0.9 : 0.35 }

    func invalidateResults() {
        stop(); results = []; renderedSources = []; showingProcessed = false
        status = "Режим изменён. Нажмите «Выровнять всё» ещё раз."
    }
    func displayedURL(for source: URL) -> URL {
        guard showingProcessed else { return source }
        return processedURL(for: source)
    }
    func processedURL(for source: URL) -> URL {
        guard let index = renderedSources.firstIndex(of: source), results.indices.contains(index) else { return source }
        return results[index]
    }
    func toggleSelection(_ url: URL) {
        if selected.contains(url) { selected.remove(url) } else { selected.insert(url) }
    }
    func removeTrack(_ index: Int) {
        guard !busy, tracks.indices.contains(index) else { return }
        let removed = tracks[index]
        selected.remove(removed)
        if index == 0 {
            lead = doubles.first
            if !doubles.isEmpty { doubles.removeFirst() }
        } else {
            doubles.remove(at: index - 1)
        }
        invalidateResults()
    }
    func chooseTracks() {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [.wav]
        panel.allowsMultipleSelection = true
        if panel.runModal() == .OK {
            stop()
            let urls = panel.urls
            lead = urls.first
            doubles = Array(urls.dropFirst())
            selected = Set(urls)
            results = []; renderedSources = []; showingProcessed = false
            status = "Загружено \(urls.count) дорожек. Они уже выделены."
        }
    }
    func play(_ url: URL?) {
        guard let url else { return }
        do { stop(); player = try AVAudioPlayer(contentsOf: url); player?.play() }
        catch { status = error.localizedDescription }
    }
    func stop() { player?.stop(); ensemble.forEach { $0.stop() }; ensemble = [] }
    func playAll(processed: Bool) {
        let audition = tracks.map { processed ? processedURL(for: $0) : $0 }
        guard !audition.isEmpty else { return }
        do {
            showingProcessed = processed
            stop(); ensemble = try audition.map { try AVAudioPlayer(contentsOf: $0) }
            let gain = 1.0 / Float(ensemble.count)
            ensemble.forEach { $0.volume = gain; $0.prepareToPlay() }
            let time = ensemble[0].deviceCurrentTime + 0.15
            ensemble.forEach { $0.play(atTime: time) }
        } catch { stop(); status = error.localizedDescription }
    }
    func align() {
        let selection = selectedTracks
        guard selection.count >= 2, !busy else { return }
        let reference = selection[0]
        let targets = Array(selection.dropFirst())
        let engine = Bundle.main.bundleURL.appendingPathComponent("Contents/MacOS/dubl_render")
        guard FileManager.default.isExecutableFile(atPath: engine.path) else {
            status = "Не найден движок dubl_render. Запустите собранный DUBL.app."; return
        }
        busy = true; results = []; renderedSources = []; stop(); status = "Выравниваем \(selection.count) выделенных дорожек…"
        let selectedMode = mode
        Task {
            let outcome: Result<[URL], Error> = await Task.detached {
                do {
                    let directory = FileManager.default.temporaryDirectory.appendingPathComponent("dubl-" + UUID().uuidString)
                    return .success(try BatchRenderer.render(engine: engine, lead: reference, doubles: targets, mode: selectedMode, directory: directory))
                } catch { return .failure(error) }
            }.value
            busy = false
            switch outcome {
            case .success(let urls):
                results = urls; renderedSources = targets; showingProcessed = true
                status = "Готово: \(selection.count) дорожек сведены. Нажмите «До» или «После»."
            case .failure(let error): status = error.localizedDescription
            }
        }
    }
    func save() {
        guard let result = results.first else { return }
        let panel = NSSavePanel(); panel.nameFieldStringValue = "DUBL-results"
        guard panel.runModal() == .OK, let target = panel.url else { return }
        do {
            guard !FileManager.default.fileExists(atPath: target.path) else {
                status = "Выберите новое имя: существующие файлы не перезаписываются."; return
            }
            try FileManager.default.copyItem(at: result.deletingLastPathComponent(), to: target)
            status = "Сохранено: " + target.lastPathComponent
        } catch { status = error.localizedDescription }
    }
}

private enum DublColors {
    static let canvas = Color(red: 0.050, green: 0.045, blue: 0.075)
    static let surface = Color(red: 0.075, green: 0.068, blue: 0.105)
    static let lane = Color(red: 0.058, green: 0.053, blue: 0.082)
    static let border = Color(red: 0.19, green: 0.18, blue: 0.25)
    static let guide = Color(red: 0.98, green: 0.78, blue: 0.20)
    static let dub = Color(red: 1.00, green: 0.48, blue: 0.18)
    static let output = Color(red: 0.55, green: 0.39, blue: 1.00)
    static let text = Color(red: 0.92, green: 0.91, blue: 0.96)
    static let muted = Color(red: 0.54, green: 0.52, blue: 0.62)
    static func track(_ index: Int) -> Color {
        [output, Color(red: 1.00, green: 0.34, blue: 0.62), dub, Color(red: 0.20, green: 0.72, blue: 0.95), guide][index % 5]
    }
}

private struct IconButton: View {
    let symbol: String; let help: String; let action: () -> Void
    var body: some View {
        Button(action: action) { Image(systemName: symbol).font(.system(size: 13, weight: .semibold)).frame(width: 28, height: 28) }
            .buttonStyle(.plain).foregroundStyle(DublColors.muted).help(help)
    }
}

private struct TimelineRuler: View {
    var body: some View {
        HStack(spacing: 0) {
            HStack(spacing: 8) {
                Image(systemName: "slider.horizontal.3")
                Text("ДОРОЖКИ")
            }
            .font(.system(size: 9, weight: .bold))
            .tracking(0.8)
            .foregroundStyle(DublColors.muted)
            .padding(.horizontal, 11)
            .frame(width: 190, alignment: .leading)

            Rectangle().fill(DublColors.border).frame(width: 1)
            GeometryReader { geometry in
                ZStack(alignment: .topLeading) {
                    DublColors.surface
                    HStack(spacing: 0) {
                        ForEach(0..<9, id: \.self) { index in
                            VStack(alignment: .leading, spacing: 2) {
                                Text(String(format: "0:%02d", index * 4))
                                    .font(.system(size: 8, weight: .medium, design: .monospaced))
                                    .foregroundStyle(DublColors.muted)
                                Rectangle().fill(DublColors.muted.opacity(0.75)).frame(width: 1, height: 5)
                            }
                            .frame(width: geometry.size.width / 9, alignment: .leading)
                        }
                    }
                    .padding(.leading, 7)
                }
            }
        }
        .frame(height: 31)
        .background(DublColors.surface)
    }
}

private struct TrackHeader: View {
    let title: String; let subtitle: String; let accent: Color; let selected: Bool; let toggle: (() -> Void)?; let play: (() -> Void)?; let remove: (() -> Void)?
    var body: some View {
        HStack(spacing: 8) {
            Rectangle().fill(accent).frame(width: 3)
            if let toggle {
                Button(action: toggle) {
                    Image(systemName: selected ? "checkmark.circle.fill" : "circle")
                        .font(.system(size: 13, weight: .semibold))
                        .foregroundStyle(selected ? accent : DublColors.muted)
                }.buttonStyle(.plain).help(selected ? "Убрать из выделения" : "Добавить в выделение")
            }
            VStack(alignment: .leading, spacing: 5) {
                Text(title).font(.system(size: 10, weight: .bold)).foregroundStyle(DublColors.text).lineLimit(1)
                Text(subtitle).font(.system(size: 8, weight: .bold)).tracking(0.7).foregroundStyle(DublColors.muted)
            }
            Spacer()
            if let play { IconButton(symbol: "play.fill", help: "Прослушать", action: play) }
            if let remove { IconButton(symbol: "xmark", help: "Убрать дорожку", action: remove) }
        }
        .padding(.trailing, 5)
        .frame(width: 190, alignment: .leading)
        .background(DublColors.surface)
    }
}

private struct TimelineGrid: View {
    var body: some View {
        Canvas { context, size in
            context.fill(Path(CGRect(origin: .zero, size: size)), with: .color(DublColors.lane))
            for index in 0...16 {
                let x = CGFloat(index) * size.width / 16
                var path = Path(); path.move(to: CGPoint(x: x, y: 0)); path.addLine(to: CGPoint(x: x, y: size.height))
                context.stroke(path, with: .color(DublColors.border.opacity(index.isMultiple(of: 2) ? 0.8 : 0.35)), lineWidth: 1)
            }
            var middle = Path(); middle.move(to: CGPoint(x: 0, y: size.height / 2)); middle.addLine(to: CGPoint(x: size.width, y: size.height / 2))
            context.stroke(middle, with: .color(DublColors.border.opacity(0.35)), lineWidth: 1)
        }
    }
}

private struct EmptyLane: View {
    let title: String; let subtitle: String; let text: String; let accent: Color; let actionTitle: String; let action: () -> Void
    var body: some View {
        HStack(spacing: 0) {
            TrackHeader(title: title, subtitle: subtitle, accent: accent, selected: false, toggle: nil, play: nil, remove: nil)
            Rectangle().fill(DublColors.border).frame(width: 1)
            ZStack {
                TimelineGrid()
                HStack(spacing: 12) {
                    Image(systemName: "waveform.badge.plus").font(.system(size: 18, weight: .light)).foregroundStyle(accent)
                    Text(text).font(.system(size: 11, weight: .medium)).foregroundStyle(DublColors.muted)
                    Spacer()
                    Button(actionTitle, action: action).buttonStyle(.bordered).tint(accent).controlSize(.small)
                }.padding(.horizontal, 8)
            }
        }.frame(height: 72)
    }
}

private struct AudioLane: View {
    let role: String; let sourceURL: URL; let displayURL: URL; let accent: Color; let selected: Bool; let processed: Bool; let toggle: () -> Void; let play: () -> Void; let remove: () -> Void
    var body: some View {
        HStack(spacing: 0) {
            TrackHeader(title: role, subtitle: sourceURL.lastPathComponent, accent: accent, selected: selected, toggle: toggle, play: play, remove: remove)
            Rectangle().fill(DublColors.border).frame(width: 1)
            GeometryReader { geometry in
                ZStack(alignment: .leading) {
                    TimelineGrid()
                    ZStack(alignment: .topLeading) {
                        RoundedRectangle(cornerRadius: 4).fill(accent.opacity(0.27))
                        RoundedRectangle(cornerRadius: 4).stroke(accent.opacity(0.9), lineWidth: 1)
                        Waveform(url: displayURL, color: accent).padding(.horizontal, 6).padding(.top, 10)
                        HStack(spacing: 6) {
                            Text(sourceURL.deletingPathExtension().lastPathComponent)
                            if processed { Text("ALIGNED").foregroundStyle(DublColors.guide) }
                        }
                            .font(.system(size: 8, weight: .bold))
                            .foregroundStyle(DublColors.text.opacity(0.9))
                            .lineLimit(1)
                            .padding(.horizontal, 6).padding(.top, 3)
                    }
                    .frame(width: max(180, geometry.size.width * 0.72), height: 54)
                    .padding(.leading, 14)
                }
            }
        }.frame(height: 72)
    }
}

private struct AmountMeter: View {
    let title: String; let value: Double; let detail: String; let accent: Color
    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack { Text(title).font(.system(size: 10, weight: .bold)).tracking(0.8); Spacer(); Text(detail).font(.system(size: 9, weight: .semibold)).foregroundStyle(accent) }.foregroundStyle(DublColors.text)
            GeometryReader { geometry in
                ZStack(alignment: .leading) { Capsule().fill(DublColors.border).frame(height: 4); Capsule().fill(accent).frame(width: geometry.size.width * value, height: 4) }
            }.frame(height: 4)
            HStack { Text("МЯГКО"); Spacer(); Text("ТОЧНО") }.font(.system(size: 8, weight: .bold)).foregroundStyle(DublColors.muted)
        }
    }
}

private struct ControlPanel: View {
    @ObservedObject var session: Session
    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            Text("ОБРАБОТКА").font(.system(size: 10, weight: .bold)).tracking(1.2).foregroundStyle(DublColors.muted).padding(.horizontal, 18).frame(height: 39, alignment: .leading)
            Divider().overlay(DublColors.border)
            VStack(alignment: .leading, spacing: 22) {
                VStack(alignment: .leading, spacing: 9) {
                    Text("РЕЖИМ").font(.system(size: 9, weight: .bold)).tracking(0.8).foregroundStyle(DublColors.muted)
                    Picker("Режим", selection: $session.mode) { Text("Natural").tag("natural"); Text("Tight").tag("tight"); Text("Locked").tag("locked") }
                        .labelsHidden().pickerStyle(.segmented).onChange(of: session.mode) { _ in session.invalidateResults() }
                }
                AmountMeter(title: "ТАЙМИНГ", value: session.timingAmount, detail: "АКТИВЕН", accent: DublColors.guide)
                AmountMeter(title: "PITCH", value: session.pitchAmount, detail: "GLOBAL", accent: DublColors.dub)
                HStack(spacing: 10) {
                    Image(systemName: "checkmark.shield.fill").foregroundStyle(DublColors.output)
                    VStack(alignment: .leading, spacing: 2) {
                        Text("ФОРМАНТЫ").font(.system(size: 10, weight: .bold)).foregroundStyle(DublColors.text)
                        Text("Защищены автоматически").font(.system(size: 10)).foregroundStyle(DublColors.muted)
                    }
                }
                Divider().overlay(DublColors.border)
                VStack(alignment: .leading, spacing: 7) {
                    Text("В ЭТОЙ СЕССИИ").font(.system(size: 9, weight: .bold)).tracking(0.8).foregroundStyle(DublColors.muted)
                    Text("Дорожек: \(session.tracks.count)")
                    Text("Выделено: \(session.selected.count)")
                    Text(session.results.isEmpty ? "Не обработано" : "Результат готов")
                }.font(.system(size: 11, weight: .medium)).foregroundStyle(DublColors.text)
                if session.selected.count >= 2 {
                    Text("Первая выделенная дорожка служит опорой")
                        .font(.system(size: 9, weight: .medium))
                        .foregroundStyle(DublColors.muted)
                        .fixedSize(horizontal: false, vertical: true)
                }
                Spacer(minLength: 8)
                Button { session.align() } label: {
                    HStack {
                        if session.busy { ProgressView().controlSize(.small) }
                        Image(systemName: session.busy ? "waveform" : "wand.and.stars")
                        Text(session.busy ? "ОБРАБОТКА…" : "ВЫРОВНЯТЬ ВСЁ").font(.system(size: 12, weight: .bold))
                    }.frame(maxWidth: .infinity, minHeight: 34)
                }.buttonStyle(.borderedProminent).tint(DublColors.output).disabled(session.busy || session.selected.count < 2)
            }.padding(18)
        }.frame(width: 270).background(DublColors.surface)
    }
}

struct Content: View {
    @StateObject private var session = Session()
    @State private var dropTargeted = false

    private func acceptDrop(_ providers: [NSItemProvider]) -> Bool {
        guard !session.busy else { return false }
        for provider in providers {
            provider.loadItem(forTypeIdentifier: UTType.fileURL.identifier, options: nil) { item, _ in
                let url = (item as? Data).flatMap { URL(dataRepresentation: $0, relativeTo: nil) } ?? item as? URL
                guard let url, url.pathExtension.lowercased() == "wav" else { return }
                Task { @MainActor in
                    guard !session.busy else { return }
                    session.stop()
                    if session.lead == nil { session.lead = url }
                    else if !session.doubles.contains(url) { session.doubles.append(url) }
                    session.selected.insert(url)
                    session.results = []; session.renderedSources = []; session.showingProcessed = false
                    session.status = "Дорожка добавлена и выделена."
                }
            }
        }
        return true
    }

    var body: some View {
        VStack(spacing: 0) {
            HStack(spacing: 14) {
                Text("ДУБЛЬ").font(.system(size: 19, weight: .black, design: .rounded)).tracking(0.6).foregroundStyle(DublColors.text)
                Text("VOCAL ALIGNMENT").font(.system(size: 9, weight: .bold)).tracking(1.2).foregroundStyle(DublColors.muted)
                Spacer()
                Text("\(session.modeTitle) · Timing + Pitch").font(.system(size: 11, weight: .medium)).foregroundStyle(DublColors.muted).padding(.horizontal, 14).frame(height: 28).background(DublColors.canvas, in: RoundedRectangle(cornerRadius: 6))
                IconButton(symbol: "arrow.counterclockwise", help: "Сбросить результат") { session.invalidateResults() }
                IconButton(symbol: "stop.fill", help: "Остановить воспроизведение") { session.stop() }
            }.padding(.horizontal, 16).frame(height: 51).background(DublColors.surface)
            Divider().overlay(DublColors.border)
            HStack(spacing: 0) {
                VStack(spacing: 0) {
                    ScrollView {
                        LazyVStack(spacing: 1) {
                            TimelineRuler()
                            if session.tracks.isEmpty {
                                EmptyLane(title: "ДОРОЖКИ", subtitle: "ПУСТАЯ СЕССИЯ", text: "Перетащите сюда две или больше вокальных дорожек", accent: DublColors.output, actionTitle: "ВЫБРАТЬ WAV") { session.chooseTracks() }
                            } else {
                                ForEach(Array(session.tracks.enumerated()), id: \.element) { index, source in
                                    let displayed = session.displayedURL(for: source)
                                    AudioLane(
                                        role: "ТРЕК \(String(format: "%02d", index + 1))",
                                        sourceURL: source,
                                        displayURL: displayed,
                                        accent: DublColors.track(index),
                                        selected: session.selected.contains(source),
                                        processed: displayed != source,
                                        toggle: { session.toggleSelection(source) },
                                        play: { session.play(displayed) },
                                        remove: { session.removeTrack(index) }
                                    )
                                }
                                Button { session.chooseTracks() } label: { Label("ЗАМЕНИТЬ НАБОР ДОРОЖЕК", systemImage: "plus").font(.system(size: 9, weight: .bold)).tracking(0.6).frame(maxWidth: .infinity, minHeight: 30) }
                                    .buttonStyle(.plain).foregroundStyle(DublColors.output).background(DublColors.lane)
                            }
                        }
                    }.background(DublColors.canvas)
                        .overlay { Rectangle().stroke(dropTargeted ? DublColors.output : Color.clear, lineWidth: 2) }
                        .onDrop(of: [UTType.fileURL.identifier], isTargeted: $dropTargeted) { acceptDrop($0) }
                    Divider().overlay(DublColors.border)
                    HStack(spacing: 8) {
                        Button { session.playAll(processed: false) } label: { Label("ДО", systemImage: "play.fill") }.disabled(session.tracks.isEmpty)
                        Button { session.playAll(processed: true) } label: { Label("ПОСЛЕ", systemImage: "play.fill") }.disabled(session.results.isEmpty)
                        Button { session.stop() } label: { Label("СТОП", systemImage: "stop.fill") }
                        Spacer(); Text(session.status).font(.system(size: 10, weight: .medium)).foregroundStyle(DublColors.muted).lineLimit(1); Spacer()
                        Button { session.save() } label: { Label("СОХРАНИТЬ WAV", systemImage: "square.and.arrow.down") }.disabled(session.results.isEmpty)
                    }.buttonStyle(.bordered).controlSize(.small).padding(.horizontal, 14).frame(height: 49).background(DublColors.surface)
                }.disabled(session.busy)
                Divider().overlay(DublColors.border)
                ControlPanel(session: session)
            }
        }.preferredColorScheme(.dark).background(DublColors.canvas).frame(minWidth: 980, minHeight: 620)
    }
}

@main struct DublApp: App {
    var body: some Scene { WindowGroup { Content() }.defaultSize(width: 1180, height: 720) }
}
