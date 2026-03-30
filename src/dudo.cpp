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
            if (a == DUDO)
                sb << "DUDO";
            else
                sb << claimNum[a] << "*" << claimRank[a];
        }
    }
    return sb.str();
}

int infoSetToInteger(const int playerRoll, const bool isClaimed[]) {
    int infoSetNum = playerRoll;
    for (int a = NUM_ACTIONS - 1; a >= 0; a--)
        infoSetNum = 2 * infoSetNum + (isClaimed[a] ? 1 : 0);
    return infoSetNum;
}

class Node {
    const int playerRoll; // the player's own roll
    bool claimed[NUM_ACTIONS]; // which claims have been made
    const int lastClaim;
    std::array<double, NUM_ACTIONS> regretSum;
    std::array<double, NUM_ACTIONS> strategySum;
    std::array<double, NUM_ACTIONS> strategy;
    friend class DudoTrainer;
public:
    Node(int playerRoll, bool claimed[], int lastClaim) : playerRoll(playerRoll), claimed{}, lastClaim(lastClaim),
                                                          regretSum{0}, strategySum{0}, strategy{0} {
        for (int a = 0; a < NUM_ACTIONS; a++) {
            this->claimed[a] = claimed[a];
        }
    }
    std::array<double, NUM_ACTIONS>& getStrategy(double realizationWeight) {
        double normalizingSum = 0.0;
        if (lastClaim == -1) { // can't claim DUDO on the first turn
            for (int a = 0; a < NUM_ACTIONS - 1; a++) {
                strategy[a] = regretSum[a] > 0 ? regretSum[a] : 0;
                normalizingSum += strategy[a];
            }
            for (int a = 0; a < NUM_ACTIONS - 1; ++a) {
                if (normalizingSum > 0) {
                    strategy[a] /= normalizingSum;
                } else {
                    strategy[a] = 1.0 / (NUM_ACTIONS - 1);
                }
                strategySum[a] += realizationWeight * strategy[a];
            }
        } else {
            for (int a = lastClaim + 1; a < NUM_ACTIONS; ++a) {
                strategy[a] = regretSum[a] > 0 ? regretSum[a] : 0;
                normalizingSum += strategy[a];
            }
            for (int a = lastClaim + 1; a < NUM_ACTIONS; ++a) {
                if (normalizingSum > 0) {
                    strategy[a] /= normalizingSum;
                } else {
                    strategy[a] = 1.0 / (NUM_ACTIONS - lastClaim - 1);
                }
                strategySum[a] += realizationWeight * strategy[a];
            }
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
    double cfr(int playerRoll[], bool isClaimed[], int claimCount, double p0, double p1) {
        double nodeUtil = 0.0;
        int lastClaim = -1;
        for (int a = NUM_ACTIONS - 2; a >= 0; a--) {
            if (isClaimed[a]) {
                lastClaim = a;
                break;
            }
        }
        int currentPlayer = claimCount % 2;
        int infoSetNum = infoSetToInteger(playerRoll[currentPlayer], isClaimed);
        if (nodeMap.find(infoSetNum) == nodeMap.end()) {
            nodeMap.emplace(infoSetNum, Node{playerRoll[currentPlayer], isClaimed, lastClaim});
        }
        if (isClaimed[DUDO]) {
            int lastClaimNum = claimNum[lastClaim];
            int lastClaimRank = claimRank[lastClaim];
            int count = 0;
            for (int i = 0; i < 2; ++i) {
                if (playerRoll[i] == lastClaimRank || playerRoll[i] == 1) {
                    count++;
                }
            }
            return (count >= lastClaimNum) ? 1.0 : -1.0;
        }
        Node& node = nodeMap.at(infoSetNum);
        auto strategy = node.getStrategy(currentPlayer == 0 ? p0 : p1);
        double util[NUM_ACTIONS] = {0.0};
        for (int a = 0; a < NUM_ACTIONS; ++a) {
            if (strategy[a] > 0) { // can only pick strategy for valid actions
                bool nextClaimed[NUM_ACTIONS];
                for (int i = 0; i < NUM_ACTIONS; ++i) {
                    nextClaimed[i] = isClaimed[i];
                }
                nextClaimed[a] = true;
                if (currentPlayer == 0) {
                    util[a] = -cfr(playerRoll, nextClaimed, claimCount + 1, p0 * strategy[a], p1);
                } else {
                    util[a] = -cfr(playerRoll, nextClaimed, claimCount + 1, p0, p1 * strategy[a]);
                }
                nodeUtil += strategy[a] * util[a];
            }
        }
        for (int a = 0; a < NUM_ACTIONS; ++a) {
            if (strategy[a] > 0) {  // Only update legal actions
                double regret = util[a] - nodeUtil;
                node.regretSum[a] += (currentPlayer == 0 ? p1 : p0) * regret;
            }
        }

        return nodeUtil;
    }
public:
    void train(int iterations) {
        std::random_device rd;
        std::mt19937 gen(rd());
        double util = 0.0;

        for (int i = 0; i < iterations; ++i) {
            int playerRoll[2] = { (gen() % NUM_SIDES) + 1, (gen() % NUM_SIDES) + 1 };
            bool isClaimed[NUM_ACTIONS] = {0};
            util += cfr(playerRoll, isClaimed, 0, 1.0, 1.0);
        }

        std::cout << "Average game value: " << util / iterations << std::endl;
        for (const auto& pair : nodeMap) {
            std::cout << pair.second << std::endl;
        }
    }
};

int main() {
    DudoTrainer trainer;
    trainer.train(1000000);
    return 0;
}