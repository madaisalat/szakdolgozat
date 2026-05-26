#include "Backtester.h"
#include "Predictor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

using namespace std;

Backtester::Backtester(Database& database) : globalDb(database), gen(random_device{}()), dis(0.0, 1.0) {}

vector<ScheduledMatch> Backtester::loadFixtures(const string& filepath) {
    vector<ScheduledMatch> fixtures;
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "[!] Could not open fixtures file: " << filepath << endl;
        return fixtures;
    }

    string line;
    while (getline(file, line)) {
        size_t comma = line.find(',');
        if (comma != string::npos) {
            ScheduledMatch m;
            m.homeTeam = line.substr(0, comma);
            m.awayTeam = line.substr(comma + 1);
            fixtures.push_back(m);
        }
    }
    file.close();
    return fixtures;
}

vector<RealMatchResult> Backtester::loadRealResults(const string& filepath) {
    vector<RealMatchResult> results;
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "[!] Could not open real results file: " << filepath << endl;
        return results;
    }

    string line;
    getline(file, line);

    while (getline(file, line)) {
        stringstream ss(line);
        string cell;
        vector<string> row;

        while (getline(ss, cell, ',')) {
            row.push_back(cell);
        }

        if (row.size() < 7) { continue; }

        string homeName = row[3];
        string awayName = row[4];
        
        try {
            int hg = stoi(row[5]);
            int ag = stoi(row[6]);

            RealMatchResult r;
            r.homeTeam = homeName;
            r.awayTeam = awayName;
            r.actualHomeGoals = hg;
            r.actualAwayGoals = ag;

            results.push_back(r);
        } 
        catch (const exception& e) {
            cerr << "[!] Error parsing real match data (Teams: " << homeName << " vs " << awayName << "): " << e.what() << endl;
            continue;
        }
    }

    file.close();
    cout << "Successfully loaded " << results.size() << " real results from " << filepath << " for verification!" << endl;
    return results;
}

void Backtester::runRollingBacktest(const string& fixturesFile, const string& realResultsFile) {
    vector<ScheduledMatch> fixtures = loadFixtures(fixturesFile);
    vector<RealMatchResult> realResults = loadRealResults(realResultsFile);

    if (fixtures.empty() || realResults.empty()) {
        cout << "[!] Error loading files. Fixtures: " << fixtures.size() << ", Real Results: " << realResults.size() << endl;
        return;
    }

    Database sandboxDb = globalDb; 
    Predictor predictor(sandboxDb);
    LeagueTable leagueTable; 

    int totalMatches = 0;
    int correctOutcomes = 0;
    int correctScores = 0;
    int correctDoubleChance = 0;
    
    int blockSize = 10;
    int numFixtures = fixtures.size();

    cout << "\n==============================================================" << endl;
    cout << "                     ROLLING SIMULATION START" << endl;
    cout << "==============================================================" << endl;

    for (int i = 0; i < numFixtures; i += blockSize) {
        int currentBlockEnd = min(i + blockSize, numFixtures);
        cout << "\n--- Simulating Block: " << (i / blockSize) + 1 << " (Matches " << i+1 << "-" << currentBlockEnd << ") ---" << endl;
        struct TempCombinedPred {
            int simHome;
            int simAway;
            char simResult;
            string dcTipp;
        };
        vector<TempCombinedPred> blockPredictions;

        for (int j = i; j < currentBlockEnd; ++j) {
            const auto& f = fixtures[j];
            Prediction pred = predictor.predict(f.homeTeam, f.awayTeam);
            double awayWinProb = 1.0 - (pred.homeWinProb + pred.drawProb);

            int simHomeGoals = 0;
            int simAwayGoals = 0;
            char simRes = 'D';
            string dcChoice = "";

            if (pred.homeWinProb >= pred.drawProb && pred.homeWinProb >= awayWinProb) {
                simRes = 'H';
                dcChoice = "1X";
                simHomeGoals = static_cast<int>(pred.expectedHomeGoals + 0.5);
                simAwayGoals = static_cast<int>(pred.expectedAwayGoals);
                if (simHomeGoals <= simAwayGoals) simHomeGoals = simAwayGoals + 1;
            } 
            else if (pred.drawProb >= pred.homeWinProb && pred.drawProb >= awayWinProb) {
                simRes = 'D';
                if (pred.homeWinProb >= awayWinProb) {
                    dcChoice = "1X";
                } else {
                    dcChoice = "X2";
                }
                simHomeGoals = static_cast<int>((pred.expectedHomeGoals + pred.expectedAwayGoals) / 2.0 + 0.5);
                simAwayGoals = simHomeGoals;
            } 
            else {
                simRes = 'A';
                dcChoice = "X2";
                simAwayGoals = static_cast<int>(pred.expectedAwayGoals + 0.5);
                simHomeGoals = static_cast<int>(pred.expectedHomeGoals);
                if (simAwayGoals <= simHomeGoals) simAwayGoals = simHomeGoals + 1;
            }

            if (simHomeGoals < 0 || simHomeGoals > 100) simHomeGoals = 0;
            if (simAwayGoals < 0 || simAwayGoals > 100) simAwayGoals = 0;

            blockPredictions.push_back({simHomeGoals, simAwayGoals, simRes, dcChoice});

            leagueTable.updateMatch(f.homeTeam, f.awayTeam, simHomeGoals, simAwayGoals);
        }

        for (int j = i; j < currentBlockEnd; ++j) {
            const auto& f = fixtures[j];
            const auto& pred = blockPredictions[j - i];

            int actualHomeGoals = 0;
            int actualAwayGoals = 0;
            bool found = false;

            for (const auto& realMatch : realResults) {
                if (realMatch.homeTeam == f.homeTeam && realMatch.awayTeam == f.awayTeam) {
                    actualHomeGoals = realMatch.actualHomeGoals;
                    actualAwayGoals = realMatch.actualAwayGoals;
                    found = true;
                    break;
                }
            }

            if (!found) {
                cerr << "[!] Match not found in real data: " << f.homeTeam << " vs " << f.awayTeam << endl;
                continue;
            }

            char actualRes = (actualHomeGoals > actualAwayGoals) ? 'H' : (actualHomeGoals < actualAwayGoals) ? 'A' : 'D';
            totalMatches++;
            bool outcomeHit = (pred.simResult == actualRes);
            if (outcomeHit) correctOutcomes++;

            bool scoreHit = (pred.simHome == actualHomeGoals && pred.simAway == actualAwayGoals);
            if (scoreHit) correctScores++;

            bool dcHit = false;
            if (pred.dcTipp == "1X" && (actualRes == 'H' || actualRes == 'D')) dcHit = true;
            if (pred.dcTipp == "X2" && (actualRes == 'A' || actualRes == 'D')) dcHit = true;
            if (dcHit) correctDoubleChance++;

            cout << "[MATCH] " << left << setw(14) << f.homeTeam << " vs " << setw(14) << f.awayTeam 
                 << " | Pred: " << pred.simHome << "-" << pred.simAway << " (" << pred.simResult << " / DC: " << pred.dcTipp << ")"
                 << " | Real: " << actualHomeGoals << "-" << actualAwayGoals 
                 << " | 1X2: " << (outcomeHit ? "CORRECT" : "INCORRECT")
                 << " | DC: " << (dcHit ? "CORRECT" : "INCORRECT") << endl;

            sandboxDb.getMatches().emplace_back("BACKTEST_ACTUAL", time(nullptr), f.homeTeam, f.awayTeam, actualHomeGoals, actualAwayGoals, 10, 10, 5, 5);
            int matchIdx = static_cast<int>(sandboxDb.getMatches().size() - 1);

            Team* homeTeamPtr = sandboxDb.getOrCreateTeam(f.homeTeam);
            Team* awayTeamPtr = sandboxDb.getOrCreateTeam(f.awayTeam);

            homeTeamPtr->addMatchIndex(matchIdx, actualHomeGoals, actualAwayGoals);
            awayTeamPtr->addMatchIndex(matchIdx, actualAwayGoals, actualHomeGoals);

            double Ra = homeTeamPtr->getElo();
            double Rb = awayTeamPtr->getElo();
            double Ea = 1.0 / (1.0 + pow(10.0, (Rb - Ra) / 400.0));
            double Eb = 1.0 - Ea;
            
            double Sa = 0.5, Sb = 0.5;
            if (actualRes == 'H') { Sa = 1.0; Sb = 0.0; }
            else if (actualRes == 'A') { Sa = 0.0; Sb = 1.0; }

            double K = 20.0;
            homeTeamPtr->setElo(Ra + K * (Sa - Ea));
            awayTeamPtr->setElo(Rb + K * (Sb - Eb));
        }

        sandboxDb.calculateBaselineParameters();
    }
    cout << "\n==============================================================" << endl;
    cout << "                           RESULTS" << endl;
    cout << "==============================================================" << endl;
    cout << "Total Matches Evaluated: " << totalMatches << endl;
    cout << "Correct Outcomes (1X2):  " << correctOutcomes << " (" << fixed << setprecision(2) << (double)correctOutcomes / totalMatches * 100 << "%)" << endl;
    cout << "Correct Double Chance:   " << correctDoubleChance << " (" << (double)correctDoubleChance / totalMatches * 100 << "%)" << endl;
    cout << "Exact Scores Guessed:    " << correctScores << " (" << (double)correctScores / totalMatches * 100 << "%)" << endl;
    cout << "==============================================================" << endl;
}

void Backtester::runBacktest(const string& fixturesFile) {
    vector<ScheduledMatch> fixtures = loadFixtures(fixturesFile);
    if (fixtures.empty()) {
        cout << "[!] No fixtures loaded!" << endl;
        return;
    }

    Database sandboxDb = globalDb; 
    Predictor predictor(sandboxDb);
    LeagueTable leagueTable;

    cout << "\n==============================================================" << endl;
    cout << "                  STOCHASTICS SIMULATION START" << endl;
    cout << "==============================================================" << endl;
    cout << "  Simulating: " << setw(4) << fixtures.size() << " fixtures" << endl;
    cout << "==============================================================" << endl;
    for (const auto& f : fixtures) {
        Prediction pred = predictor.predict(f.homeTeam, f.awayTeam);

        double diceRoll = dis(gen);
        int simHomeGoals = 0;
        int simAwayGoals = 0;

        if (diceRoll < pred.homeWinProb) {
            simHomeGoals = static_cast<int>(pred.expectedHomeGoals + 0.5);
            simAwayGoals = static_cast<int>(pred.expectedAwayGoals);
            if (simHomeGoals <= simAwayGoals) simHomeGoals = simAwayGoals + 1;
        } 
        else if (diceRoll < (pred.homeWinProb + pred.drawProb)) {
            simHomeGoals = static_cast<int>((pred.expectedHomeGoals + pred.expectedAwayGoals) / 2.0 + 0.5);
            simAwayGoals = simHomeGoals;
        } 
        else {
            simAwayGoals = static_cast<int>(pred.expectedAwayGoals + 0.5);
            simHomeGoals = static_cast<int>(pred.expectedHomeGoals);
            if (simAwayGoals <= simHomeGoals) simAwayGoals = simHomeGoals + 1;
        }

        if (simHomeGoals < 0 || simHomeGoals > 100) simHomeGoals = 0;
        if (simAwayGoals < 0 || simAwayGoals > 100) simAwayGoals = 0;

        sandboxDb.getMatches().emplace_back("SIMULATED", time(nullptr), f.homeTeam, f.awayTeam, simHomeGoals, simAwayGoals, 10, 10, 5, 5);
        
        int matchIdx = static_cast<int>(sandboxDb.getMatches().size() - 1);
        
        Team* homeTeamPtr = sandboxDb.getOrCreateTeam(f.homeTeam);
        Team* awayTeamPtr = sandboxDb.getOrCreateTeam(f.awayTeam);
        
        homeTeamPtr->addMatchIndex(matchIdx, simHomeGoals, simAwayGoals);
        awayTeamPtr->addMatchIndex(matchIdx, simAwayGoals, simHomeGoals);

        double Ra = homeTeamPtr->getElo();
        double Rb = awayTeamPtr->getElo();
        double Ea = 1.0 / (1.0 + pow(10.0, (Rb - Ra) / 400.0));
        double Eb = 1.0 - Ea;
        double Sa = 0.5, Sb = 0.5;
        if (simHomeGoals > simAwayGoals) { Sa = 1.0; Sb = 0.0; }
        else if (simAwayGoals > simHomeGoals) { Sa = 0.0; Sb = 1.0; }
        
        double K = 20.0;
        homeTeamPtr->setElo(Ra + K * (Sa - Ea));
        awayTeamPtr->setElo(Rb + K * (Sb - Eb));

        sandboxDb.calculateBaselineParameters();
        leagueTable.updateMatch(f.homeTeam, f.awayTeam, simHomeGoals, simAwayGoals);

        cout << "[SIM] " << left << setw(18) << f.homeTeam << " " 
             << simHomeGoals << " - " << simAwayGoals << " " 
             << right << setw(18) << f.awayTeam << " (Dice: " << fixed << setprecision(2) << diceRoll << ")" << endl;
    }

    leagueTable.sortTable();
    leagueTable.displayTable();
}

void Backtester::runDeterministicBacktest(const string& fixturesFile) {
    vector<ScheduledMatch> fixtures = loadFixtures(fixturesFile);
    if (fixtures.empty()) {
        cout << "[!] No fixtures loaded!" << endl;
        return;
    }

    Database sandboxDb = globalDb; 
    Predictor predictor(sandboxDb);
    LeagueTable leagueTable;

    cout << "\n==============================================================" << endl;
    cout << "                   MOST LIKELY SIMULATION START" << endl;
    cout << "==============================================================" << endl;
    cout << "  Simulating: " << setw(4) << fixtures.size() << " fixtures" << endl;
    cout << "==============================================================" << endl;

    for (const auto& f : fixtures) {
        Prediction pred = predictor.predict(f.homeTeam, f.awayTeam);

        double awayWinProb = 1.0 - (pred.homeWinProb + pred.drawProb);

        int simHomeGoals = 0;
        int simAwayGoals = 0;
        string choiceType = "";

        if (pred.homeWinProb >= pred.drawProb && pred.homeWinProb >= awayWinProb) {
            choiceType = "HOME WIN";
            simHomeGoals = static_cast<int>(pred.expectedHomeGoals + 0.5);
            simAwayGoals = static_cast<int>(pred.expectedAwayGoals);
            if (simHomeGoals <= simAwayGoals) simHomeGoals = simAwayGoals + 1;
        } 
        else if (pred.drawProb >= pred.homeWinProb && pred.drawProb >= awayWinProb) {
            choiceType = "DRAW";
        
            if (pred.homeWinProb > awayWinProb) {
                choiceType += " (" + f.homeTeam + " favored)";
            } 
            else if (awayWinProb > pred.homeWinProb) {
                choiceType += " (" + f.awayTeam + " favored)";
            } 
            else {
                choiceType += " (Equal)";
            }

            simHomeGoals = static_cast<int>((pred.expectedHomeGoals + pred.expectedAwayGoals) / 2.0 + 0.5);
            simAwayGoals = simHomeGoals;
        } 
        else {
            choiceType = "AWAY WIN";
            simAwayGoals = static_cast<int>(pred.expectedAwayGoals + 0.5);
            simHomeGoals = static_cast<int>(pred.expectedHomeGoals);
            if (simAwayGoals <= simHomeGoals) simAwayGoals = simHomeGoals + 1;
        }

        if (simHomeGoals < 0 || simHomeGoals > 100) simHomeGoals = 0;
        if (simAwayGoals < 0 || simAwayGoals > 100) simAwayGoals = 0;

        sandboxDb.getMatches().emplace_back("SIMULATED", time(nullptr), f.homeTeam, f.awayTeam, simHomeGoals, simAwayGoals, 10, 10, 5, 5);
        
        int matchIdx = static_cast<int>(sandboxDb.getMatches().size() - 1);
        
        Team* homeTeamPtr = sandboxDb.getOrCreateTeam(f.homeTeam);
        Team* awayTeamPtr = sandboxDb.getOrCreateTeam(f.awayTeam);
        
        homeTeamPtr->addMatchIndex(matchIdx, simHomeGoals, simAwayGoals);
        awayTeamPtr->addMatchIndex(matchIdx, simAwayGoals, simHomeGoals);

        double Ra = homeTeamPtr->getElo();
        double Rb = awayTeamPtr->getElo();
        double Ea = 1.0 / (1.0 + pow(10.0, (Rb - Ra) / 400.0));
        double Eb = 1.0 - Ea;
        double Sa = 0.5, Sb = 0.5;
        if (simHomeGoals > simAwayGoals) { Sa = 1.0; Sb = 0.0; }
        else if (simAwayGoals > simHomeGoals) { Sa = 0.0; Sb = 1.0; }
        
        double K = 20.0;
        homeTeamPtr->setElo(Ra + K * (Sa - Ea));
        awayTeamPtr->setElo(Rb + K * (Sb - Eb));

        sandboxDb.calculateBaselineParameters();
        leagueTable.updateMatch(f.homeTeam, f.awayTeam, simHomeGoals, simAwayGoals);

        cout << "[SIM] " << left << setw(18) << f.homeTeam << " " 
             << simHomeGoals << " - " << simAwayGoals << " " 
             << right << setw(18) << f.awayTeam << " (Choice: " << choiceType << ")" << endl;
    }

    leagueTable.sortTable();
    leagueTable.displayTable();
}