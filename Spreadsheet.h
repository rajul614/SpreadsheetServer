/*
 * Filename:Spreadsheet.h
 * Author:Ty-Rack-US Park
 * Last Modified: 4/8/2015
 * Version 1.0
 */

#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "DependencyGraph.h"
#include <string.h>
#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <mutex>

class Spreadsheet
{
 public:
  Spreadsheet(); // Default constructor
  Spreadsheet(const char* fname); // Constructs spreadsheet with given filename
 
  bool setCell(std::string name, std::string contents); // Set the contents of a cell
  std::pair<std::string, std::string> undo(); // Undo previous change on spreadsheet
  bool saveFile(); // Save the spreadsheet to file

  std::mutex lock; // For locking simultaneous access by multiple threads

  const char* filename; // Name of spreadsheet file
  std::vector<int> sockets; // List of sockets accessing the spreadsheet
  std::map<std::string, std::string> cells; // Map of cell names to their contents
  std::vector<std::pair<std::string, std::string> > undo_stack; // Stack of changes made to the server

 private:
  DependencyGraph graph; // Dependency Graph
  int circular; // Identifies when circular dependency occurs

  // Find dependent cells and catches circular dependencies
  std::set<std::string> getCellsToRecalculate(std::set<std::string> names); 
  std::set<std::string> getCellsToRecalculate(std::string name);
  void visit(std::string start, std::string name, std::set<std::string> visited,  std::set<std::string> changed);
  
  };

#endif
