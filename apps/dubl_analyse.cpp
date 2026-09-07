#include <dubl/alignment.hpp>
#include <dubl/features.hpp>
#include <dubl/warp_plan.hpp>
#include <dubl/wav_reader.hpp>

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
  std::filesystem::path report;
};

std::optional<Arguments> parseArguments(int argc, char** argv) {
  Arguments parsed;
  bool has_lead = false;
  bool has_dub = false;
  bool has_report = false;
  for (int i = 1; i < argc; i += 2) {
    if (i + 1 >= argc) return std::nullopt;
    const std::string_view flag = argv[i];
    if (flag == "--lead" && !has_lead) {
      parsed.lead = argv[i + 1];
      has_lead = true;
    } else if (flag == "--double" && !has_dub) {
      parsed.dub = argv[i + 1];
      has_dub = true;
    } else if (flag == "--report" && !has_report) {
      parsed.report = argv[i + 1];
      has_report = true;
    } else {
      return std::nullopt;
    }
  }
  if (!has_lead || !has_dub || !has_report) return std::nullopt;
  return parsed;
}

bool writeReport(const std::filesystem::path& path, const dubl::AudioBuffer& lead,
                 const dubl::AudioBuffer& dub,
                 const std::vector<dubl::FrameFeature>& lead_features,
                 const std::vector<dubl::FrameFeature>& dub_features,
                 const dubl::WarpPlan& plan) {
  std::ofstream output(path, std::ios::trunc);
  if (!output) return false;
  std::size_t identity_count = 0;
  for (const auto& point : plan.points) if (point.identity) ++identity_count;

  output << std::fixed << std::setprecision(6);
  output << "{\n"
         << "  \"schema_version\": 1,\n"
         << "  \"confidence\": " << plan.confidence << ",\n"
         << "  \"input\": {\n"
         << "    \"lead_sample_rate\": " << lead.sample_rate << ",\n"
         << "    \"double_sample_rate\": " << dub.sample_rate << ",\n"
         << "    \"lead_sample_count\": " << lead.samples.size() << ",\n"
         << "    \"double_sample_count\": " << dub.samples.size() << ",\n"
         << "    \"lead_feature_frames\": " << lead_features.size() << ",\n"
         << "    \"double_feature_frames\": " << dub_features.size() << "\n"
         << "  },\n"
         << "  \"identity_point_count\": " << identity_count << ",\n"
         << "  \"safe_point_count\": " << plan.points.size() - identity_count << ",\n"
         << "  \"warp_points\": [\n";
  for (std::size_t i = 0; i < plan.points.size(); ++i) {
    const auto& point = plan.points[i];
    output << "    {\"source_seconds\": " << point.source_seconds
           << ", \"target_seconds\": " << point.target_seconds
           << ", \"pitch_ratio\": " << point.pitch_ratio
           << ", \"identity\": " << (point.identity ? "true" : "false") << "}";
    output << (i + 1 == plan.points.size() ? "\n" : ",\n");
  }
  output << "  ]\n}\n";
  return static_cast<bool>(output);
}

}  // namespace

int main(int argc, char** argv) {
  const auto arguments = parseArguments(argc, argv);
  if (!arguments) {
    std::cerr << "usage: dubl_analyse --lead lead.wav --double double.wav --report report.json\n";
    return 2;
  }
  const auto lead = dubl::loadMonoWav(arguments->lead);
  const auto dub = dubl::loadMonoWav(arguments->dub);
  if (!lead.audio || !dub.audio) {
    std::cerr << "unable to load validated mono WAV input\n";
    return 3;
  }
  const auto lead_features = dubl::extractFeatures(*lead.audio);
  const auto dub_features = dubl::extractFeatures(*dub.audio);
  const auto alignment = dubl::alignFeatures(lead_features, dub_features);
  const auto plan = dubl::makeSafeWarpPlan(alignment, lead_features, dub_features);
  if (!writeReport(arguments->report, *lead.audio, *dub.audio,
                   lead_features, dub_features, plan)) {
    std::cerr << "unable to write report\n";
    return 4;
  }
  return 0;
}
