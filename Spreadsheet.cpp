/*
 * Filename:Spreadsheet.cpp
 * Author:Ty-Rack-US Park
 * Last Modified: 4/14/2015
 * Version 1.0
 */


#include "Spreadsheet.h"
#include<boost/regex.hpp>
#include<boost/algorithm/string.hpp>


using namespace boost;
using namespace std;

/*
 * Default constructor
 */
Spreadsheet::Spreadsheet()
{
}


/*
* Reads from file provided. Loads the given spreadsheet if it exists. 
* If not, makes a new one.
*/
Spreadsheet::Spreadsheet(const char* fname)
{
  string line;
  filename = fname; 
  circular = 11;
  std::string f;

  // Append a .sprd to the filename
  f.append(filename);
  f.append(".sprd");

  // Open the specified file
  ifstream sprdfile (f);
 
  // If the file exists
  if (sprdfile.is_open())
    {
      // Read every line from the file
      while (getline (sprdfile,line) )
	{
	  // Parse the line from the file into cell name and contents
	  std::string cellname, contents;
	  stringstream ss(line);	  
	  ss >> cellname;

	  // Contents can have spaces in them
	  string c;
	  while(ss >> c)
	    contents = contents + c + " ";
	  
	  // Remove the last " "
	  contents = contents.substr(0, contents.size()-1);

	  // Call setCell to add the cell and its contents to the spreadsheet
	  setCell(cellname, contents);

	  // Remove the change from the undo stack
	  undo_stack.pop_back();
	}

      // Done reading, close the file
      sprdfile.close();
    }
  else
    {
      // File does not exist on server, if a change is made it will be saved
      cout << "Spreadsheet does not exist on server yet" << endl;             
    }
}

/*
 * Set the contents of a cell to the provided contents
 * If the contents are "", it is removed from the map
 *
 * Return - false if a circular dependency occured
 *          true  if the change is valid
 */
bool Spreadsheet::setCell(std::string name, std::string contents)
{

  // If the contents aren't being changed
  try    
    {
      if(cells.at(name).compare(contents) == 0)		
	return true;		     
    }
  catch (const std::out_of_range& oor)
    { }

  // Normalize the cell name by capitalizing the letter
  char c = name[0];
  if (c >= 97 && c <= 122)
    name[0] -= 32;

  // If the content contains a formula
  if(contents.substr(0,1).compare("=")==0)	    
    {
      string content_formula = contents.substr(1);
      string token;
      vector<string> variables;
      vector<string> temp;
      vector<string> dependees_backup;
      vector<string> dependents;

      // Keep track of old dependees in case of circular dependency
      dependees_backup = graph.GetDependees(name);

      // Split the contents by the operators
      string delim("+-/* ");
      boost::split(temp, content_formula, boost::is_any_of(delim));
      
      // Find all variables in formula
      for(int i=0; i<temp.size(); i++)
	{	  
	  token = temp.at(i);
	  char c = token[0];

	  // First character is uppercase letter
	  if (c >= 65 && c <= 90)	    
	      variables.push_back(token);	    

	  // First character is lowercase letter
	  else if (c >= 97 && c <= 122)
	    {
	      // Capitalize the variable
	      token[0] -= 32;
	      variables.push_back(token);
	    }
	}
            
      // Replace the dependees of the current cell with the variables in the formula
      graph.ReplaceDependees(name, variables);
      graph.PrintMap();
      // Search for circular dependencies
      try
	{
	  getCellsToRecalculate(name);
	}
      catch(int i)
	{
	  if(i == circular)
	    {
	      // Undo the changes in the dependency graph
	      cout << "Circular dependency!" << endl;
	      graph.ReplaceDependees(name, dependees_backup);
	      return false;
	    }
	}
    }
  // Setting contents to a string or double
  else
    {
      // Clear the dependees
      graph.ReplaceDependees(name, vector<string>());
    }

  // Find the contents of the cell before making changes
  map<string, string>::iterator it = cells.find(name);
  string prev_contents;

  // If cell is non-empty, get the contents
  if (it != cells.end())
      prev_contents = it->second;

  // Otherwise cell contains an empty string
  else
    prev_contents = "";

  // Cells containing an empty string need to be removed from the map
  if(contents.compare("")==0)
    cells.erase(name);

  // Remove the previous cell and update with the changes
  else
    {
      // Insert the cell into the map
      std::pair<std::map<string,string>::iterator, bool> ret;
      ret = cells.insert ( std::pair<std::string,std::string>(name, contents));

      // If the cell was already in the map
      if(ret.second == false)
      	{
	  // Erase the previous contents and insert the new changes
	  cells.erase(name);
	  cells.insert ( std::pair<std::string,std::string>(name, contents));
      	}
    }

  // Add the change to the undo stack
  undo_stack.push_back(pair<string, string>(name, prev_contents));

  // Change has been made, save the file
  saveFile();
  return true;
}

/*
 * Undos the previous change made to the spreadsheet
 */
std::pair<std::string, std::string> Spreadsheet::undo()
{

  // Undo stack is empty
  if (undo_stack.size() == 0)
      return std::pair<std::string, std::string>("ERROR", "ERROR");
  else
    {
      // Get the previous cell and its contents
      pair<string, string> prev;
      string name, contents;
      prev = undo_stack.back();
      name = prev.first;
      contents = prev.second;

      // Create the return value for the server
      std::pair<std::string,std::string> undo = pair<string,string>(name, contents);

      // Remove the change from the stack
      undo_stack.erase(undo_stack.end()-1);


      setCell(name, contents);
      undo_stack.erase(undo_stack.end()-1);

    
      // Changes were made, save the file
      saveFile();

      return undo;
    }

}

/*
 * Save the cells and their contents to the file
 *
 */
bool Spreadsheet::saveFile()
{
  // Append a .sprd to their filename
  std::string f;
  f.append(filename);
  f.append(".sprd");

  ofstream myfile (f);
  if (myfile.is_open())
    {
      // Write the cell name and its contents to the file
      // Format: cell contents\n
      for(std::map<string, string>::iterator it = cells.begin(); it != cells.end(); ++it)
	{
	  myfile << it->first + " " + it->second + "\n"; 
	}
      myfile.close();
      return true;
    }
  else 
    {
      // File didn't open
      cout << "Unable to open file" << endl;
      return false;
    }
}

std::set<std::string> Spreadsheet::getCellsToRecalculate(std::set<std::string> names)
{
  
  std::set<std::string> visited;
  std::set<std::string> changed;
  for(std::set<std::string>::iterator it = names.begin(); it != names.end(); ++it) { 
    const bool vis = visited.find(*it) != visited.end();
     if(!vis){
       visit(*it, *it, visited, changed);
     }
  }
  return changed;

  
}

std::set<std::string> Spreadsheet::getCellsToRecalculate(std::string name)
{
  std::set<std::string> s;
  s.insert(name);
  return getCellsToRecalculate(s); 
}

void Spreadsheet::visit(std::string start,  std::string name, std::set<std::string> visited, std::set<std::string> changed)
{
  visited.insert(name);
  std::vector<std::string> d  = graph.GetDependents(name);
  for(std::vector<std::string>::iterator it = d.begin(); it != d.end(); ++it) {
    const bool vis = visited.find(*it) != visited.end();
    if(*it == start){
      throw circular;
    } else if(!vis){
      visit(start, *it, visited, changed);
    }
  }

  changed.insert(changed.begin(), name);
}


