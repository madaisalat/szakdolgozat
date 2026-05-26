#include <iostream>
#include <limits>
#include <string>
#include <iomanip>
#include "Database.h"   
#include "Predictor.h"
#include "Backtester.h"

using namespace std;

Database db;

void handleLoadData() {
    string filename;
    cout << "\nFilename (e.g. E0.csv): ";
    cin >> filename;
    db.loadFromCSV("data/" + filename);
}

void handleClearDatabase() {
    if (db.getTeams().empty()) {
        cout << "\n[!] Database is already empty." << endl;
        return;
    }
    db.clear();
    cout << "\n[!] Database cleared!" << endl;
}

void handleViewTeams() {
    const auto& teams = db.getTeams();

    if (teams.empty()) {
        cout << "\n[!] Database is empty! You can download data from https://football-data.co.uk/" << endl;
        return;
    }

    cout << "\n====================================================" << endl;
    cout << "               List of teams (ABC)" << endl;
    cout << "====================================================" << endl;
    cout << left << setw(25) << "Team name" 
         << right << setw(10) << "Elo" 
         << setw(10) << "Match" << endl;
    cout << "----------------------------------------------------" << endl;

    for (auto const& [name, teamPtr] : teams) {
        cout << left << setw(25) << name 
             << right << setw(10) << fixed << setprecision(1) << teamPtr->getElo()
             << setw(10) << teamPtr->getMatchesPlayed() << endl;
    }
    cout << "====================================================" << endl;
}

void handleRunPrediction() {
    if (db.getTeams().empty()) {
        cout << "\n[!] No data for prediction! You can download data from https://football-data.co.uk/" << endl;
        return;
    }

    string home, away;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\nHome team: ";
    getline(cin, home);
    cout << "Away team: ";
    getline(cin, away);

    if (!db.teamExists(home) || !db.teamExists(away)) {
        cout << "\n[!] Unknown team name!" << endl;
        return;
    }

    Predictor predictor(db);
    Prediction res = predictor.predict(home, away);

    cout << "\n============================================" << endl;
    cout << " Prediction: " << home << " vs " << away << endl;
    cout << "============================================" << endl;
    cout << fixed << setprecision(2);
    cout << "Home win (1): " << setw(6) << (res.homeWinProb * 100) << "%" << endl;
    cout << "Draw     (X): " << setw(6) << (res.drawProb * 100) << "%" << endl;
    cout << "Away win (2): " << setw(6) << (res.awayWinProb * 100) << "%" << endl;
    cout << "--------------------------------------------" << endl;
    cout << "Expected goals (xG): " << res.expectedHomeGoals << " - " << res.expectedAwayGoals << endl;
    cout << "Expected total goals: " << res.expectedTotalGoals << endl;
    cout << "--------------------------------------------" << endl;
    cout << "Over 2.5 goals:       " << setw(6) << (res.over25Prob * 100) << "%" << endl;
    cout << "Under 2.5 goals:      " << setw(6) << ((1.0 - res.over25Prob) * 100) << "%" << endl;
    cout << "Both teams to score:  " << setw(6) << (res.bttsProb * 100) << "%" << endl;
    cout << "--------------------------------------------" << endl;
    cout << "TOP 3 MOST LIKELY CORRECT SCORES:" << endl;
    for (size_t i = 0; i < res.topScores.size(); ++i) {
        cout << i + 1 << ". " << home << " " 
             << res.topScores[i].homeGoals << " - " << res.topScores[i].awayGoals << " " << away
             << " (" << (res.topScores[i].probability * 100) << "%)" << endl;
    }
    cout << "============================================" << endl;
}
void handleRunBacktest() {
    if (db.getTeams().empty()) {
        cout << "\n[!] Load database (1) before running backtest!" << endl;
        return;
    }

    cout << "\n==============================================" << endl;
    cout << "        SELECT BACKTEST VALIDATION MODE" << endl;
    cout << "==============================================" << endl;
    cout << "1. Stochastic Backtest (Dice Roll)" << endl;
    cout << "2. Deterministic Backtest (Most Likely Outcomes)" << endl;
    cout << "3. Rolling Window Backtest (Iterative Learning)" << endl;
    cout << "----------------------------------------------" << endl;
    cout << "Choose an option (1-3): ";
    
    int choice;
    if (!(cin >> choice) || choice < 1 || choice > 3) {
        cout << "\n[!] Invalid selection. Returning to menu." << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    string fixturesFilename;
    cout << "Fixtures filename (e.g. PL25.txt): ";
    cin >> fixturesFilename;
    string fixturesPath = "data/" + fixturesFilename;

    Backtester backtester(db);

    switch (choice) {
        case 1:
            cout << "\n[i] Starting Stochastic Backtest..." << endl;
            backtester.runBacktest(fixturesPath);
            break;
            
        case 2:
            cout << "\n[i] Starting Deterministic Backtest..." << endl;
            backtester.runDeterministicBacktest(fixturesPath);
            break;
            
        case 3: {
            string resultsFilename;
            cout << "Real results filename (e.g. E25.csv): ";
            cin >> resultsFilename;
            string resultsPath = "data/" + resultsFilename;
            
            cout << "\n[i] Starting Rolling Window Backtest..." << endl;
            backtester.runRollingBacktest(fixturesPath, resultsPath);
            break;
        }
        
        default:
            break;
    }
}

int main() {
    int choice;
    bool running = true;

    while (running) {
        cout << "\n====================================" << endl;
        cout << "   FOOTBALL MATCH PREDICTOR SYSTEM" << endl;
        cout << "====================================" << endl;
        cout << "1. Load data (CSV)" << endl;
        cout << "2. Leaderboard (ELO)" << endl;
        cout << "3. Match prediction" << endl;
        cout << "4. Run Backtest Simulation" << endl;
        cout << "5. Clear Database" << endl;
        cout << "0. Quit" << endl;
        cout << "------------------------------------" << endl;
        cout << "Choose an option: ";

        if (!(cin >> choice)) {
            cout << "\n[!] Invalid input!" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1: handleLoadData(); break;
            case 2: handleViewTeams(); break;
            case 3: handleRunPrediction(); break;
            case 4: handleRunBacktest(); break;
            case 5: handleClearDatabase(); break;
            case 0: 
                cout << "\nClosing..." << endl;
                running = false; 
                break;
            default: 
                cout << "\n[!] Invalid option (1-5)!" << endl;
        }
    }
    return 0;
}