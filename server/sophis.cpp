#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <thread>
#include <future>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include "Sophis.h"
#include "Tree.h"
#include "Node.h"

#ifndef M_SPLIT
#define M_SPLIT 3
#endif // !M_SPLIT
#ifndef DISC_ACT_NO
#define DISC_ACT_NO 2
#endif // !DISC_ACT_NO



std::vector<double> sophis(std::array<double, 4> x0, int budget = 10000, double discount = 0.9, double ts = 0.05) {
  const int maxDepth = (int)pow(budget, 0.40);
  static Tree* tree = new Tree(x0, maxDepth, budget);
  tree->clear(x0);

  double Lf = 1.1;
  double L_rho = 1.1;

  int usedBudget = 0;

  while (usedBudget <= budget) {
    int shortestDepth = tree->getMinHeight();
    if (shortestDepth >= maxDepth) {
      break;
    }
    int currentDepth = shortestDepth;
    while (currentDepth < maxDepth) {
      Node* i_plus = tree->getMaxRewardNode(currentDepth);
      auto nodesAtDepth = tree->getNodesAtDepth(currentDepth);
      int k_plus = i_plus->getMaxContributionDim(L_rho, Lf, discount);
      double left = L_rho * (i_plus->uc_right[k_plus] - i_plus->uc_left[k_plus]) * pow(discount, k_plus) * (1 - pow(discount * Lf, i_plus->D - k_plus)) / (1 - discount * Lf);
      double right = pow(discount, i_plus->D) / (1 - discount);
      if (left <= right) {
        usedBudget += tree->discreteSplit(i_plus, discount, ts);
      }
      else {
        usedBudget += tree->continuousSplit(i_plus, k_plus, discount, ts);
      }
      currentDepth++;
    }
  }
  std::vector<double> actions = tree->getMaxRewardNode()->getActions();

  return actions;
}

std::vector<double> parallelSophis(std::array<double, 4> x0, int budget = 10000, double discount = 0.9, double ts = 0.05, short numThreads = 3) {
  const int maxDepth = (int)pow(budget, 0.40);
  static Tree* tree = new Tree(x0, maxDepth, budget);
  tree->clear(x0);

  double Lf = 1.1;
  double L_rho = 1.1;

  int usedBudget = 0;
  std::mutex treeMutex;

  while (usedBudget < budget) {
    int shortestDepth = tree->getMinHeight();
    if (shortestDepth >= maxDepth) {
      break;
    }
    int currentDepth = shortestDepth;
    while (currentDepth < maxDepth) {
      std::vector<std::future<int>> budgets;
      budgets.reserve(numThreads);

      for (int i = 0; i < numThreads; i++) {
        auto f = [L_rho, Lf, discount, ts, maxDepth, &treeMutex](int currentDepth) {
          int usedBudget = 0;
          if (currentDepth > maxDepth) {
            return 0;
          }
          Node* i_plus = tree->getMaxRewardNode(currentDepth);
          if (!i_plus) {
            return 0;
          }
          int k_plus = i_plus->getMaxContributionDim(L_rho, Lf, discount);
          double left = L_rho * (i_plus->uc_right[k_plus] - i_plus->uc_left[k_plus]) * pow(discount, k_plus) * (1 - pow(discount * Lf, i_plus->D - k_plus)) / (1 - discount * Lf);
          double right = pow(discount, i_plus->D) / (1 - discount);
          if (left <= right) {
            return tree->discreteSplit(i_plus, discount, ts, treeMutex);
          }
          return tree->continuousSplit(i_plus, k_plus, discount, ts, treeMutex);
          };
        budgets.emplace_back(std::async(std::launch::async, f, currentDepth + i));
      }
      currentDepth++;
      for (auto& budget : budgets) {
        usedBudget += budget.get();
      }
    }
  }
  std::vector<double> actions = tree->getMaxRewardNode()->getActions();

  return actions;
}

std::vector<double> parallelSophis2(std::array<double, 4> x0, int budget = 10000, double discount = 0.9, double ts = 0.05, short numThreads = 3) {
  const int maxDepth = (int)pow(budget, 0.40);
  static Tree* tree = new Tree(x0, maxDepth, budget);
  tree->clear(x0);

  double Lf = 1.1;
  double L_rho = 1.1;

  int usedBudget = 0;
  std::mutex treeMutex;

  boost::asio::thread_pool pool(numThreads);

  while (usedBudget < budget) {
    int shortestDepth = tree->getMinHeight();
    if (shortestDepth >= maxDepth) {
      break;
    }
    int currentDepth = shortestDepth;
    while (currentDepth < maxDepth) {
      std::vector<std::promise<int>> promises(numThreads);
      for (int i = 0; i < numThreads; i++) {
        auto f = [L_rho, Lf, discount, ts, maxDepth, &treeMutex, promise = &promises[i]](int currentDepth) {
          int usedBudget = 0;
          if (currentDepth > maxDepth) {
            promise->set_value(0);
            return;
          }
          Node* i_plus = tree->getMaxRewardNode(currentDepth);
          if (!i_plus) {
            promise->set_value(0);
            return;
          }
          int k_plus = i_plus->getMaxContributionDim(L_rho, Lf, discount);
          double left = L_rho * (i_plus->uc_right[k_plus] - i_plus->uc_left[k_plus]) * pow(discount, k_plus) * (1 - pow(discount * Lf, i_plus->D - k_plus)) / (1 - discount * Lf);
          double right = pow(discount, i_plus->D) / (1 - discount);
          if (left <= right) {
            promise->set_value(tree->discreteSplit(i_plus, discount, ts, treeMutex));
            return;
          }
          promise->set_value(tree->continuousSplit(i_plus, k_plus, discount, ts, treeMutex));
          return;
          };
        boost::asio::post(pool, std::bind(f, currentDepth + i));
      }
      currentDepth++;
      for (auto& promise : promises) {
        usedBudget += promise.get_future().get();
      }
    }
  }
  std::vector<double> actions = tree->getMaxRewardNode()->getActions();

  return actions;
}
