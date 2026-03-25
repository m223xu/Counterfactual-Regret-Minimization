#include <iostream>
#include <sstream>

static const int NUM_SIDES = 6;
static const int NUM_ACTIONS = (2 * NUM_SIDES) + 1;
static const int DUDO = NUM_ACTIONS - 1;
static const int claimNum[] = {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2};
static const int claimRank[] = {2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6, 1};

class DudoTrainer {
    std::string claimHistoryToString(bool isClaimed[]) {
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

    int infoSetToInteger(int playerRoll, bool isClaimed[]) {
        int infoSetNum = playerRoll;
        for (int a = NUM_ACTIONS - 2; a >= 0; a--)
            infoSetNum = 2 * infoSetNum + (isClaimed[a] ? 1 : 0);
        return infoSetNum;
    }
public:
};

int main() {

    return 0;
}