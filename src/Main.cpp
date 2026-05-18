#include <iostream>
#include <limits>
#include <string>
#include <iomanip>
#include "Database.h"   
#include "Predictor.h"

using namespace std;

Database db;

void handleLoadData() {
    string filename;
    cout << "\nFilename2 (e.g. E0.csv): ";
    cin >> filename;
    db.loadFromCSV("data/" + filename);
}

void handleViewTeams() {
    const auto& teams = db.getTeams();

    if (teams.empty()) {
        cout << "\n[!] Database is empty!" << endl;
        return;
    }

    cout << "\n====================================================" << endl;
    cout << "          List of teams (ABC)" << endl;
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
        cout << "\n[!] No data for prediction!" << endl;
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
    cout << "============================================" << endl;
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
            case 0: 
                cout << "\nClosing..." << endl;
                running = false; 
                break;
            default: 
                cout << "\n[!] Invalid option (1-4)!" << endl;
        }
    }
    return 0;
}