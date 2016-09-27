
/*
 * Filename:Dependency.c
 * Author:Ty-Rack-US Park
 * Last Modified: 4/8/2015
 * Version: 1.0
 */

#include "DependencyGraph.h"
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <iostream>

using namespace std;

/*
* Constructor
*/
DependencyGraph::DependencyGraph(){
  PairCount = 0;
}

/* Returns how many pairs are in the Dependency Grap
 */
int DependencyGraph::size(){
  return PairCount;
}

DependencyGraph::DependencyGraph(const DependencyGraph &other){

}

DependencyGraph::~DependencyGraph(){

}


/*
 * Checks if the Graph has 's' has any dependents
 */
bool DependencyGraph::HasDependents(string s){
  if(key_to_dependents.count(s) == 1) // Only one element can exist in this map
    return true;
  return false;
}

/*
 * Checks if the Graph has 's' has any dependees
 */
bool  DependencyGraph::HasDependees(string s){
  if(key_to_dependees.count(s) == 1) // Only one element can exist in this map
    return true;
  return false;
}

/*
 * Returns vector of dependents of 's'
 */
vector<string> DependencyGraph::GetDependents(string s){
  if(key_to_dependents.count(s) == 1) {
    vector<string> items = key_to_dependents[s];
    return items;
  } else {
    vector<string> item;
    return item;
  }
}

/*
 * Returns vecto of dependees of 't'
 */

vector<string> DependencyGraph::GetDependees(string t){
  if(key_to_dependees.count(t) == 1) {
    vector<string> items = key_to_dependees[t];
    return items;
  } else {
    vector<string> item;
    return item;
  }
}



/*
 * Adds the Dependency as (dependee, dependent)
 */
void DependencyGraph::AddDependency(string s, string t){

  if(key_to_dependents.count(s) == 1) { // This ensures only 1 unique is entered into the graph
    //if dependent exists
    if(find(key_to_dependents[s].begin(), key_to_dependents[s].end(), t) == key_to_dependents[s].end()){
      key_to_dependents[s].push_back(t);    
    }
  } else {
    //if dependent doesn't exist, create a new vector with it and add it to the graph
    vector<string> newDependents;
    newDependents.push_back(t);
    key_to_dependents[s] = newDependents;
  }
  
  //dependees
 if(key_to_dependees.count(t) == 1) { // This ensures only 1 unique is entered into the graph
      //if dependee exists
   if(find(key_to_dependees[t].begin(), key_to_dependees[t].end(), s) == key_to_dependees[t].end()){ // Only insert into the vector if it does not already exist
      key_to_dependees[t].push_back(s);
      PairCount++;
    }
  } else {
    //if dependee doesn't exist, create a new vector with it and add it to the graph
    vector<string> newDependees;
    newDependees.push_back(s);
    key_to_dependees[t] = newDependees;
    PairCount++;
  } 

}


/*
 * Removes Dependency for (Dependee, Dependent)
 */
void DependencyGraph::RemoveDependency(string s, string t){
  bool remove = false;
  if(key_to_dependents.count(s) == 1 ){
    //if exists
    if(find(key_to_dependents[s].begin(), key_to_dependents[s].end(), t) != key_to_dependents[s].end()){
      remove = true;
      if(key_to_dependents[s].size() == 1){ // Either completely remove dependency or party remove it
	key_to_dependents.erase(s);
      }else{
	key_to_dependents[s].erase(find(key_to_dependents[s].begin(), key_to_dependents[s].end(), t));
      }
    }
  } if(remove) { 
    PairCount--;
      if(key_to_dependees[t].size() == 1){ // Either completely remove dependency or party remove it
	key_to_dependees.erase(t);
      }else{
	key_to_dependees[t].erase(find(key_to_dependees[t].begin(), key_to_dependees[t].end(), s));
      }
  } 
  
}


/*
 * Replaces dependents of 's' witht the newDenpendents vector provided
 */
void DependencyGraph::ReplaceDependents(string s, vector<string> newDependents){
  if(key_to_dependents.count(s) == 1) {
    for (vector<string>::iterator it = key_to_dependents[s].begin(); it != key_to_dependents[s].end(); ++it)
      RemoveDependency(s, *it);
    for (vector<string>::iterator it = newDependents.begin(); it != newDependents.end(); ++it)
	AddDependency(s, *it);
  } else  { // Only add if it does not exist
    for (vector<string>::iterator it = newDependents.begin(); it != newDependents.end(); ++it)
	AddDependency(s, *it);
  }
 
}

/*
 * Replaces dependees of 's' witht the newDenpendees vector provided
 */
void DependencyGraph::ReplaceDependees(string s, vector<string> newDependees) {
  if(key_to_dependees.count(s) == 1) {
    for (int i = key_to_dependees[s].size() - 1; i >= 0; i--)
      RemoveDependency(key_to_dependees[s].at(i), s);
    for (vector<string>::iterator it = newDependees.begin(); it != newDependees.end(); ++it)
      AddDependency(*it, s);
  } else  { // Only add if it does not exist
    for (vector<string>::iterator it = newDependees.begin(); it != newDependees.end(); ++it)
      AddDependency(*it, s);
  }
}

/*
 * Debigging method to print dependencies.
 */
void DependencyGraph::PrintMap(){
  for(map<string, vector<string> >::iterator it = key_to_dependents.begin(); it != key_to_dependents.end(); it++){
      vector<string> d = it->second;
      for(vector<string>::iterator it2 = d.begin(); it2 != d.end(); it2++)
	cout << "Dependees " << it->first << " Dependents " << *it2 << endl; 
  }

}

