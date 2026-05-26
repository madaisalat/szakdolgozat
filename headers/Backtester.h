#ifndef BACKTESTER_H
#define BACKTESTER_H

#include "Database.h"
#include "LeagueTable.h"
#include <string>
#include <vector>
#include <random>

struct ScheduledMatch {
    std::string homeTeam;
    std::string awayTeam;
};
struct RealMatchResult {
    std::string homeTeam;
    std::string awayTeam;
    int actualHomeGoals;
    int actualAwayGoals;
};

class Backtester {
private:
    Database& globalDb;
    std::mt19937 gen;
    std::uniform_real_distribution<double> dis;
    std::vector<ScheduledMatch> loadFixtures(const std::string& filepath);
    std::vector<RealMatchResult> loadRealResults(const std::string& filepath);

public:
    Backtester(Database& database);
    void runBacktest(const std::string& fixturesFile);
    void runDeterministicBacktest(const std::string& fixturesFile);
    void runRollingBacktest(const std::string& fixturesFile, const std::string& realResultsFile);
};

#endif