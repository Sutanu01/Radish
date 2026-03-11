# Radish

A Codeforces rating predictor written in C++. Built out of curiosity and fun.

Works similarly to the [Carrot](https://github.com/meooow25/carrot) browser extension — but runs locally on your machine. Feed it a contest ID and a handle, and it predicts the rating change.

## How is it different from Carrot?

1. Written in C++ with FFT-based convolution — fast responses (assuming CF APIs cooperate).
2. Runs locally — set it up once, check your predicted rating during a contest whenever you want.
3. **May not be as accurate as Carrot** ( **goated** obviously).
4. Some edge cases may be ignored. Suggestions and contributions are welcome.

## Requirements

- **g++** with C++17 support (MinGW-W64 recommended on Windows)
- **Windows** (currently uses `URLDownloadToFileA` from `urlmon.h` for HTTP requests)
- Internet connection (to hit the Codeforces API)

No external libraries to install — [simdjson](https://github.com/simdjson/simdjson) is bundled in the repo.

### Cross-Platform Usage

The only Windows-specific part is the HTTP download function in `dependencies/data/api_handler.cpp` (`URLDownloadToFileA`). To run on Linux/macOS, replace that function with a `curl` or `libcurl` call. Everything else is standard C++17.

## Build & Run

```bash
g++ -std=c++17 -O2 -I. -Idependencies/data -Idependencies/math main.cpp dependencies/data/api_handler.cpp dependencies/data/simdjson.cpp dependencies/math/math_logic.cpp -lurlmon -o radish.exe
```

```bash
.\radish.exe
```

It will prompt for a contest ID and a handle (or `all` for the full board).

> **Tip:** To skip the prompts, edit the `contest_id` and `my_cf_handle` variables directly in `main.cpp`, recompile, and run.

## Project Structure

```
Radish/
├── main.cpp                        # Entry point — pipeline from API fetch to output
├── command.txt                     # Build & run commands for reference
├── dependencies/
│   ├── data/
│   │   ├── api_handler.hpp         # Contestant struct + API function declarations
│   │   ├── api_handler.cpp         # HTTP fetch, JSON parsing, rating population
│   │   ├── simdjson.h              # simdjson header (bundled)
│   │   └── simdjson.cpp            # simdjson implementation (bundled)
│   └── math/
│       ├── math_logic.hpp          # FFT class + rating math declarations
│       └── math_logic.cpp          # FFT convolution, seed, performance, delta
```

## How It Works

1. **Fetch standings** from the CF API for a given contest.
2. **Populate pre-contest ratings** using Codeforces APIs.
3. **Build probability arrays** and run FFT convolution to compute expected seeds for all ratings in O(n log n).
4. **Compute** seed, geometric mean rank, predicted rating, performance, and delta for each contestant.
5. **Apply deflation** adjustment to non-newcomers.
6. **Output** results for a single handle or the full board.
