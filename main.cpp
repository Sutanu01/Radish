#include <iostream>
#include <string>
#include <vector>
#include "dependencies/data/api_handler.hpp"
#include "dependencies/math/math_logic.hpp"


using namespace std;


int main() {

    int contest_id = 2207;
    string my_cf_handle="Sutanu01";

    cout << "Enter the contest ID: ";
    cin >> contest_id;
    cout << "Enter handle to look up (or 'all' for full board): ";
    cin >> my_cf_handle;
    
    // make changes accordingly 
    // i would suggest to comment out taking input part
    // just make changes in the variables directly -> then compile -> run


    

    vector<CodeforcesAPI::Contestant> contestants;
    try{
        const string url =
            "https://codeforces.com/api/contest.standings?contestId=" +
            to_string(contest_id) +
            "&from=1&count=100000&showUnofficial=false";
    
        const string json_data = CodeforcesAPI::FetchData(url);
        if (json_data.empty()) {
            cerr << "Failed to fetch data from Codeforces API.\n";
            return 1;
        }
        contestants = CodeforcesAPI::ProcessContestStandings(json_data);
        if (contestants.empty()) {
            cerr << "No contestants parsed from API response.\n";
            return 1;
        }
        CodeforcesAPI::PopulatePreContestRatings(contestants, contest_id);
    }
    catch(const exception& e){
        cerr << "Error during API fetch/parse: " << e.what() << "\n";
        return 1;
    }
    catch(...){
        cerr << "Unknown error during API fetch/parse.\n";
        return 1;
    }
    



    // calculating the seed , ratings etc...
    int minRating = 0;
    for (const auto& c : contestants) {
        if (c.participantType == "CONTESTANT")
            minRating = min(minRating, c.preContestRating);
    }
    int shift = (minRating < 0) ? -minRating : 0;
    int effectiveMax = RatingMath::MAX_RATING + shift;

    vector<double> A(effectiveMax + 1, 0);
    vector<double> B(2 * effectiveMax + 1, 0);

    for (const auto& c : contestants) {
        if (c.participantType == "CONTESTANT")
            A[c.preContestRating + shift]++;
    }
    for (int i = 0; i < 2 * effectiveMax + 1; i++) {
        B[i] = RatingMath::prob_func(i - effectiveMax);
    }

    RatingMath::FFT fft;
    fft.convolution(A, B);
    const vector<double>& S = fft.S;

    for (auto& c : contestants) {
        if (c.participantType != "CONTESTANT") continue;
        c.seed            = RatingMath::GetSeed(S, c.preContestRating + shift);
        c.meanRank        = RatingMath::GetMeanRank(c.seed, c.rank);
        c.predictedRating = RatingMath::FindRatingForSeed(S, c.meanRank) - shift;
        c.performance     = RatingMath::ComputePerformance(S, c.rank) - shift;
        c.delta           = RatingMath::ComputeDelta(S, c.preContestRating + shift, c.rank);
    }




    // Deflation rating adjustments
    {
        int sum_delta = 0;
        int non_newcomer_count = 0;
        for (const auto& c : contestants) {
            if (c.participantType != "CONTESTANT") continue;
            if (c.preContestRating != 0) {
                sum_delta += c.delta;
                non_newcomer_count++;
            }
        }
        if (non_newcomer_count > 1) {
            int deflation = -(sum_delta/non_newcomer_count) - 1;
            for (auto& c : contestants) {
                if (c.participantType != "CONTESTANT") continue;
                if (c.preContestRating != 0) {
                    c.delta += deflation;
                }
            }
        }
    }

    cout<<endl<<endl;


    
    if (my_cf_handle != "all") {
        bool found = false;
        for (const auto& c : contestants) {
            if (c.handle == my_cf_handle) {
                cout << "Handle:           " << c.handle << "\n"
                     << "Rank:             " << c.rank << "\n"
                     << "Pre-contest:      " << c.preContestRating << "\n"
                     << "Seed:             " << c.seed << "\n"
                     << "Mean Rank:        " << c.meanRank << "\n"
                     << "Predicted Rating: " << c.predictedRating << "\n"
                     << "Performance:      " << c.performance << "\n"
                     << "Delta:            " << c.delta << "\n";
                found = true;
                break;
            }
        }
        if (!found) cerr << "Handle '" << my_cf_handle << "' not found in standings.\n";
    } else {
        cout << "Contest " << contest_id << " predicted ratings (" << contestants.size() << " rows):\n";
        for (const auto& c : contestants) {
            if (c.participantType != "CONTESTANT") continue;
            cout << "Rank " << c.rank
                 << " | " << c.handle
                 << " | Pre: " << c.preContestRating
                 << " | Seed: " << c.seed
                 << " | Pred: " << c.predictedRating
                 << " | Perf: " << c.performance
                 << " | Delta: " << c.delta << "\n";
        }
    }

    return 0;
}
