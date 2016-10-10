/* 
 * File:   config.h
 * Author: guiwen
 *
 * Created on July 2, 2014, 9:46 PM
 */

#ifndef CONFIG_H
#define	CONFIG_H

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include "singleton.h"
#include "kstwo.h"
using namespace std;

class Configuration
{
public:
    // clear all values
    void Clear();

    // load a configuration file
    bool Load(const string& File);
    // load ks configuration file
    bool LoadKS(const string& File);
    // check if value associated with given key exists
    bool Contains(const string& key) const;

    // get value associated with given key
    bool Get(const string& key, string& value) const;
    bool Get(const string& key, int&    value) const;
    bool Get(const string& key, long&   value) const;
    bool Get(const string& key, double& value) const;
    bool Get(const string& key, bool&   value) const;
    int sum_deltas[INDEX_NUM][HIS_NUM];
    int num_points[INDEX_NUM];  // 20 histogram, each represent a mean such as 350-399,  number of point in each histogram;
private:
    // the container
    map<string,string> data;

    // remove leading and trailing tabs and spaces
    string Trim(const string& str);

 // 20 histogram, each represent a mean such as 350-399,  number of point in each histogram;    
};
typedef Singleton<Configuration> conf;

#endif	/* CONFIG_H */

