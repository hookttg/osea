#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <glog/logging.h>
#include "kstwo.h"
#include "config.h"
#include "singleton.h"
// ----------------------------------
// method implementations
// ----------------------------------

void Configuration::Clear() {
    data.clear();
}

bool Configuration::LoadKS(const string& file) {
    ifstream inFile(file.c_str());

    if (!inFile.good()) {
        LOG(ERROR) << "Cannot read configuration file"<<file ;        
        return false;
    }

    memset(num_points, 0, sizeof (int)*INDEX_NUM);
    memset(sum_deltas, 0, sizeof (int)*INDEX_NUM * HIS_NUM);

    FILE *dist_file = fopen(file.c_str(), "r");
    if (dist_file == NULL) {
        LOG(ERROR) << "No " << file;
        return false;
    }
    for (int i = 0; i < INDEX_NUM; i++) {
        int index;
        fscanf(dist_file, "%d %d\n", &index, &num_points[i]);
        for (int j = 0; j < HIS_NUM; j++) {
            fscanf(dist_file, "%d %d\n", &index, &sum_deltas[i][j]);
        }
    }
    fclose(dist_file);
    return true;
}
bool Configuration::Load(const string& file) {
    ifstream inFile(file.c_str());

    if (!inFile.good()) {
        LOG(ERROR) << "Cannot read configuration file:"<<file ;
        return false;
    }

    while (inFile.good() && !inFile.eof()) {
        string line;
        getline(inFile, line);

        // filter out comments
        if (!line.empty()) {
            int pos = line.find('#');

            if (pos != string::npos) {
                line = line.substr(0, pos);
            }
        }

        // split line into key and value
        if (!line.empty()) {
            int pos = line.find('=');

            if (pos != string::npos) {
                string key = Trim(line.substr(0, pos));
                string value = Trim(line.substr(pos + 1));

                if (!key.empty() && !value.empty()) {
                    data[key] = value;
                }
            }
        }
    }


    memset(num_points, 0, sizeof (int)*INDEX_NUM);
    memset(sum_deltas, 0, sizeof (int)*INDEX_NUM * HIS_NUM);

   FILE *dist_file = fopen("/etc/dist.conf", "r");
    //FILE *dist_file = fopen("/etc/dist.conf", "r");
    if (dist_file == NULL) {
        LOG(ERROR) << "no /etc/dist.conf";
        return false;
    }
    for (int i = 0; i < INDEX_NUM; i++) {
        int index;
        fscanf(dist_file, "%d %d\n", &index, &num_points[i]);
        for (int j = 0; j < HIS_NUM; j++) {
            fscanf(dist_file, "%d %d\n", &index, &sum_deltas[i][j]);
        }
    }
    fclose(dist_file);
    return true;
}
bool Configuration::Contains(const string& key) const {
    return data.find(key) != data.end();
}

bool Configuration::Get(const string& key, string& value) const {
    map<string, string>::const_iterator iter = data.find(key);

    if (iter != data.end()) {
        value = iter->second;
        return true;
    } else {
        return false;
    }
}

bool Configuration::Get(const string& key, int& value) const {
    string str;

    if (Get(key, str)) {
        value = atoi(str.c_str());
        return true;
    } else {
        return false;
    }
}

bool Configuration::Get(const string& key, long& value) const {
    string str;

    if (Get(key, str)) {
        value = atol(str.c_str());
        return true;
    } else {
        return false;
    }
}

bool Configuration::Get(const string& key, double& value) const {
    string str;

    if (Get(key, str)) {
        value = atof(str.c_str());
        return true;
    } else {
        return false;
    }
}

bool Configuration::Get(const string& key, bool& value) const {
    string str;

    if (Get(key, str)) {
        value = (str == "true");
        return true;
    } else {
        return false;
    }
}

string Configuration::Trim(const string& str) {
    int first = str.find_first_not_of(" \t");
    string news="";
    if (first != string::npos) {
        int last = str.find_last_not_of(" \t");

        news= str.substr(first, last - first + 1);
        return news;
    } 
    
    return news;
    
}

