#include <array>
#include <vector>
#include "Node.h"
#include "Pendulum.h"

Node::Node(int maxDepth) {
  this->states = { 0.0, 0.0, 0.0, 0.0 };
  this->depth = 0;
  this->rew = 0.0;
  this->nr = 0;
  this->D = 0;
  this->C = 0;
  this->num_splits = std::vector<int>(maxDepth + 1, 0);
  this->uc_left = std::vector<double>(maxDepth + 1, 0.0);
  this->uc_right = std::vector<double>(maxDepth + 1, 1.0);
  this->ud = std::vector<unsigned short>(maxDepth + 1, 0);
}

void Node::clear() {
  this->nr = 0;
  this->states = { 0.0, 0.0, 0.0, 0.0 };
  this->depth = 0;
  this->rew = 0.0;
  this->nr = 0;
  this->D = 0;
  this->C = 0;
  this->children.clear();
  std::fill(this->num_splits.begin(), this->num_splits.end(), 0);
  std::fill(this->uc_left.begin(), this->uc_left.end(), 0.0);
  std::fill(this->uc_right.begin(), this->uc_right.end(), 1.0);
  std::fill(this->ud.begin(), this->ud.end(), 0);
}

Node* Node::addChildNode(Node* child) {
  child->depth = this->depth + 1;
  child->D = this->D;
  child->C = this->C;
  child->uc_left = this->uc_left;
  child->uc_right = this->uc_right;
  child->ud = this->ud;
  child->num_splits = this->num_splits;
  child->states = this->states;
  this->children.push_back(child);

  return child;
}

int Node::getMaxContributionDim(double L_rho, double Lf, double discount) {
  int k_max = 0;
  double max_delta = 0;
  for (short k = 0; k <= this->D; k++)
  {
    const double delta = L_rho * (this->uc_right[k] - this->uc_left[k]) * pow(discount, k) * (1 - pow(discount * Lf, this->D - k)) / (1 - discount * Lf);;
    if (delta > max_delta)
    {
      max_delta = delta;
      k_max = k;
    }
  }
  return k_max;
}

std::vector<double> Node::getActions() {
  std::vector<double> actions;
  for (int i = 0; i < this->D; i++) {
    actions.push_back(Pendulum::getNormalInput(this->uc_left[i], this->uc_right[i], this->ud[i]));
  }
  return actions;
}
