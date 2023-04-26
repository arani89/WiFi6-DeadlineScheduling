
#pragma once

inline const int RU_1992 = 6;
inline const int RU_996 = 5;
inline const int RU_484 = 4;
inline const int RU_242 = 3;
inline const int RU_106 = 2;
inline const int RU_52 = 1;
inline const int  RU_26 = 0;

inline int ndp_data_transferred = 0;
inline double ndp_transmission_time = 0;
inline int ndp_resource_allocated = 0;

// to get time for throughput
inline double ndp_first_bit = -1;
inline double ndp_last_bit = -2;
class InputParser{
    private:
        std::vector <std::string> tokens;
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }
        /// @author iain
        const std::string& getCmdOption(const std::string &option) const{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }
        /// @author iain
        bool cmdOptionExists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
        }
};
class Config {
    public:
        std::vector<int> config;
        std::vector<int> masterconfig;
        int maxRU = RU_484;
        int bandwidth = 40;
        int mcs_range = 1;
        int mcs_lowerval = 11;
        bool variableMCS = false;
        int ndp_ol = 0;
};
