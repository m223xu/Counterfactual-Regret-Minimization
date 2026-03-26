#include <iostream>
#include <sstream>
#include <array>
#include <map>
#include <random>

static const int NUM_SIDES = 6;
static const int NUM_ACTIONS = (2 * NUM_SIDES) + 1;
static const int DUDO = NUM_ACTIONS - 1;
static const int claimNum[] = {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2};
static const int claimRank[] = {2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6, 1};

std::string claimHistoryToString(const bool isClaimed[]) {
    std::stringstream sb;
    for (int a = 0; a < NUM_ACTIONS; a++) {
        if (isClaimed[a]) {
            if (sb.tellp() > 0)
                sb << ",";
            sb << claimNum[a] << "*" << claimRank[a];
        }
    }
    return sb.str();
}

int infoSetToInteger(const int playerRoll, const bool isClaimed[]) {
    int infoSetNum = playerRoll;
    for (int a = NUM_ACTIONS - 2; a >= 0; a--)
        infoSetNum = 2 * infoSetNum + (isClaimed[a] ? 1 : 0);
    return infoSetNum;
}

class Node {
    int playerRoll; // the player's own roll
    bool claimed[NUM_ACTIONS - 1]; // which claims have been made
    std::array<double, NUM_ACTIONS> regretSum;
    std::array<double, NUM_ACTIONS> strategySum;
    std::array<double, NUM_ACTIONS> strategy;
public:
    Node(int playerRoll) : playerRoll(playerRoll), claimed{}, regretSum{0}, strategySum{0}, strategy{0} {}
    std::array<double, NUM_ACTIONS>& getStrategy(double realizationWeight) {
        double normalizingSum = 0.0;
        for (int a = 0; a < NUM_ACTIONS; ++a) {
            strategy[a] = regretSum[a] > 0 ? regretSum[a] : 0;
            normalizingSum += strategy[a];
        }
        for (int a = 0; a < NUM_ACTIONS; ++a) {
            if (normalizingSum > 0) {
                strategy[a] /= normalizingSum;
            } else {
                strategy[a] = 1.0 / NUM_ACTIONS;
            }
            strategySum[a] += realizationWeight * strategy[a];
        }
        return strategy;
    }
    std::array<double, NUM_ACTIONS> getAverageStrategy() const {
        std::array<double, NUM_ACTIONS> avgStrategy;
        double normalizingSum = 0.0;
        for (int a = 0; a < NUM_ACTIONS; ++a) {
            normalizingSum += strategySum[a];
        }
        for (int a = 0; a < NUM_ACTIONS; ++a) {
            if (normalizingSum > 0) {
                avgStrategy[a] = strategySum[a] / normalizingSum;
            } else {
                avgStrategy[a] = 1.0 / NUM_ACTIONS;
            }
        }
        return avgStrategy;
    }

    friend std::ostream& operator<<(std::ostream& os, const Node& node) {
        os << "Player Roll: " << node.playerRoll << " ";
        os << "Claim History: " << claimHistoryToString(node.claimed) << " ";
        os << "Average Strategy: ";
        auto avgStrategy = node.getAverageStrategy();
        for (int a = 0; a < NUM_ACTIONS; ++a) {
            os << "Action " << a << ": " << avgStrategy[a] << " ";
        }
        return os;
    }
};

class DudoTrainer {
    std::map<int, Node> nodeMap; // maps info set numbers to nodes
    double cfr(int playerRoll[], int isClaimed[], double p0, double p1) {
        double nodeUtil = 0.0;
        return nodeUtil;
    }
public:
    void train(int iterations) {
        std::random_device rd;
        std::mt19937 gen(rd());
        double util = 0.0;

        for (int i = 0; i < iterations; ++i) {
            int playerRoll[2] = { (gen() % NUM_SIDES) + 1, (gen() % NUM_SIDES) + 1 };
            int isClaimed[NUM_ACTIONS - 1] = {0};
            util += cfr(playerRoll, isClaimed, 1.0, 1.0);
        }

        std::cout << "Average game value: " << util / iterations << std::endl;
        for (const auto& pair : nodeMap) {
            std::cout << pair.second << std::endl;
        }
    }
};

int main() {
    DudoTrainer trainer;
    trainer.train(100000);
    return 0;
}