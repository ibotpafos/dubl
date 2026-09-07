#include <dubl/alignment.hpp>
#include <dubl/features.hpp>
#include <dubl/time_renderer.hpp>
#include <dubl/warp_plan.hpp>
#include <dubl/wav_reader.hpp>
#include <dubl/wav_writer.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace {

struct Arguments {
  std::filesystem::path lead;
  std::filesystem::path dub;
  std::filesystem::path output;
  std::filesystem::path report;
};

std::optional<Arguments> parseArguments(int argc, char** argv) {
  Arguments parsed;
  bool lead = false, dub = false, output = false, report = false;
  for (int i = 1; i < argc; i += 2) {
    if (i + 1 >= argc) return std::nullopt;
    const std::string_view flag = argv[i];
    if (flag == "--lead" && !lead) {
      parsed.lead = argv[i + 1]; lead = true;
    } else if (flag == "--double" && !dub) {
      parsed.dub = argv[i + 1]; dub = true;
    } else if (flag == "--output" && !output) {
      parsed.output = argv[i + 1]; output = true;
    } else if (flag == "--report" && !report) {
      parsed.report = argv[i + 1]; report = true;
    } else {
      return std::nullopt;
    }
  }
  return lead && dub && output && report ? std::optional<Arguments>{parsed} : std::nullopt;
}

std::filesystem::path normalized(const std::filesystem::path& path) {
  return std::filesystem::absolute(path).lexically_normal();
}

bool pathsAreSafe(const Arguments& arguments) {
  const auto lead = normalized(arguments.lead);
  const auto dub = normalized(arguments.dub);
  const auto output = normalized(arguments.output);
  const auto report = normalized(arguments.report);
  return output != lead && output != dub && report != lead && report != dub &&
         output != report && !std::filesystem::exists(output) &&
         !std::filesystem::exists(report);
}

bool writeReport(const std::filesystem::path& path, const dubl::WarpPlan& plan,
                 std::size_t sample_count) {
  const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
  const auto temporary = path.parent_path() /
      ("." + path.filename().string() + ".dubl-tmp-" + std::to_string(stamp));
  std::ofstream stream(temporary, std::ios::trunc);
  if (!stream) return false;
  std::size_t identity_count = 0;
  for (const auto& point : plan.points) if (point.identity) ++identity_count;
  stream << std::fixed << std::setprecision(6)
         << "{\n  \"schema_version\": 1,\n"
         << "  \"renderer\": \"timing-linear-v1\",\n"
         << "  \"pitch_rendered\": false,\n"
         << "  \"confidence\": " << plan.confidence << ",\n"
         << "  \"sample_count\": " << sample_count << ",\n"
         << "  \"warp_point_count\": " << plan.points.size() << ",\n"
         << "  \"identity_point_count\": " << identity_count << "\n}\n";
  stream.flush();
  if (!stream) {
    stream.close();
    std::error_code ignored;
    std::filesystem::remove(temporary, ignored);
    return false;
  }
  stream.close();
  std::error_code publish_error;
  std::filesystem::create_hard_link(temporary, path, publish_error);
  std::error_code cleanup_error;
  std::filesystem::remove(temporary, cleanup_error);
  return !publish_error;
}

}  // namespace

int main(int argc, char** argv) {
  const auto arguments = parseArguments(argc, argv);
  if (!arguments) {
    std::cerr << "usage: dubl_render --lead lead.wav --double double.wav "
                 "--output aligned.wav --report report.json\n";
    return 2;
  }
  if (!pathsAreSafe(*arguments)) {
    std::cerr << "output and report must be new paths distinct from both inputs\n";
    return 3;
  }
  const auto lead = dubl::loadMonoWav(arguments->lead);
  const auto dub = dubl::loadMonoWav(arguments->dub);
  if (!lead.audio || !dub.audio) {
    std::cerr << "unable to load validated mono WAV input\n";
    return 4;
  }
  if (lead.audio->sample_rate != dub.audio->sample_rate) {
    std::cerr << "lead and double sample rates must match\n";
    return 5;
  }
  const auto lead_features = dubl::extractFeatures(*lead.audio);
  const auto dub_features = dubl::extractFeatures(*dub.audio);
  if (lead_features.empty() || dub_features.empty()) {
    std::cerr << "audio is too short for analysis\n";
    return 6;
  }
  const auto alignment = dubl::alignFeatures(lead_features, dub_features);
  const auto plan = dubl::makeSafeWarpPlan(alignment, lead_features, dub_features);
  const auto rendered = dubl::renderTimeWarp(*dub.audio, plan);
  if (!rendered.audio) {
    std::cerr << "unable to render a safe time map\n";
    return 7;
  }
  if (dubl::writeMonoPcm16Wav(arguments->output, *rendered.audio).error !=
      dubl::WavWriteError::none) {
    std::cerr << "unable to publish output WAV\n";
    return 8;
  }
  if (!writeReport(arguments->report, plan, rendered.audio->samples.size())) {
    std::error_code ignored;
    std::filesystem::remove(arguments->output, ignored);
    std::cerr << "unable to publish report\n";
    return 9;
  }
  return 0;
}
