#include <iostream>
#include <array>
#include <unordered_map>
#include <map>
#include <string>
#include <random>

// This is a simplified implementation of the Counterfactual Regret Minimization (CFR) algorithm for a Kuhn poker game.

enum {
    PASS = 0,
    BET,
    NUM_ACTIONS
};

class Node {
    std::string infoSet; // unique identifier for the game state
    std::array<double, NUM_ACTIONS> regretSum;
    std::array<double, NUM_ACTIONS> strategySum;
    std::array<double, NUM_ACTIONS> strategy;

    friend class KuhnTrainer;

public:
    // realizationWeight is the probability of reaching this node in the current iteration
    const std::array<double, NUM_ACTIONS>& getStrategy(double realizationWeight) {
        double normalizingSum = 0.0;

        for (size_t a = 0; a < NUM_ACTIONS; ++a) {
            strategy[a] = regretSum[a] > 0 ? regretSum[a] : 0;
            normalizingSum += strategy[a];
        }

        for (size_t a = 0; a < NUM_ACTIONS; ++a) {
            if (normalizingSum > 0) {
                strategy[a] /= normalizingSum;
            } else {
                strategy[a] = 1.0 / NUM_ACTIONS;
            }
            strategySum[a] += realizationWeight * strategy[a];
        }

        return strategy;
    }

    const std::array<double, NUM_ACTIONS> getAverageStrategy() const {
        std::array<double, NUM_ACTIONS> avgStrategy;
        double normalizingSum = 0.0;

        for (size_t a = 0; a < NUM_ACTIONS; ++a) {
            normalizingSum += strategySum[a];
        }

        for (size_t a = 0; a < NUM_ACTIONS; ++a) {
            if (normalizingSum > 0) {
                avgStrategy[a] = strategySum[a] / normalizingSum;
            } else {
                avgStrategy[a] = 1.0 / NUM_ACTIONS;
            }
        }

        return avgStrategy;
    }

    friend std::ostream& operator<<(std::ostream& out, const Node& node) {
        out << node.infoSet << ": ";
        auto avgStrategy = node.getAverageStrategy(); // Update average strategy before printing
        out << "Average Strategy: ";

        for (size_t a = 0; a < NUM_ACTIONS; ++a) {
            out << "Action " << a << ": " << avgStrategy[a] << " ";
        }

        return out;
    }

    Node(std::string infoSet) : infoSet(infoSet) {
        regretSum.fill(0.0);
        strategySum.fill(0.0);
        strategy.fill(0.0);
    }
};

class KuhnTrainer {
    // each node represents a game state and contains the regret sums, strategy sums, and current strategy for that state
    std::map<std::string, Node> nodeMap; // maps info sets to nodes, unordered_map can be used for faster access but map is used here for ordered output

    double cfr(char cards[], std::string history, double p0, double p1) {
        int plays = static_cast<int>(history.length());
        int player = plays % 2;
        bool isPlayerCardHigher = cards[player] > cards[1 - player];

        // terminal states
        if (plays == 3) {
            if (history[2] == 'p') { // pbp
                return 1.0;
            } else { // pbb
                return isPlayerCardHigher ? 2.0 : -2.0;
            }
        } else if (plays == 2) {
            if (history[0] == 'p' && history[1] == 'p') { // pp
                return isPlayerCardHigher ? 1.0 : -1.0;
            } else if (history[0] == 'b' && history[1] == 'p') { // bp
                return isPlayerCardHigher ? 1.0 : -1.0;
            } else if (history[0] == 'b' && history[1] == 'b') { // bb
                return isPlayerCardHigher ? 2.0 : -2.0;
            }
        }

        std::string infoSet = std::string() + cards[player] + history;

        if (nodeMap.find(infoSet) == nodeMap.end()) {
            nodeMap.emplace(infoSet, Node{infoSet});
        }

        Node& node = nodeMap.at(infoSet);
        auto strategy = node.getStrategy(player == 0 ? p0 : p1);
        std::array<double, NUM_ACTIONS> util;
        double nodeUtil = 0.0;

        for (int a = 0; a < NUM_ACTIONS; ++a) {
            std::string nextHistory = history + (a == PASS ? 'p' : 'b');

            if (player == 0) {
                util[a] = -cfr(cards, nextHistory, p0 * strategy[a], p1);
            } else {
                util[a] = -cfr(cards, nextHistory, p0, p1 * strategy[a]);
            }

            nodeUtil += strategy[a] * util[a];
        }

        for (int a = 0; a < NUM_ACTIONS; ++a) {
            double regret = util[a] - nodeUtil;
            node.regretSum[a] += (player == 0 ? p1 : p0) * regret;
        }

        return nodeUtil;
    }

public:
    void train(int iterations) {
        char cards[3] = {'1', '2', '3'};
        int n = static_cast<int>(sizeof(cards) / sizeof(cards[0]));
        std::random_device rd;
        std::mt19937 gen(rd());
        double util = 0.0;

        for (int i = 0; i < iterations; ++i) {
            for (int c1 = n - 1; c1 > 0; --c1) {
                std::uniform_int_distribution<int> dist(0, c1);
                int c2 = dist(gen);
                std::swap(cards[c1], cards[c2]);
            }
            util += cfr(cards, "", 1.0, 1.0);
        }

        std::cout << "Average game value: " << util / iterations << std::endl;
        for (auto& pair : nodeMap) {
            std::cout << pair.second << std::endl;
        }
    }
};


int main() {
    KuhnTrainer trainer;
    trainer.train(1000000);
    return 0;
}
