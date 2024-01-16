#include <array>
#include <queue>
#include <mutex>
#include "Tree.h"
#include "Node.h"
#include "Pendulum.h"

#ifndef DISC_ACT_NO
#define DISC_ACT_NO 1
#endif // !DISC_ACT_NO
#ifndef M_SPLIT
#define M_SPLIT 3
#endif // !M_SPLIT



Node* Tree::newChildNode(Node* parent) {
  Node* child = &preAllocNodes[treeSize];
  child->nr = treeSize++;
  parent->addChildNode(child);
  return child;
}

Node* Tree::newChildNode(Node* parent, std::mutex& mut) {
  mut.lock();
  Node* child = &preAllocNodes[treeSize];
  child->nr = treeSize++;
  mut.unlock();
  parent->addChildNode(child);
  return child;
}

Tree::Tree(std::array<double, 4> x0, int maxDepth, int treeSize) {
  this->preAllocNodes = std::vector<Node>(treeSize, Node(maxDepth));
  Node* rootNode = &this->preAllocNodes[0];
  rootNode->nr = treeSize++;
  rootNode->states = x0;
  rootNode->rew = Pendulum::rewardFunc(x0, 0.5);
  this->root = rootNode;
}

void Tree::clear(std::array<double, 4> x0) {
  for (Node& node : this->preAllocNodes) {
    node.clear();
  }
  this->root->states = x0;
  this->root->rew = Pendulum::rewardFunc(x0, 0.5);
  treeSize = 1;
}

int Tree::getMinHeight() {
  std::queue<Node*> queue;

  int minH = 0;
  Node* temp_node = root;

  while (temp_node->children.size() != 0) {
    for (Node* child : temp_node->children) {
      queue.push(child);
    }
    temp_node = queue.front();
    queue.pop();
  }
  minH = temp_node->depth;
  return minH;
}

Node* Tree::getMaxRewardNode(int depth = -1) {
  Node* maxRewardNode = this->root;
  double maxR = 0;
  bool still_searching = true;
  for (int i = 1; i < this->treeSize; i++) {
    auto tempNode = &preAllocNodes[i];
    if (tempNode->depth == depth && !tempNode->children.size() && tempNode->rew >= maxR)
    {
      maxRewardNode = tempNode;
      maxR = tempNode->rew;
    }
  }
  if (depth != 0 && maxRewardNode == this->root) {
    return NULL;
  }
  return maxRewardNode;
}

std::vector<Node*> Tree::getNodesAtDepth(int depth) { // returns nodes at depth in ascending order of reward
  std::queue<Node*> queue;
  if (depth > 0) {
    std::vector<Node*> nodes;
    for (Node* child : this->root->children) {
      queue.push(child);
    }

    Node* tempNode = this->root;
    while (queue.size()) {
      tempNode = queue.front();
      queue.pop();

      if (tempNode->depth == depth) {
        nodes.push_back(tempNode);
        continue;
      }

      for (Node* child : tempNode->children) {
        queue.push(child);
      }
    }
    std::sort(nodes.begin(), nodes.end(), [](const Node* a, const Node* b) {
      return a->rew < b->rew;
      });
    return nodes;
  }
  else {
    return { this->root };
  }
}

Node* Tree::getMaxRewardNode() {
  std::queue<Node*> queue;

  Node* temp_node = this->root;
  Node* maxRewardNode = this->root;
  double maxR = 0;
  bool still_searching = true;
  while (still_searching == true) {
    still_searching = false;
    for (Node* child : temp_node->children) {
      queue.push(child);
    }

    if (queue.size() > 0)
    {
      temp_node = queue.front();
      queue.pop();
      still_searching = true;
    }

    if (temp_node->rew >= maxR)
    {
      maxRewardNode = temp_node;
      maxR = temp_node->rew;
    }
  }
  return maxRewardNode;

}

int Tree::discreteSplit(Node* parent, double discount, double ts) {
  parent->children.reserve(DISC_ACT_NO);
  for (int i = 0; i < DISC_ACT_NO; i++)
  {
    Node* child = newChildNode(parent);
    child->ud[child->D] = i;
    child->D = parent->D + 1;
    double current_action = Pendulum::getNormalInput(child->uc_left[child->D - 1], child->uc_right[child->D - 1], child->ud[child->D - 1]);
    child->states = Pendulum::simulate(current_action, child->states, ts);
    child->rew = Pendulum::rewardFunc(child->states, current_action) * pow(discount, child->D) + parent->rew;
  }
  return DISC_ACT_NO;
}

int Tree::discreteSplit(Node* parent, double discount, double ts, std::mutex& mut) {
  parent->children.reserve(DISC_ACT_NO);
  for (int i = 0; i < DISC_ACT_NO; i++)
  {
    Node* child = newChildNode(parent, mut);
    child->ud[child->D] = i;
    child->D = parent->D + 1;
    double current_action = Pendulum::getNormalInput(child->uc_left[child->D - 1], child->uc_right[child->D - 1], child->ud[child->D - 1]);
    child->rew = Pendulum::rewardFunc(child->states, current_action) * pow(discount, child->D) + parent->rew;
    child->states = Pendulum::simulate(current_action, child->states, ts);
  }
  return DISC_ACT_NO;
}

int Tree::continuousSplit(Node* parent, int k_plus, double discount, double ts) {
  int usedBudget = 0;
  parent->children.reserve(M_SPLIT);
  parent->num_splits[k_plus]++;
  for (int i = 0; i < M_SPLIT; i++)
  {
    Node* child = newChildNode(parent);
    if (k_plus >= parent->C)
    {
      k_plus = parent->C;
      child->C = parent->C + 1;
    }
    child->uc_left[k_plus] = parent->uc_left[k_plus] + i * (parent->uc_right[k_plus] - parent->uc_left[k_plus]) / M_SPLIT;
    child->uc_right[k_plus] = parent->uc_left[k_plus] + (i + 1) * (parent->uc_right[k_plus] - parent->uc_left[k_plus]) / M_SPLIT;

    if (i != 1) {
      child->states = root->states;
      child->rew = root->rew;
      for (int j = 0; j < child->D; j++)
      {
        double current_action = Pendulum::getNormalInput(child->uc_left[j], child->uc_right[j], child->ud[j]);
        child->rew += Pendulum::rewardFunc(child->states, current_action) * pow(discount, j);
        child->states = Pendulum::simulate(current_action, child->states, ts);
        usedBudget++;
      }
    }
  }
  return usedBudget;
}

int Tree::continuousSplit(Node* parent, int k_plus, double discount, double ts, std::mutex& mut) {
  int usedBudget = 0;
  parent->children.reserve(M_SPLIT);
  parent->num_splits[k_plus]++;
  for (int i = 0; i < M_SPLIT; i++)
  {
    Node* child = newChildNode(parent, mut);
    if (k_plus >= parent->C)
    {
      k_plus = parent->C;
      child->C = parent->C + 1;
    }
    child->uc_left[k_plus] = parent->uc_left[k_plus] + i * (parent->uc_right[k_plus] - parent->uc_left[k_plus]) / M_SPLIT;
    child->uc_right[k_plus] = parent->uc_left[k_plus] + (i + 1) * (parent->uc_right[k_plus] - parent->uc_left[k_plus]) / M_SPLIT;

    if (i != 1) {
      child->states = root->states;
      child->rew = root->rew;
      for (int j = 0; j < child->D; j++)
      {
        double current_action = Pendulum::getNormalInput(child->uc_left[j], child->uc_right[j], child->ud[j]);
        child->states = Pendulum::simulate(current_action, child->states, ts);
        child->rew += Pendulum::rewardFunc(child->states, current_action) * pow(discount, j);
        usedBudget++;
      }
    }
  }
  return usedBudget;
}
