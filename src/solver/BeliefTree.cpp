/** @file BeliefTree.cpp
 *
 * Contains the implementation of BeliefTree.
 */
#include "solver/BeliefTree.hpp"

#include <memory>                       // for unique_ptr
#include <vector>                       // for vector
#include <iostream>

#include "global.hpp"                     // for make_unique

#include "solver/BeliefNode.hpp"               // for BeliefNode
#include "solver/Solver.hpp"

#include "solver/abstract-problem/Observation.hpp"

#include "solver/belief-estimators/estimators.hpp"

#include "solver/mappings/actions/ActionMapping.hpp"
#include "solver/mappings/actions/ActionPool.hpp"

namespace solver {
BeliefTree::BeliefTree(Solver *solver) :
    solver_(solver),
    root_(nullptr),
    allNodes_() {
	reset();
}

// Do nothing!
BeliefTree::~BeliefTree() {
}

/* ------------------- Simple getters --------------------- */
BeliefNode *BeliefTree::getRoot() const {
    return root_.get();
}
BeliefNode *BeliefTree::getNode(long id) const {
    BeliefNode *node = allNodes_[id];
    if (node->getId() != id) {
        std::ostringstream message;
        message << "ERROR: ID mismatch in Belief Tree - ID should be " << id;
        message << " but was " << node->getId();
        debug::show_message(message.str());;
    }
    return allNodes_[id];
}
long BeliefTree::getNumberOfNodes() const {
    return allNodes_.size();
}
std::vector<BeliefNode *> BeliefTree::getNodes() const {
    return allNodes_;
}

/* ------------------- Creation of new nodes in the tree ------------------- */
BeliefNode *BeliefTree::createOrGetChild(BeliefNode *node,
        Action const &action, Observation const &obs) {
    bool isNew;
    BeliefNode *childNode;
    std::tie(childNode, isNew) = node->createOrGetChild(solver_, action, obs);
    if (isNew) {
        addNode(childNode);
        HistoricalData *data = node->getHistoricalData();
        if (data != nullptr) {
            childNode->setHistoricalData(data->createChild(action, obs));
        }
        childNode->setMapping(solver_->getActionPool()->createActionMapping(childNode));
        solver_->getEstimationStrategy()->setValueEstimator(solver_, childNode);
    }
    return childNode;
}

/* ============================ PRIVATE ============================ */


/* ------------------- Node index modification ------------------- */
void BeliefTree::addNode(BeliefNode *node) {
    long id = node->id_;
    if (id < 0) {
        // Negative ID => allocate a new one.
        id = allNodes_.size();
        node->id_ = id;
        allNodes_.push_back(nullptr);
    }
    if (allNodes_[id] != nullptr) {
        debug::show_message("ERROR: Node already exists - overwriting!!");
    }
    allNodes_[id] = node;
}

/* ------------------- Tree modification ------------------- */
BeliefNode *BeliefTree::reset() {
    allNodes_.clear();
    root_ = std::make_unique<BeliefNode>();
    BeliefNode *rootPtr = root_.get();
    addNode(rootPtr);
    return rootPtr;
}
void BeliefTree::initializeRoot() {
    root_->setHistoricalData(solver_->getModel()->createRootHistoricalData());
    root_->setMapping(solver_->getActionPool()->createActionMapping(root_.get()));
    solver_->getEstimationStrategy()->setValueEstimator(solver_, root_.get());
}
} /* namespace solver */
