#include "BeliefNode.hpp"

#include <cmath>                        // for log, sqrt
#include <ctime>                        // for clock, CLOCKS_PER_SEC, clock_t

#include <iostream>                     // for cerr, endl, operator<<, basic_ostream, ostream
#include <map>                          // for _Rb_tree_iterator, map<>::iterator, map
#include <memory>                       // for unique_ptr
#include <random>                       // for uniform_int_distribution
#include <tuple>                        // for tie, tuple
#include <type_traits>                  // for remove_reference<>::type
#include <utility>                      // for pair, make_pair, move
#include <vector>                       // for vector

#include "defs.hpp"                     // for RandomGenerator, make_unique

#warning Action and Observation should be classes!
#include "Action.hpp"                   // for Action
#include "ActionNode.hpp"               // for ActionNode
#include "HistoryEntry.hpp"             // for HistoryEntry
#include "Observation.hpp"              // for Observation
#include "State.hpp"                    // for State

using std::cerr;
using std::endl;

namespace solver {
long BeliefNode::currId = 0;
std::clock_t BeliefNode::startTime = std::clock();

BeliefNode::BeliefNode() :
    BeliefNode(currId) {
}

BeliefNode::BeliefNode(long id) :
    id_(id),
    nParticles_(0),
    nextActionToTry_(0),
    bestMeanQValue_(0),
    bestAction_(-1),
    tLastAddedParticle_(0),
    tNNComp_(-1.0),
    nnBel_(nullptr),
    particles_(),
    actChildren_() {
    if (currId <= id) {
        currId = id + 1;
    }
}

// Do-nothing destructor
BeliefNode::~BeliefNode() {
}

Action BeliefNode::getUcbAction(double ucbExploreCoefficient) {
    double tmpVal;
    ActionMap::iterator actionIter = actChildren_.begin();
    double maxVal = (actionIter->second->meanQValue_ + ucbExploreCoefficient
                     *std::sqrt(std::log(nParticles_)
                             / actionIter->second->nParticles_));
    Action bestActId = actionIter->first;
    actionIter++;
    for (; actionIter != actChildren_.end(); actionIter++) {
        tmpVal =
            (actionIter->second->meanQValue_ + ucbExploreCoefficient
             * std::sqrt(
                     std::log(nParticles_)
                     / actionIter->second->nParticles_));
        if (maxVal < tmpVal) {
            maxVal = tmpVal;
            bestActId = actionIter->first;
        }
    }
    return bestActId;
}

Action BeliefNode::getBestAction() {
    if (getNActChildren() == 0) {
        cerr << "No children - could not retrieve best action!!" << endl;
        return -1;
    }
    updateBestValue();
    return bestAction_;
}

double BeliefNode::getBestMeanQValue() {
    updateBestValue();
    return bestMeanQValue_;
}

void BeliefNode::updateBestValue() {
    if (getNActChildren() == 0) {
        return;
    }
    ActionMap::iterator actionIter = actChildren_.begin();
    double bestQVal = actionIter->second->meanQValue_;
    bestAction_ = actionIter->first;
    actionIter++;
    for (; actionIter != actChildren_.end(); actionIter++) {
        if (bestQVal < actionIter->second->meanQValue_) {
            bestQVal = actionIter->second->meanQValue_;
            bestAction_ = actionIter->first;
        }
    }
}

void BeliefNode::add(HistoryEntry *newHistEntry) {
    tLastAddedParticle_ = (double) (std::clock() - startTime)
        * 10000/ CLOCKS_PER_SEC;
    particles_.push_back(newHistEntry);
    nParticles_++;
}

std::pair<BeliefNode *, bool> BeliefNode::addChild(Action const &action,
        Observation const &obs) {
    std::unique_ptr<ActionNode> newActionNode = std::make_unique<ActionNode>(
                action);
    bool added = false;
    std::map<Action, std::unique_ptr<ActionNode>>::iterator actionIter;
    std::tie(actionIter, added) = actChildren_.insert(std::make_pair(
                        action, std::move(newActionNode)));

    ActionNode *actChild = actionIter->second.get();
    return actChild->addChild(obs);
}

HistoryEntry *BeliefNode::sampleAParticle(RandomGenerator *randGen) {
    return particles_[std::uniform_int_distribution<unsigned long>(
                          0, nParticles_ - 1)(*randGen)];
}

void BeliefNode::updateQValue(Action &action, double increase) {
    ActionMap::iterator iter = actChildren_.find(action);
    if (iter == actChildren_.end()) {
        return;
    }
    iter->second->updateQValue(increase);
    updateBestValue();
}

void BeliefNode::updateQValue(Action &action, double oldValue, double newValue,
        bool reduceParticles) {
    ActionMap::iterator iter = actChildren_.find(action);
    if (iter == actChildren_.end()) {
        return;
    }
    iter->second->updateQValue(oldValue, newValue, reduceParticles);
    updateBestValue();
}

double BeliefNode::distL1Independent(BeliefNode *b) {
    double dist = 0.0;
    for (HistoryEntry *entry1 : particles_) {
        for (HistoryEntry *entry2 : b->particles_) {
            dist += entry1->getState()->distanceTo(*entry2->getState());
        }
    }
    return dist / (nParticles_ * b->nParticles_);
}

BeliefNode *BeliefNode::getChild(Action const &action, Observation const &obs) {
    ActionMap::iterator iter = actChildren_.find(action);
    if (iter == actChildren_.end()) {
        return nullptr;
    }
    return iter->second->getBeliefChild(obs);
}

Action BeliefNode::getNextActionToTry() {
    return nextActionToTry_++;
}
} /* namespace solver */
