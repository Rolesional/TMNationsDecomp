#include "tmnf/gbx/campaign.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::string jsonEscape(const std::string_view value) {
  std::string result;
  result.reserve(value.size());
  for (const unsigned char character : value) {
    if (character == '"' || character == '\\') {
      result.push_back('\\');
    }
    if (character >= 0x20U) {
      result.push_back(static_cast<char>(character));
    }
  }
  return result;
}

} // namespace

int main(const int argc, const char* const* argv) {
  if (argc != 2) {
    std::cerr << "usage: campaign_inspect <file.ConstructionCampaign.Gbx>\n";
    return 2;
  }
  const std::filesystem::path path(argv[1]);
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    std::cerr << "unable to open: " << path << '\n';
    return 2;
  }
  const std::vector<char> raw((std::istreambuf_iterator<char>(stream)),
                              std::istreambuf_iterator<char>());
  const std::span bytes(reinterpret_cast<const std::byte*>(raw.data()), raw.size());
  const tmnf::gbx::CampaignParseResult parsed = tmnf::gbx::parseCampaign(bytes);
  if (!parsed) {
    std::cerr << "unable to parse campaign: " << parsed.error << '\n';
    return 1;
  }

  std::size_t mapCount = 0;
  for (const tmnf::gbx::CampaignGroup& group : parsed.campaign.groups) {
    mapCount += group.maps.size();
  }
  std::cout << "{\n  \"groupCount\": " << parsed.campaign.groups.size()
            << ",\n  \"mapCount\": " << mapCount << ",\n  \"groups\": [\n";
  for (std::size_t groupIndex = 0; groupIndex < parsed.campaign.groups.size(); ++groupIndex) {
    const tmnf::gbx::CampaignGroup& group = parsed.campaign.groups[groupIndex];
    std::cout << "    {\"name\": \"" << jsonEscape(group.name) << "\", \"maps\": [\n";
    for (std::size_t mapIndex = 0; mapIndex < group.maps.size(); ++mapIndex) {
      const tmnf::gbx::ChallengeIdent& map = group.maps[mapIndex];
      std::cout << "      {\"uid\": \"" << jsonEscape(map.id) << "\", \"collection\": \""
                << jsonEscape(map.collection) << "\", \"author\": \"" << jsonEscape(map.author)
                << "\"}" << (mapIndex + 1U == group.maps.size() ? "" : ",") << '\n';
    }
    std::cout << "    ]}" << (groupIndex + 1U == parsed.campaign.groups.size() ? "" : ",") << '\n';
  }
  std::cout << "  ]\n}\n";
  return 0;
}
