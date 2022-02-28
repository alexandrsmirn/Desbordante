#pragma once

#include <vector>
#include <list>
#include <stack>
#include <queue>

#include "CandidateHashTree.h"
#include "Itemset.h"
#include "ARAlgorithm.h"
#include "Node.h"

class EnumerationTree : public ARAlgorithm {
private:
    //std::unique_ptr<CandidateHashTree> candidateHashTree; //TODO может убрать из полей, а создавать просто в методе?

    Node root;
    //std::unordered_map<Node*, std::list<Node>> candidates;
    unsigned levelNumber = 1;

    std::unordered_map<Node*, std::list<Node>>& generateNextCandidateLevel(std::unordered_map<Node*, std::list<Node>>& candidates); //or list?
    void performCounting();
    void foo(std::list<Node const*> & trace, Node const& node);

    bool canBePruned(std::vector<unsigned> const& itemset);
    static void updatePath(std::stack<Node*> & path, std::vector<Node> & vertices);
    void generateCandidates(std::vector<Node>& children,
                            std::unordered_map<Node*, std::list<Node>>& candidates);
    static void updatePath(std::stack<Node const*> & path, std::vector<Node> const& vertices);
    static void updatePath(std::queue<Node const*> & path, std::vector<Node> const& vertices);
    std::unordered_map<Node*, std::list<Node>> createFirstLevelCandidates();
    void appendToTree(std::unordered_map<Node*, std::list<Node>>& childrenMap);

    double getSupport(std::vector<unsigned> const& frequentItemset) const override;
    unsigned long long generateAllRules() override;
    unsigned long long findFrequent() override;
public:
    explicit EnumerationTree(double minsup, double minconf,
                             std::filesystem::path const& path,
                             TransactionalInputFormat inputFormat = TransactionalInputFormat::TwoColumns,
                             bool hasTransactionID = false,
                             char separator = ',',
                             bool hasHeader = true)
            : ARAlgorithm(minsup, minconf, path, inputFormat, hasTransactionID, separator, hasHeader) {}

    std::list<std::set<std::string>> getAllFrequent() const override;
};
