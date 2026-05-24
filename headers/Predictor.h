#ifndef PREDICTOR_H
#define PREDICTOR_H

#include "Database.h"
#include <string>
#include <vector>

struct CorrectScore {
    int homeGoals;
    int awayGoals;
    double probability;
};

struct Prediction {
    double homeWinProb;
    double drawProb;
    double awayWinProb;
    double expectedHomeGoals;
    double expectedAwayGoals;

    double expectedTotalGoals;
    double over25Prob;
    double bttsProb;
    std::vector<CorrectScore> topScores;
};

class Predictor {
private:
    Database& db;
    
    double homeAdvantage;
    double rho;
    double xi;
    double eloWeight = 0.002;

    double poisson(int k, double lambda);
    double tau(int x, int y, double lambda, double mu, double rho);
    double calculateLogLikelihood();

public:
    Predictor(Database& database, double timeDecay = 0.0065);
    Prediction predict(const std::string& homeTeamName, const std::string& awayTeamName);
};

#endif