#include "ParticleSet.hpp"

#include <iostream>
#include <iterator>
#include <unordered_map>
#include <vector>

namespace solver {

ParticleSet::ParticleSet() :
    particleMap_(),
    particles_() {
}

std::vector<HistoryEntry *>::const_iterator ParticleSet::begin() const {
    return particles_.begin();
}

std::vector<HistoryEntry *>::const_iterator ParticleSet::end() const {
    return particles_.end();
}

unsigned long ParticleSet::size() {
    return particles_.size();
}

void ParticleSet::add(HistoryEntry *entry) {
    if (contains(entry)) {
        std::cerr << "ERROR: Duplicate particle." << std::endl;
        return;
    }
    particles_.push_back(entry);
    particleMap_.emplace(entry, particles_.size() - 1);
}

void ParticleSet::remove(HistoryEntry *entry) {
    unsigned long index = particleMap_[entry];
    unsigned long lastIndex = particles_.size() - 1;

    if (index != lastIndex) {
        HistoryEntry *lastEntry = particles_[lastIndex];
        particles_[index] = lastEntry;
        particleMap_[lastEntry] = index;
    }

    // Remove extraneous elements.
    particles_.pop_back();
    particleMap_.erase(entry);
}

HistoryEntry *ParticleSet::get(unsigned long index) {
    return particles_[index];
}

bool ParticleSet::contains(HistoryEntry *entry) {
    return particleMap_.count(entry) > 0;
}
} /* namespace solver */
