# Football Match Predictor System

A cross-platform C++ application for football match outcome prediction.

## Prerequisites
- **Windows**: Install [Visual Studio](https://visualstudio.microsoft.com/) (with C++ Desktop development) or [MinGW](https://www.mingw-w64.org/).
- **macOS**: Install Xcode Command Line Tools (`xcode-select --install`).
- **Linux**: `sudo apt install build-essential cmake`
- **Linux**: `sudo apt install build-essential`

## How to Compile

To compile the project manually, open a terminal or command prompt in the project root directory and run the command for your system:

### Windows (MinGW/GCC)
```cmd
g++ -std=c++17 -Iheaders src/*.cpp -o FootballPredictor.exe
```

### macOS / Linux (GCC/Clang)
```bash
g++ -std=c++17 -Iheaders src/*.cpp -o FootballPredictor
```

## How to Run

**Note:** You must run the program from the project root so it can find the `data/` folder.

### Windows
```cmd
FootballPredictor.exe
```

### macOS / Linux
```bash
./FootballPredictor
```

## Data Setup
The program expects CSV data files in the `data/` directory. You can download historical data from Football-Data.co.uk.

Example:
1. Place `E0.csv` into the `data/` folder.
2. When prompted in the app, type `E0.csv`.
