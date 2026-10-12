#include "v2/game/PropRepository.hpp"

#include <algorithm>

namespace v2::game {

void PropRepository::register_manifest(const std::vector<PropDescriptor>& descriptors) {
    for (const auto& desc : descriptors) {
        // First manifest wins — skip existing keys.
        const auto it = std::find_if(records_.begin(), records_.end(),
                                     [&](const PropRecord& r) { return r.key == desc.key; });
        if (it != records_.end()) {
            continue;
        }

        PropRecord rec;
        rec.prop_id = static_cast<uint32_t>(records_.size() + 1);
        rec.key = desc.key;
        rec.owner_id.clear();
        rec.held = false;
        records_.push_back(std::move(rec));
    }
}

bool PropRepository::try_grant(uint32_t prop_id, const std::string& holder_id) {
    if (prop_id == 0 || prop_id > records_.size()) {
        return false;
    }
    auto& rec = records_[prop_id - 1];
    if (!rec.owner_id.empty()) {
        return false;  // already held
    }
    rec.owner_id = holder_id;
    rec.held = true;
    return true;
}

std::optional<PropRecord> PropRepository::release(uint32_t prop_id) {
    if (prop_id == 0 || prop_id > records_.size()) {
        return std::nullopt;
    }
    auto& rec = records_[prop_id - 1];
    const PropRecord before = rec;
    rec.owner_id.clear();
    rec.held = false;
    return before;
}

void PropRepository::release_all_of(const std::string& owner_id) {
    for (auto& rec : records_) {
        if (rec.owner_id == owner_id) {
            rec.owner_id.clear();
            rec.held = false;
        }
    }
}

std::optional<PropRecord> PropRepository::get(uint32_t prop_id) const {
    if (prop_id == 0 || prop_id > records_.size()) {
        return std::nullopt;
    }
    return records_[prop_id - 1];
}

void PropRepository::for_each(const std::function<void(const PropRecord&)>& fn) const {
    for (const auto& rec : records_) {
        fn(rec);
    }
}

size_t PropRepository::size() const {
    return records_.size();
}

}  // namespace v2::game
