#pragma once

#include "tmnf/gbx/challenge.hpp"

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace tmnf::gbx {

struct CampaignGroup final {
  std::string name;
  std::vector<ChallengeIdent> maps;
};

struct Campaign final {
  std::vector<CampaignGroup> groups;
};

struct CampaignParseResult final {
  Campaign campaign;
  std::string error;

  [[nodiscard]] explicit operator bool() const noexcept { return error.empty(); }
};

// Reads the ordered challenge groups from a CGameCtnCampaign document.
[[nodiscard]] CampaignParseResult parseCampaign(std::span<const std::byte> documentBytes);

} // namespace tmnf::gbx
