#pragma once

#include <string>
#include <vector>

namespace CodeforcesAPI {

    struct Contestant {
        std::string handle;
        std::string participantType;
        int rank = 0;
        int preContestRating = 0;
        double seed = 0.0;
        double meanRank = 0.0;
        int predictedRating = 0;
        int performance = 0;
        int delta = 0;
    };
    std::string FetchData(const std::string& url);

    std::vector<Contestant> ProcessContestStandings(const std::string& json_data);

    void PopulatePreContestRatings(std::vector<Contestant>& contestants, int contest_id);
}