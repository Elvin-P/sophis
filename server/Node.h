#pragma once

#include <array>
#include <vector>


class Node {
public:
  int nr;
  std::array<double, 4> states;
  std::vector<Node*> children;
  unsigned short depth;
  std::vector<double> uc_left;   // the continuous action intervals are separated into 2 vectors, one for the left side of the interval, and one for the right
  std::vector<double> uc_right;
  std::vector<unsigned short> ud; // for the discrete action, we only remember the last discrete action, aka the one that led to the node
  std::vector<int> num_splits;
  double rew;     //sum of rewards up to this node
  unsigned short D;
  unsigned short C;

public:
  Node(int maxDepth);
  void clear();
  std::array<double, 4> getStates() const { return this->states; }
  Node* addChildNode(Node*);
  int getMaxContributionDim(double, double, double);
  std::vector<double> getActions();
};
