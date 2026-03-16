#include "api_handler.hpp"
#include <fstream>
#include <iostream>
#include <cctype>
#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include <windows.h>
#include <urlmon.h>
#include "simdjson.h"


using namespace std;
using namespace simdjson;

namespace CodeforcesAPI {

    bool equals_ignore_case(const string& a, const string& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); i++) {
            if (tolower(static_cast<unsigned char>(a[i])) != tolower(static_cast<unsigned char>(b[i]))) {
                return false;
            }
        }
        return true;
    }
    // Downloads JSON from the given URL to a temp file and returns its contents as a string 

    // (WINDOWS ONLY: uses URLDownloadToFileA from urlmon.h) 

    string fetch_api_data(const string& url) { 
        const string temp_path = "cf_api_response.tmp.json";
        cout << "Fetching from Codeforces API...\n";
        HRESULT res = URLDownloadToFileA(NULL, url.c_str(), temp_path.c_str(), 0, NULL);
        if (res == S_OK) {
            ifstream in(temp_path, ios::binary);
            if (in) {
                return string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
            }
        } else {
            cerr << "Windows API Download failed. HRESULT Error Code: " << res << "\n";
        }
        return "";
    }

    // Parses and extracts rank, handle, and participantType for each row
    vector<Contestant> process_contest_standings(const string& json_data) {
        vector<Contestant> contestants;
        if (json_data.empty()) return contestants;

        ondemand::parser parser;
        padded_string padded(json_data);
        ondemand::document doc; 
        auto error = parser.iterate(padded).get(doc);
        if (error) {
            cerr << "JSON Parse Error: " << error_message(error) << "\n";
            return contestants;
        }
        ondemand::array rows;
        error = doc["result"]["rows"].get(rows);
        if (error) {
            cerr << "Could not find 'rows' in JSON.\n";
            return contestants;
        }
        for (ondemand::object row : rows) {
            Contestant c{};
            int64_t rank_value = 0;
            if (!row["rank"].get(rank_value)) {
                c.rank = static_cast<int>(rank_value);
            }
            string_view participant_type_view;
            if (!row["party"]["participantType"].get(participant_type_view)) {
                c.participantType = string(participant_type_view);
            }
            ondemand::array members;
            if (!row["party"]["members"].get(members)) {
                for (ondemand::object member : members) {
                    string_view handle_view;
                    member["handle"].get(handle_view);
                    c.handle = string(handle_view);
                    break; 
                }
            }
            contestants.push_back(c);
        }

        return contestants;
    }

    // For contests that are already over . used this api just to get the rating of the contestant before the round. 
    // (just for testing purposes actually on old contests nvm)
    bool build_rating_changes_map(int contest_id, unordered_map<string, int>& rating_map) {
        const string url =
            "https://codeforces.com/api/contest.ratingChanges?contestId=" +
            to_string(contest_id);

        const string json_data = fetch_api_data(url);
        if (json_data.empty()) return false;
        rating_map.clear();
        ondemand::parser parser;
        padded_string padded(json_data);
        ondemand::document doc;
        auto error = parser.iterate(padded).get(doc);
        if (error) return false;
        string_view status;
        if (doc["status"].get(status)) return false;
        if (status != "OK") return false;
        ondemand::array changes;
        if (doc["result"].get(changes)) return false;
        size_t loaded_count = 0;
        for (ondemand::object change : changes) {
            string_view handle_view;
            if (change["handle"].get(handle_view)) continue;
            int64_t old_rating_value = 0;
            if (change["oldRating"].get(old_rating_value)) continue;
            string h(handle_view);
            for (char& ch : h) ch = tolower(static_cast<unsigned char>(ch));
            rating_map[h] = static_cast<int>(old_rating_value);
            ++loaded_count;
        }

        return loaded_count > 0;
    }

    // Get all the user who have particiapted in atleast one contest and their current rating before the contest. Data for ongoing contest , i.e real use case
    bool build_rated_list_map(unordered_map<string, int>& rating_map) {
        const string url =
            "https://codeforces.com/api/user.ratedList";

        const string json_data = fetch_api_data(url);
        if (json_data.empty()) return false;
        ondemand::parser parser;
        padded_string padded(json_data);
        ondemand::document doc;
        auto error = parser.iterate(padded).get(doc);
        if (error) return false;
        string_view status;
        if (doc["status"].get(status)) return false;
        if (status != "OK") return false;
        ondemand::array users;
        if (doc["result"].get(users)) return false;
        for (ondemand::object user : users) {
            string_view handle_view;
            if (user["handle"].get(handle_view)) continue;
            int64_t rating_value = 0;
            if (user["rating"].get(rating_value)) continue;
            string h(handle_view);
            for (char& ch : h) ch = tolower(static_cast<unsigned char>(ch));
            rating_map[h] = static_cast<int>(rating_value);
        }
        return true;
    }
    std::string FetchData(const std::string& url) {
        return fetch_api_data(url);
    }
    std::vector<Contestant> ProcessContestStandings(const std::string& json_data) {
        return process_contest_standings(json_data);
    }

    // Compiles the above two get rating functions to load the ratings into the contestant vector.
    void PopulatePreContestRatings(std::vector<Contestant>& contestants, int contest_id) {
        unordered_map<string, int> rating_map;
        bool have_rating_changes = build_rating_changes_map(contest_id, rating_map);
        if (!have_rating_changes) {
            build_rated_list_map(rating_map);
        }
        for (auto& c : contestants) {
            string key = c.handle;
            for (char& ch : key) ch = tolower(static_cast<unsigned char>(ch));
            auto it = rating_map.find(key);
            c.preContestRating = (it != rating_map.end()) ? it->second : 0;
        }
    }
}