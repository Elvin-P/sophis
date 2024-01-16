#pragma once
#include <vector>
#include <mutex>
#include "Node.h"

class Tree {
private:
  Node* root;
  int treeSize = 0;
  std::vector<Node> preAllocNodes;

private:
  Node* newChildNode(Node*);
  Node* newChildNode(Node*, std::mutex&);
public:
  Tree(std::array<double, 4>, int, int);
  Node* getRoot() { return this->root; }
  Node* getMaxRewardNode(int);
  Node* getMaxRewardNode();
  std::vector<Node*> getNodesAtDepth(int);
  void clear(std::array<double, 4>);
  int getSize() { return this->treeSize; }
  int getMinHeight();
  int discreteSplit(Node*, double, double);
  int continuousSplit(Node*, int, double, double);
  int discreteSplit(Node*, double, double, std::mutex&);
  int continuousSplit(Node*, int, double, double, std::mutex&);
};
