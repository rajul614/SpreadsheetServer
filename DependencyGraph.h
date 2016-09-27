/*
 * Filename:Dependency.h
 * Author:Ty-Rack-US Park
 * Last Modified: 4/8/2015
 * Version: 1.0
 */
#ifndef DEPENDENCYGRAPH_H
#define DEPENDENCYGRAPH_H

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <set>


/*
 * This class represents a data structure for representing different
 * cells in a spreadsheet and the relations of those cells towards each other. 
 */
class DependencyGraph {

 public:
  DependencyGraph(); // Default Constructor Empty Dependency Graph
  DependencyGraph(const DependencyGraph &other); // Copy constructor
  ~DependencyGraph();

 
  int size(); //Returns how many pairs are in the Dependency Graph
  std::vector<std::string> GetDependents(std::string s);//  Returns vector of dependents of 's'
  std::vector<std::string> GetDependees(std::string s);//   Returns vecto of dependees of 't'
 bool HasDependents(std::string s);
 bool HasDependees(std::string s);
 void AddDependency(std::string s, std::string t);//   Adds the Dependency as (dependee, dependent)
 void RemoveDependency(std::string s, std::string t);//   Removes Dependency for (Dependee, Dependent)
 void ReplaceDependents(std::string s, std::vector<std::string> newDependents);//   Replaces dependents of 's' witht the newDenpendents vector provided
 void ReplaceDependees(std::string s, std::vector<std::string> newDependees);// Replaces dependees of 's' witht the newDenpendees vector provided
 void PrintMap(); // Debigging method to print dependencies.

 private:
  int PairCount;
  std::map<std::string, std::vector<std::string> > key_to_dependents;
  std::map<std::string, std::vector<std::string> > key_to_dependees;
  
};

 #endif

