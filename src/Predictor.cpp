#include "Predictor.h"
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace std;

Predictor::Predictor(Database& database, double timeDecay) : db(database), xi(timeDecay), homeAdvantage(0.2), rho(-0.1) {}

double Predictor::poisson(int k, double lambda) {
    return (pow(lambda, k) * exp(-lambda)) / tgamma(k + 1);
}

double Predictor::tau(int x, int y, double lambda, double mu, double rho) {
    if (x == 0 && y == 0) return 1.0 - (lambda * mu * rho);
    if (x == 0 && y == 1) return 1.0 + (lambda * rho);
    if (x == 1 && y == 0) return 1.0 + (mu * rho);
    if (x == 1 && y == 1) return 1.0 - rho;
    return 1.0;
}

double Predictor::calculateLogLikelihood() {
    double totalLL = 0.0;
    const auto& matches = db.getMatches();
    long currentTime = time(nullptr);

    for (const auto& m : matches) {
        double daysDiff = (currentTime - m.timestamp) / (24.0 * 3600.0);
        double weight = exp(-xi * (daysDiff / 365.0));

        Team* home = db.getOrCreateTeam(m.homeTeamName);
        Team* away = db.getOrCreateTeam(m.awayTeamName);

        double lambda = exp(homeAdvantage + home->getAttack() - away->getDefense());
        double mu = exp(away->getAttack() - home->getDefense());
        double prob = poisson(m.homeGoals, lambda) * poisson(m.awayGoals, mu);
        
        prob *= tau(m.homeGoals, m.awayGoals, lambda, mu, rho);

        if (prob > 0) {
            totalLL += weight * log(prob);
        }
    }
    return totalLL;
}

Prediction Predictor::predict(const string& homeName, const string& awayName) {
    Team* home = db.getOrCreateTeam(homeName);
    Team* away = db.getOrCreateTeam(awayName);

    double baseLambda = exp(homeAdvantage + home->getAttack() - away->getDefense());
    double baseMu = exp(away->getAttack() - home->getDefense());
    double eloDiff = home->getElo() - away->getElo();
    double eloAdjustment = eloDiff * eloWeight;
    double lambda = baseLambda + eloAdjustment;
    double mu = baseMu - eloAdjustment;

    if (lambda < 0.1) { lambda = 0.1; }
    if (mu < 0.1) { mu = 0.1; }

    Prediction res = {0.0, 0.0, 0.0, lambda, mu, (lambda + mu), 0.0, 0.0, {}};
    vector<CorrectScore> allScores;
    allScores.reserve(81);

    for (int i = 0; i <= 8; ++i) {
        for (int u = 0; u <= 8; ++u) {
            double p = poisson(i, lambda) * poisson(u, mu) * tau(i, u, lambda, mu, rho);

            if (i > u) res.homeWinProb += p;
            else if (i == u) res.drawProb += p;
            else res.awayWinProb += p;

            if ((i + u) > 2) {
                res.over25Prob += p;
            }
            if (i > 0 && u > 0) {
                res.bttsProb += p;
            }
            allScores.push_back({i, u, p});
        }
    }
    
    sort(allScores.begin(), allScores.end(), [](const CorrectScore& a, const CorrectScore& b) {
        return a.probability > b.probability;
    });

    for (int i = 0; i < 3; ++i) {
        res.topScores.push_back(allScores[i]);
    }

    return res;
}